// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONE_H
#define _ASMJIT_CORE_ZONE_H

// [Dependencies]
#include "../core/intutils.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::Zone]
// ============================================================================

//! Zone memory.
//!
//! Zone is an incremental memory allocator that allocates memory by simply
//! incrementing a pointer. It allocates blocks of memory by using C's `malloc()`,
//! but divides these blocks into smaller segments requested by calling
//! `Zone::alloc()` and friends.
//!
//! Zone has no function to release the allocated memory. It has to be released
//! all at once by calling `reset()`. If you need a more friendly allocator that
//! also supports `release()`, consider using `Zone` with `ZoneAllocator`.
class Zone {
public:
  ASMJIT_NONCOPYABLE(Zone)

  //! \internal
  //!
  //! A single block of memory.
  struct Block {
    Block* prev;                         //!< Link to the previous block.
    Block* next;                         //!< Link to the next block.
    size_t size;                         //!< Size of the block.
    uint8_t data[sizeof(void*)];         //!< Data.
  };

  enum : uint32_t {
    //! Zone allocator overhead.
    kZoneOverhead = Globals::kAllocOverhead + uint32_t(sizeof(Block))
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `Zone` allocator.
  //!
  //! The `blockSize` parameter describes the default size of the block. If the
  //! `size` parameter passed to `alloc()` is greater than the default size
  //! `Zone` will allocate and use a larger block, but it will not change the
  //! default `blockSize`.
  //!
  //! It's not required, but it's good practice to set `blockSize` to a
  //! reasonable value that depends on the usage of `Zone`. Greater block sizes
  //! are generally safer and perform better than unreasonably low values.
  ASMJIT_API Zone(uint32_t blockSize, uint32_t blockAlignment = 32) noexcept;

  inline Zone(Zone&& other) noexcept
    : _ptr(other._ptr),
      _end(other._end),
      _block(other._block),
      _blockSize(other._blockSize),
      _blockAlignmentShift(other._blockAlignmentShift) {
    other._ptr = nullptr;
    other._end = nullptr;
    other._block = nullptr;
  }

  //! Destroy the `Zone` instance.
  //!
  //! This will destroy the `Zone` instance and release all blocks of memory
  //! allocated by it. It performs implicit `reset(true)`.
  ASMJIT_API ~Zone() noexcept;

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the `Zone` invalidating all blocks allocated.
  //!
  //! If `releaseMemory` is true all buffers will be released to the system.
  ASMJIT_API void reset(bool releaseMemory = false) noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the default block size.
  inline uint32_t blockSize() const noexcept { return _blockSize; }
  //! Get the default block alignment.
  inline uint32_t blockAlignment() const noexcept { return (uint32_t)1 << _blockAlignmentShift; }
  //! Get remaining size of the current block.
  inline size_t remainingSize() const noexcept { return (size_t)(_end - _ptr); }

  //! Get the current zone cursor (dangerous).
  //!
  //! This is a function that can be used to get exclusive access to the current
  //! block's memory buffer.
  inline uint8_t* cursor() noexcept { return _ptr; }
  //! Get the end of the current zone block, only useful if you use `cursor()`.
  inline uint8_t* end() noexcept { return _end; }

  //! Set the current zone cursor to `p` (must match the current block).
  inline void setCursor(uint8_t* p) noexcept {
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _ptr = p;
  }

  // --------------------------------------------------------------------------
  // [Align the current pointer to `alignment` and return it]
  // --------------------------------------------------------------------------

  inline uint8_t* align(size_t alignment) noexcept {
    _ptr = std::min<uint8_t*>(IntUtils::alignUp(_ptr, alignment), _end);
    ASMJIT_ASSERT(_ptr >= _block->data && _ptr <= _end);
    return _ptr;
  }

  // --------------------------------------------------------------------------
  // [Alloc]
  // --------------------------------------------------------------------------

  //! Allocate `size` bytes of memory.
  //!
  //! Pointer returned is valid until the `Zone` instance is destroyed or reset
  //! by calling `reset()`. If you plan to make an instance of C++ from the
  //! given pointer use placement `new` and `delete` operators:
  //!
  //! ~~~
  //! using namespace asmjit;
  //!
  //! class Object { ... };
  //!
  //! // Create Zone with default block size of approximately 65536 bytes.
  //! Zone zone(65536 - Zone::kZoneOverhead);
  //!
  //! // Create your objects using zone object allocating, for example:
  //! Object* obj = static_cast<Object*>( zone.alloc(sizeof(Object)) );
  //
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
  //! ~~~
  ASMJIT_FORCEINLINE void* alloc(size_t size) noexcept {
    uint8_t* ptr = _ptr;
    size_t remainingBytes = (size_t)(_end - ptr);

    if (ASMJIT_UNLIKELY(remainingBytes < size))
      return _alloc(size);

    _ptr += size;
    ASMJIT_ASSERT(_ptr <= _end);

    return static_cast<void*>(ptr);
  }

