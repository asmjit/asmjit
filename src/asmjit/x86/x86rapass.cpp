// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../core/cpuinfo.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86instdb.h"
#include "../x86/x86internal_p.h"
#include "../x86/x86rapass_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

// ============================================================================
// [asmjit::x86::X86RAPass - Helpers]
// ============================================================================

static ASMJIT_INLINE uint64_t immMaskFromSize(uint32_t size) noexcept {
  ASMJIT_ASSERT(size > 0 && size < 256);
  static const uint64_t masks[] = {
    0x00000000000000FFu, //   1
    0x000000000000FFFFu, //   2
    0x00000000FFFFFFFFu, //   4
    0xFFFFFFFFFFFFFFFFu, //   8
    0x0000000000000000u, //  16
    0x0000000000000000u, //  32
    0x0000000000000000u, //  64
    0x0000000000000000u, // 128
    0x0000000000000000u  // 256
  };
  return masks[Support::ctz(size)];
}

// ============================================================================
// [asmjit::x86::X86OpInfo]
// ============================================================================

namespace X86OpInfo {
  enum {
    Any = BaseReg::kIdBad,
    Zax = Gp::kIdAx,
    Zbx = Gp::kIdBx,
    Zcx = Gp::kIdCx,
    Zdx = Gp::kIdDx,
    Zsi = Gp::kIdSi,
    Zdi = Gp::kIdDi
  };

  #define R(IDX) { uint16_t(OpInfo::kUse | OpInfo::kRead ), uint8_t(IDX), uint8_t(0) }
  #define W(IDX) { uint16_t(OpInfo::kOut | OpInfo::kWrite), uint8_t(IDX), uint8_t(0) }
  #define X(IDX) { uint16_t(OpInfo::kUse | OpInfo::kRW   ), uint8_t(IDX), uint8_t(0) }
  #define NONE() { uint16_t(0), uint8_t(Any), uint8_t(0) }
  #define DEFINE_OPS(NAME, ...) static const OpInfo NAME[6] = { __VA_ARGS__ }
  #define RETURN_OPS(...) do { DEFINE_OPS(ops, __VA_ARGS__); return ops; } while(0)

