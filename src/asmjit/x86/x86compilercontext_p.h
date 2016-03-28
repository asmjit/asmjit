// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86COMPILERCONTEXT_P_H
#define _ASMJIT_X86_X86COMPILERCONTEXT_P_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/compiler.h"
#include "../base/compilercontext_p.h"
#include "../base/utils.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86VarMap]
// ============================================================================

struct X86VarMap : public VarMap {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get variable-attributes list as VarAttr data.
  ASMJIT_INLINE VarAttr* getVaList() const {
    return const_cast<VarAttr*>(_list);
  }

  //! Get variable-attributes list as VarAttr data (by class).
  ASMJIT_INLINE VarAttr* getVaListByClass(uint32_t rc) const {
    return const_cast<VarAttr*>(_list) + _start.get(rc);
  }

  //! Get position of variables (by class).
  ASMJIT_INLINE uint32_t getVaStart(uint32_t rc) const {
    return _start.get(rc);
  }

  //! Get count of variables (by class).
  ASMJIT_INLINE uint32_t getVaCountByClass(uint32_t rc) const {
    return _count.get(rc);
  }

  //! Get VarAttr at `index`.
  ASMJIT_INLINE VarAttr* getVa(uint32_t index) const {
    ASMJIT_ASSERT(index < _vaCount);
    return getVaList() + index;
  }

  //! Get VarAttr of `c` class at `index`.
  ASMJIT_INLINE VarAttr* getVaByClass(uint32_t rc, uint32_t index) const {
    ASMJIT_ASSERT(index < _count._regs[rc]);
    return getVaListByClass(rc) + index;
  }

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  //! Find VarAttr.
  ASMJIT_INLINE VarAttr* findVa(VarData* vd) const {
    VarAttr* list = getVaList();
    uint32_t count = getVaCount();

    for (uint32_t i = 0; i < count; i++)
      if (list[i].getVd() == vd)
        return &list[i];

    return nullptr;
  }

  //! Find VarAttr (by class).
  ASMJIT_INLINE VarAttr* findVaByClass(uint32_t rc, VarData* vd) const {
    VarAttr* list = getVaListByClass(rc);
    uint32_t count = getVaCountByClass(rc);

    for (uint32_t i = 0; i < count; i++)
      if (list[i].getVd() == vd)
        return &list[i];

    return nullptr;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Special registers on input.
  //!
  //! Special register(s) restricted to one or more physical register. If there
  //! is more than one special register it means that we have to duplicate the
  //! variable content to all of them (it means that the same varible was used
  //! by two or more operands). We forget about duplicates after the register
  //! allocation finishes and marks all duplicates as non-assigned.
  X86RegMask _inRegs;

  //! Special registers on output.
  //!
  //! Special register(s) used on output. Each variable can have only one
  //! special register on the output, 'X86VarMap' contains all registers from
  //! all 'VarAttr's.
  X86RegMask _outRegs;

  //! Clobbered registers (by a function call).
  X86RegMask _clobberedRegs;

  //! Start indexes of variables per register class.
  X86RegCount _start;
  //! Count of variables per register class.
  X86RegCount _count;

  //! VarAttr list.
  VarAttr _list[1];
};

// ============================================================================
// [asmjit::X86StateCell]
// ============================================================================

//! X86/X64 state-cell.
union X86StateCell {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getState() const {
    return _state;
  }

  ASMJIT_INLINE void setState(uint32_t state) {
    _state = static_cast<uint8_t>(state);
  }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() { _packed = 0; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _packed;

  struct {
    uint8_t _state : 2;
    uint8_t _unused : 6;
  };
};

// ============================================================================
// [asmjit::X86VarState]
// ============================================================================

//! X86/X64 state.
struct X86VarState : VarState {
  enum {
    //! Base index of Gp registers.
    kGpIndex = 0,
    //! Count of Gp registers.
    kGpCount = 16,

    //! Base index of Mm registers.
    kMmIndex = kGpIndex + kGpCount,
    //! Count of Mm registers.
    kMmCount = 8,

    //! Base index of Xmm registers.
    kXmmIndex = kMmIndex + kMmCount,
    //! Count of Xmm registers.
    kXmmCount = 16,

    //! Count of all registers in `X86VarState`.
    kAllCount = kXmmIndex + kXmmCount
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE VarData** getList() {
    return _list;
  }

  ASMJIT_INLINE VarData** getListByClass(uint32_t rc) {
    switch (rc) {
      case kX86RegClassGp : return _listGp;
      case kX86RegClassMm : return _listMm;
      case kX86RegClassXyz: return _listXmm;

      default:
        return nullptr;
    }
  }

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset(size_t numCells) {
    ::memset(this, 0, kAllCount * sizeof(VarData*) +
                      2         * sizeof(X86RegMask) +
                      numCells  * sizeof(X86StateCell));
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    //! List of all allocated variables in one array.
    VarData* _list[kAllCount];

    struct {
      //! Allocated Gp registers.
      VarData* _listGp[kGpCount];
      //! Allocated Mm registers.
      VarData* _listMm[kMmCount];
      //! Allocated Xmm registers.
      VarData* _listXmm[kXmmCount];
    };
  };

