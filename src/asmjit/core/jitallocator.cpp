// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_JIT

#include "../core/arch.h"
#include "../core/jitallocator.h"
#include "../core/osutils.h"
#include "../core/support.h"
#include "../core/virtmem.h"
#include "../core/zone.h"
#include "../core/zonelist.h"
#include "../core/zonetree.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::JitAllocator - Constants]
// ============================================================================

enum JitAllocatorConstants : uint32_t {
  //! Number of pools to use when `JitAllocator::kOptionUseMultiplePools` is set.
  //!
  //! Each pool increases granularity twice to make memory management more
  //! efficient. Ideal number of pools appears to be 3 to 4 as it distributes
  //! small and large functions properly.
  kJitAllocatorMultiPoolCount = 3,

  //! Minimum granularity (and the default granularity for pool #0).
  kJitAllocatorBaseGranularity = 64,

  //! Maximum block size (16MB).
  kJitAllocatorMaxBlockSize = 1024 * 1024 * 16
};

static inline uint32_t JitAllocator_defaultFillPattern() noexcept {
  // X86 and X86_64 - 4x 'int3' instruction.
  if (ASMJIT_ARCH_X86)
    return 0xCCCCCCCCu;

  // Unknown...
  return 0u;
}

// ============================================================================
// [asmjit::JitAllocator - BitFlipIterator]
// ============================================================================

//! BitWord[] iterator used by `JitAllocator` that can flip the search pattern
//! during iteration.
template<typename T>
class BitFlipIterator {
public:
  ASMJIT_INLINE BitFlipIterator(const T* data, size_t numBitWords, size_t start = 0, T xorMask = 0) noexcept {
    init(data, numBitWords, start, xorMask);
  }

  ASMJIT_INLINE void init(const T* data, size_t numBitWords, size_t start = 0, T xorMask = 0) noexcept {
    const T* ptr = data + (start / Support::bitSizeOf<T>());
    size_t idx = Support::alignDown(start, Support::bitSizeOf<T>());
    size_t end = numBitWords * Support::bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = (*ptr++ ^ xorMask) & (Support::allOnes<T>() << (start % Support::bitSizeOf<T>()));
      while (!bitWord && (idx += Support::bitSizeOf<T>()) < end)
        bitWord = *ptr++ ^ xorMask;
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bitWord;
    _xorMask = xorMask;
  }

  ASMJIT_INLINE bool hasNext() const noexcept {
    return _current != T(0);
  }

  ASMJIT_INLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = Support::ctz(bitWord);
    bitWord ^= T(1u) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += Support::bitSizeOf<T>()) < _end)
      bitWord = *_ptr++ ^ _xorMask;

    _current = bitWord;
    return n;
  }

  ASMJIT_INLINE size_t nextAndFlip() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = Support::ctz(bitWord);
    bitWord ^= Support::allOnes<T>() << bit;
    _xorMask ^= Support::allOnes<T>();

    size_t n = _idx + bit;
    while (!bitWord && (_idx += Support::bitSizeOf<T>()) < _end)
      bitWord = *_ptr++ ^ _xorMask;

    _current = bitWord;
    return n;
  }

  ASMJIT_INLINE size_t peekNext() const noexcept {
    ASMJIT_ASSERT(_current != T(0));
    return _idx + Support::ctz(_current);
  }

  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
  T _xorMask;
};

// ============================================================================
// [asmjit::JitAllocator - Pool]
// ============================================================================

class JitAllocatorBlock;

class JitAllocatorPool {
public:
  ASMJIT_NONCOPYABLE(JitAllocatorPool)

  inline JitAllocatorPool(uint32_t granularity) noexcept
    : blocks(),
      cursor(nullptr),
      blockCount(0),
      granularity(uint16_t(granularity)),
      granularityLog2(uint8_t(Support::ctz(granularity))),
      emptyBlockCount(0),
      totalAreaSize(0),
      totalAreaUsed(0),
      totalOverheadBytes(0) {}

  inline void reset() noexcept {
    blocks.reset();
    cursor = nullptr;
    blockCount = 0;
    totalAreaSize = 0;
    totalAreaUsed = 0;
    totalOverheadBytes = 0;
  }

  inline size_t byteSizeFromAreaSize(uint32_t areaSize) const noexcept { return size_t(areaSize) * granularity; }
  inline uint32_t areaSizeFromByteSize(size_t size) const noexcept { return uint32_t((size + granularity - 1) >> granularityLog2); }

  inline size_t bitWordCountFromAreaSize(uint32_t areaSize) const noexcept {
    using namespace Support;
    return alignUp<size_t>(areaSize, kBitWordSizeInBits) / kBitWordSizeInBits;
  }

  //! Double linked list of blocks.
  ZoneList<JitAllocatorBlock> blocks;
  //! Where to start looking first.
  JitAllocatorBlock* cursor;

  //! Count of blocks.
  uint32_t blockCount;
  //! Allocation granularity.
  uint16_t granularity;
  //! Log2(granularity).
  uint8_t granularityLog2;
  //! Count of empty blocks (either 0 or 1 as we won't keep more blocks empty).
  uint8_t emptyBlockCount;

  //! Number of bits reserved across all blocks.
  size_t totalAreaSize;
  //! Number of bits used across all blocks.
  size_t totalAreaUsed;
  //! Overhead of all blocks (in bytes).
  size_t totalOverheadBytes;
};

