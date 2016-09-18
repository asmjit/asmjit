// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_DISABLE_BUILDER)

// [Dependencies]
#include "../x86/x86inst.h"
#include "../x86/x86operand.h"
#include "../x86/x86ssetoavxpass_p.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::X86SseToAvxPass]
// ============================================================================

class X86SseToAvxPass : public CBPass {
  ASMJIT_NONCOPYABLE(X86SseToAvxPass)
public:
  X86SseToAvxPass() noexcept : CBPass("SseToAvx") {}
  virtual Error process(Zone* zone) noexcept override;

  enum ProbeMask {
    kProbeMmx  = 1U << X86Reg::kRegMm,    //!< Instruction uses MMX registers.
    kProbeXmm  = 1U << X86Reg::kRegXmm    //!< Instruction uses XMM registers.
  };

  static ASMJIT_INLINE uint32_t probeRegs(const Operand* opArray, uint32_t opCount) noexcept {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < opCount; i++) {
      const Operand& op = opArray[i];
      if (!op.isReg()) continue;
      mask |= Utils::mask(static_cast<const Reg&>(op).getRegType());
    }
    return mask;
  }
};

Error X86SseToAvxPass::process(Zone* zone) noexcept {
  ASMJIT_UNUSED(zone);

  CBNode* node_ = cb()->getFirstNode();
  while (node_) {
    if (node_->getType() == CBNode::kNodeInst) {
      CBInst* node = static_cast<CBInst*>(node_);
      uint32_t instId = node->getInstId();

      // Skip invalid and high-level instructions; we don't care here.
      if (!X86Inst::isDefinedId(instId)) continue;

      // Skip non-SSE instructions.
      const X86Inst& instData = X86Inst::getInst(instId);
      if (!instData.isSseFamily()) continue;

      // Skip instructions that don't use XMM registers.
      uint32_t regs = probeRegs(node->getOpArray(), node->getOpCount());
      if (!(regs & kProbeXmm)) continue;

      if (!(regs & kProbeMmx)) {
        // This is the common case.
        const X86Inst::SseData& sseData = instData.getSseData();

        // TODO: Wait for some fixes in CBInst first.
      }
      else {
        // If this instruction uses MMX register it means that it's a conversion
        // between MMX and XMM (and vice versa), this cannot be directly translated
        // to AVX as there is no such AVX instruction that works with MMX registers.

        // TODO: Needs a mem-slot to be able to do this.
      }

    }

    node_ = node_->getNext();
  }

  return kErrorOk;
}

Error X86SseToAvxPassInit::add(CodeBuilder* cb) noexcept { return cb->addPassT<X86SseToAvxPass>(); }

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 && !ASMJIT_DISABLE_BUILDER
