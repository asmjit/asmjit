// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_LOONGARCH64)

#include "../core/formatter.h"
#include "../core/funcargscontext_p.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../loongarch/la64emithelper_p.h"
#include "../loongarch/la64formatter_p.h"
#include "../loongarch/la64instapi_p.h"
#include "../loongarch/la64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

// a64::EmitHelper - Emit Operations
// =================================

#define EMIT_LD(df, dst, src) \
{ \
    if(RegType::kNone == src.indexType()) { \
        return emitter->ld_##df(dst.as<Gp>(), src); \
    } else { \
        return emitter->ldx_##df(dst.as<Gp>(), src); \
    } \
}

#define EMIT_ST(df, dst, src) \
{ \
    if(RegType::kNone == dst.indexType()) { \
        return emitter->st_##df(src.as<Gp>(), dst); \
    } else { \
        return emitter->stx_##df(src.as<Gp>(), dst); \
    } \
}

#define EMIT_FLD(df, dst, src) \
{ \
    if(RegType::kNone == src.indexType()) { \
        return emitter->fld_##df(dst.as<Vec>(), src); \
    } else { \
        return emitter->fldx_##df(dst.as<Vec>(), src); \
    } \
}

#define EMIT_FST(df, dst, src) \
{ \
    if(RegType::kNone == dst.indexType()) { \
        return emitter->fst_##df(src.as<Vec>(), dst); \
    } else { \
        return emitter->fstx_##df(src.as<Vec>(), dst); \
    } \
}

#define EMIT_SIMD_LD(act, dst, src) \
{ \
    if(RegType::kNone == src.indexType()) { \
        return emitter->act(dst.as<Vec>(), src); \
    } else { \
        return emitter->act##x(dst.as<Vec>(), src); \
    } \
}

#define EMIT_SIMD_ST(act, dst, src) \
{ \
    if(RegType::kNone == dst.indexType()) { \
        return emitter->act(src.as<Vec>(), dst); \
    } else { \
        return emitter->act##x(src.as<Vec>(), dst); \
    } \
}

ASMJIT_FAVOR_SIZE Error EmitHelper::emitRegMove(
  const Operand_& dst_,
  const Operand_& src_, TypeId typeId, const char* comment) {

  Emitter* emitter = _emitter->as<Emitter>();

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeUtils::isValid(typeId) && !TypeUtils::isAbstract(typeId));

  emitter->setInlineComment(comment);

  if (dst_.isReg() && src_.isMem()) {
    Reg dst(dst_.as<Reg>());
    Mem src(src_.as<Mem>());

    switch (typeId) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
        EMIT_LD(b, dst, src);

      case TypeId::kInt16:
      case TypeId::kUInt16:
        EMIT_LD(h, dst, src);

      case TypeId::kInt32:
      case TypeId::kUInt32:
        EMIT_LD(w, dst, src);

      case TypeId::kInt64:
      case TypeId::kUInt64:
        EMIT_LD(d, dst, src);

      default: {
        if (TypeUtils::isFloat32(typeId) || TypeUtils::isVec32(typeId))
          EMIT_FLD(s, dst, src);

        if (TypeUtils::isFloat64(typeId) || TypeUtils::isVec64(typeId))
          EMIT_FLD(d, dst, src);

        if (TypeUtils::isVec128(typeId))
          EMIT_SIMD_LD(vld, dst, src);

        if (TypeUtils::isVec256(typeId))
          EMIT_SIMD_LD(xvld, dst, src);

        break;
      }
    }
  }

  if (dst_.isMem() && src_.isReg()) {
    Mem dst(dst_.as<Mem>());
    Reg src(src_.as<Reg>());

    switch (typeId) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
        EMIT_ST(b, dst, src);

      case TypeId::kInt16:
      case TypeId::kUInt16:
        EMIT_ST(h, dst, src);

      case TypeId::kInt32:
      case TypeId::kUInt32:
        EMIT_ST(w, dst, src);

      case TypeId::kInt64:
      case TypeId::kUInt64:
        EMIT_ST(d, dst, src);

      default: {
        if (TypeUtils::isFloat32(typeId) || TypeUtils::isVec32(typeId))
          EMIT_FST(s, dst, src);

        if (TypeUtils::isFloat64(typeId) || TypeUtils::isVec64(typeId))
          EMIT_FST(d, dst, src);

        if (TypeUtils::isVec128(typeId))
          EMIT_SIMD_ST(vst, dst, src);

        if (TypeUtils::isVec256(typeId))
          EMIT_SIMD_ST(xvst, dst, src);

        break;
      }
    }
  }

  if (dst_.isReg() && src_.isReg()) {
    Reg dst(dst_.as<Reg>());
    Reg src(src_.as<Reg>());

    switch (typeId) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
      case TypeId::kInt16:
      case TypeId::kUInt16:
      case TypeId::kInt32:
      case TypeId::kUInt32:
      case TypeId::kInt64:
      case TypeId::kUInt64:
        return emitter->move(dst.as<Gp>().x(), src.as<Gp>().x());

      default: {
        if (TypeUtils::isFloat32(typeId) || TypeUtils::isVec32(typeId))
          return emitter->fmov_s(dst.as<Vec>().s(), src.as<Vec>().s());

        if (TypeUtils::isFloat64(typeId) || TypeUtils::isVec64(typeId))
          return emitter->fmov_d(dst.as<Vec>().b8(), src.as<Vec>().b8());

        if (TypeUtils::isVec128(typeId))
          return emitter->vor_v(dst.as<Vec>(), src.as<Vec>(), src.as<Vec>());

        if (TypeUtils::isVec256(typeId))
          return emitter->xvor_v(dst.as<Vec>(), src.as<Vec>(), src.as<Vec>());

        break;
      }
    }
  }

  emitter->setInlineComment(nullptr);
  return DebugUtils::errored(kErrorInvalidState);
}

