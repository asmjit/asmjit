// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86COMPILER_H
#define _ASMJIT_X86_X86COMPILER_H

// [Dependencies - AsmJit]
#include "../base/compiler.h"
#include "../x86/x86assembler.h"
#include "../x86/x86defs.h"
#include "../x86/x86func.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [CodeGen-Begin]
// ============================================================================

#define INST_0x(_Inst_, _Code_) \
  ASMJIT_INLINE InstNode* _Inst_() { \
    return emit(_Code_); \
  }

#define INST_1x(_Inst_, _Code_, _Op0_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0) { \
    return emit(_Code_, o0); \
  }

#define INST_1x_(_Inst_, _Code_, _Op0_, _Cond_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0); \
  }

#define INST_1i(_Inst_, _Code_, _Op0_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0) { \
    return emit(_Code_, o0); \
  } \
  /* @overload */ \
  ASMJIT_INLINE InstNode* _Inst_(int o0) { \
    return emit(_Code_, o0); \
  }

#define INST_1i_(_Inst_, _Code_, _Op0_, _Cond_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0); \
  } \
  /* @overload */ \
  ASMJIT_INLINE InstNode* _Inst_(int o0) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0); \
  }

#define INST_1cc(_Inst_, _Code_, _Translate_, _Op0_) \
  ASMJIT_INLINE InstNode* _Inst_(uint32_t cc, const _Op0_& o0) { \
    return emit(_Translate_(cc), o0); \
  } \
  \
  ASMJIT_INLINE InstNode* _Inst_##a(const _Op0_& o0) { return emit(_Code_##a, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##ae(const _Op0_& o0) { return emit(_Code_##ae, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##b(const _Op0_& o0) { return emit(_Code_##b, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##be(const _Op0_& o0) { return emit(_Code_##be, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##c(const _Op0_& o0) { return emit(_Code_##c, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##e(const _Op0_& o0) { return emit(_Code_##e, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##g(const _Op0_& o0) { return emit(_Code_##g, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##ge(const _Op0_& o0) { return emit(_Code_##ge, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##l(const _Op0_& o0) { return emit(_Code_##l, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##le(const _Op0_& o0) { return emit(_Code_##le, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##na(const _Op0_& o0) { return emit(_Code_##na, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nae(const _Op0_& o0) { return emit(_Code_##nae, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nb(const _Op0_& o0) { return emit(_Code_##nb, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nbe(const _Op0_& o0) { return emit(_Code_##nbe, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nc(const _Op0_& o0) { return emit(_Code_##nc, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##ne(const _Op0_& o0) { return emit(_Code_##ne, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##ng(const _Op0_& o0) { return emit(_Code_##ng, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nge(const _Op0_& o0) { return emit(_Code_##nge, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nl(const _Op0_& o0) { return emit(_Code_##nl, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nle(const _Op0_& o0) { return emit(_Code_##nle, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##no(const _Op0_& o0) { return emit(_Code_##no, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##np(const _Op0_& o0) { return emit(_Code_##np, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##ns(const _Op0_& o0) { return emit(_Code_##ns, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##nz(const _Op0_& o0) { return emit(_Code_##nz, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##o(const _Op0_& o0) { return emit(_Code_##o, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##p(const _Op0_& o0) { return emit(_Code_##p, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##pe(const _Op0_& o0) { return emit(_Code_##pe, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##po(const _Op0_& o0) { return emit(_Code_##po, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##s(const _Op0_& o0) { return emit(_Code_##s, o0); } \
  ASMJIT_INLINE InstNode* _Inst_##z(const _Op0_& o0) { return emit(_Code_##z, o0); }

#define INST_2x(_Inst_, _Code_, _Op0_, _Op1_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    return emit(_Code_, o0, o1); \
  }

#define INST_2x_(_Inst_, _Code_, _Op0_, _Op1_, _Cond_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1); \
  }

#define INST_2i(_Inst_, _Code_, _Op0_, _Op1_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    return emit(_Code_, o0, o1); \
  } \
  /* @overload */ \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, int o1) { \
    return emit(_Code_, o0, o1); \
  }

#define INST_2i_(_Inst_, _Code_, _Op0_, _Op1_, _Cond_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1); \
  } \
  /* @overload */ \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, int o1) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1); \
  }

#define INST_2cc(_Inst_, _Code_, _Translate_, _Op0_, _Op1_) \
  ASMJIT_INLINE InstNode* _Inst_(uint32_t cc, const _Op0_& o0, const _Op1_& o1) { \
    return emit(_Translate_(cc), o0, o1); \
  } \
  \
  ASMJIT_INLINE InstNode* _Inst_##a(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##a, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##ae(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ae, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##b(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##b, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##be(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##be, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##c(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##c, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##e(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##e, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##g(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##g, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##ge(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ge, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##l(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##l, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##le(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##le, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##na(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##na, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nae(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nae, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nb(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nb, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nbe(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nbe, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nc(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nc, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##ne(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ne, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##ng(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ng, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nge(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nge, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nl(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nl, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nle(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nle, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##no(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##no, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##np(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##np, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##ns(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ns, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##nz(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nz, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##o(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##o, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##p(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##p, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##pe(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##pe, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##po(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##po, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##s(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##s, o0, o1); } \
  ASMJIT_INLINE InstNode* _Inst_##z(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##z, o0, o1); }

#define INST_3x(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3x_(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Cond_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3i(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    return emit(_Code_, o0, o1, o2); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1, int o2) { \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3i_(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Cond_) \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2); \
  } \
  /* @overload */ \
  ASMJIT_INLINE InstNode* _Inst_(const _Op0_& o0, const _Op1_& o1, int o2) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2); \
  }

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct X86X64CallNode;
struct X86X64FuncNode;
struct VarState;

// ============================================================================
// [asmjit::x86x64::kVarAttr]
// ============================================================================

//! @brief X86/X64 VarAttr flags.
ASMJIT_ENUM(kVarAttr) {
  kVarAttrGpbLo = 0x10000000,
  kVarAttrGpbHi = 0x20000000
};

// ============================================================================
// [asmjit::x86x64::VarInst]
// ============================================================================

struct VarInst : public BaseVarInst {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get variable-attributes list as VarAttr data.
  ASMJIT_INLINE VarAttr* getVaList() const
  { return const_cast<VarAttr*>(_list); }

  //! @brief Get variable-attributes list as VarAttr data (by class).
  ASMJIT_INLINE VarAttr* getVaListByClass(uint32_t c) const
  { return const_cast<VarAttr*>(_list) + _start.get(c); }

  //! @brief Get position of variables (by class).
  ASMJIT_INLINE uint32_t getVaStart(uint32_t c) const
  { return _start.get(c); }

  //! @brief Get count of variables (all).
  ASMJIT_INLINE uint32_t getVaCount() const
  { return _vaCount; }

  //! @brief Get count of variables (by class).
  ASMJIT_INLINE uint32_t getVaCountByClass(uint32_t c) const
  { return _count.get(c); }

  //! @brief Get VarAttr at @a index.
  ASMJIT_INLINE VarAttr* getVa(uint32_t index) const
  {
    ASMJIT_ASSERT(index < _vaCount);
    return getVaList() + index;
  }

  //! @brief Get VarAttr of @a c class at @a index.
  ASMJIT_INLINE VarAttr* getVaByClass(uint32_t c, uint32_t index) const
  {
    ASMJIT_ASSERT(index < _count._regs[c]);
    return getVaListByClass(c) + index;
  }

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  //! @brief Find VarAttr.
  ASMJIT_INLINE VarAttr* findVa(VarData* vd) const
  {
    VarAttr* list = getVaList();
    uint32_t count = getVaCount();

    for (uint32_t i = 0; i < count; i++)
      if (list[i].getVd() == vd)
        return &list[i];

    return NULL;
  }

  //! @brief Find VarAttr (by class).
  ASMJIT_INLINE VarAttr* findVaByClass(uint32_t c, VarData* vd) const
  {
    VarAttr* list = getVaListByClass(c);
    uint32_t count = getVaCountByClass(c);

    for (uint32_t i = 0; i < count; i++)
      if (list[i].getVd() == vd)
        return &list[i];

    return NULL;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Variables count.
  uint32_t _vaCount;

  //! @brief Special registers on input.
  //!
  //! Special register(s) restricted to one or more physical register. If there
  //! is more than one special register it means that we have to duplicate the
  //! variable content to all of them (it means that the same varible was used
  //! by two or more operands). We forget about duplicates after the register
  //! allocation finishes and marks all duplicates as non-assigned.
  RegMask _inRegs;

  //! @brief Special registers on output.
  //!
  //! Special register(s) used on output. Each variable can have only one
  //! special register on the output, 'VarInst' contains all registers from
  //! all 'VarAttr's.
  RegMask _outRegs;

  //! @brief Clobbered registers (by a function call).
  RegMask _clobberedRegs;

  //! @brief Start indexes of variables per register class.
  RegCount _start;
  //! @brief Count of variables per register class.
  RegCount _count;

  //! @brief VarAttr list.
  VarAttr _list[1];
};

// ============================================================================
// [asmjit::x86x64::StateCell]
// ============================================================================

//! @brief X86/X64 state-cell.
union StateCell {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getState() const { return _state; }
  ASMJIT_INLINE void setState(uint32_t state) { _state = static_cast<uint8_t>(state); }

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
// [asmjit::x86x64::VarState]
// ============================================================================

//! @brief X86/X64 state.
struct VarState : BaseVarState {
  enum {
    //! @brief Base index for Gp registers.
    kGpIndex = 0,
    //! @brief Count of Gp registers.
    kGpCount = 16,

    //! @brief Base index for Mm registers.
    kMmIndex = kGpIndex + kGpCount,
    //! @brief Count of Mm registers.
    kMmCount = 8,

    //! @brief Base index for Xmm registers.
    kXmmIndex = kMmIndex + kMmCount,
    //! @brief Count of Xmm registers.
    kXmmCount = 16,

    //! @brief Count of all registers in @ref VarState.
    kAllCount = kXmmIndex + kXmmCount
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE VarData** getList()
  { return _list; }

  ASMJIT_INLINE VarData** getListByClass(uint32_t c)
  {
    switch (c) {
      case kRegClassGp: return _listGp;
      case kRegClassMm: return _listMm;
      case kRegClassXy: return _listXmm;

      default:
        return NULL;
    }
  }

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset(size_t numCells)
  {
    ::memset(this, 0, kAllCount * sizeof(VarData* ) +
                      2         * sizeof(RegMask  ) +
                      numCells  * sizeof(StateCell));
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    //! @brief List of all allocated variables in one array.
    VarData* _list[kAllCount];

    struct {
      //! @brief Allocated Gp registers.
      VarData* _listGp[kGpCount];
      //! @brief Allocated Mm registers.
      VarData* _listMm[kMmCount];
      //! @brief Allocated Xmm registers.
      VarData* _listXmm[kXmmCount];
    };
  };

  //! @brief Occupied registers (mask).
  RegMask _occupied;
  //! @brief Modified registers (mask).
  RegMask _modified;

