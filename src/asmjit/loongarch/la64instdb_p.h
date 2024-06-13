// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_LA64INSTDB_H_P_INCLUDED
#define ASMJIT_LA_LA64INSTDB_H_P_INCLUDED

#include "../core/codeholder.h"
#include "../core/instdb_p.h"
#include "../loongarch/la64instdb.h"
#include "../loongarch/la64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

namespace InstDB {

// a64::InstDB - Constants Used by Instructions
// ============================================

// GP register types supported by base instructions.
static constexpr uint32_t kW = 0x1;
static constexpr uint32_t kX = 0x2;
static constexpr uint32_t kWX = 0x3;

// GP high register IDs supported by the instruction.
static constexpr uint32_t kZR = Gp::kIdZr;
static constexpr uint32_t kSP = Gp::kIdSp;

// a64::InstDB - RWInfo
// ====================

enum RWInfoType : uint32_t {
  kRWI_R,
  kRWI_RW,
  kRWI_RX,
  kRWI_RRW,
  kRWI_RWX,
  kRWI_W,
  kRWI_WRW,
  kRWI_WRX,
  kRWI_WRRW,
  kRWI_WRRX,
  kRWI_WW,
  kRWI_X,
  kRWI_XRX,
  kRWI_XXRRX,

  kRWI_LDn,
  kRWI_STn,

  kRWI_SpecialStart = kRWI_LDn
};

// a64::InstDB - ElementType
// =========================

enum InstElementType : uint8_t {
  kET_None = uint8_t(VecElementType::kNone),
  kET_B    = uint8_t(VecElementType::kB),
  kET_H    = uint8_t(VecElementType::kH),
  kET_S    = uint8_t(VecElementType::kW),
  kET_D    = uint8_t(VecElementType::kD),
};

// a64::InstDB - GpType
// ====================

enum GpType : uint8_t {
  kGp_W,
  kGp_X,
  kGp_X_SP
};

// a64::InstDB - OPSig
// ===================

enum kOpSignature : uint32_t {
  kOp_GpW = GpW::kSignature,
  kOp_GpX = GpX::kSignature,

  kOp_B = VecB::kSignature,
  kOp_H = VecH::kSignature,
  kOp_S = VecS::kSignature,
  kOp_D = VecD::kSignature,
  kOp_Q = VecV::kSignature,

  kOp_V8B = VecD::kSignature | Vec::kSignatureElementB,
  kOp_V4H = VecD::kSignature | Vec::kSignatureElementH,
  kOp_V2S = VecD::kSignature | Vec::kSignatureElementS,

  kOp_V16B = VecV::kSignature | Vec::kSignatureElementB,
  kOp_V8H = VecV::kSignature | Vec::kSignatureElementH,
  kOp_V4S = VecV::kSignature | Vec::kSignatureElementS,
  kOp_V2D = VecV::kSignature | Vec::kSignatureElementD
};

// a64::InstDB - HFConv
// ====================

enum kHFConv : uint32_t {
  //! FP16 version of the instruction is not available.
  kHF_N,

  //! Doesn't do any change to the opcode.
  kHF_0,

  kHF_A,
  kHF_B,
  kHF_C,
  kHF_D,

  kHF_Count
};

// a64::InstDB - VOType
// ====================

//! Vector operand type combinations used by FP&SIMD instructions.
enum VOType : uint32_t {
  kVO_V_B,
  kVO_V_BH,
  kVO_V_BH_4S,
  kVO_V_BHS,
  kVO_V_BHS_D2,
  kVO_V_HS,
  kVO_V_S,

  kVO_V_B8H4,
  kVO_V_B8H4S2,
  kVO_V_B8D1,
  kVO_V_H4S2,

  kVO_V_B16,
  kVO_V_B16H8,
  kVO_V_B16H8S4,
  kVO_V_B16D2,
  kVO_V_H8S4,
  kVO_V_S4,
  kVO_V_D2,

  kVO_SV_BHS,
  kVO_SV_B8H4S2,
  kVO_SV_HS,
  kVO_V_Any,
  kVO_SV_Any,

