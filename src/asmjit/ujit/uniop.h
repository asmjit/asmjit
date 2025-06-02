// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_UJIT_UNIOP_H_INCLUDED
#define ASMJIT_UJIT_UNIOP_H_INCLUDED

#include "ujitbase.h"

#if !defined(ASMJIT_NO_UJIT)

ASMJIT_BEGIN_SUB_NAMESPACE(ujit)

//! \addtogroup asmjit_ujit
//! \{

enum class UniOpCond : uint32_t {
  kAssignAnd,
  kAssignOr,
  kAssignXor,
  kAssignAdd,
  kAssignSub,
  kAssignShr,
  kTest,
  kBitTest,
  kCompare,

  kMaxValue = kCompare
};

enum class UniOpM : uint32_t {
  kStoreZeroReg,
  kStoreZeroU8,
  kStoreZeroU16,
  kStoreZeroU32,
  kStoreZeroU64
};

enum class UniOpRM : uint32_t {
  kLoadReg,
  kLoadI8,
  kLoadU8,
  kLoadI16,
  kLoadU16,
  kLoadI32,
  kLoadU32,
  kLoadI64,
  kLoadU64,
  kLoadMergeU8,
  kLoadShiftU8,
  kLoadMergeU16,
  kLoadShiftU16
};

enum class UniOpMR : uint32_t {
  kStoreReg,
  kStoreU8,
  kStoreU16,
  kStoreU32,
  kStoreU64,
  kAddReg,
  kAddU8,
  kAddU16,
  kAddU32,
  kAddU64
};

//! Arithmetic operation having 2 operands (dst, src).
enum class UniOpRR : uint32_t {
  kAbs,
  kNeg,
  kNot,
  kBSwap,
  kCLZ,
  kCTZ,
  kReflect,
  kMaxValue = kReflect
};

//! Arithmetic operation having 3 operands (dst, src1, src2).
enum class UniOpRRR : uint32_t {
  kAnd,
  kOr,
  kXor,
  kBic,
  kAdd,
  kSub,
  kMul,
  kUDiv,
  kUMod,
  kSMin,
  kSMax,
  kUMin,
  kUMax,
  kSll,
  kSrl,
  kSra,
  kRol,
  kRor,
  kSBound,

  kMaxValue = kSBound
};

enum class UniOpVR : uint32_t {
  kMov,
  kMovU32,
  kMovU64,
  kInsertU8,
  kInsertU16,
  kInsertU32,
  kInsertU64,
  kExtractU8,
  kExtractU16,
  kExtractU32,
  kExtractU64,
  kCvtIntToF32,
  kCvtIntToF64,
  kCvtTruncF32ToInt,
  kCvtRoundF32ToInt,
  kCvtTruncF64ToInt,
  kCvtRoundF64ToInt,

  kMaxValue = kCvtRoundF64ToInt
};

enum class UniOpVM : uint32_t {
  kLoad8,
  kLoad16_U16,
  kLoad32_U32,
  kLoad32_F32,

  kLoad64_U32,
  kLoad64_U64,
  kLoad64_F32,
  kLoad64_F64,

  kLoad128_U32,
  kLoad128_U64,
  kLoad128_F32,
  kLoad128_F64,

  kLoad256_U32,
  kLoad256_U64,
  kLoad256_F32,
  kLoad256_F64,

  kLoad512_U32,
  kLoad512_U64,
  kLoad512_F32,
  kLoad512_F64,

  kLoadN_U32,
  kLoadN_U64,
  kLoadN_F32,
  kLoadN_F64,

  kLoadCvt16_U8ToU64,
  kLoadCvt32_U8ToU64,
  kLoadCvt64_U8ToU64,

  kLoadCvt32_I8ToI16,
  kLoadCvt32_U8ToU16,
  kLoadCvt32_I8ToI32,
  kLoadCvt32_U8ToU32,
  kLoadCvt32_I16ToI32,
  kLoadCvt32_U16ToU32,
  kLoadCvt32_I32ToI64,
  kLoadCvt32_U32ToU64,