  // Common cases.
  DEFINE_OPS(op_r    , R(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_w    , W(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_x    , X(Any), R(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_xx   , X(Any), X(Any), R(Any), R(Any), R(Any), R(Any));
  DEFINE_OPS(op_w_all, W(Any), W(Any), W(Any), W(Any), W(Any), W(Any));

  static ASMJIT_INLINE const OpInfo* get(uint32_t instId, const InstDB::InstInfo& instInfo, const Operand* opArray, uint32_t opCount) noexcept {
    if (!instInfo.hasFixedRM()) {
      if (instInfo.isUseXX()) return op_xx;
      if (instInfo.isUseX()) return op_x;
      if (instInfo.isUseW()) return op_w;
      if (instInfo.isUseR()) return op_r;
    }
    else {
      switch (instId) {
        case Inst::kIdAaa:
        case Inst::kIdAad:
        case Inst::kIdAam:
        case Inst::kIdAas:
        case Inst::kIdDaa:
        case Inst::kIdDas:
          RETURN_OPS(X(Zax));

        case Inst::kIdCpuid:
          RETURN_OPS(X(Zax), W(Zbx), X(Zcx), W(Zdx));

        case Inst::kIdCbw:
        case Inst::kIdCdqe:
        case Inst::kIdCwde:
          RETURN_OPS(X(Zax));

        case Inst::kIdCdq:
        case Inst::kIdCwd:
        case Inst::kIdCqo:
          RETURN_OPS(W(Zdx), R(Zax));

        case Inst::kIdCmpxchg:
          RETURN_OPS(X(Any), R(Any), X(Zax));

        case Inst::kIdCmpxchg8b:
        case Inst::kIdCmpxchg16b:
          RETURN_OPS(NONE(), X(Zdx), X(Zax), R(Zcx), R(Zbx));

        case Inst::kIdDiv:
        case Inst::kIdIdiv:
          if (opCount == 2)
            RETURN_OPS(X(Zax), R(Any));
          else
            RETURN_OPS(X(Zdx), X(Zax), R(Any));

        case Inst::kIdImul:
          if (opCount == 2) {
            if (Reg::isGpw(opArray[0]) && opArray[1].size() == 1)
              RETURN_OPS(W(Zax), R(Any)); // imul ax, r8/m8
            else
              RETURN_OPS(X(Any), R(Any)); // imul r?, r?/m?
          }

          if (opCount == 3) {
            if (opArray[2].isImm())
              return op_w;
            else
              RETURN_OPS(W(Zdx), X(Zax), R(Any));
          }
          break;

        case Inst::kIdMul:
          if (opCount == 2)
            RETURN_OPS(X(Zax), R(Any));
          else
            RETURN_OPS(W(Zdx), X(Zax), R(Any));

        case Inst::kIdMulx:
          RETURN_OPS(W(Any), W(Any), R(Any), R(Zdx));

        case Inst::kIdJecxz:
        case Inst::kIdLoop:
        case Inst::kIdLoope:
        case Inst::kIdLoopne:
          RETURN_OPS(R(Zcx));

        case Inst::kIdLahf: RETURN_OPS(W(Zax));
        case Inst::kIdSahf: RETURN_OPS(R(Zax));

        case Inst::kIdRet: break;
        case Inst::kIdEnter: break;
        case Inst::kIdLeave: break;

        case Inst::kIdMonitor    : RETURN_OPS(R(Zax), R(Zcx), R(Zdx));
        case Inst::kIdMwait      : RETURN_OPS(R(Zax), R(Zcx));

        case Inst::kIdPush       : return op_r;
        case Inst::kIdPop        : return op_w;

        case Inst::kIdRcl:
        case Inst::kIdRcr:
        case Inst::kIdRol:
        case Inst::kIdRor:
        case Inst::kIdSal:
        case Inst::kIdSar:
        case Inst::kIdShl:
        case Inst::kIdShr:
          RETURN_OPS(X(Any), R(Zcx));

        case Inst::kIdShld:
        case Inst::kIdShrd:
          RETURN_OPS(X(Any), R(Any), R(Zcx));

        case Inst::kIdRdtsc:
        case Inst::kIdRdtscp:
          RETURN_OPS(W(Zdx), W(Zax), W(Zcx));

        case Inst::kIdXrstor:
        case Inst::kIdXrstor64:
        case Inst::kIdXsave:
        case Inst::kIdXsave64:
        case Inst::kIdXsaveopt:
        case Inst::kIdXsaveopt64:
          RETURN_OPS(W(Any), R(Zdx), R(Zax));

        case Inst::kIdXgetbv:
          RETURN_OPS(W(Zdx), W(Zax), R(Zcx));

        case Inst::kIdXsetbv:
          RETURN_OPS(R(Zdx), R(Zax), R(Zcx));

        case Inst::kIdIn  : RETURN_OPS(W(Zax), R(Zdx));
        case Inst::kIdIns : RETURN_OPS(X(Zdi), R(Zdx));
        case Inst::kIdOut : RETURN_OPS(R(Zdx), R(Zax));
        case Inst::kIdOuts: RETURN_OPS(R(Zdx), X(Zsi));

        case Inst::kIdCmps: RETURN_OPS(X(Zsi), X(Zdi));
        case Inst::kIdLods: RETURN_OPS(W(Zax), X(Zsi));
        case Inst::kIdMovs: RETURN_OPS(X(Zdi), X(Zsi));
        case Inst::kIdScas: RETURN_OPS(X(Zdi), R(Zax));
        case Inst::kIdStos: RETURN_OPS(X(Zdi), R(Zax));

        case Inst::kIdMaskmovq:
        case Inst::kIdMaskmovdqu:
        case Inst::kIdVmaskmovdqu:
          RETURN_OPS(R(Any), R(Any), R(Zdi));

        case Inst::kIdBlendvpd:
        case Inst::kIdBlendvps:
        case Inst::kIdPblendvb:
        case Inst::kIdSha256rnds2:
          RETURN_OPS(W(Any), R(Any), R(0));

        case Inst::kIdPcmpestri  :
        case Inst::kIdVpcmpestri : RETURN_OPS(R(Any), R(Any), NONE(), W(Zcx));
        case Inst::kIdPcmpistri  :
        case Inst::kIdVpcmpistri : RETURN_OPS(R(Any), R(Any), NONE(), W(Zcx), R(Zax), R(Zdx));
        case Inst::kIdPcmpestrm  :
        case Inst::kIdVpcmpestrm : RETURN_OPS(R(Any), R(Any), NONE(), W(0));
        case Inst::kIdPcmpistrm  :
        case Inst::kIdVpcmpistrm : RETURN_OPS(R(Any), R(Any), NONE(), W(0)  , R(Zax), R(Zdx));
      }
    }

    return op_x;
  }

  #undef RETURN_OPS
  #undef DEFINE_OPS
  #undef NONE
  #undef X
  #undef W
  #undef R
} // X86OpInfo namespace

// ============================================================================
// [asmjit::x86::X86RACFGBuilder]
// ============================================================================

class X86RACFGBuilder : public RACFGBuilder<X86RACFGBuilder> {
public:
  inline X86RACFGBuilder(X86RAPass* pass) noexcept
    : RACFGBuilder<X86RACFGBuilder>(pass),
      _is64Bit(pass->gpSize() == 8) {}

  inline Compiler* cc() const noexcept { return static_cast<Compiler*>(_cc); }

  Error newInst(InstNode** out, uint32_t instId, const OpInfo* opInfo, const uint32_t* physRegs, const Operand_& o0, const Operand_& o1) noexcept;
  Error onInst(InstNode* inst, uint32_t& controlType, RAInstBuilder& ib) noexcept;
  Error onCall(FuncCallNode* call, RAInstBuilder& ib) noexcept;
  Error moveImmToRegArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_, BaseReg* out) noexcept;
  Error moveImmToStackArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_) noexcept;
  Error moveRegToStackArg(FuncCallNode* call, const FuncValue& arg, const BaseReg& reg) noexcept;
  Error onRet(FuncRetNode* funcRet, RAInstBuilder& ib) noexcept;

  bool _is64Bit;
};

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - Utilities]
// ============================================================================

Error X86RACFGBuilder::newInst(InstNode** out, uint32_t instId, const OpInfo* opInfo, const uint32_t* physRegs, const Operand_& o0, const Operand_& o1) noexcept {
  InstNode* inst = cc()->newInstNode(instId, 0, o0, o1);
  if (ASMJIT_UNLIKELY(!inst))
    return DebugUtils::errored(kErrorNoHeapMemory);

  RAInstBuilder ib;
  uint32_t opCount = inst->opCount();

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = inst->opType(i);
    if (op.isReg()) {
      // Register operand.
      const Reg& reg = op.as<Reg>();
      uint32_t flags = opInfo[i].flags();

      uint32_t vIndex = Operand::unpackId(reg.id());
      if (vIndex < Operand::kPackedIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->group();
        uint32_t allocable = _pass->_availableRegs[group];

        uint32_t useId = BaseReg::kIdBad;
        uint32_t outId = BaseReg::kIdBad;

        uint32_t useRewriteMask = 0;
        uint32_t outRewriteMask = 0;

        if (opInfo[i].isUse()) {
          useId = physRegs[i];
          useRewriteMask = Support::mask(inst->getRewriteIndex(&reg._reg.id));
        }
        else {
          outId = physRegs[i];
          outRewriteMask = Support::mask(inst->getRewriteIndex(&reg._reg.id));
        }

        ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));
      }
    }
  }
  ASMJIT_PROPAGATE(_pass->assignRAInst(inst, _curBlock, ib));

  *out = inst;
  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - OnInst]
