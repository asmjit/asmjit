// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_A64OPERAND_H_INCLUDED
#define ASMJIT_LA_A64OPERAND_H_INCLUDED

#include "../loongarch/laoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \addtogroup asmjit_a64
//! \{

class GpW;
class GpX;

class VecB;
class VecH;
class VecS;
class VecD;
class VecV;

//! General purpose register (Loongarch64).
class Gp : public Reg {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Gp, Reg)

  //! Special register id.
  enum Id : uint32_t {
    //! Register that depends on OS, could be used as TLS offset.
    kIdOs = 2,
    //! Frame pointer register id.
    kIdFp = 22,
    //! Link register id.
    kIdLr = 1,
    //! Stack register id.
    kIdSp = 3,
    //! Zero register id.
    kIdZr = 0   //FIXME
  };

  //! Test whether this register is ZR register.
  ASMJIT_INLINE_NODEBUG constexpr bool isZR() const noexcept { return id() == kIdZr; }
  //! Test whether this register is SP register.
  ASMJIT_INLINE_NODEBUG constexpr bool isSP() const noexcept { return id() == kIdSp; }

  //! Cast this register to a 32-bit W register (returns a new operand).
  ASMJIT_INLINE_NODEBUG GpW w() const noexcept;
  //! \overload
  ASMJIT_INLINE_NODEBUG GpW r32() const noexcept;
  //! Cast this register to a 64-bit X register (returns a new operand).
  ASMJIT_INLINE_NODEBUG GpX x() const noexcept;
  //! \overload
  ASMJIT_INLINE_NODEBUG GpX r64() const noexcept;
};

//! 32-bit general purpose W register.
class GpW : public Gp { ASMJIT_DEFINE_FINAL_REG(GpW, Gp, RegTraits<RegType::kLA_GpW>); };
//! 64-bit general purpose X register.
class GpX : public Gp { ASMJIT_DEFINE_FINAL_REG(GpX, Gp, RegTraits<RegType::kLA_GpX>); };

#ifndef _DOXYGEN
ASMJIT_INLINE_NODEBUG GpW Gp::w() const noexcept { return GpW(id()); }
ASMJIT_INLINE_NODEBUG GpX Gp::x() const noexcept { return GpX(id()); }
ASMJIT_INLINE_NODEBUG GpW Gp::r32() const noexcept { return GpW(id()); }
ASMJIT_INLINE_NODEBUG GpX Gp::r64() const noexcept { return GpX(id()); }
#endif

//! Vector element type.
enum class VecElementType : uint32_t {
  //! No element type specified.
  kNone = 0,
  //! Byte elements (B8 or B16).
  kB,
  //! Halfword elements (H4 or H8).
  kH,
  //! Singleword elements (S2 or S4).
  kW,
  //! Doubleword elements (D2).
  kD,
  //! Byte elements grouped by 4 bytes (B4).
  //!

  //! Maximum value of \ref VecElementType
  kMaxValue = kD
};

//! Vector register.
class Vec : public BaseVec {
public:
  ASMJIT_DEFINE_ABSTRACT_REG(Vec, BaseVec)

  //! \cond
  //! Shortcuts.
  enum SignatureReg : uint32_t {
    kSignatureElementB = uint32_t(VecElementType::kB) << kSignatureRegElementTypeShift,
    kSignatureElementH = uint32_t(VecElementType::kH) << kSignatureRegElementTypeShift,
    kSignatureElementS = uint32_t(VecElementType::kW) << kSignatureRegElementTypeShift,
    kSignatureElementD = uint32_t(VecElementType::kD) << kSignatureRegElementTypeShift,
  };
  //! \endcond

  //! Returns whether the register has element type or element index (or both).
  ASMJIT_INLINE_NODEBUG constexpr bool hasElementTypeOrIndex() const noexcept { return _signature.hasField<kSignatureRegElementTypeMask | kSignatureRegElementFlagMask>(); }

