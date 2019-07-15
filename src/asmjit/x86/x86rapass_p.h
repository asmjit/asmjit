// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86RAPASS_P_H
#define _ASMJIT_X86_X86RAPASS_P_H

#include "../core/build.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/rabuilders_p.h"
#include "../core/rapass_p.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL

//! \defgroup asmjit_x86_ra X86 RA
//! \ingroup asmjit_x86
//!
//! \brief X86/X64 register allocation.

//! \addtogroup asmjit_x86_ra
//! \{

// ============================================================================
// [asmjit::X86RAPass]
// ============================================================================

//! X86 register allocation pass.
//!
//! Takes care of generating function prologs and epilogs, and also performs
//! register allocation.
class X86RAPass : public RAPass {
public:
  ASMJIT_NONCOPYABLE(X86RAPass)
  typedef RAPass Base;

  bool _avxEnabled;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  X86RAPass() noexcept;
  virtual ~X86RAPass() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Returns the compiler casted to `x86::Compiler`.
  inline Compiler* cc() const noexcept { return static_cast<Compiler*>(_cb); }

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  inline uint32_t choose(uint32_t sseInstId, uint32_t avxInstId) noexcept {
    return _avxEnabled ? avxInstId : sseInstId;
  }

  // --------------------------------------------------------------------------
  // [OnInit / OnDone]
  // --------------------------------------------------------------------------

  void onInit() noexcept override;
  void onDone() noexcept override;

  // --------------------------------------------------------------------------
  // [CFG]
  // --------------------------------------------------------------------------

  Error buildCFG() noexcept override;

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  Error onEmitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept override;
  Error onEmitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept override;

  Error onEmitLoad(uint32_t workId, uint32_t dstPhysId) noexcept override;
  Error onEmitSave(uint32_t workId, uint32_t srcPhysId) noexcept override;

  Error onEmitJump(const Label& label) noexcept override;
  Error onEmitPreCall(FuncCallNode* node) noexcept override;
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // _ASMJIT_X86_X86RAPASS_P_H