  kLoadCvt64_I8ToI16,
  kLoadCvt64_U8ToU16,
  kLoadCvt64_I8ToI32,
  kLoadCvt64_U8ToU32,
  kLoadCvt64_I16ToI32,
  kLoadCvt64_U16ToU32,
  kLoadCvt64_I32ToI64,
  kLoadCvt64_U32ToU64,

  kLoadCvt128_I8ToI16,
  kLoadCvt128_U8ToU16,
  kLoadCvt128_I8ToI32,
  kLoadCvt128_U8ToU32,
  kLoadCvt128_I16ToI32,
  kLoadCvt128_U16ToU32,
  kLoadCvt128_I32ToI64,
  kLoadCvt128_U32ToU64,

  kLoadCvt256_I8ToI16,
  kLoadCvt256_U8ToU16,
  kLoadCvt256_I16ToI32,
  kLoadCvt256_U16ToU32,
  kLoadCvt256_I32ToI64,
  kLoadCvt256_U32ToU64,

  kLoadCvtN_U8ToU64,

  kLoadCvtN_I8ToI16,
  kLoadCvtN_U8ToU16,
  kLoadCvtN_I8ToI32,
  kLoadCvtN_U8ToU32,
  kLoadCvtN_I16ToI32,
  kLoadCvtN_U16ToU32,
  kLoadCvtN_I32ToI64,
  kLoadCvtN_U32ToU64,

  kLoadInsertU8,
  kLoadInsertU16,
  kLoadInsertU32,
  kLoadInsertU64,
  kLoadInsertF32,
  kLoadInsertF32x2,
  kLoadInsertF64,

  kMaxValue = kLoadInsertF64
};

enum class UniOpMV : uint32_t {
  kStore8,
  kStore16_U16,
  kStore32_U32,
  kStore32_F32,

  kStore64_U32,
  kStore64_U64,
  kStore64_F32,
  kStore64_F64,

  kStore128_U32,
  kStore128_U64,
  kStore128_F32,
  kStore128_F64,

  kStore256_U32,
  kStore256_U64,
  kStore256_F32,
  kStore256_F64,

  kStore512_U32,
  kStore512_U64,
  kStore512_F32,
  kStore512_F64,

  kStoreN_U32,
  kStoreN_U64,
  kStoreN_F32,
  kStoreN_F64,

  kStoreExtractU16,
  kStoreExtractU32,
  kStoreExtractU64,

  /*
  kStoreCvtz64_U16ToU8,
  kStoreCvtz64_U32ToU16,
  kStoreCvtz64_U64ToU32,
  kStoreCvts64_I16ToI8,
  kStoreCvts64_I16ToU8,
  kStoreCvts64_U16ToU8,
  kStoreCvts64_I32ToI16,
  kStoreCvts64_U32ToU16,
  kStoreCvts64_I64ToI32,
  kStoreCvts64_U64ToU32,

  kStoreCvtz128_U16ToU8,
  kStoreCvtz128_U32ToU16,
  kStoreCvtz128_U64ToU32,
  kStoreCvts128_I16ToI8,
  kStoreCvts128_I16ToU8,
  kStoreCvts128_U16ToU8,
  kStoreCvts128_I32ToI16,
  kStoreCvts128_U32ToU16,
  kStoreCvts128_I64ToI32,
  kStoreCvts128_U64ToU32,

  kStoreCvtz256_U16ToU8,
  kStoreCvtz256_U32ToU16,
  kStoreCvtz256_U64ToU32,
  kStoreCvts256_I16ToI8,
  kStoreCvts256_I16ToU8,
  kStoreCvts256_U16ToU8,
  kStoreCvts256_I32ToI16,
  kStoreCvts256_U32ToU16,
  kStoreCvts256_I64ToI32,
  kStoreCvts256_U64ToU32,

  kStoreCvtzN_U16ToU8,
  kStoreCvtzN_U32ToU16,
  kStoreCvtzN_U64ToU32,
  kStoreCvtsN_I16ToI8,
  kStoreCvtsN_I16ToU8,
  kStoreCvtsN_U16ToU8,
  kStoreCvtsN_I32ToI16,
  kStoreCvtsN_U32ToU16,
  kStoreCvtsN_I64ToI32,
  kStoreCvtsN_U64ToU32,
  */