// ============================================================================

Error X86RACFGBuilder::onInst(InstNode* inst, uint32_t& controlType, RAInstBuilder& ib) noexcept {
  uint32_t instId = inst->id();
  uint32_t opCount = inst->opCount();

  const OpInfo* opInfo = nullptr;

  if (Inst::isDefinedId(instId)) {
    const InstDB::InstInfo& instInfo = InstDB::infoById(instId);
    bool hasGpbHiConstraint = false;
    uint32_t singleRegOps = 0;

    if (opCount) {
      const Operand* opArray = inst->operands();
      opInfo = X86OpInfo::get(instId, instInfo, opArray, opCount);

      for (uint32_t i = 0; i < opCount; i++) {
        const Operand& op = opArray[i];
        if (op.isReg()) {
          // Register operand.
          const Reg& reg = op.as<Reg>();
          uint32_t flags = opInfo[i].flags();
          uint32_t allowedRegs = 0xFFFFFFFFu;

          // X86-specific constraints related to LO|HI general purpose registers.
          if (reg.isGpb()) {
            flags |= RATiedReg::kX86Gpb;
            if (!_is64Bit) {
              // Restrict to first four - AL|AH|BL|BH|CL|CH|DL|DH. In 32-bit mode
              // it's not possible to access SIL|DIL, etc, so this is just enough.
              allowedRegs = 0x0Fu;
            }
            else {
              // If we encountered GPB-HI register the situation is much more
              // complicated than in 32-bit mode. We need to patch all registers
              // to not use ID higher than 7 and all GPB-LO registers to not use
              // index higher than 3. Instead of doing the patching here we just
              // set a flag and will do it later, to not complicate this loop.
              if (reg.isGpbHi()) {
                hasGpbHiConstraint = true;
                allowedRegs = 0x0Fu;
              }
            }
          }

          uint32_t vIndex = Operand::unpackId(reg.id());
          if (vIndex < Operand::kPackedIdCount) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

            uint32_t group = workReg->group();
            uint32_t allocable = _pass->_availableRegs[group] & allowedRegs;

            uint32_t useId = BaseReg::kIdBad;
            uint32_t outId = BaseReg::kIdBad;

            uint32_t useRewriteMask = 0;
            uint32_t outRewriteMask = 0;

            if (opInfo[i].isUse()) {
              useId = opInfo[i].physId();
              useRewriteMask = Support::mask(inst->getRewriteIndex(&reg._reg.id));
            }
            else {
              outId = opInfo[i].physId();
              outRewriteMask = Support::mask(inst->getRewriteIndex(&reg._reg.id));
            }

            ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));

            if (singleRegOps == i)
              singleRegOps++;
          }
        }
        else if (op.isMem()) {
          // Memory operand.
          const Mem& mem = op.as<Mem>();
          if (mem.isRegHome()) {
            RAWorkReg* workReg;
            ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(mem.baseId()), &workReg));
            _pass->getOrCreateStackSlot(workReg);
          }
          else if (mem.hasBaseReg()) {
            uint32_t vIndex = Operand::unpackId(mem.baseId());
            if (vIndex < Operand::kPackedIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              uint32_t group = workReg->group();
              uint32_t allocable = _pass->_availableRegs[group];

              uint32_t useId = BaseReg::kIdBad;
              uint32_t outId = BaseReg::kIdBad;

              uint32_t useRewriteMask = 0;
              uint32_t outRewriteMask = 0;

              useId = opInfo[i].physId();
              useRewriteMask = Support::mask(inst->getRewriteIndex(&mem._reg.id));

              uint32_t flags = useId != BaseReg::kIdBad ? uint32_t(opInfo[i].flags()) : uint32_t(RATiedReg::kUse | RATiedReg::kRead);
              ASMJIT_PROPAGATE(ib.add(workReg, flags, allocable, useId, useRewriteMask, outId, outRewriteMask));
            }
          }

          if (mem.hasIndexReg()) {
            uint32_t vIndex = Operand::unpackId(mem.indexId());
            if (vIndex < Operand::kPackedIdCount) {
              RAWorkReg* workReg;
              ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

              uint32_t group = workReg->group();
              uint32_t allocable = _pass->_availableRegs[group];
              uint32_t rewriteMask = Support::mask(inst->getRewriteIndex(&mem._mem.index));

              ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocable, BaseReg::kIdBad, rewriteMask, BaseReg::kIdBad, 0));
            }
          }
        }
      }
    }

    // Handle extra operand (either REP {cx|ecx|rcx} or AVX-512 {k} selector).
    if (inst->hasExtraReg()) {
      uint32_t vIndex = Operand::unpackId(inst->extraReg().id());
      if (vIndex < Operand::kPackedIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->group();
        uint32_t rewriteMask = Support::mask(inst->getRewriteIndex(&inst->extraReg()._id));

        if (group == Gp::kGroupKReg) {
          // AVX-512 mask selector {k} register - read-only, allocable to any register except {k0}.
          uint32_t allocableRegs= _pass->_availableRegs[group] & ~Support::mask(0);
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocableRegs, BaseReg::kIdBad, rewriteMask, BaseReg::kIdBad, 0));
          singleRegOps = 0;
        }
        else {
          // REP {cx|ecx|rcx} register - read & write, allocable to {cx|ecx|rcx} only.
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRW, 0, Gp::kIdCx, rewriteMask, Gp::kIdBad, 0));
        }
      }
      else {
        uint32_t group = inst->extraReg().group();
        if (group == Gp::kGroupKReg && inst->extraReg().id() != 0)
          singleRegOps = 0;
      }
    }

    // Handle X86 constraints.
    if (hasGpbHiConstraint) {
      for (uint32_t i = 0; i < ib.tiedRegCount(); i++) {
        RATiedReg* tiedReg = ib[i];
        tiedReg->_allocableRegs &= tiedReg->hasFlag(RATiedReg::kX86Gpb) ? 0x0Fu : 0xFFu;
      }
    }

    if (ib.tiedRegCount() == 1) {
      // Handle special cases of some instructions where all operands share the same
      // register. In such case the single operand becomes read-only or write-only.
      uint32_t singleRegCase = InstDB::kSingleRegNone;
      if (singleRegOps == opCount) {
        singleRegCase = instInfo.singleRegCase();
      }
      else if (opCount == 2 && inst->opType(1).isImm()) {
        // Handle some tricks used by X86 asm.
        const BaseReg& reg = inst->opType(0).as<BaseReg>();
        const Imm& imm = inst->opType(1).as<Imm>();

        const RAWorkReg* workReg = _pass->workRegById(ib[0]->workId());
        uint32_t workRegSize = workReg->info().size();

        switch (inst->id()) {
          case Inst::kIdOr: {
            // Sets the value of the destination register to -1, previous content unused.
            if (reg.size() >= 4 || reg.size() >= workRegSize) {
              if (imm.i64() == -1 || imm.u64() == immMaskFromSize(reg.size()))
                singleRegCase = InstDB::kSingleRegWO;
            }
            ASMJIT_FALLTHROUGH;
          }

          case Inst::kIdAdd:
          case Inst::kIdAnd:
          case Inst::kIdRol:
          case Inst::kIdRor:
          case Inst::kIdSar:
          case Inst::kIdShl:
          case Inst::kIdShr:
          case Inst::kIdSub:
          case Inst::kIdXor: {
            // Updates [E|R]FLAGS without changing the content.
            if (reg.size() != 4 || reg.size() >= workRegSize) {
              if (imm.u64() == 0)
                singleRegCase = InstDB::kSingleRegRO;
            }
            break;
          }
        }
      }

      switch (singleRegCase) {
        case InstDB::kSingleRegNone:
          break;
        case InstDB::kSingleRegRO:
          ib[0]->makeReadOnly();
          opInfo = X86OpInfo::op_r;
          break;
        case InstDB::kSingleRegWO:
          ib[0]->makeWriteOnly();
          opInfo = X86OpInfo::op_w_all;
          break;
      }
    }

    controlType = instInfo.controlType();
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - OnCall]
// ============================================================================

