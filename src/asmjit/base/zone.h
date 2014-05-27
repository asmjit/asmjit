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
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base_util
//! \{

// ============================================================================
// [asmjit::Zone]
// ============================================================================

//! Fast incremental memory allocator.
//!
//! Memory allocator designed to allocate small objects that will be invalidated
//! (freed) all at once.
struct Zone {
  // --------------------------------------------------------------------------
  // [Chunk]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! One allocated chunk of memory.
  struct Chunk {
    //! Get count of remaining (unused) bytes in chunk.
    ASMJIT_INLINE size_t getRemainingSize() const { return size - pos; }

    //! Link to previous chunk.
    Chunk* prev;
    //! Position in this chunk.
    size_t pos;
    //! Size of this chunk (in bytes).
    size_t size;

    //! Data.
    uint8_t data[sizeof(void*)];
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `Zone` allocator.
  //!
  //! The `chunkSize` parameter describes the size of the chunk. If `alloc()`
  //! requires more memory than `chunkSize` then a bigger chunk will be
  //! allocated, however `chunkSize` will not be changed.
  ASMJIT_API Zone(size_t chunkSize);

  //! Destroy `Zone` instance.
  //!
  //! Destructor released all chunks allocated by `Zone`. The `reset()` member
  //! function does the same without actually destroying `Zone` object itself.
  ASMJIT_API ~Zone();

  // --------------------------------------------------------------------------
  // [Clear / Reset]
  // --------------------------------------------------------------------------

  //! Reset the `Zone` releasing all chunks allocated.
  //!
  //! Calling `clear()` will release all chunks allocated by `Zone` except the
  //! first one that will be reused if needed.
  ASMJIT_API void clear();

  //! Reset the `Zone` releasing all chunks allocated.
  //!
  //! Calling `reset()` will release all chunks allocated by `Zone`.
  ASMJIT_API void reset();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get chunk size.
  ASMJIT_INLINE size_t getChunkSize() const { return _chunkSize; }

  // --------------------------------------------------------------------------
  // [Alloc]
  // --------------------------------------------------------------------------

  //! Allocate `size` bytes of memory.
  //!
  //! Pointer allocated by this way will be valid until `Zone` object is
  //! destroyed. To create class by this way use placement `new` and `delete`
  //! operators:
  //!
  //! ~~~
  //! // Example of simple class allocation.
  //! using namespace asmjit
  //!
  //! // Your class.
  //! class Object {
  //!   // members...
  //! };
  //!
  //! // Your function
  //! void f() {
  //!   // Create zone object with chunk size of 65536 bytes.
  //!   Zone zone(65536);
  //!
  //!   // Create your objects using zone object allocating, for example:
  //!   Object* obj = new(zone.alloc(sizeof(YourClass))) Object();
  //!
  //!   // ... lifetime of your objects ...
  //!
  //!   // Destroy your objects:
  //!   obj->~Object();
  //!
  //!   // Zone destructor will free all memory allocated through it, you can
  //!   // call `zone.reset()` if you wan't to reuse current `Zone`.
  //! }
  //! ~~~
  ASMJIT_INLINE void* alloc(size_t size) {
    Chunk* cur = _chunks;

    if (cur->getRemainingSize() < size)
      return _alloc(size);

    uint8_t* p = cur->data + cur->pos;
    cur->pos += size;
    ASMJIT_ASSERT(cur->pos <= cur->size);

    return (void*)p;
  }

  //! Like `alloc()`, but the return is casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* allocT(size_t size = sizeof(T)) {
    return static_cast<T*>(alloc(size));
  }

  //! \internal
  ASMJIT_API void* _alloc(size_t size);

  //! Allocate `size` bytes of zeroed memory.
  ASMJIT_INLINE void* allocZeroed(size_t size) {
    Chunk* cur = _chunks;

    if (cur->getRemainingSize() < size)
      return _allocZeroed(size);

    uint8_t* p = cur->data + cur->pos;
    cur->pos += size;
    ASMJIT_ASSERT(cur->pos <= cur->size);

    ::memset(p, 0, size);
    return (void*)p;
  }

  //! \internal
  ASMJIT_API void* _allocZeroed(size_t size);

  //! Helper to duplicate data.
  ASMJIT_API void* dup(const void* data, size_t size);

  //! Helper to duplicate string.
  ASMJIT_API char* sdup(const char* str);

  //! Helper to duplicate formatted string, maximum length is 256 bytes.
  ASMJIT_API char* sformat(const char* str, ...);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Last allocated chunk of memory.
  Chunk* _chunks;
  //! Default chunk size.
  size_t _chunkSize;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ZONE_H
