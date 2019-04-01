// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_H
#define _ASMJIT_X86_H

//! \defgroup asmjit_x86 X86
//!
//! \brief X86/X64 Backend.

//! \defgroup asmjit_x86_api X86 API
//! \ingroup asmjit_x86
//!
//! \brief X86/X64 API.

//! \defgroup asmjit_x86_db X86 DB
//! \ingroup asmjit_x86
//!
//! \brief X86/X64 instruction database.

//! \cond INTERNAL

//! \defgroup asmjit_x86_ra X86 RA
//! \ingroup asmjit_x86
//!
//! \brief X86/X64 register allocation.

//! \endcond

//! \namespace asmjit::x86
//!
//! X86/X64 API.

#include "./core.h"

#include "./x86/x86assembler.h"
#include "./x86/x86builder.h"
#include "./x86/x86compiler.h"
#include "./x86/x86emitter.h"
#include "./x86/x86features.h"
#include "./x86/x86globals.h"
#include "./x86/x86instdb.h"
#include "./x86/x86operand.h"

#endif // _ASMJIT_X86_H
