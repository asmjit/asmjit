// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_MEMMGR_H
#define _ASMJIT_CORE_MEMMGR_H

// [Dependencies]
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

//! Memory management.
namespace MemMgr {

// ============================================================================
// [asmjit::MemMgr - Alloc / Release]
// ============================================================================

#if defined(ASMJIT_CUSTOM_ALLOC)   && \
    defined(ASMJIT_CUSTOM_REALLOC) && \
    defined(ASMJIT_CUSTOM_FREE)

static inline void* alloc(size_t size) noexcept { return ASMJIT_CUSTOM_ALLOC(size); }
static inline void* realloc(void* p, size_t size) noexcept { return ASMJIT_CUSTOM_REALLOC(p, size); }
static inline void release(void* p) noexcept { ASMJIT_CUSTOM_FREE(p); }

#elif !defined(ASMJIT_CUSTOM_ALLOC)   && \
      !defined(ASMJIT_CUSTOM_REALLOC) && \
      !defined(ASMJIT_CUSTOM_FREE)

static inline void* alloc(size_t size) noexcept { return std::malloc(size); }
static inline void* realloc(void* p, size_t size) noexcept { return std::realloc(p, size); }
static inline void release(void* p) noexcept { std::free(p); }

#else
#error "[asmjit] You must provide either none or all of ASMJIT_CUSTOM_[ALLOC|REALLOC|FREE]"
#endif

} // MemMgr namespace

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_MEMMGR_H