  kVO_Count
};

// a64::InstDB - EncodingId
// ========================

// ${EncodingId:Begin}
// ------------------- Automatically generated, do not edit -------------------
enum EncodingId : uint32_t {
  kEncodingNone = 0,
  kEncodingBaseBranchRel,
  kEncodingBaseLFIVV,
  kEncodingBaseLIC,
  kEncodingBaseLII,
  kEncodingBaseLIR,
  kEncodingBaseLIRR,
  kEncodingBaseLIV,
  kEncodingBaseLRI,
  kEncodingBaseLRR,
  kEncodingBaseLRRI,
  kEncodingBaseLRRII,
  kEncodingBaseLRRIL,
  kEncodingBaseLRRL,
  kEncodingBaseLRRR,
  kEncodingBaseLRRRI,
  kEncodingBaseLRRRT,
  kEncodingBaseLRV,
  kEncodingBaseLVI,
  kEncodingBaseLVR,
  kEncodingBaseLVRR,
  kEncodingBaseLVV,
  kEncodingBaseLVVV,
  kEncodingBaseLdSt,
  kEncodingBaseOp,
  kEncodingBaseOpImm,
  kEncodingFpLVVVV,
  kEncodingJBTLRRI,
  kEncodingLCldst,
  kEncodingLFPldst,
  kEncodingLPldst,
  kEncodingLasxIX,
  kEncodingLasxRXI,
  kEncodingLasxXI,
  kEncodingLasxXII,
  kEncodingLasxXR,
  kEncodingLasxXRI,
  kEncodingLasxXRII,
  kEncodingLasxXX,
  kEncodingLasxXXI,
  kEncodingLasxXXR,
  kEncodingLasxXXX,
  kEncodingLasxXXXX,
  kEncodingLfVVVI,
  kEncodingLsxIV,
  kEncodingLsxRVI,
  kEncodingLsxVI,
  kEncodingLsxVII,
  kEncodingLsxVR,
  kEncodingLsxVRI,
  kEncodingLsxVRII,
  kEncodingLsxVV,
  kEncodingLsxVVI,
  kEncodingLsxVVR,
  kEncodingLsxVVV,
  kEncodingLsxVVVV,
  kEncodingSimdLdst
};
// ----------------------------------------------------------------------------
// ${EncodingId:End}

// a64::InstDB::EncodingData
// =========================

namespace EncodingData {

#define M_OPCODE(field, bits) \
  uint32_t _##field : bits; \
  ASMJIT_INLINE_NODEBUG constexpr uint32_t field() const noexcept { return uint32_t(_##field) << (32 - bits); }

struct BaseOp {
  uint32_t opcode;
};
struct BaseOpImm {
  //uint32_t opcode;
  M_OPCODE(opcode, 17)
  uint16_t immBits;
  uint16_t immOffset;
};

struct BaseLIC {
  M_OPCODE(opcode, 17)
  uint32_t immOffset : 5;
};

struct BaseLRI {
  //uint32_t opcode;
  M_OPCODE(opcode, 22)
  uint32_t rType : 8;
  //uint32_t rHiId : 8;
  uint32_t rShift : 8;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 3;
};

struct BaseLIR {
  //uint32_t opcode;
  M_OPCODE(opcode, 22)
  uint32_t aType : 2;
  uint32_t aShift : 5;
  //uint32_t aImmSize : 3;
  uint32_t aImmOffset : 5;
};

struct BaseLIV {
  M_OPCODE(opcode, 22)
  uint32_t aShift : 5;
  uint32_t aImmOffset : 5;
};

typedef BaseLIV BaseLVI;

struct BaseLRR {
  //uint32_t opcode;
  M_OPCODE(opcode, 22)
  uint32_t aType : 2;
  //uint32_t aHiId : 6;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  //uint32_t bHiId : 6;
  uint32_t bShift : 5;
  uint32_t uniform : 1;
};

struct BaseLVV {
  M_OPCODE(opcode, 22)
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t uniform : 1;
};

struct BaseLRV {
  M_OPCODE(opcode, 22)
  uint32_t aType : 2;
  //uint32_t aHiId : 6;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t uniform : 1;
};

struct BaseLVR {
  M_OPCODE(opcode, 22)
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  uint32_t uniform : 1;
};

struct BaseLRRL {
  //uint32_t opcode;
  M_OPCODE(opcode, 17)
  uint32_t aType : 2;
  //uint32_t aHiId : 6;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  //uint32_t bHiId : 6;
  uint32_t bShift : 5;
  //uint32_t uniform : 1;
};

struct BaseLRRRI {
  M_OPCODE(opcode, 15)
  uint32_t aType : 2;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  uint32_t cType : 2;
  uint32_t cShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 2;
};

//typedef BaseLRRR BaseLRRRT;

struct BaseLRRR {
  M_OPCODE(opcode, 17)
  //uint32_t opcode;
  uint32_t aType : 2;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  uint32_t cType : 2;
  uint32_t cShift : 5;
  uint32_t uniform : 1;
};

struct BaseLVRR {
  M_OPCODE(opcode, 17)
  //uint32_t opcode;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  uint32_t cType : 2;
  uint32_t cShift : 5;
  uint32_t uniform : 1;
};

struct BaseLVVV {
  M_OPCODE(opcode, 17)
  //uint32_t opcode;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t cShift : 5;
  uint32_t uniform : 1;
};

struct BaseLII {
  M_OPCODE(opcode, 6)
  uint32_t aImmSize : 5;
  uint32_t aImmOffset : 5;
  uint32_t bImmSize : 3;
  uint32_t bImmOffset : 5;
  uint32_t cImmSize : 16;
  uint32_t cImmOffset : 5;
  uint32_t uniform : 1;
};

struct BaseLRRII {
  M_OPCODE(opcode, 11)
  uint32_t aType : 2;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  //uint32_t aImmSize : 6;
  //uint32_t aImmDiscardLsb : 5;
  uint32_t aImmOffset : 5;
  //uint32_t bImmSize : 6;
  //uint32_t bImmDiscardLsb : 5;
  uint32_t bImmOffset : 5;
  uint32_t uniform : 2;
};

struct BaseLRRI {
  M_OPCODE(opcode, 17)
  uint32_t aType : 2;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 3;
};

struct BaseLFIVV {
  M_OPCODE(opcode, 17)
  uint32_t aImmOffset : 5;
  uint32_t bShift : 5;
  uint32_t cShift : 5;
  uint32_t uniform : 2;
};

typedef BaseLRRI BaseLIRR;

struct BaseLRRIL {
  M_OPCODE(opcode, 6)
  uint32_t aType : 2;
  uint32_t aHiId : 6;
  uint32_t aImmOffset : 5;
};

struct BaseLldst {
  uint32_t offsetOp : 10;
  uint32_t prePostOp : 10;
  uint32_t rType : 2;
  //uint32_t xOffset : 5;
  uint32_t offsetShift : 5;
};

struct BaseLdSt {
  uint32_t _opcode;
  uint32_t opShift : 5;
  constexpr uint32_t opcode() const noexcept { return _opcode << opShift; }
};

typedef BaseLRRR BaseLRRRT;
//typedef BaseOp BTLRRI;

struct FpLVVVV {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode << 20; }
};

struct LsxVVVV {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t cShift : 5;
  uint32_t dShift : 5;
  uint32_t uniform : 2;
  constexpr uint32_t opcode() const noexcept { return _opcode << 20; }
};

struct LsxVVV {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t cShift : 5;
  uint32_t uniform : 2;
  constexpr uint32_t opcode() const noexcept { return _opcode << 15; }
};

struct LsxVRI {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType: 2;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 4;
  constexpr uint32_t opcode() const noexcept { return _opcode << 11; }
};

struct LasxXRI {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType: 2;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 4;
  constexpr uint32_t opcode() const noexcept { return _opcode << 12; }
};

struct SimdLdst {
  uint32_t _opcode;
  uint32_t opShift: 5;
  uint32_t offsetLen: 5;
  constexpr uint32_t opcode() const noexcept { return _opcode << opShift; }
};

struct LsxVRII {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType: 2;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t bImmOffset : 5;
  uint32_t uniform : 3;
  constexpr uint32_t opcode() const noexcept { return _opcode << 19; }
};

struct LasxXRII {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType: 2;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t bImmOffset : 5;
  uint32_t uniform : 3;
  constexpr uint32_t opcode() const noexcept { return _opcode << 20; }
};

struct LsxVRR {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType: 2;
  uint32_t bShift : 5;
  uint32_t cType : 2;
  uint32_t cShift : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 15; }
};

struct LasxXRR {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType: 2;
  uint32_t bShift : 5;
  uint32_t cType : 2;
  uint32_t cShift : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 15; }
};

struct LsxVVR {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t cType : 2;
  uint32_t cShift : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 15; }
};