  //! Returns whether the vector register has associated a vector element type.
  ASMJIT_INLINE_NODEBUG constexpr bool hasElementType() const noexcept { return _signature.hasField<kSignatureRegElementTypeMask>(); }
  //! Returns vector element type of the register.
  ASMJIT_INLINE_NODEBUG constexpr VecElementType elementType() const noexcept { return VecElementType(_signature.getField<kSignatureRegElementTypeMask>()); }
  //! Sets vector element type of the register to `elementType`.
  ASMJIT_INLINE_NODEBUG void setElementType(VecElementType elementType) noexcept { _signature.setField<kSignatureRegElementTypeMask>(uint32_t(elementType)); }
  //! Resets vector element type to none.
  ASMJIT_INLINE_NODEBUG void resetElementType() noexcept { _signature.setField<kSignatureRegElementTypeMask>(0); }

  ASMJIT_INLINE_NODEBUG constexpr bool isVecB8() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecD>::kSignature | kSignatureElementB); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH4() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecD>::kSignature | kSignatureElementH); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecS2() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecD>::kSignature | kSignatureElementS); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecD1() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecD>::kSignature); }

  ASMJIT_INLINE_NODEBUG constexpr bool isVecB16() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecV>::kSignature | kSignatureElementB); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecH8() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecV>::kSignature | kSignatureElementH); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecS4() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecV>::kSignature | kSignatureElementS); }
  ASMJIT_INLINE_NODEBUG constexpr bool isVecD2() const noexcept { return _signature.subset(kBaseSignatureMask | kSignatureRegElementTypeMask) == (RegTraits<RegType::kLA_VecV>::kSignature | kSignatureElementD); }

  //! Creates a cloned register with element access.
  ASMJIT_INLINE_NODEBUG Vec at(uint32_t elementIndex) const noexcept {
    return Vec((signature() & ~kSignatureRegElementIndexMask) | (elementIndex << kSignatureRegElementIndexShift) | kSignatureRegElementFlagMask, id());
  }

  //! Cast this register to an 8-bit B register.
  ASMJIT_INLINE_NODEBUG VecB b() const noexcept;
  //! Cast this register to a 16-bit H register.
  ASMJIT_INLINE_NODEBUG VecH h() const noexcept;
  //! Cast this register to a 32-bit S register.
  ASMJIT_INLINE_NODEBUG VecS s() const noexcept;
  //! Cast this register to a 64-bit D register.
  ASMJIT_INLINE_NODEBUG VecD d() const noexcept;
  //! Cast this register to a 128-bit Q register.
  ASMJIT_INLINE_NODEBUG VecV q() const noexcept;
  //! Cast this register to a 128-bit V register.
  ASMJIT_INLINE_NODEBUG VecV v() const noexcept;

  //! Casts this register to b (clone).
  ASMJIT_INLINE_NODEBUG Vec v8() const noexcept;
  //! Casts this register to h (clone).
  ASMJIT_INLINE_NODEBUG Vec v16() const noexcept;
  //! Casts this register to s (clone).
  ASMJIT_INLINE_NODEBUG Vec v32() const noexcept;
  //! Casts this register to d (clone).
  ASMJIT_INLINE_NODEBUG Vec v64() const noexcept;
  //! Casts this register to q (clone).
  ASMJIT_INLINE_NODEBUG Vec v128() const noexcept;

  //! Cast this register to a 128-bit V.B[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV b(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.H[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV h(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.S[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV s(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.D[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV d(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.H2[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV h2(uint32_t elementIndex) const noexcept;
  //! Cast this register to a 128-bit V.B4[elementIndex] register.
  ASMJIT_INLINE_NODEBUG VecV b4(uint32_t elementIndex) const noexcept;

  //! Cast this register to V.8B.
  ASMJIT_INLINE_NODEBUG VecD b8() const noexcept;
  //! Cast this register to V.16B.
  ASMJIT_INLINE_NODEBUG VecV b16() const noexcept;
  //! Cast this register to V.2H.
  ASMJIT_INLINE_NODEBUG VecS h2() const noexcept;
  //! Cast this register to V.4H.
  ASMJIT_INLINE_NODEBUG VecD h4() const noexcept;
  //! Cast this register to V.8H.
  ASMJIT_INLINE_NODEBUG VecV h8() const noexcept;
  //! Cast this register to V.2S.
  ASMJIT_INLINE_NODEBUG VecD s2() const noexcept;
  //! Cast this register to V.4S.
  ASMJIT_INLINE_NODEBUG VecV s4() const noexcept;
  //! Cast this register to V.2D.
  ASMJIT_INLINE_NODEBUG VecV d2() const noexcept;

  static ASMJIT_INLINE_NODEBUG constexpr OperandSignature _makeElementAccessSignature(VecElementType elementType, uint32_t elementIndex) noexcept {
    return OperandSignature{
      uint32_t(RegTraits<RegType::kLA_VecV>::kSignature)       |
      uint32_t(kSignatureRegElementFlagMask)                    |
      (uint32_t(elementType) << kSignatureRegElementTypeShift)  |
      (uint32_t(elementIndex << kSignatureRegElementIndexShift))};
  }
};