  //! Allocate `size` bytes of memory aligned to the given `alignment`.
  inline void* allocAligned(size_t size, size_t alignment) noexcept {
    align(alignment);
    return alloc(size);
  }

  //! Allocate `size` bytes without any checks.
  //!
  //! Can only be called if `remainingSize()` returns size at least equal
  //! to `size`.
  inline void* allocNoCheck(size_t size) noexcept {
    ASMJIT_ASSERT((size_t)(_end - _ptr) >= size);

    uint8_t* ptr = _ptr;
    _ptr += size;
    return static_cast<void*>(ptr);
  }

  //! Allocate `size` bytes of zeroed memory. See `alloc()` for more details.
  ASMJIT_API void* allocZeroed(size_t size) noexcept;

  //! Like `alloc()`, but the return pointer is casted to `T*`.
  template<typename T>
  inline T* allocT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(alloc(size));
  }

  //! Like `alloc()`, but the return pointer is casted to `T*`.
  template<typename T>
  inline T* allocAlignedT(size_t size, size_t alignment) noexcept {
    return static_cast<T*>(allocAligned(size, alignment));
  }

  //! Like `allocNoCheck()`, but the return pointer is casted to `T*`.
  template<typename T>
  inline T* allocNoCheckT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(allocNoCheck(size));
  }

  //! Like `allocZeroed()`, but the return pointer is casted to `T*`.
  template<typename T>
  inline T* allocZeroedT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(allocZeroed(size));
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T>
  inline T* newT() noexcept {
    void* p = allocAligned(sizeof(T), alignof(T));
    if (ASMJIT_UNLIKELY(!p))
      return nullptr;
    return new(p) T();
  }
  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T, typename... Args>
  inline T* newT(Args... args) noexcept {
    void* p = allocAligned(sizeof(T), alignof(T));
    if (ASMJIT_UNLIKELY(!p))
      return nullptr;
    return new(p) T(args...);
  }

  //! \internal
  ASMJIT_API void* _alloc(size_t size) noexcept;

  //! Helper to duplicate data.
  ASMJIT_API void* dup(const void* data, size_t size, bool nullTerminate = false) noexcept;

  //! Helper to duplicate data.
  inline void* dupAligned(const void* data, size_t size, size_t alignment, bool nullTerminate = false) noexcept {
    align(alignment);
    return dup(data, size, nullTerminate);
  }

  //! Helper to duplicate a formatted string, maximum size is 256 bytes.
  ASMJIT_API char* sformat(const char* str, ...) noexcept;

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(Zone& other) noexcept {
    std::swap(_ptr, other._ptr);
    std::swap(_end, other._end);
    std::swap(_block, other._block);
    std::swap(_blockSizeAndAlignmentShift, other._blockSizeAndAlignmentShift);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t* _ptr;                         //!< Pointer in the current block's buffer.
  uint8_t* _end;                         //!< End of the current block's buffer.
  Block* _block;                         //!< Current block.

  #if ASMJIT_ARCH_BITS == 64
  union {
    struct {
      uint32_t _blockSize;               //!< Default size of a newly allocated block.
      uint32_t _blockAlignmentShift;     //!< Minimum alignment of each block.
    };
    uint64_t _blockSizeAndAlignmentShift;
  };
  #else
  union {
    struct {
      uint32_t _blockSize : 29;          //!< Default size of a newly allocated block.
      uint32_t _blockAlignmentShift : 3; //!< Minimum alignment of each block.
    };
    uint32_t _blockSizeAndAlignmentShift;
  };
  #endif
};

// ============================================================================
// [asmjit::ZoneAllocator]
// ============================================================================

//! Zone-based memory allocator that uses an existing `Zone` and provides a
//! `release()` functionality on top of it. It uses `Zone` only for chunks
//! that can be pooled, and uses libc `malloc()` for chunks that are large.
//!
//! The advantage of ZoneAllocator is that it can allocate small chunks of memory
//! really fast, and these chunks, when released, will be reused by consecutive
//! calls to `alloc()`. Also, since ZoneAllocator uses `Zone`, you can turn any
//! `Zone` into a `ZoneAllocator`, and use it in your `Pass` when necessary.
//!
//! ZoneAllocator is used by AsmJit containers to make containers having only
//! few elements fast (and lightweight) and to allow them to grow and use
//! dynamic blocks when require more storage.
class ZoneAllocator {
public:
  ASMJIT_NONCOPYABLE(ZoneAllocator)

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

