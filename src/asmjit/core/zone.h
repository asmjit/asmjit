// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ZONE_H_INCLUDED
#define ASMJIT_CORE_ZONE_H_INCLUDED

#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_zone
//! \{

//! Zone memory.
//!
//! Zone is an incremental memory allocator that allocates memory by simply incrementing a pointer. It allocates
//! blocks of memory by using C's `malloc()`, but divides these blocks into smaller segments requested by calling
//! `Zone::alloc()` and friends.
//!
//! Zone has no function to release the allocated memory. It has to be released all at once by calling `reset()`.
//! If you need a more friendly allocator that also supports `release()`, consider using `Zone` with `ZoneAllocator`.
class Zone {
public:
  ASMJIT_NONCOPYABLE(Zone)

  //! \cond INTERNAL

  //! A single block of memory managed by `Zone`.
  struct alignas(Globals::kZoneAlignment) Block {
    //! Link to the next block (single-linked list).
    Block* next;
    //! Size represents the number of bytes that can be allocated (it doesn't include overhead).
    size_t size;

    ASMJIT_INLINE_NODEBUG uint8_t* data() const noexcept {
      return const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(this) + sizeof(*this));
    }

    ASMJIT_INLINE_NODEBUG uint8_t* end() const noexcept {
      return data() + size;
    }
  };

  static inline constexpr size_t kMinBlockSize = 256; // The number is ridiculously small, but still possible.
  static inline constexpr size_t kMaxBlockSize = size_t(1) << (sizeof(size_t) * 8 - 1);
  static inline constexpr size_t kBlockSize = sizeof(Block);

  static ASMJIT_API const Block _zeroBlock;

  template<typename T>
  static ASMJIT_INLINE_CONSTEXPR size_t alignedSizeOf() noexcept {
    return Support::alignUp(sizeof(T), Globals::kZoneAlignment);
  }

  //! \endcond

  //! \name Members
  //! \{

  //! Pointer in the current block.
  uint8_t* _ptr;
  //! End of the current block.
  uint8_t* _end;
  //! Current block.
  Block* _block;
  //! First block (single-linked list).
  Block* _first;

  //! Current block size shift - reverted to _minimumBlockSizeShift every time the Zone is `reset(ResetPolicy::kHard)`.
  uint8_t _currentBlockSizeShift;
  //! Minimum log2(blockSize) to allocate.
  uint8_t _minimumBlockSizeShift;
  //! Maximum log2(blockSize) to allocate.
  uint8_t _maximumBlockSizeShift;
  //! True when the Zone has a static block (static blocks are used by ZoneTmp).
  uint8_t _hasStaticBlock;
  //! Reserved for future use, must be zero.
  uint32_t _reserved;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new Zone.
  //!
  //! The `blockSize` parameter describes the default size of the block. If the `size` parameter passed to `alloc()`
  //! is greater than the default size `Zone` will allocate and use a larger block, but it will not change the
  //! default `blockSize`.
  //!
  //! It's not required, but it's good practice to set `blockSize` to a reasonable value that depends on the usage
  //! of `Zone`. Greater block sizes are generally safer and perform better than unreasonably low block sizes.
  ASMJIT_INLINE_NODEBUG explicit Zone(size_t minimumBlockSize) noexcept {
    _init(minimumBlockSize, nullptr);
  }

  //! Creates a new Zone with a first block pointing to a `temporary` memory.
  ASMJIT_INLINE_NODEBUG Zone(size_t minimumBlockSize, const Support::Temporary& temporary) noexcept {
    _init(minimumBlockSize, &temporary);
  }

  //! \overload
  ASMJIT_INLINE_NODEBUG Zone(size_t minimumBlockSize, const Support::Temporary* temporary) noexcept {
    _init(minimumBlockSize, temporary);
  }

  //! Moves an existing `Zone`.
  //!
  //! \note You cannot move an existing `ZoneTmp` as it uses embedded storage. Attempting to move `ZoneTmp` would
  //! cause an undefined behavior (covered by assertions in debug mode).
  inline Zone(Zone&& other) noexcept
    : _ptr(other._ptr),
      _end(other._end),
      _block(other._block),
      _first(other._first),
      _currentBlockSizeShift(other._currentBlockSizeShift),
      _minimumBlockSizeShift(other._minimumBlockSizeShift),
      _maximumBlockSizeShift(other._maximumBlockSizeShift),
      _hasStaticBlock(other._hasStaticBlock),
      _reserved(other._reserved) {
    ASMJIT_ASSERT(!other.hasStaticBlock());
    other._ptr = other._block->data();
    other._end = other._block->data();
    other._block = const_cast<Block*>(&_zeroBlock);
    other._first = const_cast<Block*>(&_zeroBlock);
  }

  //! Destroys the `Zone` instance.
  //!
  //! This will destroy the `Zone` instance and release all blocks of memory allocated by it. It performs implicit
  //! `reset(ResetPolicy::kHard)`.
  ASMJIT_INLINE_NODEBUG ~Zone() noexcept { reset(ResetPolicy::kHard); }

  ASMJIT_API void _init(size_t blockSize, const Support::Temporary* temporary) noexcept;

  //! Resets the `Zone` invalidating all blocks allocated.
  //!
  //! See `Globals::ResetPolicy` for more details.
  ASMJIT_API void reset(ResetPolicy resetPolicy = ResetPolicy::kSoft) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns a minimum block size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t minimumBlockSize() const noexcept { return size_t(1) << _minimumBlockSizeShift; }

  //! Returns a maximum block size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t maximumBlockSize() const noexcept { return size_t(1) << _maximumBlockSizeShift; }

  //! Tests whether this `Zone` is actually a `ZoneTmp` that uses temporary memory.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint8_t hasStaticBlock() const noexcept { return _hasStaticBlock; }

  //! Returns remaining size of the current block.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t remainingSize() const noexcept { return (size_t)(_end - _ptr); }

  //! Returns the current zone cursor (dangerous).
  //!
  //! This is a function that can be used to get exclusive access to the current block's memory buffer.
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* ptr() noexcept { return reinterpret_cast<T*>(_ptr); }

  //! Returns the end of the current zone block, only useful if you use `ptr()`.
  template<typename T = uint8_t>
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* end() noexcept { return reinterpret_cast<T*>(_end); }

  //! Sets the current zone pointer to `ptr` (must be within the current block).
  template<typename T>
  ASMJIT_INLINE void setPtr(T* ptr) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(ptr);
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _ptr = p;
  }

  //! Sets the end zone pointer to `end` (must be within the current block).
  template<typename T>
  ASMJIT_INLINE void setEnd(T* end) noexcept {
    uint8_t* p = reinterpret_cast<uint8_t*>(end);
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _end = p;
  }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE void swap(Zone& other) noexcept {
    // This could lead to a disaster.
    ASMJIT_ASSERT(!this->hasStaticBlock());
    ASMJIT_ASSERT(!other.hasStaticBlock());

    std::swap(_ptr, other._ptr);
    std::swap(_end, other._end);
    std::swap(_block, other._block);
    std::swap(_first, other._first);
    std::swap(_currentBlockSizeShift, other._currentBlockSizeShift);
    std::swap(_minimumBlockSizeShift, other._minimumBlockSizeShift);
    std::swap(_maximumBlockSizeShift, other._maximumBlockSizeShift);
    std::swap(_hasStaticBlock, other._hasStaticBlock);
    std::swap(_reserved, other._reserved);
  }

  //! Aligns the current pointer to `alignment`.
  ASMJIT_INLINE_NODEBUG void align(size_t alignment) noexcept {
    _ptr = Support::min(Support::alignUp(_ptr, alignment), _end);
  }

  //! \}

  //! \name Allocation
  //! \{

  //! \cond INTERNAL

  //! Internal alloc function used by inline wrappers.
  [[nodiscard]]
  ASMJIT_API void* _alloc(size_t size) noexcept;

  //! \endcond

  //! Allocates the requested memory specified by `size` and optionally casts the returned value to `T*`.
  //!
  //! Pointer returned is valid until the `Zone` instance is destroyed or reset by calling `reset()`. If you plan to
  //! make an instance of C++ from the given pointer use placement `new` and `delete` operators:
  //!
  //! ```
  //! using namespace asmjit;
  //!
  //! class Object { ... };
  //!
  //! // Create Zone with default block size of 65536 bytes (the maximum size per alloc() would be slightly less).
  //! Zone zone(65536);
  //!
  //! // Create your objects using zone object allocating, for example:
  //! Object* obj = static_cast<Object*>( zone.alloc(sizeof(Object)) );
  //!
  //! if (!obj) {
  //!   // Handle out of memory error.
  //! }
  //!
  //! // Placement `new` and `delete` operators can be used to instantiate it.
  //! new(obj) Object();
  //!
  //! // ... lifetime of your objects ...
  //!
  //! // To destroy the instance (if required).
  //! obj->~Object();
  //!
  //! // Reset or destroy `Zone`.
  //! zone.reset();
  //! ```
  template<typename T = void>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc(size_t size) noexcept {
    ASMJIT_ASSERT(Support::isAligned(size, Globals::kZoneAlignment));
#if defined(__GNUC__)
    // We can optimize this function a little bit if we know that `size` is relatively small - which would mean
    // that we cannot possibly overflow `_ptr`. Since most of the time `alloc()` is used for known types (which
    // implies their size is known as well) this optimization is worth it as it may save us 1 or 2 instructions.
    if (__builtin_constant_p(size) && size <= 1024u) {
      uint8_t* after = _ptr + size;

      if (ASMJIT_UNLIKELY(after > _end)) {
        return static_cast<T*>(_alloc(size));
      }

      uint8_t* p = _ptr;
      _ptr = after;
      return static_cast<T*>(static_cast<void*>(p));
    }
#endif

    if (ASMJIT_UNLIKELY(size > remainingSize())) {
      return static_cast<T*>(_alloc(size));
    }

    uint8_t* p = _ptr;
    _ptr += size;
    return static_cast<T*>(static_cast<void*>(p));
  }

  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE T* alloc() noexcept {
    return alloc<T>(alignedSizeOf<T>());
  }

  //! Allocates `size` bytes of zeroed memory. See `alloc()` for more details.
  [[nodiscard]]
  ASMJIT_API void* allocZeroed(size_t size) noexcept;

  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T>
  [[nodiscard]]
  ASMJIT_INLINE T* newT() noexcept {
    void* p = alloc(alignedSizeOf<T>());
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) T();
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T, typename... Args>
  [[nodiscard]]
  ASMJIT_INLINE T* newT(Args&&... args) noexcept {
    void* p = alloc(alignedSizeOf<T>());
    if (ASMJIT_UNLIKELY(!p)) {
      return nullptr;
    }
    return new(Support::PlacementNew{p}) T(std::forward<Args>(args)...);
  }

  //! Helper to duplicate data.
  [[nodiscard]]
  ASMJIT_API void* dup(const void* data, size_t size, bool nullTerminate = false) noexcept;

  //! Helper to duplicate a formatted string, maximum size is 256 bytes.
  [[nodiscard]]
  ASMJIT_API char* sformat(const char* str, ...) noexcept;

  //! \}
};

