// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_JITALLOCATOR_H
#define _ASMJIT_CORE_JITALLOCATOR_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_JIT

// [Dependencies]
#include "../core/globals.h"
#include "../core/osutils.h"
#include "../core/zonelist.h"
#include "../core/zonerbtree.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
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

  enum Flags : uint32_t {
    kFlagSecureMode = 0x80000000U        //!< Always mark unused memory by fill-pattern.
  };

  //! Minimum granularity (and the default granularity for pool #0).
  static constexpr uint32_t kMinimumGranularity = 64;

  //! Number of pools that contain blocks.
  //!
  //! Each pool increases granularity twice to make memory management more
  //! efficient. Ideal number of pools appears to be 3 to 4 as it distributes
  //! small and large functions properly.
  static constexpr uint32_t kPoolCount = 3;

  //! Minimum block size.
  static constexpr uint32_t kMinBlockSize = 65536;   // 64kB.
  //! Maximum block size.
  static constexpr uint32_t kMaxBlockSize = 1048576; // 1MB.

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

    inline uint32_t getGranularity() const noexcept {
      return _granularity;
    }

    inline void setGranularity(uint32_t granularity) noexcept {
      ASMJIT_ASSERT(granularity < 65536U);
      _granularity = uint16_t(granularity);
      _granularityLog2 = uint8_t(IntUtils::ctz(granularity));
    }

    inline size_t byteSizeFromAreaSize(uint32_t areaSize) const noexcept { return size_t(areaSize) * _granularity; }
    inline uint32_t areaSizeFromByteSize(size_t size) const noexcept { return uint32_t((size + _granularity - 1) >> _granularityLog2); }

    inline size_t bitWordCountFromAreaSize(uint32_t areaSize) const noexcept {
      return IntUtils::alignUp<size_t>(areaSize, Globals::kBitWordSize) / Globals::kBitWordSize;
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

  class Block : public ZoneRBNodeT<Block>,
                public ZoneListNode<Block> {
  public:
    ASMJIT_NONCOPYABLE(Block)

    enum Flags {
      kFlagDirty           = 0x00000001U //!< Block is dirty (some information needs update).
    };

    inline Block(Pool* pool,
                 uint8_t* virtMem,
                 size_t blockSize,
                 Globals::BitWord* usedBitVector,
                 Globals::BitWord* stopBitVector,
                 uint32_t areaSize) noexcept
      : ZoneRBNodeT(),
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

    inline Pool* getPool() const noexcept { return _pool; }

    inline uint8_t* getVirtMem() const noexcept { return _virtMem; }
    inline size_t getBlockSize() const noexcept { return _blockSize; }

    inline uint32_t getFlags() const noexcept { return _flags; }
    inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
    inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
    inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

    inline uint32_t getAreaSize() const noexcept { return _areaSize; }
    inline uint32_t getAreaUsed() const noexcept { return _areaUsed; }

    inline uint32_t getAreaAvailable() const noexcept { return getAreaSize() - getAreaUsed(); }
    inline uint32_t getLargestUnusedArea() const noexcept { return _largestUnusedArea; }

    inline void increaseUsedArea(uint32_t areaSize) noexcept {
      _areaUsed += areaSize;
      _pool->_totalAreaUsed += areaSize;
    }

    inline void decreaseUsedArea(uint32_t areaSize) noexcept {
      _areaUsed -= areaSize;
      _pool->_totalAreaUsed -= areaSize;
    }

    // RBTree default CMP uses '<' and '>' operators.
    inline bool operator<(const Block& other) const noexcept { return getVirtMem() < other.getVirtMem(); }
    inline bool operator>(const Block& other) const noexcept { return getVirtMem() > other.getVirtMem(); }

    // Special implementation for querying blocks by `key`, which must be in `[virtMem, virtMem + blockSize)` interval.
    inline bool operator<(const uint8_t* key) const noexcept { return getVirtMem() + getBlockSize() <= key; }
    inline bool operator>(const uint8_t* key) const noexcept { return getVirtMem() > key; }

    Pool* _pool;                         //!< Link to the pool that owns this block.
    uint8_t* _virtMem;                   //!< Virtual memory address.
    size_t _blockSize;                   //!< Virtual memory size (block size) [bytes].

    uint32_t _flags;                     //!< Block flags.
    uint32_t _areaSize;                  //!< Size of the whole block area (bit-vector size).
    uint32_t _areaUsed;                  //!< Used area (number of bits in bit-vector used).
    uint32_t _largestUnusedArea;         //!< The largest unused continuous area in the bit-vector (or `_areaSize` to initiate rescan).
    uint32_t _searchStart;               //!< Start of a search range (for unused bits).
    uint32_t _searchEnd;                 //!< End of a search range (for unused bits).

    Globals::BitWord* _usedBitVector;    //!< Used bit-vector (0 = unused    , 1 = used).
    Globals::BitWord* _stopBitVector;    //!< Stop bit-vector (0 = don't care, 1 = stop).
  };

  struct Statistics {
    inline void reset() noexcept {
      _blockCount = 0;
      _usedSize = 0;
      _reservedSize = 0;
      _overheadSize = 0;
    }

    //! Get count of blocks managed by `JitAllocator`.
    inline size_t getBlockCount() const noexcept { return _blockCount; }

    //! Get how many bytes are currently used.
    inline size_t getUsedSize() const noexcept { return _usedSize; }
    //! Number of bytes unused by the allocator at the moment.
    inline size_t getUnusedSize() const noexcept { return _reservedSize - _usedSize; }
    //! Total number of bytes bytes reserved by the allocator (sum of sizes of all blocks).
    inline size_t getReservedSize() const noexcept { return _reservedSize; }
    //! Number of bytes the allocator needs to manage the allocated memory.
    inline size_t getOverheadSize() const noexcept { return _overheadSize; }

    inline double getUsedAsPercent() const noexcept {
      return (double(getUsedSize()) / (double(getReservedSize()) + 1e-16)) * 100.0;
    }

    inline double getUnusedAsPercent() const noexcept {
      return (double(getUnusedSize()) / (double(getReservedSize()) + 1e-16)) * 100.0;
    }

    inline double getOverheadAsPercent() const noexcept {
      return (double(getOverheadSize()) / (double(getReservedSize()) + 1e-16)) * 100.0;
    }

    size_t _blockCount;                  //!< Number of blocks `JitAllocator` maintaints.
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

  //! Get allocation flags, see \ref Flags.
  inline uint32_t getFlags() const noexcept { return _flags; }
  //! Get whether the allocator has the given `flag` set.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  //! Get a page size (smallest possible allocable chunk of virtual memory).
  inline uint32_t getPageSize() const noexcept { return _pageSize; }
  //! Get a minimum block size (a minimum size of block that the allocator would allocate).
  inline uint32_t getBlockSize() const noexcept { return _blockSize; }

  //! Get a pattern that is used to fill unused memory if `kFlagSecureMode` is enabled.
  inline uint32_t getFillPattern() const noexcept { return _fillPattern; }

  //! Get allocation statistics.
  ASMJIT_API Statistics getStatistics() const noexcept;

  // --------------------------------------------------------------------------
  // [Alloc / Release]
  // --------------------------------------------------------------------------

  //! Allocate `size` bytes of virtual memory.
  ASMJIT_API void* alloc(size_t size) noexcept;
  //! Release a memory returned by `alloc()`.
  ASMJIT_API Error release(void* p) noexcept;
  //! Free extra memory allocated with `p`.
  ASMJIT_API Error shrink(void* p, size_t used) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _flags;                       //!< Allocator flags, see \ref Flags.
  uint32_t _pageSize;                    //!< System page size (also a minimum block size).
  uint32_t _blockSize;                   //!< Default block size.
  uint32_t _fillPattern;                 //!< A pattern that is used to fill unused memory if secure mode is enabled.

  mutable Lock _lock;                    //!< Lock for thread safety.
  ZoneRBTree<Block> _tree;               //!< Blocks from all pools in RBTree.
  Pool _pools[kPoolCount];               //!< Allocator pools.
};

//! \}

ASMJIT_END_NAMESPACE

#endif
#endif
