// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_LA_A64RAPASS_P_H_INCLUDED
#define ASMJIT_LA_A64RAPASS_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/rabuilders_p.h"
#include "../core/rapass_p.h"
#include "../loongarch/la64assembler.h"
#include "../loongarch/la64compiler.h"
#include "../loongarch/la64emithelper_p.h"

ASMJIT_BEGIN_SUB_NAMESPACE(la64)

//! \cond INTERNAL
//! \addtogroup asmjit_a64
//! \{

//! LA register allocation pass.
//!
//! Takes care of generating function prologs and epilogs, and also performs
//! register allocation.
class LARAPass : public BaseRAPass {
public:
  ASMJIT_NONCOPYABLE(LARAPass)
  typedef BaseRAPass Base;

  EmitHelper _emitHelper;

  //! \name Construction & Destruction
  //! \{

  LARAPass() noexcept;
  ~LARAPass() noexcept override;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the compiler casted to `la::Compiler`.
  ASMJIT_INLINE_NODEBUG Compiler* cc() const noexcept { return static_cast<Compiler*>(_cb); }

  //! Returns emit helper.
  ASMJIT_INLINE_NODEBUG EmitHelper* emitHelper() noexcept { return &_emitHelper; }

  //! \}

  //! \name Events
  //! \{

  void onInit() noexcept override;
  void onDone() noexcept override;

  //! \}

  //! \name CFG
  //! \{

  Error buildCFG() noexcept override;

  //! \}

  //! \name Rewrite
  //! \{

  Error _rewrite(BaseNode* first, BaseNode* stop) noexcept override;

  //! \}

  //! \name Prolog & Epilog
  //! \{

  Error updateStackFrame() noexcept override;

  //! \}

  //! \name Emit Helpers
  //! \{

  Error emitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept override;
  Error emitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept override;

  Error emitLoad(uint32_t workId, uint32_t dstPhysId) noexcept override;
  Error emitSave(uint32_t workId, uint32_t srcPhysId) noexcept override;

  Error emitJump(const Label& label) noexcept override;
  Error emitPreCall(InvokeNode* invokeNode) noexcept override;

  //! \}
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_LA_A64RAPASS_P_H_INCLUDED
