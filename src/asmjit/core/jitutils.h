// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_JITUTILS_H
#define _ASMJIT_CORE_JITUTILS_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_JIT

// [Dependencies]
#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_jit
//! \{

// ============================================================================
// [asmjit::JitUtils]
// ============================================================================

//! JIT utilities that provide virtual memory allocation.
namespace JitUtils {

//! Memory access flags.
enum VirtMemFlags : uint32_t {
  kVirtMemNoFlags         = 0x00000000u, //!< No access flags.
  kVirtMemWrite           = 0x00000001u, //!< Memory is writable.
  kVirtMemExecute         = 0x00000002u, //!< Memory is executable.

  kVirtMemWriteExecute    = kVirtMemWrite |
                            kVirtMemExecute
};

//! Virtual memory information.
struct MemInfo {
  uint32_t pageSize;                     //!< Virtual memory page size.
  uint32_t pageGranularity;              //!< Virtual memory page granularity.
};

//! Get virtual memory information, see `JitUtils::MemInfo` for more details.
ASMJIT_API MemInfo memInfo() noexcept;

//! Allocate virtual memory.
//!
//! NOTE: `size` should be aligned to page size, use `memInfo()` to obtain it.
ASMJIT_API void* virtualAlloc(size_t size, uint32_t vmFlags) noexcept;

//! Release virtual memory previously allocated by `JitUtils::virtualAlloc()`.
ASMJIT_API Error virtualRelease(void* p, size_t size) noexcept;

} // JitUtils

//! \}

ASMJIT_END_NAMESPACE

#endif
#endif
