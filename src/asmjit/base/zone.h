// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ZONE_H
#define _ASMJIT_BASE_ZONE_H

// [Dependencies]
#include "../base/globals.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::Zone]
// ============================================================================

//! Memory zone.
//!
//! Zone is an incremental memory allocator that allocates memory by simply
//! incrementing a pointer. It allocates blocks of memory by using standard
//! C `malloc`, but divides these blocks into smaller segments requested by
//! calling `Zone::alloc()` and friends.
//!
//! Zone has no function to release the allocated memory. It has to be released
//! all at once by calling `reset()`. If you need a more friendly allocator that
//! also supports `release()`, consider using \ref Zone with \ref ZoneHeap.
class Zone {
public:
  //! \internal
  //!
  //! A single block of memory.
  struct Block {
    Block* prev;                         //!< Link to the previous block.
    Block* next;                         //!< Link to the next block.
    size_t size;                         //!< Size of the block.
    uint8_t data[sizeof(void*)];         //!< Data.
  };

  enum {
    //! Zone allocator overhead.
    kZoneOverhead = kMemAllocOverhead + static_cast<int>(sizeof(Block))
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
  //! are generally safer and performs better than unreasonably low values.
  ASMJIT_API Zone(uint32_t blockSize, uint32_t blockAlignment = 0) noexcept;

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
  ASMJIT_INLINE uint32_t getBlockSize() const noexcept { return _blockSize; }
  //! Get the default block alignment.
  ASMJIT_INLINE uint32_t getBlockAlignment() const noexcept { return (uint32_t)1 << _blockAlignmentShift; }
  //! Get remaining size of the current block.
  ASMJIT_INLINE size_t getRemainingSize() const noexcept { return (size_t)(_end - _ptr); }

  //! Get the current zone cursor (dangerous).
  //!
  //! This is a function that can be used to get exclusive access to the current
  //! block's memory buffer.
  ASMJIT_INLINE uint8_t* getCursor() noexcept { return _ptr; }
  //! Get the end of the current zone block, only useful if you use `getCursor()`.
  ASMJIT_INLINE uint8_t* getEnd() noexcept { return _end; }

  //! Set the current zone cursor to `p` (must match the current block).
  //!
  //! This is a counterpart of `getZoneCursor()`.
  ASMJIT_INLINE void setCursor(uint8_t* p) noexcept {
    ASMJIT_ASSERT(p >= _ptr && p <= _end);
    _ptr = p;
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
  ASMJIT_INLINE void* alloc(size_t size) noexcept {
    uint8_t* ptr = _ptr;
    size_t remainingBytes = (size_t)(_end - ptr);

    if (ASMJIT_UNLIKELY(remainingBytes < size))
      return _alloc(size);

    _ptr += size;
    ASMJIT_ASSERT(_ptr <= _end);

    return static_cast<void*>(ptr);
  }

  //! Allocate `size` bytes without any checks.
  //!
  //! Can only be called if `getRemainingSize()` returns size at least equal
  //! to `size`.
  ASMJIT_INLINE void* allocNoCheck(size_t size) noexcept {
    ASMJIT_ASSERT((size_t)(_end - _ptr) >= size);

    uint8_t* ptr = _ptr;
    _ptr += size;
    return static_cast<void*>(ptr);
  }

  //! Allocate `size` bytes of zeroed memory.
  //!
  //! See \ref alloc() for more details.
  ASMJIT_API void* allocZeroed(size_t size) noexcept;

  //! Like `alloc()`, but the return pointer is casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* allocT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(alloc(size));
  }

  //! Like `allocNoCheck()`, but the return pointer is casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* allocNoCheckT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(allocNoCheck(size));
  }

  //! Like `allocZeroed()`, but the return pointer is casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* allocZeroedT(size_t size = sizeof(T)) noexcept {
    return static_cast<T*>(allocZeroed(size));
  }

  //! \internal
  ASMJIT_API void* _alloc(size_t size) noexcept;

  //! Helper to duplicate data.
  ASMJIT_API void* dup(const void* data, size_t size, bool nullTerminate = false) noexcept;

  //! Helper to duplicate formatted string, maximum length is 256 bytes.
  ASMJIT_API char* sformat(const char* str, ...) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t* _ptr;                         //!< Pointer in the current block's buffer.
  uint8_t* _end;                         //!< End of the current block's buffer.
  Block* _block;                         //!< Current block.

#if ASMJIT_ARCH_64BIT
  uint32_t _blockSize;                   //!< Default size of a newly allocated block.
  uint32_t _blockAlignmentShift;         //!< Minimum alignment of each block.
#else
  uint32_t _blockSize : 29;              //!< Default size of a newly allocated block.
  uint32_t _blockAlignmentShift : 3;     //!< Minimum alignment of each block.
#endif
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ZONE_H