//! \ref Zone with `N` bytes of a static storage, used for the initial block.
//!
//! Temporary zones are used in cases where it's known that some memory will be required, but in many cases it won't
//! exceed N bytes, so the whole operation can be performed without a dynamic memory allocation.
template<size_t N>
class ZoneTmp : public Zone {
public:
  ASMJIT_NONCOPYABLE(ZoneTmp)

  //! Temporary storage, embedded after \ref Zone.
  struct Storage {
    char data[N];
  } _storage;

  //! Creates a temporary zone. Dynamic block size is specified by `blockSize`.
  inline explicit ZoneTmp(size_t blockSize) noexcept
    : Zone(blockSize, Support::Temporary(_storage.data, N)) {}
};

//! Zone-based memory allocator that uses an existing `Zone` and provides a `release()` functionality on top of it.
//! It uses `Zone` only for chunks that can be pooled, and uses libc `malloc()` for chunks that are large.
//!
//! The advantage of ZoneAllocator is that it can allocate small chunks of memory really fast, and these chunks,
//! when released, will be reused by consecutive calls to `alloc()`. Also, since ZoneAllocator uses `Zone`, you can
//! turn any `Zone` into a `ZoneAllocator`, and use it in your `Pass` when necessary.
//!
//! ZoneAllocator is used by AsmJit containers to make containers having only few elements fast (and lightweight)
//! and to allow them to grow and use dynamic blocks when require more storage.
class ZoneAllocator {
public:
  ASMJIT_NONCOPYABLE(ZoneAllocator)