  kMaxValue = kStoreExtractU64
};

enum class UniOpVV : uint32_t {
  kMov,
  kMovU64,

  kBroadcastU8Z,
  kBroadcastU16Z,
  kBroadcastU8,
  kBroadcastU16,
  kBroadcastU32,
  kBroadcastU64,
  kBroadcastF32,
  kBroadcastF64,
  kBroadcastV128_U32,
  kBroadcastV128_U64,
  kBroadcastV128_F32,
  kBroadcastV128_F64,
  kBroadcastV256_U32,
  kBroadcastV256_U64,
  kBroadcastV256_F32,
  kBroadcastV256_F64,

  kAbsI8,
  kAbsI16,
  kAbsI32,
  kAbsI64,

  kNotU32,
  kNotU64,

  kCvtI8LoToI16,
  kCvtI8HiToI16,
  kCvtU8LoToU16,
  kCvtU8HiToU16,
  kCvtI8ToI32,
  kCvtU8ToU32,
  kCvtI16LoToI32,
  kCvtI16HiToI32,
  kCvtU16LoToU32,
  kCvtU16HiToU32,
  kCvtI32LoToI64,
  kCvtI32HiToI64,
  kCvtU32LoToU64,
  kCvtU32HiToU64,

  kAbsF32,
  kAbsF64,

  kNegF32,
  kNegF64,

  kNotF32,
  kNotF64,

  kTruncF32S,
  kTruncF64S,
  kTruncF32,
  kTruncF64,

  kFloorF32S,
  kFloorF64S,
  kFloorF32,
  kFloorF64,

  kCeilF32S,
  kCeilF64S,
  kCeilF32,
  kCeilF64,

  kRoundF32S,
  kRoundF64S,
  kRoundF32,
  kRoundF64,

  kRcpF32,
  kRcpF64,

  kSqrtF32S,
  kSqrtF64S,
  kSqrtF32,
  kSqrtF64,

  kCvtF32ToF64S,
  kCvtF64ToF32S,
  kCvtI32ToF32,
  kCvtF32LoToF64,
  kCvtF32HiToF64,
  kCvtF64ToF32Lo,
  kCvtF64ToF32Hi,
  kCvtI32LoToF64,
  kCvtI32HiToF64,
  kCvtTruncF32ToI32,
  kCvtTruncF64ToI32Lo,
  kCvtTruncF64ToI32Hi,
  kCvtRoundF32ToI32,
  kCvtRoundF64ToI32Lo,
  kCvtRoundF64ToI32Hi,

  kMaxValue = kCvtRoundF64ToI32Hi
};

enum class UniOpVVI : uint32_t {
  kSllU16,
  kSllU32,
  kSllU64,
  kSrlU16,
  kSrlU32,
  kSrlU64,
  kSraI16,
  kSraI32,
  kSraI64,
  kSllbU128,
  kSrlbU128,
  kSwizzleU16x4,
  kSwizzleLoU16x4,
  kSwizzleHiU16x4,
  kSwizzleU32x4,
  kSwizzleU64x2,
  kSwizzleF32x4,
  kSwizzleF64x2,
  kSwizzleU64x4,
  kSwizzleF64x4,
  kExtractV128_I32,
  kExtractV128_I64,
  kExtractV128_F32,
  kExtractV128_F64,
  kExtractV256_I32,
  kExtractV256_I64,
  kExtractV256_F32,
  kExtractV256_F64,

#if defined(ASMJIT_UJIT_AARCH64)
  kSrlRndU16,
  kSrlRndU32,
  kSrlRndU64,
  kSrlAccU16,
  kSrlAccU32,
  kSrlAccU64,
  kSrlRndAccU16,
  kSrlRndAccU32,
  kSrlRndAccU64,
  kSrlnLoU16,
  kSrlnHiU16,
  kSrlnLoU32,
  kSrlnHiU32,
  kSrlnLoU64,
  kSrlnHiU64,
  kSrlnRndLoU16,
  kSrlnRndHiU16,
  kSrlnRndLoU32,
  kSrlnRndHiU32,
  kSrlnRndLoU64,
  kSrlnRndHiU64,

  kMaxValue = kSrlnRndHiU64

#elif defined(ASMJIT_UJIT_X86)

