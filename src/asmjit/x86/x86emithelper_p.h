// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED
#define ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED

#include "../core/api-config.h"

#include "../core/emithelper_p.h"
#include "../core/func.h"
#include "../x86/x86emitter.h"
#include "../x86/x86instapi_p.h"
#include "../x86/x86operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG RegType vecTypeIdToRegType(TypeId typeId) noexcept {
  return uint32_t(typeId) <= uint32_t(TypeId::_kVec128End) ? RegType::kVec128 :
         uint32_t(typeId) <= uint32_t(TypeId::_kVec256End) ? RegType::kVec256 : RegType::kVec512;
}

class EmitHelper : public BaseEmitHelper {
public:
  bool _avxEnabled;
  bool _avx512Enabled;

  ASMJIT_INLINE_NODEBUG explicit EmitHelper(BaseEmitter* emitter = nullptr, bool avxEnabled = false, bool avx512Enabled = false) noexcept
    : BaseEmitHelper(emitter),
      _avxEnabled(avxEnabled || avx512Enabled),
      _avx512Enabled(avx512Enabled) {}

  ASMJIT_INLINE_NODEBUG virtual ~EmitHelper() noexcept = default;

  Error emitRegMove(
    const Operand_& dst_,
    const Operand_& src_, TypeId typeId, const char* comment = nullptr) override;

  Error emitArgMove(
    const Reg& dst_, TypeId dstTypeId,
    const Operand_& src_, TypeId srcTypeId, const char* comment = nullptr) override;

  Error emitRegSwap(
    const Reg& a,
    const Reg& b, const char* comment = nullptr) override;

  Error emitProlog(const FuncFrame& frame);
  Error emitEpilog(const FuncFrame& frame);
};

void initEmitterFuncs(BaseEmitter* emitter) noexcept;

static ASMJIT_INLINE void updateEmitterFuncs(BaseEmitter* emitter) noexcept {
#ifndef ASMJIT_NO_VALIDATION
  emitter->_funcs.validate = emitter->is32Bit() ? InstInternal::validateX86 : InstInternal::validateX64;
#else
  DebugUtils::unused(emitter);
#endif
}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_X86_X86EMITHELPER_P_H_INCLUDED