Error X86RACFGBuilder::onCall(FuncCallNode* call, RAInstBuilder& ib) noexcept {
  uint32_t argCount = call->argCount();
  uint32_t retCount = call->retCount();
  const FuncDetail& fd = call->detail();

  for (uint32_t argIndex = 0; argIndex < argCount; argIndex++) {
    for (uint32_t argHi = 0; argHi <= kFuncArgHi; argHi += kFuncArgHi) {
      if (!fd.hasArg(argIndex + argHi))
        continue;

      const FuncValue& arg = fd.arg(argIndex + argHi);
      const Operand& op = call->arg(argIndex + argHi);

      if (op.isNone())
        continue;

      if (op.isReg()) {
        const Reg& reg = op.as<Reg>();
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(reg.id()), &workReg));

        if (arg.isReg()) {
          uint32_t regGroup = workReg->group();
          uint32_t argGroup = Reg::groupOf(arg.regType());

          if (regGroup == argGroup) {
            ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, 0, arg.regId(), 0, Gp::kIdBad, 0));
          }
          else {
            // TODO:
            ASMJIT_ASSERT(!"IMPLEMENTED");
          }
        }
        else {
          ASMJIT_PROPAGATE(moveRegToStackArg(call, arg, op.as<BaseReg>()));
        }
      }
      else if (op.isImm()) {
        if (arg.isReg()) {
          BaseReg reg;
          ASMJIT_PROPAGATE(moveImmToRegArg(call, arg, op.as<Imm>(), &reg));

          RAWorkReg* workReg;
          ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(reg.id()), &workReg));
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, 0, arg.regId(), 0, Gp::kIdBad, 0));
        }
        else {
          ASMJIT_PROPAGATE(moveImmToStackArg(call, arg, op.as<Imm>()));
        }
      }
      else {
        return DebugUtils::errored(kErrorInvalidState);
      }
    }
  }

  for (uint32_t retIndex = 0; retIndex < retCount; retIndex++) {
    const FuncValue& ret = fd.ret(retIndex);
    const Operand& op = call->ret(retIndex);

    if (op.isReg()) {
      const Reg& reg = op.as<Reg>();
      RAWorkReg* workReg;
      ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(Operand::unpackId(reg.id()), &workReg));

      if (ret.isReg()) {
        uint32_t regGroup = workReg->group();
        uint32_t retGroup = Reg::groupOf(ret.regType());

        if (regGroup == retGroup) {
          ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kOut | RATiedReg::kWrite, 0, Gp::kIdBad, 0, ret.regId(), 0));
        }
        else {
          // TODO:
          ASMJIT_ASSERT(!"IMPLEMENTED");
        }
      }
      else {
        return DebugUtils::errored(kErrorInvalidState);
      }
    }
  }

  // Setup clobbered registers.
  ib._clobbered[0] = Support::lsbMask<uint32_t>(_pass->_physRegCount[0]) & ~fd.preservedRegs(0);
  ib._clobbered[1] = Support::lsbMask<uint32_t>(_pass->_physRegCount[1]) & ~fd.preservedRegs(1);
  ib._clobbered[2] = Support::lsbMask<uint32_t>(_pass->_physRegCount[2]) & ~fd.preservedRegs(2);
  ib._clobbered[3] = Support::lsbMask<uint32_t>(_pass->_physRegCount[3]) & ~fd.preservedRegs(3);

  // This block has function call(s).
  _pass->func()->frame().updateCallStackSize(fd.argStackSize());
  _curBlock->addFlags(RABlock::kFlagHasFuncCalls);

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - MoveImmToRegArg]
// ============================================================================