  //! \cond INTERNAL

  // In short, we pool chunks of these sizes:
  //   [32, 64, 96, 128, 192, 256, 320, 384, 448, 512]

  //! How many bytes per a low granularity pool (has to be at least 16).
  static inline constexpr uint32_t kLoGranularity = 32;
  //! Number of slots of a low granularity pool.
  static inline constexpr uint32_t kLoCount = 4;
  //! Maximum size of a block that can be allocated in a low granularity pool.
  static inline constexpr uint32_t kLoMaxSize = kLoGranularity * kLoCount;

  //! How many bytes per a high granularity pool.
  static inline constexpr uint32_t kHiGranularity = 64;
  //! Number of slots of a high granularity pool.
  static inline constexpr uint32_t kHiCount = 6;
  //! Maximum size of a block that can be allocated in a high granularity pool.
  static inline constexpr uint32_t kHiMaxSize = kLoMaxSize + kHiGranularity * kHiCount;

  //! Number of slots.
  static inline constexpr uint32_t kSlotCount = kLoCount + kHiCount;

  //! Alignment of every pointer returned by `alloc()`.
  static inline constexpr uint32_t kBlockAlignment = kLoGranularity;

  //! Single-linked list used to store unused chunks.
  struct Slot {
    //! Link to a next slot in a single-linked list.
    Slot* next;
  };