Error EmitHelper::emitRegSwap(
  const BaseReg& a,
  const BaseReg& b, const char* comment) {

  DebugUtils::unused(a, b, comment);
  return DebugUtils::errored(kErrorInvalidState);
}

// TODO: [LA] EmitArgMove is unfinished.
Error EmitHelper::emitArgMove(
  const BaseReg& dst_, TypeId dstTypeId,
  const Operand_& src_, TypeId srcTypeId, const char* comment) {

  // Deduce optional `dstTypeId`, which may be `TypeId::kVoid` in some cases.
  if (dstTypeId == TypeId::kVoid) {
    const ArchTraits& archTraits = ArchTraits::byArch(_emitter->arch());
    dstTypeId = archTraits.regTypeToTypeId(dst_.type());
  }

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeUtils::isValid(dstTypeId) && !TypeUtils::isAbstract(dstTypeId));
  ASMJIT_ASSERT(TypeUtils::isValid(srcTypeId) && !TypeUtils::isAbstract(srcTypeId));

  Reg dst(dst_.as<Reg>());
  Operand src(src_);

  uint32_t dstSize = TypeUtils::sizeOf(dstTypeId);
  uint32_t srcSize = TypeUtils::sizeOf(srcTypeId);

  if (TypeUtils::isInt(dstTypeId)) {
    if (TypeUtils::isInt(srcTypeId)) {
      uint32_t x = uint32_t(dstSize == 8);

      dst.setSignature(OperandSignature{x ? uint32_t(GpX::kSignature) : uint32_t(GpW::kSignature)});
      _emitter->setInlineComment(comment);

      if (src.isReg()) {
        src.setSignature(dst.signature());
        return _emitter->emit(Inst::kIdMove, dst, src);
      }
      else if (src.isMem()) {
        InstId instId = Inst::kIdNone;
          switch (srcTypeId) {
          case TypeId::kInt8: instId = Inst::kIdLd_b; break;
          case TypeId::kUInt8: instId = Inst::kIdLd_b; break;
          case TypeId::kInt16: instId = Inst::kIdLd_h; break;
          case TypeId::kUInt16: instId = Inst::kIdLd_h; break;
          case TypeId::kInt32: instId = x ? Inst::kIdLd_w : Inst::kIdLd_w; break;  //FIXME
          case TypeId::kUInt32: instId = Inst::kIdLd_w; break;
          case TypeId::kInt64: instId = Inst::kIdLd_d; break;
          case TypeId::kUInt64: instId = Inst::kIdLd_d; break;
          default:
            return DebugUtils::errored(kErrorInvalidState);
        }
        return _emitter->emit(instId, dst, src);
      }
    }
  }

  if (TypeUtils::isFloat(dstTypeId) || TypeUtils::isVec(dstTypeId)) {
    if (TypeUtils::isFloat(srcTypeId) || TypeUtils::isVec(srcTypeId)) {
      switch (srcSize) {
        case 2: dst.as<Vec>().setSignature(OperandSignature{VecH::kSignature}); break;
        case 4: dst.as<Vec>().setSignature(OperandSignature{VecS::kSignature}); break;
        case 8: dst.as<Vec>().setSignature(OperandSignature{VecD::kSignature}); break;
        case 16: dst.as<Vec>().setSignature(OperandSignature{VecV::kSignature}); break;
        default:
          return DebugUtils::errored(kErrorInvalidState);
      }

      _emitter->setInlineComment(comment);

      if (src.isReg()) {
        InstId instId = srcSize <= 4 ? Inst::kIdVand_v : Inst::kIdVand_v;  //FIXME
        src.setSignature(dst.signature());
        return _emitter->emit(instId, dst, src, src);
      }
      else if (src.isMem()) {
        return _emitter->emit(Inst::kIdVld, dst, src);
      }
    }
  }

  return DebugUtils::errored(kErrorInvalidState);
}

