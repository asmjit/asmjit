// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_GLOBALS_H
#define _ASMJIT_BASE_GLOBALS_H

// [Dependencies - AsmJit]
#include "../build.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::kGlobal]
// ============================================================================

ASMJIT_ENUM(kGlobal) {
  //! @brief Invalid operand id.
  kInvalidValue = 0xFFFFFFFF,

  //! @brief Invalid register index.
  kInvalidReg = 0xFF,

  //! @brief Minimum reserved bytes in @ref Buffer.
  kBufferGrow = 32U,

  //! @brief Minimum size of assembler/compiler code buffer.
  kMemAllocMinimum = 4096,

  //! @brief Memory grow threshold.
  //!
  //! If the grow threshold is reached capacity is not doubled anymore.
  kMemAllocGrowMax = 8192 * 1024,

  //! @brief An overhead of the host memory allocator.
  //!
  //! We decrement the overhead from our pools so the host operating system
  //! doesn't need allocate an extra virtual page to put the data it needs
  //! to manage the requested memory block (for example if a single virtual
  //! page is 4096 and we require the same memory size we decrease our
  //! requirement by kMemAllocOverhead).
  kMemAllocOverhead = sizeof(intptr_t) * 4,
};

static const size_t kInvalidIndex = ~static_cast<size_t>(0);

// ============================================================================
// [asmjit::kArch]
// ============================================================================

//! @brief Architecture.
ASMJIT_ENUM(kArch) {
  //! @brief No/Unknown architecture.
  kArchNone = 0,

  //! @brief X86 architecture.
  kArchX86 = 1,
  //! @brief X64 architecture, also called AMD64.
  kArchX64 = 2,

  //! @brief Arm architecture.
  kArchArm = 4,

#if defined(ASMJIT_HOST_X86)
  kArchHost = kArchX86,
#endif // ASMJIT_HOST

#if defined(ASMJIT_HOST_X64)
  kArchHost = kArchX86,
#endif // ASMJIT_HOST

#if defined(ASMJIT_HOST_ARM)
  kArchHost = kArchArm,
#endif // ASMJIT_HOST_ARM

  //! @brief Whether the host is 64-bit.
  kArchHost64Bit = sizeof(intptr_t) >= 8
};

// ============================================================================
// [asmjit::_Initialize]
// ============================================================================

struct _Initialize {};
static const _Initialize Initialize = {};

// ============================================================================
// [asmjit::_DontInitialize]
// ============================================================================

struct _DontInitialize {};
static const _DontInitialize DontInitialize = {};

//! @}

} // asmjit namespace

// ============================================================================
// [asmjit_cast<>]
// ============================================================================

//! @brief Cast used to cast pointer to function. It's like reinterpret_cast<>,
//! but uses internally C style cast to work with MinGW.
//!
//! If you are using single compiler and @c reinterpret_cast<> works for you,
//! there is no reason to use @c asmjit_cast<>. If you are writing
//! cross-platform software with various compiler support, consider using
//! @c asmjit_cast<> instead of @c reinterpret_cast<>.
template<typename T, typename Z>
static ASMJIT_INLINE T asmjit_cast(Z* p) { return (T)p; }

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_GLOBALS_H