struct LsxVVI {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 4;
  constexpr uint32_t opcode() const noexcept { return _opcode << 11; }
};

struct LsxVII {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t bImmOffset : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 15; }
};

struct LsxVV {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 10; }
};

struct LsxVR {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t bType : 2;
  uint32_t bShift : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 10; }
};

struct LsxIV {
  uint32_t _opcode;
  uint32_t aImmOffset : 5;
  uint32_t aShift : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 10; }
};

struct LsxRVI {
  uint32_t _opcode;
  uint32_t aType : 2;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 2;
  constexpr uint32_t opcode() const noexcept { return _opcode << 11; }
};

struct LasxRXI {
  uint32_t _opcode;
  uint32_t aType : 2;
  uint32_t aShift : 5;
  uint32_t bShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 12; }
};

struct LsxVI {
  uint32_t _opcode;
  uint32_t aShift : 5;
  uint32_t aImmOffset : 5;
  uint32_t uniform : 1;
  constexpr uint32_t opcode() const noexcept { return _opcode << 18; }
};

typedef LsxVVVV LasxXXXX;
typedef LsxVVV LasxXXX;
typedef LsxVVR LasxXXR;
typedef LsxVVI LasxXXI;
typedef LsxVII LasxXII;
typedef LsxVV LasxXX;
typedef LsxIV LasxIX;
typedef LsxVR LasxXR;
typedef LsxVI LasxXI;
//typedef LsxVRII LasxXRII;
//typedef LsxVRI LasxXRI;
struct BTLRRI {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode << 26; }
};