// ============================================================================
// [asmjit::JitAllocator - Block]
// ============================================================================

class JitAllocatorBlock : public ZoneTreeNodeT<JitAllocatorBlock>,
                          public ZoneListNode<JitAllocatorBlock> {
public:
  ASMJIT_NONCOPYABLE(JitAllocatorBlock)

  enum Flags : uint32_t {
    //! Block is empty.
    kFlagEmpty = 0x00000001u,
    //! Block is dirty (largestUnusedArea, searchStart, searchEnd).
    kFlagDirty = 0x00000002u,
    //! Block is dual-mapped.
    kFlagDualMapped = 0x00000004u
  };

  inline JitAllocatorBlock(
    JitAllocatorPool* pool,
    VirtMem::DualMapping mapping,
    size_t blockSize,
    uint32_t blockFlags,
    Support::BitWord* usedBitVector,
    Support::BitWord* stopBitVector,
    uint32_t areaSize) noexcept
    : ZoneTreeNodeT(),
      pool(pool),
      mapping(mapping),
      blockSize(blockSize),
      flags(blockFlags),
      areaSize(areaSize),
      areaUsed(0),
      largestUnusedArea(areaSize),
      searchStart(0),
      searchEnd(areaSize),
      usedBitVector(usedBitVector),
      stopBitVector(stopBitVector) {}

  inline uint8_t* roPtr() const noexcept { return static_cast<uint8_t*>(mapping.ro); }
  inline uint8_t* rwPtr() const noexcept { return static_cast<uint8_t*>(mapping.rw); }

  inline bool hasFlag(uint32_t f) const noexcept { return (flags & f) != 0; }
  inline void addFlags(uint32_t f) noexcept { flags |= f; }
  inline void clearFlags(uint32_t f) noexcept { flags &= ~f; }

  inline uint32_t areaAvailable() const noexcept { return areaSize - areaUsed; }

  inline void increaseUsedArea(uint32_t value) noexcept {
    areaUsed += value;
    pool->totalAreaUsed += value;
  }

  inline void decreaseUsedArea(uint32_t value) noexcept {
    areaUsed -= value;
    pool->totalAreaUsed -= value;
  }

  // RBTree default CMP uses '<' and '>' operators.
  inline bool operator<(const JitAllocatorBlock& other) const noexcept { return roPtr() < other.roPtr(); }
  inline bool operator>(const JitAllocatorBlock& other) const noexcept { return roPtr() > other.roPtr(); }

  // Special implementation for querying blocks by `key`, which must be in `[BlockPtr, BlockPtr + BlockSize)` range.
  inline bool operator<(const uint8_t* key) const noexcept { return roPtr() + blockSize <= key; }
  inline bool operator>(const uint8_t* key) const noexcept { return roPtr() > key; }

  //! Link to the pool that owns this block.
  JitAllocatorPool* pool;
  //! Virtual memory mapping - either single mapping (both pointers equal) or
  //! dual mapping, where one pointer is Read+Execute and the second Read+Write.
  VirtMem::DualMapping mapping;
  //! Virtual memory size (block size) [bytes].
  size_t blockSize;

  //! Block flags.
  uint32_t flags;
  //! Size of the whole block area (bit-vector size).
  uint32_t areaSize;
  //! Used area (number of bits in bit-vector used).
  uint32_t areaUsed;
  //! The largest unused continuous area in the bit-vector (or `areaSize` to initiate rescan).
  uint32_t largestUnusedArea;
  //! Start of a search range (for unused bits).
  uint32_t searchStart;
  //! End of a search range (for unused bits).
  uint32_t searchEnd;

  //! Used bit-vector (0 = unused, 1 = used).
  Support::BitWord* usedBitVector;
  //! Stop bit-vector (0 = don't care, 1 = stop).
  Support::BitWord* stopBitVector;
};

// ============================================================================
// [asmjit::JitAllocator - PrivateImpl]
// ============================================================================

class JitAllocatorPrivateImpl : public JitAllocator::Impl {
public:
  inline JitAllocatorPrivateImpl(JitAllocatorPool* pools, size_t poolCount) noexcept
    : JitAllocator::Impl {},
      pools(pools),
      poolCount(poolCount) {}
  inline ~JitAllocatorPrivateImpl() noexcept {}

  //! Lock for thread safety.
  mutable Lock lock;
  //! System page size (also a minimum block size).
  uint32_t pageSize;

  //! Blocks from all pools in RBTree.
  ZoneTree<JitAllocatorBlock> tree;
  //! Allocator pools.
  JitAllocatorPool* pools;
  //! Number of allocator pools.
  size_t poolCount;
};

static const JitAllocator::Impl JitAllocatorImpl_none {};
static const JitAllocator::CreateParams JitAllocatorParams_none {};

// ============================================================================
// [asmjit::JitAllocator - Utilities]
// ============================================================================

