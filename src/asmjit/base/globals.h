// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_GLOBALS_H
#define _ASMJIT_BASE_GLOBALS_H

// [Dependencies]
#include "../asmjit_build.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::TypeDefs]
// ============================================================================

//! AsmJit error core (uint32_t).
typedef uint32_t Error;

// ============================================================================
// [asmjit::GlobalDefs]
// ============================================================================

//! Invalid index
//!
//! Invalid index is the last possible index that is never used in practice. In
//! AsmJit it is used exclusively with strings to indicate the the length of the
//! string is not known and has to be determined.
static const size_t kInvalidIndex = ~static_cast<size_t>(0);

//! Invalid base address.
static const uint64_t kNoBaseAddress = ~static_cast<uint64_t>(0);

//! Global constants.
ASMJIT_ENUM(GlobalDefs) {
  //! Invalid instruction.
  kInvalidInst = 0,
  //! Invalid register id.
  kInvalidReg = 0xFF,
  //! Invalid value or id.
  kInvalidValue = 0xFFFFFFFF,

  //! Host memory allocator overhead.
  //!
  //! The overhead is decremented from all zone allocators so the operating
  //! system doesn't have to allocate one extra virtual page to keep tract of
  //! the requested memory block.
  //!
  //! The number is actually a guess.
  kMemAllocOverhead = sizeof(intptr_t) * 4,

  //! Memory grow threshold.
  kMemAllocGrowMax = 8192 * 1024
};

// ============================================================================
// [asmjit::ptr_cast]
// ============================================================================

//! Cast designed to cast between function and void* pointers.
template<typename Dst, typename Src>
ASMJIT_INLINE Dst ptr_cast(Src p) noexcept { return (Dst)p; }

// ============================================================================
// [asmjit::ErrorCode]
// ============================================================================

//! AsmJit error codes.
ASMJIT_ENUM(ErrorCode) {
  //! No error (success).
  //!
  //! This is default state and state you want.
  kErrorOk = 0,

  //! Heap memory allocation failed.
  kErrorNoHeapMemory,

  //! Virtual memory allocation failed.
  kErrorNoVirtualMemory,

  //! Invalid argument.
  kErrorInvalidArgument,

  //! Invalid state.
  //!
  //! If this error is returned it means that either you are doing something
  //! wrong or AsmJit caught itself by doing something wrong. This error should
  //! not be underestimated.
  kErrorInvalidState,

  //! Incompatible architecture.
  kErrorInvalidArch,

  //! The object is not initialized.
  kErrorNotInitialized,
  //! The object is already initialized.
  kErrorAlreadyInitialized,

  //! CodeHolder can't have attached more than one \ref Assembler at a time.
  kErrorSlotOccupied,

  //! No code generated.
  //!
  //! Returned by runtime if the \ref CodeHolder contains no code.
  kErrorNoCodeGenerated,
  //! Code generated is larger than allowed.
  kErrorCodeTooLarge,

  //! Attempt to use uninitialized label.
  kErrorInvalidLabel,
  //! Label index overflow - a single `Assembler` instance can hold more than
  //! 2 billion labels (2147483391 to be exact). If there is an attempt to
  //! create more labels this error is returned.
  kErrorLabelIndexOverflow,
  //! Label is already bound.
  kErrorLabelAlreadyBound,
  //! Label is already defined (named labels).
  kErrorLabelAlreadyDefined,
  //! Label name is too long.
  kErrorLabelNameTooLong,
  //! Label must always be local if it's anonymous (without a name).
  kErrorInvalidLabelName,
  //! Parent id passed to `CodeHolder::newNamedLabelId()` was invalid.
  kErrorInvalidParentLabel,
  //! Parent id specified for a non-local (global) label.
  kErrorNonLocalLabelCantHaveParent,

  //! Relocation index overflow.
  kErrorRelocIndexOverflow,
  //! Invalid relocation entry.
  kErrorInvalidRelocEntry,

  //! Invalid instruction.
  kErrorInvalidInstruction,
  //! Invalid register type.
  kErrorInvalidRegType,
  //! Invalid register's physical id.
  kErrorInvalidPhysId,
  //! Invalid register's virtual id.
  kErrorInvalidVirtId,
  //! Invalid REX prefix.
  kErrorInvalidRexPrefix,
  //! Invalid mask register (not 'k').
  kErrorInvalidKMaskReg,
  //! Invalid {k} use (not supported by the instruction).
  kErrorInvalidKMaskUse,
  //! Invalid {k}{z} use (not supported by the instruction).
  kErrorInvalidKZeroUse,
  //! Invalid broadcast - Currently only related to invalid use of AVX-512 {1tox}.
  kErrorInvalidBroadcast,
  //! Invalid 'embedded-rounding' {er} or 'suppress-all-exceptions' {sae} (AVX-512).
  kErrorInvalidEROrSAE,
  //! Invalid address used (not encodable).
  kErrorInvalidAddress,
  //! Invalid index register used in memory address (not encodable).
  kErrorInvalidAddressIndex,
  //! Invalid address scale (not encodable).
  kErrorInvalidAddressScale,
  //! Invalid use of 64-bit address.
  kErrorInvalidAddress64Bit,
  //! Invalid displacement (not encodable).
  kErrorInvalidDisplacement,
  //! Invalid segment.
  kErrorInvalidSegment,

  //! Mismatching operand size (size of multiple operands doesn't match the operation size).
  kErrorOperandSizeMismatch,
  //! Ambiguous operand size (memory has zero size while it's required to determine the operation type.
  kErrorAmbiguousOperandSize,

  //! Invalid TypeId.
  kErrorInvalidTypeId,
  //! Invalid use of a 8-bit GPB-HIGH register.
  kErrorInvalidUseOfGpbHi,
  //! Invalid use of a 64-bit GPQ register in 32-bit mode.
  kErrorInvalidUseOfGpq,
  //! Invalid use of an 80-bit float (TypeId::kF80).
  kErrorInvalidUseOfF80,

  //! AsmJit requires a physical register, but no one is available.
  kErrorNoMorePhysRegs,
  //! A variable has been assigned more than once to a function argument (CodeCompiler).
  kErrorOverlappingRegArgs,
  //! Invalid register to hold stack arguments offset.
  kErrorOverlappingStackRegWithRegArg,

  //! Count of AsmJit error codes.
  kErrorCount
};

