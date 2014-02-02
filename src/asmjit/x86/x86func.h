// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86FUNC_H
#define _ASMJIT_X86_X86FUNC_H

// [Dependencies - AsmJit]
#include "../base/defs.h"
#include "../base/func.h"
#include "../x86/x86defs.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [asmjit::x86x64::kFuncConv]
// ============================================================================

//! @brief X86 function calling conventions.
//!
//! Calling convention is scheme how function arguments are passed into
//! function and how functions returns values. In assembler programming
//! it's needed to always comply with function calling conventions, because
//! even small inconsistency can cause undefined behavior or crash.
//!
//! List of calling conventions for 32-bit x86 mode:
//! - @c kFuncConvCDecl - Calling convention for C runtime.
//! - @c kFuncConvStdCall - Calling convention for WinAPI functions.
//! - @c kFuncConvMsThisCall - Calling convention for C++ members under
//!      Windows (produced by MSVC and all MSVC compatible compilers).
//! - @c kFuncConvMsFastCall - Fastest calling convention that can be used
//!      by MSVC compiler.
//! - @c kFuncConv_BORNANDFASTCALL - Borland fastcall convention.
//! - @c kFuncConvGccFastCall - GCC fastcall convention (2 register arguments).
//! - @c kFuncConvGccRegParm1 - GCC regparm(1) convention.
//! - @c kFuncConvGccRegParm2 - GCC regparm(2) convention.
//! - @c kFuncConvGccRegParm3 - GCC regparm(3) convention.
//!
//! List of calling conventions for 64-bit x86 mode (x64):
//! - @c kFuncConvX64W - Windows 64-bit calling convention (WIN64 ABI).
//! - @c kFuncConvX64U - Unix 64-bit calling convention (AMD64 ABI).
//!
//! There is also @c kFuncConvHost that is defined to fit best to your
//! compiler.
//!
//! These types are used together with @c asmjit::Compiler::addFunc()
//! method.
ASMJIT_ENUM(kFuncConv) {
  // --------------------------------------------------------------------------
  // [X64]
  // --------------------------------------------------------------------------

  //! @brief X64 calling convention for Windows platform (WIN64 ABI).
  //!
  //! For first four arguments are used these registers:
  //! - 1. 32/64-bit integer or floating point argument - rcx/xmm0
  //! - 2. 32/64-bit integer or floating point argument - rdx/xmm1
  //! - 3. 32/64-bit integer or floating point argument - r8/xmm2
  //! - 4. 32/64-bit integer or floating point argument - r9/xmm3
  //!
  //! Note first four arguments here means arguments at positions from 1 to 4
  //! (included). For example if second argument is not passed in register then
  //! rdx/xmm1 register is unused.
  //!
  //! All other arguments are pushed on the stack in right-to-left direction.
  //! Stack is aligned by 16 bytes. There is 32-byte shadow space on the stack
  //! that can be used to save up to four 64-bit registers (probably designed to
  //! be used to save first four arguments passed in registers).
  //!
  //! Arguments direction:
  //! - Right to Left (except for first 4 parameters that's in registers)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - RAX register.
  //! - Floating points - XMM0 register.
  //!
  //! Stack is always aligned by 16 bytes.
  //!
  //! More information about this calling convention can be found on MSDN:
  //! http://msdn.microsoft.com/en-us/library/9b372w95.aspx .
  kFuncConvX64W = 1,

  //! @brief X64 calling convention for Unix platforms (AMD64 ABI).
  //!
  //! First six 32 or 64-bit integer arguments are passed in rdi, rsi, rdx,
  //! rcx, r8, r9 registers. First eight floating point or Xmm arguments
  //! are passed in xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7 registers.
  //! This means that in registers can be transferred up to 14 arguments total.
  //!
  //! There is also RED ZONE below the stack pointer that can be used for
  //! temporary storage. The red zone is the space from [rsp-128] to [rsp-8].
  //!
  //! Arguments direction:
  //! - Right to Left (Except for arguments passed in registers).
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - RAX register.
  //! - Floating points - XMM0 register.
  //!
  //! Stack is always aligned by 16 bytes.
  kFuncConvX64U = 2,

  // --------------------------------------------------------------------------
  // [X86]
  // --------------------------------------------------------------------------

  //! @brief Cdecl calling convention (used by C runtime).
  //!
  //! Compatible across MSVC and GCC.
  //!
  //! Arguments direction:
  //! - Right to Left
  //!
  //! Stack is cleaned by:
  //! - Caller.
  kFuncConvCDecl = 3,

  //! @brief Stdcall calling convention (used by WinAPI).
  //!
  //! Compatible across MSVC and GCC.
  //!
  //! Arguments direction:
  //! - Right to Left
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  kFuncConvStdCall = 4,

  //! @brief MSVC specific calling convention used by MSVC/Intel compilers
  //! for struct/class methods.
  //!
  //! This is MSVC (and Intel) only calling convention used in Windows
  //! world for C++ class methods. Implicit 'this' pointer is stored in
  //! ECX register instead of storing it on the stack.
  //!
  //! Arguments direction:
  //! - Right to Left (except this pointer in ECX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  //!
  //! C++ class methods that have variable count of arguments uses different
  //! calling convention called cdecl.
  //!
  //! @note This calling convention is always used by MSVC for class methods,
  //! it's implicit and there is no way how to override it.
  kFuncConvMsThisCall = 5,

  //! @brief MSVC specific fastcall.
  //!
  //! Two first parameters (evaluated from left-to-right) are in ECX:EDX
  //! registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first two integer arguments in ECX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  //!
  //! @note This calling convention differs to GCC one in stack cleaning
  //! mechanism.
  kFuncConvMsFastCall = 6,

  //! @brief Borland specific fastcall with 2 parameters in registers.
  //!
  //! Two first parameters (evaluated from left-to-right) are in ECX:EDX
  //! registers, all others on the stack in left-to-right order.
  //!
  //! Arguments direction:
  //! - Left to Right (except to first two integer arguments in ECX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  //!
  //! @note Arguments on the stack are in left-to-right order that differs
  //! to other fastcall conventions used in different compilers.
  kFuncConvBorlandFastCall = 7,

  //! @brief GCC specific fastcall convention.
  //!
  //! Two first parameters (evaluated from left-to-right) are in ECX:EDX
  //! registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first two integer arguments in ECX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Callee.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  //!
  //! @note This calling convention should be compatible with
  //! @c kFuncConvMsFastCall.
  kFuncConvGccFastCall = 8,

  //! @brief GCC specific regparm(1) convention.
  //!
  //! The first parameter (evaluated from left-to-right) is in EAX register,
  //! all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first one integer argument in EAX)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  kFuncConvGccRegParm1 = 9,

  //! @brief GCC specific regparm(2) convention.
  //!
  //! Two first parameters (evaluated from left-to-right) are in EAX:EDX
  //! registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first two integer arguments in EAX:EDX)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  kFuncConvGccRegParm2 = 10,

  //! @brief GCC specific fastcall with 3 parameters in registers.
  //!
  //! Three first parameters (evaluated from left-to-right) are in
  //! EAX:EDX:ECX registers, all others on the stack in right-to-left order.
  //!
  //! Arguments direction:
  //! - Right to Left (except to first three integer arguments in EAX:EDX:ECX)
  //!
  //! Stack is cleaned by:
  //! - Caller.
  //!
  //! Return value:
  //! - Integer types - EAX:EDX registers.
  //! - Floating points - fp0 register.
  kFuncConvGccRegParm3 = 11,

  //! @internal
  //!
  //! @brief Count of function calling conventions.
  _kFuncConvCount = 12,

  // --------------------------------------------------------------------------
  // [Host]
  // --------------------------------------------------------------------------

  //! @def kFuncConvHost
  //! @brief Default calling convention for current platform / operating system.

  //! @def kFuncConvHostCDecl
  //! @brief Default C calling convention based on current compiler's settings.

  //! @def kFuncConvHostStdCall
  //! @brief Compatibility for __stdcall calling convention.
  //!
  //! @note This enumeration is always set to a value which is compatible with
  //! current compilers __stdcall calling convention. In 64-bit mode the value
  //! is compatible with @ref kFuncConvX64W or @ref kFuncConvX64U.

  //! @def kFuncConvHostFastCall
  //! @brief Compatibility for __fastcall calling convention.
  //!
  //! @note This enumeration is always set to a value which is compatible with
  //! current compilers __fastcall calling convention. In 64-bit mode the value
  //! is compatible with @ref kFuncConvX64W or @ref kFuncConvX64U.

#if defined(ASMJIT_HOST_X86)

  kFuncConvHost = kFuncConvCDecl,
  kFuncConvHostCDecl = kFuncConvCDecl,
  kFuncConvHostStdCall = kFuncConvStdCall,

# if defined(_MSC_VER)
  kFuncConvHostFastCall = kFuncConvMsFastCall
# elif defined(__GNUC__)
  kFuncConvHostFastCall = kFuncConvGccFastCall
# elif defined(__BORLANDC__)
  kFuncConvHostFastCall = kFuncConvBorlandFastCall
# else
#  error "asmjit/x86/x86func.h - asmjit::kFuncConvHostFastCall not supported."
# endif

#else

# if defined(ASMJIT_OS_WINDOWS)
  kFuncConvHost = kFuncConvX64W,
# else
  kFuncConvHost = kFuncConvX64U,
# endif

  kFuncConvHostCDecl = kFuncConvHost,
  kFuncConvHostStdCall = kFuncConvHost,
  kFuncConvHostFastCall = kFuncConvHost

#endif // ASMJIT_HOST
};

