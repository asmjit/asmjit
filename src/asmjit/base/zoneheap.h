// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ZONEHEAP_H
#define _ASMJIT_BASE_ZONEHEAP_H

// [Dependencies]
#include "../base/utils.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ZoneHeap]
// ============================================================================

//! Zone-based memory allocator that uses an existing \ref Zone and provides
//! a `release()` functionality on top of it. It uses \ref Zone only for chunks
//! that can be pooled, and uses libc `malloc()` for chunks that are large.
//!
//! The advantage of ZoneHeap is that it can allocate small chunks of memory
//! really fast, and these chunks, when released, will be reused by consecutive
//! calls to `alloc()`. Also, since ZoneHeap uses \ref Zone, you can turn any
//! \ref Zone into a \ref ZoneHeap, and use it in your \ref CBPass when necessary.
//!
//! ZoneHeap is used by AsmJit containers to make containers having only
//! few elements fast (and lightweight) and to allow them to grow and use
//! dynamic blocks when require more storage.
class ZoneHeap {
  ASMJIT_NONCOPYABLE(ZoneHeap)

  enum {
    // In short, we pool chunks of these sizes:
    //   [32, 64, 96, 128, 192, 256, 320, 384, 448, 512]

    //! How many bytes per a low granularity pool (has to be at least 16).
    kLoGranularity = 32,
    //! Number of slots of a low granularity pool.
    kLoCount = 4,
    //! Maximum size of a block that can be allocated in a low granularity pool.
    kLoMaxSize = kLoGranularity * kLoCount,

    //! How many bytes per a high granularity pool.
    kHiGranularity = 64,
    //! Number of slots of a high granularity pool.
    kHiCount = 6,
    //! Maximum size of a block that can be allocated in a high granularity pool.
    kHiMaxSize = kLoMaxSize + kHiGranularity * kHiCount,

    //! Alignment of every pointer returned by `alloc()`.
    kBlockAlignment = kLoGranularity
  };

  //! Single-linked list used to store unused chunks.
  struct Slot {
    //! Link to a next slot in a single-linked list.
    Slot* next;
  };

  //! A block of memory that has been allocated dynamically and is not part of
  //! block-list used by the allocator. This is used to keep track of all these
  //! blocks so they can be freed by `reset()` if not freed explicitly.
  struct DynamicBlock {
    DynamicBlock* prev;
    DynamicBlock* next;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `ZoneHeap`.
  //!
  //! NOTE: To use it, you must first `init()` it.
  ASMJIT_INLINE ZoneHeap() noexcept {
    ::memset(this, 0, sizeof(*this));
  }
  //! Create a new `ZoneHeap` initialized to use `zone`.
  explicit ASMJIT_INLINE ZoneHeap(Zone* zone) noexcept {
    ::memset(this, 0, sizeof(*this));
    _zone = zone;
  }
  //! Destroy the `ZoneHeap`.
  ASMJIT_INLINE ~ZoneHeap() noexcept { reset(); }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Get if the `ZoneHeap` is initialized (i.e. has `Zone`).
  ASMJIT_INLINE bool isInitialized() const noexcept { return _zone != nullptr; }

  //! Convenience method to initialize the `ZoneHeap` with `zone`.
  //!
  //! It's the same as calling `reset(zone)`.
  ASMJIT_INLINE void init(Zone* zone) noexcept { reset(zone); }

  //! Reset this `ZoneHeap` and also forget about the current `Zone` which
  //! is attached (if any). Reset optionally attaches a new `zone` passed, or
  //! keeps the `ZoneHeap` in an uninitialized state, if `zone` is null.
  ASMJIT_API void reset(Zone* zone = nullptr) noexcept;

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Get the slot index to be used for `size`. Returns `true` if a valid slot
  //! has been written to `slot` and `allocatedSize` has been filled with slot
  //! exact size (`allocatedSize` can be equal or slightly greater than `size`).
  static ASMJIT_INLINE bool _getSlotIndex(size_t size, uint32_t& slot) noexcept {
    ASMJIT_ASSERT(size > 0);
    if (size > kHiMaxSize)
      return false;

    if (size <= kLoMaxSize)
      slot = static_cast<uint32_t>((size - 1) / kLoGranularity);
    else
      slot = static_cast<uint32_t>((size - kLoMaxSize - 1) / kHiGranularity) + kLoCount;

    return true;
  }

  //! \overload
  static ASMJIT_INLINE bool _getSlotIndex(size_t size, uint32_t& slot, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(size > 0);
    if (size > kHiMaxSize)
      return false;

    if (size <= kLoMaxSize) {
      slot = static_cast<uint32_t>((size - 1) / kLoGranularity);
      allocatedSize = Utils::alignTo(size, kLoGranularity);
    }
    else {
      slot = static_cast<uint32_t>((size - kLoMaxSize - 1) / kHiGranularity) + kLoCount;
      allocatedSize = Utils::alignTo(size, kHiGranularity);
    }

    return true;
  }

  // --------------------------------------------------------------------------
  // [Alloc / Release]
  // --------------------------------------------------------------------------

  ASMJIT_API void* _alloc(size_t size, size_t& allocatedSize) noexcept;
  ASMJIT_API void* _allocZeroed(size_t size, size_t& allocatedSize) noexcept;
  ASMJIT_API void _releaseDynamic(void* p, size_t size) noexcept;

  //! Allocate `size` bytes of memory, ideally from an available pool.
  //!
  //! NOTE: `size` can't be zero, it will assert in debug mode in such case.
  ASMJIT_INLINE void* alloc(size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    size_t allocatedSize;
    return _alloc(size, allocatedSize);
  }

  //! Like `alloc(size)`, but provides a second argument `allocatedSize` that
  //! provides a way to know how big the block returned actually is. This is
  //! useful for containers to prevent growing too early.
  ASMJIT_INLINE void* alloc(size_t size, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(isInitialized());
    return _alloc(size, allocatedSize);
  }

  //! Like `alloc()`, but the return pointer is casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* allocT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(alloc(size));
  }

  //! Like `alloc(size)`, but returns zeroed memory.
  ASMJIT_INLINE void* allocZeroed(size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());

    size_t allocatedSize;
    return _allocZeroed(size, allocatedSize);
  }

  //! Like `alloc(size, allocatedSize)`, but returns zeroed memory.
  ASMJIT_INLINE void* allocZeroed(size_t size, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(isInitialized());

    return _allocZeroed(size, allocatedSize);
  }

  //! Like `allocZeroed()`, but the return pointer is casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* allocZeroedT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(allocZeroed(size));
  }

  //! Release the memory previously allocated by `alloc()`. The `size` argument
  //! has to be the same as used to call `alloc()` or `allocatedSize` returned
  //! by `alloc()`.
  ASMJIT_INLINE void release(void* p, size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());

    ASMJIT_ASSERT(p != nullptr);
    ASMJIT_ASSERT(size != 0);

    uint32_t slot;
    if (_getSlotIndex(size, slot)) {
      //printf("RELEASING %p of size %d (SLOT %u)\n", p, int(size), slot);
      static_cast<Slot*>(p)->next = static_cast<Slot*>(_slots[slot]);
      _slots[slot] = static_cast<Slot*>(p);
    }
    else {
      _releaseDynamic(p, size);
    }
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Zone* _zone;                           //!< Zone used to allocate memory that fits into slots.
  Slot* _slots[kLoCount + kHiCount];     //!< Indexed slots containing released memory.
  DynamicBlock* _dynamicBlocks;          //!< Dynamic blocks for larger allocations (no slots).
};

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ZONEHEAP_H