Error X86RACFGBuilder::moveImmToRegArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_, BaseReg* out) noexcept {
  ASMJIT_ASSERT(arg.isReg());

  Imm imm(imm_);
  switch (arg.typeId()) {
    case Type::kIdI8: imm.signExtend8Bits(); goto MovU32;
    case Type::kIdU8: imm.zeroExtend8Bits(); goto MovU32;
    case Type::kIdI16: imm.signExtend16Bits(); goto MovU32;
    case Type::kIdU16: imm.zeroExtend16Bits(); goto MovU32;

    case Type::kIdI32:
    case Type::kIdU32:
MovU32:
      imm.zeroExtend32Bits();
      ASMJIT_PROPAGATE(cc()->_newReg(*out, Type::kIdU32, nullptr));

MovAny:
      {
        InstNode* inst;
        uint32_t physIds[] = { arg.regId(), BaseReg::kIdBad };
        ASMJIT_PROPAGATE(newInst(&inst, Inst::kIdMov, X86OpInfo::op_w, physIds, *out, imm));

        cc()->addBefore(inst, call);
        return kErrorOk;
      }

    case Type::kIdI64:
    case Type::kIdU64:
      // Prefer smaller code, moving to GPD automatically zero extends in 64-bit mode.
      if (imm.isU32())
        goto MovU32;

      ASMJIT_PROPAGATE(cc()->_newReg(*out, Type::kIdU64, nullptr));
      goto MovAny;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - MoveImmToStackArg]
// ============================================================================

Error X86RACFGBuilder::moveImmToStackArg(FuncCallNode* call, const FuncValue& arg, const Imm& imm_) noexcept {
  ASMJIT_ASSERT(arg.isStack());

  Mem mem = ptr(_pass->_sp.as<Gp>(), arg.stackOffset());
  Imm imm[2];

  mem.setSize(4);
  imm[0] = imm_;

  uint32_t nMovs = 0;
  static const uint32_t noPhysIds[] = { BaseReg::kIdBad, BaseReg::kIdBad };

  // One stack entry has the same size as the native register size. That means
  // that if we want to move a 32-bit integer on the stack in 64-bit mode, we
  // need to extend it to a 64-bit integer first. In 32-bit mode, pushing a
  // 64-bit on stack is done in two steps by pushing low and high parts
  // separately.
  switch (arg.typeId()) {
    case Type::kIdI8: imm[0].signExtend8Bits(); goto MovU32;
    case Type::kIdU8: imm[0].zeroExtend8Bits(); goto MovU32;
    case Type::kIdI16: imm[0].signExtend16Bits(); goto MovU32;
    case Type::kIdU16: imm[0].zeroExtend16Bits(); goto MovU32;

    case Type::kIdI32:
    case Type::kIdU32:
    case Type::kIdF32:
MovU32:
      imm[0].zeroExtend32Bits();
      nMovs = 1;
      break;

    case Type::kIdI64:
    case Type::kIdU64:
    case Type::kIdF64:
    case Type::kIdMmx32:
    case Type::kIdMmx64:
      if (_is64Bit && imm[0].isI32()) {
        mem.setSize(8);
        nMovs = 1;
        break;
      }

      imm[1].setU32(imm[0].u32Hi());
      imm[0].zeroExtend32Bits();
      nMovs = 2;
      break;

    default:
      return DebugUtils::errored(kErrorInvalidState);
  }

  for (uint32_t i = 0; i < nMovs; i++) {
    InstNode* inst;
    ASMJIT_PROPAGATE(newInst(&inst, Inst::kIdMov, X86OpInfo::op_w, noPhysIds, mem, imm[i]));
    cc()->addBefore(inst, call);
    mem.addOffsetLo32(int32_t(mem.size()));
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - MoveRegToStackArg]
// ============================================================================

Error X86RACFGBuilder::moveRegToStackArg(FuncCallNode* call, const FuncValue& arg, const BaseReg& reg) noexcept {
  ASMJIT_ASSERT(arg.isStack());

  return kErrorOk;
  /*
  Mem mem = ptr(_pass->_sp.as<Gp>(), arg.stackOffset());
  Reg r0, r1;

  uint32_t gpSize = cc()->gpSize();
  uint32_t instId = 0;

  uint32_t dstTypeId = arg.typeId();
  // uint32_t srcTypeId = reg.;


  switch (dstTypeId) {
    case Type::kIdI64:
    case Type::kIdU64:
      // Extend BYTE->QWORD (GP).
      if (Type::isGp8(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpbLo>(srcPhysId);

        instId = (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI8) ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (Type::isGpw(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpw>(srcPhysId);

        instId = (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI16) ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend DWORD->QWORD (GP).
      if (Type::isGpd(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpd>(srcPhysId);

        instId = Inst::kIdMovsxd;
        if (dstTypeId == Type::kIdI64 && srcTypeId == Type::kIdI32)
          goto _ExtendMovGpXQ;
        else
          goto _ZeroExtendGpDQ;
      }

      // Move QWORD (GP).
      if (Type::isGpq(srcTypeId)) goto MovGpQ;
      if (Type::isMmx(srcTypeId)) goto MovMmQ;
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    case Type::kIdI32:
    case Type::kIdU32:
    case Type::kIdI16:
    case Type::kIdU16:
      // DWORD <- WORD (Zero|Sign Extend).
      if (Type::isGpw(srcTypeId)) {
        bool isDstSigned = dstTypeId == Type::kIdI16 || dstTypeId == Type::kIdI32;
        bool isSrcSigned = srcTypeId == Type::kIdI8  || srcTypeId == Type::kIdI16;

        r1.setRegT<Reg::kTypeGpw>(srcPhysId);
        instId = isDstSigned && isSrcSigned ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto _ExtendMovGpD;
      }

      // DWORD <- BYTE (Zero|Sign Extend).
      if (Type::isGpb(srcTypeId)) {
        bool isDstSigned = dstTypeId == Type::kIdI16 || dstTypeId == Type::kIdI32;
        bool isSrcSigned = srcTypeId == Type::kIdI8  || srcTypeId == Type::kIdI16;

        r1.setRegT<Reg::kTypeGpbLo>(srcPhysId);
        instId = isDstSigned && isSrcSigned ? Inst::kIdMovsx : Inst::kIdMovzx;
        goto _ExtendMovGpD;
      }
      ASMJIT_FALLTHROUGH;

    case Type::kIdI8:
    case Type::kIdU8:
      if (Type::isInt(srcTypeId)) goto MovGpD;
      if (Type::isMmx(srcTypeId)) goto MovMmD;
      if (Type::isVec(srcTypeId)) goto MovXmmD;
      break;

    case Type::kIdMmx32:
    case Type::kIdMmx64:
      // Extend BYTE->QWORD (GP).
      if (Type::isGpb(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpbLo>(srcPhysId);

        instId = Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      // Extend WORD->QWORD (GP).
      if (Type::isGpw(srcTypeId)) {
        r1.setRegT<Reg::kTypeGpw>(srcPhysId);

        instId = Inst::kIdMovzx;
        goto _ExtendMovGpXQ;
      }

      if (Type::isGpd(srcTypeId)) goto _ExtendMovGpDQ;
      if (Type::isGpq(srcTypeId)) goto MovGpQ;
      if (Type::isMmx(srcTypeId)) goto MovMmQ;
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    case Type::kIdF32:
    case Type::kIdF32x1:
      if (Type::isVec(srcTypeId)) goto MovXmmD;
      break;

    case Type::kIdF64:
    case Type::kIdF64x1:
      if (Type::isVec(srcTypeId)) goto MovXmmQ;
      break;

    default:
      // TODO: Vector types by stack.
      break;
  }
  return DebugUtils::errored(kErrorInvalidState);

  // Extend+Move Gp.
_ExtendMovGpD:
  m0.setSize(4);
  r0.setRegT<Reg::kTypeGpd>(srcPhysId);

  cc()->emit(instId, r0, r1);
  cc()->emit(Inst::kIdMov, m0, r0);
  return kErrorOk;

_ExtendMovGpXQ:
  if (gpSize == 8) {
    m0.setSize(8);
    r0.setRegT<Reg::kTypeGpq>(srcPhysId);

    cc()->emit(instId, r0, r1);
    cc()->emit(Inst::kIdMov, m0, r0);
  }
  else {
    m0.setSize(4);
    r0.setRegT<Reg::kTypeGpd>(srcPhysId);

    cc()->emit(instId, r0, r1);

_ExtendMovGpDQ:
    cc()->emit(Inst::kIdMov, m0, r0);
    m0.addOffsetLo32(4);
    cc()->emit(Inst::kIdAnd, m0, 0);
  }
  return kErrorOk;

_ZeroExtendGpDQ:
  m0.setSize(4);
  r0.setRegT<Reg::kTypeGpd>(srcPhysId);
  goto _ExtendMovGpDQ;

MovGpD:
  m0.setSize(4);
  r0.setRegT<Reg::kTypeGpd>(srcPhysId);
  return cc()->emit(Inst::kIdMov, m0, r0);

MovGpQ:
  m0.setSize(8);
  r0.setRegT<Reg::kTypeGpq>(srcPhysId);
  return cc()->emit(Inst::kIdMov, m0, r0);

MovMmD:
  m0.setSize(4);
  r0.setRegT<Reg::kTypeMm>(srcPhysId);
  return cc()->emit(Inst::kIdMovd, m0, r0);

MovMmQ:
  m0.setSize(8);
  r0.setRegT<Reg::kTypeMm>(srcPhysId);
  return cc()->emit(Inst::kIdMovq, m0, r0);

MovXmmD:
  m0.setSize(4);
  r0.setRegT<Reg::kTypeXmm>(srcPhysId);
  return cc()->emit(Inst::kIdMovss, m0, r0);

MovXmmQ:
  m0.setSize(8);
  r0.setRegT<Reg::kTypeXmm>(srcPhysId);
  return cc()->emit(Inst::kIdMovlps, m0, r0);
  */
}

// ============================================================================
// [asmjit::x86::X86RACFGBuilder - OnReg]
// ============================================================================

Error X86RACFGBuilder::onRet(FuncRetNode* funcRet, RAInstBuilder& ib) noexcept {
  const FuncDetail& funcDetail = _pass->func()->detail();
  const Operand* opArray = funcRet->operands();
  uint32_t opCount = funcRet->opCount();

  for (uint32_t i = 0; i < opCount; i++) {
    const Operand& op = opArray[i];
    if (op.isNone()) continue;

    const FuncValue& ret = funcDetail.ret(i);
    if (ASMJIT_UNLIKELY(!ret.isReg()))
      return DebugUtils::errored(kErrorInvalidState);

    if (op.isReg()) {
      // Register return value.
      const Reg& reg = op.as<Reg>();
      uint32_t vIndex = Operand::unpackId(reg.id());

      if (vIndex < Operand::kPackedIdCount) {
        RAWorkReg* workReg;
        ASMJIT_PROPAGATE(_pass->virtIndexAsWorkReg(vIndex, &workReg));

        uint32_t group = workReg->group();
        uint32_t allocable = _pass->_availableRegs[group];
        ASMJIT_PROPAGATE(ib.add(workReg, RATiedReg::kUse | RATiedReg::kRead, allocable, ret.regId(), 0, BaseReg::kIdBad, 0));
      }
    }
    else {
      return DebugUtils::errored(kErrorInvalidState);
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::x86::X86RAPass - Construction / Destruction]
// ============================================================================

X86RAPass::X86RAPass() noexcept
  : RAPass(),
    _avxEnabled(false) {}
X86RAPass::~X86RAPass() noexcept {}

// ============================================================================
// [asmjit::x86::X86RAPass - OnInit / OnDone]
// ============================================================================

void X86RAPass::onInit() noexcept {
  uint32_t archId = cc()->archId();
  uint32_t baseRegCount = archId == ArchInfo::kIdX86 ? 8u : 16u;

  _archTraits[Reg::kGroupGp] |= RAArchTraits::kHasSwap;

  _physRegCount.set(Reg::kGroupGp  , baseRegCount);
  _physRegCount.set(Reg::kGroupVec , baseRegCount);
  _physRegCount.set(Reg::kGroupMm  , 8);
  _physRegCount.set(Reg::kGroupKReg, 8);
  _buildPhysIndex();

  _availableRegCount = _physRegCount;
  _availableRegs[Reg::kGroupGp  ] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupGp  ));
  _availableRegs[Reg::kGroupVec ] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupVec ));
  _availableRegs[Reg::kGroupMm  ] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupMm  ));
  _availableRegs[Reg::kGroupKReg] = Support::lsbMask<uint32_t>(_physRegCount.get(Reg::kGroupKReg));

  // The architecture specific setup makes implicitly all registers available. So
  // make unavailable all registers that are special and cannot be used in general.
  bool hasFP = _func->frame().hasPreservedFP();

  makeUnavailable(Reg::kGroupGp, Gp::kIdSp);            // ESP|RSP used as a stack-pointer (SP).
  if (hasFP) makeUnavailable(Reg::kGroupGp, Gp::kIdBp); // EBP|RBP used as a frame-pointer (FP).

  _sp = cc()->zsp();
  _fp = cc()->zbp();
  _avxEnabled = _func->frame().isAvxEnabled();
}