static inline JitAllocatorPrivateImpl* JitAllocatorImpl_new(const JitAllocator::CreateParams* params) noexcept {
  VirtMem::Info vmInfo = VirtMem::info();

  if (!params)
    params = &JitAllocatorParams_none;

  uint32_t options = params->options;
  uint32_t blockSize = params->blockSize;
  uint32_t granularity = params->granularity;
  uint32_t fillPattern = params->fillPattern;

  // Setup pool count to [1..3].
  size_t poolCount = 1;
  if (options & JitAllocator::kOptionUseMultiplePools)
    poolCount = kJitAllocatorMultiPoolCount;;

  // Setup block size [64kB..256MB].
  if (blockSize < 64 * 1024 || blockSize > 256 * 1024 * 1024 || !Support::isPowerOf2(blockSize))
    blockSize = vmInfo.pageGranularity;

  // Setup granularity [64..256].
  if (granularity < 64 || granularity > 256 || !Support::isPowerOf2(granularity))
    granularity = kJitAllocatorBaseGranularity;

  // Setup fill-pattern.
  if (!(options & JitAllocator::kOptionCustomFillPattern))
    fillPattern = JitAllocator_defaultFillPattern();

  size_t size = sizeof(JitAllocatorPrivateImpl) + sizeof(JitAllocatorPool) * poolCount;
  void* p = ::malloc(size);
  if (ASMJIT_UNLIKELY(!p))
    return nullptr;

  JitAllocatorPool* pools = reinterpret_cast<JitAllocatorPool*>((uint8_t*)p + sizeof(JitAllocatorPrivateImpl));
  JitAllocatorPrivateImpl* impl = new(Support::PlacementNew { p }) JitAllocatorPrivateImpl(pools, poolCount);

  impl->options = options;
  impl->blockSize = blockSize;
  impl->granularity = granularity;
  impl->fillPattern = fillPattern;
  impl->pageSize = vmInfo.pageSize;

  for (size_t poolId = 0; poolId < poolCount; poolId++)
    new(Support::PlacementNew { &pools[poolId] }) JitAllocatorPool(granularity << poolId);

  return impl;
}

static inline void JitAllocatorImpl_destroy(JitAllocatorPrivateImpl* impl) noexcept {
  impl->~JitAllocatorPrivateImpl();
  ::free(impl);
}

static inline size_t JitAllocatorImpl_sizeToPoolId(const JitAllocatorPrivateImpl* impl, size_t size) noexcept {
  size_t poolId = impl->poolCount - 1;
  size_t granularity = size_t(impl->granularity) << poolId;

  while (poolId) {
    if (Support::alignUp(size, granularity) == size)
      break;
    poolId--;
    granularity >>= 1;
  }

  return poolId;
}

static inline size_t JitAllocatorImpl_bitVectorSizeToByteSize(uint32_t areaSize) noexcept {
  using Support::kBitWordSizeInBits;
  return ((areaSize + kBitWordSizeInBits - 1u) / kBitWordSizeInBits) * sizeof(Support::BitWord);
}

static inline size_t JitAllocatorImpl_calculateIdealBlockSize(JitAllocatorPrivateImpl* impl, JitAllocatorPool* pool, size_t allocationSize) noexcept {
  JitAllocatorBlock* last = pool->blocks.last();
  size_t blockSize = last ? last->blockSize : size_t(impl->blockSize);

  if (blockSize < kJitAllocatorMaxBlockSize)
    blockSize *= 2u;

  if (allocationSize > blockSize) {
    blockSize = Support::alignUp(allocationSize, impl->blockSize);
    if (ASMJIT_UNLIKELY(blockSize < allocationSize))
      return 0; // Overflown.
  }

  return blockSize;
}

ASMJIT_FAVOR_SPEED static void JitAllocatorImpl_fillPattern(void* mem, uint32_t pattern, size_t sizeInBytes) noexcept {
  size_t n = sizeInBytes / 4u;
  uint32_t* p = static_cast<uint32_t*>(mem);

  for (size_t i = 0; i < n; i++)
    p[i] = pattern;
}

// Allocate a new `JitAllocatorBlock` for the given `blockSize`.
//
// NOTE: The block doesn't have `kFlagEmpty` flag set, because the new block
// is only allocated when it's actually needed, so it would be cleared anyway.
static JitAllocatorBlock* JitAllocatorImpl_newBlock(JitAllocatorPrivateImpl* impl, JitAllocatorPool* pool, size_t blockSize) noexcept {
  using Support::BitWord;
  using Support::kBitWordSizeInBits;

  uint32_t areaSize = uint32_t((blockSize + pool->granularity - 1) >> pool->granularityLog2);
  uint32_t numBitWords = (areaSize + kBitWordSizeInBits - 1u) / kBitWordSizeInBits;

  JitAllocatorBlock* block = static_cast<JitAllocatorBlock*>(::malloc(sizeof(JitAllocatorBlock)));
  BitWord* bitWords = nullptr;
  VirtMem::DualMapping virtMem {};
  Error err = kErrorOutOfMemory;

  if (block != nullptr)
    bitWords = static_cast<BitWord*>(::malloc(size_t(numBitWords) * 2 * sizeof(BitWord)));

  uint32_t blockFlags = 0;
  if (bitWords != nullptr) {
    if (impl->options & JitAllocator::kOptionUseDualMapping) {
      err = VirtMem::allocDualMapping(&virtMem, blockSize, VirtMem::kAccessReadWrite | VirtMem::kAccessExecute);
      blockFlags |= JitAllocatorBlock::kFlagDualMapped;
    }
    else {
      err = VirtMem::alloc(&virtMem.ro, blockSize, VirtMem::kAccessReadWrite | VirtMem::kAccessExecute);
      virtMem.rw = virtMem.ro;
    }
  }

  // Out of memory.
  if (ASMJIT_UNLIKELY(!block || !bitWords || err != kErrorOk)) {
    if (bitWords) ::free(bitWords);
    if (block) ::free(block);
    return nullptr;
  }

  // Fill the memory if the secure mode is enabled.
  if (impl->options & JitAllocator::kOptionFillUnusedMemory)
    JitAllocatorImpl_fillPattern(virtMem.rw, impl->fillPattern, blockSize);

  ::memset(bitWords, 0, size_t(numBitWords) * 2 * sizeof(BitWord));
  return new(Support::PlacementNew { block }) JitAllocatorBlock(pool, virtMem, blockSize, blockFlags, bitWords, bitWords + numBitWords, areaSize);
}