struct BaseBLI {
  uint32_t _opcode;
  //uint32_t btypel : 2;
  constexpr uint32_t opcode() const noexcept { return _opcode << 26; }
};

           //FIXME
struct BaseBranchRel {
  uint32_t _opcode;
  OffsetType offsetType : 8;
  constexpr uint32_t opcode() const noexcept { return _opcode << 26; }
};

struct JBTLRRI {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode << 26; }
};

struct LfVVVI {
  M_OPCODE(opcode, 14)
  uint32_t ashift : 5;
  uint32_t bshift : 5;
  uint32_t cshift : 5;
  uint32_t immSize : 4;
  uint32_t immShift : 4;
  uint32_t uniform : 2;
};

struct LFldst {
  uint32_t offsetOp : 10;
};

typedef LFldst LPldst;
typedef LFldst LCldst;

struct LFPldst {
  uint32_t offsetOp : 8;
};

typedef BaseOp BaseBranchReg;
/* typedef BaseOp BaseBranchRel; */
typedef BaseOp BaseBranchCmp;
typedef BaseOp BaseBranchTst;
typedef BaseOp BaseExtract;
typedef BaseOp BaseBfc;
typedef BaseOp BaseBfi;
typedef BaseOp BaseBfx;
typedef BaseOp BaseCCmp;
typedef BaseOp BaseCInc;
typedef BaseOp BaseCSet;
typedef BaseOp BaseCSel;
typedef BaseOp BaseMovKNZ;
typedef BaseOp BaseMull;

struct FSimdGeneric {
  uint32_t _scalarOp : 28;
  uint32_t _scalarHf : 4;
  uint32_t _vectorOp : 28;
  uint32_t _vectorHf : 4;

