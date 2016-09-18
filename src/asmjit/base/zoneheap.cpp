// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../base/utils.h"
#include "../base/zoneheap.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ZoneHeap - Helpers]
// ============================================================================

static bool ZoneHeap_hasDynamicBlock(ZoneHeap* self, ZoneHeap::DynamicBlock* block) noexcept {
  ZoneHeap::DynamicBlock* cur = self->_dynamicBlocks;
  while (cur) {
    if (cur == block)
      return true;
    cur = cur->next;
  }
  return false;
}

// ============================================================================
// [asmjit::ZoneHeap - Init / Reset]
// ============================================================================

void ZoneHeap::reset(Zone* zone) noexcept {
  // Free dynamic blocks.
  DynamicBlock* block = _dynamicBlocks;
  while (block) {
    DynamicBlock* next = block->next;
    ASMJIT_FREE(block);
    block = next;
  }

  // Zero the entire class and initialize to the given `zone`.
  ::memset(this, 0, sizeof(*this));
  _zone = zone;
}

// ============================================================================
// [asmjit::ZoneHeap - Alloc / Release]
// ============================================================================

void* ZoneHeap::_alloc(size_t size, size_t& allocatedSize) noexcept {
  ASMJIT_ASSERT(isInitialized());

  // We use our memory pool only if the requested block is of a reasonable size.
  uint32_t slot;
  if (_getSlotIndex(size, slot, allocatedSize)) {
    // Slot reuse.
    uint8_t* p = reinterpret_cast<uint8_t*>(_slots[slot]);
    size = allocatedSize;

    if (p) {
      _slots[slot] = reinterpret_cast<Slot*>(p)->next;
      //printf("ALLOCATED %p of size %d (SLOT %d)\n", p, int(size), slot);
      return p;
    }

    // So use Zone to allocate a new chunk for us. But before we use it, we
    // check if there is enough room for the new chunk in zone, and if not,
    // we redistribute the remaining memory in Zone's current block into slots.
    Zone* zone = _zone;
    p = Utils::alignTo(zone->getCursor(), kBlockAlignment);
    size_t remain = (p <= zone->getEnd()) ? (size_t)(zone->getEnd() - p) : size_t(0);

    if (ASMJIT_LIKELY(remain >= size)) {
      zone->setCursor(p + size);
      //printf("ALLOCATED %p of size %d (SLOT %d)\n", p, int(size), slot);
      return p;
    }
    else {
      // Distribute the remaining memory to suitable slots.
      if (remain >= kLoGranularity) {
        do {
          size_t distSize = Utils::iMin<size_t>(remain, kLoMaxSize);
          uint32_t distSlot = static_cast<uint32_t>((distSize - kLoGranularity) / kLoGranularity);
          ASMJIT_ASSERT(distSlot < kLoCount);

          reinterpret_cast<Slot*>(p)->next = _slots[distSlot];
          _slots[distSlot] = reinterpret_cast<Slot*>(p);

          p += distSize;
          remain -= distSize;
        } while (remain >= kLoGranularity);
        zone->setCursor(p);
      }

      p = static_cast<uint8_t*>(zone->_alloc(size));
      if (ASMJIT_UNLIKELY(!p)) {
        allocatedSize = 0;
        return nullptr;
      }

      //printf("ALLOCATED %p of size %d (SLOT %d)\n", p, int(size), slot);
      return p;
    }
  }
  else {
    // Allocate a dynamic block.
    size_t overhead = sizeof(DynamicBlock) + sizeof(DynamicBlock*) + kBlockAlignment;

    // Handle a possible overflow.
    if (ASMJIT_UNLIKELY(overhead >= ~static_cast<size_t>(0) - size))
      return nullptr;

    void* p = ASMJIT_ALLOC(size + overhead);
    if (ASMJIT_UNLIKELY(!p)) {
      allocatedSize = 0;
      return nullptr;
    }

    // Link as first in `_dynamicBlocks` double-linked list.
    DynamicBlock* block = static_cast<DynamicBlock*>(p);
    DynamicBlock* next = _dynamicBlocks;

    if (next)
      next->prev = block;

    block->prev = nullptr;
    block->next = next;
    _dynamicBlocks = block;

    // Align the pointer to the guaranteed alignment and store `DynamicBlock`
    // at the end of the memory block, so `_releaseDynamic()` can find it.
    p = Utils::alignTo(static_cast<uint8_t*>(p) + sizeof(DynamicBlock) + sizeof(DynamicBlock*), kBlockAlignment);
    reinterpret_cast<DynamicBlock**>(p)[-1] = block;

    allocatedSize = size;
    //printf("ALLOCATED DYNAMIC %p of size %d\n", p, int(size));
    return p;
  }
}

void* ZoneHeap::_allocZeroed(size_t size, size_t& allocatedSize) noexcept {
  ASMJIT_ASSERT(isInitialized());

  void* p = _alloc(size, allocatedSize);
  if (ASMJIT_UNLIKELY(!p)) return p;
  return ::memset(p, 0, allocatedSize);
}

void ZoneHeap::_releaseDynamic(void* p, size_t size) noexcept {
  ASMJIT_ASSERT(isInitialized());
  //printf("RELEASING DYNAMIC %p of size %d\n", p, int(size));

  // Pointer to `DynamicBlock` is stored at [-1].
  DynamicBlock* block = reinterpret_cast<DynamicBlock**>(p)[-1];
  ASMJIT_ASSERT(ZoneHeap_hasDynamicBlock(this, block));

  // Unlink and free.
  DynamicBlock* prev = block->prev;
  DynamicBlock* next = block->next;

  if (prev)
    prev->next = next;
  else
    _dynamicBlocks = next;

  if (next)
    next->prev = prev;

  ASMJIT_FREE(block);
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"