  //! Occupied registers (mask).
  X86RegMask _occupied;
  //! Modified registers (mask).
  X86RegMask _modified;

  //! Variables data, the length is stored in `X86Context`.
  X86StateCell _cells[1];
};

// ============================================================================
// [asmjit::X86Context]
// ============================================================================

#if defined(ASMJIT_DEBUG)
# define ASMJIT_X86_CHECK_STATE _checkState();
#else
# define ASMJIT_X86_CHECK_STATE
#endif // ASMJIT_DEBUG

//! \internal
//!
//! Compiler context, used by `X86Compiler`.
//!
//! Compiler context takes care of generating function prolog and epilog, and
//! also performs register allocation. It's used during the compilation phase
//! and considered an implementation detail and asmjit consumers don't have
//! access to it. The context is used once per function and it's reset after
//! the function is processed.
struct X86Context : public Context {
  ASMJIT_NO_COPY(X86Context)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `X86Context` instance.
  X86Context(X86Compiler* compiler);
  //! Destroy the `X86Context` instance.
  virtual ~X86Context();

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  virtual void reset(bool releaseMemory = false) override;

  // --------------------------------------------------------------------------
  // [Arch]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isX64() const { return _zsp.getSize() == 16; }
  ASMJIT_INLINE uint32_t getRegSize() const { return _zsp.getSize(); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get compiler as `X86Compiler`.
  ASMJIT_INLINE X86Compiler* getCompiler() const { return static_cast<X86Compiler*>(_compiler); }
  //! Get function as `X86FuncNode`.
  ASMJIT_INLINE X86FuncNode* getFunc() const { return reinterpret_cast<X86FuncNode*>(_func); }
  //! Get clobbered registers (global).
  ASMJIT_INLINE uint32_t getClobberedRegs(uint32_t rc) { return _clobberedRegs.get(rc); }

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86VarMap* newVarMap(uint32_t vaCount) {
    return static_cast<X86VarMap*>(
      _zoneAllocator.alloc(sizeof(X86VarMap) + vaCount * sizeof(VarAttr)));
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

  void emitConvertVarToVar(uint32_t dstType, uint32_t dstIndex, uint32_t srcType, uint32_t srcIndex);
  void emitMoveVarOnStack(uint32_t dstType, const X86Mem* dst, uint32_t srcType, uint32_t srcIndex);
  void emitMoveImmOnStack(uint32_t dstType, const X86Mem* dst, const Imm* src);

  void emitMoveImmToReg(uint32_t dstType, uint32_t dstIndex, const Imm* src);

  // --------------------------------------------------------------------------
  // [Register Management]
  // --------------------------------------------------------------------------

  void _checkState();

  // --------------------------------------------------------------------------
  // [Attach / Detach]
  // --------------------------------------------------------------------------

  //! Attach.
  //!
  //! Attach a register to the 'VarData', changing 'VarData' members to show
  //! that the variable is currently alive and linking variable with the
  //! current 'X86VarState'.
  template<int C>
  ASMJIT_INLINE void attach(VarData* vd, uint32_t regIndex, bool modified) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(regIndex != kInvalidReg);

    // Prevent Esp allocation if C==Gp.
    ASMJIT_ASSERT(C != kX86RegClassGp || regIndex != kX86RegIndexSp);

    uint32_t regMask = Utils::mask(regIndex);

    vd->setState(kVarStateReg);
    vd->setModified(modified);
    vd->setRegIndex(regIndex);
    vd->addHomeIndex(regIndex);

    _x86State.getListByClass(C)[regIndex] = vd;
    _x86State._occupied.or_(C, regMask);
    _x86State._modified.or_(C, static_cast<uint32_t>(modified) << regIndex);

    ASMJIT_X86_CHECK_STATE
  }

  //! Detach.
  //!
  //! The opposite of 'Attach'. Detach resets the members in 'VarData'
  //! (regIndex, state and changed flags) and unlinks the variable with the
  //! current 'X86VarState'.
  template<int C>
  ASMJIT_INLINE void detach(VarData* vd, uint32_t regIndex, uint32_t vState) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getRegIndex() == regIndex);
    ASMJIT_ASSERT(vState != kVarStateReg);

    uint32_t regMask = Utils::mask(regIndex);

    vd->setState(vState);
    vd->resetRegIndex();
    vd->setModified(false);