static void JitAllocatorImpl_deleteBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  ASMJIT_UNUSED(impl);

  if (block->flags & JitAllocatorBlock::kFlagDualMapped)
    VirtMem::releaseDualMapping(&block->mapping, block->blockSize);
  else
    VirtMem::release(block->mapping.ro, block->blockSize);

  ::free(block->usedBitVector);
  ::free(block);
}

static void JitAllocatorImpl_insertBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  JitAllocatorPool* pool = block->pool;

  if (!pool->cursor)
    pool->cursor = block;

  // Add to RBTree and List.
  impl->tree.insert(block);
  pool->blocks.append(block);

  // Update statistics.
  pool->blockCount++;
  pool->totalAreaSize += block->areaSize;
  pool->totalOverheadBytes += sizeof(JitAllocatorBlock) + JitAllocatorImpl_bitVectorSizeToByteSize(block->areaSize) * 2u;
}

static void JitAllocatorImpl_removeBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  JitAllocatorPool* pool = block->pool;

  // Remove from RBTree and List.
  if (pool->cursor == block)
    pool->cursor = block->hasPrev() ? block->prev() : block->next();

  impl->tree.remove(block);
  pool->blocks.unlink(block);

  // Update statistics.
  pool->blockCount--;
  pool->totalAreaSize -= block->areaSize;
  pool->totalOverheadBytes -= sizeof(JitAllocatorBlock) + JitAllocatorImpl_bitVectorSizeToByteSize(block->areaSize) * 2u;
}

static void JitAllocatorImpl_wipeOutBlock(JitAllocatorPrivateImpl* impl, JitAllocatorBlock* block) noexcept {
  JitAllocatorPool* pool = block->pool;

  if (block->hasFlag(JitAllocatorBlock::kFlagEmpty))
    return;

  uint32_t areaSize = block->areaSize;
  uint32_t granularity = pool->granularity;
  size_t numBitWords = pool->bitWordCountFromAreaSize(areaSize);

  if (impl->options & JitAllocator::kOptionFillUnusedMemory) {
    BitFlipIterator<Support::BitWord> it(block->usedBitVector, numBitWords);

    while (it.hasNext()) {
      uint32_t start = uint32_t(it.nextAndFlip());
      uint32_t end = areaSize;

      if (it.hasNext())
        end = uint32_t(it.nextAndFlip());

      JitAllocatorImpl_fillPattern(block->rwPtr() + start * granularity, impl->fillPattern, (end - start) * granularity);
    }
  }

  ::memset(block->usedBitVector, 0, size_t(numBitWords) * sizeof(Support::BitWord));
  ::memset(block->stopBitVector, 0, size_t(numBitWords) * sizeof(Support::BitWord));

  block->areaUsed = 0;
  block->largestUnusedArea = areaSize;
  block->searchStart = 0;
  block->searchEnd = areaSize;
  block->addFlags(JitAllocatorBlock::kFlagEmpty);
  block->clearFlags(JitAllocatorBlock::kFlagDirty);
}

// ============================================================================
// [asmjit::JitAllocator - Construction / Destruction]
// ============================================================================

JitAllocator::JitAllocator(const CreateParams* params) noexcept {
  _impl = JitAllocatorImpl_new(params);
  if (ASMJIT_UNLIKELY(!_impl))
    _impl = const_cast<JitAllocator::Impl*>(&JitAllocatorImpl_none);
}

JitAllocator::~JitAllocator() noexcept {
  if (_impl == &JitAllocatorImpl_none)
    return;

  reset(Globals::kResetHard);
  JitAllocatorImpl_destroy(static_cast<JitAllocatorPrivateImpl*>(_impl));
}

// ============================================================================
// [asmjit::JitAllocator - Reset]
// ============================================================================

void JitAllocator::reset(uint32_t resetPolicy) noexcept {
  if (_impl == &JitAllocatorImpl_none)
    return;

  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  impl->tree.reset();
  size_t poolCount = impl->poolCount;

  for (size_t poolId = 0; poolId < poolCount; poolId++) {
    JitAllocatorPool& pool = impl->pools[poolId];
    JitAllocatorBlock* block = pool.blocks.first();

    JitAllocatorBlock* blockToKeep = nullptr;
    if (resetPolicy != Globals::kResetHard && !(impl->options & kOptionImmediateRelease)) {
      blockToKeep = block;
      block = block->next();
    }

    while (block) {
      JitAllocatorBlock* next = block->next();
      JitAllocatorImpl_deleteBlock(impl, block);
      block = next;
    }

    pool.reset();

    if (blockToKeep) {
      blockToKeep->_listNodes[0] = nullptr;
      blockToKeep->_listNodes[1] = nullptr;
      JitAllocatorImpl_wipeOutBlock(impl, blockToKeep);
      JitAllocatorImpl_insertBlock(impl, blockToKeep);
      pool.emptyBlockCount = 1;
    }
  }
}