  kMaxValue = kExtractV256_F64

#else

  kMaxValue = kExtractV256_F64

#endif // ASMJIT_UJIT_AARCH64
};

enum class UniOpVVV : uint32_t {
  kAndU32,
  kAndU64,
  kOrU32,
  kOrU64,
  kXorU32,
  kXorU64,
  kAndnU32,
  kAndnU64,
  kBicU32,
  kBicU64,
  kAvgrU8,
  kAvgrU16,
  kAddU8,
  kAddU16,
  kAddU32,
  kAddU64,
  kSubU8,
  kSubU16,
  kSubU32,
  kSubU64,
  kAddsI8,
  kAddsU8,
  kAddsI16,
  kAddsU16,
  kSubsI8,
  kSubsU8,
  kSubsI16,
  kSubsU16,
  kMulU16,
  kMulU32,
  kMulU64,
  kMulhI16,
  kMulhU16,
  kMulU64_LoU32,
  kMHAddI16_I32,
  kMinI8,
  kMinU8,
  kMinI16,
  kMinU16,
  kMinI32,
  kMinU32,
  kMinI64,
  kMinU64,
  kMaxI8,
  kMaxU8,
  kMaxI16,
  kMaxU16,
  kMaxI32,
  kMaxU32,
  kMaxI64,
  kMaxU64,
  kCmpEqU8,
  kCmpEqU16,
  kCmpEqU32,
  kCmpEqU64,
  kCmpGtI8,
  kCmpGtU8,
  kCmpGtI16,
  kCmpGtU16,
  kCmpGtI32,
  kCmpGtU32,
  kCmpGtI64,
  kCmpGtU64,
  kCmpGeI8,
  kCmpGeU8,
  kCmpGeI16,
  kCmpGeU16,
  kCmpGeI32,
  kCmpGeU32,
  kCmpGeI64,
  kCmpGeU64,
  kCmpLtI8,
  kCmpLtU8,
  kCmpLtI16,
  kCmpLtU16,
  kCmpLtI32,
  kCmpLtU32,
  kCmpLtI64,
  kCmpLtU64,
  kCmpLeI8,
  kCmpLeU8,
  kCmpLeI16,
  kCmpLeU16,
  kCmpLeI32,
  kCmpLeU32,
  kCmpLeI64,
  kCmpLeU64,

  kAndF32,
  kAndF64,
  kOrF32,
  kOrF64,
  kXorF32,
  kXorF64,
  kAndnF32,
  kAndnF64,
  kBicF32,
  kBicF64,
  kAddF32S,
  kAddF64S,
  kAddF32,
  kAddF64,
  kSubF32S,
  kSubF64S,
  kSubF32,
  kSubF64,
  kMulF32S,
  kMulF64S,
  kMulF32,
  kMulF64,
  kDivF32S,
  kDivF64S,
  kDivF32,
  kDivF64,
  kMinF32S,
  kMinF64S,
  kMinF32,
  kMinF64,
  kMaxF32S,
  kMaxF64S,
  kMaxF32,
  kMaxF64,
  kCmpEqF32S,
  kCmpEqF64S,
  kCmpEqF32,
  kCmpEqF64,
  kCmpNeF32S,
  kCmpNeF64S,
  kCmpNeF32,
  kCmpNeF64,
  kCmpGtF32S,
  kCmpGtF64S,
  kCmpGtF32,
  kCmpGtF64,
  kCmpGeF32S,
  kCmpGeF64S,
  kCmpGeF32,
  kCmpGeF64,
  kCmpLtF32S,
  kCmpLtF64S,
  kCmpLtF32,
  kCmpLtF64,
  kCmpLeF32S,
  kCmpLeF64S,
  kCmpLeF32,
  kCmpLeF64,
  kCmpOrdF32S,
  kCmpOrdF64S,
  kCmpOrdF32,
  kCmpOrdF64,
  kCmpUnordF32S,
  kCmpUnordF64S,
  kCmpUnordF32,
  kCmpUnordF64,

  kHAddF64,

  kCombineLoHiU64,
  kCombineLoHiF64,
  kCombineHiLoU64,
  kCombineHiLoF64,

