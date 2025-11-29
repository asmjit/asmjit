// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH32)

#include <asmjit/core/formatter.h>
#include <asmjit/core/funcargscontext_p.h>
#include <asmjit/core/string.h>
#include <asmjit/core/type.h>
#include <asmjit/support/support.h>
#include <asmjit/arm/a32emithelper_p.h>
#include <asmjit/arm/a32formatter_p.h>
#include <asmjit/arm/a32operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

// a32::EmitHelper - Emit Operations
// =================================

ASMJIT_FAVOR_SIZE Error EmitHelper::emit_reg_move(
  const Operand_& dst_,
  const Operand_& src_, TypeId type_id, const char* comment) {

  Emitter* emitter = _emitter->as<Emitter>();

  // Invalid or abstract TypeIds are not allowed.
  ASMJIT_ASSERT(TypeUtils::is_valid(type_id) && !TypeUtils::is_abstract(type_id));

  emitter->set_inline_comment(comment);

  if (dst_.is_reg() && src_.is_mem()) {
    Reg dst(dst_.as<Reg>());
    Mem src(src_.as<Mem>());

    switch (type_id) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
        return emitter->ldrb(dst.as<Gp>(), src);

      case TypeId::kInt16:
      case TypeId::kUInt16:
        return emitter->ldrh(dst.as<Gp>(), src);

      case TypeId::kInt32:
      case TypeId::kUInt32:
        return emitter->ldr(dst.as<Gp>(), src);

      default: {
        if (TypeUtils::is_float32(type_id) || TypeUtils::is_vec32(type_id))
          return emitter->vldr_32(dst.as<Vec>().s(), src);

        if (TypeUtils::is_float64(type_id) || TypeUtils::is_vec64(type_id))
          return emitter->vldr_64(dst.as<Vec>().d(), src);

        // TODO: AArch32.
        // if (TypeUtils::is_vec128(type_id))
        //   return emitter->vldr(dst.as<Vec>().q(), src);

        break;
      }
    }
  }

  if (dst_.is_mem() && src_.is_reg()) {
    Mem dst(dst_.as<Mem>());
    Reg src(src_.as<Reg>());

    switch (type_id) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
        return emitter->strb(src.as<Gp>(), dst);

      case TypeId::kInt16:
      case TypeId::kUInt16:
        return emitter->strh(src.as<Gp>(), dst);

      case TypeId::kInt32:
      case TypeId::kUInt32:
        return emitter->str(src.as<Gp>(), dst);

      default: {
        if (TypeUtils::is_float32(type_id) || TypeUtils::is_vec32(type_id))
          return emitter->vstr_32(src.as<Vec>().s(), dst);

        if (TypeUtils::is_float64(type_id) || TypeUtils::is_vec64(type_id))
          return emitter->vstr_64(src.as<Vec>().d(), dst);

        // TODO: AArch32
        // if (TypeUtils::isVec128(type_id))
        //   return emitter->vstr(src.as<Vec>().q(), dst);

        break;
      }
    }
  }

  if (dst_.is_reg() && src_.is_reg()) {
    Reg dst(dst_.as<Reg>());
    Reg src(src_.as<Reg>());

    switch (type_id) {
      case TypeId::kInt8:
      case TypeId::kUInt8:
      case TypeId::kInt16:
      case TypeId::kUInt16:
      case TypeId::kInt32:
      case TypeId::kUInt32:
        return emitter->mov(src.as<Gp>(), dst.as<Gp>());

      default: {
        if (TypeUtils::is_float32(type_id) || TypeUtils::is_vec32(type_id))
          return emitter->vmov(dst.as<Vec>().s(), src.as<Vec>().s());

        if (TypeUtils::is_float64(type_id) || TypeUtils::is_vec64(type_id))
          return emitter->vmov(dst.as<Vec>().d(), src.as<Vec>().d());

        if (TypeUtils::is_vec128(type_id))
          return emitter->vmov(dst.as<Vec>().q(), src.as<Vec>().q());

        break;
      }
    }
  }

  emitter->set_inline_comment(nullptr);
  return make_error(Error::kInvalidState);
}

Error EmitHelper::emit_reg_swap(
  const Reg& a,
  const Reg& b, const char* comment) {

  Support::maybe_unused(a, b, comment);
  return make_error(Error::kInvalidState);
}

Error EmitHelper::emit_arg_move(
  const Reg& dst_, TypeId dst_type_id,
  const Operand_& src_, TypeId src_type_id, const char* comment) {

  // TODO: AArch32 - EmitArgMove is unfinished.
  Support::maybe_unused(dst_, dst_type_id, src_, src_type_id, comment);
  return make_error(Error::kInvalidState);
}

// a32::EmitHelper - Emit Prolog & Epilog
// ======================================

struct LoadStoreInstructions {
  InstId singleInstId;
  InstId pairInstId;
};