  //! @brief Variables data (count of variables is stored in @ref Context).
  StateCell _cells[1];
};

// ============================================================================
// [asmjit::X86X64FuncNode]
// ============================================================================

//! @brief X86/X64 function node.
struct X86X64FuncNode : public FuncNode {
  ASMJIT_NO_COPY(X86X64FuncNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref X86X64FuncNode instance.
  ASMJIT_INLINE X86X64FuncNode(BaseCompiler* compiler) : FuncNode(compiler) {
    _decl = &_x86Decl;
    _saveRestoreRegs.reset();

    _alignStackSize = 0;
    _alignedMemStackSize = 0;
    _pushPopStackSize = 0;
    _moveStackSize = 0;
    _extraStackSize = 0;

    _stackFrameRegIndex = kInvalidReg;
    _isStackFrameRegPreserved = false;
    _stackFrameCopyGpIndex[0] = kInvalidReg;
    _stackFrameCopyGpIndex[1] = kInvalidReg;
    _stackFrameCopyGpIndex[2] = kInvalidReg;
    _stackFrameCopyGpIndex[3] = kInvalidReg;
    _stackFrameCopyGpIndex[4] = kInvalidReg;
    _stackFrameCopyGpIndex[5] = kInvalidReg;
  }

  //! @brief Destroy the @ref X86X64FuncNode instance.
  ASMJIT_INLINE ~X86X64FuncNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get function declaration as @ref X86X64FuncDecl.
  ASMJIT_INLINE X86X64FuncDecl* getDecl() const {
    return const_cast<X86X64FuncDecl*>(&_x86Decl);
  }

  //! @brief Get argument.
  ASMJIT_INLINE VarData* getArg(uint32_t i) const {
    ASMJIT_ASSERT(i < _x86Decl.getArgCount());
    return static_cast<VarData*>(_argList[i]);
  }

  //! @brief Get registers which have to be saved in prolog/epilog.
  ASMJIT_INLINE uint32_t getSaveRestoreRegs(uint32_t c) { return _saveRestoreRegs.get(c); }

  //! @brief Get stack size needed to align stack back to the nature alignment.
  ASMJIT_INLINE uint32_t getAlignStackSize() const { return _alignStackSize; }
  //! @brief Set stack size needed to align stack back to the nature alignment.
  ASMJIT_INLINE void setAlignStackSize(uint32_t s) { _alignStackSize = s; }

  //! @brief Get aligned stack size used by variables and memory allocated on the stack.
  ASMJIT_INLINE uint32_t getAlignedMemStackSize() const { return _alignedMemStackSize; }

  //! @brief Get stack size used by push/pop sequences in prolog/epilog.
  ASMJIT_INLINE uint32_t getPushPopStackSize() const { return _pushPopStackSize; }
  //! @brief Set stack size used by push/pop sequences in prolog/epilog.
  ASMJIT_INLINE void setPushPopStackSize(uint32_t s) { _pushPopStackSize = s; }

  //! @brief Get stack size used by mov sequences in prolog/epilog.
  ASMJIT_INLINE uint32_t getMoveStackSize() const { return _moveStackSize; }
  //! @brief Set stack size used by mov sequences in prolog/epilog.
  ASMJIT_INLINE void setMoveStackSize(uint32_t s) { _moveStackSize = s; }

  //! @brief Get extra stack size.
  ASMJIT_INLINE uint32_t getExtraStackSize() const { return _extraStackSize; }
  //! @brief Set extra stack size.
  ASMJIT_INLINE void setExtraStackSize(uint32_t s) { _extraStackSize  = s; }

  //! @brief Get whether the function has stack frame register.
  //!
  //! @note Stack frame register can be used for both - aligning purposes or
  //! generating standard prolog/epilog sequence.
  //!
  //! @note Used only when stack is misaligned.
  ASMJIT_INLINE bool hasStackFrameReg() const { return _stackFrameRegIndex != kInvalidReg; }
  //! @brief Get stack frame register index.
  //!
  //! @note Used only when stack is misaligned.
  ASMJIT_INLINE uint32_t getStackFrameRegIndex() const { return _stackFrameRegIndex; }
  //! @brief Get whether the stack frame register is preserved.
  //!
  //! @note Used only when stack is misaligned.
  ASMJIT_INLINE bool isStackFrameRegPreserved() const { return static_cast<bool>(_isStackFrameRegPreserved); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief X86 function decl.
  X86X64FuncDecl _x86Decl;
  //! @brief Registers which must be saved/restored in prolog/epilog.
  RegMask _saveRestoreRegs;

  //! @brief Stack size needed to align function back to the nature alignment.
  uint32_t _alignStackSize;
  //! @brief Like @ref _memStackSize, but aligned.
  uint32_t _alignedMemStackSize;

  //! @brief Stack required for push/pop in prolog/epilog (X86/X64 specific).
  uint32_t _pushPopStackSize;
  //! @brief Stack required for movs in prolog/epilog (X86/X64 specific).
  uint32_t _moveStackSize;

  //! @brief Stack required to put extra data (for example function arguments
  //! when manually aligning to requested alignment).
  uint32_t _extraStackSize;

  //! @brief Stack frame register.
  uint8_t _stackFrameRegIndex;
  //! @brief Whether the stack frame register is preserved.
  uint8_t _isStackFrameRegPreserved;
  //! @brief Gp registers indexes that can be used to copy function arguments
  //! to a new location in case we are doing manual stack alignment.
  uint8_t _stackFrameCopyGpIndex[6];
};

// ============================================================================
// [asmjit::X86X64CallNode]
// ============================================================================

//! @brief X86/X64 function-call node.
struct X86X64CallNode : public CallNode {
  ASMJIT_NO_COPY(X86X64CallNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref X86X64CallNode instance.
  ASMJIT_INLINE X86X64CallNode(BaseCompiler* compiler, const Operand& target) : CallNode(compiler, target) {
    _decl = &_x86Decl;
    _usedArgs.reset();
  }

  //! @brief Destroy the @ref X86X64CallNode instance.
  ASMJIT_INLINE ~X86X64CallNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get function prototype.
  ASMJIT_INLINE X86X64FuncDecl* getDecl() const {
    return const_cast<X86X64FuncDecl*>(&_x86Decl);
  }

  // --------------------------------------------------------------------------
  // [Prototype]
  // --------------------------------------------------------------------------

  //! @brief Set function prototype.
  ASMJIT_API Error setPrototype(uint32_t conv, const FuncPrototype& p);

  // --------------------------------------------------------------------------
  // [Arg / Ret]
  // --------------------------------------------------------------------------

  //! @brief Set argument at @a i to @a op.
  ASMJIT_API bool _setArg(uint32_t i, const Operand& op);
  //! @brief Set return at @a i to @a op.
  ASMJIT_API bool _setRet(uint32_t i, const Operand& op);

  //! @brief Set argument at @a i to @a var.
  ASMJIT_INLINE bool setArg(uint32_t i, const BaseVar& var) { return _setArg(i, var); }
  //! @brief Set argument at @a i to @a imm.
  ASMJIT_INLINE bool setArg(uint32_t i, const Imm& imm) { return _setArg(i, imm); }
  //! @brief Set return at @a i to @a var.
  ASMJIT_INLINE bool setRet(uint32_t i, const BaseVar& var) { return _setRet(i, var); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief X86 declaration.
  X86X64FuncDecl _x86Decl;
  //! @brief Mask of all registers actually used to pass function arguments.
  //!
  //! @note This bit-mask is not the same as @c X86X64Func::_passed. It contains
  //! only registers actually used to do the call while X86X64Func::_passed
  //! mask contains all registers for all function prototype combinations.
  RegMask _usedArgs;
};

// ============================================================================
// [asmjit::x86x64::X86X64Compiler]
// ============================================================================

//! @brief X86/X64 compiler.
//!
//! This class is used to store instruction stream and allows to modify
//! it on the fly. It uses different concept than @c asmjit::Assembler class
//! and in fact @c asmjit::Assembler is only used as a backend. Compiler never
//! emits machine code and each instruction you use is stored to instruction
//! array instead. This allows to modify instruction stream later and for
//! example to reorder instructions to make better performance.
//!
//! @ref asmjit::X86X64Compiler moves code generation to a higher level. Higher
//! level constructs allows to write more abstract and extensible code that
//! is not possible with pure @c asmjit::Assembler class. Because
//! @c asmjit::Compiler needs to create many objects and lifetime of these
//! objects is small (same as @c asmjit::Compiler lifetime itself) it uses
//! very fast memory management model. This model allows to create object
//! instances in nearly zero time (compared to @c malloc() or @c new()
//! operators) so overhead by creating machine code by @c asmjit::Compiler
//! is minimized.
//!
//! @section asmjit_compiler_introduction The Story
//!
//! Before telling you how Compiler works I'd like to write a story. I'd like
//! to cover reasons why this class was created and why I'm recommending to use
//! it. When I released the first version of AsmJit (0.1) it was a toy. The
//! first function I wrote was function which is still available as testjit and
//! which simply returns 1024. The reason why function works for both 32-bit/
//! 64-bit mode and for Windows/Unix specific calling conventions is luck, no
//! arguments usage and no registers usage except returning value in EAX/RAX.
//!
//! Then I started a project called BlitJit which was targetted to generating
//! JIT code for computer graphics. After writing some lines I decided that I
//! can't join pieces of code together without abstraction, should be
//! pixels source pointer in ESI/RSI or EDI/RDI or it's completelly
//! irrellevant? What about destination pointer and SSE2 register for reading
//! input pixels? The simple answer might be "just pick some one and use it".
//!
//! Another reason for abstraction is function calling-conventions. It's really
//! not easy to write assembler code for 32-bit and 64-bit platform supporting
//! three calling conventions (32-bit is similar between Windows and Unix, but
//! 64-bit calling conventions are different).
//!
//! At this time I realized that I can't write code which uses named registers,
//! I need to abstract it. In most cases you don't need specific register, you
//! need to emit instruction that does something with 'virtual' register(s),
//! memory, immediate or label.
//!
//! The first version of AsmJit with Compiler was 0.5 (or 0.6?, can't remember).
//! There was support for 32-bit and 64-bit mode, function calling conventions,
//! but when emitting instructions the developer needed to decide which
//! registers are changed, which are only read or completely overwritten. This
//! model helped a lot when generating code, especially when joining more
//! code-sections together, but there was also small possibility for mistakes.
//! Simply the first version of Compiler was great improvement over low-level
//! Assembler class, but the API design wasn't perfect.
//!
//! The second version of Compiler, completelly rewritten and based on
//! different goals, is part of AsmJit starting at version 1.0. This version
//! was designed after the first one and it contains serious improvements over
//! the old one. The first improvement is that you just use instructions with
//! virtual registers - called variables. When using compiler there is no way
//! to use native registers, there are variables instead. AsmJit is smarter
//! than before and it knows which register is needed only for read (r),
//! read/write (w) or overwrite (x). Supported are also instructions which
//! are using some registers in implicit way (these registers are not part of
//! instruction definition in string form). For example to use CPUID instruction
//! you must give it four variables which will be automatically allocated in
//! input/output registers (EAX, EBX, ECX, EDX).
//!
//! Another improvement is algorithm used by a register allocator. In first
//! version the registers were allocated when creating instruction stream. In
//! new version registers are allocated after calling @c Compiler::make(),
//! thus register allocator has information about scope of all variables and
//! statistics of their usage. The algorithm to allocate registers is very
//! simple and it's always called as a 'linear scan register allocator'. When
//! you get out of registers the all possible variables are scored and the worst
//! is spilled. Of course algorithm ignores the variables used for current
//! instruction.
//!
//! In addition, because registers are allocated after the code stream is
//! generated, the state switches between jumps are handled by Compiler too.
//! You don't need to worry about jumps, compiler always do this dirty work
//! for you.
//!
//! The nearly last thing I'd like to present is calling other functions from
//! the generated code. AsmJit uses a @c FuncPrototype class to hold function
//! parameters, their position in stack (or register index) and return value.
//! This class is used internally, but it can be used to create your own
//! function calling-convention. All standard function calling conventions are
//! implemented.
//!
//! Please enjoy the new version of Compiler, it was created for writing a
//! low-level code using high-level API, leaving developer to concentrate on
//! real problems and not to solving a register puzzle.
//!
//! @section asmjit_compiler_codegeneration Code Generation
//!
//! First that is needed to know about compiler is that compiler never emits
//! machine code. It's used as a middleware between @c asmjit::Assembler and
//! your code. There is also convenience method @c make() that allows to
//! generate machine code directly without creating @c asmjit::Assembler
//! instance.
//!
//! Comparison of generating machine code through @c Assembler and directly
//! by @c Compiler:
//!
//! @code
//! // Assembler instance is low level code generation class that emits
//! // machine code.
//! Assembler a;
//!
//! // Compiler instance is high level code generation class that stores all
//! // instructions in internal representation.
//! Compiler c;
//!
//! // ... put your code here ...
//!
//! // Final step - generate code. asmjit::Compiler::serialize() will send all
//! // instructions into Assembler and this ensures generating real machine code.
//! c.serialize(a);
//!
//! // Your function
//! void* fn = a.make();
//! @endcode
//!
//! Example how to generate machine code using only @c Compiler (preferred):
//!
//! @code
//! // Compiler instance is enough.
//! Compiler c;
//!
//! // ... put your code here ...
//!
//! // Your function
//! void* fn = c.make();
//! @endcode
//!
//! You can see that there is @c asmjit::Compiler::serialize() function that
//! emits instructions into @c asmjit::Assembler(). This layered architecture
//! means that each class is used for something different and there is no code
//! duplication. For convenience there is also @c asmjit::Compiler::make()
//! method that can create your function using @c asmjit::Assembler, but
//! internally (this is preferred bahavior when using @c asmjit::Compiler).
//!
//! The @c make() method allocates memory using @ref BaseRuntime instance passed
//! into the @c Compiler constructor. If code generator is used to create JIT
//! function then virtual memory allocated by @c MemoryManager is used. To get
//! global memory manager use @c MemoryManager::getGlobal().
//!
//! @code
//! // Compiler instance is enough.
//! Compiler c;
//!
//! // ... put your code using Compiler instance ...
//!
//! // Your function
//! void* fn = c.make();
//!
//! // Free it if you don't want it anymore
//! // (using global memory manager instance)
//! MemoryManager::getGlobal()->free(fn);
//! @endcode
//!
//! @section asmjit_compiler_Functions Functions
//!
//! To build functions with @c Compiler, see @c asmjit::Compiler::addFunc()
//! method.
//!
//! @section asmjit_compiler_Variables Variables
//!
//! Compiler is able to manage variables and function arguments. Function
//! arguments are moved to variables by using @c setArg() method, where the
//! first parameter is argument index and second parameter is the variable
//! instance. To declare variable use @c newGpVar(), @c newMmVar() and @c
//! newXmmVar() methods. The @c newXXX() methods accept also parameter
//! describing the variable type. For example the @c newGpVar() method always
//! creates variable which size matches the target architecture size (for
//! 32-bit target the 32-bit variable is created, for 64-bit target the
//! variable size is 64-bit). To override this behavior the variable type
//! must be specified.
//!
//! @code
//! // Compiler and function declaration - void f(int*);
//! Compiler c;
//! GpVar a0(c, kVarTypeIntPtr);
//!
//! c.addFunc(kFuncConvHost, BuildFunction1<FnVoid, int*>());
//! c.setArg(0, a0);
//!
//! // Create your variables.
//! GpVar x0(c, kVarTypeInt32);
//! GpVar x1(c, kVarTypeInt32);
//!
//! // Init your variables.
//! c.mov(x0, 1);
//! c.mov(x1, 2);
//!
//! // ... your code ...
//! c.add(x0, x1);
//! // ... your code ...
//!
//! // Store result to a given pointer in first argument
//! c.mov(dword_ptr(a0), x0);
//!
//! // End of function body.
//! c.endFunc();
//!
//! // Make the function.
//! typedef void (*MyFunc)(int*);
//! MyFunc func = asmjit_cast<MyFunc>(c.make());
//! @endcode
//!
//! This code snipped needs to be explained. You can see that there are more
//! variable types that can be used by @c Compiler. Most useful variables can
//! be allocated using general purpose registers (@c GpVar), MMX registers
//! (@c MmVar) or SSE/SSE2 registers (@c XmmVar).
//!
//! X86/X64 variable types:
//!
//! - @c kVarTypeInt8 - Signed 8-bit integer, mapped to Gpd register (eax, ebx, ...).
//! - @c kVarTypeUInt8 - Unsigned 8-bit integer, mapped to Gpd register (eax, ebx, ...).
//!
//! - @c kVarTypeInt16 - Signed 16-bit integer, mapped to Gpd register (eax, ebx, ...).
//! - @c kVarTypeUInt16 - Unsigned 16-bit integer, mapped to Gpd register (eax, ebx, ...).
//!
//! - @c kVarTypeInt32 - Signed 32-bit integer, mapped to Gpd register (eax, ebx, ...).
//! - @c kVarTypeUInt32 - Unsigned 32-bit integer, mapped to Gpd register (eax, ebx, ...).
//!
//! - @c kVarTypeInt64 - Signed 64-bit integer, mapped to Gpq register (rax, rbx, ...).
//! - @c kVarTypeUInt64 - Unsigned 64-bit integer, mapped to Gpq register (rax, rbx, ...).
//!
//! - @c kVarTypeIntPtr - intptr_t, mapped to Gpd/Gpq register; depends on target, not host!
//! - @c kVarTypeUIntPtr - uintptr_t, mapped to Gpd/Gpq register; depends on target, not host!
//!
//! - @c kVarTypeFp32 - 32-bit floating point register (fp0, fp1, ...).
//! - @c kVarTypeFp64 - 64-bit floating point register (fp0, fp1, ...).
//! - @c kVarTypeFpEx - 80-bit floating point register (fp0, fp1, ...).
//!
//! - @c kVarTypeMm - 64-bit Mm register (mm0, mm1, ...).
//!
//! - @c kVarTypeXmm - 128-bit SSE register.
//! - @c kVarTypeXmmSs - 128-bit SSE register that contains a scalar 32-bit SP-FP value.
//! - @c kVarTypeXmmSd - 128-bit SSE register that contains a scalar 64-bit DP-FP value.
//! - @c kVarTypeXmmPs - 128-bit SSE register that contains 4 packed 32-bit SP-FP values.
//! - @c kVarTypeXmmPd - 128-bit SSE register that contains 2 packed 64-bit DP-FP values.
//!
//! - @c kVarTypeYmm - 256-bit AVX register.
//! - @c kVarTypeYmmPs - 256-bit AVX register that contains 4 packed 32-bit SP-FP values.
//! - @c kVarTypeYmmPd - 256-bit AVX register that contains 2 packed 64-bit DP-FP values.
//!
//! Variable states:
//!
//! - @c kVarStateUnused - State that is assigned to newly created
//!   variables or to not used variables (dereferenced to zero).
//! - @c kVarStateReg - State that means that variable is currently
//!   allocated in register.
//! - @c kVarStateMem - State that means that variable is currently
//!   only in memory location.
//!
//! When you create new variable, initial state is always @c kVarStateUnused,
//! allocating it to register or spilling to memory changes this state to
//! @c kVarStateReg or @c kVarStateMem, respectively.
//! During variable lifetime it's usual that its state is changed multiple
//! times. To generate better code, you can control allocating and spilling
//! by using up to four types of methods that allows it (see next list).
//!
//! Explicit variable allocating / spilling methods:
//!
//! - @c Compiler::alloc() - Explicit method to alloc variable into
//!      register. You can use this before loops or code blocks.
//!
//! - @c Compiler::spill() - Explicit method to spill variable. If variable
//!      is in register and you call this method, it's moved to its home memory
//!      location. If variable is not in register no operation is performed.
//!
//! - @c Compiler::unuse() - Unuse variable (you can use this to end the
//!      variable scope or sub-scope).
//!
//! Please see AsmJit tutorials (testcompiler.cpp and testvariables.cpp) for
//! more complete examples.
//!
//! @section asmjit_compiler_MemoryManagement Memory Management
//!
//! @c Compiler Memory management follows these rules:
//! - Everything created by @c Compiler is always freed by @c Compiler.
//! - To get decent performance, compiler always uses larger memory buffer
//!   for objects to allocate and when compiler instance is destroyed, this
//!   buffer is freed. Destructors of active objects are called when
//!   destroying compiler instance. Destructors of abadonded compiler
//!   objects are called immediately after abadonding them.
//! - This type of memory management is called 'zone memory management'.
//!
//! This means that you can't use any @c Compiler object after destructing it,
//! it also means that each object like @c Label, @c BaseVar and others are
//! created and managed by @c BaseCompiler itself. These objects contain ID
//! which is used internally by Compiler to store additional information about
//! these objects.
//!
//! @section asmjit_compiler_StateManagement Control-Flow and State Management.
//!
//! The @c Compiler automatically manages state of the variables when using
//! control flow instructions like jumps, conditional jumps and calls. There
//! is minimal heuristics for choosing the method how state is saved or restored.
//!
//! Generally the state can be changed only when using jump or conditional jump
//! instruction. When using non-conditional jump then state change is embedded
//! into the instruction stream before the jump. When using conditional jump
//! the @c Compiler decides whether to restore state before the jump or whether
//! to use another block where state is restored. The last case is that no-code
//! have to be emitted and there is no state change (this is of course ideal).
//!
//! Choosing whether to embed 'restore-state' section before conditional jump
//! is quite simple. If jump is likely to be 'taken' then code is embedded, if
//! jump is unlikely to be taken then the small code section for state-switch
//! will be generated instead.
//!
//! Next example is the situation where the extended code block is used to
//! do state-change:
//!
//! @code
//! Compiler c;
//!
//! c.addFunc(kFuncConvHost, FuncBuilder0<FnVoid>());
//!
//! // Labels.
//! Label L0(c);
//!
//! // Variables.
//! GpVar var0(c, kVarTypeInt32);
//! GpVar var1(c, kVarTypeInt32);
//!
//! // Cleanup. After these two lines, the var0 and var1 will be always stored
//! // in registers. Our example is very small, but in larger code the var0 can
//! // be spilled by xor(var1, var1).
//! c.xor_(var0, var0);
//! c.xor_(var1, var1);
//! c.cmp(var0, var1);
//! // State:
//! //   var0 - register.
//! //   var1 - register.
//!
//! // We manually spill these variables.
//! c.spill(var0);
//! c.spill(var1);
//! // State:
//! //   var0 - memory.
//! //   var1 - memory.
//!
//! // Conditional jump to L0. It will be always taken, but compiler thinks that
//! // it is unlikely taken so it will embed state change code somewhere.
//! c.je(L0);
//!
//! // Do something. The variables var0 and var1 will be allocated again.
//! c.add(var0, 1);
//! c.add(var1, 2);
//! // State:
//! //   var0 - register.
//! //   var1 - register.
//!
//! // Bind label here, the state is not changed.
//! c.bind(L0);
//! // State:
//! //   var0 - register.
//! //   var1 - register.
//!
//! // We need to use var0 and var1, because if compiler detects that variables
//! // are out of scope then it optimizes the state-change.
//! c.sub(var0, var1);
//! // State:
//! //   var0 - register.
//! //   var1 - register.
//!
//! c.endFunc();
//! @endcode
//!
//! The output:
//!
//! @verbatim
//! xor eax, eax                    ; xor var_0, var_0
//! xor ecx, ecx                    ; xor var_1, var_1
//! cmp eax, ecx                    ; cmp var_0, var_1
//! mov [esp - 24], eax             ; spill var_0
//! mov [esp - 28], ecx             ; spill var_1
//! je L0_Switch
//! mov eax, [esp - 24]             ; alloc var_0
//! add eax, 1                      ; add var_0, 1
//! mov ecx, [esp - 28]             ; alloc var_1
//! add ecx, 2                      ; add var_1, 2
//! L0:
//! sub eax, ecx                    ; sub var_0, var_1
//! ret
//!
//! ; state-switch begin
//! L0_Switch0:
//! mov eax, [esp - 24]             ; alloc var_0
//! mov ecx, [esp - 28]             ; alloc var_1
//! jmp short L0
//! ; state-switch end
//! @endverbatim
//!
//! You can see that the state-switch section was generated (see L0_Switch0).
//! The compiler is unable to restore state immediately when emitting the
//! forward jump (the code is generated from first to last instruction and
//! the target state is simply not known at this time).
//!
//! To tell @c Compiler that you want to embed state-switch code before jump
//! it's needed to create backward jump (where also processor expects that it
//! will be taken). To demonstrate the possibility to embed state-switch before
//! jump we use slightly modified code:
//!
//! @code
//! Compiler c;
//!
//! c.addFunc(kFuncConvHost, FuncBuilder0<FnVoid>());
//!
//! // Labels.
//! Label L0(c);
//!
//! // Variables.
//! GpVar var0(c, kVarTypeInt32);
//! GpVar var1(c, kVarTypeInt32);
//!
//! // Cleanup. After these two lines, the var0 and var1 will be always stored
//! // in registers. Our example is very small, but in larger code the var0 can
//! // be spilled by xor(var1, var1).
//! c.xor_(var0, var0);
//! c.xor_(var1, var1);
//! // State:
//! //   var0 - register.
//! //   var1 - register.
//!
//! // We manually spill these variables.
//! c.spill(var0);
//! c.spill(var1);
//! // State:
//! //   var0 - memory.
//! //   var1 - memory.
//!
//! // Bind our label here.
//! c.bind(L0);
//!
//! // Do something, the variables will be allocated again.
//! c.add(var0, 1);
//! c.add(var1, 2);
//! // State:
//! //   var0 - register.
//! //   var1 - register.
//!
//! // Backward conditional jump to L0. The default behavior is that it is taken
//! // so state-change code will be embedded here.
//! c.je(L0);
//!
//! c.endFunc();
//! @endcode
//!
//! The output:
//!
//! @verbatim
//! xor ecx, ecx                    ; xor var_0, var_0
//! xor edx, edx                    ; xor var_1, var_1
//! mov [esp - 24], ecx             ; spill var_0
//! mov [esp - 28], edx             ; spill var_1
//! L2:
//! mov ecx, [esp - 24]             ; alloc var_0
//! add ecx, 1                      ; add var_0, 1
//! mov edx, [esp - 28]             ; alloc var_1
//! add edx, 2                      ; add var_1, 2
//!
//! ; state-switch begin
//! mov [esp - 24], ecx             ; spill var_0
//! mov [esp - 28], edx             ; spill var_1
//! ; state-switch end
//!
//! je short L2
//! ret
//! @endverbatim
//!
//! Please notice where the state-switch section is located. The @c Compiler
//! decided that jump is likely to be taken so the state change is embedded
//! before the conditional jump. To change this behavior into the previous
//! case it's needed to add an option (kInstOptionTaken/kInstOptionNotTaken).
//!
//! Replacing the <code>c.je(L0)</code> by <code>c.taken(); c.je(L0)</code>
//! will generate code like this:
//!
//! @verbatim
//! xor ecx, ecx                    ; xor var_0, var_0
//! xor edx, edx                    ; xor var_1, var_1
//! mov [esp - 24], ecx             ; spill var_0
//! mov [esp - 28], edx             ; spill var_1
//! L0:
//! mov ecx, [esp - 24]             ; alloc var_0
//! add ecx, 1                      ; add var_0, a
//! mov edx, [esp - 28]             ; alloc var_1
//! add edx, 2                      ; add var_1, 2
//! je L0_Switch, 2
//! ret
//!
//! ; state-switch begin
//! L0_Switch:
//! mov [esp - 24], ecx             ; spill var_0
//! mov [esp - 28], edx             ; spill var_1
//! jmp short L0
//! ; state-switch end
//! @endverbatim
//!
//! This section provided information about how state-change works. The
//! behavior is deterministic and it can be overridden.
//!
//! @section asmjit_compiler_AdvancedCodeGeneration Advanced Code Generation
//!
//! This section describes advanced method of code generation available to
//! @c Compiler (but also to @c Assembler). When emitting code to instruction
//! stream the methods like @c mov(), @c add(), @c sub() can be called directly
//! (advantage is static-type control performed also by C++ compiler) or
//! indirectly using @c emit() method. The @c emit() method needs only
//! instruction code and operands.
//!
//! Example of code generating by standard type-safe API:
//!
//! @code
//! Compiler c;
//!
//! GpVar var0(c, kVarTypeInt32);
//! GpVar var1(c, kVarTypeInt32);
//!
//! ...
//!
//! c.mov(var0, 0);
//! c.add(var0, var1);
//! c.sub(var0, var1);
//! @endcode
//!
//! The code above can be rewritten as:
//!
//! @code
//! Compiler c;
//!
//! GpVar var0(c, kVarTypeInt32);
//! GpVar var1(c, kVarTypeInt32);
//!
//! ...
//!
//! c.emit(kInstMov, var0, 0);
//! c.emit(kInstAdd, var0, var1);
//! c.emit(kInstSub, var0, var1);
//! @endcode
//!
//! The advantage of first snippet is very friendly API and type-safe control
//! that is controlled by the C++ compiler. The advantage of second snippet is
//! availability to replace or generate instruction code in different places.
//! See the next example how the @c emit() method can be used to generate
//! abstract code.
//!
//! Use case:
//!
//! @code
//! bool emitArithmetic(Compiler& c, XmmVar& var0, XmmVar& var1, const char* op)
//! {
//!   uint32_t code = kInstNone;
//!
//!   if (strcmp(op, "ADD") == 0)
//!     code = kInstAddss;
//!   else if (strcmp(op, "SUBTRACT") == 0)
//!     code = kInstSubss;
//!   else if (strcmp(op, "MULTIPLY") == 0)
//!     code = kInstMulss;
//!   else if (strcmp(op, "DIVIDE") == 0)
//!     code = kInstDivss;
//!   else
//!     // Invalid parameter?
//!     return false;
//!
//!   c.emit(code, var0, var1);
//! }
//! @endcode
//!
//! Other use cases are waiting for you! Be sure that instruction you are
//! emitting is correct and encodable, because if not, Assembler will set
//! status code to @c kErrorAssemblerUnknownInst.
struct X86X64Compiler : public BaseCompiler {
  ASMJIT_NO_COPY(X86X64Compiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a @ref X86X64Compiler instance.
  ASMJIT_API X86X64Compiler(BaseRuntime* runtime);
  //! @brief Destroy the @ref X86X64Compiler instance.
  ASMJIT_API ~X86X64Compiler();

  // --------------------------------------------------------------------------
  // [Inst / Emit]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref InstNode.
  ASMJIT_API InstNode* newInst(uint32_t code);
  //! @overload
  ASMJIT_API InstNode* newInst(uint32_t code, const Operand& o0);
  //! @overload
  ASMJIT_API InstNode* newInst(uint32_t code, const Operand& o0, const Operand& o1);
  //! @overload
  ASMJIT_API InstNode* newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2);
  //! @overload
  ASMJIT_API InstNode* newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3);
  //! @overload
  ASMJIT_API InstNode* newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3, const Operand& o4);

  //! @brief Add a new @ref InstNode.
  ASMJIT_API InstNode* emit(uint32_t code);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0, const Operand& o1);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3, const Operand& o4);

  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, int o0);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0, int o1);
  //! @overload
  ASMJIT_API InstNode* emit(uint32_t code, const Operand& o0, const Operand& o1, int o2);

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref X86X64FuncNode.
  ASMJIT_API X86X64FuncNode* newFunc(uint32_t conv, const FuncPrototype& p);