  kInterleaveLoU8,
  kInterleaveHiU8,
  kInterleaveLoU16,
  kInterleaveHiU16,
  kInterleaveLoU32,
  kInterleaveHiU32,
  kInterleaveLoU64,
  kInterleaveHiU64,
  kInterleaveLoF32,
  kInterleaveHiF32,
  kInterleaveLoF64,
  kInterleaveHiF64,

  kPacksI16_I8,
  kPacksI16_U8,
  kPacksI32_I16,
  kPacksI32_U16,

  kSwizzlev_U8,

#if defined(ASMJIT_UJIT_AARCH64)

  kMulwLoI8,
  kMulwLoU8,
  kMulwHiI8,
  kMulwHiU8,
  kMulwLoI16,
  kMulwLoU16,
  kMulwHiI16,
  kMulwHiU16,
  kMulwLoI32,
  kMulwLoU32,
  kMulwHiI32,
  kMulwHiU32,

  kMAddwLoI8,
  kMAddwLoU8,
  kMAddwHiI8,
  kMAddwHiU8,
  kMAddwLoI16,
  kMAddwLoU16,
  kMAddwHiI16,
  kMAddwHiU16,
  kMAddwLoI32,
  kMAddwLoU32,
  kMAddwHiI32,
  kMAddwHiU32,

  kMaxValue = kMAddwHiU32

#elif defined(ASMJIT_UJIT_X86)

  kPermuteU8,
  kPermuteU16,
  kPermuteU32,
  kPermuteU64,

  kMaxValue = kPermuteU64

#else

  kMaxValue = kSwizzlev_U8

#endif // ASMJIT_UJIT_AARCH64
};

enum class UniOpVVVI : uint32_t {
  kAlignr_U128,
  kInterleaveShuffleU32x4,
  kInterleaveShuffleU64x2,
  kInterleaveShuffleF32x4,
  kInterleaveShuffleF64x2,
  kInsertV128_U32,
  kInsertV128_F32,
  kInsertV128_U64,
  kInsertV128_F64,
  kInsertV256_U32,
  kInsertV256_F32,
  kInsertV256_U64,
  kInsertV256_F64,

  kMaxValue = kInsertV256_F64
};

enum class UniOpVVVV : uint32_t {
  kBlendV_U8,

  kMAddU16,
  kMAddU32,

  kMAddF32S,
  kMAddF64S,
  kMAddF32,
  kMAddF64,

  kMSubF32S,
  kMSubF64S,
  kMSubF32,
  kMSubF64,

  kNMAddF32S,
  kNMAddF64S,
  kNMAddF32,
  kNMAddF64,

  kNMSubF32S,
  kNMSubF64S,
  kNMSubF32,
  kNMSubF64,

  kMaxValue = kNMSubF64
};

//! Pipeline optimization flags used by \ref UniCompiler.
enum class UniOptFlags : uint32_t {
  //! No flags.
  kNone = 0x0u,

  //! CPU has instructions that can perform 8-bit masked loads and stores.
  kMaskOps8Bit = 0x00000001u,

  //! CPU has instructions that can perform 16-bit masked loads and stores.
  kMaskOps16Bit = 0x00000002u,

  //! CPU has instructions that can perform 32-bit masked loads and stores.
  kMaskOps32Bit = 0x00000004u,

  //! CPU has instructions that can perform 64-bit masked loads and stores.
  kMaskOps64Bit = 0x00000008u,

  //! CPU provides low-latency 32-bit multiplication (AMD CPUs).
  kFastVpmulld = 0x00000010u,

  //! CPU provides low-latency 64-bit multiplication (AMD CPUs).
  kFastVpmullq = 0x00000020u,

  //! CPU performs hardware gathers faster than a sequence of loads and packing.
  kFastGather = 0x00000040u,

  //! CPU has fast stores with mask.
  //!
  //! \note This is a hint to the compiler to emit a masked store instead of a sequence having branches.
  kFastStoreWithMask = 0x00000080u
};
ASMJIT_DEFINE_ENUM_FLAGS(UniOptFlags)

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_UJIT
#endif // ASMJIT_UJIT_UNIOP_H_INCLUDED