  //! A block of memory that has been allocated dynamically and is not part of block-list used by the allocator.
  //! This is used to keep track of all these blocks so they can be freed by `reset()` if not freed explicitly.
  struct DynamicBlock {
    DynamicBlock* prev;
    DynamicBlock* next;
  };

  //! Returns the slot index to be used for `size`. Returns `true` if a valid slot has been written to `slot` and
  //! `allocatedSize` has been filled with slot exact size (`allocatedSize` can be equal or slightly greater than
  //! `size`).
  [[nodiscard]]
  static ASMJIT_INLINE bool _getSlotIndex(size_t size, size_t& slot) noexcept {
    size_t slot_lo = (size - 1u) / kLoGranularity;
    size_t slot_hi = (size - kLoCount * kLoGranularity + kLoCount * kHiGranularity - 1u) / kHiGranularity;

    slot = Support::min(slot_lo, slot_hi);
    return slot < kSlotCount;
  }

  //! \overload
  [[nodiscard]]
  static ASMJIT_INLINE bool _getSlotIndex(size_t size, size_t& slot, size_t& allocatedSize) noexcept {
    size_t slot_lo = (size - 1u) / kLoGranularity;
    size_t slot_hi = (size - kLoCount * kLoGranularity + kLoCount * kHiGranularity - 1u) / kHiGranularity;

    slot = Support::min(slot_lo, slot_hi);
    allocatedSize = Support::alignUp(size, slot < kLoCount ? kLoGranularity : kHiGranularity);

    return slot < kSlotCount;
  }

  //! \endcond

  //! \name Members
  //! \{