  constexpr uint32_t scalarOp() const noexcept { return uint32_t(_scalarOp) << 10; }
  constexpr uint32_t vectorOp() const noexcept { return uint32_t(_vectorOp) << 10; }
  constexpr uint32_t scalarHf() const noexcept { return uint32_t(_scalarHf); }
  constexpr uint32_t vectorHf() const noexcept { return uint32_t(_vectorHf); }
};

typedef FSimdGeneric FSimdVV;
typedef FSimdGeneric FSimdVVV;
typedef FSimdGeneric FSimdVVVV;

struct FSimdSV {
  uint32_t opcode;
};

struct FSimdVVVe {
  uint32_t _scalarOp : 28;
  uint32_t _scalarHf : 4;
  uint32_t _vectorOp;
  uint32_t _elementOp;

  constexpr uint32_t scalarOp() const noexcept { return uint32_t(_scalarOp) << 10; }
  constexpr uint32_t scalarHf() const noexcept { return uint32_t(_scalarHf); };
  constexpr uint32_t vectorOp() const noexcept { return uint32_t(_vectorOp) << 10; }
  constexpr uint32_t vectorHf() const noexcept { return kHF_C; }
  constexpr uint32_t elementScalarOp() const noexcept { return (uint32_t(_elementOp) << 10) | (0x5u << 28); }
  constexpr uint32_t elementVectorOp() const noexcept { return (uint32_t(_elementOp) << 10); }
};

struct SimdFcadd {
  uint32_t _opcode;

  constexpr uint32_t opcode() const noexcept { return _opcode << 10; }
};

struct SimdFcmla {
  uint32_t _regularOp;
  uint32_t _elementOp;

  constexpr uint32_t regularOp() const noexcept { return uint32_t(_regularOp) << 10; }
  constexpr uint32_t elementOp() const noexcept { return (uint32_t(_elementOp) << 10); }
};

struct SimdFccmpFccmpe {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode; }
};

struct SimdFcm {
  uint32_t _registerOp : 28;
  uint32_t _registerHf : 4;

  uint32_t _zeroOp : 28;

  constexpr bool hasRegisterOp() const noexcept { return _registerOp != 0; }
  constexpr bool hasZeroOp() const noexcept { return _zeroOp != 0; }

  constexpr uint32_t registerScalarOp() const noexcept { return (uint32_t(_registerOp) << 10) | (0x5u << 28); }
  constexpr uint32_t registerVectorOp() const noexcept { return uint32_t(_registerOp) << 10; }
  constexpr uint32_t registerScalarHf() const noexcept { return uint32_t(_registerHf); }
  constexpr uint32_t registerVectorHf() const noexcept { return uint32_t(_registerHf); }

  constexpr uint32_t zeroScalarOp() const noexcept { return (uint32_t(_zeroOp) << 10) | (0x5u << 28); }
  constexpr uint32_t zeroVectorOp() const noexcept { return (uint32_t(_zeroOp) << 10); }
};

struct SimdFcmpFcmpe {
  uint32_t _opcode;
  constexpr uint32_t opcode() const noexcept { return _opcode; }
};

struct SimdFcvtLN {
  uint32_t _opcode : 22;
  uint32_t _isCvtxn : 1;
  uint32_t _hasScalar : 1;

  constexpr uint32_t scalarOp() const noexcept { return (uint32_t(_opcode) << 10) | (0x5u << 28); }
  constexpr uint32_t vectorOp() const noexcept { return (uint32_t(_opcode) << 10); }

  constexpr uint32_t isCvtxn() const noexcept { return _isCvtxn; }
  constexpr uint32_t hasScalar() const noexcept { return _hasScalar; }
};

struct SimdFcvtSV {
  uint32_t _vectorIntOp;
  uint32_t _vectorFpOp;
  uint32_t _generalOp : 31;
  uint32_t _isFloatToInt : 1;

  constexpr uint32_t scalarIntOp() const noexcept { return (uint32_t(_vectorIntOp) << 10) | (0x5u << 28); }
  constexpr uint32_t vectorIntOp() const noexcept { return uint32_t(_vectorIntOp) << 10; }
  constexpr uint32_t scalarFpOp() const noexcept { return (uint32_t(_vectorFpOp) << 10) | (0x5u << 28); }
  constexpr uint32_t vectorFpOp() const noexcept { return uint32_t(_vectorFpOp) << 10; }
  constexpr uint32_t generalOp() const noexcept { return (uint32_t(_generalOp) << 10); }

