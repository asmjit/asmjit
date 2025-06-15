// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/support.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

// Zone - Globals
// ==============

// Zero size block used by `Zone` that doesn't have any memory allocated. Should be allocated in read-only memory
// and should never be modified.
const Zone::Block Zone::_zeroBlock {};

static inline void Zone_assignZeroBlock(Zone* zone) noexcept {
  Zone::Block* block = const_cast<Zone::Block*>(&zone->_zeroBlock);
  zone->_ptr = block->data();
  zone->_end = block->data();
  zone->_block = block;
}

static inline void Zone_assignBlock(Zone* zone, Zone::Block* block) noexcept {
  zone->_ptr = Support::alignUp(block->data(), Globals::kZoneAlignment);
  zone->_end = block->data() + block->size;
  zone->_block = block;
}

// Zone - Initialization & Reset
// =============================

void Zone::_init(size_t blockSize, const Support::Temporary* temporary) noexcept {
  ASMJIT_ASSERT(blockSize >= kMinBlockSize);
  ASMJIT_ASSERT(blockSize <= kMaxBlockSize);

  Zone_assignZeroBlock(this);

  size_t blockSizeShift = Support::bitSizeOf<size_t>() - Support::clz(blockSize);

  _currentBlockSizeShift = uint8_t(blockSizeShift);
  _minimumBlockSizeShift = uint8_t(blockSizeShift);
  _maximumBlockSizeShift = uint8_t(25); // (1 << 25) Equals 32 MiB blocks (should be enough for all cases)
  _hasStaticBlock = uint8_t(temporary != nullptr);
  _reserved = uint8_t(0u);

  // Setup the first [temporary] block, if necessary.
  if (temporary) {
    Block* block = temporary->data<Block>();
    block->prev = nullptr;
    block->next = nullptr;

    ASMJIT_ASSERT(temporary->size() >= kBlockSize);
    block->size = temporary->size() - kBlockSize;

    Zone_assignBlock(this, block);
  }
}

void Zone::reset(ResetPolicy resetPolicy) noexcept {
  Block* cur = _block;

  // Can't be altered.
  if (cur == &_zeroBlock) {
    return;
  }

  if (resetPolicy == ResetPolicy::kHard) {
    bool hasStatic = hasStaticBlock();
    Block* initial = const_cast<Zone::Block*>(&_zeroBlock);

    _ptr = initial->data();
    _end = initial->data();
    _block = initial;
    _currentBlockSizeShift = _minimumBlockSizeShift;

    // Since cur can be in the middle of the double-linked list, we have to traverse both directions (`prev` and
    // `next`) separately to visit all.
    Block* next = cur->next;
    do {
      Block* prev = cur->prev;

      if (prev == nullptr && hasStatic) {
        // If this is the first block and this Zone is actually a ZoneTmp then the first block cannot be freed.
        cur->prev = nullptr;
        cur->next = nullptr;
        Zone_assignBlock(this, cur);
        break;
      }

      ::free(cur);
      cur = prev;
    } while (cur);

    cur = next;
    while (cur) {
      next = cur->next;
      ::free(cur);
      cur = next;
    }
  }
  else {
    while (cur->prev) {
      cur = cur->prev;
    }
    Zone_assignBlock(this, cur);
  }
}

// Zone - Alloc
// ============