// a64::EmitHelper - Emit Prolog & Epilog
// ======================================

struct LoadStoreInstructions {
  InstId singleInstId;
  InstId pairInstId;
};

struct PrologEpilogInfo {
  struct RegPair {
    uint8_t ids[2];
    uint16_t offset;
  };

  struct GroupData {
    RegPair pairs[16];
    uint32_t pairCount;
  };

  Support::Array<GroupData, 2> groups;
  uint32_t sizeTotal;

  Error init(const FuncFrame& frame) noexcept {
    uint32_t offset = 0;

    for (RegGroup group : Support::EnumValues<RegGroup, RegGroup::kGp, RegGroup::kVec>{}) {
      GroupData& data = groups[group];

      uint32_t n = 0;
      uint32_t pairCount = 0;
      RegPair* pairs = data.pairs;

      uint32_t slotSize = frame.saveRestoreRegSize(group);
      uint32_t savedRegs = frame.savedRegs(group);

      if (group == RegGroup::kGp && frame.hasPreservedFP()) {
        // Must be at the beginning of the push/pop sequence.
        ASMJIT_ASSERT(pairCount == 0);

        pairs[0].offset = uint16_t(offset);
        pairs[0].ids[0] = Gp::kIdFp;
        pairs[0].ids[1] = Gp::kIdLr;
        offset += slotSize * 2;
        pairCount++;

        savedRegs &= ~Support::bitMask(Gp::kIdFp, Gp::kIdLr);
      }

      Support::BitWordIterator<uint32_t> it(savedRegs);
      while (it.hasNext()) {
        pairs[pairCount].ids[n] = uint8_t(it.next());

        if (++n == 2) {
          pairs[pairCount].offset = uint16_t(offset);
          offset += slotSize * 2;

          n = 0;
          pairCount++;
        }
      }

      if (n == 1) {
        pairs[pairCount].ids[1] = uint8_t(BaseReg::kIdBad);
        pairs[pairCount].offset = uint16_t(offset);
        offset += slotSize * 2;
        pairCount++;
      }

      data.pairCount = pairCount;
    }

    sizeTotal = offset;
    return kErrorOk;
  }
};

