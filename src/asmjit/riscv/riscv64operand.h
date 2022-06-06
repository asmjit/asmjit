// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_RISCV_RISCV64OPERAND_H_INCLUDED
#define ASMJIT_RISCV_RISCV64OPERAND_H_INCLUDED

#include "../riscv/riscvoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

using riscv::GpX64;
using riscv::Pc;

// TODO: [RISC-V] doc
#ifndef _DOXYGEN
namespace regs {
#endif

using namespace ::asmjit::riscv::regs;

static constexpr GpX64 x0 = GpX64(0);
static constexpr GpX64 x1 = GpX64(1);
static constexpr GpX64 x2 = GpX64(2);
static constexpr GpX64 x3 = GpX64(3);
static constexpr GpX64 x4 = GpX64(4);
static constexpr GpX64 x5 = GpX64(5);
static constexpr GpX64 x6 = GpX64(6);
static constexpr GpX64 x7 = GpX64(7);
static constexpr GpX64 x8 = GpX64(8);
static constexpr GpX64 x9 = GpX64(9);
static constexpr GpX64 x10 = GpX64(10);
static constexpr GpX64 x11 = GpX64(11);
static constexpr GpX64 x12 = GpX64(12);
static constexpr GpX64 x13 = GpX64(13);
static constexpr GpX64 x14 = GpX64(14);
static constexpr GpX64 x15 = GpX64(15);
static constexpr GpX64 x16 = GpX64(16);
static constexpr GpX64 x17 = GpX64(17);
static constexpr GpX64 x18 = GpX64(18);
static constexpr GpX64 x19 = GpX64(19);
static constexpr GpX64 x20 = GpX64(20);
static constexpr GpX64 x21 = GpX64(21);
static constexpr GpX64 x22 = GpX64(22);
static constexpr GpX64 x23 = GpX64(23);
static constexpr GpX64 x24 = GpX64(24);
static constexpr GpX64 x25 = GpX64(25);
static constexpr GpX64 x26 = GpX64(26);
static constexpr GpX64 x27 = GpX64(27);
static constexpr GpX64 x28 = GpX64(28);
static constexpr GpX64 x29 = GpX64(29);
static constexpr GpX64 x30 = GpX64(30);
static constexpr GpX64 x31 = GpX64(31);

static constexpr Pc pc = Pc(0);

#ifndef _DOXYGEN
} // {regs}

// Make `riscv64::regs` accessible through `riscv64` namespace as well.
using namespace regs;
#endif

// TODO: [RISC-V]

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_RISCV_RISCV64OPERAND_H_INCLUDED
