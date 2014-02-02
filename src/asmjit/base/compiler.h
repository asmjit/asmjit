// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_COMPILER_H
#define _ASMJIT_BASE_COMPILER_H

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/codegen.h"
#include "../base/defs.h"
#include "../base/error.h"
#include "../base/func.h"
#include "../base/intutil.h"
#include "../base/podlist.h"
#include "../base/podvector.h"
#include "../base/runtime.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BaseCompiler;

struct VarAttr;
struct VarData;
struct BaseVarInst;
struct BaseVarState;

struct BaseNode;
struct EndNode;
struct InstNode;
struct JumpNode;

// ============================================================================
// [asmjit::kVarAttrFlags]
// ============================================================================

//! @brief Variable attribute flags.
ASMJIT_ENUM(kVarAttrFlags) {
  //! @brief Variable is accessed through register on input.
  kVarAttrInReg = 0x00000001,
  //! @brief Variable is accessed through register on output.
  kVarAttrOutReg = 0x00000002,
  //! @brief Variable is accessed through register on input & output.
  kVarAttrInOutReg = 0x00000003,

  //! @brief Variable is accessed through memory on input.
  kVarAttrInMem = 0x00000004,
  //! @brief Variable is accessed through memory on output.
  kVarAttrOutMem = 0x00000008,
  //! @brief Variable is accessed through memory on input & output.
  kVarAttrInOutMem = 0x0000000C,

  //! @brief It can be decided whether it's better to alloc variable to register
  //! or memory on the input.
  kVarAttrInDecide = 0x00000010,
  //! @brief It can be decided whether it's better to alloc variable to register
  //! or memory on the output.
  kVarAttrOutDecide = 0x00000020,
  //! @brief It can be decided whether it's better to alloc variable to register
  //! or memory on the input & output.
  kVarAttrInOutDecide = 0x00000030,

  //! @brief Variable is converted to other type/class on the input.
  kVarAttrInConv = 0x00000040,
  //! @brief Variable is converted from other type/class on the output.
  kVarAttrOutConv = 0x00000080,
  //! @brief Combination of @ref kVarAttrInConv and @ref kVarAttrOutConv.
  kVarAttrInOutConv = 0x000000C0,

  //! @brief Variable is a function call operand.
  kVarAttrInCall = 0x00000100,
  //! @brief Variable is a function argument passed in register.
  kVarAttrInArg = 0x00000200,
  //! @brief Variable is a function argument passed on the stack.
  kVarAttrInStack = 0x00000400,
  //! @brief Variable is a function return value passed in register.
  kVarAttrOutRet = 0x00000800,

  //! @brief Variable should be unused at the end of the instruction/node.
  kVarAttrUnuse = 0x00001000,

  kVarAttrInAll = kVarAttrInReg | kVarAttrInMem | kVarAttrInDecide | kVarAttrInCall | kVarAttrInArg | kVarAttrInStack,
  kVarAttrOutAll = kVarAttrOutReg | kVarAttrOutMem | kVarAttrOutDecide | kVarAttrOutRet,

  //! @brief Variable is already allocated on the input.
  kVarAttrAllocInDone = 0x00400000,
  //! @brief Variable is already allocated on the output.
  kVarAttrAllocOutDone = 0x00800000
};

// ============================================================================
// [asmjit::kVarHint]
// ============================================================================

//! @brief Variable hint (used by @ref BaseCompiler).
//!
//! @sa @ref BaseCompiler.
ASMJIT_ENUM(kVarHint) {
  //! @brief Alloc variable.
  kVarHintAlloc = 0,
  //! @brief Spill variable.
  kVarHintSpill = 1,
  //! @brief Save variable if modified.
  kVarHintSave = 2,
  //! @brief Save variable if modified and mark it as unused.
  kVarHintSaveAndUnuse = 3,
  //! @brief Mark variable as unused.
  kVarHintUnuse = 4
};

// ============================================================================
// [asmjit::kVarState]
// ============================================================================

//! @brief State of variable.
//!
//! @note State of variable is used only during make process and it's not
//! visible to the developer.
ASMJIT_ENUM(kVarState) {
  //! @brief Variable is currently not used.
  kVarStateUnused = 0,

  //! @brief Variable is in register.
  //!
  //! Variable is currently allocated in register.
  kVarStateReg = 1,

  //! @brief Variable is in memory location or spilled.
  //!
  //! Variable was spilled from register to memory or variable is used for
  //! memory only storage.
  kVarStateMem = 2
};

// ============================================================================
// [asmjit::kNodeType]
// ============================================================================

//! @brief Type of node (see @ref BaseNode).
ASMJIT_ENUM(kNodeType) {
  //! @brief Invalid node (internal, can't be used).
  kNodeTypeNone = 0,
  //! @brief Node is an .align directive, see @ref AlignNode.
  kNodeTypeAlign,
  //! @brief Node is an embedded data, see @ref EmbedNode.
  kNodeTypeEmbed,
  //! @brief Node is a comment, see @ref CommentNode.
  kNodeTypeComment,
  //! @brief Node is a variable hint (alloc, spill, use, unuse), see @ref HintNode.
  kNodeTypeHint,
  //! @brief Node is a label, see @ref TargetNode.
  kNodeTypeTarget,
  //! @brief Node is an instruction, see @ref InstNode.
  kNodeTypeInst,
  //! @brief Node is a function declaration, see @ref FuncNode.
  kNodeTypeFunc,
  //! @brief Node is an end of the function, see @ref EndNode.
  kNodeTypeEnd,
  //! @brief Node is a return, see @ref RetNode.
  kNodeTypeRet,
  //! @brief Node is a function call, see @ref CallNode.
  kNodeTypeCall,
  //! @brief Node is a function call argument moved on stack, see @ref SArgNode.
  kNodeTypeSArg
};

// ============================================================================
// [asmjit::kNodeFlag]
// ============================================================================

ASMJIT_ENUM(kNodeFlag) {
  //! @brief Whether the node was translated by @ref BaseContext.
  kNodeFlagIsTranslated = 0x0001,

  //! @Brief Whether the @ref InstNode is a jump.
  kNodeFlagIsJmp = 0x0002,
  //! @Brief Whether the @ref InstNode is a conditional jump.
  kNodeFlagIsJcc = 0x0004,

  //! @brief Whether the @ref InstNode is an unconditinal jump or conditional
  //! jump that is likely to be taken.
  kNodeFlagIsTaken = 0x0008,

  //! @brief Whether the @ref Node will return from a function.
  //!
  //! This flag is used by both @ref EndNode and @ref RetNode.
  kNodeFlagIsRet = 0x0010,

  //! @brief Whether the instruction is special.
  kNodeFlagIsSpecial = 0x0020,

  //! @brief Whether the instruction is an FPU instruction.
  kNodeFlagIsFp = 0x0040
};

// ============================================================================
// [asmjit::MemCell]
// ============================================================================

struct MemCell {
  ASMJIT_NO_COPY(MemCell)

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get cell offset.
  ASMJIT_INLINE int32_t getOffset() const { return _offset; }
  //! @brief Set cell offset.
  ASMJIT_INLINE void setOffset(int32_t offset) { _offset = offset; }

  //! @brief Get cell size.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }
  //! @brief Set cell size.
  ASMJIT_INLINE void setSize(uint32_t size) { _size = size; }

  //! @brief Get cell alignment.
  ASMJIT_INLINE uint32_t getAlignment() const { return _alignment; }
  //! @brief Set cell alignment.
  ASMJIT_INLINE void setAlignment(uint32_t alignment) { _alignment = alignment; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Next active cell.
  MemCell* _next;

  //! @brief Offset, relative to base-offset.
  int32_t _offset;
  //! @brief Size.
  uint32_t _size;
  //! @brief Alignment.
  uint32_t _alignment;
};

// ============================================================================
// [asmjit::VarBits]
// ============================================================================

//! @brief Bit-array used by variable-liveness analysis.
struct VarBits {
  // --------------------------------------------------------------------------
  // [Enums]
  // --------------------------------------------------------------------------