    _x86State.getListByClass(C)[regIndex] = nullptr;
    _x86State._occupied.andNot(C, regMask);
    _x86State._modified.andNot(C, regMask);

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Rebase]
  // --------------------------------------------------------------------------

  //! Rebase.
  //!
  //! Change the register of the 'VarData' changing also the current 'X86VarState'.
  //! Rebase is nearly identical to 'Detach' and 'Attach' sequence, but doesn't
  //! change the `VarData`s modified flag.
  template<int C>
  ASMJIT_INLINE void rebase(VarData* vd, uint32_t newRegIndex, uint32_t oldRegIndex) {
    ASMJIT_ASSERT(vd->getClass() == C);

    uint32_t newRegMask = Utils::mask(newRegIndex);
    uint32_t oldRegMask = Utils::mask(oldRegIndex);
    uint32_t bothRegMask = newRegMask ^ oldRegMask;

    vd->setRegIndex(newRegIndex);

    _x86State.getListByClass(C)[oldRegIndex] = nullptr;
    _x86State.getListByClass(C)[newRegIndex] = vd;

    _x86State._occupied.xor_(C, bothRegMask);
    _x86State._modified.xor_(C, bothRegMask & -static_cast<int32_t>(vd->isModified()));

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Load / Save]
  // --------------------------------------------------------------------------

  //! Load.
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

    ASMJIT_X86_CHECK_STATE
  }