// ============================================================================
// [asmjit::x86x64::kFuncHint]
// ============================================================================

//! @brief X86 function hints.
ASMJIT_ENUM(kFuncHint) {
  //! @brief Use push/pop sequences instead of mov sequences in function prolog
  //! and epilog.
  kFuncHintPushPop = 16,
  //! @brief Add emms instruction to the function epilog.
  kFuncHintEmms = 17,
  //! @brief Add sfence instruction to the function epilog.
  kFuncHintSFence = 18,
  //! @brief Add lfence instruction to the function epilog.
  kFuncHintLFence = 19
};

// ============================================================================
// [asmjit::x86x64::kFuncFlags]
// ============================================================================

//! @brief X86 function flags.
ASMJIT_ENUM(kFuncFlags) {
  //! @brief Whether to emit register load/save sequence using push/pop pairs.
  kFuncFlagPushPop = 0x00010000,

  //! @brief Whether to emit "enter" instead of three instructions in case
  //! that the function is not naked or misaligned.
  kFuncFlagEnter = 0x00020000,

  //! @brief Whether to emit "leave" instead of two instructions in case
  //! that the function is not naked or misaligned.
  kFuncFlagLeave = 0x00040000,

  //! @brief Whether it's required to move arguments to a new stack location,
  //! because of manual aligning.
  kFuncFlagMoveArgs = 0x00080000,

  //! @brief Whether to emit EMMS instruction in epilog (auto-detected).
  kFuncFlagEmms = 0x01000000,

  //! @brief Whether to emit SFence instruction in epilog (auto-detected).
  //!
  //! @note @ref kFuncFlagSFence and @ref kFuncFlagLFence
  //! combination will result in emitting mfence.
  kFuncFlagSFence = 0x02000000,

  //! @brief Whether to emit LFence instruction in epilog (auto-detected).
  //!
  //! @note @ref kFuncFlagSFence and @ref kFuncFlagLFence
  //! combination will result in emitting mfence.
  kFuncFlagLFence = 0x04000000
};

