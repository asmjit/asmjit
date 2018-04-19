// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_GLOBALS_H
#define _ASMJIT_CORE_GLOBALS_H

// [Dependencies]
#include "../core/build.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::Globals]
// ============================================================================

namespace Globals {

// ============================================================================
// [asmjit::Globals::<global>]
// ============================================================================

//! Host memory allocator overhead.
constexpr uint32_t kMemAllocOverhead = uint32_t(sizeof(intptr_t) * 4);

//! Host memory allocator alignment.
constexpr uint32_t kMemAllocAlignment = 8;

//! Aggressive growing strategy threshold.
constexpr uint32_t kAllocThreshold = 8192 * 1024;

//! Maximum height of RB-Tree is:
//!
//!   `2 * log2(n + 1)`.
//!
//! Size of RB node is at least two pointers (without data),
//! so a theoretical architecture limit would be:
//!
//!   `2 * log2(addressableMemorySize / sizeof(Node) + 1)`
//!
//! Which yields 30 on 32-bit arch and 61 on 64-bit arch.
//! The final value was adjusted by +1 for safety reasons.
constexpr uint32_t kMaxTreeHeight = (ASMJIT_ARCH_BITS == 32 ? 30 : 61) + 1;

//! Maximum number of operands per a single instruction.
constexpr uint32_t kMaxOpCount = 6;

// TODO: Use this one.
constexpr uint32_t kMaxFuncArgs = 16;

//! Maximum number of physical registers AsmJit can use per register group.
constexpr uint32_t kMaxPhysRegs = 32;

//! Maximum alignment.
constexpr uint32_t kMaxAlignment = 64;

//! Maximum label or symbol size in bytes.
constexpr uint32_t kMaxLabelNameSize = 2048;

//! Maximum size of comment.
constexpr uint32_t kMaxCommentSize = 1024;

//! Returned by `indexOf()` and similar when working with containers that use 32-bit index/size.
constexpr uint32_t kNotFound = std::numeric_limits<uint32_t>::max();

//! The size of the string is not known, but the string is null terminated.
constexpr size_t kNullTerminated = std::numeric_limits<size_t>::max();

//! Invalid base address.
constexpr uint64_t kNoBaseAddress = ~uint64_t(0);

// ============================================================================
// [asmjit::Globals::ByteOrder]
// ============================================================================

//! Byte order.
enum ByteOrder : uint32_t {
  kByteOrderLE      = 0,
  kByteOrderBE      = 1,
  kByteOrderNative  = ASMJIT_ARCH_LE ? kByteOrderLE : kByteOrderBE,
  kByteOrderSwapped = ASMJIT_ARCH_LE ? kByteOrderBE : kByteOrderLE
};

// ============================================================================
// [asmjit::Globals::ResetPolicy]
// ============================================================================

//! Reset policy used by most `reset()` functions.
enum ResetPolicy : uint32_t {
  //! Soft reset, doesn't deallocate memory (default).
  kResetSoft = 0,
  //! Hard reset, releases all memory used, if any.
  kResetHard = 1
};

// ============================================================================
// [asmjit::Globals::Link]
// ============================================================================

enum Link : uint32_t {
  kLinkLeft  = 0,
  kLinkRight = 1,

  kLinkPrev  = 0,
  kLinkNext  = 1,

  kLinkFirst = 0,
  kLinkLast  = 1,

  kLinkCount = 2
};

struct Init_ {};
struct NoInit_ {};

constexpr Init_ Init {};
constexpr NoInit_ NoInit {};

} // Globals namespace

// ============================================================================
// [asmjit::Error]
// ============================================================================

//! AsmJit error type (uint32_t).
typedef uint32_t Error;

//! AsmJit error codes.
enum ErrorCode : uint32_t {
  //! No error (success).
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
  //! never be ignored.
  kErrorInvalidState,

  //! Invalid or incompatible architecture.
  kErrorInvalidArch,

  //! The object is not initialized.
  kErrorNotInitialized,
  //! The object is already initialized.
  kErrorAlreadyInitialized,

  //! Built-in feature was disabled at compile time and it's not available.
  kErrorFeatureNotEnabled,

  //! No code generated.
  //!
  //! Returned by runtime if the `CodeHolder` contains no code.
  kErrorNoCodeGenerated,
  //! Code generated is larger than allowed.
  kErrorCodeTooLarge,

