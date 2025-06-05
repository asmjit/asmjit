// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_A64_H_INCLUDED
#define ASMJIT_A64_H_INCLUDED

//! \addtogroup asmjit_a64
//!
//! ### Emitters
//!
//!   - \ref a64::Assembler - AArch64 assembler (must read, provides examples).
//!   - \ref a64::Builder - AArch64 builder.
//!   - \ref a64::Compiler - AArch64 compiler.
//!   - \ref a64::Emitter - AArch64 emitter (abstract).
//!
//! ### Supported Instructions
//!
//!   - Emitters:
//!     - \ref a64::EmitterExplicitT - Provides all instructions that use explicit operands, provides also utility
//!       functions. The member functions provided are part of all AArch64 emitters.
//!
//!   - Instruction representation:
//!     - \ref a64::Inst::Id - instruction identifiers.
//!
//! ### Register Operands
//!
//!   - \ref arm::Reg - Base class of all AArch32/AArch64 registers.
//!     - \ref a64::Gp - General purpose register (AArch64):
//!       - \ref a64::GpW - 32-bit general purpose register (AArch64).
//!       - \ref a64::GpX - 64-bit general purpose register (AArch64).
//!     - \ref a64::Vec - Vector (SIMD) register:
//!       - \ref a64::VecB - 8-bit SIMD register.
//!       - \ref a64::VecH - 16-bit SIMD register.
//!       - \ref a64::VecS - 32-bit SIMD register.
//!       - \ref a64::VecD - 64-bit SIMD register.
//!       - \ref a64::VecV - 128-bit SIMD register.
//!
//! ### Memory Operands
//!
//!   - \ref arm::Mem - AArch32/AArch64 memory operand that provides support for all ARM addressing features
//!     including base, index, pre/post increment, and ARM-specific shift addressing and index extending.
//!
//! ### Other
//!
//!   - \ref arm::Shift - Shift operation and value.
//!   - \ref arm::Utils - Utilities that can help during code generation for AArch32 and AArch64.

#include "./core.h"
#include "./loongarch/la64assembler.h"
#include "./loongarch/la64builder.h"
#include "./loongarch/la64compiler.h"
#include "./loongarch/la64emitter.h"
#include "./loongarch/la64globals.h"
#include "./loongarch/la64instdb.h"
#include "./loongarch/la64operand.h"
#include "./loongarch/laglobals.h"
#include "./loongarch/laoperand.h"
#include "./loongarch/lautils.h"

#endif // ASMJIT_A64_H_INCLUDED