  //! @brief Add a new function.
  //!
  //! @param cconv Calling convention to use (see @c kFuncConv enum)
  //! @param params Function arguments prototype.
  //!
  //! This method is usually used as a first step when generating functions
  //! by @c Compiler. First parameter @a cconv specifies function calling
  //! convention to use. Second parameter @a params specifies function
  //! arguments. To create function arguments are used templates
  //! @c BuildFunction0<...>, @c BuildFunction1<...>, @c BuildFunction2<...>,
  //! etc...
  //!
  //! Templates with BuildFunction prefix are used to generate argument IDs
  //! based on real C++ types. See next example how to generate function with
  //! two 32-bit integer arguments.
  //!
  //! @code
  //! // Building function using asmjit::Compiler example.
  //!
  //! // Compiler instance
  //! Compiler c;
  //!
  //! // Begin of function (also emits function @c Prolog)
  //! c.addFunc(
  //!   // Default calling convention (32-bit cdecl or 64-bit for host OS)
  //!   kFuncConvHost,
  //!   // Using function builder to generate arguments list
  //!   BuildFunction2<FnVoid, int, int>());
  //!
  //! // End of function (also emits function @c Epilog)
  //! c.endFunc();
  //! @endcode
  //!
  //! You can see that building functions is really easy. Previous code snipped
  //! will generate code for function with two 32-bit integer arguments. You
  //! can access arguments by @c asmjit::Function::argument() method. Arguments
  //! are indexed from 0 (like everything in C).
  //!
  //! @code
  //! // Accessing function arguments through asmjit::Function example.
  //!
  //! // Compiler instance
  //! Compiler c;
  //! GpVar a0(c, kVarTypeInt32);
  //! GpVar a1(c, kVarTypeInt32);
  //!
  //! // Begin of function (also emits function @c Prolog)
  //! c.addFunc(
  //!   // Default calling convention (32-bit cdecl or 64-bit for host OS)
  //!   kFuncConvHost,
  //!   // Using function builder to generate arguments list
  //!   BuildFunction2<FnVoid, int, int>());
  //!
  //! c.setArg(0, a0);
  //! c.setArg(1, a1);
  //!
  //! // Use them.
  //! c.add(a0, a1);
  //!
  //! // End of function - emits function epilog and return instruction.
  //! c.endFunc();
  //! @endcode
  //!
  //! Arguments are like variables. How to manipulate with variables is
  //! documented in @c asmjit::Compiler, variables section.
  //!
  //! @note To get current function use @c currentFunction() method or save
  //! pointer to @c asmjit::Function returned by @c asmjit::Compiler::addFunc<>
  //! method. Recommended is to save the pointer.
  //!
  //! @sa @c BuildFunction0, @c BuildFunction1, @c BuildFunction2, ...
  ASMJIT_API X86X64FuncNode* addFunc(uint32_t conv, const FuncPrototype& p);

  //! @brief End of current function.
  ASMJIT_API EndNode* endFunc();

  //! @brief Get current function as @ref X86X64FuncNode.
  //!
  //! This method can be called within @c addFunc() and @c endFunc()
  //! block to get current function you are working with. It's recommended
  //! to store @c asmjit::Function pointer returned by @c addFunc<> method,
  //! because this allows you in future implement function sections outside of
  //! function itself (yeah, this is possible!).
  ASMJIT_INLINE X86X64FuncNode* getFunc() const { return static_cast<X86X64FuncNode*>(_func); }

  // --------------------------------------------------------------------------
  // [Ret]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref RetNode.
  ASMJIT_API RetNode* newRet(const Operand& o0, const Operand& o1);
  //! @brief Add a new @ref RetNode.
  ASMJIT_API RetNode* addRet(const Operand& o0, const Operand& o1);

  // --------------------------------------------------------------------------
  // [Call]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref X86X64CallNode.
  ASMJIT_API X86X64CallNode* newCall(const Operand& o0, uint32_t conv, const FuncPrototype& p);
  //! @brief Add a new @ref X86X64CallNode.
  ASMJIT_API X86X64CallNode* addCall(const Operand& o0, uint32_t conv, const FuncPrototype& p);

  // --------------------------------------------------------------------------
  // [Vars]
  // --------------------------------------------------------------------------

  //! @brief Set function argument to @a var.
  ASMJIT_API Error setArg(uint32_t argIndex, BaseVar& var);

  //! @overridden
  ASMJIT_API virtual Error _newVar(BaseVar* var, uint32_t type, const char* name);

  //! @brief Create a new Gp variable.
  ASMJIT_INLINE GpVar newGpVar(uint32_t vType = kVarTypeIntPtr, const char* name = NULL) {
    ASMJIT_ASSERT(vType < kVarTypeCount);
    ASMJIT_ASSERT(IntUtil::inInterval<uint32_t>(vType, _kVarTypeIntStart, _kVarTypeIntEnd));

    GpVar var(DontInitialize);
    _newVar(&var, vType, name);
    return var;
  }

  //! @brief Create a new Mm variable.
  ASMJIT_INLINE MmVar newMmVar(uint32_t vType = kVarTypeMm, const char* name = NULL) {
    ASMJIT_ASSERT(vType < kVarTypeCount);
    ASMJIT_ASSERT(IntUtil::inInterval<uint32_t>(vType, _kVarTypeMmStart, _kVarTypeMmEnd));

    MmVar var(DontInitialize);
    _newVar(&var, vType, name);
    return var;
  }

  //! @brief Create a new Xmm variable.
  ASMJIT_INLINE XmmVar newXmmVar(uint32_t vType = kVarTypeXmm, const char* name = NULL) {
    ASMJIT_ASSERT(vType < kVarTypeCount);
    ASMJIT_ASSERT(IntUtil::inInterval<uint32_t>(vType, _kVarTypeXmmStart, _kVarTypeXmmEnd));

    XmmVar var(DontInitialize);
    _newVar(&var, vType, name);
    return var;
  }

  //! @brief Create a new Ymm variable.
  ASMJIT_INLINE YmmVar newYmmVar(uint32_t vType = kVarTypeYmm, const char* name = NULL) {
    ASMJIT_ASSERT(vType < kVarTypeCount);
    ASMJIT_ASSERT(IntUtil::inInterval<uint32_t>(vType, _kVarTypeYmmStart, _kVarTypeYmmEnd));

    YmmVar var(DontInitialize);
    _newVar(&var, vType, name);
    return var;
  }

  //! @brief Get memory home of variable @a var.
  ASMJIT_API void getMemoryHome(BaseVar& var, GpVar* home, int* displacement = NULL);

  //! @brief Set memory home of variable @a var.
  //!
  //! Default memory home location is on stack (ESP/RSP), but when needed the
  //! bebahior can be changed by this method.
  //!
  //! It is an error to chaining memory home locations. For example the given
  //! code is invalid:
  //!
  //! @code
  //! Compiler c;
  //!
  //! ...
  //!
  //! GpVar v0(c, kVarTypeIntPtr);
  //! GpVar v1(c, kVarTypeIntPtr);
  //! GpVar v2(c, kVarTypeIntPtr);
  //! GpVar v3(c, kVarTypeIntPtr);
  //!
  //! c.setMemoryHome(v1, v0, 0); // Allowed, [v0+0] is memory home for v1.
  //! c.setMemoryHome(v2, v0, 4); // Allowed, [v0+4] is memory home for v2.
  //! c.setMemoryHome(v3, v2);    // CHAINING, NOT ALLOWED!
  //! @endcode
  ASMJIT_API void setMemoryHome(BaseVar& var, const GpVar& home, int displacement = 0);

  // --------------------------------------------------------------------------
  // [Stack]
  // --------------------------------------------------------------------------

  //! @overridden
  ASMJIT_API virtual Error _newStack(BaseMem* mem, uint32_t size, uint32_t alignment, const char* name);

  //! @brief Create a new memory chunk allocated on the stack.
  ASMJIT_INLINE Mem newStack(uint32_t size, uint32_t alignment, const char* name = NULL) {
    Mem m(DontInitialize);
    _newStack(&m, size, alignment, name);
    return m;
  }

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! @brief Add 8-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* db(uint8_t x) { return embed(&x, 1); }
  //! @brief Add 16-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dw(uint16_t x) { return embed(&x, 2); }
  //! @brief Add 32-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dd(uint32_t x) { return embed(&x, 4); }
  //! @brief Add 64-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dq(uint64_t x) { return embed(&x, 8); }

  //! @brief Add 8-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dint8(int8_t x) { return embed(&x, static_cast<uint32_t>(sizeof(int8_t))); }
  //! @brief Add 8-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* duint8(uint8_t x) { return embed(&x, static_cast<uint32_t>(sizeof(uint8_t))); }

  //! @brief Add 16-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dint16(int16_t x) { return embed(&x, static_cast<uint32_t>(sizeof(int16_t))); }
  //! @brief Add 16-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* duint16(uint16_t x) { return embed(&x, static_cast<uint32_t>(sizeof(uint16_t))); }

  //! @brief Add 32-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dint32(int32_t x) { return embed(&x, static_cast<uint32_t>(sizeof(int32_t))); }
  //! @brief Add 32-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* duint32(uint32_t x) { return embed(&x, static_cast<uint32_t>(sizeof(uint32_t))); }

  //! @brief Add 64-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dint64(int64_t x) { return embed(&x, static_cast<uint32_t>(sizeof(int64_t))); }
  //! @brief Add 64-bit integer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* duint64(uint64_t x) { return embed(&x, static_cast<uint32_t>(sizeof(uint64_t))); }

  //! @brief Add float data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dfloat(float x) { return embed(&x, static_cast<uint32_t>(sizeof(float))); }
  //! @brief Add double data to the instuction stream.
  ASMJIT_INLINE EmbedNode* ddouble(double x) { return embed(&x, static_cast<uint32_t>(sizeof(double))); }

  //! @brief Add pointer data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dptr(void* x) { return embed(&x, static_cast<uint32_t>(sizeof(void*))); }

  //! @brief Add Mm data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dmm(const MmData& x) { return embed(&x, static_cast<uint32_t>(sizeof(MmData))); }
  //! @brief Add Xmm data to the instuction stream.
  ASMJIT_INLINE EmbedNode* dxmm(const XmmData& x) { return embed(&x, static_cast<uint32_t>(sizeof(XmmData))); }

  //! @brief Add data in a given structure instance to the instuction stream.
  template<typename T>
  ASMJIT_INLINE EmbedNode* dstruct(const T& x) { return embed(&x, static_cast<uint32_t>(sizeof(T))); }

  // --------------------------------------------------------------------------
  // [Make]
  // --------------------------------------------------------------------------

  //! @overridden
  ASMJIT_API virtual void* make();

  // -------------------------------------------------------------------------
  // [Serialize]
  // -------------------------------------------------------------------------

  //! @overridden
  ASMJIT_API virtual Error serialize(BaseAssembler& assembler);

  // -------------------------------------------------------------------------
  // [Options]
  // -------------------------------------------------------------------------

  //! @brief Force short form of jmp/jcc/other instruction.
  ASMJIT_INLINE X86X64Compiler& short_()
  { _options |= kInstOptionShortForm; return *this; }

  //! @brief Force long form of jmp/jcc/other instruction.
  ASMJIT_INLINE X86X64Compiler& long_()
  { _options |= kInstOptionLongForm; return *this; }

  //! @brief Condition is likely to be taken.
  ASMJIT_INLINE X86X64Compiler& taken()
  { _options |= kInstOptionTaken; return *this; }

  //! @brief Condition is unlikely to be taken.
  ASMJIT_INLINE X86X64Compiler& notTaken()
  { _options |= kInstOptionNotTaken; return *this; }

  //! @brief Lock prefix.
  ASMJIT_INLINE X86X64Compiler& lock()
  { _options |= kInstOptionLock; return *this; }

  // --------------------------------------------------------------------------
  // [X86 Instructions]
  // --------------------------------------------------------------------------

  //! @brief Add with carry.
  INST_2x(adc, kInstAdc, GpVar, GpVar)
  //! @overload
  INST_2x(adc, kInstAdc, GpVar, Mem)
  //! @overload
  INST_2i(adc, kInstAdc, GpVar, Imm)
  //! @overload
  INST_2x(adc, kInstAdc, Mem, GpVar)
  //! @overload
  INST_2i(adc, kInstAdc, Mem, Imm)

  //! @brief Add.
  INST_2x(add, kInstAdd, GpVar, GpVar)
  //! @overload
  INST_2x(add, kInstAdd, GpVar, Mem)
  //! @overload
  INST_2i(add, kInstAdd, GpVar, Imm)
  //! @overload
  INST_2x(add, kInstAdd, Mem, GpVar)
  //! @overload
  INST_2i(add, kInstAdd, Mem, Imm)

  //! @brief And.
  INST_2x(and_, kInstAnd, GpVar, GpVar)
  //! @overload
  INST_2x(and_, kInstAnd, GpVar, Mem)
  //! @overload
  INST_2i(and_, kInstAnd, GpVar, Imm)
  //! @overload
  INST_2x(and_, kInstAnd, Mem, GpVar)
  //! @overload
  INST_2i(and_, kInstAnd, Mem, Imm)

  //! @brief Bit scan forward.
  INST_2x_(bsf, kInstBsf, GpVar, GpVar, !o0.isGpb())
  //! @overload
  INST_2x_(bsf, kInstBsf, GpVar, Mem, !o0.isGpb())

  //! @brief Bit scan reverse.
  INST_2x_(bsr, kInstBsr, GpVar, GpVar, !o0.isGpb())
  //! @overload
  INST_2x_(bsr, kInstBsr, GpVar, Mem, !o0.isGpb())

  //! @brief Byte swap (32-bit or 64-bit registers only) (i486).
  INST_1x_(bswap, kInstBswap, GpVar, o0.getSize() >= 4)

  //! @brief Bit test.
  INST_2x(bt, kInstBt, GpVar, GpVar)
  //! @overload
  INST_2i(bt, kInstBt, GpVar, Imm)
  //! @overload
  INST_2x(bt, kInstBt, Mem, GpVar)
  //! @overload
  INST_2i(bt, kInstBt, Mem, Imm)

  //! @brief Bit test and complement.
  INST_2x(btc, kInstBtc, GpVar, GpVar)
  //! @overload
  INST_2i(btc, kInstBtc, GpVar, Imm)
  //! @overload
  INST_2x(btc, kInstBtc, Mem, GpVar)
  //! @overload
  INST_2i(btc, kInstBtc, Mem, Imm)

  //! @brief Bit test and reset.
  INST_2x(btr, kInstBtr, GpVar, GpVar)
  //! @overload
  INST_2i(btr, kInstBtr, GpVar, Imm)
  //! @overload
  INST_2x(btr, kInstBtr, Mem, GpVar)
  //! @overload
  INST_2i(btr, kInstBtr, Mem, Imm)

  //! @brief Bit test and set.
  INST_2x(bts, kInstBts, GpVar, GpVar)
  //! @overload
  INST_2i(bts, kInstBts, GpVar, Imm)
  //! @overload
  INST_2x(bts, kInstBts, Mem, GpVar)
  //! @overload
  INST_2i(bts, kInstBts, Mem, Imm)

  //! @brief Call.
  ASMJIT_INLINE X86X64CallNode* call(const GpVar& dst, uint32_t conv, const FuncPrototype& p)
  { return addCall(dst, conv, p); }
  //! @overload
  ASMJIT_INLINE X86X64CallNode* call(const Mem& dst, uint32_t conv, const FuncPrototype& p)
  { return addCall(dst, conv, p); }
  //! @overload
  ASMJIT_INLINE X86X64CallNode* call(const Imm& dst, uint32_t conv, const FuncPrototype& p)
  { return addCall(dst, conv, p); }
  //! @overload
  ASMJIT_INLINE X86X64CallNode* call(void* dst, uint32_t conv, const FuncPrototype& p) {
    Imm imm((intptr_t)dst);
    return addCall(imm, conv, p);
  }
  //! @overload
  ASMJIT_INLINE X86X64CallNode* call(const Label& label, uint32_t conv, const FuncPrototype& p)
  { return addCall(label, conv, p); }

  //! @brief Clear carry flag
  INST_0x(clc, kInstClc)
  //! @brief Clear direction flag
  INST_0x(cld, kInstCld)
  //! @brief Complement carry Flag.
  INST_0x(cmc, kInstCmc)

  //! @brief Convert byte to word (AX <- Sign Extend AL).
  INST_1x(cbw, kInstCbw, GpVar  /* al */)
  //! @brief Convert word to dword (DX:AX <- Sign Extend AX).
  INST_2x(cwd, kInstCwd, GpVar  /* dx */, GpVar /* ax */)
  //! @brief Convert word to dword (EAX <- Sign Extend AX).
  INST_1x(cwde, kInstCwde, GpVar /* eax */)
  //! @brief Convert dword to qword (EDX:EAX <- Sign Extend EAX).
  INST_2x(cdq, kInstCdq, GpVar /* edx */, GpVar /* eax */)

  //! @brief Conditional move.
  INST_2cc(cmov, kInstCmov, condToCmovcc, GpVar, GpVar)
  //! @brief Conditional move.
  INST_2cc(cmov, kInstCmov, condToCmovcc, GpVar, Mem)