  //! Invalid directive.
  kErrorInvalidDirective,
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
  //! Invalid register group.
  kErrorInvalidRegGroup,
  //! Invalid register's physical id.
  kErrorInvalidPhysId,
  //! Invalid register's virtual id.
  kErrorInvalidVirtId,
  //! Invalid prefix combination.
  kErrorInvalidPrefixCombination,
  //! Invalid LOCK prefix.
  kErrorInvalidLockPrefix,
  //! Invalid XACQUIRE prefix.
  kErrorInvalidXAcquirePrefix,
  //! Invalid XRELEASE prefix.
  kErrorInvalidXReleasePrefix,
  //! Invalid REP prefix.
  kErrorInvalidRepPrefix,
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
  //! Invalid segment (X86).
  kErrorInvalidSegment,

  //! Invalid immediate (out of bounds on X86 and invalid pattern on ARM).
  kErrorInvalidImmediate,

  //! Invalid operand size.
  kErrorInvalidOperandSize,
  //! Ambiguous operand size (memory has zero size while it's required to determine the operation type.
  kErrorAmbiguousOperandSize,
  //! Mismatching operand size (size of multiple operands doesn't match the operation size).
  kErrorOperandSizeMismatch,

  //! Invalid option.
  kErrorInvalidOption,
  //! Option already defined.
  kErrorOptionAlreadyDefined,

  //! Invalid TypeId.
  kErrorInvalidTypeId,
  //! Invalid use of a 8-bit GPB-HIGH register.
  kErrorInvalidUseOfGpbHi,
  //! Invalid use of a 64-bit GPQ register in 32-bit mode.
  kErrorInvalidUseOfGpq,
  //! Invalid use of an 80-bit float (Type::kIdF80).
  kErrorInvalidUseOfF80,
  //! Some registers in the instruction muse be consecutive (some ARM and AVX512 neural-net instructions).
  kErrorNotConsecutiveRegs,

  //! AsmJit requires a physical register, but no one is available.
  kErrorNoMorePhysRegs,
  //! A variable has been assigned more than once to a function argument (BaseCompiler).
  kErrorOverlappedRegs,
  //! Invalid register to hold stack arguments offset.
  kErrorOverlappingStackRegWithRegArg,

  //! Count of AsmJit error codes.
  kErrorCount
};

// ============================================================================
// [asmjit::PointerCast]
// ============================================================================

namespace AsmJitInternal {
  //! Cast designed to cast between function and void* pointers.
  template<typename Dst, typename Src>
  static inline Dst ptr_cast(Src p) noexcept { return (Dst)p; }
}

template<typename Func>
static inline Func ptr_as_func(void* func) noexcept { return AsmJitInternal::ptr_cast<Func, void*>(func); }
template<typename Func>
static inline void* func_as_ptr(Func func) noexcept { return AsmJitInternal::ptr_cast<void*, Func>(func); }

// ============================================================================
// [asmjit::DebugUtils]
// ============================================================================

namespace DebugUtils {

//! Returns the error `err` passed.
//!
//! Provided for debugging purposes. Putting a breakpoint inside `errored` can
//! help with tracing the origin of any error reported / returned by AsmJit.
static constexpr Error errored(Error err) noexcept { return err; }

//! Get a printable version of `asmjit::Error` code.
ASMJIT_API const char* errorAsString(Error err) noexcept;

//! Called to output debugging message(s).
ASMJIT_API void debugOutput(const char* str) noexcept;

//! Called on assertion failure.
//!
//! \param file Source file name where it happened.
//! \param line Line in the source file.
//! \param msg Message to display.
//!
//! If you have problems with assertions put a breakpoint at assertionFailed()
//! function (asmjit/core/globals.cpp) and check the call stack to locate the
//! failing code.
ASMJIT_API void ASMJIT_NORETURN assertionFailed(const char* file, int line, const char* msg) noexcept;

#if defined(ASMJIT_BUILD_DEBUG)
  #define ASMJIT_ASSERT(EXP)                                           \
    do {                                                               \
      if (ASMJIT_LIKELY(EXP))                                          \
        break;                                                         \
      ::asmjit::DebugUtils::assertionFailed(__FILE__, __LINE__, #EXP); \
    } while (0)
#else
  #define ASMJIT_ASSERT(EXP) ((void)0)
#endif

//! \internal
//!
//! Used by AsmJit to propagate a possible `Error` produced by `...` to the caller.
#define ASMJIT_PROPAGATE(...)               \
  do {                                      \
    ::asmjit::Error _err = __VA_ARGS__;     \
    if (ASMJIT_UNLIKELY(_err))              \
      return _err;                          \
  } while (0)

} // DebugUtils namespace

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_GLOBALS_H