void X86RAPass::onDone() noexcept {}

// ============================================================================
// [asmjit::x86::X86RAPass - BuildCFG]
// ============================================================================

Error X86RAPass::buildCFG() noexcept {
  return X86RACFGBuilder(this).run();
}

// ============================================================================
// [asmjit::x86::X86RAPass - OnEmit]
// ============================================================================

Error X86RAPass::onEmitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseReg dst(wReg->info().signature(), dstPhysId);
  BaseReg src(wReg->info().signature(), srcPhysId);

  const char* comment = nullptr;

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.setFormat("<MOVE> %s", workRegById(workId)->name());
    comment = _tmpString.data();
  }
  #endif

  return X86Internal::emitRegMove(cc()->as<Emitter>(), dst, src, wReg->typeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept {
  RAWorkReg* waReg = workRegById(aWorkId);
  RAWorkReg* wbReg = workRegById(bWorkId);

  bool is64Bit = std::max(waReg->typeId(), wbReg->typeId()) >= Type::kIdI64;
  uint32_t sign = is64Bit ? uint32_t(RegTraits<Reg::kTypeGpq>::kSignature)
                          : uint32_t(RegTraits<Reg::kTypeGpd>::kSignature);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.setFormat("<SWAP> %s, %s", waReg->name(), wbReg->name());
    cc()->setInlineComment(_tmpString.data());
  }
  #endif

  return cc()->emit(Inst::kIdXchg, Reg(sign, aPhysId), Reg(sign, bPhysId));
}