  //! @brief Compare two operands.
  INST_2x(cmp, kInstCmp, GpVar, GpVar)
  //! @overload
  INST_2x(cmp, kInstCmp, GpVar, Mem)
  //! @overload
  INST_2i(cmp, kInstCmp, GpVar, Imm)
  //! @overload
  INST_2x(cmp, kInstCmp, Mem, GpVar)
  //! @overload
  INST_2i(cmp, kInstCmp, Mem, Imm)

  //! @brief Compare and exchange (i486).
  INST_3x(cmpxchg, kInstCmpxchg, GpVar /* eax */, GpVar, GpVar)
  //! @overload
  INST_3x(cmpxchg, kInstCmpxchg, GpVar /* eax */, Mem, GpVar)

  //! @brief Compares the 64-bit value in EDX:EAX with the memory operand (Pentium).
  ASMJIT_INLINE InstNode* cmpxchg8b(
    const GpVar& cmp_edx, const GpVar& cmp_eax,
    const GpVar& cmp_ecx, const GpVar& cmp_ebx,
    const Mem& dst) {

    return emit(kInstCmpxchg8b, cmp_edx, cmp_eax, cmp_ecx, cmp_ebx, dst);
  }

  //! @brief CPU identification (i486).
  ASMJIT_INLINE InstNode* cpuid(
    const GpVar& inout_eax,
    const GpVar& out_ebx,
    const GpVar& out_ecx,
    const GpVar& out_edx) {

    // Destination variables must be different.
    ASMJIT_ASSERT(inout_eax.getId() != out_ebx.getId() &&
                  out_ebx.getId() != out_ecx.getId() &&
                  out_ecx.getId() != out_edx.getId());

    return emit(kInstCpuid, inout_eax, out_ebx, out_ecx, out_edx);
  }

  //! @brief Accumulate crc32 value (polynomial 0x11EDC6F41) (SSE4.2).
  INST_2x_(crc32, kInstCrc32, GpVar, GpVar, o0.isRegType(kRegTypeGpd) || o0.isRegType(kRegTypeGpq))
  //! @overload
  INST_2x_(crc32, kInstCrc32, GpVar, Mem, o0.isRegType(kRegTypeGpd) || o0.isRegType(kRegTypeGpq))

  //! @brief Decrement by 1.
  INST_1x(dec, kInstDec, GpVar)
  //! @overload
  INST_1x(dec, kInstDec, Mem)

  //! @brief Unsigned divide (o0:o1 <- o0:o1 / o2).
  //!
  //! Remainder is stored in @a o0, quotient is stored in @a o1.
  INST_3x_(div, kInstDiv, GpVar, GpVar, GpVar, o0.getId() != o1.getId())
  //! @overload
  INST_3x_(div, kInstDiv, GpVar, GpVar, Mem, o0.getId() != o1.getId())

  //! @brief Signed divide (o0:o1 <- o0:o1 / o2).
  //!
  //! Remainder is stored in @a o0, quotient is stored in @a o1.
  INST_3x_(idiv, kInstIdiv, GpVar, GpVar, GpVar, o0.getId() != o1.getId())
  //! @overload
  INST_3x_(idiv, kInstIdiv, GpVar, GpVar, Mem, o0.getId() != o1.getId())

  //! @brief Signed multiply (o0:o1 <- o1 * o2).
  //!
  //! Hi value is stored in @a o0, lo value is stored in @a o1.
  INST_3x_(imul, kInstImul, GpVar, GpVar, GpVar, o0.getId() != o1.getId())
  //! @overload
  INST_3x_(imul, kInstImul, GpVar, GpVar, Mem, o0.getId() != o1.getId())

  //! @brief Signed multiply.
  INST_2x(imul, kInstImul, GpVar, GpVar)
  //! @overload
  INST_2x(imul, kInstImul, GpVar, Mem)
  //! @overload
  INST_2i(imul, kInstImul, GpVar, Imm)

  //! @brief Signed multiply.
  INST_3i(imul, kInstImul, GpVar, GpVar, Imm)
  //! @overload
  INST_3i(imul, kInstImul, GpVar, Mem, Imm)

  //! @brief Increment by 1.
  INST_1x(inc, kInstInc, GpVar)
  //! @overload
  INST_1x(inc, kInstInc, Mem)

  //! @brief Interrupt.
  INST_1i(int_, kInstInt, Imm)
  //! @brief Interrupt 3 - trap to debugger.
  ASMJIT_INLINE InstNode* int3() { return int_(3); }

  //! @brief Jump to label @a label if condition @a cc is met.
  INST_1cc(j, kInstJ, condToJcc, Label)

  //! @brief Jump.
  INST_1x(jmp, kInstJmp, GpVar)
  //! @overload
  INST_1x(jmp, kInstJmp, Mem)
  //! @overload
  INST_1x(jmp, kInstJmp, Label)
  //! @overload
  INST_1x(jmp, kInstJmp, Imm)
  //! @overload
  ASMJIT_INLINE InstNode* jmp(void* dst) { return jmp(Imm((intptr_t)dst)); }

  //! @brief Load AH from flags.
  INST_1x(lahf, kInstLahf, GpVar)

  //! @brief Load effective address
  INST_2x(lea, kInstLea, GpVar, Mem)

  //! @brief Move.
  INST_2x(mov, kInstMov, GpVar, GpVar)
  //! @overload
  INST_2x(mov, kInstMov, GpVar, Mem)
  //! @overload
  INST_2i(mov, kInstMov, GpVar, Imm)
  //! @overload
  INST_2x(mov, kInstMov, Mem, GpVar)
  //! @overload
  INST_2i(mov, kInstMov, Mem, Imm)

  //! @brief Move from segment register.
  INST_2x(mov, kInstMov, GpVar, SegReg)
  //! @overload
  INST_2x(mov, kInstMov, Mem, SegReg)
  //! @brief Move to segment register.
  INST_2x(mov, kInstMov, SegReg, GpVar)
  //! @overload
  INST_2x(mov, kInstMov, SegReg, Mem)

  //! @brief Move (AL|AX|EAX|RAX <- absolute address in immediate).
  ASMJIT_INLINE InstNode* mov_ptr(const GpVar& dst, void* src) {
    Imm imm(static_cast<int64_t>((intptr_t)src));
    return emit(kInstMovptr, dst, imm);
  }
  //! @brief Move (absolute address in immediate <- AL|AX|EAX|RAX).
  ASMJIT_INLINE InstNode* mov_ptr(void* dst, const GpVar& src) {
    Imm imm(static_cast<int64_t>((intptr_t)dst));
    return emit(kInstMovptr, imm, src);
  }

  //! @brief Move data after swapping bytes (SSE3 - Atom).
  INST_2x_(movbe, kInstMovbe, GpVar, Mem, !o0.isGpb());
  //! @overload
  INST_2x_(movbe, kInstMovbe, Mem, GpVar, !o1.isGpb());

  //! @brief Move with sign-extension.
  INST_2x(movsx, kInstMovsx, GpVar, GpVar)
  //! @overload
  INST_2x(movsx, kInstMovsx, GpVar, Mem)

  //! @brief Move with zero-extension.
  INST_2x(movzx, kInstMovzx, GpVar, GpVar)
  //! @overload
  INST_2x(movzx, kInstMovzx, GpVar, Mem)

  //! @brief Unsigned multiply (o0:o1 <- o1 * o2).
  INST_3x_(mul, kInstMul, GpVar, GpVar, GpVar, o0.getId() != o1.getId())
  //! @overload
  INST_3x_(mul, kInstMul, GpVar, GpVar, Mem, o0.getId() != o1.getId())

  //! @brief Two's complement negation.
  INST_1x(neg, kInstNeg, GpVar)
  //! @overload
  INST_1x(neg, kInstNeg, Mem)

  //! @brief No operation.
  INST_0x(nop, kInstNop)

  //! @brief One's complement negation.
  INST_1x(not_, kInstNot, GpVar)
  //! @overload
  INST_1x(not_, kInstNot, Mem)

  //! @brief Or.
  INST_2x(or_, kInstOr, GpVar, GpVar)
  //! @overload
  INST_2x(or_, kInstOr, GpVar, Mem)
  //! @overload
  INST_2i(or_, kInstOr, GpVar, Imm)
  //! @overload
  INST_2x(or_, kInstOr, Mem, GpVar)
  //! @overload
  INST_2i(or_, kInstOr, Mem, Imm)

  //! @brief Pop a value from the stack.
  INST_1x_(pop, kInstPop, GpVar, o0.getSize() == 2 || o0.getSize() == _regSize)
  //! @overload
  INST_1x_(pop, kInstPop, Mem, o0.getSize() == 2 || o0.getSize() == _regSize)

  //! @brief Pop stack into EFLAGS Register (32-bit or 64-bit).
  INST_0x(popf, kInstPopf)

  //! @brief Return the count of number of bits set to 1 (SSE4.2).
  INST_2x_(popcnt, kInstPopcnt, GpVar, GpVar, !o0.isGpb() && o0.getSize() == o1.getSize())
  //! @overload
  INST_2x_(popcnt, kInstPopcnt, GpVar, Mem, !o0.isGpb())

  //! @brief Push word/dword/qword on the stack.
  INST_1x_(push, kInstPush, GpVar, o0.getSize() == 2 || o0.getSize() == _regSize)
  //! @brief Push word/dword/qword on the stack.
  INST_1x_(push, kInstPush, Mem,o0.getSize() == 2 || o0.getSize() == _regSize)
  //! @brief Push segment register on the stack.
  INST_1x(push, kInstPush, SegReg)
  //! @brief Push word/dword/qword on the stack.
  INST_1i(push, kInstPush, Imm)

  //! @brief Push EFLAGS register (32-bit or 64-bit) on the stack.
  INST_0x(pushf, kInstPushf)

  //! @brief Rotate bits left.
  INST_2x(rcl, kInstRcl, GpVar, GpVar)
  //! @overload
  INST_2x(rcl, kInstRcl, Mem, GpVar)
  //! @brief Rotate bits left.
  INST_2i(rcl, kInstRcl, GpVar, Imm)
  //! @overload
  INST_2i(rcl, kInstRcl, Mem, Imm)

  //! @brief Rotate bits right.
  INST_2x(rcr, kInstRcr, GpVar, GpVar)
  //! @overload
  INST_2x(rcr, kInstRcr, Mem, GpVar)
  //! @brief Rotate bits right.
  INST_2i(rcr, kInstRcr, GpVar, Imm)
  //! @overload
  INST_2i(rcr, kInstRcr, Mem, Imm)

