// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86CONTEXT_P_H
#define _ASMJIT_X86_X86CONTEXT_P_H

// [Dependencies - AsmJit]
#include "../base/compiler.h"
#include "../base/context_p.h"
#include "../base/intutil.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86defs.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [asmjit::Context]
// ============================================================================

//! @internal
//!
//! @brief Compiler context is used by @ref X86X64Compiler.
//!
//! Compiler context is used during compilation and normally developer doesn't
//! need access to it. The context is user per function (it's reset after each
//! function is generated).
struct X86X64Context : public BaseContext {
  ASMJIT_NO_COPY(X86X64Context)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref Context instance.
  X86X64Context(X86X64Compiler* compiler);
  //! @brief Destroy the @ref Context instance.
  virtual ~X86X64Context();

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  virtual void reset();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get compiler as @ref X86X64Compiler.
  ASMJIT_INLINE X86X64Compiler* getCompiler() const { return static_cast<X86X64Compiler*>(_compiler); }
  //! @brief Get function as @ref X86X64FuncNode.
  ASMJIT_INLINE X86X64FuncNode* getFunc() const { return reinterpret_cast<X86X64FuncNode*>(_func); }

  ASMJIT_INLINE bool isX64() const { return _baseRegsCount == 16; }

  //! @brief Get clobbered registers (global).
  ASMJIT_INLINE uint32_t getClobberedRegs(uint32_t c) { return _clobberedRegs.get(c); }

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE VarInst* newVarInst(uint32_t vaCount) {
    return static_cast<VarInst*>(
      _zoneAllocator.alloc(sizeof(VarInst) + vaCount * sizeof(VarAttr)));
  }

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  void emitLoad(VarData* vd, uint32_t regIndex, const char* reason);
  void emitSave(VarData* vd, uint32_t regIndex, const char* reason);
  void emitMove(VarData* vd, uint32_t toRegIndex, uint32_t fromRegIndex, const char* reason);
  void emitSwapGp(VarData* aVd, VarData* bVd, uint32_t aIndex, uint32_t bIndex, const char* reason);

  void emitPushSequence(uint32_t regs);
  void emitPopSequence(uint32_t regs);

  void emitMoveVarOnStack(uint32_t dstType, const Mem* dst, uint32_t srcType, uint32_t srcIndex);
  void emitMoveImmOnStack(uint32_t dstType, const Mem* dst, const Imm* src);

  void emitMoveImmToReg(uint32_t dstType, uint32_t dstIndex, const Imm* src);

  // --------------------------------------------------------------------------
  // [Register Management]
  // --------------------------------------------------------------------------

  void _checkState();

#if defined(ASMJIT_DEBUG)
#define ASMJIT_CONTEXT_CHECK_STATE _checkState();
#else
#define ASMJIT_CONTEXT_CHECK_STATE
#endif // ASMJIT_DEBUG

  ASMJIT_INLINE uint32_t getRegsCount(uint32_t c) const {
    if (c == kRegClassGp || c == kRegClassXy)
      return _baseRegsCount;
    else
      return 8;
  }

  ASMJIT_INLINE uint32_t getRegSize() const {
    return _zsp.getSize();
  }

  // --------------------------------------------------------------------------
  // [Attach / Detach]
  // --------------------------------------------------------------------------

  //! @brief Attach.
  //!
  //! Attach a register to the 'VarData', changing 'VarData' members to show
  //! that the variable is currently alive and linking variable with the
  //! current 'VarState'.
  template<int C>
  ASMJIT_INLINE void attach(VarData* vd, uint32_t regIndex, bool modified) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(regIndex != kInvalidReg);

    // Prevent Esp allocation if C==Gp.
    ASMJIT_ASSERT(C != kRegClassGp || regIndex != kRegIndexSp);

    uint32_t regMask = IntUtil::mask(regIndex);

    vd->setState(kVarStateReg);
    vd->setRegIndex(regIndex);
    vd->setModified(modified);