Error X86RAPass::onEmitLoad(uint32_t workId, uint32_t dstPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseReg dstReg(wReg->info().signature(), dstPhysId);
  BaseMem srcMem(workRegAsMem(wReg));

  const char* comment = nullptr;

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.setFormat("<LOAD> %s", workRegById(workId)->name());
    comment = _tmpString.data();
  }
  #endif

  return X86Internal::emitRegMove(cc()->as<Emitter>(), dstReg, srcMem, wReg->typeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitSave(uint32_t workId, uint32_t srcPhysId) noexcept {
  RAWorkReg* wReg = workRegById(workId);
  BaseMem dstMem(workRegAsMem(wReg));
  BaseReg srcReg(wReg->info().signature(), srcPhysId);

  const char* comment = nullptr;

  #ifndef ASMJIT_DISABLE_LOGGING
  if (_loggerFlags & FormatOptions::kFlagAnnotations) {
    _tmpString.setFormat("<SAVE> %s", workRegById(workId)->name());
    comment = _tmpString.data();
  }
  #endif

  return X86Internal::emitRegMove(cc()->as<Emitter>(), dstMem, srcReg, wReg->typeId(), _avxEnabled, comment);
}

Error X86RAPass::onEmitJump(const Label& label) noexcept {
  return cc()->jmp(label);
}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_COMPILER

























































































#if 0
ASMJIT_INLINE void X86CallAlloc::allocImmsOnStack() {
  FuncCallNode* node = getNode();
  FuncDetail& fd = node->getDetail();

  uint32_t argCount = fd.argCount();
  Operand_* args = node->_args;

  for (uint32_t i = 0; i < argCount; i++) {
    Operand_& op = args[i];
    if (!op.isImm()) continue;

    const Imm& imm = static_cast<const Imm&>(op);
    const FuncDetail::Value& arg = fd.arg(i);
    uint32_t varType = arg.typeId();

    if (arg.isReg()) {
      _context->emitImmToReg(varType, arg.regId(), &imm);
    }
    else {
      Mem dst = ptr(_context->_zsp, -int(_context->gpSize()) + arg.stackOffset());
      _context->emitImmToStack(varType, &dst, &imm);
    }
  }
}

template<int C>
ASMJIT_INLINE void X86CallAlloc::duplicate() {
  TiedReg* tiedRegs = getTiedRegsByGroup(C);
  uint32_t tiedCount = getTiedCountByGroup(C);

  for (uint32_t i = 0; i < tiedCount; i++) {
    TiedReg* tied = &tiedRegs[i];
    if ((tied->flags & TiedReg::kRReg) == 0) continue;

    uint32_t inRegs = tied->inRegs;
    if (!inRegs) continue;

    VirtReg* vreg = tied->vreg;
    uint32_t physId = vreg->physId();

    ASMJIT_ASSERT(physId != BaseReg::kIdBad);

    inRegs &= ~Support::mask(physId);
    if (!inRegs) continue;

    for (uint32_t dupIndex = 0; inRegs != 0; dupIndex++, inRegs >>= 1) {
      if (inRegs & 0x1) {
        _context->emitMove(vreg, dupIndex, physId, "Duplicate");
        _context->_clobberedRegs.or_(C, Support::mask(dupIndex));
      }
    }
  }
}

// ============================================================================
// [asmjit::x86::X86CallAlloc - Ret]
// ============================================================================

ASMJIT_INLINE void X86CallAlloc::ret() {
  FuncCallNode* node = getNode();
  FuncDetail& fd = node->detail();
  Operand_* rets = node->_ret;

  for (uint32_t i = 0; i < 2; i++) {
    const FuncDetail::Value& ret = fd.ret(i);
    Operand_* op = &rets[i];

    if (!ret.isReg() || !op->isVirtReg())
      continue;

    VirtReg* vreg = _cc->virtRegById(op->id());
    uint32_t regId = ret.regId();

    switch (vreg->group()) {
      case Reg::kGroupGp:
        _context->unuse<Reg::kGroupGp>(vreg);
        _context->attach<Reg::kGroupGp>(vreg, regId, true);
        break;

      case Reg::kGroupMm:
        _context->unuse<Reg::kGroupMm>(vreg);
        _context->attach<Reg::kGroupMm>(vreg, regId, true);
        break;

      case Reg::kGroupVec:
        if (Reg::groupOf(ret.regType()) == Reg::kGroupVec) {
          _context->unuse<Reg::kGroupVec>(vreg);
          _context->attach<Reg::kGroupVec>(vreg, regId, true);
        }
        else {
          uint32_t elementId = Type::baseOf(vreg->typeId());
          uint32_t size = (elementId == Type::kIdF32) ? 4 : 8;

          Mem m = _context->varMem(vreg);
          m.setSize(size);

          _context->unuse<Reg::kGroupVec>(vreg, VirtReg::kStateMem);
          _cc->fstp(m);
        }
        break;
    }
  }
}

// ============================================================================
// [asmjit::x86::X86RAPass - Translate - Ret]
// ============================================================================

static Error X86RAPass_translateRet(X86RAPass* self, FuncRetNode* rNode, LabelNode* exitTarget) {
  Compiler* cc = self->cc();
  BaseNode* node = rNode->next();

  // 32-bit mode requires to push floating point return value(s), handle it
  // here as it's a special case.
  X86RAData* raData = rNode->passData<X86RAData>();
  if (raData) {
    TiedReg* tiedRegs = raData->tiedRegs;
    uint32_t tiedTotal = raData->tiedTotal;

    for (uint32_t i = 0; i < tiedTotal; i++) {
      TiedReg* tied = &tiedRegs[i];
      if (tied->flags & (TiedReg::kX86Fld4 | TiedReg::kX86Fld8)) {
        VirtReg* vreg = tied->vreg;
        Mem m(self->varMem(vreg));

        uint32_t elementId = Type::baseOf(vreg->typeId());
        m.setSize(elementId == Type::kIdF32 ? 4 :
                  elementId == Type::kIdF64 ? 8 :
                  (tied->flags & TiedReg::kX86Fld4) ? 4 : 8);

        cc->fld(m);
      }
    }
  }

  // Decide whether to `jmp` or not in case we are next to the return label.
  while (node) {
    switch (node->type()) {
      // If we have found an exit label we just return, there is no need to
      // emit jump to that.
      case BaseNode::kNodeLabel:
        if (static_cast<LabelNode*>(node) == exitTarget)
          return kErrorOk;
        goto _EmitRet;

      case BaseNode::kNodeData:
      case BaseNode::kNodeInst:
      case BaseNode::kNodeFuncCall:
      case BaseNode::kNodeFuncExit:
        goto _EmitRet;

      // Continue iterating.
      case BaseNode::kNodeComment:
      case BaseNode::kNodeAlign:
      case BaseNode::kNodeHint:
        break;

      // Invalid node to be here.
      case BaseNode::kNodeFunc:
        return DebugUtils::errored(kErrorInvalidState);

      // We can't go forward from here.
      case BaseNode::kNodeSentinel:
        return kErrorOk;
    }

    node = node->next();
  }

_EmitRet:
  {
    cc->_setCursor(rNode);
    cc->jmp(exitTarget->label());
  }
  return kErrorOk;
}
#endif