  //! Zone used to allocate memory that fits into slots.
  Zone* _zone {};
  //! Indexed slots containing released memory.
  Slot* _slots[kSlotCount] {};
  //! Dynamic blocks for larger allocations (no slots).
  DynamicBlock* _dynamicBlocks {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `ZoneAllocator`.
  //!
  //! \note To use it, you must first `init()` it.
  ASMJIT_INLINE_NODEBUG ZoneAllocator() noexcept {}

  //! Creates a new `ZoneAllocator` initialized to use `zone`.
  ASMJIT_INLINE_NODEBUG explicit ZoneAllocator(Zone* zone) noexcept
    : _zone(zone) {}

  //! Destroys the `ZoneAllocator`.
  ASMJIT_INLINE_NODEBUG ~ZoneAllocator() noexcept { reset(); }

  //! Tests whether the `ZoneAllocator` is initialized (i.e. has `Zone`).
  ASMJIT_INLINE_NODEBUG bool isInitialized() const noexcept { return _zone != nullptr; }

  //! Convenience function to initialize the `ZoneAllocator` with `zone`.
  //!
  //! It's the same as calling `reset(zone)`.
  ASMJIT_INLINE_NODEBUG void init(Zone* zone) noexcept { reset(zone); }

  //! Resets this `ZoneAllocator` and also forget about the current `Zone` which is attached (if any). Reset
  //! optionally attaches a new `zone` passed, or keeps the `ZoneAllocator` in an uninitialized state, if
  //! `zone` is null.
  ASMJIT_API void reset(Zone* zone = nullptr) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the assigned `Zone` of this allocator or null if this `ZoneAllocator` is not initialized.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Zone* zone() const noexcept { return _zone; }

  //! \}

  //! \name Allocation
  //! \{

  //! \cond INTERNAL
  [[nodiscard]]
  ASMJIT_API void* _alloc(size_t size, size_t& allocatedSize) noexcept;

  [[nodiscard]]
  ASMJIT_API void* _allocZeroed(size_t size, size_t& allocatedSize) noexcept;

  ASMJIT_API void _releaseDynamic(void* p, size_t size) noexcept;
  //! \endcond

  //! Allocates `size` bytes of memory, ideally from an available pool.
  //!
  //! \note `size` can't be zero, it will assert in debug mode in such case.
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc(size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    size_t allocatedSize;
    return static_cast<T*>(_alloc(size, allocatedSize));
  }

  //! Like `alloc(size)`, but provides a second argument `allocatedSize` that provides a way to know how big
  //! the block returned actually is. This is useful for containers to prevent growing too early.
  template<typename T = void>
  [[nodiscard]]
  inline T* alloc(size_t size, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(isInitialized());
    return static_cast<T*>(_alloc(size, allocatedSize));
  }

  //! Like `alloc(size)`, but returns zeroed memory.
  template<typename T = void>
  [[nodiscard]]
  inline void* allocZeroed(size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    size_t allocatedSize;
    return static_cast<T*>(_allocZeroed(size, allocatedSize));
  }

  //! Like `alloc(size, allocatedSize)`, but returns zeroed memory.
  template<typename T = void>
  [[nodiscard]]
  inline T* allocZeroed(size_t size, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(isInitialized());
    return static_cast<T*>(_allocZeroed(size, allocatedSize));
  }

  //! Releases the memory previously allocated by `alloc()`. The `size` argument has to be the same as used to call
  //! `alloc()` or `allocatedSize` returned  by `alloc()`.
  inline void release(void* p, size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    ASMJIT_ASSERT(p != nullptr);
    ASMJIT_ASSERT(size != 0);

    size_t slot;
    if (_getSlotIndex(size, slot)) {
      static_cast<Slot*>(p)->next = static_cast<Slot*>(_slots[slot]);
      _slots[slot] = static_cast<Slot*>(p);
    }
    else {
      _releaseDynamic(p, size);
    }
  }

  //! \}
};

//! Helper class for implementing pooling of arena-allocated objects.
template<typename T, size_t SizeOfT = sizeof(T)>
class ZonePool {
public:
  ASMJIT_NONCOPYABLE(ZonePool)

  struct Link { Link* next; };
  Link* _data {};

  ASMJIT_INLINE_NODEBUG ZonePool() noexcept = default;

  //! Resets the arena pool.
  //!
  //! Reset must be called after the associated `ArenaAllocator` has been reset, otherwise the existing pool will
  //! collide with possible allocations made on the `ArenaAllocator` object after the reset.
  ASMJIT_INLINE_NODEBUG void reset() noexcept { _data = nullptr; }

  //! Allocates a memory (or reuses the existing allocation) of `SizeOfT` (in bytes).
  [[nodiscard]]
  ASMJIT_INLINE T* alloc(Zone& zone) noexcept {
    Link* p = _data;
    if (ASMJIT_UNLIKELY(p == nullptr)) {
      return zone.alloc<T>(Support::alignUp(SizeOfT, Globals::kZoneAlignment));
    }
    _data = p->next;
    return static_cast<T*>(static_cast<void*>(p));
  }

  //! Pools the previously allocated memory.
  ASMJIT_INLINE void release(T* ptr) noexcept {
    ASMJIT_ASSERT(ptr != nullptr);
    Link* p = reinterpret_cast<Link*>(ptr);

    p->next = _data;
    _data = p;
  }
};
//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ZONE_H_INCLUDED