// ============================================================================
// [asmjit::JitAllocator - Statistics]
// ============================================================================

JitAllocator::Statistics JitAllocator::statistics() const noexcept {
  Statistics statistics;
  statistics.reset();

  if (ASMJIT_LIKELY(_impl != &JitAllocatorImpl_none)) {
    JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
    ScopedLock locked(impl->lock);

    size_t poolCount = impl->poolCount;
    for (size_t poolId = 0; poolId < poolCount; poolId++) {
      const JitAllocatorPool& pool = impl->pools[poolId];
      statistics._blockCount   += size_t(pool.blockCount);
      statistics._reservedSize += size_t(pool.totalAreaSize) * pool.granularity;
      statistics._usedSize     += size_t(pool.totalAreaUsed) * pool.granularity;
      statistics._overheadSize += size_t(pool.totalOverheadBytes);
    }
  }

  return statistics;
}

// ============================================================================
// [asmjit::JitAllocator - Alloc / Release]
// ============================================================================

Error JitAllocator::alloc(void** roPtrOut, void** rwPtrOut, size_t size) noexcept {
  if (ASMJIT_UNLIKELY(_impl == &JitAllocatorImpl_none))
    return DebugUtils::errored(kErrorNotInitialized);

  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  constexpr uint32_t kNoIndex = std::numeric_limits<uint32_t>::max();

  *roPtrOut = nullptr;
  *rwPtrOut = nullptr;

  // Align to the minimum granularity by default.
  size = Support::alignUp<size_t>(size, impl->granularity);
  if (ASMJIT_UNLIKELY(size == 0))
    return DebugUtils::errored(kErrorInvalidArgument);

  if (ASMJIT_UNLIKELY(size > std::numeric_limits<uint32_t>::max() / 2))
    return DebugUtils::errored(kErrorTooLarge);

  ScopedLock locked(impl->lock);
  JitAllocatorPool* pool = &impl->pools[JitAllocatorImpl_sizeToPoolId(impl, size)];

  uint32_t areaIndex = kNoIndex;
  uint32_t areaSize = uint32_t(pool->areaSizeFromByteSize(size));

  // Try to find the requested memory area in existing blocks.
  JitAllocatorBlock* block = pool->blocks.first();
  if (block) {
    JitAllocatorBlock* initial = block;
    do {
      JitAllocatorBlock* next = block->hasNext() ? block->next() : pool->blocks.first();
      if (block->areaAvailable() >= areaSize) {
        if (block->hasFlag(JitAllocatorBlock::kFlagDirty) || block->largestUnusedArea >= areaSize) {
          uint32_t blockAreaSize = block->areaSize;
          uint32_t searchStart = block->searchStart;
          uint32_t searchEnd = block->searchEnd;

          BitFlipIterator<Support::BitWord> it(
            block->usedBitVector,
            pool->bitWordCountFromAreaSize(searchEnd),
            searchStart,
            Support::allOnes<Support::BitWord>());

          // If there is unused area available then there has to be at least one match.
          ASMJIT_ASSERT(it.hasNext());

          uint32_t bestArea = blockAreaSize;
          uint32_t largestArea = 0;
          uint32_t holeIndex = uint32_t(it.peekNext());
          uint32_t holeEnd = holeIndex;

          searchStart = holeIndex;
          do {
            holeIndex = uint32_t(it.nextAndFlip());
            if (holeIndex >= searchEnd) break;

            holeEnd = it.hasNext() ? Support::min(searchEnd, uint32_t(it.nextAndFlip())) : searchEnd;
            uint32_t holeSize = holeEnd - holeIndex;

            if (holeSize >= areaSize && bestArea >= holeSize) {
              largestArea = Support::max(largestArea, bestArea);
              bestArea = holeSize;
              areaIndex = holeIndex;
            }
            else {
              largestArea = Support::max(largestArea, holeSize);
            }
          } while (it.hasNext());
          searchEnd = holeEnd;

          // Because we have traversed the entire block, we can now mark the
          // largest unused area that can be used to cache the next traversal.
          block->searchStart = searchStart;
          block->searchEnd = searchEnd;
          block->largestUnusedArea = largestArea;
          block->clearFlags(JitAllocatorBlock::kFlagDirty);

          if (areaIndex != kNoIndex) {
            if (searchStart == areaIndex)
              block->searchStart += areaSize;
            break;
          }
        }
      }

      block = next;
    } while (block != initial);
  }

  // Allocate a new block if there is no region of a required width.
  if (areaIndex == kNoIndex) {
    size_t blockSize = JitAllocatorImpl_calculateIdealBlockSize(impl, pool, size);
    if (ASMJIT_UNLIKELY(!blockSize))
      return DebugUtils::errored(kErrorOutOfMemory);

    block = JitAllocatorImpl_newBlock(impl, pool, blockSize);

    if (ASMJIT_UNLIKELY(!block))
      return DebugUtils::errored(kErrorOutOfMemory);

    JitAllocatorImpl_insertBlock(impl, block);
    areaIndex = 0;
    block->searchStart = areaSize;
    block->largestUnusedArea = block->areaSize - areaSize;
  }

  // Update statistics.
  block->increaseUsedArea(areaSize);

  // Handle special cases.
  if (block->hasFlag(JitAllocatorBlock::kFlagEmpty)) {
    pool->emptyBlockCount--;
    block->clearFlags(JitAllocatorBlock::kFlagEmpty);
  }

  if (block->areaAvailable() == 0) {
    // The whole block is filled.
    block->searchStart = block->areaSize;
    block->searchEnd = 0;
    block->largestUnusedArea = 0;
    block->clearFlags(JitAllocatorBlock::kFlagDirty);
  }

  // Mark the newly allocated space as occupied and also the sentinel.
  Support::bitVectorFill(block->usedBitVector, areaIndex, areaSize);
  Support::bitVectorSetBit(block->stopBitVector, areaIndex + areaSize - 1, true);

  // Return a pointer to the allocated memory.
  size_t offset = pool->byteSizeFromAreaSize(areaIndex);
  ASMJIT_ASSERT(offset <= block->blockSize - size);

  *roPtrOut = block->roPtr() + offset;
  *rwPtrOut = block->rwPtr() + offset;
  return kErrorOk;
}