  //! @brief Read time-stamp counter (Pentium).
  INST_2x_(rdtsc, kInstRdtsc, GpVar, GpVar, o0.getId() != o1.getId())
  //! @brief Read time-stamp counter and processor id (Pentium).
  INST_3x_(rdtscp, kInstRdtscp, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Load ECX/RCX bytes from DS:[ESI/RSI] to AL.
  INST_3x_(rep_lodsb, kInstRepLodsb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Load ECX/RCX dwords from DS:[ESI/RSI] to AL.
  INST_3x_(rep_lodsd, kInstRepLodsd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Load ECX/RCX words from DS:[ESI/RSI] to AX.
  INST_3x_(rep_lodsw, kInstRepLodsw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Move ECX/RCX bytes from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x_(rep_movsb, kInstRepMovsb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Move ECX/RCX dwords from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x_(rep_movsd, kInstRepMovsd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Move ECX/RCX dwords from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x_(rep_movsw, kInstRepMovsw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Fill ECX/RCX bytes at ES:[EDI/RDI] with AL.
  INST_3x_(rep_stosb, kInstRepStosb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Fill ECX/RCX dwords at ES:[EDI/RDI] with EAX.
  INST_3x_(rep_stosd, kInstRepStosd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Fill ECX/RCX words at ES:[EDI/RDI] with AX.
  INST_3x_(rep_stosw, kInstRepStosw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Repeated find nonmatching bytes in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x_(repe_cmpsb, kInstRepeCmpsb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Repeated find nonmatching dwords in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x_(repe_cmpsd, kInstRepeCmpsd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Repeated find nonmatching words in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x_(repe_cmpsw, kInstRepeCmpsw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Find non-AL byte starting at ES:[EDI/RDI].
  INST_3x_(repe_scasb, kInstRepeScasb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Find non-EAX dword starting at ES:[EDI/RDI].
  INST_3x_(repe_scasd, kInstRepeScasd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Find non-AX word starting at ES:[EDI/RDI].
  INST_3x_(repe_scasw, kInstRepeScasw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Find matching bytes in [RDI] and [RSI].
  INST_3x_(repne_cmpsb, kInstRepneCmpsb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Find matching dwords in [RDI] and [RSI].
  INST_3x_(repne_cmpsd, kInstRepneCmpsd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Find matching words in [RDI] and [RSI].
  INST_3x_(repne_cmpsw, kInstRepneCmpsw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Find AL, starting at ES:[EDI/RDI].
  INST_3x_(repne_scasb, kInstRepneScasb, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Find EAX, starting at ES:[EDI/RDI].
  INST_3x_(repne_scasd, kInstRepneScasd, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())
  //! @brief Find AX, starting at ES:[EDI/RDI].
  INST_3x_(repne_scasw, kInstRepneScasw, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Return.
  ASMJIT_INLINE RetNode* ret() { return addRet(noOperand, noOperand); }
  //! @overload
  ASMJIT_INLINE RetNode* ret(const GpVar& o0) { return addRet(o0, noOperand); }
  //! @overload
  ASMJIT_INLINE RetNode* ret(const GpVar& o0, const GpVar& o1) { return addRet(o0, o1); }
  //! @overload
  ASMJIT_INLINE RetNode* ret(const XmmVar& o0) { return addRet(o0, noOperand); }
  //! @overload
  ASMJIT_INLINE RetNode* ret(const XmmVar& o0, const XmmVar& o1) { return addRet(o0, o1); }

  //! @brief Rotate bits left.
  INST_2x(rol, kInstRol, GpVar, GpVar)
  //! @overload
  INST_2x(rol, kInstRol, Mem, GpVar)
  //! @brief Rotate bits left.
  INST_2i(rol, kInstRol, GpVar, Imm)
  //! @overload
  INST_2i(rol, kInstRol, Mem, Imm)

  //! @brief Rotate bits right.
  INST_2x(ror, kInstRor, GpVar, GpVar)
  //! @overload
  INST_2x(ror, kInstRor, Mem, GpVar)
  //! @brief Rotate bits right.
  INST_2i(ror, kInstRor, GpVar, Imm)
  //! @overload
  INST_2i(ror, kInstRor, Mem, Imm)

  //! @brief Store @a var (allocated in AH/AX/EAX/RAX) into Flags.
  INST_1x(sahf, kInstSahf, GpVar)

  //! @brief Integer subtraction with borrow.
  INST_2x(sbb, kInstSbb, GpVar, GpVar)
  //! @overload
  INST_2x(sbb, kInstSbb, GpVar, Mem)
  //! @overload
  INST_2i(sbb, kInstSbb, GpVar, Imm)
  //! @overload
  INST_2x(sbb, kInstSbb, Mem, GpVar)
  //! @overload
  INST_2i(sbb, kInstSbb, Mem, Imm)

  //! @brief Shift bits left.
  INST_2x(sal, kInstSal, GpVar, GpVar)
  //! @overload
  INST_2x(sal, kInstSal, Mem, GpVar)
  //! @brief Shift bits left.
  INST_2i(sal, kInstSal, GpVar, Imm)
  //! @overload
  INST_2i(sal, kInstSal, Mem, Imm)

  //! @brief Shift bits right.
  INST_2x(sar, kInstSar, GpVar, GpVar)
  //! @overload
  INST_2x(sar, kInstSar, Mem, GpVar)
  //! @brief Shift bits right.
  INST_2i(sar, kInstSar, GpVar, Imm)
  //! @overload
  INST_2i(sar, kInstSar, Mem, Imm)

  //! @brief Set byte on condition.
  INST_1cc(set, kInstSet, condToSetcc, GpVar)
  //! @brief Set byte on condition.
  INST_1cc(set, kInstSet, condToSetcc, Mem)

  //! @brief Shift bits left.
  INST_2x(shl, kInstShl, GpVar, GpVar)
  //! @overload
  INST_2x(shl, kInstShl, Mem, GpVar)
  //! @brief Shift bits left.
  INST_2i(shl, kInstShl, GpVar, Imm)
  //! @overload
  INST_2i(shl, kInstShl, Mem, Imm)

  //! @brief Shift bits right.
  INST_2x(shr, kInstShr, GpVar, GpVar)
  //! @overload
  INST_2x(shr, kInstShr, Mem, GpVar)
  //! @brief Shift bits right.
  INST_2i(shr, kInstShr, GpVar, Imm)
  //! @overload
  INST_2i(shr, kInstShr, Mem, Imm)

  //! @brief Double precision shift left.
  INST_3x(shld, kInstShld, GpVar, GpVar, GpVar)
  //! @overload
  INST_3x(shld, kInstShld, Mem, GpVar, GpVar)
  //! @brief Double precision shift left.
  INST_3i(shld, kInstShld, GpVar, GpVar, Imm)
  //! @overload
  INST_3i(shld, kInstShld, Mem, GpVar, Imm)

  //! @brief Double precision shift right.
  INST_3x(shrd, kInstShrd, GpVar, GpVar, GpVar)
  //! @overload
  INST_3x(shrd, kInstShrd, Mem, GpVar, GpVar)
  //! @brief Double precision shift right.
  INST_3i(shrd, kInstShrd, GpVar, GpVar, Imm)
  //! @overload
  INST_3i(shrd, kInstShrd, Mem, GpVar, Imm)

  //! @brief Set carry flag to 1.
  INST_0x(stc, kInstStc)
  //! @brief Set direction flag to 1.
  INST_0x(std, kInstStd)

  //! @brief Subtract.
  INST_2x(sub, kInstSub, GpVar, GpVar)
  //! @overload
  INST_2x(sub, kInstSub, GpVar, Mem)
  //! @overload
  INST_2i(sub, kInstSub, GpVar, Imm)
  //! @overload
  INST_2x(sub, kInstSub, Mem, GpVar)
  //! @overload
  INST_2i(sub, kInstSub, Mem, Imm)

  //! @brief Logical compare.
  INST_2x(test, kInstTest, GpVar, GpVar)
  //! @overload
  INST_2i(test, kInstTest, GpVar, Imm)
  //! @overload
  INST_2x(test, kInstTest, Mem, GpVar)
  //! @overload
  INST_2i(test, kInstTest, Mem, Imm)

  //! @brief Undefined instruction - Raise #UD exception.
  INST_0x(ud2, kInstUd2)

  //! @brief Exchange and add.
  INST_2x(xadd, kInstXadd, GpVar, GpVar)
  //! @overload
  INST_2x(xadd, kInstXadd, Mem, GpVar)

  //! @brief Exchange register/memory with register.
  INST_2x(xchg, kInstXchg, GpVar, GpVar)
  //! @overload
  INST_2x(xchg, kInstXchg, Mem, GpVar)
  //! @overload
  INST_2x(xchg, kInstXchg, GpVar, Mem)

  //! @brief Xor.
  INST_2x(xor_, kInstXor, GpVar, GpVar)
  //! @overload
  INST_2x(xor_, kInstXor, GpVar, Mem)
  //! @overload
  INST_2i(xor_, kInstXor, GpVar, Imm)
  //! @overload
  INST_2x(xor_, kInstXor, Mem, GpVar)
  //! @overload
  INST_2i(xor_, kInstXor, Mem, Imm)

  // --------------------------------------------------------------------------
  // [MMX]
  // --------------------------------------------------------------------------

  //! @brief Move dword (MMX).
  INST_2x(movd, kInstMovd, Mem, MmVar)
  //! @overload
  INST_2x(movd, kInstMovd, GpVar, MmVar)
  //! @overload
  INST_2x(movd, kInstMovd, MmVar, Mem)
  //! @overload
  INST_2x(movd, kInstMovd, MmVar, GpVar)

  //! @brief Move qword (MMX).
  INST_2x(movq, kInstMovq, MmVar, MmVar)
  //! @overload
  INST_2x(movq, kInstMovq, Mem, MmVar)
  //! @overload
  INST_2x(movq, kInstMovq, MmVar, Mem)

  //! @brief Pack with signed saturation (MMX).
  INST_2x(packsswb, kInstPacksswb, MmVar, MmVar)
  //! @overload
  INST_2x(packsswb, kInstPacksswb, MmVar, Mem)

  //! @brief Pack with signed saturation (MMX).
  INST_2x(packssdw, kInstPackssdw, MmVar, MmVar)
  //! @overload
  INST_2x(packssdw, kInstPackssdw, MmVar, Mem)

  //! @brief Pack with unsigned saturation (MMX).
  INST_2x(packuswb, kInstPackuswb, MmVar, MmVar)
  //! @overload
  INST_2x(packuswb, kInstPackuswb, MmVar, Mem)

  //! @brief Packed byte add (MMX).
  INST_2x(paddb, kInstPaddb, MmVar, MmVar)
  //! @overload
  INST_2x(paddb, kInstPaddb, MmVar, Mem)

  //! @brief Packed word add (MMX).
  INST_2x(paddw, kInstPaddw, MmVar, MmVar)
  //! @overload
  INST_2x(paddw, kInstPaddw, MmVar, Mem)

  //! @brief Packed dword add (MMX).
  INST_2x(paddd, kInstPaddd, MmVar, MmVar)
  //! @overload
  INST_2x(paddd, kInstPaddd, MmVar, Mem)

  //! @brief Packed add with saturation (MMX).
  INST_2x(paddsb, kInstPaddsb, MmVar, MmVar)
  //! @overload
  INST_2x(paddsb, kInstPaddsb, MmVar, Mem)

  //! @brief Packed add with saturation (MMX).
  INST_2x(paddsw, kInstPaddsw, MmVar, MmVar)
  //! @overload
  INST_2x(paddsw, kInstPaddsw, MmVar, Mem)

  //! @brief Packed add unsigned with saturation (MMX).
  INST_2x(paddusb, kInstPaddusb, MmVar, MmVar)
  //! @overload
  INST_2x(paddusb, kInstPaddusb, MmVar, Mem)

  //! @brief Packed add unsigned with saturation (MMX).
  INST_2x(paddusw, kInstPaddusw, MmVar, MmVar)
  //! @overload
  INST_2x(paddusw, kInstPaddusw, MmVar, Mem)

  //! @brief And (MMX).
  INST_2x(pand, kInstPand, MmVar, MmVar)
  //! @overload
  INST_2x(pand, kInstPand, MmVar, Mem)

  //! @brief And-not (MMX).
  INST_2x(pandn, kInstPandn, MmVar, MmVar)
  //! @overload
  INST_2x(pandn, kInstPandn, MmVar, Mem)

  //! @brief Packed compare bytes for equal (MMX).
  INST_2x(pcmpeqb, kInstPcmpeqb, MmVar, MmVar)
  //! @overload
  INST_2x(pcmpeqb, kInstPcmpeqb, MmVar, Mem)

  //! @brief Packed compare words for equal (MMX).
  INST_2x(pcmpeqw, kInstPcmpeqw, MmVar, MmVar)
  //! @overload
  INST_2x(pcmpeqw, kInstPcmpeqw, MmVar, Mem)

  //! @brief Packed compare dwords for equal (MMX).
  INST_2x(pcmpeqd, kInstPcmpeqd, MmVar, MmVar)
  //! @overload
  INST_2x(pcmpeqd, kInstPcmpeqd, MmVar, Mem)

  //! @brief Packed compare bytes for greater than (MMX).
  INST_2x(pcmpgtb, kInstPcmpgtb, MmVar, MmVar)
  //! @overload
  INST_2x(pcmpgtb, kInstPcmpgtb, MmVar, Mem)

  //! @brief Packed compare words for greater than (MMX).
  INST_2x(pcmpgtw, kInstPcmpgtw, MmVar, MmVar)
  //! @overload
  INST_2x(pcmpgtw, kInstPcmpgtw, MmVar, Mem)

  //! @brief Packed compare dwords for greater than (MMX).
  INST_2x(pcmpgtd, kInstPcmpgtd, MmVar, MmVar)
  //! @overload
  INST_2x(pcmpgtd, kInstPcmpgtd, MmVar, Mem)

  //! @brief Packed multiply high (MMX).
  INST_2x(pmulhw, kInstPmulhw, MmVar, MmVar)
  //! @overload
  INST_2x(pmulhw, kInstPmulhw, MmVar, Mem)

  //! @brief Packed multiply low (MMX).
  INST_2x(pmullw, kInstPmullw, MmVar, MmVar)
  //! @overload
  INST_2x(pmullw, kInstPmullw, MmVar, Mem)

  //! @brief Bitwise logical or (MMX).
  INST_2x(por, kInstPor, MmVar, MmVar)
  //! @overload
  INST_2x(por, kInstPor, MmVar, Mem)

  //! @brief Packed multiply and add (MMX).
  INST_2x(pmaddwd, kInstPmaddwd, MmVar, MmVar)
  //! @overload
  INST_2x(pmaddwd, kInstPmaddwd, MmVar, Mem)

  //! @brief Packed shift left logical (MMX).
  INST_2x(pslld, kInstPslld, MmVar, MmVar)
  //! @overload
  INST_2x(pslld, kInstPslld, MmVar, Mem)
  //! @overload
  INST_2i(pslld, kInstPslld, MmVar, Imm)

  //! @brief Packed shift left logical (MMX).
  INST_2x(psllq, kInstPsllq, MmVar, MmVar)
  //! @overload
  INST_2x(psllq, kInstPsllq, MmVar, Mem)
  //! @overload
  INST_2i(psllq, kInstPsllq, MmVar, Imm)

  //! @brief Packed shift left logical (MMX).
  INST_2x(psllw, kInstPsllw, MmVar, MmVar)
  //! @overload
  INST_2x(psllw, kInstPsllw, MmVar, Mem)
  //! @overload
  INST_2i(psllw, kInstPsllw, MmVar, Imm)

  //! @brief Packed shift right arithmetic (MMX).
  INST_2x(psrad, kInstPsrad, MmVar, MmVar)
  //! @overload
  INST_2x(psrad, kInstPsrad, MmVar, Mem)
  //! @overload
  INST_2i(psrad, kInstPsrad, MmVar, Imm)

  //! @brief Packed shift right arithmetic (MMX).
  INST_2x(psraw, kInstPsraw, MmVar, MmVar)
  //! @overload
  INST_2x(psraw, kInstPsraw, MmVar, Mem)
  //! @overload
  INST_2i(psraw, kInstPsraw, MmVar, Imm)

  //! @brief Packed shift right logical (MMX).
  INST_2x(psrld, kInstPsrld, MmVar, MmVar)
  //! @overload
  INST_2x(psrld, kInstPsrld, MmVar, Mem)
  //! @overload
  INST_2i(psrld, kInstPsrld, MmVar, Imm)

  //! @brief Packed shift right logical (MMX).
  INST_2x(psrlq, kInstPsrlq, MmVar, MmVar)
  //! @overload
  INST_2x(psrlq, kInstPsrlq, MmVar, Mem)
  //! @overload
  INST_2i(psrlq, kInstPsrlq, MmVar, Imm)

  //! @brief Packed shift right logical (MMX).
  INST_2x(psrlw, kInstPsrlw, MmVar, MmVar)
  //! @overload
  INST_2x(psrlw, kInstPsrlw, MmVar, Mem)
  //! @overload
  INST_2i(psrlw, kInstPsrlw, MmVar, Imm)

  //! @brief Packed subtract (MMX).
  INST_2x(psubb, kInstPsubb, MmVar, MmVar)
  //! @overload
  INST_2x(psubb, kInstPsubb, MmVar, Mem)

  //! @brief Packed subtract (MMX).
  INST_2x(psubw, kInstPsubw, MmVar, MmVar)
  //! @overload
  INST_2x(psubw, kInstPsubw, MmVar, Mem)

  //! @brief Packed subtract (MMX).
  INST_2x(psubd, kInstPsubd, MmVar, MmVar)
  //! @overload
  INST_2x(psubd, kInstPsubd, MmVar, Mem)

  //! @brief Packed subtract with saturation (MMX).
  INST_2x(psubsb, kInstPsubsb, MmVar, MmVar)
  //! @overload
  INST_2x(psubsb, kInstPsubsb, MmVar, Mem)

  //! @brief Packed subtract with saturation (MMX).
  INST_2x(psubsw, kInstPsubsw, MmVar, MmVar)
  //! @overload
  INST_2x(psubsw, kInstPsubsw, MmVar, Mem)

  //! @brief Packed subtract with unsigned saturation (MMX).
  INST_2x(psubusb, kInstPsubusb, MmVar, MmVar)
  //! @overload
  INST_2x(psubusb, kInstPsubusb, MmVar, Mem)

  //! @brief Packed subtract with unsigned saturation (MMX).
  INST_2x(psubusw, kInstPsubusw, MmVar, MmVar)
  //! @overload
  INST_2x(psubusw, kInstPsubusw, MmVar, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckhbw, kInstPunpckhbw, MmVar, MmVar)
  //! @overload
  INST_2x(punpckhbw, kInstPunpckhbw, MmVar, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckhwd, kInstPunpckhwd, MmVar, MmVar)
  //! @overload
  INST_2x(punpckhwd, kInstPunpckhwd, MmVar, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckhdq, kInstPunpckhdq, MmVar, MmVar)
  //! @overload
  INST_2x(punpckhdq, kInstPunpckhdq, MmVar, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpcklbw, kInstPunpcklbw, MmVar, MmVar)
  //! @overload
  INST_2x(punpcklbw, kInstPunpcklbw, MmVar, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpcklwd, kInstPunpcklwd, MmVar, MmVar)
  //! @overload
  INST_2x(punpcklwd, kInstPunpcklwd, MmVar, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckldq, kInstPunpckldq, MmVar, MmVar)
  //! @overload
  INST_2x(punpckldq, kInstPunpckldq, MmVar, Mem)

  //! @brief Xor (MMX).
  INST_2x(pxor, kInstPxor, MmVar, MmVar)
  //! @overload
  INST_2x(pxor, kInstPxor, MmVar, Mem)

  //! @brief Empty MMX state.
  INST_0x(emms, kInstEmms)

  // --------------------------------------------------------------------------
  // [3dNow]
  // --------------------------------------------------------------------------

  //! @brief Packed SP-FP to integer convert (3dNow!).
  INST_2x(pf2id, kInstPf2id, MmVar, MmVar)
  //! @overload
  INST_2x(pf2id, kInstPf2id, MmVar, Mem)

  //! @brief  Packed SP-FP to integer word convert (3dNow!).
  INST_2x(pf2iw, kInstPf2iw, MmVar, MmVar)
  //! @overload
  INST_2x(pf2iw, kInstPf2iw, MmVar, Mem)

  //! @brief Packed SP-FP accumulate (3dNow!).
  INST_2x(pfacc, kInstPfacc, MmVar, MmVar)
  //! @overload
  INST_2x(pfacc, kInstPfacc, MmVar, Mem)

  //! @brief Packed SP-FP addition (3dNow!).
  INST_2x(pfadd, kInstPfadd, MmVar, MmVar)
  //! @overload
  INST_2x(pfadd, kInstPfadd, MmVar, Mem)

  //! @brief Packed SP-FP compare - dst == src (3dNow!).
  INST_2x(pfcmpeq, kInstPfcmpeq, MmVar, MmVar)
  //! @overload
  INST_2x(pfcmpeq, kInstPfcmpeq, MmVar, Mem)

  //! @brief Packed SP-FP compare - dst >= src (3dNow!).
  INST_2x(pfcmpge, kInstPfcmpge, MmVar, MmVar)
  //! @overload
  INST_2x(pfcmpge, kInstPfcmpge, MmVar, Mem)

  //! @brief Packed SP-FP compare - dst > src (3dNow!).
  INST_2x(pfcmpgt, kInstPfcmpgt, MmVar, MmVar)
  //! @overload
  INST_2x(pfcmpgt, kInstPfcmpgt, MmVar, Mem)

  //! @brief Packed SP-FP maximum (3dNow!).
  INST_2x(pfmax, kInstPfmax, MmVar, MmVar)
  //! @overload
  INST_2x(pfmax, kInstPfmax, MmVar, Mem)

  //! @brief Packed SP-FP minimum (3dNow!).
  INST_2x(pfmin, kInstPfmin, MmVar, MmVar)
  //! @overload
  INST_2x(pfmin, kInstPfmin, MmVar, Mem)

  //! @brief Packed SP-FP multiply (3dNow!).
  INST_2x(pfmul, kInstPfmul, MmVar, MmVar)
  //! @overload
  INST_2x(pfmul, kInstPfmul, MmVar, Mem)

  //! @brief Packed SP-FP negative accumulate (3dNow!).
  INST_2x(pfnacc, kInstPfnacc, MmVar, MmVar)
  //! @overload
  INST_2x(pfnacc, kInstPfnacc, MmVar, Mem)

  //! @brief Packed SP-FP mixed accumulate (3dNow!).
  INST_2x(pfpnacc, kInstPfpnacc, MmVar, MmVar)
  //! @overload
  INST_2x(pfpnacc, kInstPfpnacc, MmVar, Mem)

  //! @brief Packed SP-FP reciprocal approximation (3dNow!).
  INST_2x(pfrcp, kInstPfrcp, MmVar, MmVar)
  //! @overload
  INST_2x(pfrcp, kInstPfrcp, MmVar, Mem)

  //! @brief Packed SP-FP reciprocal, first iteration step (3dNow!).
  INST_2x(pfrcpit1, kInstPfrcpit1, MmVar, MmVar)
  //! @overload
  INST_2x(pfrcpit1, kInstPfrcpit1, MmVar, Mem)

  //! @brief Packed SP-FP reciprocal, second iteration step (3dNow!).
  INST_2x(pfrcpit2, kInstPfrcpit2, MmVar, MmVar)
  //! @overload
  INST_2x(pfrcpit2, kInstPfrcpit2, MmVar, Mem)

  //! @brief Packed SP-FP reciprocal square root, first iteration step (3dNow!).
  INST_2x(pfrsqit1, kInstPfrsqit1, MmVar, MmVar)
  //! @overload
  INST_2x(pfrsqit1, kInstPfrsqit1, MmVar, Mem)

  //! @brief Packed SP-FP reciprocal square root approximation (3dNow!).
  INST_2x(pfrsqrt, kInstPfrsqrt, MmVar, MmVar)
  //! @overload
  INST_2x(pfrsqrt, kInstPfrsqrt, MmVar, Mem)

  //! @brief Packed SP-FP subtract (3dNow!).
  INST_2x(pfsub, kInstPfsub, MmVar, MmVar)
  //! @overload
  INST_2x(pfsub, kInstPfsub, MmVar, Mem)

  //! @brief Packed SP-FP reverse subtract (3dNow!).
  INST_2x(pfsubr, kInstPfsubr, MmVar, MmVar)
  //! @overload
  INST_2x(pfsubr, kInstPfsubr, MmVar, Mem)

  //! @brief Packed dwords to SP-FP (3dNow!).
  INST_2x(pi2fd, kInstPi2fd, MmVar, MmVar)
  //! @overload
  INST_2x(pi2fd, kInstPi2fd, MmVar, Mem)

  //! @brief Packed words to SP-FP (3dNow!).
  INST_2x(pi2fw, kInstPi2fw, MmVar, MmVar)
  //! @overload
  INST_2x(pi2fw, kInstPi2fw, MmVar, Mem)

  //! @brief Packed swap dword (3dNow!)
  INST_2x(pswapd, kInstPswapd, MmVar, MmVar)
  //! @overload
  INST_2x(pswapd, kInstPswapd, MmVar, Mem)

  //! @brief Prefetch (3dNow!).
  INST_1x(prefetch_3dnow, kInstPrefetch3dNow, Mem)

  //! @brief Prefetch and set cache to modified (3dNow!).
  INST_1x(prefetchw_3dnow, kInstPrefetchw3dNow, Mem)

  //! @brief Faster EMMS (3dNow!).
  INST_0x(femms, kInstFemms)

  // --------------------------------------------------------------------------
  // [SSE]
  // --------------------------------------------------------------------------

  //! @brief Packed SP-FP add (SSE).
  INST_2x(addps, kInstAddps, XmmVar, XmmVar)
  //! @overload
  INST_2x(addps, kInstAddps, XmmVar, Mem)

  //! @brief Scalar SP-FP add (SSE).
  INST_2x(addss, kInstAddss, XmmVar, XmmVar)
  //! @overload
  INST_2x(addss, kInstAddss, XmmVar, Mem)

  //! @brief And-not For SP-FP (SSE).
  INST_2x(andnps, kInstAndnps, XmmVar, XmmVar)
  //! @overload
  INST_2x(andnps, kInstAndnps, XmmVar, Mem)

  //! @brief And for SP-FP (SSE).
  INST_2x(andps, kInstAndps, XmmVar, XmmVar)
  //! @overload
  INST_2x(andps, kInstAndps, XmmVar, Mem)

  //! @brief Packed SP-FP compare (SSE).
  INST_3i(cmpps, kInstCmpps, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(cmpps, kInstCmpps, XmmVar, Mem, Imm)

  //! @brief Compare scalar SP-FP Values (SSE).
  INST_3i(cmpss, kInstCmpss, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(cmpss, kInstCmpss, XmmVar, Mem, Imm)

  //! @brief Scalar ordered SP-FP compare and set EFLAGS (SSE).
  INST_2x(comiss, kInstComiss, XmmVar, XmmVar)
  //! @overload
  INST_2x(comiss, kInstComiss, XmmVar, Mem)

  //! @brief Packed signed INT32 to packed SP-FP conversion (SSE).
  INST_2x(cvtpi2ps, kInstCvtpi2ps, XmmVar, MmVar)
  //! @overload
  INST_2x(cvtpi2ps, kInstCvtpi2ps, XmmVar, Mem)

  //! @brief Packed SP-FP to packed INT32 conversion (SSE).
  INST_2x(cvtps2pi, kInstCvtps2pi, MmVar, XmmVar)
  //! @overload
  INST_2x(cvtps2pi, kInstCvtps2pi, MmVar, Mem)

  //! @brief Scalar signed INT32 to SP-FP conversion (SSE).
  INST_2x(cvtsi2ss, kInstCvtsi2ss, XmmVar, GpVar)
  //! @overload
  INST_2x(cvtsi2ss, kInstCvtsi2ss, XmmVar, Mem)

  //! @brief Scalar SP-FP to Signed INT32 conversion (SSE).
  INST_2x(cvtss2si, kInstCvtss2si, GpVar, XmmVar)
  //! @overload
  INST_2x(cvtss2si, kInstCvtss2si, GpVar, Mem)

  //! @brief Packed SP-FP to packed INT32 conversion (truncate) (SSE).
  INST_2x(cvttps2pi, kInstCvttps2pi, MmVar, XmmVar)
  //! @overload
  INST_2x(cvttps2pi, kInstCvttps2pi, MmVar, Mem)

  //! @brief Scalar SP-FP to signed INT32 conversion (truncate) (SSE).
  INST_2x(cvttss2si, kInstCvttss2si, GpVar, XmmVar)
  //! @overload
  INST_2x(cvttss2si, kInstCvttss2si, GpVar, Mem)

  //! @brief Packed SP-FP divide (SSE).
  INST_2x(divps, kInstDivps, XmmVar, XmmVar)
  //! @overload
  INST_2x(divps, kInstDivps, XmmVar, Mem)

  //! @brief Scalar SP-FP divide (SSE).
  INST_2x(divss, kInstDivss, XmmVar, XmmVar)
  //! @overload
  INST_2x(divss, kInstDivss, XmmVar, Mem)

  //! @brief Load streaming SIMD extension control/status (SSE).
  INST_1x(ldmxcsr, kInstLdmxcsr, Mem)

  //! @brief Byte mask write (SSE).
  //!
  //! @note The default memory location is specified by DS:EDI.
  INST_2x(maskmovq, kInstMaskmovq, MmVar, MmVar)

  //! @brief Packed SP-FP maximum (SSE).
  INST_2x(maxps, kInstMaxps, XmmVar, XmmVar)
  //! @overload
  INST_2x(maxps, kInstMaxps, XmmVar, Mem)

  //! @brief Scalar SP-FP maximum (SSE).
  INST_2x(maxss, kInstMaxss, XmmVar, XmmVar)
  //! @overload
  INST_2x(maxss, kInstMaxss, XmmVar, Mem)

  //! @brief Packed SP-FP minimum (SSE).
  INST_2x(minps, kInstMinps, XmmVar, XmmVar)
  //! @overload
  INST_2x(minps, kInstMinps, XmmVar, Mem)

  //! @brief Scalar SP-FP minimum (SSE).
  INST_2x(minss, kInstMinss, XmmVar, XmmVar)
  //! @overload
  INST_2x(minss, kInstMinss, XmmVar, Mem)

  //! @brief Move aligned packed SP-FP values (SSE).
  INST_2x(movaps, kInstMovaps, XmmVar, XmmVar)
  //! @overload
  INST_2x(movaps, kInstMovaps, XmmVar, Mem)
  //! @brief Move aligned packed SP-FP values (SSE).
  INST_2x(movaps, kInstMovaps, Mem, XmmVar)

  //! @brief Move dword.
  INST_2x(movd, kInstMovd, Mem, XmmVar)
  //! @overload
  INST_2x(movd, kInstMovd, GpVar, XmmVar)
  //! @overload
  INST_2x(movd, kInstMovd, XmmVar, Mem)
  //! @overload
  INST_2x(movd, kInstMovd, XmmVar, GpVar)

  //! @brief Move qword (SSE).
  INST_2x(movq, kInstMovq, XmmVar, XmmVar)
  //! @overload
  INST_2x(movq, kInstMovq, Mem, XmmVar)
  //! @overload
  INST_2x(movq, kInstMovq, XmmVar, Mem)

  //! @brief Move 64 bits non-temporal (SSE).
  INST_2x(movntq, kInstMovntq, Mem, MmVar)

  //! @brief High to low packed SP-FP (SSE).
  INST_2x(movhlps, kInstMovhlps, XmmVar, XmmVar)

  //! @brief Move high packed SP-FP (SSE).
  INST_2x(movhps, kInstMovhps, XmmVar, Mem)
  //! @brief Move high packed SP-FP (SSE).
  INST_2x(movhps, kInstMovhps, Mem, XmmVar)

  //! @brief Move low to high packed SP-FP (SSE).
  INST_2x(movlhps, kInstMovlhps, XmmVar, XmmVar)

  //! @brief Move low packed SP-FP (SSE).
  INST_2x(movlps, kInstMovlps, XmmVar, Mem)
  //! @brief Move low packed SP-FP (SSE).
  INST_2x(movlps, kInstMovlps, Mem, XmmVar)

  //! @brief Move aligned four packed SP-FP non-temporal (SSE).
  INST_2x(movntps, kInstMovntps, Mem, XmmVar)

  //! @brief Move scalar SP-FP (SSE).
  INST_2x(movss, kInstMovss, XmmVar, XmmVar)
  //! @overload
  INST_2x(movss, kInstMovss, XmmVar, Mem)
  //! @overload
  INST_2x(movss, kInstMovss, Mem, XmmVar)

  //! @brief Move unaligned packed SP-FP values (SSE).
  INST_2x(movups, kInstMovups, XmmVar, XmmVar)
  //! @overload
  INST_2x(movups, kInstMovups, XmmVar, Mem)
  //! @overload
  INST_2x(movups, kInstMovups, Mem, XmmVar)

  //! @brief Packed SP-FP multiply (SSE).
  INST_2x(mulps, kInstMulps, XmmVar, XmmVar)
  //! @overload
  INST_2x(mulps, kInstMulps, XmmVar, Mem)

  //! @brief Scalar SP-FP multiply (SSE).
  INST_2x(mulss, kInstMulss, XmmVar, XmmVar)
  //! @overload
  INST_2x(mulss, kInstMulss, XmmVar, Mem)

  //! @brief Or for SP-FP data (SSE).
  INST_2x(orps, kInstOrps, XmmVar, XmmVar)
  //! @overload
  INST_2x(orps, kInstOrps, XmmVar, Mem)

  //! @brief Packed average (SSE).
  INST_2x(pavgb, kInstPavgb, MmVar, MmVar)
  //! @overload
  INST_2x(pavgb, kInstPavgb, MmVar, Mem)

  //! @brief Packed average (SSE).
  INST_2x(pavgw, kInstPavgw, MmVar, MmVar)
  //! @overload
  INST_2x(pavgw, kInstPavgw, MmVar, Mem)

  //! @brief Extract word (SSE).
  INST_3i(pextrw, kInstPextrw, GpVar, MmVar, Imm)

  //! @brief Insert word (SSE).
  INST_3i(pinsrw, kInstPinsrw, MmVar, GpVar, Imm)
  //! @overload
  INST_3i(pinsrw, kInstPinsrw, MmVar, Mem, Imm)

  //! @brief Packed signed integer word maximum (SSE).
  INST_2x(pmaxsw, kInstPmaxsw, MmVar, MmVar)
  //! @overload
  INST_2x(pmaxsw, kInstPmaxsw, MmVar, Mem)

  //! @brief Packed unsigned integer byte maximum (SSE).
  INST_2x(pmaxub, kInstPmaxub, MmVar, MmVar)
  //! @overload
  INST_2x(pmaxub, kInstPmaxub, MmVar, Mem)

  //! @brief Packed signed integer word minimum (SSE).
  INST_2x(pminsw, kInstPminsw, MmVar, MmVar)
  //! @overload
  INST_2x(pminsw, kInstPminsw, MmVar, Mem)

  //! @brief Packed unsigned integer byte minimum (SSE).
  INST_2x(pminub, kInstPminub, MmVar, MmVar)
  //! @overload
  INST_2x(pminub, kInstPminub, MmVar, Mem)

  //! @brief Move byte mask to integer (SSE).
  INST_2x(pmovmskb, kInstPmovmskb, GpVar, MmVar)

  //! @brief Packed multiply high unsigned (SSE).
  INST_2x(pmulhuw, kInstPmulhuw, MmVar, MmVar)
  //! @overload
  INST_2x(pmulhuw, kInstPmulhuw, MmVar, Mem)

  //! @brief Packed sum of absolute differences (SSE).
  INST_2x(psadbw, kInstPsadbw, MmVar, MmVar)
  //! @overload
  INST_2x(psadbw, kInstPsadbw, MmVar, Mem)

  //! @brief Packed shuffle word (SSE).
  INST_3i(pshufw, kInstPshufw, MmVar, MmVar, Imm)
  //! @overload
  INST_3i(pshufw, kInstPshufw, MmVar, Mem, Imm)

  //! @brief Packed SP-FP reciprocal (SSE).
  INST_2x(rcpps, kInstRcpps, XmmVar, XmmVar)
  //! @overload
  INST_2x(rcpps, kInstRcpps, XmmVar, Mem)

  //! @brief Scalar SP-FP reciprocal (SSE).
  INST_2x(rcpss, kInstRcpss, XmmVar, XmmVar)
  //! @overload
  INST_2x(rcpss, kInstRcpss, XmmVar, Mem)

  //! @brief Prefetch (SSE).
  INST_2i(prefetch, kInstPrefetch, Mem, Imm)

  //! @brief Compute sum of absolute differences (SSE).
  INST_2x(psadbw, kInstPsadbw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psadbw, kInstPsadbw, XmmVar, Mem)

  //! @brief Packed SP-FP Square root reciprocal (SSE).
  INST_2x(rsqrtps, kInstRsqrtps, XmmVar, XmmVar)
  //! @overload
  INST_2x(rsqrtps, kInstRsqrtps, XmmVar, Mem)

  //! @brief Scalar SP-FP Square root reciprocal (SSE).
  INST_2x(rsqrtss, kInstRsqrtss, XmmVar, XmmVar)
  //! @overload
  INST_2x(rsqrtss, kInstRsqrtss, XmmVar, Mem)

  //! @brief Store fence (SSE).
  INST_0x(sfence, kInstSfence)

  //! @brief Shuffle SP-FP (SSE).
  INST_3i(shufps, kInstShufps, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(shufps, kInstShufps, XmmVar, Mem, Imm)

  //! @brief Packed SP-FP square root (SSE).
  INST_2x(sqrtps, kInstSqrtps, XmmVar, XmmVar)
  //! @overload
  INST_2x(sqrtps, kInstSqrtps, XmmVar, Mem)

  //! @brief Scalar SP-FP square root (SSE).
  INST_2x(sqrtss, kInstSqrtss, XmmVar, XmmVar)
  //! @overload
  INST_2x(sqrtss, kInstSqrtss, XmmVar, Mem)

  //! @brief Store streaming SIMD extension control/status (SSE).
  INST_1x(stmxcsr, kInstStmxcsr, Mem)

  //! @brief Packed SP-FP subtract (SSE).
  INST_2x(subps, kInstSubps, XmmVar, XmmVar)
  //! @overload
  INST_2x(subps, kInstSubps, XmmVar, Mem)

  //! @brief Scalar SP-FP subtract (SSE).
  INST_2x(subss, kInstSubss, XmmVar, XmmVar)
  //! @overload
  INST_2x(subss, kInstSubss, XmmVar, Mem)

  //! @brief Unordered scalar SP-FP compare and set EFLAGS (SSE).
  INST_2x(ucomiss, kInstUcomiss, XmmVar, XmmVar)
  //! @overload
  INST_2x(ucomiss, kInstUcomiss, XmmVar, Mem)

  //! @brief Unpack high packed SP-FP data (SSE).
  INST_2x(unpckhps, kInstUnpckhps, XmmVar, XmmVar)
  //! @overload
  INST_2x(unpckhps, kInstUnpckhps, XmmVar, Mem)

  //! @brief Unpack low packed SP-FP data (SSE).
  INST_2x(unpcklps, kInstUnpcklps, XmmVar, XmmVar)
  //! @overload
  INST_2x(unpcklps, kInstUnpcklps, XmmVar, Mem)

  //! @brief Xor for SP-FP data (SSE).
  INST_2x(xorps, kInstXorps, XmmVar, XmmVar)
  //! @overload
  INST_2x(xorps, kInstXorps, XmmVar, Mem)

  //! @brief Packed DP-FP add (SSE2).
  INST_2x(addpd, kInstAddpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(addpd, kInstAddpd, XmmVar, Mem)

  //! @brief Scalar DP-FP add (SSE2).
  INST_2x(addsd, kInstAddsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(addsd, kInstAddsd, XmmVar, Mem)

  //! @brief And-not for DP-FP (SSE2).
  INST_2x(andnpd, kInstAndnpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(andnpd, kInstAndnpd, XmmVar, Mem)

  //! @brief And for DP-FP (SSE2).
  INST_2x(andpd, kInstAndpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(andpd, kInstAndpd, XmmVar, Mem)

  //! @brief Flush cache line (SSE2).
  INST_1x(clflush, kInstClflush, Mem)

  //! @brief Packed DP-FP compare (SSE2).
  INST_3i(cmppd, kInstCmppd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(cmppd, kInstCmppd, XmmVar, Mem, Imm)

  //! @brief Compare scalar SP-FP values (SSE2).
  INST_3i(cmpsd, kInstCmpsd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(cmpsd, kInstCmpsd, XmmVar, Mem, Imm)

  //! @brief Scalar ordered DP-FP compare and set EFLAGS (SSE2).
  INST_2x(comisd, kInstComisd, XmmVar, XmmVar)
  //! @overload
  INST_2x(comisd, kInstComisd, XmmVar, Mem)

  //! @brief Convert packed Dword integers to packed DP-FP values (SSE2).
  INST_2x(cvtdq2pd, kInstCvtdq2pd, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtdq2pd, kInstCvtdq2pd, XmmVar, Mem)

  //! @brief Convert packed Dword integers to packed SP-FP values (SSE2).
  INST_2x(cvtdq2ps, kInstCvtdq2ps, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtdq2ps, kInstCvtdq2ps, XmmVar, Mem)

  //! @brief Convert packed DP-FP values to packed dword integers (SSE2).
  INST_2x(cvtpd2dq, kInstCvtpd2dq, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtpd2dq, kInstCvtpd2dq, XmmVar, Mem)

  //! @brief Convert packed DP-FP values to packed dword integers (SSE2).
  INST_2x(cvtpd2pi, kInstCvtpd2pi, MmVar, XmmVar)
  //! @overload
  INST_2x(cvtpd2pi, kInstCvtpd2pi, MmVar, Mem)

  //! @brief Convert packed DP-FP values to packed SP-FP values (SSE2).
  INST_2x(cvtpd2ps, kInstCvtpd2ps, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtpd2ps, kInstCvtpd2ps, XmmVar, Mem)

  //! @brief Convert packed dword integers to packed DP-FP values (SSE2).
  INST_2x(cvtpi2pd, kInstCvtpi2pd, XmmVar, MmVar)
  //! @overload
  INST_2x(cvtpi2pd, kInstCvtpi2pd, XmmVar, Mem)

  //! @brief Convert packed SP-FP values to packed dword integers (SSE2).
  INST_2x(cvtps2dq, kInstCvtps2dq, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtps2dq, kInstCvtps2dq, XmmVar, Mem)

  //! @brief Convert packed SP-FP values to packed DP-FP values (SSE2).
  INST_2x(cvtps2pd, kInstCvtps2pd, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtps2pd, kInstCvtps2pd, XmmVar, Mem)

  //! @brief Convert scalar DP-FP value to dword Integer (SSE2).
  INST_2x(cvtsd2si, kInstCvtsd2si, GpVar, XmmVar)
  //! @overload
  INST_2x(cvtsd2si, kInstCvtsd2si, GpVar, Mem)

  //! @brief Convert scalar DP-FP value to scalar SP-FP value (SSE2).
  INST_2x(cvtsd2ss, kInstCvtsd2ss, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtsd2ss, kInstCvtsd2ss, XmmVar, Mem)

  //! @brief Convert dword integer to scalar DP-FP value (SSE2).
  INST_2x(cvtsi2sd, kInstCvtsi2sd, XmmVar, GpVar)
  //! @overload
  INST_2x(cvtsi2sd, kInstCvtsi2sd, XmmVar, Mem)

  //! @brief Convert scalar SP-FP value to scalar DP-FP value (SSE2).
  INST_2x(cvtss2sd, kInstCvtss2sd, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvtss2sd, kInstCvtss2sd, XmmVar, Mem)

  //! @brief Convert with truncation packed DP-FP values to packed dword Integers (SSE2).
  INST_2x(cvttpd2pi, kInstCvttpd2pi, MmVar, XmmVar)
  //! @overload
  INST_2x(cvttpd2pi, kInstCvttpd2pi, MmVar, Mem)

  //! @brief Convert with truncation packed DP-FP values to packed qword Integers (SSE2).
  INST_2x(cvttpd2dq, kInstCvttpd2dq, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvttpd2dq, kInstCvttpd2dq, XmmVar, Mem)

  //! @brief Convert with truncation packed SP-FP values to packed qword Integers (SSE2).
  INST_2x(cvttps2dq, kInstCvttps2dq, XmmVar, XmmVar)
  //! @overload
  INST_2x(cvttps2dq, kInstCvttps2dq, XmmVar, Mem)

  //! @brief Convert with truncation scalar DP-FP value to signed dword Integer (SSE2).
  INST_2x(cvttsd2si, kInstCvttsd2si, GpVar, XmmVar)
  //! @overload
  INST_2x(cvttsd2si, kInstCvttsd2si, GpVar, Mem)

  //! @brief Packed DP-FP divide (SSE2).
  INST_2x(divpd, kInstDivpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(divpd, kInstDivpd, XmmVar, Mem)

  //! @brief Scalar DP-FP divide (SSE2).
  INST_2x(divsd, kInstDivsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(divsd, kInstDivsd, XmmVar, Mem)

  //! @brief Load fence (SSE2).
  INST_0x(lfence, kInstLfence)

  //! @brief Store selected bytes of oword (SSE2).
  //!
  //! @note Target is DS:EDI.
  INST_2x(maskmovdqu, kInstMaskmovdqu, XmmVar, XmmVar)

  //! @brief Return maximum packed DP-FP values (SSE2).
  INST_2x(maxpd, kInstMaxpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(maxpd, kInstMaxpd, XmmVar, Mem)

  //! @brief Return maximum scalar DP-FP value (SSE2).
  INST_2x(maxsd, kInstMaxsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(maxsd, kInstMaxsd, XmmVar, Mem)

  //! @brief Memory fence (SSE2).
  INST_0x(mfence, kInstMfence)

  //! @brief Return minimum packed DP-FP Values (SSE2).
  INST_2x(minpd, kInstMinpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(minpd, kInstMinpd, XmmVar, Mem)

  //! @brief Return minimum scalar DP-FP value (SSE2).
  INST_2x(minsd, kInstMinsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(minsd, kInstMinsd, XmmVar, Mem)

  //! @brief Move aligned oword (SSE2).
  INST_2x(movdqa, kInstMovdqa, XmmVar, XmmVar)
  //! @overload
  INST_2x(movdqa, kInstMovdqa, XmmVar, Mem)
  //! @overload
  INST_2x(movdqa, kInstMovdqa, Mem, XmmVar)

  //! @brief Move unaligned oword (SSE2).
  INST_2x(movdqu, kInstMovdqu, XmmVar, XmmVar)
  //! @overload
  INST_2x(movdqu, kInstMovdqu, XmmVar, Mem)
  //! @overload
  INST_2x(movdqu, kInstMovdqu, Mem, XmmVar)

  //! @brief Extract packed SP-FP sign mask (SSE2).
  INST_2x(movmskps, kInstMovmskps, GpVar, XmmVar)

  //! @brief Extract packed DP-FP sign mask (SSE2).
  INST_2x(movmskpd, kInstMovmskpd, GpVar, XmmVar)

  //! @brief Move scalar DP-FP value (SSE2).
  INST_2x(movsd, kInstMovsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(movsd, kInstMovsd, XmmVar, Mem)
  //! @overload
  INST_2x(movsd, kInstMovsd, Mem, XmmVar)

  //! @brief Move aligned packed DP-FP values (SSE2).
  INST_2x(movapd, kInstMovapd, XmmVar, XmmVar)
  //! @overload
  INST_2x(movapd, kInstMovapd, XmmVar, Mem)
  //! @overload
  INST_2x(movapd, kInstMovapd, Mem, XmmVar)

  //! @brief Move qword from Xmm to Mm register (SSE2).
  INST_2x(movdq2q, kInstMovdq2q, MmVar, XmmVar)

  //! @brief Move qword from Mm to Xmm register (SSE2).
  INST_2x(movq2dq, kInstMovq2dq, XmmVar, MmVar)

  //! @brief Move high packed DP-FP value (SSE2).
  INST_2x(movhpd, kInstMovhpd, XmmVar, Mem)
  //! @overload
  INST_2x(movhpd, kInstMovhpd, Mem, XmmVar)

  //! @brief Move low packed DP-FP value (SSE2).
  INST_2x(movlpd, kInstMovlpd, XmmVar, Mem)
  //! @overload
  INST_2x(movlpd, kInstMovlpd, Mem, XmmVar)

  //! @brief Store oword using non-temporal hint (SSE2).
  INST_2x(movntdq, kInstMovntdq, Mem, XmmVar)

  //! @brief Store dword using non-temporal hint (SSE2).
  INST_2x(movnti, kInstMovnti, Mem, GpVar)

  //! @brief Store packed DP-FP values using non-temporal hint (SSE2).
  INST_2x(movntpd, kInstMovntpd, Mem, XmmVar)

  //! @brief Move unaligned packed DP-FP values (SSE2).
  INST_2x(movupd, kInstMovupd, XmmVar, XmmVar)
  //! @overload
  INST_2x(movupd, kInstMovupd, XmmVar, Mem)
  //! @overload
  INST_2x(movupd, kInstMovupd, Mem, XmmVar)

  //! @brief Packed DP-FP multiply (SSE2).
  INST_2x(mulpd, kInstMulpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(mulpd, kInstMulpd, XmmVar, Mem)

  //! @brief Scalar DP-FP multiply (SSE2).
  INST_2x(mulsd, kInstMulsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(mulsd, kInstMulsd, XmmVar, Mem)

  //! @brief Or for DP-FP data (SSE2).
  INST_2x(orpd, kInstOrpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(orpd, kInstOrpd, XmmVar, Mem)

  //! @brief Pack with signed saturation (SSE2).
  INST_2x(packsswb, kInstPacksswb, XmmVar, XmmVar)
  //! @overload
  INST_2x(packsswb, kInstPacksswb, XmmVar, Mem)

  //! @brief Pack with signed saturation (SSE2).
  INST_2x(packssdw, kInstPackssdw, XmmVar, XmmVar)
  //! @overload
  INST_2x(packssdw, kInstPackssdw, XmmVar, Mem)

  //! @brief Pack with unsigned saturation (SSE2).
  INST_2x(packuswb, kInstPackuswb, XmmVar, XmmVar)
  //! @overload
  INST_2x(packuswb, kInstPackuswb, XmmVar, Mem)

  //! @brief Packed byte add (SSE2).
  INST_2x(paddb, kInstPaddb, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddb, kInstPaddb, XmmVar, Mem)

  //! @brief Packed word add (SSE2).
  INST_2x(paddw, kInstPaddw, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddw, kInstPaddw, XmmVar, Mem)

  //! @brief Packed dword add (SSE2).
  INST_2x(paddd, kInstPaddd, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddd, kInstPaddd, XmmVar, Mem)

  //! @brief Packed qword add (SSE2).
  INST_2x(paddq, kInstPaddq, MmVar, MmVar)
  //! @overload
  INST_2x(paddq, kInstPaddq, MmVar, Mem)

  //! @brief Packed qword add (SSE2).
  INST_2x(paddq, kInstPaddq, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddq, kInstPaddq, XmmVar, Mem)

  //! @brief Packed add with saturation (SSE2).
  INST_2x(paddsb, kInstPaddsb, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddsb, kInstPaddsb, XmmVar, Mem)

  //! @brief Packed add with saturation (SSE2).
  INST_2x(paddsw, kInstPaddsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddsw, kInstPaddsw, XmmVar, Mem)

  //! @brief Packed add unsigned with saturation (SSE2).
  INST_2x(paddusb, kInstPaddusb, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddusb, kInstPaddusb, XmmVar, Mem)

  //! @brief Packed add unsigned with saturation (SSE2).
  INST_2x(paddusw, kInstPaddusw, XmmVar, XmmVar)
  //! @overload
  INST_2x(paddusw, kInstPaddusw, XmmVar, Mem)

  //! @brief And (SSE2).
  INST_2x(pand, kInstPand, XmmVar, XmmVar)
  //! @overload
  INST_2x(pand, kInstPand, XmmVar, Mem)

  //! @brief And-not (SSE2).
  INST_2x(pandn, kInstPandn, XmmVar, XmmVar)
  //! @overload
  INST_2x(pandn, kInstPandn, XmmVar, Mem)

  //! @brief Spin loop hint (SSE2).
  INST_0x(pause, kInstPause)

  //! @brief Packed average (SSE2).
  INST_2x(pavgb, kInstPavgb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pavgb, kInstPavgb, XmmVar, Mem)

  //! @brief Packed average (SSE2).
  INST_2x(pavgw, kInstPavgw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pavgw, kInstPavgw, XmmVar, Mem)

  //! @brief Packed compare bytes for equal (SSE2).
  INST_2x(pcmpeqb, kInstPcmpeqb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpeqb, kInstPcmpeqb, XmmVar, Mem)

  //! @brief Packed compare words for equal (SSE2).
  INST_2x(pcmpeqw, kInstPcmpeqw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpeqw, kInstPcmpeqw, XmmVar, Mem)

  //! @brief Packed compare dwords for equal (SSE2).
  INST_2x(pcmpeqd, kInstPcmpeqd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpeqd, kInstPcmpeqd, XmmVar, Mem)

  //! @brief Packed compare bytes for greater than (SSE2).
  INST_2x(pcmpgtb, kInstPcmpgtb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpgtb, kInstPcmpgtb, XmmVar, Mem)

  //! @brief Packed compare words for greater than (SSE2).
  INST_2x(pcmpgtw, kInstPcmpgtw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpgtw, kInstPcmpgtw, XmmVar, Mem)

  //! @brief Packed compare dwords for greater than (SSE2).
  INST_2x(pcmpgtd, kInstPcmpgtd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpgtd, kInstPcmpgtd, XmmVar, Mem)

  //! @brief Extract word (SSE2).
  INST_3i(pextrw, kInstPextrw, GpVar, XmmVar, Imm)

  //! @brief Insert word (SSE2).
  INST_3i(pinsrw, kInstPinsrw, XmmVar, GpVar, Imm)
  //! @overload
  INST_3i(pinsrw, kInstPinsrw, XmmVar, Mem, Imm)

  //! @brief Packed signed integer word maximum (SSE2).
  INST_2x(pmaxsw, kInstPmaxsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaxsw, kInstPmaxsw, XmmVar, Mem)

  //! @brief Packed unsigned integer byte maximum (SSE2).
  INST_2x(pmaxub, kInstPmaxub, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaxub, kInstPmaxub, XmmVar, Mem)

  //! @brief Packed signed integer word minimum (SSE2).
  INST_2x(pminsw, kInstPminsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pminsw, kInstPminsw, XmmVar, Mem)

  //! @brief Packed unsigned integer byte minimum (SSE2).
  INST_2x(pminub, kInstPminub, XmmVar, XmmVar)
  //! @overload
  INST_2x(pminub, kInstPminub, XmmVar, Mem)

  //! @brief Move byte mask (SSE2).
  INST_2x(pmovmskb, kInstPmovmskb, GpVar, XmmVar)

  //! @brief Packed multiply high (SSE2).
  INST_2x(pmulhw, kInstPmulhw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmulhw, kInstPmulhw, XmmVar, Mem)

  //! @brief Packed multiply high Unsigned (SSE2).
  INST_2x(pmulhuw, kInstPmulhuw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmulhuw, kInstPmulhuw, XmmVar, Mem)

  //! @brief Packed multiply low (SSE2).
  INST_2x(pmullw, kInstPmullw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmullw, kInstPmullw, XmmVar, Mem)

  //! @brief Packed multiply to QWORD (SSE2).
  INST_2x(pmuludq, kInstPmuludq, MmVar, MmVar)
  //! @overload
  INST_2x(pmuludq, kInstPmuludq, MmVar, Mem)

  //! @brief Packed multiply to QWORD (SSE2).
  INST_2x(pmuludq, kInstPmuludq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmuludq, kInstPmuludq, XmmVar, Mem)

  //! @brief Or (SSE2).
  INST_2x(por, kInstPor, XmmVar, XmmVar)
  //! @overload
  INST_2x(por, kInstPor, XmmVar, Mem)

  //! @brief Packed shift left logical (SSE2).
  INST_2x(pslld, kInstPslld, XmmVar, XmmVar)
  //! @overload
  INST_2x(pslld, kInstPslld, XmmVar, Mem)
  //! @overload
  INST_2i(pslld, kInstPslld, XmmVar, Imm)

  //! @brief Packed shift left logical (SSE2).
  INST_2x(psllq, kInstPsllq, XmmVar, XmmVar)
  //! @overload
  INST_2x(psllq, kInstPsllq, XmmVar, Mem)
  //! @overload
  INST_2i(psllq, kInstPsllq, XmmVar, Imm)

  //! @brief Packed shift left logical (SSE2).
  INST_2x(psllw, kInstPsllw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psllw, kInstPsllw, XmmVar, Mem)
  //! @overload
  INST_2i(psllw, kInstPsllw, XmmVar, Imm)

  //! @brief Packed shift left logical (SSE2).
  INST_2i(pslldq, kInstPslldq, XmmVar, Imm)

  //! @brief Packed shift right arithmetic (SSE2).
  INST_2x(psrad, kInstPsrad, XmmVar, XmmVar)
  //! @overload
  INST_2x(psrad, kInstPsrad, XmmVar, Mem)
  //! @overload
  INST_2i(psrad, kInstPsrad, XmmVar, Imm)

  //! @brief Packed shift right arithmetic (SSE2).
  INST_2x(psraw, kInstPsraw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psraw, kInstPsraw, XmmVar, Mem)
  //! @overload
  INST_2i(psraw, kInstPsraw, XmmVar, Imm)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubb, kInstPsubb, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubb, kInstPsubb, XmmVar, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubw, kInstPsubw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubw, kInstPsubw, XmmVar, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubd, kInstPsubd, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubd, kInstPsubd, XmmVar, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubq, kInstPsubq, MmVar, MmVar)
  //! @overload
  INST_2x(psubq, kInstPsubq, MmVar, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubq, kInstPsubq, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubq, kInstPsubq, XmmVar, Mem)

  //! @brief Packed multiply and Add (SSE2).
  INST_2x(pmaddwd, kInstPmaddwd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaddwd, kInstPmaddwd, XmmVar, Mem)

  //! @brief Shuffle packed dwords (SSE2).
  INST_3i(pshufd, kInstPshufd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pshufd, kInstPshufd, XmmVar, Mem, Imm)

  //! @brief Shuffle packed high words (SSE2).
  INST_3i(pshufhw, kInstPshufhw, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pshufhw, kInstPshufhw, XmmVar, Mem, Imm)

  //! @brief Shuffle packed low words (SSE2).
  INST_3i(pshuflw, kInstPshuflw, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pshuflw, kInstPshuflw, XmmVar, Mem, Imm)

  //! @brief Packed shift right logical (SSE2).
  INST_2x(psrld, kInstPsrld, XmmVar, XmmVar)
  //! @overload
  INST_2x(psrld, kInstPsrld, XmmVar, Mem)
  //! @overload
  INST_2i(psrld, kInstPsrld, XmmVar, Imm)

  //! @brief Packed shift right logical (SSE2).
  INST_2x(psrlq, kInstPsrlq, XmmVar, XmmVar)
  //! @overload
  INST_2x(psrlq, kInstPsrlq, XmmVar, Mem)
  //! @overload
  INST_2i(psrlq, kInstPsrlq, XmmVar, Imm)

  //! @brief Oword shift right logical (SSE2).
  INST_2i(psrldq, kInstPsrldq, XmmVar, Imm)

  //! @brief Packed shift right logical (SSE2).
  INST_2x(psrlw, kInstPsrlw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psrlw, kInstPsrlw, XmmVar, Mem)
  //! @overload
  INST_2i(psrlw, kInstPsrlw, XmmVar, Imm)

  //! @brief Packed subtract with saturation (SSE2).
  INST_2x(psubsb, kInstPsubsb, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubsb, kInstPsubsb, XmmVar, Mem)

  //! @brief Packed subtract with saturation (SSE2).
  INST_2x(psubsw, kInstPsubsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubsw, kInstPsubsw, XmmVar, Mem)

  //! @brief Packed subtract with unsigned saturation (SSE2).
  INST_2x(psubusb, kInstPsubusb, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubusb, kInstPsubusb, XmmVar, Mem)

  //! @brief Packed subtract with unsigned saturation (SSE2).
  INST_2x(psubusw, kInstPsubusw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psubusw, kInstPsubusw, XmmVar, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhbw, kInstPunpckhbw, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpckhbw, kInstPunpckhbw, XmmVar, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhwd, kInstPunpckhwd, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpckhwd, kInstPunpckhwd, XmmVar, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhdq, kInstPunpckhdq, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpckhdq, kInstPunpckhdq, XmmVar, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhqdq, kInstPunpckhqdq, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpckhqdq, kInstPunpckhqdq, XmmVar, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpcklbw, kInstPunpcklbw, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpcklbw, kInstPunpcklbw, XmmVar, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpcklwd, kInstPunpcklwd, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpcklwd, kInstPunpcklwd, XmmVar, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpckldq, kInstPunpckldq, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpckldq, kInstPunpckldq, XmmVar, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpcklqdq, kInstPunpcklqdq, XmmVar, XmmVar)
  //! @overload
  INST_2x(punpcklqdq, kInstPunpcklqdq, XmmVar, Mem)

  //! @brief Xor (SSE2).
  INST_2x(pxor, kInstPxor, XmmVar, XmmVar)
  //! @overload
  INST_2x(pxor, kInstPxor, XmmVar, Mem)

  //! @brief Shuffle DP-FP (SSE2).
  INST_3i(shufpd, kInstShufpd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(shufpd, kInstShufpd, XmmVar, Mem, Imm)

  //! @brief Compute square roots of packed DP-FP Values (SSE2).
  INST_2x(sqrtpd, kInstSqrtpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(sqrtpd, kInstSqrtpd, XmmVar, Mem)

  //! @brief Compute square root of scalar DP-FP value (SSE2).
  INST_2x(sqrtsd, kInstSqrtsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(sqrtsd, kInstSqrtsd, XmmVar, Mem)

  //! @brief Packed DP-FP subtract (SSE2).
  INST_2x(subpd, kInstSubpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(subpd, kInstSubpd, XmmVar, Mem)

  //! @brief Scalar DP-FP subtract (SSE2).
  INST_2x(subsd, kInstSubsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(subsd, kInstSubsd, XmmVar, Mem)

  //! @brief Scalar unordered DP-FP compare and set EFLAGS (SSE2).
  INST_2x(ucomisd, kInstUcomisd, XmmVar, XmmVar)
  //! @overload
  INST_2x(ucomisd, kInstUcomisd, XmmVar, Mem)

  //! @brief Unpack and interleave high packed DP-FP values (SSE2).
  INST_2x(unpckhpd, kInstUnpckhpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(unpckhpd, kInstUnpckhpd, XmmVar, Mem)

  //! @brief Unpack and interleave low packed DP-FP values (SSE2).
  INST_2x(unpcklpd, kInstUnpcklpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(unpcklpd, kInstUnpcklpd, XmmVar, Mem)

  //! @brief Or for DP-FP data (SSE2).
  INST_2x(xorpd, kInstXorpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(xorpd, kInstXorpd, XmmVar, Mem)

  // --------------------------------------------------------------------------
  // [SSE3]
  // --------------------------------------------------------------------------

  //! @brief Packed DP-FP add/subtract (SSE3).
  INST_2x(addsubpd, kInstAddsubpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(addsubpd, kInstAddsubpd, XmmVar, Mem)

  //! @brief Packed SP-FP add/subtract (SSE3).
  INST_2x(addsubps, kInstAddsubps, XmmVar, XmmVar)
  //! @overload
  INST_2x(addsubps, kInstAddsubps, XmmVar, Mem)

  // //! @brief Store integer with truncation (SSE3).
  // INST_1x(fisttp, kInstFisttp, Mem)

  //! @brief Packed DP-FP horizontal add (SSE3).
  INST_2x(haddpd, kInstHaddpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(haddpd, kInstHaddpd, XmmVar, Mem)

  //! @brief Packed SP-FP horizontal add (SSE3).
  INST_2x(haddps, kInstHaddps, XmmVar, XmmVar)
  //! @overload
  INST_2x(haddps, kInstHaddps, XmmVar, Mem)

  //! @brief Packed DP-FP horizontal subtract (SSE3).
  INST_2x(hsubpd, kInstHsubpd, XmmVar, XmmVar)
  //! @overload
  INST_2x(hsubpd, kInstHsubpd, XmmVar, Mem)

  //! @brief Packed SP-FP horizontal subtract (SSE3).
  INST_2x(hsubps, kInstHsubps, XmmVar, XmmVar)
  //! @overload
  INST_2x(hsubps, kInstHsubps, XmmVar, Mem)

  //! @brief Load unaligned Integer 128 Bits (SSE3).
  INST_2x(lddqu, kInstLddqu, XmmVar, Mem)

  // //! @brief Setup monitor address (SSE3).
  // INST_0x(monitor, kInstMonitor)

  //! @brief Move one DP-FP and duplicate (SSE3).
  INST_2x(movddup, kInstMovddup, XmmVar, XmmVar)
  //! @overload
  INST_2x(movddup, kInstMovddup, XmmVar, Mem)

  //! @brief Move packed SP-FP high and duplicate (SSE3).
  INST_2x(movshdup, kInstMovshdup, XmmVar, XmmVar)
  //! @overload
  INST_2x(movshdup, kInstMovshdup, XmmVar, Mem)

  //! @brief Move packed SP-FP low and duplicate (SSE3).
  INST_2x(movsldup, kInstMovsldup, XmmVar, XmmVar)
  //! @overload
  INST_2x(movsldup, kInstMovsldup, XmmVar, Mem)

  // //! @brief Monitor wait (SSE3).
  // INST_0x(mwait, kInstMwait)

  // --------------------------------------------------------------------------
  // [SSSE3]
  // --------------------------------------------------------------------------

  //! @brief Packed sign (SSSE3).
  INST_2x(psignb, kInstPsignb, MmVar, MmVar)
  //! @overload
  INST_2x(psignb, kInstPsignb, MmVar, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignb, kInstPsignb, XmmVar, XmmVar)
  //! @overload
  INST_2x(psignb, kInstPsignb, XmmVar, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignw, kInstPsignw, MmVar, MmVar)
  //! @overload
  INST_2x(psignw, kInstPsignw, MmVar, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignw, kInstPsignw, XmmVar, XmmVar)
  //! @overload
  INST_2x(psignw, kInstPsignw, XmmVar, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignd, kInstPsignd, MmVar, MmVar)
  //! @overload
  INST_2x(psignd, kInstPsignd, MmVar, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignd, kInstPsignd, XmmVar, XmmVar)
  //! @overload
  INST_2x(psignd, kInstPsignd, XmmVar, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddw, kInstPhaddw, MmVar, MmVar)
  //! @overload
  INST_2x(phaddw, kInstPhaddw, MmVar, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddw, kInstPhaddw, XmmVar, XmmVar)
  //! @overload
  INST_2x(phaddw, kInstPhaddw, XmmVar, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddd, kInstPhaddd, MmVar, MmVar)
  //! @overload
  INST_2x(phaddd, kInstPhaddd, MmVar, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddd, kInstPhaddd, XmmVar, XmmVar)
  //! @overload
  INST_2x(phaddd, kInstPhaddd, XmmVar, Mem)

  //! @brief Packed horizontal add and saturate (SSSE3).
  INST_2x(phaddsw, kInstPhaddsw, MmVar, MmVar)
  //! @overload
  INST_2x(phaddsw, kInstPhaddsw, MmVar, Mem)

  //! @brief Packed horizontal add and saturate (SSSE3).
  INST_2x(phaddsw, kInstPhaddsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(phaddsw, kInstPhaddsw, XmmVar, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubw, kInstPhsubw, MmVar, MmVar)
  //! @overload
  INST_2x(phsubw, kInstPhsubw, MmVar, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubw, kInstPhsubw, XmmVar, XmmVar)
  //! @overload
  INST_2x(phsubw, kInstPhsubw, XmmVar, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubd, kInstPhsubd, MmVar, MmVar)
  //! @overload
  INST_2x(phsubd, kInstPhsubd, MmVar, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubd, kInstPhsubd, XmmVar, XmmVar)
  //! @overload
  INST_2x(phsubd, kInstPhsubd, XmmVar, Mem)

  //! @brief Packed horizontal subtract and saturate (SSSE3).
  INST_2x(phsubsw, kInstPhsubsw, MmVar, MmVar)
  //! @overload
  INST_2x(phsubsw, kInstPhsubsw, MmVar, Mem)

  //! @brief Packed horizontal subtract and saturate (SSSE3).
  INST_2x(phsubsw, kInstPhsubsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(phsubsw, kInstPhsubsw, XmmVar, Mem)

  //! @brief Multiply and add packed signed and unsigned bytes (SSSE3).
  INST_2x(pmaddubsw, kInstPmaddubsw, MmVar, MmVar)
  //! @overload
  INST_2x(pmaddubsw, kInstPmaddubsw, MmVar, Mem)

  //! @brief Multiply and add packed signed and unsigned bytes (SSSE3).
  INST_2x(pmaddubsw, kInstPmaddubsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaddubsw, kInstPmaddubsw, XmmVar, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsb, kInstPabsb, MmVar, MmVar)
  //! @overload
  INST_2x(pabsb, kInstPabsb, MmVar, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsb, kInstPabsb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pabsb, kInstPabsb, XmmVar, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsw, kInstPabsw, MmVar, MmVar)
  //! @overload
  INST_2x(pabsw, kInstPabsw, MmVar, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsw, kInstPabsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pabsw, kInstPabsw, XmmVar, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsd, kInstPabsd, MmVar, MmVar)
  //! @overload
  INST_2x(pabsd, kInstPabsd, MmVar, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsd, kInstPabsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pabsd, kInstPabsd, XmmVar, Mem)

  //! @brief Packed multiply high with round and scale (SSSE3).
  INST_2x(pmulhrsw, kInstPmulhrsw, MmVar, MmVar)
  //! @overload
  INST_2x(pmulhrsw, kInstPmulhrsw, MmVar, Mem)

  //! @brief Packed multiply high with round and scale (SSSE3).
  INST_2x(pmulhrsw, kInstPmulhrsw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmulhrsw, kInstPmulhrsw, XmmVar, Mem)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_2x(pshufb, kInstPshufb, MmVar, MmVar)
  //! @overload
  INST_2x(pshufb, kInstPshufb, MmVar, Mem)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_2x(pshufb, kInstPshufb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pshufb, kInstPshufb, XmmVar, Mem)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_3i(palignr, kInstPalignr, MmVar, MmVar, Imm)
  //! @overload
  INST_3i(palignr, kInstPalignr, MmVar, Mem, Imm)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_3i(palignr, kInstPalignr, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(palignr, kInstPalignr, XmmVar, Mem, Imm)

  // --------------------------------------------------------------------------
  // [SSE4.1]
  // --------------------------------------------------------------------------

  //! @brief Blend packed DP-FP values (SSE4.1).
  INST_3i(blendpd, kInstBlendpd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(blendpd, kInstBlendpd, XmmVar, Mem, Imm)

  //! @brief Blend packed SP-FP values (SSE4.1).
  INST_3i(blendps, kInstBlendps, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(blendps, kInstBlendps, XmmVar, Mem, Imm)

  //! @brief Variable blend packed DP-FP values (SSE4.1).
  INST_3x(blendvpd, kInstBlendvpd, XmmVar, XmmVar, XmmVar)
  //! @overload
  INST_3x(blendvpd, kInstBlendvpd, XmmVar, Mem, XmmVar)

  //! @brief Variable blend packed SP-FP values (SSE4.1).
  INST_3x(blendvps, kInstBlendvps, XmmVar, XmmVar, XmmVar)
  //! @overload
  INST_3x(blendvps, kInstBlendvps, XmmVar, Mem, XmmVar)

  //! @brief Dot product of packed DP-FP values (SSE4.1).
  INST_3i(dppd, kInstDppd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(dppd, kInstDppd, XmmVar, Mem, Imm)

  //! @brief Dot product of packed SP-FP values (SSE4.1).
  INST_3i(dpps, kInstDpps, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(dpps, kInstDpps, XmmVar, Mem, Imm)

  //! @brief Extract packed SP-FP value (SSE4.1).
  INST_3i(extractps, kInstExtractps, GpVar, XmmVar, Imm)
  //! @overload
  INST_3i(extractps, kInstExtractps, Mem, XmmVar, Imm)

  //! @brief Load oword non-temporal aligned hint (SSE4.1).
  INST_2x(movntdqa, kInstMovntdqa, XmmVar, Mem)

  //! @brief Compute multiple packed sums of absolute difference (SSE4.1).
  INST_3i(mpsadbw, kInstMpsadbw, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(mpsadbw, kInstMpsadbw, XmmVar, Mem, Imm)

  //! @brief Pack with unsigned saturation (SSE4.1).
  INST_2x(packusdw, kInstPackusdw, XmmVar, XmmVar)
  //! @overload
  INST_2x(packusdw, kInstPackusdw, XmmVar, Mem)

  //! @brief Variable blend packed bytes (SSE4.1).
  INST_3x(pblendvb, kInstPblendvb, XmmVar, XmmVar, XmmVar)
  //! @overload
  INST_3x(pblendvb, kInstPblendvb, XmmVar, Mem, XmmVar)

  //! @brief Blend packed words (SSE4.1).
  INST_3i(pblendw, kInstPblendw, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pblendw, kInstPblendw, XmmVar, Mem, Imm)

  //! @brief Compare packed qword Data for Equal (SSE4.1).
  INST_2x(pcmpeqq, kInstPcmpeqq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpeqq, kInstPcmpeqq, XmmVar, Mem)

  //! @brief Extract byte (SSE4.1).
  INST_3i(pextrb, kInstPextrb, GpVar, XmmVar, Imm)
  //! @overload
  INST_3i(pextrb, kInstPextrb, Mem, XmmVar, Imm)

  //! @brief Extract dword (SSE4.1).
  INST_3i(pextrd, kInstPextrd, GpVar, XmmVar, Imm)
  //! @overload
  INST_3i(pextrd, kInstPextrd, Mem, XmmVar, Imm)

  //! @brief Extract qword (SSE4.1).
  INST_3i(pextrq, kInstPextrq, GpVar, XmmVar, Imm)
  //! @overload
  INST_3i(pextrq, kInstPextrq, Mem, XmmVar, Imm)

  //! @brief Extract word (SSE4.1).
  INST_3i(pextrw, kInstPextrw, Mem, XmmVar, Imm)

  //! @brief Packed horizontal word minimum (SSE4.1).
  INST_2x(phminposuw, kInstPhminposuw, XmmVar, XmmVar)
  //! @overload
  INST_2x(phminposuw, kInstPhminposuw, XmmVar, Mem)

  //! @brief Insert byte (SSE4.1).
  INST_3i(pinsrb, kInstPinsrb, XmmVar, GpVar, Imm)
  //! @overload
  INST_3i(pinsrb, kInstPinsrb, XmmVar, Mem, Imm)

  //! @brief Insert dword (SSE4.1).
  INST_3i(pinsrd, kInstPinsrd, XmmVar, GpVar, Imm)
  //! @overload
  INST_3i(pinsrd, kInstPinsrd, XmmVar, Mem, Imm)

  //! @brief Insert qword (SSE4.1).
  INST_3i(pinsrq, kInstPinsrq, XmmVar, GpVar, Imm)
  //! @overload
  INST_3i(pinsrq, kInstPinsrq, XmmVar, Mem, Imm)

  //! @brief Maximum of packed word integers (SSE4.1).
  INST_2x(pmaxuw, kInstPmaxuw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaxuw, kInstPmaxuw, XmmVar, Mem)

  //! @brief Maximum of packed signed byte integers (SSE4.1).
  INST_2x(pmaxsb, kInstPmaxsb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaxsb, kInstPmaxsb, XmmVar, Mem)

  //! @brief Maximum of packed signed dword integers (SSE4.1).
  INST_2x(pmaxsd, kInstPmaxsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaxsd, kInstPmaxsd, XmmVar, Mem)

  //! @brief Maximum of packed unsigned dword integers (SSE4.1).
  INST_2x(pmaxud, kInstPmaxud, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmaxud,kInstPmaxud , XmmVar, Mem)

  //! @brief Minimum of packed signed byte integers (SSE4.1).
  INST_2x(pminsb, kInstPminsb, XmmVar, XmmVar)
  //! @overload
  INST_2x(pminsb, kInstPminsb, XmmVar, Mem)

  //! @brief Minimum of packed word integers (SSE4.1).
  INST_2x(pminuw, kInstPminuw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pminuw, kInstPminuw, XmmVar, Mem)

  //! @brief Minimum of packed dword integers (SSE4.1).
  INST_2x(pminud, kInstPminud, XmmVar, XmmVar)
  //! @overload
  INST_2x(pminud, kInstPminud, XmmVar, Mem)

  //! @brief Minimum of packed dword integers (SSE4.1).
  INST_2x(pminsd, kInstPminsd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pminsd, kInstPminsd, XmmVar, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxbw, kInstPmovsxbw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovsxbw, kInstPmovsxbw, XmmVar, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxbd, kInstPmovsxbd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovsxbd, kInstPmovsxbd, XmmVar, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxbq, kInstPmovsxbq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovsxbq, kInstPmovsxbq, XmmVar, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxwd, kInstPmovsxwd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovsxwd, kInstPmovsxwd, XmmVar, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovsxwq, kInstPmovsxwq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovsxwq, kInstPmovsxwq, XmmVar, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovsxdq, kInstPmovsxdq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovsxdq, kInstPmovsxdq, XmmVar, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxbw, kInstPmovzxbw, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovzxbw, kInstPmovzxbw, XmmVar, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxbd, kInstPmovzxbd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovzxbd, kInstPmovzxbd, XmmVar, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxbq, kInstPmovzxbq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovzxbq, kInstPmovzxbq, XmmVar, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxwd, kInstPmovzxwd, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovzxwd, kInstPmovzxwd, XmmVar, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovzxwq, kInstPmovzxwq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovzxwq, kInstPmovzxwq, XmmVar, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovzxdq, kInstPmovzxdq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmovzxdq, kInstPmovzxdq, XmmVar, Mem)

  //! @brief Multiply packed signed dword integers (SSE4.1).
  INST_2x(pmuldq, kInstPmuldq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmuldq, kInstPmuldq, XmmVar, Mem)

  //! @brief Multiply packed signed integers and store low result (SSE4.1).
  INST_2x(pmulld, kInstPmulld, XmmVar, XmmVar)
  //! @overload
  INST_2x(pmulld, kInstPmulld, XmmVar, Mem)

  //! @brief Logical compare (SSE4.1).
  INST_2x(ptest, kInstPtest, XmmVar, XmmVar)
  //! @overload
  INST_2x(ptest, kInstPtest, XmmVar, Mem)

  //! @brief Round packed SP-FP values (SSE4.1).
  INST_3i(roundps, kInstRoundps, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(roundps, kInstRoundps, XmmVar, Mem, Imm)

  //! @brief Round scalar SP-FP values (SSE4.1).
  INST_3i(roundss, kInstRoundss, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(roundss, kInstRoundss, XmmVar, Mem, Imm)

  //! @brief Round packed DP-FP values (SSE4.1).
  INST_3i(roundpd, kInstRoundpd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(roundpd, kInstRoundpd, XmmVar, Mem, Imm)

  //! @brief Round scalar DP-FP values (SSE4.1).
  INST_3i(roundsd, kInstRoundsd, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(roundsd, kInstRoundsd, XmmVar, Mem, Imm)

  // --------------------------------------------------------------------------
  // [SSE4.2]
  // --------------------------------------------------------------------------

  //! @brief Packed compare explicit length strings, return index (SSE4.2).
  INST_3i(pcmpestri, kInstPcmpestri, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pcmpestri, kInstPcmpestri, XmmVar, Mem, Imm)

  //! @brief Packed compare explicit length strings, return mask (SSE4.2).
  INST_3i(pcmpestrm, kInstPcmpestrm, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pcmpestrm, kInstPcmpestrm, XmmVar, Mem, Imm)

  //! @brief Packed compare implicit length strings, return index (SSE4.2).
  INST_3i(pcmpistri, kInstPcmpistri, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pcmpistri, kInstPcmpistri, XmmVar, Mem, Imm)

  //! @brief Packed compare implicit length strings, return mask (SSE4.2).
  INST_3i(pcmpistrm, kInstPcmpistrm, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(pcmpistrm, kInstPcmpistrm, XmmVar, Mem, Imm)

  //! @brief Compare packed data for greater than (SSE4.2).
  INST_2x(pcmpgtq, kInstPcmpgtq, XmmVar, XmmVar)
  //! @overload
  INST_2x(pcmpgtq, kInstPcmpgtq, XmmVar, Mem)

  // --------------------------------------------------------------------------
  // [AESNI]
  // --------------------------------------------------------------------------

  //! @brief Perform a single round of the AES decryption flow.
  INST_2x(aesdec, kInstAesdec, XmmVar, XmmVar)
  //! @overload
  INST_2x(aesdec, kInstAesdec, XmmVar, Mem)

  //! @brief Perform the last round of the AES decryption flow.
  INST_2x(aesdeclast, kInstAesdeclast, XmmVar, XmmVar)
  //! @overload
  INST_2x(aesdeclast, kInstAesdeclast, XmmVar, Mem)

  //! @brief Perform a single round of the AES encryption flow.
  INST_2x(aesenc, kInstAesenc, XmmVar, XmmVar)
  //! @overload
  INST_2x(aesenc, kInstAesenc, XmmVar, Mem)

  //! @brief Perform the last round of the AES encryption flow.
  INST_2x(aesenclast, kInstAesenclast, XmmVar, XmmVar)
  //! @overload
  INST_2x(aesenclast, kInstAesenclast, XmmVar, Mem)

  //! @brief Perform the InvMixColumns transformation.
  INST_2x(aesimc, kInstAesimc, XmmVar, XmmVar)
  //! @overload
  INST_2x(aesimc, kInstAesimc, XmmVar, Mem)

  //! @brief Assist in expanding the AES cipher key.
  INST_3i(aeskeygenassist, kInstAeskeygenassist, XmmVar, XmmVar, Imm)
  //! @overload
  INST_3i(aeskeygenassist, kInstAeskeygenassist, XmmVar, Mem, Imm)

  // --------------------------------------------------------------------------
  // [PCLMULQDQ]
  // --------------------------------------------------------------------------

  //! @brief Carry-less multiplication quadword.
  INST_3i(pclmulqdq, kInstPclmulqdq, XmmVar, XmmVar, Imm);
  //! @overload
  INST_3i(pclmulqdq, kInstPclmulqdq, XmmVar, Mem, Imm);
};

//! @}

} // x86x64 namespace
} // asmjit namespace

// ============================================================================
// [asmjit::x86]
// ============================================================================

#if defined(ASMJIT_BUILD_X86)

namespace asmjit {
namespace x86 {

//! @addtogroup asmjit_x86x64
//! @{

struct Compiler : public X86X64Compiler {
  ASMJIT_NO_COPY(Compiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a @ref Compiler instance.
  ASMJIT_API Compiler(BaseRuntime* runtime);
  //! @brief Destroy the @ref Compiler instance.
  ASMJIT_API ~Compiler();

  // -------------------------------------------------------------------------
  // [Options]
  // -------------------------------------------------------------------------

  //! @overload
  ASMJIT_INLINE Compiler& short_()
  { _options |= kInstOptionShortForm; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& long_()
  { _options |= kInstOptionLongForm; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& taken()
  { _options |= kInstOptionTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& notTaken()
  { _options |= kInstOptionNotTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& lock()
  { _options |= kInstOptionLock; return *this; }

  // --------------------------------------------------------------------------
  // [X86-Only Instructions]
  // --------------------------------------------------------------------------

  //! @brief Decimal adjust AL after addition (32-bit).
  INST_1x(daa, kInstDaa, GpVar)
  //! @brief Decimal adjust AL after subtraction (32-bit).
  INST_1x(das, kInstDas, GpVar)

  //! @brief Pop all Gp registers (EDI|ESI|EBP|EBX|EDX|ECX|EAX).
  INST_0x(popa, kInstPopa)

  //! @brief Push all Gp registers (EAX|ECX|EDX|EBX|original ESP|EBP|ESI|EDI).
  INST_0x(pusha, kInstPusha)
};

//! @}

} // x86 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X86

// ============================================================================
// [asmjit::x64]
// ============================================================================

#if defined(ASMJIT_BUILD_X64)

namespace asmjit {
namespace x64 {

//! @addtogroup asmjit_x86x64
//! @{

struct Compiler : public X86X64Compiler {
  ASMJIT_NO_COPY(Compiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a @ref Compiler instance.
  ASMJIT_API Compiler(BaseRuntime* runtime);
  //! @brief Destroy the @ref Compiler instance.
  ASMJIT_API ~Compiler();

  // -------------------------------------------------------------------------
  // [Options]
  // -------------------------------------------------------------------------

  //! @overload
  ASMJIT_INLINE Compiler& short_()
  { _options |= kInstOptionShortForm; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& long_()
  { _options |= kInstOptionLongForm; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& taken()
  { _options |= kInstOptionTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& notTaken()
  { _options |= kInstOptionNotTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Compiler& lock()
  { _options |= kInstOptionLock; return *this; }

  //! @brief Force rex prefix.
  ASMJIT_INLINE Compiler& rex()
  { _options |= kInstOptionRex; return *this; }

  // --------------------------------------------------------------------------
  // [X64-Only Instructions]
  // --------------------------------------------------------------------------

  //! @brief Convert dword to qword (RAX <- Sign Extend EAX).
  INST_1x(cdqe, kInstCdqe, GpVar /* eax */)
  //! @brief Convert qword to oword (RDX:RAX <- Sign Extend RAX).
  INST_2x(cqo, kInstCdq, GpVar /* rdx */, GpVar /* rax */)

  //! @brief Compares the 128-bit value in RDX:RAX with the memory operand (X64).
  ASMJIT_INLINE InstNode* cmpxchg16b(
    const GpVar& cmp_edx, const GpVar& cmp_eax,
    const GpVar& cmp_ecx, const GpVar& cmp_ebx,
    const Mem& dst)
  { return emit(kInstCmpxchg16b, cmp_edx, cmp_eax, cmp_ecx, cmp_ebx, dst); }

  //! @brief Move dword to qword with sign-extension.
  INST_2x(movsxd, kInstMovsxd, GpVar, GpVar)
  //! @overload
  INST_2x(movsxd, kInstMovsxd, GpVar, Mem)

  //! @brief Load ECX/RCX Qdwords from DS:[ESI/RSI] to RAX.
  INST_3x_(rep_lodsq, kInstRepLodsq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Move ECX/RCX Qdwords from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x_(rep_movsq, kInstRepMovsq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Fill ECX/RCX Qdwords at ES:[EDI/RDI] with RAX.
  INST_3x_(rep_stosq, kInstRepStosq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Repeated find nonmatching Qdwords in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x_(repe_cmpsq, kInstRepeCmpsq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Find non-RAX QWORD starting at ES:[EDI/RDI].
  INST_3x_(repe_scasq, kInstRepeScasq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Find matching Qdwords in [RDI] and [RSI].
  INST_3x_(repne_cmpsq, kInstRepneCmpsq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  //! @brief Find RAX, starting at ES:[EDI/RDI].
  INST_3x_(repne_scasq, kInstRepneScasq, GpVar, GpVar, GpVar, o0.getId() != o1.getId() && o1.getId() != o2.getId())

  using X86X64Compiler::movq;

  //! @overload
  INST_2x(movq, kInstMovq, GpVar, MmVar)
  //! @overload
  INST_2x(movq, kInstMovq, MmVar, GpVar)

  //! @overload
  INST_2x(movq, kInstMovq, GpVar, XmmVar)
  //! @overload
  INST_2x(movq, kInstMovq, XmmVar, GpVar)
};

//! @}

} // x64 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X64

// ============================================================================
// [CodeGen-End]
// ============================================================================

#undef INST_0x

#undef INST_1x
#undef INST_1x_
#undef INST_1i
#undef INST_1i_
#undef INST_1cc

#undef INST_2x
#undef INST_2x_
#undef INST_2i
#undef INST_2i_
#undef INST_2cc

#undef INST_3x
#undef INST_3x_
#undef INST_3i
#undef INST_3i_

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86COMPILER_H