  //! Create a new `ZoneAllocator`.
  //!
  //! NOTE: To use it, you must first `init()` it.
  inline ZoneAllocator() noexcept {
    std::memset(this, 0, sizeof(*this));
  }
  //! Create a new `ZoneAllocator` initialized to use `zone`.
  explicit inline ZoneAllocator(Zone* zone) noexcept {
    std::memset(this, 0, sizeof(*this));
    _zone = zone;
  }
  inline ZoneAllocator(ZoneAllocator&& o)
    : _zone{ o._zone }, _dynamicBlocks{ o._dynamicBlocks }
  {
    o._zone = nullptr;
    o._dynamicBlocks = nullptr;

    for (int i = 0; i != kLoCount + kHiCount; ++i) {
      _slots[i] = o._slots[i];
      o._slots[i] = nullptr;
    }
  }
  //! Destroy the `ZoneAllocator`.
  inline ~ZoneAllocator() noexcept { reset(); }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Get whether the `ZoneAllocator` is initialized (i.e. has `Zone`).
  inline bool isInitialized() const noexcept { return _zone != nullptr; }

  //! Convenience method to initialize the `ZoneAllocator` with `zone`.
  //!
  //! It's the same as calling `reset(zone)`.
  inline void init(Zone* zone) noexcept { reset(zone); }

  //! Reset this `ZoneAllocator` and also forget about the current `Zone` which
  //! is attached (if any). Reset optionally attaches a new `zone` passed, or
  //! keeps the `ZoneAllocator` in an uninitialized state, if `zone` is null.
  ASMJIT_API void reset(Zone* zone = nullptr) noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the `Zone` the allocator is using, or null if it's not initialized.
  inline Zone* zone() const noexcept { return _zone; }

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Get the slot index to be used for `size`. Returns `true` if a valid slot
  //! has been written to `slot` and `allocatedSize` has been filled with slot
  //! exact size (`allocatedSize` can be equal or slightly greater than `size`).
  static ASMJIT_FORCEINLINE bool _getSlotIndex(size_t size, uint32_t& slot) noexcept {
    ASMJIT_ASSERT(size > 0);
    if (size > kHiMaxSize)
      return false;

    if (size <= kLoMaxSize)
      slot = uint32_t((size - 1) / kLoGranularity);
    else
      slot = uint32_t((size - kLoMaxSize - 1) / kHiGranularity) + kLoCount;

    return true;
  }

  //! \overload
  static ASMJIT_FORCEINLINE bool _getSlotIndex(size_t size, uint32_t& slot, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(size > 0);
    if (size > kHiMaxSize)
      return false;

    if (size <= kLoMaxSize) {
      slot = uint32_t((size - 1) / kLoGranularity);
      allocatedSize = IntUtils::alignUp(size, kLoGranularity);
    }
    else {
      slot = uint32_t((size - kLoMaxSize - 1) / kHiGranularity) + kLoCount;
      allocatedSize = IntUtils::alignUp(size, kHiGranularity);
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
  inline void* alloc(size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    size_t allocatedSize;
    return _alloc(size, allocatedSize);
  }

  //! Like `alloc(size)`, but provides a second argument `allocatedSize` that
  //! provides a way to know how big the block returned actually is. This is
  //! useful for containers to prevent growing too early.
  inline void* alloc(size_t size, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(isInitialized());
    return _alloc(size, allocatedSize);
  }

  //! Like `alloc()`, but the return pointer is casted to `T*`.
  template<typename T>
  inline T* allocT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(alloc(size));
  }

  //! Like `alloc(size)`, but returns zeroed memory.
  inline void* allocZeroed(size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    size_t allocatedSize;
    return _allocZeroed(size, allocatedSize);
  }

  //! Like `alloc(size, allocatedSize)`, but returns zeroed memory.
  inline void* allocZeroed(size_t size, size_t& allocatedSize) noexcept {
    ASMJIT_ASSERT(isInitialized());
    return _allocZeroed(size, allocatedSize);
  }

  //! Like `allocZeroed()`, but the return pointer is casted to `T*`.
  template<typename T>
  inline T* allocZeroedT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(allocZeroed(size));
  }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T>
  inline T* newT() noexcept {
    void* p = allocT<T>();
    return p ? new(p) T() : nullptr;
  }
  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T, typename... Args>
  inline T* newT(Args... args) noexcept {
    void* p = allocT<T>();
    return p ? new(p) T(args...) : nullptr;
  }

  //! Release the memory previously allocated by `alloc()`. The `size` argument
  //! has to be the same as used to call `alloc()` or `allocatedSize` returned
  //! by `alloc()`.
  inline void release(void* p, size_t size) noexcept {
    ASMJIT_ASSERT(isInitialized());
    ASMJIT_ASSERT(p != nullptr);
    ASMJIT_ASSERT(size != 0);

    uint32_t slot;
    if (_getSlotIndex(size, slot)) {
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

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONE_H