Error JitAllocator::release(void* ro) noexcept {
  if (ASMJIT_UNLIKELY(_impl == &JitAllocatorImpl_none))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(!ro))
    return DebugUtils::errored(kErrorInvalidArgument);

  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  ScopedLock locked(impl->lock);

  JitAllocatorBlock* block = impl->tree.get(static_cast<uint8_t*>(ro));
  if (ASMJIT_UNLIKELY(!block))
    return DebugUtils::errored(kErrorInvalidState);

  // Offset relative to the start of the block.
  JitAllocatorPool* pool = block->pool;
  size_t offset = (size_t)((uint8_t*)ro - block->roPtr());

  // The first bit representing the allocated area and its size.
  uint32_t areaIndex = uint32_t(offset >> pool->granularityLog2);
  uint32_t areaLast = uint32_t(Support::bitVectorIndexOf(block->stopBitVector, areaIndex, true));
  uint32_t areaSize = areaLast - areaIndex + 1;

  // Update the search region and statistics.
  block->searchStart = Support::min(block->searchStart, areaIndex);
  block->searchEnd = Support::max(block->searchEnd, areaLast + 1);
  block->addFlags(JitAllocatorBlock::kFlagDirty);
  block->decreaseUsedArea(areaSize);

  // Clear all occupied bits and also the sentinel.
  Support::bitVectorClear(block->usedBitVector, areaIndex, areaSize);
  Support::bitVectorSetBit(block->stopBitVector, areaLast, false);

  // Fill the released memory if the secure mode is enabled.
  if (impl->options & kOptionFillUnusedMemory)
    JitAllocatorImpl_fillPattern(block->rwPtr() + areaIndex * pool->granularity, impl->fillPattern, areaSize * pool->granularity);

  // Release the whole block if it became empty.
  if (block->areaUsed == 0) {
    if (pool->emptyBlockCount || (impl->options & kOptionImmediateRelease)) {
      JitAllocatorImpl_removeBlock(impl, block);
      JitAllocatorImpl_deleteBlock(impl, block);
    }
    else {
      pool->emptyBlockCount++;
      block->largestUnusedArea = areaSize;
      block->searchStart = 0;
      block->searchEnd = areaSize;
      block->addFlags(JitAllocatorBlock::kFlagEmpty);
      block->clearFlags(JitAllocatorBlock::kFlagDirty);
    }
  }

  return kErrorOk;
}

Error JitAllocator::shrink(void* ro, size_t newSize) noexcept {
  if (ASMJIT_UNLIKELY(_impl == &JitAllocatorImpl_none))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(!ro))
    return DebugUtils::errored(kErrorInvalidArgument);

  if (ASMJIT_UNLIKELY(newSize == 0))
    return release(ro);

  JitAllocatorPrivateImpl* impl = static_cast<JitAllocatorPrivateImpl*>(_impl);
  ScopedLock locked(impl->lock);
  JitAllocatorBlock* block = impl->tree.get(static_cast<uint8_t*>(ro));

  if (ASMJIT_UNLIKELY(!block))
    return DebugUtils::errored(kErrorInvalidArgument);

  // Offset relative to the start of the block.
  JitAllocatorPool* pool = block->pool;
  size_t offset = (size_t)((uint8_t*)ro - block->roPtr());

  // The first bit representing the allocated area and its size.
  uint32_t areaIndex = uint32_t(offset >> pool->granularityLog2);
  uint32_t areaOldSize = uint32_t(Support::bitVectorIndexOf(block->stopBitVector, areaIndex, true)) + 1 - areaIndex;
  uint32_t areaNewSize = pool->areaSizeFromByteSize(newSize);

  if (ASMJIT_UNLIKELY(areaNewSize > areaOldSize))
    return DebugUtils::errored(kErrorInvalidState);

  uint32_t areaDiff = areaOldSize - areaNewSize;
  if (!areaDiff)
    return kErrorOk;

  // Update the search region and statistics.
  block->searchStart = Support::min(block->searchStart, areaIndex + areaNewSize);
  block->searchEnd = Support::max(block->searchEnd, areaIndex + areaOldSize);
  block->addFlags(JitAllocatorBlock::kFlagDirty);
  block->decreaseUsedArea(areaDiff);

  // Unmark the released space and move the sentinel.
  Support::bitVectorClear(block->usedBitVector, areaIndex + areaNewSize, areaDiff);
  Support::bitVectorSetBit(block->stopBitVector, areaIndex + areaOldSize - 1, false);
  Support::bitVectorSetBit(block->stopBitVector, areaIndex + areaNewSize - 1, true);

  // Fill released memory if the secure mode is enabled.
  if (impl->options & kOptionFillUnusedMemory)
    JitAllocatorImpl_fillPattern(
      block->rwPtr() + (areaIndex + areaOldSize) * pool->granularity,
      fillPattern(),
      areaDiff * pool->granularity);

  return kErrorOk;
}

