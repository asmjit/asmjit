// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_ARM_ARMEMITTER_H
#define _ASMJIT_ARM_ARMEMITTER_H

#include "../core/emitter.h"
#include "../core/support.h"
#include "../arm/arminstdb.h"
#include "../arm/armoperand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(arm)

//! \addtogroup asmjit_arm_api
//! \{

// ============================================================================
// [asmjit::A32EmitterT / A64EmitterT]
// ============================================================================

#define ASMJIT_EMIT static_cast<This*>(this)->emit

#define ASMJIT_INST_0x(NAME, ID) \
  inline Error NAME() { return ASMJIT_EMIT(ArmInst::kId##ID); }

#define ASMJIT_INST_1x(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return ASMJIT_EMIT(ArmInst::kId##ID, o0); }

#define ASMJIT_INST_1i(NAME, ID, T0) \
  inline Error NAME(const T0& o0) { return ASMJIT_EMIT(ArmInst::kId##ID, o0); } \
  inline Error NAME(int o0) { return ASMJIT_EMIT(ArmInst::kId##ID, Support::asInt(o0)); } \
  inline Error NAME(unsigned int o0) { return ASMJIT_EMIT(ArmInst::kId##ID, Support::asInt(o0)); } \
  inline Error NAME(int64_t o0) { return ASMJIT_EMIT(ArmInst::kId##ID, Support::asInt(o0)); } \
  inline Error NAME(uint64_t o0) { return ASMJIT_EMIT(ArmInst::kId##ID, Support::asInt(o0)); }

#define ASMJIT_INST_2x(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1); }

#define ASMJIT_INST_2i(NAME, ID, T0, T1) \
  inline Error NAME(const T0& o0, const T1& o1) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1); } \
  inline Error NAME(const T0& o0, int o1) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, Support::asInt(o1)); } \
  inline Error NAME(const T0& o0, unsigned int o1) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, Support::asInt(o1)); } \
  inline Error NAME(const T0& o0, int64_t o1) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, Support::asInt(o1)); } \
  inline Error NAME(const T0& o0, uint64_t o1) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, Support::asInt(o1)); }

#define ASMJIT_INST_3x(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2); }

#define ASMJIT_INST_3i(NAME, ID, T0, T1, T2) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2); } \
  inline Error NAME(const T0& o0, const T1& o1, int o2) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, Support::asInt(o2)); } \
  inline Error NAME(const T0& o0, const T1& o1, unsigned int o2) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, Support::asInt(o2)); } \
  inline Error NAME(const T0& o0, const T1& o1, int64_t o2) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, Support::asInt(o2)); } \
  inline Error NAME(const T0& o0, const T1& o1, uint64_t o2) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, Support::asInt(o2)); }

#define ASMJIT_INST_4x(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2, o3); }

#define ASMJIT_INST_4i(NAME, ID, T0, T1, T2, T3) \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, const T3& o3) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2, o3); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, int o3) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2, Support::asInt(o3)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, unsigned int o3) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2, Support::asInt(o3)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, int64_t o3) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2, Support::asInt(o3)); } \
  inline Error NAME(const T0& o0, const T1& o1, const T2& o2, uint64_t o3) { return ASMJIT_EMIT(ArmInst::kId##ID, o0, o1, o2, Support::asInt(o3)); }

//! ARM32 emitter.
//!
//! NOTE: This class cannot be instantiated, you can only cast to it and use it
//! as emitter that emits to either A32Assembler, A32Builder, or A32Compiler (use
//! with caution with A32Compiler as it expects virtual registers to be used).
template<typename This>
class A32EmitterT {
  inline A32EmitterT() noexcept {}

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

protected:
  inline This& _addInstOptions(uint32_t options) noexcept {
    static_cast<This*>(this)->addInstOptions(options);
    return *static_cast<This*>(this);
  }

public:
  // --------------------------------------------------------------------------
  // [ARM Instructions]
  // --------------------------------------------------------------------------



  // --------------------------------------------------------------------------
  // [NEON Instructions]
  // --------------------------------------------------------------------------


};

#undef ASMJIT_INST_0x
#undef ASMJIT_INST_1x
#undef ASMJIT_INST_1i
#undef ASMJIT_INST_2x
#undef ASMJIT_INST_2i
#undef ASMJIT_INST_3x
#undef ASMJIT_INST_3i
#undef ASMJIT_INST_4x
#undef ASMJIT_INST_4i
#undef ASMJIT_EMIT

// ============================================================================
// [asmjit::A32Emitter / A64Emitter]
// ============================================================================

template<typename This>
class A32Emitter : public A32EmitterT<This> {
  ASMJIT_NONCONSTRUCTIBLE(A32Emitter)

public:
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_ARM_ARMEMITTER_H