void* Zone::_alloc(size_t size) noexcept {
  ASMJIT_ASSERT(Support::isAligned(size, Globals::kZoneAlignment));

  // Overhead of block alignment (we want to achieve at least Globals::kZoneAlignment).
  constexpr size_t kAlignmentOverhead =
    (Globals::kZoneAlignment <= Globals::kAllocAlignment)
      ? size_t(0)
      : Globals::kZoneAlignment - Globals::kAllocAlignment;

  // Total overhead per a block allocated with malloc - we want to decrease the size of each block by this value to
  // make sure that malloc is not mmapping() additional page just to hold metadata.
  constexpr size_t kBlockSizeOverhead = kBlockSize + Globals::kAllocOverhead + kAlignmentOverhead;

  Block* curBlock = _block;
  Block* next = curBlock->next;

  // If the `Zone` has been soft-reset the current block doesn't have to be the last one. Check if there is a block
  // that can be used instead of allocating a new one. If there is a `next` block it's completely unused, we don't
  // have to check for remaining bytes in that case.
  if (next) {
    uint8_t* ptr = Support::alignUp(next->data(), Globals::kZoneAlignment);
    uint8_t* end = next->data() + next->size;

    if (size <= (size_t)(end - ptr)) {
      _block = next;
      _ptr = ptr + size;
      _end = end;
      return static_cast<void*>(ptr);
    }
  }

  // Calculates the initial size of a next block - in most cases this would be enough for the allocation. In
  // general we want to gradually increase block size when more and more blocks are allocated until the maximum
  // block size. Since we use shifts (aka log2(size) sizes) we just need block count and minumum/maximum block
  // size shift to calculate the final size.
  uint32_t blockSizeShift = uint32_t(_currentBlockSizeShift);
  size_t blockSize = size_t(1) << blockSizeShift;

  // Allocate a new block. We have to accommodate all possible overheads so after the memory is allocated and
  // then properly aligned there will be size for the requested memory. In 99.9999% cases this is never a problem,
  // but we must be sure that even rare border cases would allocate properly.

  if (ASMJIT_UNLIKELY(size > blockSize - kBlockSizeOverhead)) {
    // If the requested size is larger than a default calculated block size -> increase block size so the
    // allocation would be enough to fit the requested size.
    if (ASMJIT_UNLIKELY(size > SIZE_MAX - kBlockSizeOverhead)) {
      // This would probably never happen in practice - however, it needs to be done to stop malicious cases like
      // `alloc(SIZE_MAX)`.
      return nullptr;
    }
    blockSize = size + kAlignmentOverhead + kBlockSize;
  }
  else {
    blockSize -= Globals::kAllocOverhead;
  }

  // Allocate new block.
  Block* newBlock = static_cast<Block*>(::malloc(blockSize));

  if (ASMJIT_UNLIKELY(!newBlock)) {
    return nullptr;
  }

  // blockSize includes the struct size, which must be avoided when assigning the size to a newly allocated block.
  size_t realBlockSize = blockSize - kBlockSize;

  // Align the pointer to `minimumAlignment` and adjust the size of this block accordingly. It's the same as using
  // `minimumAlignment - Support::alignUpDiff()`, just written differently.
  newBlock->prev = nullptr;
  newBlock->next = nullptr;
  newBlock->size = realBlockSize;

  if (curBlock != &_zeroBlock) {
    newBlock->prev = curBlock;
    curBlock->next = newBlock;

    // Does only happen if there is a next block, but the requested memory can't fit into it. In this case a new
    // buffer is allocated and inserted between the current block and the next one.
    if (next) {
      newBlock->next = next;
      next->prev = newBlock;
    }
  }

  uint8_t* ptr = Support::alignUp(newBlock->data(), Globals::kZoneAlignment);
  uint8_t* end = newBlock->data() + realBlockSize;

  _ptr = ptr + size;
  _end = end;
  _block = newBlock;
  _currentBlockSizeShift = uint8_t(Support::min<uint32_t>(uint32_t(blockSizeShift) + 1u, _maximumBlockSizeShift));

  ASMJIT_ASSERT(_ptr <= _end);
  return static_cast<void*>(ptr);
}

void* Zone::allocZeroed(size_t size) noexcept {
  ASMJIT_ASSERT(Support::isAligned(size, Globals::kZoneAlignment));

  void* p = alloc(size);
  if (ASMJIT_UNLIKELY(!p)) {
    return p;
  }

  return memset(p, 0, size);
}