ASMJIT_FAVOR_SIZE Error EmitHelper::emitProlog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();

  PrologEpilogInfo pei;
  ASMJIT_PROPAGATE(pei.init(frame));

  static const Support::Array<Reg, 2> groupRegs = {{ a0, a0 }};  //FIXME
  static const Support::Array<LoadStoreInstructions, 2> groupInsts = {{
    { Inst::kIdSt_d  , Inst::kIdSt_d   },   //FIXME
    { Inst::kIdVst, Inst::kIdVst }
  }};

  // Emit: 'bti' (indirect branch protection).
  if (frame.hasIndirectBranchProtection()) {
    // TODO: The instruction is not available at the moment (would be ABI break).
    // ASMJIT_PROPAGATE(emitter->bti());
  }

  for (RegGroup group : Support::EnumValues<RegGroup, RegGroup::kGp, RegGroup::kVec>{}) {
    const PrologEpilogInfo::GroupData& data = pei.groups[group];
    uint32_t pairCount = data.pairCount;

    Reg regs[2] = { groupRegs[group], groupRegs[group] };
    Mem mem = ptr(sp);

    const LoadStoreInstructions& insts = groupInsts[group];
    for (uint32_t i = 0; i < pairCount; i++) {
      const PrologEpilogInfo::RegPair& pair = data.pairs[i];

      regs[0].setId(pair.ids[0]);
      regs[1].setId(pair.ids[1]);
      mem.setOffsetLo32(pair.offset);

      /* if (pair.offset == 0 && adjustInitialOffset) { */
      /*   mem.setOffset(-int(adjustInitialOffset)); */
      /*   mem.makePreIndex(); */
      /* } */

      if (pair.ids[1] == BaseReg::kIdBad)
        ASMJIT_PROPAGATE(emitter->emit(insts.singleInstId, regs[0], mem));
      else
        ASMJIT_PROPAGATE(emitter->emit(insts.pairInstId, regs[0], regs[1], mem));

      mem.resetOffsetMode();

      if (i == 0 && frame.hasPreservedFP()) {
        ASMJIT_PROPAGATE(emitter->move(fp, sp));
      }
    }
  }

  if (frame.hasStackAdjustment()) {
    uint32_t adj = frame.stackAdjustment();
    if (adj <= 0xFFFu) {
      ASMJIT_PROPAGATE(emitter->addi_d(sp, sp, -adj));
    }
    else if (adj <= 0xFFFFFFu)  {
      ASMJIT_PROPAGATE(emitter->addi_d(sp, sp, -(adj & 0x000FFFu)));
      ASMJIT_PROPAGATE(emitter->addi_d(sp, sp, -(adj & 0xFFF000u)));
    }
    else {
      return DebugUtils::errored(kErrorInvalidState);
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error EmitHelper::emitEpilog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();

  PrologEpilogInfo pei;
  ASMJIT_PROPAGATE(pei.init(frame));

  static const Support::Array<Reg, 2> groupRegs = {{ a0, a1 }};   //FXIME
  static const Support::Array<LoadStoreInstructions, 2> groupInsts = {{
    { Inst::kIdLd_d  , Inst::kIdLd_d   },
    { Inst::kIdVld, Inst::kIdVld }
  }};

  if (frame.hasStackAdjustment()) {
    uint32_t adj = frame.stackAdjustment();
    if (adj <= 0xFFFu) {
      ASMJIT_PROPAGATE(emitter->addi_d(sp, sp, adj));
    }
    else if (adj <= 0xFFFFFFu)  {
      ASMJIT_PROPAGATE(emitter->addi_d(sp, sp, adj & 0x000FFFu));
      ASMJIT_PROPAGATE(emitter->addi_d(sp, sp, adj & 0xFFF000u));
    }
    else {
      return DebugUtils::errored(kErrorInvalidState);
    }
  }

  for (int g = 1; g >= 0; g--) {
    RegGroup group = RegGroup(g);
    const PrologEpilogInfo::GroupData& data = pei.groups[group];
    uint32_t pairCount = data.pairCount;

    Reg regs[2] = { groupRegs[group], groupRegs[group] };
    Mem mem = ptr(sp);

    const LoadStoreInstructions& insts = groupInsts[group];

    for (int i = int(pairCount) - 1; i >= 0; i--) {
      const PrologEpilogInfo::RegPair& pair = data.pairs[i];

      regs[0].setId(pair.ids[0]);
      regs[1].setId(pair.ids[1]);
      mem.setOffsetLo32(pair.offset);

      if (pair.ids[1] == BaseReg::kIdBad)
        ASMJIT_PROPAGATE(emitter->emit(insts.singleInstId, regs[0], mem));
      else
        ASMJIT_PROPAGATE(emitter->emit(insts.pairInstId, regs[0], regs[1], mem));

      mem.resetOffsetMode();
    }
  }

  ASMJIT_PROPAGATE(emitter->jirl(zero, ra, 0));  //TODO CHANGE TO JR

  return kErrorOk;
}

static Error ASMJIT_CDECL Emitter_emitProlog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emitHelper(emitter);
  return emitHelper.emitProlog(frame);
}

static Error ASMJIT_CDECL Emitter_emitEpilog(BaseEmitter* emitter, const FuncFrame& frame) {
  EmitHelper emitHelper(emitter);
  return emitHelper.emitEpilog(frame);
}

static Error ASMJIT_CDECL Emitter_emitArgsAssignment(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  EmitHelper emitHelper(emitter);
  return emitHelper.emitArgsAssignment(frame, args);
}

void assignEmitterFuncs(BaseEmitter* emitter) {
  emitter->_funcs.emitProlog = Emitter_emitProlog;
  emitter->_funcs.emitEpilog = Emitter_emitEpilog;
  emitter->_funcs.emitArgsAssignment = Emitter_emitArgsAssignment;

#ifndef ASMJIT_NO_LOGGING
  emitter->_funcs.formatInstruction = FormatterInternal::formatInstruction;
#endif

#ifndef ASMJIT_NO_VALIDATION
  emitter->_funcs.validate = InstInternal::validate;
#endif
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOONGARCH64
