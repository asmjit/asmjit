// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86COMPILERFUNC_P_H
#define _ASMJIT_X86_X86COMPILERFUNC_P_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/compilerfunc.h"
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::TypeId]
// ============================================================================

#if !defined(ASMJIT_DOCGEN)
ASMJIT_TYPE_ID(X86MmReg, kX86VarTypeMm);
ASMJIT_TYPE_ID(X86MmVar, kX86VarTypeMm);
ASMJIT_TYPE_ID(X86XmmReg, kX86VarTypeXmm);
ASMJIT_TYPE_ID(X86XmmVar, kX86VarTypeXmm);
ASMJIT_TYPE_ID(X86YmmReg, kX86VarTypeYmm);
ASMJIT_TYPE_ID(X86YmmVar, kX86VarTypeYmm);
ASMJIT_TYPE_ID(X86ZmmReg, kX86VarTypeZmm);
ASMJIT_TYPE_ID(X86ZmmVar, kX86VarTypeZmm);
#endif // !ASMJIT_DOCGEN

// ============================================================================
// [asmjit::X86FuncDecl]
// ============================================================================

//! X86 function, including calling convention, arguments and their
//! register indices or stack positions.
struct X86FuncDecl : public FuncDecl {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `X86FuncDecl` instance.
  ASMJIT_INLINE X86FuncDecl() { reset(); }

  // --------------------------------------------------------------------------
  // [Accessors - X86]
  // --------------------------------------------------------------------------

  //! Get used registers mask for the given register class `rc`.
  //!
  //! \note The result depends on the function calling convention AND the
  //! function prototype. Returned mask contains only registers actually used
  //! to pass function arguments.
  ASMJIT_INLINE uint32_t getUsed(uint32_t rc) const { return _used.get(rc); }

  //! Get passed registers mask for the given register class `rc`.
  //!
  //! \note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE uint32_t getPassed(uint32_t rc) const { return _passed.get(rc); }

  //! Get preserved registers mask for the given register class `rc`.
  //!
  //! \note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE uint32_t getPreserved(uint32_t rc) const { return _preserved.get(rc); }

  //! Get ther order of passed registers (Gp).
  //!
  //! \note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE const uint8_t* getPassedOrderGp() const { return _passedOrderGp; }

  //! Get ther order of passed registers (Xmm/Ymm/Zmm).
  //!
  //! \note The result depends on the function calling convention used; the
  //! prototype of the function doesn't affect the mask returned.
  ASMJIT_INLINE const uint8_t* getPassedOrderXyz() const { return _passedOrderXyz; }

  // --------------------------------------------------------------------------
  // [SetPrototype]
  // --------------------------------------------------------------------------

  //! Set function prototype.
  //!
  //! This will set function calling convention and setup arguments variables.
  //!
  //! \note This function will allocate variables, it can be called only once.
  ASMJIT_API Error setPrototype(const FuncPrototype& p);

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_API void reset();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Used registers.
  X86RegMask _used;
  //! Passed registers (defined by the calling convention).
  X86RegMask _passed;
  //! Preserved registers (defined by the calling convention).
  X86RegMask _preserved;

  //! Order of registers used to pass Gp function arguments.
  uint8_t _passedOrderGp[8];
  //! Order of registers used to pass Xmm function arguments.
  uint8_t _passedOrderXyz[8];
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_X86_X86COMPILERFUNC_P_H