void* Zone::dup(const void* data, size_t size, bool nullTerminate) noexcept {
  if (ASMJIT_UNLIKELY(!data || !size)) {
    return nullptr;
  }

  ASMJIT_ASSERT(size != SIZE_MAX);

  size_t allocSize = Support::alignUp(size + size_t(nullTerminate), Globals::kZoneAlignment);
  uint8_t* m = alloc<uint8_t>(allocSize);

  if (ASMJIT_UNLIKELY(!m)) {
    return nullptr;
  }

  // Clear the last 8 bytes, which clears potential padding and null terminates at the same time.
  static_assert(Globals::kZoneAlignment == 8u, "the code below must be fixed if zone alignment was changed");
  Support::storeu<uint64_t>(m + allocSize - sizeof(uint64_t), 0u);

  memcpy(m, data, size);
  return static_cast<void*>(m);
}

char* Zone::sformat(const char* fmt, ...) noexcept {
  if (ASMJIT_UNLIKELY(!fmt)) {
    return nullptr;
  }

  char buf[512];
  size_t size;
  va_list ap;

  va_start(ap, fmt);
  size = unsigned(vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf) - 1, fmt, ap));
  va_end(ap);

  buf[size++] = 0;
  return static_cast<char*>(dup(buf, size));
}

// ZoneAllocator - Utilities
// =========================

#if defined(ASMJIT_BUILD_DEBUG)
static bool ZoneAllocator_hasDynamicBlock(ZoneAllocator* self, ZoneAllocator::DynamicBlock* block) noexcept {
  ZoneAllocator::DynamicBlock* cur = self->_dynamicBlocks;
  while (cur) {
    if (cur == block) {
      return true;
    }
    cur = cur->next;
  }
  return false;
}
#endif

// ZoneAllocator - Initialization & Reset
// ======================================

void ZoneAllocator::reset(Zone* zone) noexcept {
  // Free dynamic blocks.
  DynamicBlock* block = _dynamicBlocks;
  while (block) {
    DynamicBlock* next = block->next;
    ::free(block);
    block = next;
  }

  _zone = zone;
  memset(_slots, 0, sizeof(_slots));
  _dynamicBlocks = nullptr;
}

// ZoneAllocator - Alloc & Release
// ===============================

void* ZoneAllocator::_alloc(size_t size, size_t& allocatedSize) noexcept {
  ASMJIT_ASSERT(isInitialized());

  // Use the memory pool only if the requested block has a reasonable size.
  size_t slot;
  if (_getSlotIndex(size, slot, allocatedSize)) {
    // Slot reuse.
    uint8_t* p = reinterpret_cast<uint8_t*>(_slots[slot]);
    size = allocatedSize;

    if (p) {
      _slots[slot] = reinterpret_cast<Slot*>(p)->next;
      return p;
    }

    _zone->align(kBlockAlignment);
    p = _zone->ptr();
    size_t remain = (size_t)(_zone->end() - p);

    if (ASMJIT_LIKELY(remain >= size)) {
      _zone->setPtr(p + size);
      return p;
    }
    else {
      // Distribute the remaining memory to suitable slots, if possible.
      if (remain >= kLoGranularity) {
        do {
          size_t distSize = Support::min<size_t>(remain, kLoMaxSize);
          uint32_t distSlot = uint32_t((distSize - kLoGranularity) / kLoGranularity);
          ASMJIT_ASSERT(distSlot < kLoCount);

          reinterpret_cast<Slot*>(p)->next = _slots[distSlot];
          _slots[distSlot] = reinterpret_cast<Slot*>(p);

          p += distSize;
          remain -= distSize;
        } while (remain >= kLoGranularity);
        _zone->setPtr(p);
      }

      p = static_cast<uint8_t*>(_zone->_alloc(size));
      if (ASMJIT_UNLIKELY(!p)) {
        allocatedSize = 0;
        return nullptr;
      }

      return p;
    }
  }
  else {
    // Allocate a dynamic block.
    size_t blockOverhead = sizeof(DynamicBlock) + sizeof(DynamicBlock*) + kBlockAlignment;

    // Handle a possible overflow.
    if (ASMJIT_UNLIKELY(blockOverhead >= SIZE_MAX - size)) {
      return nullptr;
    }

    void* p = ::malloc(size + blockOverhead);
    if (ASMJIT_UNLIKELY(!p)) {
      allocatedSize = 0;
      return nullptr;
    }

    // Link as first in `_dynamicBlocks` double-linked list.
    DynamicBlock* block = static_cast<DynamicBlock*>(p);
    DynamicBlock* next = _dynamicBlocks;

    if (next) {
      next->prev = block;
    }

    block->prev = nullptr;
    block->next = next;
    _dynamicBlocks = block;

    // Align the pointer to the guaranteed alignment and store `DynamicBlock`
    // at the beginning of the memory block, so `_releaseDynamic()` can find it.
    p = Support::alignUp(static_cast<uint8_t*>(p) + sizeof(DynamicBlock) + sizeof(DynamicBlock*), kBlockAlignment);
    reinterpret_cast<DynamicBlock**>(p)[-1] = block;

    allocatedSize = size;
    return p;
  }
}