// ============================================================================
// [asmjit::x86x64::x86GetArchFromCConv]
// ============================================================================

static ASMJIT_INLINE uint32_t x86GetArchFromCConv(uint32_t conv) {
  return IntUtil::inInterval<uint32_t>(conv, kFuncConvX64W, kFuncConvX64U) ? kArchX64 : kArchX86;
}

// ============================================================================
// [asmjit::x86x64::X86X64FuncDecl]
// ============================================================================

//! @brief X86 function, including calling convention, arguments and their
//! register indices or stack positions.
struct X86X64FuncDecl : public FuncDecl {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref X86X64FuncDecl instance.
  ASMJIT_INLINE X86X64FuncDecl() { reset(); }

  // --------------------------------------------------------------------------
  // [Accessors - X86]
  // --------------------------------------------------------------------------

  //! @brief Get used registers (mask).
  //!
  //! @note The result depends on the function calling convention AND the
  //! function prototype. Returned mask contains only registers actually used
  //! to pass function arguments.
  ASMJIT_INLINE uint32_t getUsed(uint32_t c) const { return _used.get(c); }

  //! @brief Get passed registers (mask).
  //!
  //! @note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE uint32_t getPassed(uint32_t c) const { return _passed.get(c); }

  //! @brief Get preserved registers (mask).
  //!
  //! @note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE uint32_t getPreserved(uint32_t c) const { return _preserved.get(c); }

  //! @brief Get ther order of passed registers (Gp).
  //!
  //! @note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE const uint8_t* getPassedOrderGp() const { return _passedOrderGp; }

  //! @brief Get ther order of passed registers (Xmm).
  //!
  //! @note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE const uint8_t* getPassedOrderXmm() const { return _passedOrderXmm; }

  // --------------------------------------------------------------------------
  // [FindArgByReg]
  // --------------------------------------------------------------------------

  //! @brief Find argument ID by register class and index.
  ASMJIT_API uint32_t findArgByReg(uint32_t rClass, uint32_t rIndex) const;

  // --------------------------------------------------------------------------
  // [SetPrototype]
  // --------------------------------------------------------------------------

  //! @brief Set function prototype.
  //!
  //! This will set function calling convention and setup arguments variables.
  //!
  //! @note This function will allocate variables, it can be called only once.
  ASMJIT_API Error setPrototype(uint32_t conv, const FuncPrototype& p);

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_API void reset();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Used registers .
  RegMask _used;

  //! @brief Passed registers (defined by the calling convention).
  RegMask _passed;
  //! @brief Preserved registers (defined by the calling convention).
  RegMask _preserved;

  //! @brief Order of registers defined to pass function arguments (Gp).
  uint8_t _passedOrderGp[kFuncArgCount];
  //! @brief Order of registers defined to pass function arguments (Xmm).
  uint8_t _passedOrderXmm[kFuncArgCount];
};

//! @}

} // x86x64 namespace
} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86FUNC_H