/*
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

  static const Support::Array<Reg, 2> groupRegs = {{ x0, d0 }};
  static const Support::Array<LoadStoreInstructions, 2> groupInsts = {{
    { Inst::kIdStr  , Inst::kIdStp   },
    { Inst::kIdStr_v, Inst::kIdStp_v }
  }};

  uint32_t adjustInitialOffset = pei.sizeTotal;

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

      if (pair.offset == 0 && adjustInitialOffset) {
        mem.setOffset(-int(adjustInitialOffset));
        mem.makePreIndex();
      }

      if (pair.ids[1] == BaseReg::kIdBad)
        ASMJIT_PROPAGATE(emitter->emit(insts.singleInstId, regs[0], mem));
      else
        ASMJIT_PROPAGATE(emitter->emit(insts.pairInstId, regs[0], regs[1], mem));

      mem.resetToFixedOffset();

      if (i == 0 && frame.hasPreservedFP()) {
        ASMJIT_PROPAGATE(emitter->mov(x29, sp));
      }
    }
  }

  if (frame.hasStackAdjustment()) {
    uint32_t adj = frame.stackAdjustment();
    if (adj <= 0xFFFu) {
      ASMJIT_PROPAGATE(emitter->sub(sp, sp, adj));
    }
    else if (adj <= 0xFFFFFFu)  {
      // TODO: [ARM] Prolog - we must touch the pages otherwise it's undefined.
      ASMJIT_PROPAGATE(emitter->sub(sp, sp, adj & 0x000FFFu));
      ASMJIT_PROPAGATE(emitter->sub(sp, sp, adj & 0xFFF000u));
    }
    else {
      return make_error(Error::kInvalidState);
    }
  }

  return kErrorOk;
}

// TODO: [ARM] Emit epilog.
ASMJIT_FAVOR_SIZE Error EmitHelper::emitEpilog(const FuncFrame& frame) {
  Emitter* emitter = _emitter->as<Emitter>();

  PrologEpilogInfo pei;
  ASMJIT_PROPAGATE(pei.init(frame));

  static const Support::Array<Reg, 2> groupRegs = {{ x0, d0 }};
  static const Support::Array<LoadStoreInstructions, 2> groupInsts = {{
    { Inst::kIdLdr  , Inst::kIdLdp   },
    { Inst::kIdLdr_v, Inst::kIdLdp_v }
  }};

  uint32_t adjustInitialOffset = pei.sizeTotal;

  if (frame.hasStackAdjustment()) {
    uint32_t adj = frame.stackAdjustment();
    if (adj <= 0xFFFu) {
      ASMJIT_PROPAGATE(emitter->add(sp, sp, adj));
    }
    else if (adj <= 0xFFFFFFu)  {
      ASMJIT_PROPAGATE(emitter->add(sp, sp, adj & 0x000FFFu));
      ASMJIT_PROPAGATE(emitter->add(sp, sp, adj & 0xFFF000u));
    }
    else {
      return make_error(Error::kInvalidState);
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

      if (pair.offset == 0 && adjustInitialOffset) {
        mem.setOffset(int(adjustInitialOffset));
        mem.makePostIndex();
      }

      if (pair.ids[1] == BaseReg::kIdBad)
        ASMJIT_PROPAGATE(emitter->emit(insts.singleInstId, regs[0], mem));
      else
        ASMJIT_PROPAGATE(emitter->emit(insts.pairInstId, regs[0], regs[1], mem));

      mem.resetToFixedOffset();
    }
  }

  ASMJIT_PROPAGATE(emitter->ret(x30));

  return kErrorOk;
}
*/

static Error ASMJIT_CDECL Emitter_emit_prolog(BaseEmitter* emitter, const FuncFrame& frame) {
  // EmitHelper emitHelper(emitter);
  // return emitHelper.emitProlog(frame);
  Support::maybe_unused(emitter, frame);
  return make_error(Error::kInvalidState);
}

static Error ASMJIT_CDECL Emitter_emit_epilog(BaseEmitter* emitter, const FuncFrame& frame) {
  // EmitHelper emitHelper(emitter);
  // return emitHelper.emitEpilog(frame);
  Support::maybe_unused(emitter, frame);
  return make_error(Error::kInvalidState);
}

static Error ASMJIT_CDECL Emitter_emit_args_assignment(BaseEmitter* emitter, const FuncFrame& frame, const FuncArgsAssignment& args) {
  // EmitHelper emitHelper(emitter);
  // return emitHelper.emitArgsAssignment(frame, args);
  Support::maybe_unused(emitter, frame, args);
  return make_error(Error::kInvalidState);
}

void assignEmitterFuncs(BaseEmitter* emitter) {
  emitter->_funcs.emit_prolog = Emitter_emit_prolog;
  emitter->_funcs.emit_epilog = Emitter_emit_epilog;
  emitter->_funcs.emit_args_assignment = Emitter_emit_args_assignment;

#ifndef ASMJIT_NO_LOGGING
  emitter->_funcs.format_instruction = FormatterInternal::format_instruction;
#endif

#ifndef ASMJIT_NO_VALIDATION
  // TODO: AArch32.
  // emitter->_funcs.validate = InstInternal::validate;
#endif
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH32