void* ZoneAllocator::_allocZeroed(size_t size, size_t& allocatedSize) noexcept {
  ASMJIT_ASSERT(isInitialized());

  void* p = _alloc(size, allocatedSize);
  if (ASMJIT_UNLIKELY(!p)) {
    return p;
  }
  return memset(p, 0, allocatedSize);
}

void ZoneAllocator::_releaseDynamic(void* p, size_t size) noexcept {
  DebugUtils::unused(size);
  ASMJIT_ASSERT(isInitialized());

  // Pointer to `DynamicBlock` is stored at [-1].
  DynamicBlock* block = reinterpret_cast<DynamicBlock**>(p)[-1];
  ASMJIT_ASSERT(ZoneAllocator_hasDynamicBlock(this, block));

  // Unlink and free.
  DynamicBlock* prev = block->prev;
  DynamicBlock* next = block->next;

  if (prev) {
    prev->next = next;
  }
  else {
    _dynamicBlocks = next;
  }

  if (next) {
    next->prev = prev;
  }

  ::free(block);
}

// Zone - Tests
// ============

#if defined(ASMJIT_TEST)
UNIT(zone_allocator_slots) {
  constexpr size_t kLoMaxSize = ZoneAllocator::kLoCount * ZoneAllocator::kLoGranularity;
  constexpr size_t kHiMaxSize = ZoneAllocator::kHiCount * ZoneAllocator::kHiGranularity + kLoMaxSize;

  for (size_t size = 1; size <= kLoMaxSize; size++) {
    size_t acquired_slot;
    size_t expected_slot = (size - 1) / ZoneAllocator::kLoGranularity;

    EXPECT_TRUE(ZoneAllocator::_getSlotIndex(size, acquired_slot));
    EXPECT_EQ(acquired_slot, expected_slot);
    EXPECT_LT(acquired_slot, ZoneAllocator::kLoCount);
  }

  for (size_t size = kLoMaxSize + 1; size <= kHiMaxSize; size++) {
    size_t acquired_slot;
    size_t expected_slot = (size - kLoMaxSize - 1) / ZoneAllocator::kHiGranularity + ZoneAllocator::kLoCount;

    EXPECT_TRUE(ZoneAllocator::_getSlotIndex(size, acquired_slot));
    EXPECT_EQ(acquired_slot, expected_slot);
    EXPECT_LT(acquired_slot, ZoneAllocator::kLoCount + ZoneAllocator::kHiCount);
  }
}
#endif // ASMJIT_TEST

ASMJIT_END_NAMESPACE
