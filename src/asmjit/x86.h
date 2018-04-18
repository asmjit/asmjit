// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_H
#define _ASMJIT_X86_H

// [Dependencies]
#include "./core.h"

#include "./x86/x86assembler.h"
#include "./x86/x86builder.h"
#include "./x86/x86compiler.h"
#include "./x86/x86emitter.h"
#include "./x86/x86features.h"
#include "./x86/x86globals.h"
#include "./x86/x86instdb.h"
#include "./x86/x86operand.h"

// DEPRECATED:
ASMJIT_BEGIN_NAMESPACE
typedef ASMJIT_DEPRECATED(x86::Reg  X86Reg  , "Use x86::Reg");
typedef ASMJIT_DEPRECATED(x86::Mem  X86Mem  , "Use x86::Mem");
typedef ASMJIT_DEPRECATED(x86::Gp   X86Gp   , "Use x86::Xmm");
typedef ASMJIT_DEPRECATED(x86::Gpb  X86Gpb  , "Use x86::Gpb");
typedef ASMJIT_DEPRECATED(x86::Gpw  X86Gpw  , "Use x86::Gpw");
typedef ASMJIT_DEPRECATED(x86::Gpd  X86Gpd  , "Use x86::Gpd");
typedef ASMJIT_DEPRECATED(x86::Gpq  X86Gpq  , "Use x86::Gpq");
typedef ASMJIT_DEPRECATED(x86::Vec  X86Vec  , "Use x86::Vec");
typedef ASMJIT_DEPRECATED(x86::Xmm  X86Xmm  , "Use x86::Xmm");
typedef ASMJIT_DEPRECATED(x86::Ymm  X86Ymm  , "Use x86::Ymm");
typedef ASMJIT_DEPRECATED(x86::Zmm  X86Zmm  , "Use x86::Zmm");
typedef ASMJIT_DEPRECATED(x86::Mm   X86Mm   , "Use x86::Mm");
typedef ASMJIT_DEPRECATED(x86::KReg X86KReg , "Use x86::KReg");
typedef ASMJIT_DEPRECATED(x86::SReg X86Seg  , "Use x86::SReg");
typedef ASMJIT_DEPRECATED(x86::CReg X86CReg , "Use x86::CReg");
typedef ASMJIT_DEPRECATED(x86::DReg X86DReg , "Use x86::DReg");
typedef ASMJIT_DEPRECATED(x86::St   X86Fp   , "Use x86::St");
typedef ASMJIT_DEPRECATED(x86::Bnd  X86Bnd  , "Use x86::Bnd");
typedef ASMJIT_DEPRECATED(x86::Rip  X86Rip  , "Use x86::Rip");
ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_X86_H
