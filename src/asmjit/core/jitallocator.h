// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_JITALLOCATOR_H
#define _ASMJIT_CORE_JITALLOCATOR_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_JIT

// [Dependencies]
#include "../core/globals.h"
#include "../core/osutils.h"
#include "../core/zonelist.h"
#include "../core/zonetree.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_jit
//! \{

// ============================================================================
// [asmjit::JitAllocator]
// ============================================================================

//! A simple implementation of memory manager that uses `JitUtils::virtualAlloc()`
//! and `JitUtils::virtualRelease()` to manage executable memory.
//!
//! Implementation notes:
//!
//! - Granularity of allocated blocks is different than granularity for a typical
//!   C malloc. There are several memory pools having a different granularity to
//!   make as little waste as possible.
//!
//! - The allocator doesn't store any information in executable memory, instead,
//!   the implementation uses two bit-vectors to manage allocated memory of each
//!   block. The first bit-vector called 'used' is used to track used memory
//!   (where each bit represents memory size defined by granularity) and the second
//!   bit vector called 'stop' is used as a sentinel to mark where the allocated
//!   area ends.
//!
//! - Internally, the allocator also uses RB tree to keep track of all blocks
//!   across all polls. Each inserted block is added to the tree so it can be
//!   queried fast during `release()` and `shrink()`.
class JitAllocator {
public:
  ASMJIT_NONCOPYABLE(JitAllocator)

  enum Limits : uint32_t {
    //! Number of pools that contain blocks.
    //!
    //! Each pool increases granularity twice to make memory management more
    //! efficient. Ideal number of pools appears to be 3 to 4 as it distributes
    //! small and large functions properly.
    kPoolCount = 3,

    //! Minimum granularity (and the default granularity for pool #0).
    kMinGranularity = 64,

    //! Minimum block size (64kB).
    kMinBlockSize = 1024 * 64,

    //! Maximum block size (1MB).
    kMaxBlockSize = 1024 * 1024
  };

  enum Flags : uint32_t {
    //! Always mark unused memory by fill-pattern.
    kFlagSecureMode = 0x80000000u
  };

  class Block;

  class Pool {
  public:
    ASMJIT_NONCOPYABLE(Pool)

    inline Pool() noexcept
      : _blocks(),
        _cursor(nullptr),
        _blockCount(0),
        _granularity(0),
        _granularityLog2(0),
        _reserved(0),
        _totalAreaSize(0),
        _totalAreaUsed(0),
        _totalOverheadBytes(0) {}

    inline void reset() noexcept {
      _blocks.reset();
      _cursor = nullptr;
      _blockCount = 0;
      _totalAreaSize = 0;
      _totalAreaUsed = 0;
      _totalOverheadBytes = 0;
    }

    inline uint32_t granularity() const noexcept { return _granularity; }
    inline void setGranularity(uint32_t granularity) noexcept {
      ASMJIT_ASSERT(granularity < 65536u);
      _granularity = uint16_t(granularity);
      _granularityLog2 = uint8_t(Support::ctz(granularity));
    }

    inline size_t byteSizeFromAreaSize(uint32_t areaSize) const noexcept { return size_t(areaSize) * _granularity; }
    inline uint32_t areaSizeFromByteSize(size_t size) const noexcept { return uint32_t((size + _granularity - 1) >> _granularityLog2); }

    inline size_t bitWordCountFromAreaSize(uint32_t areaSize) const noexcept {
      using Support::kBitWordSizeInBits;
      return Support::alignUp<size_t>(areaSize, kBitWordSizeInBits) / kBitWordSizeInBits;
    }

    ZoneList<Block> _blocks;             //!< Double linked list of blocks.
    Block* _cursor;                      //!< Where to start looking first.

    uint32_t _blockCount;                //!< Count of blocks.
    uint16_t _granularity;               //!< Allocation granularity.
    uint8_t _granularityLog2;            //!< Log2(_granularity).
    uint8_t _reserved;

    size_t _totalAreaSize;               //!< Number of bits reserved across all blocks.
    size_t _totalAreaUsed;               //!< Number of bits used across all blocks.
    size_t _totalOverheadBytes;          //!< Overhead of all blocks (in bytes).
  };