  enum {
    kEntitySize = static_cast<int>(sizeof(uintptr_t)),
    kEntityBits = kEntitySize * 8
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uintptr_t getBit(uint32_t index) const {
    return (data[index / kEntityBits] >> (index % kEntityBits)) & 1;
  }

  ASMJIT_INLINE void setBit(uint32_t index) {
    data[index / kEntityBits] |= static_cast<uintptr_t>(1) << (index % kEntityBits);
  }

  ASMJIT_INLINE void delBit(uint32_t index) {
    data[index / kEntityBits] &= ~(static_cast<uintptr_t>(1) << (index % kEntityBits));
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool copyBits(const VarBits* s0, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool addBits(const VarBits* s0, uint32_t len) {
    return addBits(this, s0, len);
  }

  ASMJIT_INLINE bool addBits(const VarBits* s0, const VarBits* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] | s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool andBits(const VarBits* s1, uint32_t len) {
    return andBits(this, s1, len);
  }

  ASMJIT_INLINE bool andBits(const VarBits* s0, const VarBits* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] & s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool delBits(const VarBits* s1, uint32_t len) {
    return delBits(this, s1, len);
  }

  ASMJIT_INLINE bool delBits(const VarBits* s0, const VarBits* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] & ~s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool _addBitsDelSource(VarBits* s1, uint32_t len) {
    return _addBitsDelSource(this, s1, len);
  }

  ASMJIT_INLINE bool _addBitsDelSource(const VarBits* s0, VarBits* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t a = s0->data[i];
      uintptr_t b = s1->data[i];

      this->data[i] = a | b;
      b &= ~a;

      s1->data[i] = b;
      r |= b;
    }
    return r != 0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uintptr_t data[1];
};

// ============================================================================
// [asmjit::VarData]
// ============================================================================

//! @brief Base variable data.
struct VarData {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get variable name.
  ASMJIT_INLINE const char* getName() const { return _name; }
  //! @brief Get variable id.
  ASMJIT_INLINE uint32_t getId() const { return _id; }

  //! @brief Get whether the variable has context id.
  ASMJIT_INLINE bool hasContextId() const { return _contextId != kInvalidValue; }
  //! @brief Get context variable id (used only by @ref Context).
  ASMJIT_INLINE uint32_t getContextId() const { return _contextId; }
  //! @brief Set context variable id (used only by @ref Context).
  ASMJIT_INLINE void setContextId(uint32_t contextId) { _contextId = contextId; }
  //! @brief Reset context variable id (used only by @ref Context).
  ASMJIT_INLINE void resetContextId() { _contextId = kInvalidValue; }

  //! @brief Get variable type.
  ASMJIT_INLINE uint32_t getType() const { return _type; }
  //! @brief Get variable class.
  ASMJIT_INLINE uint32_t getClass() const { return _class; }
  //! @brief Get variable flags.
  ASMJIT_INLINE uint32_t getFlags() const { return _flags; }

  //! @brief Get variable priority.
  ASMJIT_INLINE uint32_t getPriority() const { return _priority; }

  //! @brief Get variable state (only used by @ref Context).
  ASMJIT_INLINE uint32_t getState() const { return _state; }
  //! @brief Set variable state (only used by @ref Context).
  ASMJIT_INLINE void setState(uint32_t state) { _state = static_cast<uint8_t>(state); }

  //! @brief Get register index.
  ASMJIT_INLINE uint32_t getRegIndex() const { return _regIndex; }
  //! @brief Set register index.
  ASMJIT_INLINE void setRegIndex(uint32_t regIndex) { _regIndex = static_cast<uint8_t>(regIndex); }
  //! @brief Reset register index.
  ASMJIT_INLINE void resetRegIndex() { _regIndex = static_cast<uint8_t>(kInvalidReg); }

  //! @brief Get whether the VarData is only memory allocated on the stack.
  ASMJIT_INLINE bool isStack() const { return static_cast<bool>(_isStack); }

  //! @brief Get whether the variable is a function argument passed through memory.
  ASMJIT_INLINE bool isMemArg() const { return static_cast<bool>(_isMemArg); }

  //! @brief Get variable content can be calculated by a simple instruction.
  ASMJIT_INLINE bool isCalculated() const { return static_cast<bool>(_isCalculated); }
  //! @brief Get whether to save variable when it's unused (spill).
  ASMJIT_INLINE bool saveOnUnuse() const { return static_cast<bool>(_saveOnUnuse); }

  //! @brief Get whether the variable was changed.
  ASMJIT_INLINE bool isModified() const { return static_cast<bool>(_modified); }
  //! @brief Set whether the variable was changed.
  ASMJIT_INLINE void setModified(bool modified) { _modified = modified; }

  //! @brief Get variable alignment.
  ASMJIT_INLINE uint32_t getAlignment() const { return _alignment; }
  //! @brief Get variable size.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }

  //! @brief Get home memory offset.
  ASMJIT_INLINE int32_t getMemOffset() const { return _memOffset; }
  //! @brief Set home memory offset.
  ASMJIT_INLINE void setMemOffset(int32_t offset) { _memOffset = offset; }

  //! @brief Get home memory cell.
  ASMJIT_INLINE MemCell* getMemCell() const { return _memCell; }
  //! @brief Set home memory cell.
  ASMJIT_INLINE void setMemCell(MemCell* cell) { _memCell = cell; }

  // --------------------------------------------------------------------------
  // [Accessors - Temporary Usage]
  // --------------------------------------------------------------------------

  //! @brief Get temporary VarAttr.
  ASMJIT_INLINE VarAttr* getVa() const { return _va; }
  //! @brief Set temporary VarAttr.
  ASMJIT_INLINE void setVa(VarAttr* va) { _va = va; }
  //! @brief Reset temporary VarAttr.
  ASMJIT_INLINE void resetVa() { _va = NULL; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Variable name.
  const char* _name;

  //! @brief Variable id.
  uint32_t _id;
  //! @brief Context variable id (used by @ref Context only, initially set to
  //! @c kInvalidValue).
  uint32_t _contextId;

  //! @brief Variable type.
  uint8_t _type;
  //! @brief Variable class.
  uint8_t _class;
  //! @brief Variable flags.
  uint8_t _flags;
  //! @brief Variable priority.
  uint8_t _priority;

  //! @brief Variable state (connected with actual @ref BaseVarState).
  uint8_t _state;
  //! @brief Actual register index (only used by @ref Context), during translate.
  uint8_t _regIndex;

  //! @brief Whether the variable is only used as memory allocated on the stack.
  uint8_t _isStack : 1;
  //! @brief Whether the variable is a function argument passed through memory.
  uint8_t _isMemArg : 1;
  //! @brief Whether variable content can be calculated by a simple instruction.
  //!
  //! This is used mainly by MMX and SSE2 code. This flag indicates that
  //! register allocator should never reserve memory for this variable, because
  //! the content can be generated by a single instruction (for example PXOR).
  uint8_t _isCalculated : 1;
  //! @brief Save on unuse (at end of the variable scope).
  uint8_t _saveOnUnuse : 1;
  //! @brief Whether variable was changed (connected with actual @ref BaseVarState).
  uint8_t _modified : 1;
  //! @internal
  uint8_t _reserved0 : 3;
  //! @brief Varialbe natural alignment.
  uint8_t _alignment;

  //! @brief Variable size.
  uint32_t _size;

  //! @brief Home memory offset.
  int32_t _memOffset;
  //! @brief Home memory cell, used by @c Context (initially NULL).
  MemCell* _memCell;

  //! @brief Register read access statistics.
  uint32_t rReadCount;
  //! @brief Register write access statistics.
  uint32_t rWriteCount;

  //! @brief Memory read statistics.
  uint32_t mReadCount;
  //! @brief Memory write statistics.
  uint32_t mWriteCount;

  // --------------------------------------------------------------------------
  // [Members - Temporary Usage]
  // --------------------------------------------------------------------------

  // These variables are only used during register allocation. They are
  // initialized by init() phase and cleared by cleanup() phase.

  union {
    //! @brief Temporary link to VarAttr* used by the @ref Context used in
    //! various phases, but always set back to NULL when finished.
    //!
    //! This temporary data is designed to be used by algorithms that need to
    //! store some data into variables themselves during compilation. But it's
    //! expected that after variable is compiled & translated the data is set
    //! back to zero/null. Initial value is NULL.
    VarAttr* _va;

    //! @internal
    uintptr_t _vaUInt;
  };
};

// ============================================================================
// [asmjit::VarAttr]
// ============================================================================

struct VarAttr {
  // --------------------------------------------------------------------------
  // [Setup]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void setup(VarData* vd, uint32_t flags = 0, uint32_t inRegs = 0, uint32_t allocableRegs = 0) {
    _vd = vd;
    _flags = flags;
    _varCount = 0;
    _argStackCount = 0;
    _inRegIndex = kInvalidReg;
    _outRegIndex = kInvalidReg;
    _inRegs = inRegs;
    _allocableRegs = allocableRegs;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get VarData.
  ASMJIT_INLINE VarData* getVd() const { return _vd; }
  //! @brief Set VarData.
  ASMJIT_INLINE void setVd(VarData* vd) { _vd = vd; }

  //! @brief Get flags.
  ASMJIT_INLINE uint32_t getFlags() const { return _flags; }
  //! @brief Set flags.
  ASMJIT_INLINE void setFlags(uint32_t flags) { _flags = flags; }

  //! @brief Get whether @a flag is on.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) { return (_flags & flag) != 0; }
  //! @brief Add @a flags.
  ASMJIT_INLINE void addFlags(uint32_t flags) { _flags |= flags; }
  //! @brief Mask @a flags.
  ASMJIT_INLINE void andFlags(uint32_t flags) { _flags &= flags; }
  //! @brief Clear @a flags.
  ASMJIT_INLINE void delFlags(uint32_t flags) { _flags &= ~flags; }

  //! @brief Get how many times the variable is used by the instruction/node.
  ASMJIT_INLINE uint32_t getVarCount() const { return _varCount; }
  //! @brief Set how many times the variable is used by the instruction/node.
  ASMJIT_INLINE void setVarCount(uint32_t count) { _varCount = static_cast<uint8_t>(count); }
  //! @brief Add how many times the variable is used by the instruction/node.
  ASMJIT_INLINE void addVarCount(uint32_t count = 1) { _varCount += static_cast<uint8_t>(count); }

  //! @brief Get how many times the variable is used by the function argument.
  ASMJIT_INLINE uint32_t getArgStackCount() const { return _argStackCount; }
  //! @brief Set how many times the variable is used by the function argument.
  ASMJIT_INLINE void setArgStackCount(uint32_t count) { _argStackCount = static_cast<uint8_t>(count); }
  //! @brief Add how many times the variable is used by the function argument.
  ASMJIT_INLINE void addArgStackCount(uint32_t count = 1) { _argStackCount += static_cast<uint8_t>(count); }

  //! @brief Get whether the variable has to be allocated in a specific input register.
  ASMJIT_INLINE uint32_t hasInRegIndex() const { return _inRegIndex != kInvalidReg; }
  //! @brief Get the input register index or @ref kInvalidReg.
  ASMJIT_INLINE uint32_t getInRegIndex() const { return _inRegIndex; }
  //! @brief Set the input register index.
  ASMJIT_INLINE void setInRegIndex(uint32_t index) { _inRegIndex = static_cast<uint8_t>(index); }
  //! @brief Reset the input register index.
  ASMJIT_INLINE void resetInRegIndex() { _inRegIndex = kInvalidReg; }

  //! @brief Get whether the variable has to be allocated in a specific output register.
  ASMJIT_INLINE uint32_t hasOutRegIndex() const { return _outRegIndex != kInvalidReg; }
  //! @brief Get the output register index or @ref kInvalidReg.
  ASMJIT_INLINE uint32_t getOutRegIndex() const { return _outRegIndex; }
  //! @brief Set the output register index.
  ASMJIT_INLINE void setOutRegIndex(uint32_t index) { _outRegIndex = static_cast<uint8_t>(index); }
  //! @brief Reset the output register index.
  ASMJIT_INLINE void resetOutRegIndex() { _outRegIndex = kInvalidReg; }

  //! @brief Get whether the mandatory input registers are in used.
  ASMJIT_INLINE bool hasInRegs() const { return _inRegs != 0; }
  //! @brief Get mandatory input registers (mask).
  ASMJIT_INLINE uint32_t getInRegs() const { return _inRegs; }
  //! @brief Set mandatory input registers (mask).
  ASMJIT_INLINE void setInRegs(uint32_t mask) { _inRegs = mask; }
  //! @brief Add mandatory input registers (mask).
  ASMJIT_INLINE void addInRegs(uint32_t mask) { _inRegs |= mask; }
  //! @brief And mandatory input registers (mask).
  ASMJIT_INLINE void andInRegs(uint32_t mask) { _inRegs &= mask; }
  //! @brief Clear mandatory input registers (mask).
  ASMJIT_INLINE void delInRegs(uint32_t mask) { _inRegs &= ~mask; }

  //! @brief Get allocable input registers (mask).
  ASMJIT_INLINE uint32_t getAllocableRegs() const { return _allocableRegs; }
  //! @brief Set allocable input registers (mask).
  ASMJIT_INLINE void setAllocableRegs(uint32_t mask) { _allocableRegs = mask; }
  //! @brief Add allocable input registers (mask).
  ASMJIT_INLINE void addAllocableRegs(uint32_t mask) { _allocableRegs |= mask; }
  //! @brief And allocable input registers (mask).
  ASMJIT_INLINE void andAllocableRegs(uint32_t mask) { _allocableRegs &= mask; }
  //! @brief Clear allocable input registers (mask).
  ASMJIT_INLINE void delAllocableRegs(uint32_t mask) { _allocableRegs &= ~mask; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE VarAttr& operator=(const VarAttr& other) {
    ::memcpy(this, &other, sizeof(VarAttr));
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  VarData* _vd;
  //! @brief Flags.
  uint32_t _flags;

  union {
    struct {
      //! @brief How many times the variable is used by the instruction/node.
      uint8_t _varCount;
      //! @brief How many times the variable is used as a function argument on
      //! the stack.
      //!
      //! This is important to know for function-call allocator. It doesn't
      //! allocate by arguments, but by using VarAttr's.
      uint8_t _argStackCount;
      //! @brief Input register index or @ref kInvalidReg if it's not given.
      //!
      //! Even if the input register index is not given (i.e. it may by any
      //! register), register allocator should assign an index that will be
      //! used to persist a variable into this specific index. It's helpful
      //! in situations where one variable has to be allocated in multiple
      //! registers to determine the register which will be persistent.
      uint8_t _inRegIndex;
      //! @brief Output register index or @ref kInvalidReg if it's not given.
      //!
      //! Typically @ref kInvalidReg if variable is only used on input.
      uint8_t _outRegIndex;
    };

    //! @internal
    //!
    //! @brief Packed data #0.
    uint32_t _packed;
  };

  //! @brief Mandatory input registers.
  //!
  //! Mandatory input registers are required by the instruction even if
  //! there are duplicates. This schema allows us to allocate one variable
  //! in one or more register when needed. Required mostly by instructions
  //! that have implicit register operands (imul, cpuid, ...) and function
  //! call.
  uint32_t _inRegs;

  //! @brief Allocable input registers.
  //!
  //! Optional input registers is a mask of all allocable registers for a given
  //! variable where we have to pick one of them. This mask is usually not used
  //! when _inRegs is set. If both masks are used then the register
  //! allocator tries first to find an intersection between these and allocates
  //! an extra slot if not found.
  uint32_t _allocableRegs;
};

// ============================================================================
// [asmjit::BaseVarInst]
// ============================================================================

//! @brief Variable allocation instructions.
struct BaseVarInst {};

// ============================================================================
// [asmjit::BaseVarState]
// ============================================================================

//! @brief Variable(s) state.
struct BaseVarState {};

// ============================================================================
// [asmjit::BaseNode]
// ============================================================================

//! @brief Base node.
//!
//! @ref Every node represents an abstract instruction, directive, label, or
//! macro-instruction generated by compiler.
struct BaseNode {
  ASMJIT_NO_COPY(BaseNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new @ref BaseNode.
  //!
  //! @note Always use compiler to create nodes.
  ASMJIT_INLINE BaseNode(BaseCompiler* compiler, uint32_t type); // Defined-Later.

  //! @brief Destroy @ref BaseNode.
  ASMJIT_INLINE ~BaseNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get previous node in the compiler stream.
  ASMJIT_INLINE BaseNode* getPrev() const { return _prev; }
  //! @brief Get next node in the compiler stream.
  ASMJIT_INLINE BaseNode* getNext() const { return _next; }

  //! @brief Get comment string.
  ASMJIT_INLINE const char* getComment() const { return _comment; }
  //! @brief Set comment string to @a str.
  ASMJIT_INLINE void setComment(const char* comment) { _comment = comment; }

  //! @brief Get type of node, see @ref kNodeType.
  ASMJIT_INLINE uint32_t getType() const { return _type; }

  //! @brief Get node flags.
  ASMJIT_INLINE uint32_t getFlags() const { return _flags; }
  //! @brief Set node flags to @a flags.
  ASMJIT_INLINE void setFlags(uint32_t flags) { _flags = static_cast<uint16_t>(flags); }

  //! @brief Get whether the instruction has flag @a flag.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const { return (static_cast<uint32_t>(_flags) & flag) != 0; }
  //! @brief Add instruction @a flags.
  ASMJIT_INLINE void addFlags(uint32_t flags) { _flags |= static_cast<uint16_t>(flags); }
  //! @brief Clear instruction @a flags.
  ASMJIT_INLINE void delFlags(uint32_t flags) { _flags &= static_cast<uint16_t>(~flags); }

  //! @brief Get whether the node has beed fetched.
  ASMJIT_INLINE bool isFetched() const { return _flowId != 0; }
  //! @brief Get whether the node has been translated.
  ASMJIT_INLINE bool isTranslated() const { return hasFlag(kNodeFlagIsTranslated); }

  //! @brief Whether the instruction is an unconditional jump.
  ASMJIT_INLINE bool isJmp() const { return hasFlag(kNodeFlagIsJmp); }
  //! @brief Whether the instruction is a conditional jump.
  ASMJIT_INLINE bool isJcc() const { return hasFlag(kNodeFlagIsJcc); }
  //! @brief Whether the instruction is an unconditional or conditional jump.
  ASMJIT_INLINE bool isJmpOrJcc() const { return hasFlag(kNodeFlagIsJmp | kNodeFlagIsJcc); }
  //! @brief Whether the instruction is a return.
  ASMJIT_INLINE bool isRet() const { return hasFlag(kNodeFlagIsRet); }

  //! @brief Get whether the instruction is special.
  ASMJIT_INLINE bool isSpecial() const { return hasFlag(kNodeFlagIsSpecial); }
  //! @brief Get whether the instruction accesses FPU.
  ASMJIT_INLINE bool isFp() const { return hasFlag(kNodeFlagIsFp); }

  //! @brief Get flow index.
  ASMJIT_INLINE uint32_t getFlowId() const { return _flowId; }
  //! @brief Set flow index.
  ASMJIT_INLINE void setFlowId(uint32_t flowId) { _flowId = flowId; }

  //! @brief Get whether node contains variable allocation instructions.
  ASMJIT_INLINE bool hasVarInst() const { return _varInst != NULL; }

  //! @brief Get variable allocation instructions.
  ASMJIT_INLINE BaseVarInst* getVarInst() const { return _varInst; }
  //! @brief Get variable allocation instructions <T>.
  template<typename T>
  ASMJIT_INLINE T* getVarInst() const { return static_cast<T*>(_varInst); }
  //! @brief Set variable allocation instructions.
  ASMJIT_INLINE void setVarInst(BaseVarInst* vi) { _varInst = vi; }

  //! @brief Get node state.
  ASMJIT_INLINE BaseVarState* getState() const { return _state; }
  //! @brief Get node state <T>.
  template<typename T>
  ASMJIT_INLINE T* getState() const { return static_cast<BaseVarState*>(_state); }
  //! @brief Set node state.
  ASMJIT_INLINE void setState(BaseVarState* state) { _state = state; }

  //! @brief Get whether the node has variable liveness bits.
  ASMJIT_INLINE bool hasLiveness() const { return _liveness != NULL; }
  //! @brief Get variable liveness bits.
  ASMJIT_INLINE VarBits* getLiveness() const { return _liveness; }
  //! @brief Set variable liveness bits.
  ASMJIT_INLINE void setLiveness(VarBits* liveness) { _liveness = liveness; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Previous node.
  BaseNode* _prev;
  //! @brief Next node.
  BaseNode* _next;

  //! @brief Node type, see @ref kNodeType.
  uint8_t _type;
  //! @brief Operands count (if the node has operands, otherwise zero).
  uint8_t _opCount;
  //! @brief Node flags, different meaning for every node type.
  uint16_t _flags;

  //! @brief Flow index.
  uint32_t _flowId;

  //! @brief Inline comment string, initially set to NULL.
  const char* _comment;

  //! @brief Variable allocation instructions (initially NULL, filled by prepare
  //! phase).
  BaseVarInst* _varInst;

  //! @brief Variable liveness bits (initially NULL, filled by analysis phase).
  VarBits* _liveness;

  //! @brief Saved state.
  //!
  //! Initially NULL, not all nodes have saved state, only branch/flow control
  //! nodes.
  BaseVarState* _state;
};

// ============================================================================
// [asmjit::AlignNode]
// ============================================================================

//! @brief Align node.
struct AlignNode : public BaseNode {
  ASMJIT_NO_COPY(AlignNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref AlignNode instance.
  ASMJIT_INLINE AlignNode(BaseCompiler* compiler, uint32_t size) : BaseNode(compiler, kNodeTypeAlign) {
    _size = size;
  }

  //! @brief Destroy the @ref AlignNode instance.
  ASMJIT_INLINE ~AlignNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get align size in bytes.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }
  //! @brief Set align size in bytes to @a size.
  ASMJIT_INLINE void setSize(uint32_t size) { _size; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Size of the alignment.
  uint32_t _size;
};

// ============================================================================
// [asmjit::EmbedNode]
// ============================================================================

//! @brief Embed node.
//!
//! Embed node is used to embed data into final assembler stream. The data is
//! considered to be RAW; No analysis is performed on RAW data.
struct EmbedNode : public BaseNode {
  ASMJIT_NO_COPY(EmbedNode)

  // --------------------------------------------------------------------------
  // [Enums]
  // --------------------------------------------------------------------------

  enum { kInlineBufferSize = 8 };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref EmbedNode instance.
  ASMJIT_INLINE EmbedNode(BaseCompiler* compiler, void* data, uint32_t size) : BaseNode(compiler, kNodeTypeEmbed) {
    _size = size;
    if (size <= kInlineBufferSize)
      ::memcpy(_data.buf, data, size);
    else
      _data.ptr = static_cast<uint8_t*>(data);
  }

  //! @brief Destroy the @ref EmbedNode instance.
  ASMJIT_INLINE ~EmbedNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get pointer to data.
  uint8_t* getData() { return getSize() <= kInlineBufferSize ? const_cast<uint8_t*>(_data.buf) : _data.ptr; }
  //! @brief Get size of data.
  uint32_t getSize() const { return _size; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Size of the embedded data.
  uint32_t _size;

  union {
    //! @brief data buffer.
    uint8_t buf[kInlineBufferSize];
    //! @brief Data buffer.
    uint8_t* ptr;
  } _data;
};

// ============================================================================
// [asmjit::CommentNode]
// ============================================================================

//! @brief Comment node.
//!
//! Comments allows to comment your assembler stream for better debugging
//! and visualization. Comments are usually ignored in release builds unless
//! the logger is present.
struct CommentNode : public BaseNode {
  ASMJIT_NO_COPY(CommentNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref CommentNode instance.
  ASMJIT_INLINE CommentNode(BaseCompiler* compiler, const char* comment) : BaseNode(compiler, kNodeTypeComment) {
    _comment = comment;
  }

  //! @brief Destroy the @ref CommentNode instance.
  ASMJIT_INLINE ~CommentNode() {}
};

// ============================================================================
// [asmjit::HintNode]
// ============================================================================

//! @brief Hint node.
struct HintNode : public BaseNode {
  ASMJIT_NO_COPY(HintNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref HintNode instance.
  ASMJIT_INLINE HintNode(BaseCompiler* compiler, VarData* vd, uint32_t hint, uint32_t value) : BaseNode(compiler, kNodeTypeHint) {
    _vd = vd;
    _hint = hint;
    _value = value;
  }

  //! @brief Destroy the @ref HintNode instance.
  ASMJIT_INLINE ~HintNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get variable.
  ASMJIT_INLINE VarData* getVd() const { return _vd; }

  //! @brief Get hint it (see @ref kVarHint).
  ASMJIT_INLINE uint32_t getHint() const{ return _hint; }
  //! @brief Set hint it (see @ref kVarHint).
  ASMJIT_INLINE void setHint(uint32_t hint) { _hint = hint; }

  //! @brief Get hint value.
  ASMJIT_INLINE uint32_t getValue() const { return _value; }
  //! @brief Set hint value.
  ASMJIT_INLINE void setValue(uint32_t value) { _value = value; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Variable.
  VarData* _vd;
  //! @brief Hint id.
  uint32_t _hint;
  //! @brief Value.
  uint32_t _value;
};

// ============================================================================
// [asmjit::TargetNode]
// ============================================================================

//! @brief label node.
struct TargetNode : public BaseNode {
  ASMJIT_NO_COPY(TargetNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref TargetNode instance.
  ASMJIT_INLINE TargetNode(BaseCompiler* compiler, uint32_t labelId) : BaseNode(compiler, kNodeTypeTarget) {
    _id = labelId;
    _numRefs = 0;
    _from = NULL;
  }

  //! @brief Destroy the @ref TargetNode instance.
  ASMJIT_INLINE ~TargetNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get target label.
  ASMJIT_INLINE Label getLabel() const { return Label(_id); }
  //! @brief Get target label id.
  ASMJIT_INLINE uint32_t getLabelId() const { return _id; }

  //! @brief Get first jmp instruction.
  ASMJIT_INLINE JumpNode* getFrom() const { return _from; }

  //! @brief Get whether the node has assigned state.
  ASMJIT_INLINE bool hasState() const { return _state != NULL; }
  //! @brief Get state for this target.
  ASMJIT_INLINE BaseVarState* getState() const { return _state; }
  //! @brief Set state for this target.
  ASMJIT_INLINE void setState(BaseVarState* state) { _state = state; }

  //! @brief Get number of jumps to this target.
  ASMJIT_INLINE uint32_t getNumRefs() const { return _numRefs; }
  //! @brief Set number of jumps to this target.
  ASMJIT_INLINE void setNumRefs(uint32_t i) { _numRefs = i; }
  //! @brief Add number of jumps to this target.
  ASMJIT_INLINE void addNumRefs(uint32_t i = 1) { _numRefs += i; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Label id.
  uint32_t _id;
  //! @brief Count of jumps here.
  uint32_t _numRefs;

  //! @brief First jump instruction that points to this target (label).
  JumpNode* _from;
};

// ============================================================================
// [asmjit::InstNode]
// ============================================================================

//! @brief Instruction node.
struct InstNode : public BaseNode {
  ASMJIT_NO_COPY(InstNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref InstNode instance.
  ASMJIT_INLINE InstNode(BaseCompiler* compiler, uint32_t code, uint32_t options, Operand* opList, uint32_t opCount) : BaseNode(compiler, kNodeTypeInst) {
    _code = static_cast<uint16_t>(code);
    _options = static_cast<uint8_t>(options);

    _opCount = static_cast<uint8_t>(opCount);
    _opList = opList;

    _updateMemOp();
  }

  //! @brief Destroy the @ref InstNode instance.
  ASMJIT_INLINE ~InstNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get instruction code, see @c kInstCode.
  ASMJIT_INLINE uint32_t getCode() const
  { return _code; }

  //! @brief Set instruction code to @a code.
  //!
  //! Please do not modify instruction code if you are not know what you are
  //! doing. Incorrect instruction code or operands can raise assertion() at
  //! runtime.
  ASMJIT_INLINE void setCode(uint32_t code)
  { _code = static_cast<uint16_t>(code); }

  //! @brief Whether the instruction is an unconditional jump or whether the
  //! instruction is a conditional jump which is likely to be taken.
  ASMJIT_INLINE bool isTaken() const { return hasFlag(kNodeFlagIsTaken); }

  //! @brief Get emit options.
  ASMJIT_INLINE uint32_t getOptions() const { return _options; }
  //! @brief Set emit options.
  ASMJIT_INLINE void setOptions(uint32_t options) { _options = static_cast<uint8_t>(options); }
  //! @brief Add emit options.
  ASMJIT_INLINE void addOptions(uint32_t options) { _options |= static_cast<uint8_t>(options); }
  //! @brief Mask emit options.
  ASMJIT_INLINE void andOptions(uint32_t options) { _options &= static_cast<uint8_t>(options); }
  //! @brief Clear emit options.
  ASMJIT_INLINE void delOptions(uint32_t options) { _options &= static_cast<uint8_t>(~options); }

  //! @brief Get operands list.
  ASMJIT_INLINE Operand* getOpList() { return _opList; }
  //! @overload
  ASMJIT_INLINE const Operand* getOpList() const { return _opList; }

  //! @brief Get operands count.
  ASMJIT_INLINE uint32_t getOpCount() const { return _opCount; }

  //! @brief Get whether the instruction contains a memory operand.
  ASMJIT_INLINE bool hasMemOp() const { return _memOpIndex != 0xFF; }

  //! @brief Set memory operand index (in opList), 0xFF means that instruction
  //! doesn't have a memory operand.
  ASMJIT_INLINE void setMemOpIndex(uint32_t index) { _memOpIndex = static_cast<uint8_t>(index); }
  //! @brief Reset memory operand index, setting it to 0xFF.
  ASMJIT_INLINE void resetMemOpIndex() { _memOpIndex = 0xFF; }

  //! @brief Get memory operand.
  //!
  //! @note Can only be called if the instruction has such operand, see @ref
  //! hasMemOp().
  ASMJIT_INLINE BaseMem* getMemOp() const {
    ASMJIT_ASSERT(hasMemOp());
    return static_cast<BaseMem*>(&_opList[_memOpIndex]);
  }

  //! @overload
  template<typename T>
  ASMJIT_INLINE T* getMemOp() const {
    ASMJIT_ASSERT(hasMemOp());
    return static_cast<T*>(&_opList[_memOpIndex]);
  }

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void _updateMemOp() {
    Operand* opList = getOpList();
    uint32_t opCount = getOpCount();

    uint32_t i;
    for (i = 0; i < opCount; i++)
      if (opList[i].isMem())
        goto _Update;
    i = 0xFF;

_Update:
    setMemOpIndex(i);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Instruction code, see @c kInstCode.
  uint16_t _code;
  //! @brief Instruction options, see @c kInstOptions.
  uint8_t _options;
  //! @internal
  uint8_t _memOpIndex;

  //! @brief Operands list.
  Operand* _opList;
};

// ============================================================================
// [asmjit::JumpNode]
// ============================================================================

//! @brief Jump node.
struct JumpNode : public InstNode {
  ASMJIT_NO_COPY(JumpNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE JumpNode(BaseCompiler* compiler, uint32_t code, uint32_t options, Operand* opList, uint32_t opCount) :
    InstNode(compiler, code, options, opList, opCount) {}
  ASMJIT_INLINE ~JumpNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE TargetNode* getTarget() const { return _target; }
  ASMJIT_INLINE JumpNode* getJumpNext() const { return _jumpNext; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Target node.
  TargetNode* _target;
  //! @brief Next jump to the same target in a single linked-list.
  JumpNode *_jumpNext;
};

// ============================================================================
// [asmjit::FuncNode]
// ============================================================================

//! @brief Function declaration node.
//!
//! Functions are base blocks for generating assembler output. Each generated
//! assembler stream needs standard entry and leave sequences which are compatible
//! with the operating system ABI.
//!
//! @ref FuncNode can be used to generate function prolog and epilog which are
//! compatible with a given function calling convention and to allocate and
//! manage variables that can be allocated/spilled during compilation phase.
struct FuncNode : public BaseNode {
  ASMJIT_NO_COPY(FuncNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @c FuncNode instance.
  //!
  //! @note Always use @ref asmjit::BaseCompiler::addFunc() to create a
  //! @ref FuncNode instance.
  ASMJIT_INLINE FuncNode(BaseCompiler* compiler) :
    BaseNode(compiler, kNodeTypeFunc),
    _entryNode(NULL),
    _exitNode(NULL),
    _end(NULL),
    _decl(NULL),
    _argList(NULL),
    _funcHints(IntUtil::mask(kFuncHintNaked)),
    _funcFlags(0),
    _expectedStackAlignment(0),
    _requiredStackAlignment(0),
    _redZoneSize(0),
    _spillZoneSize(0),
    _argStackSize(0),
    _memStackSize(0),
    _callStackSize(0) {}

  //! @brief Destroy the @c FuncNode instance.
  ASMJIT_INLINE ~FuncNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get function entry @ref TargetNode.
  ASMJIT_INLINE TargetNode* getEntryNode() const { return _entryNode; }
  //! @brief Get function exit @ref TargetNode.
  ASMJIT_INLINE TargetNode* getExitNode() const { return _exitNode; }

  //! @brief Get function entry label.
  ASMJIT_INLINE Label getEntryLabel() const { return _entryNode->getLabel(); }
  //! @brief Get function exit label.
  ASMJIT_INLINE Label getExitLabel() const { return _exitNode->getLabel(); }

  //! @brief Get function @ref EndNode.
  ASMJIT_INLINE EndNode* getEnd() const { return _end; }
  //! @brief Get function declaration.
  ASMJIT_INLINE FuncDecl* getDecl() const { return _decl; }

  //! @brief Get arguments list.
  ASMJIT_INLINE VarData** getArgList() const { return _argList; }
  //! @brief Get arguments count.
  ASMJIT_INLINE uint32_t getArgCount() const { return _decl->getArgCount(); }

  //! @brief Get argument at @a i.
  ASMJIT_INLINE VarData* getArg(uint32_t i) const {
    ASMJIT_ASSERT(i < getArgCount());
    return _argList[i];
  }

  //! @brief Set argument at @a i.
  ASMJIT_INLINE void setArg(uint32_t i, VarData* vd) {
    ASMJIT_ASSERT(i < getArgCount());
    _argList[i] = vd;
  }

  //! @brief Reset argument at @a i.
  ASMJIT_INLINE void resetArg(uint32_t i) {
    ASMJIT_ASSERT(i < getArgCount());
    _argList[i] = NULL;
  }

  //! @brief Get function hints.
  ASMJIT_INLINE uint32_t getFuncHints() const { return _funcHints; }
  //! @brief Get function flags.
  ASMJIT_INLINE uint32_t getFuncFlags() const { return _funcFlags; }

  //! @brief Get whether the _funcFlags has @a flag
  ASMJIT_INLINE bool hasFuncFlag(uint32_t flag) const { return (_funcFlags & flag) != 0; }
  //! @brief Set function @a flag.
  ASMJIT_INLINE void addFuncFlags(uint32_t flags) { _funcFlags |= flags; }
  //! @brief Clear function @a flag.
  ASMJIT_INLINE void clearFuncFlags(uint32_t flags) { _funcFlags &= ~flags; }

  //! @brief Get whether the function is naked.
  ASMJIT_INLINE bool isNaked() const { return hasFuncFlag(kFuncFlagIsNaked); }
  //! @brief Get whether the function is also a caller.
  ASMJIT_INLINE bool isCaller() const { return hasFuncFlag(kFuncFlagIsCaller); }
  //! @brief Get whether the required stack alignment is lower than expected one,
  //! thus it has to be aligned manually.
  ASMJIT_INLINE bool isStackMisaligned() const { return hasFuncFlag(kFuncFlagIsStackMisaligned); }
  //! @brief Get whether the stack pointer is adjusted inside function prolog/epilog.
  ASMJIT_INLINE bool isStackAdjusted() const { return hasFuncFlag(kFuncFlagIsStackAdjusted); }

  //! @brief Get whether the function is finished.
  ASMJIT_INLINE bool isFinished() const { return hasFuncFlag(kFuncFlagIsFinished); }

  //! @brief Get expected stack alignment.
  ASMJIT_INLINE uint32_t getExpectedStackAlignment() const { return _expectedStackAlignment; }
  //! @brief Set expected stack alignment.
  ASMJIT_INLINE void setExpectedStackAlignment(uint32_t alignment) { _expectedStackAlignment = alignment; }

  //! @brief Get required stack alignment.
  ASMJIT_INLINE uint32_t getRequiredStackAlignment() const { return _requiredStackAlignment; }
  //! @brief Set required stack alignment.
  ASMJIT_INLINE void setRequiredStackAlignment(uint32_t alignment) { _requiredStackAlignment = alignment; }

  //! @brief Update required stack alignment so it's not lower than expected
  //! stack alignment.
  ASMJIT_INLINE void updateRequiredStackAlignment() {
    if (_requiredStackAlignment <= _expectedStackAlignment) {
      _requiredStackAlignment = _expectedStackAlignment;
      clearFuncFlags(kFuncFlagIsStackMisaligned);
    }
    else {
      addFuncFlags(kFuncFlagIsStackMisaligned);
    }
  }

  //! @brief Set stack "Red Zone" size.
  ASMJIT_INLINE uint32_t getRedZoneSize() const { return _redZoneSize; }
  //! @brief Get stack "Red Zone" size.
  ASMJIT_INLINE void setRedZoneSize(uint32_t s) { _redZoneSize = static_cast<uint16_t>(s); }

  //! @brief Set stack "Spill Zone" size.
  ASMJIT_INLINE uint32_t getSpillZoneSize() const { return _spillZoneSize; }
  //! @brief Get stack "Spill Zone" size.
  ASMJIT_INLINE void setSpillZoneSize(uint32_t s) { _spillZoneSize = static_cast<uint16_t>(s); }

  //! @brief Get stack size used by function arguments.
  ASMJIT_INLINE uint32_t getArgStackSize() const { return _argStackSize; }

  //! @brief Get stack size used by variables and memory allocated on the stack.
  ASMJIT_INLINE uint32_t getMemStackSize() const { return _memStackSize; }

  //! @brief Get stack size used by function calls.
  ASMJIT_INLINE uint32_t getCallStackSize() const { return _callStackSize; }
  //! @brief Merge stack size used by function call with @a s.
  ASMJIT_INLINE void mergeCallStackSize(uint32_t s) { if (_callStackSize < s) _callStackSize = s; }

  // --------------------------------------------------------------------------
  // [Hints]
  // --------------------------------------------------------------------------

  //! @brief Set function hint.
  ASMJIT_INLINE void setHint(uint32_t hint, uint32_t value) {
    ASMJIT_ASSERT(hint <= 31);
    ASMJIT_ASSERT(value <= 1);

    _funcHints &= ~(1     << hint);
    _funcHints |=  (value << hint);
  }

  //! @brief Get function hint.
  ASMJIT_INLINE uint32_t getHint(uint32_t hint) const {
    ASMJIT_ASSERT(hint <= 31);
    return (_funcHints >> hint) & 0x1;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Function entry.
  TargetNode* _entryNode;
  //! @brief Function exit.
  TargetNode* _exitNode;

  //! @brief Function declaration.
  FuncDecl* _decl;
  //! @brief Function end.
  EndNode* _end;

  //! @brief Arguments list as @ref VarData.
  VarData** _argList;

  //! @brief Function hints;
  uint32_t _funcHints;
  //! @brief Function flags.
  uint32_t _funcFlags;

  //! @brief Expected stack alignment (we depend on this value).
  //!
  //! @note It can be global alignment given by the OS or described by an
  //! target platform ABI.
  uint32_t _expectedStackAlignment;
  //! @brief Required stack alignment (usually for multimedia instructions).
  uint32_t _requiredStackAlignment;

  //! @brief The "Red Zone" suze - count of bytes which might be accessed
  //! without adjusting the stack pointer.
  uint16_t _redZoneSize;
  //! @brief Spill zone size (zone used by WIN64ABI).
  uint16_t _spillZoneSize;

  //! @brief Stack size needed for function arguments.
  uint32_t _argStackSize;
  //! @brief Stack size needed for all variables and memory allocated on
  //! the stack.
  uint32_t _memStackSize;
  //! @brief Stack size needed to call other functions.
  uint32_t _callStackSize;
};

// ============================================================================
// [asmjit::EndNode]
// ============================================================================

//! @brief End of function/block node.
struct EndNode : public BaseNode {
  ASMJIT_NO_COPY(EndNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref EndNode instance.
  ASMJIT_INLINE EndNode(BaseCompiler* compiler) : BaseNode(compiler, kNodeTypeEnd) {
    _flags |= kNodeFlagIsRet;
  }

  //! @brief Destroy the @ref EndNode instance.
  ASMJIT_INLINE ~EndNode() {}
};

// ============================================================================
// [asmjit::RetNode]
// ============================================================================

//! @brief Function return node.
struct RetNode : public BaseNode {
  ASMJIT_NO_COPY(RetNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref RetNode instance.
  ASMJIT_INLINE RetNode(BaseCompiler* compiler, const Operand& o0, const Operand& o1) : BaseNode(compiler, kNodeTypeRet) {
    _flags |= kNodeFlagIsRet;
    _ret[0] = o0;
    _ret[1] = o1;
  }

  //! @brief Destroy the @ref RetNode instance.
  ASMJIT_INLINE ~RetNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get the first return operand.
  ASMJIT_INLINE Operand& getFirst() { return _ret[0]; }
  //! @overload
  ASMJIT_INLINE const Operand& getFirst() const { return _ret[0]; }

  //! @brief Get the second return operand.
  ASMJIT_INLINE Operand& getSecond() { return _ret[1]; }
   //! @overload
  ASMJIT_INLINE const Operand& getSecond() const { return _ret[1]; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Ret operand(s).
  Operand _ret[2];
};

// ============================================================================
// [asmjit::CallNode]
// ============================================================================

//! @brief Function-call node.
struct CallNode : public BaseNode {
  ASMJIT_NO_COPY(CallNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref CallNode instance.
  ASMJIT_INLINE CallNode(BaseCompiler* compiler, const Operand& target) :
    BaseNode(compiler, kNodeTypeCall),
    _decl(NULL),
    _target(target),
    _args(NULL) {}

  //! @brief Destroy the @ref CallNode instance.
  ASMJIT_INLINE ~CallNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get function declaration.
  ASMJIT_INLINE FuncDecl* getDecl() const { return _decl; }

  //! @brief Get target operand.
  ASMJIT_INLINE Operand& getTarget() { return _target; }
  //! @overload
  ASMJIT_INLINE const Operand& getTarget() const  { return _target; }

  //! @brief Get return at @a i.
  ASMJIT_INLINE Operand& getRet(uint32_t i = 0) {
    ASMJIT_ASSERT(i < 2);
    return _ret[i];
  }
  //! @overload
  ASMJIT_INLINE const Operand& getRet(uint32_t i = 0) const  {
    ASMJIT_ASSERT(i < 2);
    return _ret[i];
  }

  //! @brief Get argument at @a i.
  ASMJIT_INLINE Operand& getArg(uint32_t i) {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i];
  }
  //! @overload
  ASMJIT_INLINE const Operand& getArg(uint32_t i) const  {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i];
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Function declaration.
  FuncDecl* _decl;

  //! @brief Target (address of function, register, label, ...).
  Operand _target;
  //! @brief Return.
  Operand _ret[2];
  //! @brief Arguments.
  Operand* _args;
};

// ============================================================================
// [asmjit::SArgNode]
// ============================================================================

//! @brief Function-call 'argument on the stack' node.
struct SArgNode : public BaseNode {
  ASMJIT_NO_COPY(SArgNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref SArgNode instance.
  ASMJIT_INLINE SArgNode(BaseCompiler* compiler, VarData* vd, CallNode* call) :
    BaseNode(compiler, kNodeTypeSArg),
    _vd(vd),
    _call(call) {}

  //! @brief Destroy the @ref SArgNode instance.
  ASMJIT_INLINE ~SArgNode() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get the associated variable.
  ASMJIT_INLINE VarData* getVd() const { return _vd; }

  //! @brief Get the associated function-call.
  ASMJIT_INLINE CallNode* getCall() const { return _call; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Variable.
  VarData* _vd;
  //! @brief Associated @ref CallNode.
  CallNode* _call;
};

// ============================================================================
// [asmjit::BaseCompiler]
// ============================================================================

//! @brief Base compiler.
//!
//! @sa BaseAssembler.
struct BaseCompiler : public CodeGen {
  ASMJIT_NO_COPY(BaseCompiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref BaseCompiler instance.
  ASMJIT_API BaseCompiler(BaseRuntime* runtime);
  //! @brief Destroy the @ref BaseCompiler instance.
  ASMJIT_API virtual ~BaseCompiler();

  // --------------------------------------------------------------------------
  // [LookAhead]
  // --------------------------------------------------------------------------

  //! @brief Get maximum look ahead.
  ASMJIT_INLINE uint32_t getMaxLookAhead() const { return _maxLookAhead; }
  //! @brief Set maximum look ahead to @a val.
  ASMJIT_INLINE void setMaxLookAhead(uint32_t val) { _maxLookAhead = val; }

  // --------------------------------------------------------------------------
  // [Clear / Reset]
  // --------------------------------------------------------------------------

  //! @brief Clear everything, but keep buffers allocated.
  //!
  //! @note This method will destroy your code.
  ASMJIT_API void clear();
  //! @brief Clear everything and reset all buffers.
  //!
  //! @note This method will destroy your code.
  ASMJIT_API void reset();
  //! @brief Called by clear() and reset() to clear all data related to derived
  //! class implementation.
  ASMJIT_API virtual void _purge();

  // --------------------------------------------------------------------------
  // [Nodes]
  // --------------------------------------------------------------------------

  template<typename T>
  ASMJIT_INLINE T* newNode() {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this);
  }

  template<typename T, typename P0>
  ASMJIT_INLINE T* newNode(P0 p0) {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0);
  }

  template<typename T, typename P0, typename P1>
  ASMJIT_INLINE T* newNode(P0 p0, P1 p1) {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0, p1);
  }

  template<typename T, typename P0, typename P1, typename P2>
  ASMJIT_INLINE T* newNode(P0 p0, P1 p1, P2 p2) {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0, p1, p2);
  }

  //! @brief Get first node.
  ASMJIT_INLINE BaseNode* getFirstNode() const { return _firstNode; }
  //! @brief Get last node.
  ASMJIT_INLINE BaseNode* getLastNode() const { return _lastNode; }

  //! @brief Get current node.
  //!
  //! @note If this method returns @c NULL it means that nothing has been emitted
  //! yet.
  ASMJIT_INLINE BaseNode* getCursor() const { return _cursor; }
  //! @brief Set the current node without returning the previous node (private).
  ASMJIT_INLINE void _setCursor(BaseNode* node) { _cursor = node; }
  //! @brief Set the current node to @a node and return the previous one.
  ASMJIT_API BaseNode* setCursor(BaseNode* node);

  //! @brief Add node @a node after current and set current to @a node.
  ASMJIT_API BaseNode* addNode(BaseNode* node);
  //! @brief Add node before @a ref.
  ASMJIT_API BaseNode* addNodeBefore(BaseNode* node, BaseNode* ref);
  //! @brief Add node after @a ref.
  ASMJIT_API BaseNode* addNodeAfter(BaseNode* node, BaseNode* ref);
  //! @brief Remove node @a node.
  ASMJIT_API BaseNode* removeNode(BaseNode* node);
  //! @brief Remove multiple nodes.
  ASMJIT_API void removeNodes(BaseNode* first, BaseNode* last);

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! @brief Get current function.
  ASMJIT_INLINE FuncNode* getFunc() const { return _func; }

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref AlignNode.
  ASMJIT_API AlignNode* newAlign(uint32_t m);
  //! @brief Add a new @ref AlignNode.
  ASMJIT_API AlignNode* addAlign(uint32_t m);

  //! @brief Align target buffer to @a m bytes.
  //!
  //! Typical usage of this is to align labels at start of the inner loops.
  //!
  //! Inserts @c nop() instructions or CPU optimized NOPs.
  ASMJIT_INLINE AlignNode* align(uint32_t m) { return addAlign(m); }

  // --------------------------------------------------------------------------
  // [Target]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref TargetNode.
  ASMJIT_API TargetNode* newTarget();
  //! @brief Add a new @ref TargetNode.
  ASMJIT_API TargetNode* addTarget();

  //! @brief Get @ref TargetNode by @a id.
  ASMJIT_INLINE TargetNode* getTargetById(uint32_t id) {
    ASMJIT_ASSERT(OperandUtil::isLabelId(id));
    ASMJIT_ASSERT(id < _targets.getLength());

    return _targets[id];
  }

  //! @brief Get @ref TargetNode by @a label.
  ASMJIT_INLINE TargetNode* getTarget(const Label& label) {
    return getTargetById(label.getId());
  }

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! @brief Get count of created labels.
  ASMJIT_INLINE size_t getLabelsCount() const
  { return _targets.getLength(); }

  //! @brief Get whether @a label is created.
  ASMJIT_INLINE bool isLabelCreated(const Label& label) const
  { return static_cast<size_t>(label.getId()) < _targets.getLength(); }

  //! @internal
  //!
  //! @brief Create and initialize a new label.
  ASMJIT_API Error _newLabel(Label* dst);

  //! @brief Create and return new label.
  ASMJIT_INLINE Label newLabel() {
    Label result(DontInitialize);
    _newLabel(&result);
    return result;
  }

  //! @brief Bind label to the current offset.
  //!
  //! @note Label can be bound only once!
  ASMJIT_API void bind(const Label& label);

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref EmbedNode.
  ASMJIT_API EmbedNode* newEmbed(const void* data, uint32_t size);
  //! @brief Add a new @ref EmbedNode.
  ASMJIT_API EmbedNode* addEmbed(const void* data, uint32_t size);

  //! @brief Embed data.
  ASMJIT_INLINE EmbedNode* embed(const void* data, uint32_t size) { return addEmbed(data, size); }

  // --------------------------------------------------------------------------
  // [Comment]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref CommentNode.
  ASMJIT_API CommentNode* newComment(const char* str);
  //! @brief Add a new @ref CommentNode.
  ASMJIT_API CommentNode* addComment(const char* str);

  //! @brief Emit a single comment line.
  ASMJIT_API CommentNode* comment(const char* fmt, ...);

  // --------------------------------------------------------------------------
  // [Hint]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref HintNode.
  ASMJIT_API HintNode* newHint(BaseVar& var, uint32_t hint, uint32_t value);
  //! @brief Add a new @ref HintNode.
  ASMJIT_API HintNode* addHint(BaseVar& var, uint32_t hint, uint32_t value);

  // --------------------------------------------------------------------------
  // [Vars]
  // --------------------------------------------------------------------------

  //! @brief Get whether variable @a var is created.
  ASMJIT_INLINE bool isVarCreated(const BaseVar& var) const {
    return static_cast<size_t>(var.getId() & kOperandIdNum) < _vars.getLength();
  }

  //! @internal
  //!
  //! @brief Get @ref VarData by @a var.
  ASMJIT_INLINE VarData* getVd(const BaseVar& var) const {
    return getVdById(var.getId());
  }

  //! @internal
  //!
  //! @brief Get @ref VarData by @a id.
  ASMJIT_INLINE VarData* getVdById(uint32_t id) const {
    ASMJIT_ASSERT(id != kInvalidValue);
    ASMJIT_ASSERT(static_cast<size_t>(id & kOperandIdNum) < _vars.getLength());

    return _vars[id & kOperandIdNum];
  }

  //! @internal
  //!
  //! @brief Get an array of 'VarData*'.
  ASMJIT_INLINE VarData** _getVdArray() const {
    return const_cast<VarData**>(_vars.getData());
  }

  //! @internal
  //!
  //! @brief Create a new @ref VarData.
  ASMJIT_API VarData* _newVd(uint32_t type, uint32_t size, uint32_t c, const char* name);

  //! @brief Create a new @ref BaseVar.
  virtual Error _newVar(BaseVar* var, uint32_t type, const char* name) = 0;

  //! @brief Alloc variable @a var.
  ASMJIT_API void alloc(BaseVar& var);
  //! @brief Alloc variable @a var using @a regIndex as a register index.
  ASMJIT_API void alloc(BaseVar& var, uint32_t regIndex);
  //! @brief Alloc variable @a var using @a reg as a demanded register.
  ASMJIT_API void alloc(BaseVar& var, const BaseReg& reg);
  //! @brief Spill variable @a var.
  ASMJIT_API void spill(BaseVar& var);
  //! @brief Save variable @a var if modified.
  ASMJIT_API void save(BaseVar& var);
  //! @brief Unuse variable @a var.
  ASMJIT_API void unuse(BaseVar& var);

  //! @brief Get priority of variable @a var.
  ASMJIT_API uint32_t getPriority(BaseVar& var) const;
  //! @brief Set priority of variable @a var to @a priority.
  ASMJIT_API void setPriority(BaseVar& var, uint32_t priority);

  //! @brief Get save-on-unuse @a var property.
  ASMJIT_API bool getSaveOnUnuse(BaseVar& var) const;
  //! @brief Set save-on-unuse @a var property to @a value.
  ASMJIT_API void setSaveOnUnuse(BaseVar& var, bool value);

  //! @brief Rename variable @a var to @a name.
  //!
  //! @note Only new name will appear in the logger.
  ASMJIT_API void rename(BaseVar& var, const char* name);

  // --------------------------------------------------------------------------
  // [Stack]
  // --------------------------------------------------------------------------

  //! @brief Create a new @ref BaseMem.
  virtual Error _newStack(BaseMem* mem, uint32_t size, uint32_t alignment, const char* name) = 0;

  // --------------------------------------------------------------------------
  // [Serialize]
  // --------------------------------------------------------------------------

  //! @brief Send assembled code to @a assembler.
  virtual Error serialize(BaseAssembler& assembler) = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Flow id added to each node created (used only by @ref Context).
  uint32_t _nodeFlowId;
  //! @brief Flags added to each node created (used only by @ref Context).
  uint32_t _nodeFlags;
  //! @brief Maximum count of nodes to look ahead when allocating/spilling
  //! registers.
  uint32_t _maxLookAhead;

  //! @brief Variable mapping (translates incoming kVarType into target).
  const uint8_t* _targetVarMapping;

  //! @brief First node.
  BaseNode* _firstNode;
  //! @brief Last node.
  BaseNode* _lastNode;

  //! @brief Current node.
  BaseNode* _cursor;
  //! @brief Current function.
  FuncNode* _func;

  //! @brief Variable allocator.
  Zone _varAllocator;
  //! @brief String/data allocator.
  Zone _stringAllocator;

  //! @brief Targets.
  PodVector<TargetNode*> _targets;
  //! @brief Variables.
  PodVector<VarData*> _vars;
};

// ============================================================================
// [Defined-Later]
// ============================================================================

ASMJIT_INLINE Label::Label(BaseCompiler& c) : Operand(DontInitialize) {
  c._newLabel(this);
}

ASMJIT_INLINE BaseNode::BaseNode(BaseCompiler* compiler, uint32_t type) {
  _prev = NULL;
  _next = NULL;
  _type = static_cast<uint8_t>(type);
  _opCount = 0;
  _flags = static_cast<uint16_t>(compiler->_nodeFlags);
  _flowId = compiler->_nodeFlowId;
  _comment = NULL;
  _varInst = NULL;
  _liveness = NULL;
  _state = NULL;
}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_COMPILER_H