  constexpr uint32_t isFloatToInt() const noexcept { return _isFloatToInt; }
  constexpr uint32_t isFixedPoint() const noexcept { return _vectorFpOp != 0; }
};

struct SimdFmlal {
  uint32_t _vectorOp;
  uint32_t _elementOp;
  uint8_t _optionalQ;
  uint8_t tA;
  uint8_t tB;
  uint8_t tElement;

  constexpr uint32_t vectorOp() const noexcept { return uint32_t(_vectorOp) << 10; }
  constexpr uint32_t elementOp() const noexcept { return uint32_t(_elementOp) << 10; }
  constexpr uint32_t optionalQ() const noexcept { return _optionalQ; }
};

struct FSimdPair {
  uint32_t _scalarOp;
  uint32_t _vectorOp;

  constexpr uint32_t scalarOp() const noexcept { return uint32_t(_scalarOp) << 10; }
  constexpr uint32_t vectorOp() const noexcept { return uint32_t(_vectorOp) << 10; }
};

struct ISimdVV {
  M_OPCODE(opcode, 22)
  uint32_t vecOpType : 6;
};

struct ISimdVVx {
  M_OPCODE(opcode, 22)
  uint32_t op0Signature;
  uint32_t op1Signature;
};

struct ISimdSV {
  M_OPCODE(opcode, 22)
  uint32_t vecOpType : 6;
};

struct ISimdVVV {
  M_OPCODE(opcode, 22)
  uint32_t vecOpType : 6;
};

struct ISimdVVVx {
  M_OPCODE(opcode, 22)
  uint32_t op0Signature;
  uint32_t op1Signature;
  uint32_t op2Signature;
};

struct ISimdWWV {
  M_OPCODE(opcode, 22)
  uint32_t vecOpType : 6;
};

struct ISimdVVVe {
  uint32_t regularOp : 26; // 22 bits used.
  uint32_t regularVecType : 6;
  uint32_t elementOp : 26; // 22 bits used.
  uint32_t elementVecType : 6;
};

struct ISimdVVVI {
  M_OPCODE(opcode, 22)
  uint32_t vecOpType : 6;
  uint32_t immSize : 4;
  uint32_t immShift : 4;
  uint32_t imm64HasOneBitLess : 1;
};

struct ISimdVVVV {
  uint32_t opcode : 22;
  uint32_t vecOpType : 6;
};

struct ISimdVVVVx {
  uint32_t opcode;
  uint32_t op0Signature;
  uint32_t op1Signature;
  uint32_t op2Signature;
  uint32_t op3Signature;
};

struct SimdBicOrr {
  uint32_t registerOp;   // 22 bits used.
  uint32_t immediateOp;  // 22 bits used.
};

struct SimdCmp {
  uint32_t regOp;
  uint32_t zeroOp : 22;
  uint32_t vecOpType : 6;
};

struct SimdDot {
  uint32_t vectorOp;     // 22 bits used.
  uint32_t elementOp;    // 22 bits used.
  uint8_t tA;            // Element-type of the first operand.
  uint8_t tB;            // Element-type of the second and third operands.
  uint8_t tElement;      // Element-type of the element index[] operand.
};

struct SimdMoviMvni {
  uint32_t opcode : 31;
  uint32_t inverted : 1;
};

struct SimdLdSt {
  uint32_t uOffsetOp  : 10;
  uint32_t prePostOp  : 11;
  uint32_t registerOp : 11;
  uint32_t literalOp  : 8;
  uint32_t uAltInstId : 16;
};

struct SimdLdNStN {
  uint32_t singleOp;
  uint32_t multipleOp : 22;
  uint32_t n : 3;
  uint32_t replicate : 1;
};

struct SimdLdpStp {
  uint32_t offsetOp : 10;
  uint32_t prePostOp : 10;
};

struct SimdLdurStur {
  uint32_t opcode;
};