  class Block : public ZoneTreeNodeT<Block>,
                public ZoneListNode<Block> {
  public:
    ASMJIT_NONCOPYABLE(Block)

    enum Flags : uint32_t {
      //! Block is dirty (some members needs update).
      kFlagDirty = 0x80000000u
    };

    inline Block(Pool* pool,
                 uint8_t* virtMem,
                 size_t blockSize,
                 Support::BitWord* usedBitVector,
                 Support::BitWord* stopBitVector,
                 uint32_t areaSize) noexcept
      : ZoneTreeNodeT(),
        _pool(pool),
        _virtMem(virtMem),
        _blockSize(blockSize),
        _flags(0),
        _areaSize(areaSize),
        _areaUsed(0),
        _largestUnusedArea(areaSize),
        _searchStart(0),
        _searchEnd(areaSize),
        _usedBitVector(usedBitVector),
        _stopBitVector(stopBitVector) {}

    inline Pool* pool() const noexcept { return _pool; }

    inline uint8_t* virtMem() const noexcept { return _virtMem; }
    inline size_t blockSize() const noexcept { return _blockSize; }

    inline uint32_t flags() const noexcept { return _flags; }
    inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
    inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
    inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

    inline uint32_t areaSize() const noexcept { return _areaSize; }
    inline uint32_t areaUsed() const noexcept { return _areaUsed; }

    inline uint32_t areaAvailable() const noexcept { return areaSize() - areaUsed(); }
    inline uint32_t largestUnusedArea() const noexcept { return _largestUnusedArea; }

    inline void increaseUsedArea(uint32_t areaSize) noexcept {
      _areaUsed += areaSize;
      _pool->_totalAreaUsed += areaSize;
    }

    inline void decreaseUsedArea(uint32_t areaSize) noexcept {
      _areaUsed -= areaSize;
      _pool->_totalAreaUsed -= areaSize;
    }

    // RBTree default CMP uses '<' and '>' operators.
    inline bool operator<(const Block& other) const noexcept { return virtMem() < other.virtMem(); }
    inline bool operator>(const Block& other) const noexcept { return virtMem() > other.virtMem(); }

    // Special implementation for querying blocks by `key`, which must be in `[virtMem, virtMem + blockSize)` range.
    inline bool operator<(const uint8_t* key) const noexcept { return virtMem() + blockSize() <= key; }
    inline bool operator>(const uint8_t* key) const noexcept { return virtMem() > key; }

    Pool* _pool;                         //!< Link to the pool that owns this block.
    uint8_t* _virtMem;                   //!< Virtual memory address.
    size_t _blockSize;                   //!< Virtual memory size (block size) [bytes].

    uint32_t _flags;                     //!< Block flags.
    uint32_t _areaSize;                  //!< Size of the whole block area (bit-vector size).
    uint32_t _areaUsed;                  //!< Used area (number of bits in bit-vector used).
    uint32_t _largestUnusedArea;         //!< The largest unused continuous area in the bit-vector (or `_areaSize` to initiate rescan).
    uint32_t _searchStart;               //!< Start of a search range (for unused bits).
    uint32_t _searchEnd;                 //!< End of a search range (for unused bits).

    Support::BitWord* _usedBitVector;    //!< Used bit-vector (0 = unused    , 1 = used).
    Support::BitWord* _stopBitVector;    //!< Stop bit-vector (0 = don't care, 1 = stop).
  };

  struct Statistics {
    inline void reset() noexcept {
      _blockCount = 0;
      _usedSize = 0;
      _reservedSize = 0;
      _overheadSize = 0;
    }

    //! Get count of blocks managed by `JitAllocator`.
    inline size_t blockCount() const noexcept { return _blockCount; }

    //! Get how many bytes are currently used.
    inline size_t usedSize() const noexcept { return _usedSize; }
    //! Number of bytes unused by the allocator at the moment.
    inline size_t unusedSize() const noexcept { return _reservedSize - _usedSize; }
    //! Total number of bytes bytes reserved by the allocator (sum of sizes of all blocks).
    inline size_t reservedSize() const noexcept { return _reservedSize; }
    //! Number of bytes the allocator needs to manage the allocated memory.
    inline size_t overheadSize() const noexcept { return _overheadSize; }

    inline double usedSizeAsPercent() const noexcept {
      return (double(usedSize()) / (double(reservedSize()) + 1e-16)) * 100.0;
    }

    inline double unusedSizeAsPercent() const noexcept {
      return (double(unusedSize()) / (double(reservedSize()) + 1e-16)) * 100.0;
    }

    inline double overheadSizeAsPercent() const noexcept {
      return (double(overheadSize()) / (double(reservedSize()) + 1e-16)) * 100.0;
    }

    size_t _blockCount;                  //!< Number of blocks `JitAllocator` maintains.
    size_t _usedSize;                    //!< How many bytes are currently used / allocated.
    size_t _reservedSize;                //!< How many bytes are currently reserved by the allocator.
    size_t _overheadSize;                //!< Allocation overhead (in bytes) required to maintain all blocks.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `JitAllocator` instance.
  ASMJIT_API JitAllocator() noexcept;
  //! Destroy the `JitAllocator` instance and release all blocks held.
  ASMJIT_API ~JitAllocator() noexcept;

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Free all allocated memory - makes all pointers returned by `alloc()` invalid.
  ASMJIT_API void reset() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get allocation flags, see `Flags`.
  inline uint32_t flags() const noexcept { return _flags; }
  //! Get whether the allocator has the given `flag` set.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  //! Get a page size (smallest possible allocable chunk of virtual memory).
  inline uint32_t pageSize() const noexcept { return _pageSize; }
  //! Get a minimum block size (a minimum size of block that the allocator would allocate).
  inline uint32_t blockSize() const noexcept { return _blockSize; }

  //! Get a pattern that is used to fill unused memory if `kFlagSecureMode` is enabled.
  inline uint32_t fillPattern() const noexcept { return _fillPattern; }

  //! Get allocation statistics.
  ASMJIT_API Statistics statistics() const noexcept;

  // --------------------------------------------------------------------------
  // [Alloc / Release]
  // --------------------------------------------------------------------------

  //! Allocate `size` bytes of virtual memory.
  ASMJIT_API void* alloc(size_t size) noexcept;
  //! Release a memory returned by `alloc()`.
  ASMJIT_API Error release(void* p) noexcept;
  //! Free extra memory allocated with `p` by restricting it to `newSize` size.
  ASMJIT_API Error shrink(void* p, size_t newSize) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _flags;                       //!< Allocator flags, see `Flags`.
  uint32_t _pageSize;                    //!< System page size (also a minimum block size).
  uint32_t _blockSize;                   //!< Default block size.
  uint32_t _fillPattern;                 //!< A pattern that is used to fill unused memory if secure mode is enabled.

  mutable Lock _lock;                    //!< Lock for thread safety.
  ZoneTree<Block> _tree;                 //!< Blocks from all pools in RBTree.
  Pool _pools[kPoolCount];               //!< Allocator pools.
};

//! \}

ASMJIT_END_NAMESPACE

#endif
#endif
