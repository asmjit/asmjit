// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ZONE_H
#define _ASMJIT_BASE_ZONE_H

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::Zone]
// ============================================================================

//! @brief Fast incremental memory allocator.
//!
//! Memory allocator designed to allocate small objects that will be invalidated
//! (free) all at once.
struct Zone {
  // --------------------------------------------------------------------------
  // [Chunk]
  // --------------------------------------------------------------------------

  //! @internal
  //!
  //! @brief One allocated chunk of memory.
  struct Chunk {
    //! @brief Get count of remaining (unused) bytes in chunk.
    ASMJIT_INLINE size_t getRemainingSize() const { return size - pos; }

    //! @brief Link to previous chunk.
    Chunk* prev;
    //! @brief Position in this chunk.
    size_t pos;
    //! @brief Size of this chunk (in bytes).
    size_t size;

    //! @brief Data.
    uint8_t data[sizeof(void*)];
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new instance of @c Zone allocator.
  //!
  //! @param chunkSize Default size of the first chunk.
  ASMJIT_API Zone(size_t chunkSize);

  //! @brief Destroy @ref Zone instance.
  ASMJIT_API ~Zone();

  // --------------------------------------------------------------------------
  // [Clear / Reset]
  // --------------------------------------------------------------------------

  //! @brief Free all allocated memory except first block that remains for reuse.
  //!
  //! Note that this method will invalidate all instances using this memory
  //! allocated by this zone instance.
  ASMJIT_API void clear();

  //! @brief Free all allocated memory at once.
  //!
  //! Note that this method will invalidate all instances using this memory
  //! allocated by this zone instance.
  ASMJIT_API void reset();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get (default) chunk size.
  ASMJIT_INLINE size_t getChunkSize() const { return _chunkSize; }

  // --------------------------------------------------------------------------
  // [Alloc]
  // --------------------------------------------------------------------------

  //! @brief Allocate @c size bytes of memory.
  //!
  //! Pointer allocated by this way will be valid until @c Zone object is
  //! destroyed. To create class by this way use placement @c new and @c delete
  //! operators:
  //!
  //! @code
  //! // Example of simple class allocation.
  //! using namespace asmjit
  //!
  //! // Your class.
  //! class Object
  //! {
  //!   // members...
  //! };
  //!
  //! // Your function
  //! void f()
  //! {
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
  //!   // call @c zone.reset() if you wan't to reuse current @ref Zone.
  //! }
  //! @endcode
  ASMJIT_INLINE void* alloc(size_t size) {
    Chunk* cur = _chunks;

    if (cur == NULL || cur->getRemainingSize() < size)
      return _alloc(size);

    uint8_t* p = cur->data + cur->pos;

    cur->pos += size;
    ASMJIT_ASSERT(cur->pos <= cur->size);

    return (void*)p;
  }

  //! @brief Like @ref alloc(), but returns <code>T*</code>.
  template<typename T>
  ASMJIT_INLINE T* allocT(size_t size = sizeof(T)) {
    return static_cast<T*>(alloc(size));
  }

  //! @internal
  ASMJIT_API void* _alloc(size_t size);

  //! @brief Allocate @c size bytes of zeroed memory.
  ASMJIT_INLINE void* calloc(size_t size) {
    Chunk* cur = _chunks;

    if (cur == NULL || cur->getRemainingSize() < size)
      return _calloc(size);

    uint8_t* p = cur->data + cur->pos;

    cur->pos += size;
    ASMJIT_ASSERT(cur->pos <= cur->size);

    ::memset(p, 0, size);
    return (void*)p;
  }

  //! @internal
  ASMJIT_API void* _calloc(size_t size);

  //! @brief Helper to duplicate data.
  ASMJIT_API void* dup(const void* data, size_t size);

  //! @brief Helper to duplicate string.
  ASMJIT_API char* sdup(const char* str);

  //! @brief Helper to duplicate formatted string, maximum length is 256 bytes.
  ASMJIT_API char* sformat(const char* str, ...);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Last allocated chunk of memory.
  Chunk* _chunks;
  //! @brief Default chunk size.
  size_t _chunkSize;
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

#endif // _ASMJIT_BASE_ZONE_H
