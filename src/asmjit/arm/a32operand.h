// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32OPERAND_H_INCLUDED
#define ASMJIT_ARM_A32OPERAND_H_INCLUDED

#include "../arm/armoperand.h"
#include "../arm/a32globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \addtogroup asmjit_a32
//! \{

using arm::Reg;
using arm::Mem;
using arm::Gp;
using arm::GpW;
using GpR = GpW;

using arm::Vec;
using arm::VecB;
using arm::VecH;
using arm::VecS;
using arm::VecD;
using arm::VecV;

//! Constructs a `LSL #value` shift of a register (logical shift left).
static constexpr inline Gp lsl(const Gp& reg) noexcept { return Gp(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kLSL)), reg.id()); }
//! Constructs a `LSR #value` shift of a register (logical shift right).
static constexpr inline Gp lsr(const Gp& reg) noexcept { return Gp(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kLSR)), reg.id()); }
//! Constructs a `ASR #value` shift of a register (arithmetic shift right).
static constexpr inline Gp asr(const Gp& reg) noexcept { return Gp(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kASR)), reg.id()); }
//! Constructs a `ROR #value` shift of a register (rotate right).
static constexpr inline Gp ror(const Gp& reg) noexcept { return Gp(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kROR)), reg.id()); }
//! Constructs a `sop #value` shift of a register.
static constexpr inline Gp shift(const Gp& reg, ShiftOp sop) noexcept { return Gp(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(sop)), reg.id()); }

//! Constructs a `LSL #value` shift of a register (logical shift left).
static constexpr inline GpR lsl(const GpR& reg) noexcept { return GpR(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kLSL)), reg.id()); }
//! Constructs a `LSR #value` shift of a register (logical shift right).
static constexpr inline GpR lsr(const GpR& reg) noexcept { return GpR(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kLSR)), reg.id()); }
//! Constructs a `ASR #value` shift of a register (arithmetic shift right).
static constexpr inline GpR asr(const GpR& reg) noexcept { return GpR(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kASR)), reg.id()); }
//! Constructs a `ROR #value` shift of a register (rotate right).
static constexpr inline GpR ror(const GpR& reg) noexcept { return GpR(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(ShiftOp::kROR)), reg.id()); }
//! Constructs a `sop #value` shift of a register.
static constexpr inline GpR shift(const GpR& reg, ShiftOp sop) noexcept { return GpR(reg.signature().replacedValue<OperandSignature::kPredicateMask>(uint32_t(sop)), reg.id()); }