    _x86State.getListByClass(C)[regIndex] = vd;
    _x86State._occupied.add(C, regMask);
    _x86State._modified.add(C, static_cast<uint32_t>(modified) << regIndex);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  //! @brief Detach.
  //!
  //! The opposite of 'Attach'. Detach resets the members in 'VarData'
  //! (regIndex, state and changed flags) and unlinks the variable with the
  //! current 'VarState'.
  template<int C>
  ASMJIT_INLINE void detach(VarData* vd, uint32_t regIndex, uint32_t vState) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getRegIndex() == regIndex);
    ASMJIT_ASSERT(vState != kVarStateReg);

    uint32_t regMask = IntUtil::mask(regIndex);

    vd->setState(vState);
    vd->resetRegIndex();
    vd->setModified(false);

    _x86State.getListByClass(C)[regIndex] = NULL;
    _x86State._occupied.del(C, regMask);
    _x86State._modified.del(C, regMask);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Rebase]
  // --------------------------------------------------------------------------

  //! @brief Rebase.
  //!
  //! Change the register of the 'VarData' changing also the current 'VarState'.
  //! Rebase is nearly identical to 'Detach' and 'Attach' sequence, but doesn't
  // change the 'VarData' modified flag.
  template<int C>
  ASMJIT_INLINE void rebase(VarData* vd, uint32_t newRegIndex, uint32_t oldRegIndex) {
    ASMJIT_ASSERT(vd->getClass() == C);

    uint32_t newRegMask = IntUtil::mask(newRegIndex);
    uint32_t oldRegMask = IntUtil::mask(oldRegIndex);
    uint32_t bothRegMask = newRegMask ^ oldRegMask;

    vd->setRegIndex(newRegIndex);

    _x86State.getListByClass(C)[oldRegIndex] = NULL;
    _x86State.getListByClass(C)[newRegIndex] = vd;

    _x86State._occupied.xor_(C, bothRegMask);
    _x86State._modified.xor_(C, bothRegMask & -static_cast<int32_t>(vd->isModified()));

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Load / Save]
  // --------------------------------------------------------------------------

  //! @brief Load.
  //!
  //! Load variable from its memory slot to a register, emitting 'Load'
  //! instruction and changing the variable state to allocated.
  template<int C>
  ASMJIT_INLINE void load(VarData* vd, uint32_t regIndex) {
    // Can be only called if variable is not allocated.
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getState() != kVarStateReg);
    ASMJIT_ASSERT(vd->getRegIndex() == kInvalidReg);

    emitLoad(vd, regIndex, "Load");
    attach<C>(vd, regIndex, false);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  //! @brief Save.
  //!
  //! Save the variable into its home location, but keep it as allocated.
  template<int C>
  ASMJIT_INLINE void save(VarData* vd) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getState() == kVarStateReg);
    ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);

    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = IntUtil::mask(regIndex);

    emitSave(vd, regIndex, "Save");

    vd->setModified(false);
    _x86State._modified.del(C, regMask);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Move / Swap]
  // --------------------------------------------------------------------------

  //! @brief Move a register.
  //!
  //! Move register from one index to another, emitting 'Move' if needed. This
  //! function does nothing if register is already at the given index.
  template<int C>
  ASMJIT_INLINE void move(VarData* vd, uint32_t regIndex) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getState() == kVarStateReg);
    ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);

    uint32_t oldIndex = vd->getRegIndex();
    if (regIndex == oldIndex)
      return;

    emitMove(vd, regIndex, oldIndex, "Move");
    rebase<C>(vd, regIndex, oldIndex);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  //! @brief Swap two registers
  //!
  //! It's only possible to swap Gp registers.
  ASMJIT_INLINE void swapGp(VarData* aVd, VarData* bVd) {
    ASMJIT_ASSERT(aVd != bVd);

    ASMJIT_ASSERT(aVd->getClass() == kRegClassGp);
    ASMJIT_ASSERT(aVd->getState() == kVarStateReg);
    ASMJIT_ASSERT(aVd->getRegIndex() != kInvalidReg);

    ASMJIT_ASSERT(bVd->getClass() == kRegClassGp);
    ASMJIT_ASSERT(bVd->getState() == kVarStateReg);
    ASMJIT_ASSERT(bVd->getRegIndex() != kInvalidReg);

    uint32_t aIndex = aVd->getRegIndex();
    uint32_t bIndex = bVd->getRegIndex();

    emitSwapGp(aVd, bVd, aIndex, bIndex, "Swap");

    aVd->setRegIndex(bIndex);
    bVd->setRegIndex(aIndex);

    _x86State.getListByClass(kRegClassGp)[aIndex] = bVd;
    _x86State.getListByClass(kRegClassGp)[bIndex] = aVd;

    uint32_t m = aVd->isModified() ^ bVd->isModified();
    _x86State._modified.xor_(kRegClassGp, (m << aIndex) | (m << bIndex));

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Alloc / Spill]
  // --------------------------------------------------------------------------

  //! @brief Alloc
  template<int C>
  ASMJIT_INLINE void alloc(VarData* vd, uint32_t regIndex) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(regIndex != kInvalidReg);

    uint32_t oldRegIndex = vd->getRegIndex();
    uint32_t oldState = vd->getState();
    uint32_t regMask = IntUtil::mask(regIndex);

    ASMJIT_ASSERT(_x86State.getListByClass(C)[regIndex] == NULL || regIndex == oldRegIndex);

    if (oldState != kVarStateReg) {
      if (oldState == kVarStateMem)
        emitLoad(vd, regIndex, "Alloc");
      vd->setModified(false);
    }
    else if (oldRegIndex != regIndex) {
      emitMove(vd, regIndex, oldRegIndex, "Alloc");

      _x86State.getListByClass(C)[oldRegIndex] = NULL;
      regMask ^= IntUtil::mask(oldRegIndex);
    }
    else {
      return;
    }

    vd->setState(kVarStateReg);
    vd->setRegIndex(regIndex);

    _x86State.getListByClass(C)[regIndex] = vd;
    _x86State._occupied.xor_(C, regMask);
    _x86State._modified.xor_(C, regMask & -static_cast<int32_t>(vd->isModified()));

    ASMJIT_CONTEXT_CHECK_STATE
  }

  //! @brief Spill.
  //!
  //! Spill variable/register, saves the content to the memory-home if modified.
  template<int C>
  ASMJIT_INLINE void spill(VarData* vd) {
    ASMJIT_ASSERT(vd->getClass() == C);
    if (vd->getState() != kVarStateReg)
      return;

    uint32_t regIndex = vd->getRegIndex();

    ASMJIT_ASSERT(regIndex != kInvalidReg);
    ASMJIT_ASSERT(_x86State.getListByClass(C)[regIndex] == vd);

    if (vd->isModified())
      emitSave(vd, regIndex, "Spill");
    detach<C>(vd, regIndex, kVarStateMem);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Modify]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void modify(VarData* vd) {
    ASMJIT_ASSERT(vd->getClass() == C);

    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = IntUtil::mask(regIndex);

    vd->setModified(true);
    _x86State._modified.add(C, regMask);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Unuse]
  // --------------------------------------------------------------------------

  //! @brief Unuse.
  //!
  //! Unuse variable, it will be detached it if it's allocated then its state
  //! will be changed to kVarStateUnused.
  template<int C>
  ASMJIT_INLINE void unuse(VarData* vd, uint32_t vState = kVarStateUnused) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vState != kVarStateReg);

    uint32_t regIndex = vd->getRegIndex();
    if (regIndex != kInvalidReg)
      detach<C>(vd, regIndex, vState);
    else
      vd->setState(vState);

    ASMJIT_CONTEXT_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! @brief Get state as @ref VarState.
  ASMJIT_INLINE VarState* getState() const { return const_cast<VarState*>(&_x86State); }

  virtual void loadState(BaseVarState* src);
  virtual BaseVarState* saveState();

  virtual void switchState(BaseVarState* src);
  virtual void intersectStates(BaseVarState* a, BaseVarState* b);

  // --------------------------------------------------------------------------
  // [Memory]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Mem getVarMem(VarData* vd) {
    (void)getVarCell(vd);

    Mem mem(_memSlot);
    mem.setBase(vd->getId());
    return mem;
  }

  // --------------------------------------------------------------------------
  // [Prepare]
  // --------------------------------------------------------------------------

  virtual Error fetch();
  virtual Error analyze();

  // --------------------------------------------------------------------------
  // [Translate]
  // --------------------------------------------------------------------------

  virtual Error translate();

  // --------------------------------------------------------------------------
  // [Serialize]
  // --------------------------------------------------------------------------

  virtual Error serialize(BaseAssembler* assembler, BaseNode* start, BaseNode* stop);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief X86/X64 stack-pointer (esp or rsp).
  GpReg _zsp;
  //! @brief X86/X64 frame-pointer (ebp or rbp).
  GpReg _zbp;
  //! @brief Temporary memory operand.
  Mem _memSlot;

  //! @brief X86/X64 specific compiler state (linked with @ref _state).
  VarState _x86State;
  //! @brief Clobbered registers (for the whole function).
  RegMask _clobberedRegs;

  //! @brief Memory cell where is stored address used to restore manually
  //! aligned stack.
  MemCell* _stackFrameCell;

  //! @brief Global allocable registers mask.
  uint32_t _gaRegs[kRegClassCount];

  //! @brief X86/X64 number of Gp/Xmm registers.
  uint8_t _baseRegsCount;
  //! @brief Function arguments base pointer (register).
  uint8_t _argBaseReg;
  //! @brief Function variables base pointer (register).
  uint8_t _varBaseReg;
  //! @brief Whether to emit comments.
  uint8_t _emitComments;

  //! @brief Function arguments base offset.
  int32_t _argBaseOffset;
  //! @brief Function variables base offset.
  int32_t _varBaseOffset;

  //! @brief Function arguments displacement.
  int32_t _argActualDisp;
  //! @brief Function variables displacement.
  int32_t _varActualDisp;

  //! @brief Temporary string builder used for logging.
  StringBuilderT<256> _stringBuilder;
};

//! @}

} // x86x64 namespace
} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86CONTEXT_P_H