struct ISimdPair {
  uint32_t opcode2;      // 22 bits used.
  uint32_t opcode3 : 26; // 22 bits used.
  uint32_t opType3 : 6;
};

struct SimdShift {
  uint32_t registerOp;       // 22 bits used.
  uint32_t immediateOp : 22; // 22 bits used.
  uint32_t invertedImm : 1;
  uint32_t vecOpType : 6;
};

struct SimdShiftES {
  uint32_t opcode : 22;
  uint32_t vecOpType : 6;
};

struct SimdSm3tt {
  uint32_t opcode;
};

struct SimdSmovUmov {
  uint32_t opcode : 22;
  uint32_t vecOpType : 6;
  uint32_t isSigned : 1;
};

struct SimdSxtlUxtl {
  uint32_t opcode : 22;
  uint32_t vecOpType : 6;
};

struct SimdTblTbx {
  uint32_t opcode;
};

#undef M_OPCODE

// ${EncodingDataForward:Begin}
// ------------------- Automatically generated, do not edit -------------------
extern const BaseBranchRel baseBranchRel[8];
extern const BaseLFIVV baseLFIVV[22];
extern const BaseLIC baseLIC[3];
extern const BaseLII baseLII[2];
extern const BaseLIR baseLIR[1];
extern const BaseLIRR baseLIRR[1];
extern const BaseLIV baseLIV[1];
extern const BaseLRI baseLRI[10];
extern const BaseLRR baseLRR[35];
extern const BaseLRRI baseLRRI[18];
extern const BaseLRRII baseLRRII[4];
extern const BaseLRRIL baseLRRIL[1];
extern const BaseLRRL baseLRRL[2];
extern const BaseLRRR baseLRRR[60];
extern const BaseLRRRI baseLRRRI[5];
extern const BaseLRRRT baseLRRRT[36];
extern const BaseLRV baseLRV[3];
extern const BaseLVI baseLVI[1];
extern const BaseLVR baseLVR[3];
extern const BaseLVRR baseLVRR[8];
extern const BaseLVV baseLVV[44];
extern const BaseLVVV baseLVVV[20];
extern const BaseLdSt baseLdSt[30];
extern const BaseOp baseOp[7];
extern const BaseOpImm baseOpImm[3];
extern const FpLVVVV fpLVVVV[2];
extern const JBTLRRI jBTLRRI[1];
extern const LCldst lCldst[1];
extern const LFPldst lFPldst[8];
extern const LPldst lPldst[1];
extern const LasxIX lasxIX[10];
extern const LasxRXI lasxRXI[4];
extern const LasxXI lasxXI[1];
extern const LasxXII lasxXII[1];
extern const LasxXR lasxXR[4];
extern const LasxXRI lasxXRI[2];
extern const LasxXRII lasxXRII[4];
extern const LasxXX lasxXX[103];
extern const LasxXXI lasxXXI[171];
extern const LasxXXR lasxXXR[4];
extern const LasxXXX lasxXXX[412];
extern const LasxXXXX lasxXXXX[10];
extern const LfVVVI lfVVVI[1];
extern const LsxIV lsxIV[10];
extern const LsxRVI lsxRVI[8];
extern const LsxVI lsxVI[1];
extern const LsxVII lsxVII[1];
extern const LsxVR lsxVR[4];
extern const LsxVRI lsxVRI[4];
extern const LsxVRII lsxVRII[4];
extern const LsxVV lsxVV[86];
extern const LsxVVI lsxVVI[160];
extern const LsxVVR lsxVVR[4];
extern const LsxVVV lsxVVV[411];
extern const LsxVVVV lsxVVVV[10];
extern const SimdLdst simdLdst[16];
// ----------------------------------------------------------------------------
// ${EncodingDataForward:End}

} // {EncodingData}

// a64::InstDB - Tables
// ====================

#ifndef ASMJIT_NO_TEXT
extern const InstNameIndex instNameIndex;
extern const char _instNameStringTable[];
extern const uint32_t _instNameIndexTable[];
#endif // !ASMJIT_NO_TEXT

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_LA64INSTDB_H_P_INCLUDED