//! 8-bit view (S) of VFP/SIMD register.
class VecB : public Vec {
public:
  ASMJIT_DEFINE_FINAL_REG(VecB, Vec, RegTraits<RegType::kLA_VecB>)
};

//! 16-bit view (S) of VFP/SIMD register.
class VecH : public Vec {
public:
  ASMJIT_DEFINE_FINAL_REG(VecH, Vec, RegTraits<RegType::kLA_VecH>)
};

//! 32-bit view (S) of VFP/SIMD register.
class VecS : public Vec {
public:
  ASMJIT_DEFINE_FINAL_REG(VecS, Vec, RegTraits<RegType::kLA_VecS>)
};

//! 64-bit view (D) of VFP/SIMD register.
class VecD : public Vec {
public:
  ASMJIT_DEFINE_FINAL_REG(VecD, Vec, RegTraits<RegType::kLA_VecD>)
};

//! 128-bit vector register (Q or V).
class VecV : public Vec {
public:
  ASMJIT_DEFINE_FINAL_REG(VecV, Vec, RegTraits<RegType::kLA_VecV>)
};

ASMJIT_INLINE_NODEBUG VecB Vec::b() const noexcept { return VecB(id()); }
ASMJIT_INLINE_NODEBUG VecH Vec::h() const noexcept { return VecH(id()); }
ASMJIT_INLINE_NODEBUG VecS Vec::s() const noexcept { return VecS(id()); }
ASMJIT_INLINE_NODEBUG VecD Vec::d() const noexcept { return VecD(id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::q() const noexcept { return VecV(id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::v() const noexcept { return VecV(id()); }

ASMJIT_INLINE_NODEBUG Vec Vec::v8() const noexcept { return VecB(id()); }
ASMJIT_INLINE_NODEBUG Vec Vec::v16() const noexcept { return VecH(id()); }
ASMJIT_INLINE_NODEBUG Vec Vec::v32() const noexcept { return VecS(id()); }
ASMJIT_INLINE_NODEBUG Vec Vec::v64() const noexcept { return VecD(id()); }
ASMJIT_INLINE_NODEBUG Vec Vec::v128() const noexcept { return VecV(id()); }

ASMJIT_INLINE_NODEBUG VecV Vec::b(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(VecElementType::kB, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::h(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(VecElementType::kH, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::s(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(VecElementType::kW, elementIndex), id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::d(uint32_t elementIndex) const noexcept { return VecV(_makeElementAccessSignature(VecElementType::kD, elementIndex), id()); }

ASMJIT_INLINE_NODEBUG VecD Vec::b8() const noexcept { return VecD(OperandSignature{VecD::kSignature | kSignatureElementB}, id()); }
ASMJIT_INLINE_NODEBUG VecS Vec::h2() const noexcept { return VecS(OperandSignature{VecS::kSignature | kSignatureElementH}, id()); }
ASMJIT_INLINE_NODEBUG VecD Vec::h4() const noexcept { return VecD(OperandSignature{VecD::kSignature | kSignatureElementH}, id()); }
ASMJIT_INLINE_NODEBUG VecD Vec::s2() const noexcept { return VecD(OperandSignature{VecD::kSignature | kSignatureElementS}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::b16() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementB}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::h8() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementH}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::s4() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementS}, id()); }
ASMJIT_INLINE_NODEBUG VecV Vec::d2() const noexcept { return VecV(OperandSignature{VecV::kSignature | kSignatureElementD}, id()); }

#ifndef _DOXYGEN
namespace regs {
#endif

//! Creates a 32-bit W register operand.
static ASMJIT_INLINE_NODEBUG constexpr GpW w(uint32_t id) noexcept { return GpW(id); }
//! Creates a 64-bit X register operand.
static ASMJIT_INLINE_NODEBUG constexpr GpX x(uint32_t id) noexcept { return GpX(id); }

//! Creates a 32-bit S register operand.
static ASMJIT_INLINE_NODEBUG constexpr VecS s(uint32_t id) noexcept { return VecS(id); }
//! Creates a 64-bit D register operand.
static ASMJIT_INLINE_NODEBUG constexpr VecD d(uint32_t id) noexcept { return VecD(id); }
//! Creates a 1282-bit V register operand.
static ASMJIT_INLINE_NODEBUG constexpr VecV v(uint32_t id) noexcept { return VecV(id); }

static constexpr GpX r0 = GpX(0);
static constexpr GpX r1 = GpX(1);
static constexpr GpX r2 = GpX(2);
static constexpr GpX r3 = GpX(3);
static constexpr GpX r4 = GpX(4);
static constexpr GpX r5 = GpX(5);
static constexpr GpX r6 = GpX(6);
static constexpr GpX r7 = GpX(7);
static constexpr GpX r8 = GpX(8);
static constexpr GpX r9 = GpX(9);
static constexpr GpX r10 = GpX(10);
static constexpr GpX r11 = GpX(11);
static constexpr GpX r12 = GpX(12);
static constexpr GpX r13 = GpX(13);
static constexpr GpX r14 = GpX(14);
static constexpr GpX r15 = GpX(15);
static constexpr GpX r16 = GpX(16);
static constexpr GpX r17 = GpX(17);
static constexpr GpX r18 = GpX(18);
static constexpr GpX r19 = GpX(19);
static constexpr GpX r20 = GpX(20);
static constexpr GpX r21 = GpX(21);
static constexpr GpX r22 = GpX(22);
static constexpr GpX r23 = GpX(23);
static constexpr GpX r24 = GpX(24);
static constexpr GpX r25 = GpX(25);
static constexpr GpX r26 = GpX(26);
static constexpr GpX r27 = GpX(27);
static constexpr GpX r28 = GpX(28);
static constexpr GpX r29 = GpX(29);
static constexpr GpX r30 = GpX(30);
static constexpr GpX r31 = GpX(30);

static constexpr GpX zero = GpX(0);
static constexpr GpX ra = GpX(1);
static constexpr GpX tp = GpX(2);
static constexpr GpX sp = GpX(3);
static constexpr GpX a0 = GpX(4);
static constexpr GpX a1 = GpX(5);
static constexpr GpX a2 = GpX(6);
static constexpr GpX a3 = GpX(7);
static constexpr GpX a4 = GpX(8);
static constexpr GpX a5 = GpX(9);
static constexpr GpX a6 = GpX(10);
static constexpr GpX a7 = GpX(11);
static constexpr GpX t0 = GpX(12);
static constexpr GpX t1 = GpX(13);
static constexpr GpX t2 = GpX(14);
static constexpr GpX t3 = GpX(15);
static constexpr GpX t4 = GpX(16);
static constexpr GpX t5 = GpX(17);
static constexpr GpX t6 = GpX(18);
static constexpr GpX t7 = GpX(19);
static constexpr GpX t8 = GpX(20);
static constexpr GpX rx = GpX(21);
static constexpr GpX fp = GpX(22);
static constexpr GpX s0 = GpX(23);
static constexpr GpX s1 = GpX(24);
static constexpr GpX s2 = GpX(25);
static constexpr GpX s3 = GpX(26);
static constexpr GpX s4 = GpX(27);
static constexpr GpX s5 = GpX(28);
static constexpr GpX s6 = GpX(29);
static constexpr GpX s7 = GpX(30);
static constexpr GpX s8 = GpX(31);

static constexpr VecB b0 = VecB(0);
static constexpr VecB b1 = VecB(1);
static constexpr VecB b2 = VecB(2);
static constexpr VecB b3 = VecB(3);
static constexpr VecB b4 = VecB(4);
static constexpr VecB b5 = VecB(5);
static constexpr VecB b6 = VecB(6);
static constexpr VecB b7 = VecB(7);
static constexpr VecB b8 = VecB(8);
static constexpr VecB b9 = VecB(9);
static constexpr VecB b10 = VecB(10);
static constexpr VecB b11 = VecB(11);
static constexpr VecB b12 = VecB(12);
static constexpr VecB b13 = VecB(13);
static constexpr VecB b14 = VecB(14);
static constexpr VecB b15 = VecB(15);
static constexpr VecB b16 = VecB(16);
static constexpr VecB b17 = VecB(17);
static constexpr VecB b18 = VecB(18);
static constexpr VecB b19 = VecB(19);
static constexpr VecB b20 = VecB(20);
static constexpr VecB b21 = VecB(21);
static constexpr VecB b22 = VecB(22);
static constexpr VecB b23 = VecB(23);
static constexpr VecB b24 = VecB(24);
static constexpr VecB b25 = VecB(25);
static constexpr VecB b26 = VecB(26);
static constexpr VecB b27 = VecB(27);
static constexpr VecB b28 = VecB(28);
static constexpr VecB b29 = VecB(29);
static constexpr VecB b30 = VecB(30);
static constexpr VecB b31 = VecB(31);

static constexpr VecH h0 = VecH(0);
static constexpr VecH h1 = VecH(1);
static constexpr VecH h2 = VecH(2);
static constexpr VecH h3 = VecH(3);
static constexpr VecH h4 = VecH(4);
static constexpr VecH h5 = VecH(5);
static constexpr VecH h6 = VecH(6);
static constexpr VecH h7 = VecH(7);
static constexpr VecH h8 = VecH(8);
static constexpr VecH h9 = VecH(9);
static constexpr VecH h10 = VecH(10);
static constexpr VecH h11 = VecH(11);
static constexpr VecH h12 = VecH(12);
static constexpr VecH h13 = VecH(13);
static constexpr VecH h14 = VecH(14);
static constexpr VecH h15 = VecH(15);
static constexpr VecH h16 = VecH(16);
static constexpr VecH h17 = VecH(17);
static constexpr VecH h18 = VecH(18);
static constexpr VecH h19 = VecH(19);
static constexpr VecH h20 = VecH(20);
static constexpr VecH h21 = VecH(21);
static constexpr VecH h22 = VecH(22);
static constexpr VecH h23 = VecH(23);
static constexpr VecH h24 = VecH(24);
static constexpr VecH h25 = VecH(25);
static constexpr VecH h26 = VecH(26);
static constexpr VecH h27 = VecH(27);
static constexpr VecH h28 = VecH(28);
static constexpr VecH h29 = VecH(29);
static constexpr VecH h30 = VecH(30);
static constexpr VecH h31 = VecH(31);

static constexpr VecS v_s0 = VecS(0);
static constexpr VecS v_s1 = VecS(1);
static constexpr VecS v_s2 = VecS(2);
static constexpr VecS v_s3 = VecS(3);
static constexpr VecS v_s4 = VecS(4);
static constexpr VecS v_s5 = VecS(5);
static constexpr VecS v_s6 = VecS(6);
static constexpr VecS v_s7 = VecS(7);
static constexpr VecS v_s8 = VecS(8);
static constexpr VecS v_s9 = VecS(9);
static constexpr VecS v_s10 = VecS(10);
static constexpr VecS v_s11 = VecS(11);
static constexpr VecS v_s12 = VecS(12);
static constexpr VecS v_s13 = VecS(13);
static constexpr VecS v_s14 = VecS(14);
static constexpr VecS v_s15 = VecS(15);
static constexpr VecS v_s16 = VecS(16);
static constexpr VecS v_s17 = VecS(17);
static constexpr VecS v_s18 = VecS(18);
static constexpr VecS v_s19 = VecS(19);
static constexpr VecS v_s20 = VecS(20);
static constexpr VecS v_s21 = VecS(21);
static constexpr VecS v_s22 = VecS(22);
static constexpr VecS v_s23 = VecS(23);
static constexpr VecS v_s24 = VecS(24);
static constexpr VecS v_s25 = VecS(25);
static constexpr VecS v_s26 = VecS(26);
static constexpr VecS v_s27 = VecS(27);
static constexpr VecS v_s28 = VecS(28);
static constexpr VecS v_s29 = VecS(29);
static constexpr VecS v_s30 = VecS(30);
static constexpr VecS v_s31 = VecS(31);

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

static constexpr VecV vr0 = VecV(0);
static constexpr VecV vr1 = VecV(1);
static constexpr VecV vr2 = VecV(2);
static constexpr VecV vr3 = VecV(3);
static constexpr VecV vr4 = VecV(4);
static constexpr VecV vr5 = VecV(5);
static constexpr VecV vr6 = VecV(6);
static constexpr VecV vr7 = VecV(7);
static constexpr VecV vr8 = VecV(8);
static constexpr VecV vr9 = VecV(9);
static constexpr VecV vr10 = VecV(10);
static constexpr VecV vr11 = VecV(11);
static constexpr VecV vr12 = VecV(12);
static constexpr VecV vr13 = VecV(13);
static constexpr VecV vr14 = VecV(14);
static constexpr VecV vr15 = VecV(15);
static constexpr VecV vr16 = VecV(16);
static constexpr VecV vr17 = VecV(17);
static constexpr VecV vr18 = VecV(18);
static constexpr VecV vr19 = VecV(19);
static constexpr VecV vr20 = VecV(20);
static constexpr VecV vr21 = VecV(21);
static constexpr VecV vr22 = VecV(22);
static constexpr VecV vr23 = VecV(23);
static constexpr VecV vr24 = VecV(24);
static constexpr VecV vr25 = VecV(25);
static constexpr VecV vr26 = VecV(26);
static constexpr VecV vr27 = VecV(27);
static constexpr VecV vr28 = VecV(28);
static constexpr VecV vr29 = VecV(29);
static constexpr VecV vr30 = VecV(30);
static constexpr VecV vr31 = VecV(31);

#ifndef _DOXYGEN
} // {regs}

// Make `a64::regs` accessible through `a64` namespace as well.
using namespace regs;
#endif

//! \name Shift Operation Construction
//! \{

//! \}

//! \name Memory Operand Construction
//! \{

//FIXME del some mem operand
//! Creates `[base, offset]` memory operand (offset mode).
static ASMJIT_INLINE_NODEBUG constexpr Mem ptr(const Gp& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! Creates `[base, index]` memory operand.
static ASMJIT_INLINE_NODEBUG constexpr Mem ptr(const Gp& base, const Gp& index) noexcept {
  return Mem(base, index);
}

//! Creates `[base, index, SHIFT_OP #shift]` memory operand.
static ASMJIT_INLINE_NODEBUG constexpr Mem ptr(const Gp& base, const Gp& index, const Shift& shift) noexcept {
  return Mem(base, index, shift);
}

//! Creates `[base, offset]` memory operand.
static ASMJIT_INLINE_NODEBUG constexpr Mem ptr(const Label& base, int32_t offset = 0) noexcept {
  return Mem(base, offset);
}

//! \}

//! \}

ASMJIT_END_SUB_NAMESPACE

//! \cond INTERNAL
ASMJIT_BEGIN_NAMESPACE
ASMJIT_DEFINE_TYPE_ID(la64::GpW, TypeId::kInt32);
ASMJIT_DEFINE_TYPE_ID(la64::GpX, TypeId::kInt64);
ASMJIT_DEFINE_TYPE_ID(la64::VecS, TypeId::kFloat32x1);
ASMJIT_DEFINE_TYPE_ID(la64::VecD, TypeId::kFloat64x1);
ASMJIT_DEFINE_TYPE_ID(la64::VecV, TypeId::kInt32x4);
ASMJIT_END_NAMESPACE
//! \endcond

#endif // ASMJIT_LA_A64OPERAND_H_INCLUDED