// ============================================================================
// [asmjit::Init / NoInit]
// ============================================================================

#if !defined(ASMJIT_DOCGEN)
struct _Init {};
static const _Init Init = {};

struct _NoInit {};
static const _NoInit NoInit = {};
#endif // !ASMJIT_DOCGEN

// ============================================================================
// [asmjit::DebugUtils]
// ============================================================================

namespace DebugUtils {

//! Returns the error `err` passed.
//!
//! Provided for debugging purposes. Putting a breakpoint inside `errored` can
//! help with tracing the origin of any error reported / returned by AsmJit.
static ASMJIT_INLINE Error errored(Error err) noexcept { return err; }

//! Get a printable version of `asmjit::Error` code.
ASMJIT_API const char* errorAsString(Error err) noexcept;

//! Called in debug build to output a debugging message caused by assertion
//! failure or tracing (if enabled by ASMJIT_TRACE).
ASMJIT_API void debugOutput(const char* str) noexcept;

//! Called in debug build on assertion failure.
//!
//! \param file Source file name where it happened.
//! \param line Line in the source file.
//! \param msg Message to display.
//!
//! If you have problems with assertions put a breakpoint at assertionFailed()
//! function (asmjit/base/globals.cpp) and check the call stack to locate the
//! failing code.
ASMJIT_API void ASMJIT_NORETURN assertionFailed(const char* file, int line, const char* msg) noexcept;

#if defined(ASMJIT_DEBUG)
# define ASMJIT_ASSERT(exp)                                          \
  do {                                                               \
    if (ASMJIT_LIKELY(exp))                                          \
      break;                                                         \
    ::asmjit::DebugUtils::assertionFailed(__FILE__, __LINE__, #exp); \
  } while (0)
# define ASMJIT_NOT_REACHED()                                        \
  do {                                                               \
    ::asmjit::DebugUtils::assertionFailed(__FILE__, __LINE__,        \
      "ASMJIT_NOT_REACHED has been reached");                        \
    ASMJIT_ASSUME(0);                                                \
  } while (0)
#else
# define ASMJIT_ASSERT(exp) ASMJIT_NOP
# define ASMJIT_NOT_REACHED() ASMJIT_ASSUME(0)
#endif // DEBUG

//! \internal
//!
//! Used by AsmJit to propagate a possible `Error` produced by `...` to the caller.
#define ASMJIT_PROPAGATE(...)                                \
  do {                                                       \
    ::asmjit::Error _err = __VA_ARGS__;                      \
    if (ASMJIT_UNLIKELY(_err)) return _err;                  \
  } while (0)

} // DebugUtils namespace

//! \}

} // asmjit namespace

//! \}

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_GLOBALS_H