  //! Save.
  //!
  //! Save the variable into its home location, but keep it as allocated.
  template<int C>
  ASMJIT_INLINE void save(VarData* vd) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getState() == kVarStateReg);
    ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);

    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = Utils::mask(regIndex);

    emitSave(vd, regIndex, "Save");

    vd->setModified(false);
    _x86State._modified.andNot(C, regMask);

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Move / Swap]
  // --------------------------------------------------------------------------

  //! Move a register.
  //!
  //! Move register from one index to another, emitting 'Move' if needed. This
  //! function does nothing if register is already at the given index.
  template<int C>
  ASMJIT_INLINE void move(VarData* vd, uint32_t regIndex) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vd->getState() == kVarStateReg);
    ASMJIT_ASSERT(vd->getRegIndex() != kInvalidReg);

    uint32_t oldIndex = vd->getRegIndex();
    if (regIndex != oldIndex) {
      emitMove(vd, regIndex, oldIndex, "Move");
      rebase<C>(vd, regIndex, oldIndex);
    }

    ASMJIT_X86_CHECK_STATE
  }

  //! Swap two registers
  //!
  //! It's only possible to swap Gp registers.
  ASMJIT_INLINE void swapGp(VarData* aVd, VarData* bVd) {
    ASMJIT_ASSERT(aVd != bVd);

    ASMJIT_ASSERT(aVd->getClass() == kX86RegClassGp);
    ASMJIT_ASSERT(aVd->getState() == kVarStateReg);
    ASMJIT_ASSERT(aVd->getRegIndex() != kInvalidReg);

    ASMJIT_ASSERT(bVd->getClass() == kX86RegClassGp);
    ASMJIT_ASSERT(bVd->getState() == kVarStateReg);
    ASMJIT_ASSERT(bVd->getRegIndex() != kInvalidReg);

    uint32_t aIndex = aVd->getRegIndex();
    uint32_t bIndex = bVd->getRegIndex();

    emitSwapGp(aVd, bVd, aIndex, bIndex, "Swap");

    aVd->setRegIndex(bIndex);
    bVd->setRegIndex(aIndex);

    _x86State.getListByClass(kX86RegClassGp)[aIndex] = bVd;
    _x86State.getListByClass(kX86RegClassGp)[bIndex] = aVd;

    uint32_t m = aVd->isModified() ^ bVd->isModified();
    _x86State._modified.xor_(kX86RegClassGp, (m << aIndex) | (m << bIndex));

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Alloc / Spill]
  // --------------------------------------------------------------------------

  //! Alloc.
  template<int C>
  ASMJIT_INLINE void alloc(VarData* vd, uint32_t regIndex) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(regIndex != kInvalidReg);

    uint32_t oldRegIndex = vd->getRegIndex();
    uint32_t oldState = vd->getState();
    uint32_t regMask = Utils::mask(regIndex);

    ASMJIT_ASSERT(_x86State.getListByClass(C)[regIndex] == nullptr || regIndex == oldRegIndex);

    if (oldState != kVarStateReg) {
      if (oldState == kVarStateMem)
        emitLoad(vd, regIndex, "Alloc");
      vd->setModified(false);
    }
    else if (oldRegIndex != regIndex) {
      emitMove(vd, regIndex, oldRegIndex, "Alloc");

      _x86State.getListByClass(C)[oldRegIndex] = nullptr;
      regMask ^= Utils::mask(oldRegIndex);
    }
    else {
      ASMJIT_X86_CHECK_STATE
      return;
    }

    vd->setState(kVarStateReg);
    vd->setRegIndex(regIndex);
    vd->addHomeIndex(regIndex);

    _x86State.getListByClass(C)[regIndex] = vd;
    _x86State._occupied.xor_(C, regMask);
    _x86State._modified.xor_(C, regMask & -static_cast<int32_t>(vd->isModified()));

    ASMJIT_X86_CHECK_STATE
  }

  //! Spill.
  //!
  //! Spill variable/register, saves the content to the memory-home if modified.
  template<int C>
  ASMJIT_INLINE void spill(VarData* vd) {
    ASMJIT_ASSERT(vd->getClass() == C);

    if (vd->getState() != kVarStateReg) {
      ASMJIT_X86_CHECK_STATE
      return;
    }

    uint32_t regIndex = vd->getRegIndex();

    ASMJIT_ASSERT(regIndex != kInvalidReg);
    ASMJIT_ASSERT(_x86State.getListByClass(C)[regIndex] == vd);

    if (vd->isModified())
      emitSave(vd, regIndex, "Spill");
    detach<C>(vd, regIndex, kVarStateMem);

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Modify]
  // --------------------------------------------------------------------------

  template<int C>
  ASMJIT_INLINE void modify(VarData* vd) {
    ASMJIT_ASSERT(vd->getClass() == C);

    uint32_t regIndex = vd->getRegIndex();
    uint32_t regMask = Utils::mask(regIndex);

    vd->setModified(true);
    _x86State._modified.or_(C, regMask);

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [Unuse]
  // --------------------------------------------------------------------------

  //! Unuse.
  //!
  //! Unuse variable, it will be detached it if it's allocated then its state
  //! will be changed to kVarStateNone.
  template<int C>
  ASMJIT_INLINE void unuse(VarData* vd, uint32_t vState = kVarStateNone) {
    ASMJIT_ASSERT(vd->getClass() == C);
    ASMJIT_ASSERT(vState != kVarStateReg);

    uint32_t regIndex = vd->getRegIndex();
    if (regIndex != kInvalidReg)
      detach<C>(vd, regIndex, vState);
    else
      vd->setState(vState);

    ASMJIT_X86_CHECK_STATE
  }

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! Get state as `X86VarState`.
  ASMJIT_INLINE X86VarState* getState() const {
    return const_cast<X86VarState*>(&_x86State);
  }

  virtual void loadState(VarState* src);
  virtual VarState* saveState();

  virtual void switchState(VarState* src);
  virtual void intersectStates(VarState* a, VarState* b);

  // --------------------------------------------------------------------------
  // [Memory]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Mem getVarMem(VarData* vd) {
    (void)getVarCell(vd);

    X86Mem mem(_memSlot);
    mem.setBase(vd->getId());
    return mem;
  }

  // --------------------------------------------------------------------------
  // [Fetch]
  // --------------------------------------------------------------------------

  virtual Error fetch();

  // --------------------------------------------------------------------------
  // [Annotate]
  // --------------------------------------------------------------------------

  virtual Error annotate();

  // --------------------------------------------------------------------------
  // [Translate]
  // --------------------------------------------------------------------------

  virtual Error translate();

  // --------------------------------------------------------------------------
  // [Serialize]
  // --------------------------------------------------------------------------

  virtual Error serialize(Assembler* assembler, HLNode* start, HLNode* stop);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Count of X86/X64 registers.
  X86RegCount _regCount;
  //! X86/X64 stack-pointer (esp or rsp).
  X86GpReg _zsp;
  //! X86/X64 frame-pointer (ebp or rbp).
  X86GpReg _zbp;
  //! Temporary memory operand.
  X86Mem _memSlot;

  //! X86/X64 specific compiler state, linked to `_state`.
  X86VarState _x86State;
  //! Clobbered registers (for the whole function).
  X86RegMask _clobberedRegs;

  //! Memory cell where is stored address used to restore manually
  //! aligned stack.
  VarCell* _stackFrameCell;

  //! Global allocable registers mask.
  uint32_t _gaRegs[kX86RegClassCount];

  //! Function arguments base pointer (register).
  uint8_t _argBaseReg;
  //! Function variables base pointer (register).
  uint8_t _varBaseReg;
  //! Whether to emit comments.
  uint8_t _emitComments;

  //! Function arguments base offset.
  int32_t _argBaseOffset;
  //! Function variables base offset.
  int32_t _varBaseOffset;

  //! Function arguments displacement.
  int32_t _argActualDisp;
  //! Function variables displacement.
  int32_t _varActualDisp;

  //! Temporary string builder used for logging.
  StringBuilderTmp<256> _stringBuilder;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_X86_X86COMPILERCONTEXT_P_H