// ============================================================================
// [asmjit::JitAllocator - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
// A pseudo random number generator based on a paper by Sebastiano Vigna:
//   http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
class Random {
public:
  // Constants suggested as `23/18/5`.
  enum Steps : uint32_t {
    kStep1_SHL = 23,
    kStep2_SHR = 18,
    kStep3_SHR = 5
  };

  inline explicit Random(uint64_t seed = 0) noexcept { reset(seed); }
  inline Random(const Random& other) noexcept = default;

  inline void reset(uint64_t seed = 0) noexcept {
    // The number is arbitrary, it means nothing.
    constexpr uint64_t kZeroSeed = 0x1F0A2BE71D163FA0u;

    // Generate the state data by using splitmix64.
    for (uint32_t i = 0; i < 2; i++) {
      seed += 0x9E3779B97F4A7C15u;
      uint64_t x = seed;
      x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9u;
      x = (x ^ (x >> 27)) * 0x94D049BB133111EBu;
      x = (x ^ (x >> 31));
      _state[i] = x != 0 ? x : kZeroSeed;
    }
  }

  inline uint32_t nextUInt32() noexcept {
    return uint32_t(nextUInt64() >> 32);
  }

  inline uint64_t nextUInt64() noexcept {
    uint64_t x = _state[0];
    uint64_t y = _state[1];

    x ^= x << kStep1_SHL;
    y ^= y >> kStep3_SHR;
    x ^= x >> kStep2_SHR;
    x ^= y;

    _state[0] = y;
    _state[1] = x;
    return x + y;
  }

  uint64_t _state[2];
};

// Helper class to verify that JitAllocator doesn't return addresses that overlap.
class JitAllocatorWrapper {
public:
  explicit inline JitAllocatorWrapper(const JitAllocator::CreateParams* params) noexcept
    : _zone(1024 * 1024),
      _heap(&_zone),
      _allocator(params) {}

  // Address to a memory region of a given size.
  class Range {
  public:
    inline Range(uint8_t* addr, size_t size) noexcept
      : addr(addr),
        size(size) {}
    uint8_t* addr;
    size_t size;
  };

  // Based on JitAllocator::Block, serves our purpose well...
  class Record : public ZoneTreeNodeT<Record>,
                 public Range {
  public:
    inline Record(uint8_t* addr, size_t size)
      : ZoneTreeNodeT<Record>(),
        Range(addr, size) {}

    inline bool operator<(const Record& other) const noexcept { return addr < other.addr; }
    inline bool operator>(const Record& other) const noexcept { return addr > other.addr; }

    inline bool operator<(const uint8_t* key) const noexcept { return addr + size <= key; }
    inline bool operator>(const uint8_t* key) const noexcept { return addr > key; }
  };

  void _insert(void* p_, size_t size) noexcept {
    uint8_t* p = static_cast<uint8_t*>(p_);
    uint8_t* pEnd = p + size - 1;

    Record* record;

    record = _records.get(p);
    if (record)
      EXPECT(record == nullptr,
             "Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, p, p + size);

    record = _records.get(pEnd);
    if (record)
      EXPECT(record == nullptr,
             "Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, p, p + size);

    record = _heap.newT<Record>(p, size);
    EXPECT(record != nullptr,
           "Out of memory, cannot allocate 'Record'");

    _records.insert(record);
  }

  void _remove(void* p) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p));
    EXPECT(record != nullptr,
           "Address [%p] doesn't exist\n", p);

    _records.remove(record);
    _heap.release(record, sizeof(Record));
  }

  void* alloc(size_t size) noexcept {
    void* roPtr;
    void* rwPtr;

    Error err = _allocator.alloc(&roPtr, &rwPtr, size);
    EXPECT(err == kErrorOk,
           "JitAllocator failed to allocate '%u' bytes\n", unsigned(size));

    _insert(roPtr, size);
    return roPtr;
  }

  void release(void* p) noexcept {
    _remove(p);
    EXPECT(_allocator.release(p) == kErrorOk,
           "JitAllocator failed to release '%p'\n", p);
  }

  Zone _zone;
  ZoneAllocator _heap;
  ZoneTree<Record> _records;
  JitAllocator _allocator;
};

