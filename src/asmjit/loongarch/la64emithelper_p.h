// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_A64EMITHELPER_P_H_INCLUDED
#define ASMJIT_LA_A64EMITHELPER_P_H_INCLUDED

#include "../core/api-config.h"

#include "../core/emithelper_p.h"
#include "../core/func.h"
#include "../loongarch/la64emitter.h"
#include "../loongarch/la64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

class EmitHelper : public BaseEmitHelper {
public:
  ASMJIT_INLINE_NODEBUG explicit EmitHelper(BaseEmitter* emitter = nullptr) noexcept
    : BaseEmitHelper(emitter) {}

  Error emitRegMove(
    const Operand_& dst_,
    const Operand_& src_, TypeId typeId, const char* comment = nullptr) override;

  Error emitRegSwap(
    const BaseReg& a,
    const BaseReg& b, const char* comment = nullptr) override;

  Error emitArgMove(
    const BaseReg& dst_, TypeId dstTypeId,
    const Operand_& src_, TypeId srcTypeId, const char* comment = nullptr) override;

  Error emitProlog(const FuncFrame& frame);
  Error emitEpilog(const FuncFrame& frame);
};

void assignEmitterFuncs(BaseEmitter* emitter);

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_LA_A64EMITHELPER_P_H_INCLUDED