#ifndef _DOXYGEN
namespace regs {
#endif

using namespace ::asmjit::arm::regs;

static constexpr GpW r0 = GpW(0);
static constexpr GpW r1 = GpW(1);
static constexpr GpW r2 = GpW(2);
static constexpr GpW r3 = GpW(3);
static constexpr GpW r4 = GpW(4);
static constexpr GpW r5 = GpW(5);
static constexpr GpW r6 = GpW(6);
static constexpr GpW r7 = GpW(7);
static constexpr GpW r8 = GpW(8);
static constexpr GpW r9 = GpW(9);
static constexpr GpW r10 = GpW(10);
static constexpr GpW r11 = GpW(11);
static constexpr GpW r12 = GpW(12);
static constexpr GpW r13 = GpW(13);
static constexpr GpW r14 = GpW(14);
static constexpr GpW r15 = GpW(15);

static constexpr VecS s0 = VecS(0);
static constexpr VecS s1 = VecS(1);
static constexpr VecS s2 = VecS(2);
static constexpr VecS s3 = VecS(3);
static constexpr VecS s4 = VecS(4);
static constexpr VecS s5 = VecS(5);
static constexpr VecS s6 = VecS(6);
static constexpr VecS s7 = VecS(7);
static constexpr VecS s8 = VecS(8);
static constexpr VecS s9 = VecS(9);
static constexpr VecS s10 = VecS(10);
static constexpr VecS s11 = VecS(11);
static constexpr VecS s12 = VecS(12);
static constexpr VecS s13 = VecS(13);
static constexpr VecS s14 = VecS(14);
static constexpr VecS s15 = VecS(15);
static constexpr VecS s16 = VecS(16);
static constexpr VecS s17 = VecS(17);
static constexpr VecS s18 = VecS(18);
static constexpr VecS s19 = VecS(19);
static constexpr VecS s20 = VecS(20);
static constexpr VecS s21 = VecS(21);
static constexpr VecS s22 = VecS(22);
static constexpr VecS s23 = VecS(23);
static constexpr VecS s24 = VecS(24);
static constexpr VecS s25 = VecS(25);
static constexpr VecS s26 = VecS(26);
static constexpr VecS s27 = VecS(27);
static constexpr VecS s28 = VecS(28);
static constexpr VecS s29 = VecS(29);
static constexpr VecS s30 = VecS(30);
static constexpr VecS s31 = VecS(31);

static constexpr VecD d0 = VecD(0);
static constexpr VecD d1 = VecD(1);
static constexpr VecD d2 = VecD(2);
static constexpr VecD d3 = VecD(3);
static constexpr VecD d4 = VecD(4);
static constexpr VecD d5 = VecD(5);
static constexpr VecD d6 = VecD(6);
static constexpr VecD d7 = VecD(7);
static constexpr VecD d8 = VecD(8);
static constexpr VecD d9 = VecD(9);
static constexpr VecD d10 = VecD(10);
static constexpr VecD d11 = VecD(11);
static constexpr VecD d12 = VecD(12);
static constexpr VecD d13 = VecD(13);
static constexpr VecD d14 = VecD(14);
static constexpr VecD d15 = VecD(15);
static constexpr VecD d16 = VecD(16);
static constexpr VecD d17 = VecD(17);
static constexpr VecD d18 = VecD(18);
static constexpr VecD d19 = VecD(19);
static constexpr VecD d20 = VecD(20);
static constexpr VecD d21 = VecD(21);
static constexpr VecD d22 = VecD(22);
static constexpr VecD d23 = VecD(23);
static constexpr VecD d24 = VecD(24);
static constexpr VecD d25 = VecD(25);
static constexpr VecD d26 = VecD(26);
static constexpr VecD d27 = VecD(27);
static constexpr VecD d28 = VecD(28);
static constexpr VecD d29 = VecD(29);
static constexpr VecD d30 = VecD(30);
static constexpr VecD d31 = VecD(31);

static constexpr VecV q0 = VecV(0);
static constexpr VecV q1 = VecV(1);
static constexpr VecV q2 = VecV(2);
static constexpr VecV q3 = VecV(3);
static constexpr VecV q4 = VecV(4);
static constexpr VecV q5 = VecV(5);
static constexpr VecV q6 = VecV(6);
static constexpr VecV q7 = VecV(7);
static constexpr VecV q8 = VecV(8);
static constexpr VecV q9 = VecV(9);
static constexpr VecV q10 = VecV(10);
static constexpr VecV q11 = VecV(11);
static constexpr VecV q12 = VecV(12);
static constexpr VecV q13 = VecV(13);
static constexpr VecV q14 = VecV(14);
static constexpr VecV q15 = VecV(15);

static constexpr VecV v0 = VecV(0);
static constexpr VecV v1 = VecV(1);
static constexpr VecV v2 = VecV(2);
static constexpr VecV v3 = VecV(3);
static constexpr VecV v4 = VecV(4);
static constexpr VecV v5 = VecV(5);
static constexpr VecV v6 = VecV(6);
static constexpr VecV v7 = VecV(7);
static constexpr VecV v8 = VecV(8);
static constexpr VecV v9 = VecV(9);
static constexpr VecV v10 = VecV(10);
static constexpr VecV v11 = VecV(11);
static constexpr VecV v12 = VecV(12);
static constexpr VecV v13 = VecV(13);
static constexpr VecV v14 = VecV(14);
static constexpr VecV v15 = VecV(15);

#ifndef _DOXYGEN
} // {regs}

// Make `a32::regs` accessible through `a32` namespace as well.
using namespace regs;
#endif

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32OPERAND_H_INCLUDED