static void JitAllocatorTest_shuffle(void** ptrArray, size_t count, Random& prng) noexcept {
  for (size_t i = 0; i < count; ++i)
    std::swap(ptrArray[i], ptrArray[size_t(prng.nextUInt32() % count)]);
}

static void JitAllocatorTest_usage(JitAllocator& allocator) noexcept {
  JitAllocator::Statistics stats = allocator.statistics();
  INFO("  Block Count       : %9llu [Blocks]"        , (unsigned long long)(stats.blockCount()));
  INFO("  Reserved (VirtMem): %9llu [Bytes]"         , (unsigned long long)(stats.reservedSize()));
  INFO("  Used     (VirtMem): %9llu [Bytes] (%.1f%%)", (unsigned long long)(stats.usedSize()), stats.usedSizeAsPercent());
  INFO("  Overhead (HeapMem): %9llu [Bytes] (%.1f%%)", (unsigned long long)(stats.overheadSize()), stats.overheadSizeAsPercent());
}

UNIT(asmjit_core_jit_allocator) {
  size_t kCount = BrokenAPI::hasArg("--quick") ? 1000 : 100000;

  struct TestParams {
    const char* name;
    uint32_t options;
    uint32_t blockSize;
    uint32_t granularity;
  };

  #define OPT(OPTION) JitAllocator::OPTION
  static TestParams testParams[] = {
    { "Default", 0, 0, 0 },
    { "16MB blocks", 0, 16 * 1024 * 1024, 0 },
    { "256B granularity", 0, 0, 256 },
    { "kOptionUseDualMapping", OPT(kOptionUseDualMapping), 0, 0 },
    { "kOptionUseMultiplePools", OPT(kOptionUseMultiplePools), 0, 0 },
    { "kOptionFillUnusedMemory", OPT(kOptionFillUnusedMemory), 0, 0 },
    { "kOptionImmediateRelease", OPT(kOptionImmediateRelease), 0, 0 },
    { "kOptionUseDualMapping | kOptionFillUnusedMemory", OPT(kOptionUseDualMapping) | OPT(kOptionFillUnusedMemory), 0, 0 }
  };
  #undef OPT

  INFO("BitFlipIterator<uint32_t>");
  {
    static const uint32_t bits[] = { 0x80000000u, 0x80000000u, 0x00000000u, 0x80000000u };
    BitFlipIterator<uint32_t> it(bits, ASMJIT_ARRAY_SIZE(bits));

    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 31);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 32);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 63);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 64);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 127);
    EXPECT(!it.hasNext());
  }

  INFO("BitFlipIterator<uint64_t>");
  {
    static const uint64_t bits[] = { 0xFFFFFFFFFFFFFFFFu, 0xFFFFFFFFFFFFFFFF, 0, 0 };
    BitFlipIterator<uint64_t> it(bits, ASMJIT_ARRAY_SIZE(bits));

    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 0);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 128);
    EXPECT(!it.hasNext());
  }

  for (uint32_t testId = 0; testId < ASMJIT_ARRAY_SIZE(testParams); testId++) {
    INFO("Testing JitAllocator: %s", testParams[testId].name);

    JitAllocator::CreateParams params {};
    params.options = testParams[testId].options;
    params.blockSize = testParams[testId].blockSize;
    params.granularity = testParams[testId].granularity;

    JitAllocatorWrapper wrapper(&params);
    Random prng(100);

    size_t i;

    INFO("  Memory alloc/release test - %d allocations", kCount);

    void** ptrArray = (void**)::malloc(sizeof(void*) * size_t(kCount));
    EXPECT(ptrArray != nullptr,
          "Couldn't allocate '%u' bytes for pointer-array", unsigned(sizeof(void*) * size_t(kCount)));

    INFO("  Allocating virtual memory...");
    for (i = 0; i < kCount; i++)
      ptrArray[i] = wrapper.alloc((prng.nextUInt32() % 1024) + 8);
    JitAllocatorTest_usage(wrapper._allocator);

    INFO("  Releasing virtual memory...");
    for (i = 0; i < kCount; i++)
      wrapper.release(ptrArray[i]);
    JitAllocatorTest_usage(wrapper._allocator);

    INFO("  Allocating virtual memory...", kCount);
    for (i = 0; i < kCount; i++)
      ptrArray[i] = wrapper.alloc((prng.nextUInt32() % 1024) + 8);
    JitAllocatorTest_usage(wrapper._allocator);

    INFO("  Shuffling...");
    JitAllocatorTest_shuffle(ptrArray, unsigned(kCount), prng);

    INFO("  Releasing 50%% blocks...");
    for (i = 0; i < kCount / 2; i++)
      wrapper.release(ptrArray[i]);
    JitAllocatorTest_usage(wrapper._allocator);

    INFO("  Allocating 50%% blocks again...");
    for (i = 0; i < kCount / 2; i++)
      ptrArray[i] = wrapper.alloc((prng.nextUInt32() % 1024) + 8);
    JitAllocatorTest_usage(wrapper._allocator);

    INFO("  Releasing virtual memory...");
    for (i = 0; i < kCount; i++)
      wrapper.release(ptrArray[i]);
    JitAllocatorTest_usage(wrapper._allocator);

    ::free(ptrArray);
  }
}
#endif

ASMJIT_END_NAMESPACE

#endif
