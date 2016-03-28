// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_HLSTREAM_H
#define _ASMJIT_BASE_HLSTREAM_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/operand.h"

// TODO: Cannot depend on it.
#include "../base/compilerfunc.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Compiler;
struct VarData;
struct VarState;
struct VarMap;

struct HLInst;
struct HLJump;
struct HLLabel;
struct HLSentinel;

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::HLNode]
// ============================================================================

//! Base node (HL).
//!
//! Every node represents an abstract instruction, directive, label, or macro
//! instruction that can be serialized to `Assembler`.
struct HLNode {
  ASMJIT_NO_COPY(HLNode)

  // --------------------------------------------------------------------------
  // [Type]
  // --------------------------------------------------------------------------

  //! Type of \ref HLNode.
  ASMJIT_ENUM(Type) {
    //! Invalid node (internal, don't use).
    kTypeNone = 0,

    // --------------------------------------------------------------------------
    // [Low-Level - Assembler / Compiler]
    // --------------------------------------------------------------------------

    //! Node is \ref HLInst or \ref HLJump.
    kTypeInst,
    //! Node is \ref HLData.
    kTypeData,
    //! Node is \ref HLAlign.
    kTypeAlign,
    //! Node is \ref HLLabel.
    kTypeLabel,
    //! Node is \ref HLComment.
    kTypeComment,
    //! Node is \ref HLSentinel.
    kTypeSentinel,

    // --------------------------------------------------------------------------
    // [High-Level - Compiler-Only]
    // --------------------------------------------------------------------------

    //! Node is \ref HLHint.
    kTypeHint,
    //! Node is \ref HLFunc.
    kTypeFunc,
    //! Node is \ref HLRet.
    kTypeRet,
    //! Node is \ref HLCall.
    kTypeCall,
    //! Node is \ref HLCallArg.
    kTypeCallArg
  };

  // --------------------------------------------------------------------------
  // [Flags]
  // --------------------------------------------------------------------------

  ASMJIT_ENUM(Flags) {
    //! Whether the node has been translated, thus contains only registers.
    kFlagIsTranslated = 0x0001,

    //! Whether the node was scheduled - possibly reordered, but basically this
    //! is a mark that is set by scheduler after the node has been visited.
    kFlagIsScheduled = 0x0002,

    //! Whether the node can be safely removed by the `Compiler` in case it's
    //! unreachable.
    kFlagIsRemovable = 0x0004,

    //! Whether the node is informative only and can be safely removed.
    kFlagIsInformative = 0x0008,

    //! Whether the `HLInst` is a jump.
    kFlagIsJmp = 0x0010,
    //! Whether the `HLInst` is a conditional jump.
    kFlagIsJcc = 0x0020,

    //! Whether the `HLInst` is an unconditinal jump or conditional jump that is
    //! likely to be taken.
    kFlagIsTaken = 0x0040,

    //! Whether the `HLNode` will return from a function.
    //!
    //! This flag is used by both `HLSentinel` and `HLRet`.
    kFlagIsRet = 0x0080,

    //! Whether the instruction is special.
    kFlagIsSpecial = 0x0100,

    //! Whether the instruction is an FPU instruction.
    kFlagIsFp = 0x0200
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLNode`.
  //!
  //! \note Always use compiler to create nodes.
  ASMJIT_INLINE HLNode(Compiler* compiler, uint32_t type) noexcept; // Defined-Later.

  //! Destroy the `HLNode`.
  //!
  //! NOTE: Nodes are zone allocated, there should be no code in the destructor.
  ASMJIT_INLINE ~HLNode() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors - List]
  // --------------------------------------------------------------------------

  //! Get previous node in the compiler stream.
  ASMJIT_INLINE HLNode* getPrev() const noexcept { return _prev; }
  //! Get next node in the compiler stream.
  ASMJIT_INLINE HLNode* getNext() const noexcept { return _next; }

  // --------------------------------------------------------------------------
  // [Accessors - Comment]
  // --------------------------------------------------------------------------

  //! Get an inline comment string.
  ASMJIT_INLINE const char* getComment() const noexcept { return _comment; }
  //! Set an inline comment string to `comment`.
  ASMJIT_INLINE void setComment(const char* comment) noexcept { _comment = comment; }

  // --------------------------------------------------------------------------
  // [Accessors - Type and Flags]
  // --------------------------------------------------------------------------

  //! Get the node type, see \ref Type.
  ASMJIT_INLINE uint32_t getType() const noexcept { return _type; }
  //! Get the node flags.
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }

  //! Get whether the instruction has flag `flag`.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) const noexcept { return (static_cast<uint32_t>(_flags) & flag) != 0; }
  //! Set node flags to `flags`.
  ASMJIT_INLINE void setFlags(uint32_t flags) noexcept { _flags = static_cast<uint16_t>(flags); }
  //! Add instruction `flags`.
  ASMJIT_INLINE void orFlags(uint32_t flags) noexcept { _flags |= static_cast<uint16_t>(flags); }
  //! And instruction `flags`.
  ASMJIT_INLINE void andFlags(uint32_t flags) noexcept { _flags &= static_cast<uint16_t>(flags); }
  //! Clear instruction `flags`.
  ASMJIT_INLINE void andNotFlags(uint32_t flags) noexcept { _flags &= ~static_cast<uint16_t>(flags); }

  //! Get whether the node has beed fetched.
  ASMJIT_INLINE bool isFetched() const noexcept { return _flowId != 0; }
  //! Get whether the node has been translated.
  ASMJIT_INLINE bool isTranslated() const noexcept { return hasFlag(kFlagIsTranslated); }
  //! Get whether the node has been translated.
  ASMJIT_INLINE bool isScheduled() const noexcept { return hasFlag(kFlagIsScheduled); }

  //! Get whether the node is removable if it's in unreachable code block.
  ASMJIT_INLINE bool isRemovable() const noexcept { return hasFlag(kFlagIsRemovable); }
  //! Get whether the node is informative only (comment, hint).
  ASMJIT_INLINE bool isInformative() const noexcept { return hasFlag(kFlagIsInformative); }

  //! Whether the node is `HLLabel`.
  ASMJIT_INLINE bool isLabel() const noexcept { return _type == kTypeLabel; }
  //! Whether the `HLInst` node is an unconditional jump.
  ASMJIT_INLINE bool isJmp() const noexcept { return hasFlag(kFlagIsJmp); }
  //! Whether the `HLInst` node is a conditional jump.
  ASMJIT_INLINE bool isJcc() const noexcept { return hasFlag(kFlagIsJcc); }
  //! Whether the `HLInst` node is a conditional/unconditional jump.
  ASMJIT_INLINE bool isJmpOrJcc() const noexcept { return hasFlag(kFlagIsJmp | kFlagIsJcc); }
  //! Whether the `HLInst` node is a return.
  ASMJIT_INLINE bool isRet() const noexcept { return hasFlag(kFlagIsRet); }

  //! Get whether the node is `HLInst` and the instruction is special.
  ASMJIT_INLINE bool isSpecial() const noexcept { return hasFlag(kFlagIsSpecial); }
  //! Get whether the node is `HLInst` and the instruction uses x87-FPU.
  ASMJIT_INLINE bool isFp() const noexcept { return hasFlag(kFlagIsFp); }

  // --------------------------------------------------------------------------
  // [Accessors - FlowId]
  // --------------------------------------------------------------------------

  //! Get flow index.
  ASMJIT_INLINE uint32_t getFlowId() const noexcept { return _flowId; }
  //! Set flow index.
  ASMJIT_INLINE void setFlowId(uint32_t flowId) noexcept { _flowId = flowId; }

  // --------------------------------------------------------------------------
  // [Accessors - TokenId]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool hasTokenId(uint32_t id) const noexcept { return _tokenId == id; }
  ASMJIT_INLINE uint32_t getTokenId() const noexcept { return _tokenId; }
  ASMJIT_INLINE void setTokenId(uint32_t id) noexcept { _tokenId = id; }

  // --------------------------------------------------------------------------
  // [Accessors - VarMap]
  // --------------------------------------------------------------------------

  //! Get whether node contains variable allocation instructions.
  ASMJIT_INLINE bool hasMap() const noexcept { return _map != nullptr; }
  //! Get variable allocation instructions.
  ASMJIT_INLINE VarMap* getMap() const noexcept { return _map; }
  //! Get variable allocation instructions casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* getMap() const noexcept { return static_cast<T*>(_map); }
  //! Set variable allocation instructions.
  ASMJIT_INLINE void setMap(VarMap* map) noexcept { _map = map; }

  // --------------------------------------------------------------------------
  // [Accessors - VarState]
  // --------------------------------------------------------------------------

  //! Get whether the node has an associated `VarState`.
  ASMJIT_INLINE bool hasState() const noexcept { return _state != nullptr; }
  //! Get node state.
  ASMJIT_INLINE VarState* getState() const noexcept { return _state; }
  //! Get node state casted to `T*`.
  template<typename T>
  ASMJIT_INLINE T* getState() const noexcept { return static_cast<T*>(_state); }
  //! Set node state.
  ASMJIT_INLINE void setState(VarState* state) noexcept { _state = state; }

  // --------------------------------------------------------------------------
  // [Accessors - Liveness]
  // --------------------------------------------------------------------------

  //! Get whether the node has variable liveness bits.
  ASMJIT_INLINE bool hasLiveness() const noexcept { return _liveness != nullptr; }
  //! Get variable liveness bits.
  ASMJIT_INLINE BitArray* getLiveness() const noexcept { return _liveness; }
  //! Set variable liveness bits.
  ASMJIT_INLINE void setLiveness(BitArray* liveness) noexcept { _liveness = liveness; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Previous node.
  HLNode* _prev;
  //! Next node.
  HLNode* _next;

  //! Node type, see \ref Type.
  uint8_t _type;
  //! Count of operands (if the node has operands, otherwise zero).
  uint8_t _opCount;
  //! Flags, different meaning for every type of the node.
  uint16_t _flags;

  //! Flow index.
  uint32_t _flowId;

  //! Processing token ID.
  //!
  //! Used by some algorithms to mark nodes as visited. If the token is
  //! generated in an incrementing way the visitor can just mark nodes it
  //! visits and them compare the `HLNode`s token with it's local token.
  //! If they match the node has been visited already. Then the visitor
  //! doesn't need to clean things up as the next time the token will be
  //! different.
  uint32_t _tokenId;

  // TODO: 32-bit gap

  //! Inline comment string, initially set to nullptr.
  const char* _comment;

  //! Variable mapping (VarAttr to VarData), initially nullptr, filled during
  //! fetch phase.
  VarMap* _map;

  //! Variable liveness bits (initially nullptr, filled by analysis phase).
  BitArray* _liveness;

  //! Saved state.
  //!
  //! Initially nullptr, not all nodes have saved state, only branch/flow control
  //! nodes.
  VarState* _state;
};

// ============================================================================
// [asmjit::HLInst]
// ============================================================================

//! Instruction (HL).
//!
//! Wraps an instruction with its options and operands.
struct HLInst : public HLNode {
  ASMJIT_NO_COPY(HLInst)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLInst` instance.
  ASMJIT_INLINE HLInst(Compiler* compiler, uint32_t instId, uint32_t instOptions, Operand* opList, uint32_t opCount) noexcept
    : HLNode(compiler, kTypeInst) {

    orFlags(kFlagIsRemovable);
    _instId = static_cast<uint16_t>(instId);
    _reserved = 0;
    _instOptions = instOptions;

    _opCount = static_cast<uint8_t>(opCount);
    _opList = opList;

    _updateMemOp();
  }

  //! Destroy the `HLInst` instance.
  ASMJIT_INLINE ~HLInst() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the instruction id, see `X86InstId`.
  ASMJIT_INLINE uint32_t getInstId() const noexcept { return _instId; }
  //! Set the instruction id to `instId`.
  //!
  //! NOTE: Please do not modify instruction code if you don't know what you
  //! are doing. Incorrect instruction code and/or operands can cause random
  //! errors in production builds and will most probably trigger assertion
  //! failures in debug builds.
  ASMJIT_INLINE void setInstId(uint32_t instId) noexcept { _instId = static_cast<uint16_t>(instId); }

  //! Whether the instruction is either a jump or a conditional jump likely to
  //! be taken.
  ASMJIT_INLINE bool isTaken() const noexcept { return hasFlag(kFlagIsTaken); }

  //! Get emit options.
  ASMJIT_INLINE uint32_t getOptions() const noexcept { return _instOptions; }
  //! Set emit options.
  ASMJIT_INLINE void setOptions(uint32_t options) noexcept { _instOptions = options; }
  //! Add emit options.
  ASMJIT_INLINE void addOptions(uint32_t options) noexcept { _instOptions |= options; }
  //! Mask emit options.
  ASMJIT_INLINE void andOptions(uint32_t options) noexcept { _instOptions &= options; }
  //! Clear emit options.
  ASMJIT_INLINE void delOptions(uint32_t options) noexcept { _instOptions &= ~options; }

  //! Get operands count.
  ASMJIT_INLINE uint32_t getOpCount() const noexcept { return _opCount; }
  //! Get operands list.
  ASMJIT_INLINE Operand* getOpList() noexcept { return _opList; }
  //! \overload
  ASMJIT_INLINE const Operand* getOpList() const noexcept { return _opList; }

  //! Get whether the instruction contains a memory operand.
  ASMJIT_INLINE bool hasMemOp() const noexcept { return _memOpIndex != 0xFF; }
  //! Get memory operand.
  //!
  //! NOTE: Can only be called if the instruction has such operand,
  //! see `hasMemOp()`.
  ASMJIT_INLINE BaseMem* getMemOp() const noexcept {
    ASMJIT_ASSERT(hasMemOp());
    return static_cast<BaseMem*>(&_opList[_memOpIndex]);
  }
  //! \overload
  template<typename T>
  ASMJIT_INLINE T* getMemOp() const noexcept {
    ASMJIT_ASSERT(hasMemOp());
    return static_cast<T*>(&_opList[_memOpIndex]);
  }

  //! Set memory operand index, `0xFF` means no memory operand.
  ASMJIT_INLINE void setMemOpIndex(uint32_t index) noexcept { _memOpIndex = static_cast<uint8_t>(index); }
  //! Reset memory operand index to `0xFF` (no operand).
  ASMJIT_INLINE void resetMemOpIndex() noexcept { _memOpIndex = 0xFF; }

  // --------------------------------------------------------------------------
  // [Utils]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void _updateMemOp() noexcept {
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

  //! Instruction ID, see `InstId`.
  uint16_t _instId;
  //! \internal
  uint8_t _memOpIndex;
  //! \internal
  uint8_t _reserved;
  //! Instruction options, see `InstOptions`.
  uint32_t _instOptions;

  //! Operands list.
  Operand* _opList;
};

// ============================================================================
// [asmjit::HLJump]
// ============================================================================

//! Conditional or direct jump (HL).
//!
//! Extension of `HLInst` node, which stores more information about the jump.
struct HLJump : public HLInst {
  ASMJIT_NO_COPY(HLJump)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE HLJump(Compiler* compiler, uint32_t code, uint32_t options, Operand* opList, uint32_t opCount) noexcept
    : HLInst(compiler, code, options, opList, opCount),
      _target(nullptr),
      _jumpNext(nullptr) {}
  ASMJIT_INLINE ~HLJump() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE HLLabel* getTarget() const noexcept { return _target; }
  ASMJIT_INLINE HLJump* getJumpNext() const noexcept { return _jumpNext; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Target node.
  HLLabel* _target;
  //! Next jump to the same target in a single linked-list.
  HLJump* _jumpNext;
};

// ============================================================================
// [asmjit::HLData]
// ============================================================================

//! Data (HL).
//!
//! Wraps `.data` directive. The node contains data that will be placed at the
//! node's position in the assembler stream. The data is considered to be RAW;
//! no analysis nor byte-order conversion is performed on RAW data.
struct HLData : public HLNode {
  ASMJIT_NO_COPY(HLData)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  enum { kInlineBufferSize = 12 };

  //! Create a new `HLData` instance.
  ASMJIT_INLINE HLData(Compiler* compiler, void* data, uint32_t size) noexcept
    : HLNode(compiler, kTypeData) {

    _size = size;
    if (size <= kInlineBufferSize) {
      if (data != nullptr)
        ::memcpy(_data.buf, data, size);
    }
    else {
      _data.ptr = static_cast<uint8_t*>(data);
    }
  }

  //! Destroy the `HLData` instance.
  ASMJIT_INLINE ~HLData() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get size of the data.
  uint32_t getSize() const noexcept { return _size; }
  //! Get pointer to the data.
  uint8_t* getData() const noexcept { return _size <= kInlineBufferSize ? const_cast<uint8_t*>(_data.buf) : _data.ptr; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    //! data buffer.
    uint8_t buf[kInlineBufferSize];
    //! Data buffer.
    uint8_t* ptr;
  } _data;

  //! Size of the data.
  uint32_t _size;
};

// ============================================================================
// [asmjit::HLAlign]
// ============================================================================

//! Align directive (HL).
//!
//! Wraps `.align` directive.
struct HLAlign : public HLNode {
  ASMJIT_NO_COPY(HLAlign)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLAlign` instance.
  ASMJIT_INLINE HLAlign(Compiler* compiler, uint32_t alignMode, uint32_t offset) noexcept
    : HLNode(compiler, kTypeAlign) {

    _alignMode = alignMode;
    _offset = offset;
  }

  //! Destroy the `HLAlign` instance.
  ASMJIT_INLINE ~HLAlign() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get align mode.
  ASMJIT_INLINE uint32_t getAlignMode() const noexcept { return _alignMode; }
  //! Set align mode.
  ASMJIT_INLINE void setAlignMode(uint32_t alignMode) noexcept { _alignMode = alignMode; }

  //! Get align offset in bytes.
  ASMJIT_INLINE uint32_t getOffset() const noexcept { return _offset; }
  //! Set align offset in bytes to `offset`.
  ASMJIT_INLINE void setOffset(uint32_t offset) noexcept { _offset = offset; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Align mode, see \ref AlignMode.
  uint32_t _alignMode;
  //! Align offset (in bytes).
  uint32_t _offset;
};

// ============================================================================
// [asmjit::HLLabel]
// ============================================================================

//! label (HL).
struct HLLabel : public HLNode {
  ASMJIT_NO_COPY(HLLabel)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLLabel` instance.
  ASMJIT_INLINE HLLabel(Compiler* compiler, uint32_t labelId) noexcept
    : HLNode(compiler, kTypeLabel) {

    _id = labelId;
    _numRefs = 0;
    _from = nullptr;
  }

  //! Destroy the `HLLabel` instance.
  ASMJIT_INLINE ~HLLabel() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get target label.
  ASMJIT_INLINE Label getLabel() const noexcept { return Label(_id); }
  //! Get target label id.
  ASMJIT_INLINE uint32_t getLabelId() const noexcept { return _id; }

  //! Get first jmp instruction.
  ASMJIT_INLINE HLJump* getFrom() const noexcept { return _from; }

  //! Get whether the node has assigned state.
  ASMJIT_INLINE bool hasState() const noexcept { return _state != nullptr; }
  //! Get state for this target.
  ASMJIT_INLINE VarState* getState() const noexcept { return _state; }
  //! Set state for this target.
  ASMJIT_INLINE void setState(VarState* state) noexcept { _state = state; }

  //! Get number of jumps to this target.
  ASMJIT_INLINE uint32_t getNumRefs() const noexcept { return _numRefs; }
  //! Set number of jumps to this target.
  ASMJIT_INLINE void setNumRefs(uint32_t i) noexcept { _numRefs = i; }

  //! Add number of jumps to this target.
  ASMJIT_INLINE void addNumRefs(uint32_t i = 1) noexcept { _numRefs += i; }
  //! Subtract number of jumps to this target.
  ASMJIT_INLINE void subNumRefs(uint32_t i = 1) noexcept { _numRefs -= i; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Label id.
  uint32_t _id;
  //! Count of jumps here.
  uint32_t _numRefs;

  //! First jump instruction that points to this target (label).
  HLJump* _from;
};

// ============================================================================
// [asmjit::HLComment]
// ============================================================================

//! Comment (HL).
struct HLComment : public HLNode {
  ASMJIT_NO_COPY(HLComment)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLComment` instance.
  ASMJIT_INLINE HLComment(Compiler* compiler, const char* comment) noexcept
    : HLNode(compiler, kTypeComment) {

    orFlags(kFlagIsRemovable | kFlagIsInformative);
    _comment = comment;
  }

  //! Destroy the `HLComment` instance.
  ASMJIT_INLINE ~HLComment() noexcept {}
};

// ============================================================================
// [asmjit::HLSentinel]
// ============================================================================

//! Sentinel (HL).
struct HLSentinel : public HLNode {
  ASMJIT_NO_COPY(HLSentinel)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLSentinel` instance.
  ASMJIT_INLINE HLSentinel(Compiler* compiler) noexcept
    : HLNode(compiler, kTypeSentinel) {
    orFlags(kFlagIsRet);
  }

  //! Destroy the `HLSentinel` instance.
  ASMJIT_INLINE ~HLSentinel() noexcept {}
};

// ============================================================================
// [asmjit::HLHint]
// ============================================================================

//! Hint node.
struct HLHint : public HLNode {
  ASMJIT_NO_COPY(HLHint)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLHint` instance.
  ASMJIT_INLINE HLHint(Compiler* compiler, VarData* vd, uint32_t hint, uint32_t value) noexcept
    : HLNode(compiler, kTypeHint) {

    orFlags(kFlagIsRemovable | kFlagIsInformative);
    _vd = vd;
    _hint = hint;
    _value = value;
  }

  //! Destroy the `HLHint` instance.
  ASMJIT_INLINE ~HLHint() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get variable.
  ASMJIT_INLINE VarData* getVd() const noexcept { return _vd; }

  //! Get hint it (see `kVarHint)`.
  ASMJIT_INLINE uint32_t getHint() const noexcept { return _hint; }
  //! Set hint it (see `kVarHint)`.
  ASMJIT_INLINE void setHint(uint32_t hint) noexcept { _hint = hint; }

  //! Get hint value.
  ASMJIT_INLINE uint32_t getValue() const noexcept { return _value; }
  //! Set hint value.
  ASMJIT_INLINE void setValue(uint32_t value) noexcept { _value = value; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Variable.
  VarData* _vd;
  //! Hint id.
  uint32_t _hint;
  //! Value.
  uint32_t _value;
};

// ============================================================================
// [asmjit::HLFunc]
// ============================================================================

//! Function (HL).
struct HLFunc : public HLNode {
  ASMJIT_NO_COPY(HLFunc)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLFunc` instance.
  //!
  //! Always use `Compiler::addFunc()` to create an `HLFunc` instance.
  ASMJIT_INLINE HLFunc(Compiler* compiler) noexcept
    : HLNode(compiler, kTypeFunc),
      _entryNode(nullptr),
      _exitNode(nullptr),
      _decl(nullptr),
      _end(nullptr),
      _args(nullptr),
      _funcHints(Utils::mask(kFuncHintNaked)),
      _funcFlags(0),
      _expectedStackAlignment(0),
      _requiredStackAlignment(0),
      _redZoneSize(0),
      _spillZoneSize(0),
      _argStackSize(0),
      _memStackSize(0),
      _callStackSize(0) {}

  //! Destroy the `HLFunc` instance.
  ASMJIT_INLINE ~HLFunc() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get function entry `HLLabel`.
  ASMJIT_INLINE HLLabel* getEntryNode() const noexcept { return _entryNode; }
  //! Get function exit `HLLabel`.
  ASMJIT_INLINE HLLabel* getExitNode() const noexcept { return _exitNode; }

  //! Get function entry label.
  ASMJIT_INLINE Label getEntryLabel() const noexcept { return _entryNode->getLabel(); }
  //! Get function exit label.
  ASMJIT_INLINE Label getExitLabel() const noexcept { return _exitNode->getLabel(); }

  //! Get the function end sentinel.
  ASMJIT_INLINE HLSentinel* getEnd() const noexcept { return _end; }
  //! Get function declaration.
  ASMJIT_INLINE FuncDecl* getDecl() const noexcept { return _decl; }

  //! Get arguments count.
  ASMJIT_INLINE uint32_t getNumArgs() const noexcept { return _decl->getNumArgs(); }
  //! Get arguments list.
  ASMJIT_INLINE VarData** getArgs() const noexcept { return _args; }

  //! Get argument at `i`.
  ASMJIT_INLINE VarData* getArg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < getNumArgs());
    return _args[i];
  }

  //! Set argument at `i`.
  ASMJIT_INLINE void setArg(uint32_t i, VarData* vd) noexcept {
    ASMJIT_ASSERT(i < getNumArgs());
    _args[i] = vd;
  }

  //! Reset argument at `i`.
  ASMJIT_INLINE void resetArg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < getNumArgs());
    _args[i] = nullptr;
  }

  //! Get function hints.
  ASMJIT_INLINE uint32_t getFuncHints() const noexcept { return _funcHints; }
  //! Get function flags.
  ASMJIT_INLINE uint32_t getFuncFlags() const noexcept { return _funcFlags; }

  //! Get whether the _funcFlags has `flag`
  ASMJIT_INLINE bool hasFuncFlag(uint32_t flag) const noexcept { return (_funcFlags & flag) != 0; }
  //! Set function `flag`.
  ASMJIT_INLINE void addFuncFlags(uint32_t flags) noexcept { _funcFlags |= flags; }
  //! Clear function `flag`.
  ASMJIT_INLINE void clearFuncFlags(uint32_t flags) noexcept { _funcFlags &= ~flags; }

  //! Get whether the function is naked.
  ASMJIT_INLINE bool isNaked() const noexcept { return hasFuncFlag(kFuncFlagIsNaked); }
  //! Get whether the function is also a caller.
  ASMJIT_INLINE bool isCaller() const noexcept { return hasFuncFlag(kFuncFlagIsCaller); }
  //! Get whether the required stack alignment is lower than expected one,
  //! thus it has to be aligned manually.
  ASMJIT_INLINE bool isStackMisaligned() const noexcept { return hasFuncFlag(kFuncFlagIsStackMisaligned); }
  //! Get whether the stack pointer is adjusted inside function prolog/epilog.
  ASMJIT_INLINE bool isStackAdjusted() const noexcept { return hasFuncFlag(kFuncFlagIsStackAdjusted); }

  //! Get whether the function is finished.
  ASMJIT_INLINE bool isFinished() const noexcept { return hasFuncFlag(kFuncFlagIsFinished); }

  //! Get expected stack alignment.
  ASMJIT_INLINE uint32_t getExpectedStackAlignment() const noexcept {
    return _expectedStackAlignment;
  }

  //! Set expected stack alignment.
  ASMJIT_INLINE void setExpectedStackAlignment(uint32_t alignment) noexcept {
    _expectedStackAlignment = alignment;
  }

  //! Get required stack alignment.
  ASMJIT_INLINE uint32_t getRequiredStackAlignment() const noexcept {
    return _requiredStackAlignment;
  }

  //! Set required stack alignment.
  ASMJIT_INLINE void setRequiredStackAlignment(uint32_t alignment) noexcept {
    _requiredStackAlignment = alignment;
  }

  //! Update required stack alignment so it's not lower than expected
  //! stack alignment.
  ASMJIT_INLINE void updateRequiredStackAlignment() noexcept {
    if (_requiredStackAlignment <= _expectedStackAlignment) {
      _requiredStackAlignment = _expectedStackAlignment;
      clearFuncFlags(kFuncFlagIsStackMisaligned);
    }
    else {
      addFuncFlags(kFuncFlagIsStackMisaligned);
    }
  }

  //! Set stack "Red Zone" size.
  ASMJIT_INLINE uint32_t getRedZoneSize() const noexcept { return _redZoneSize; }
  //! Get stack "Red Zone" size.
  ASMJIT_INLINE void setRedZoneSize(uint32_t s) noexcept { _redZoneSize = static_cast<uint16_t>(s); }

  //! Set stack "Spill Zone" size.
  ASMJIT_INLINE uint32_t getSpillZoneSize() const noexcept { return _spillZoneSize; }
  //! Get stack "Spill Zone" size.
  ASMJIT_INLINE void setSpillZoneSize(uint32_t s) noexcept { _spillZoneSize = static_cast<uint16_t>(s); }

  //! Get stack size used by function arguments.
  ASMJIT_INLINE uint32_t getArgStackSize() const noexcept { return _argStackSize; }

  //! Get stack size used by variables and memory allocated on the stack.
  ASMJIT_INLINE uint32_t getMemStackSize() const noexcept { return _memStackSize; }

  //! Get stack size used by function calls.
  ASMJIT_INLINE uint32_t getCallStackSize() const noexcept { return _callStackSize; }
  //! Merge stack size used by function call with `s`.
  ASMJIT_INLINE void mergeCallStackSize(uint32_t s) noexcept { if (_callStackSize < s) _callStackSize = s; }

  // --------------------------------------------------------------------------
  // [Hints]
  // --------------------------------------------------------------------------

  //! Set function hint.
  ASMJIT_INLINE void setHint(uint32_t hint, uint32_t value) noexcept {
    ASMJIT_ASSERT(hint <= 31);
    ASMJIT_ASSERT(value <= 1);

    _funcHints &= ~(1     << hint);
    _funcHints |=  (value << hint);
  }

  //! Get function hint.
  ASMJIT_INLINE uint32_t getHint(uint32_t hint) const noexcept {
    ASMJIT_ASSERT(hint <= 31);
    return (_funcHints >> hint) & 0x1;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Function entry.
  HLLabel* _entryNode;
  //! Function exit.
  HLLabel* _exitNode;

  //! Function declaration.
  FuncDecl* _decl;
  //! Function end.
  HLSentinel* _end;

  //! Arguments list as `VarData`.
  VarData** _args;

  //! Function hints;
  uint32_t _funcHints;
  //! Function flags.
  uint32_t _funcFlags;

  //! Expected stack alignment (we depend on this value).
  //!
  //! \note It can be global alignment given by the OS or described by the
  //! target platform ABI.
  uint32_t _expectedStackAlignment;
  //! Required stack alignment (required by SIMD instructions).
  uint32_t _requiredStackAlignment;

  //! The "Red Zone" size - count of bytes which might be accessed by a left
  //! function without adjusting the stack pointer (`esp` or `rsp`) (AMD64 ABI).
  uint16_t _redZoneSize;

  //! The "Spill Zone" size - count of bytes after the function return address
  //! that can be used by the function to spill variables in (WIN64 ABI).
  uint16_t _spillZoneSize;

  //! Stack size needed for function arguments.
  uint32_t _argStackSize;
  //! Stack size needed for all variables and memory allocated on the stack.
  uint32_t _memStackSize;
  //! Stack size needed to call other functions.
  uint32_t _callStackSize;
};

// ============================================================================
// [asmjit::HLRet]
// ============================================================================

//! Function return (HL).
struct HLRet : public HLNode {
  ASMJIT_NO_COPY(HLRet)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLRet` instance.
  ASMJIT_INLINE HLRet(Compiler* compiler, const Operand& o0, const Operand& o1) noexcept
    : HLNode(compiler, kTypeRet) {

    orFlags(kFlagIsRet);
    _ret[0] = o0;
    _ret[1] = o1;
  }

  //! Destroy the `HLRet` instance.
  ASMJIT_INLINE ~HLRet() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the first return operand.
  ASMJIT_INLINE Operand& getFirst() noexcept { return _ret[0]; }
  //! \overload
  ASMJIT_INLINE const Operand& getFirst() const noexcept { return _ret[0]; }

  //! Get the second return operand.
  ASMJIT_INLINE Operand& getSecond() noexcept { return _ret[1]; }
   //! \overload
  ASMJIT_INLINE const Operand& getSecond() const noexcept { return _ret[1]; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Ret operand(s).
  Operand _ret[2];
};

// ============================================================================
// [asmjit::HLCall]
// ============================================================================

//! Function call (HL).
struct HLCall : public HLNode {
  ASMJIT_NO_COPY(HLCall)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLCall` instance.
  ASMJIT_INLINE HLCall(Compiler* compiler, const Operand& target) noexcept
    : HLNode(compiler, kTypeCall),
      _decl(nullptr),
      _target(target),
      _args(nullptr) {
    orFlags(kFlagIsRemovable);
  }

  //! Destroy the `HLCall` instance.
  ASMJIT_INLINE ~HLCall() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get function declaration.
  ASMJIT_INLINE FuncDecl* getDecl() const noexcept { return _decl; }

  //! Get target operand.
  ASMJIT_INLINE Operand& getTarget() noexcept { return _target; }
  //! \overload
  ASMJIT_INLINE const Operand& getTarget() const noexcept { return _target; }

  //! Get return at `i`.
  ASMJIT_INLINE Operand& getRet(uint32_t i = 0) noexcept {
    ASMJIT_ASSERT(i < 2);
    return _ret[i];
  }
  //! \overload
  ASMJIT_INLINE const Operand& getRet(uint32_t i = 0) const noexcept {
    ASMJIT_ASSERT(i < 2);
    return _ret[i];
  }

  //! Get argument at `i`.
  ASMJIT_INLINE Operand& getArg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i];
  }
  //! \overload
  ASMJIT_INLINE const Operand& getArg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i];
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Function declaration.
  FuncDecl* _decl;

  //! Target (address of function, register, label, ...).
  Operand _target;
  //! Return.
  Operand _ret[2];
  //! Arguments.
  Operand* _args;
};

// ============================================================================
// [asmjit::HLCallArg]
// ============================================================================

//! Function call's argument (HL).
struct HLCallArg : public HLNode {
  ASMJIT_NO_COPY(HLCallArg)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `HLCallArg` instance.
  ASMJIT_INLINE HLCallArg(Compiler* compiler, HLCall* call, VarData* sVd, VarData* cVd) noexcept
    : HLNode(compiler, kTypeCallArg),
      _call(call),
      _sVd(sVd),
      _cVd(cVd),
      _args(0) {
    orFlags(kFlagIsRemovable);
  }

  //! Destroy the `HLCallArg` instance.
  ASMJIT_INLINE ~HLCallArg() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the associated function-call.
  ASMJIT_INLINE HLCall* getCall() const noexcept { return _call; }
  //! Get source variable.
  ASMJIT_INLINE VarData* getSVd() const noexcept { return _sVd; }
  //! Get conversion variable.
  ASMJIT_INLINE VarData* getCVd() const noexcept { return _cVd; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Associated `HLCall`.
  HLCall* _call;
  //! Source variable.
  VarData* _sVd;
  //! Temporary variable used for conversion (or nullptr).
  VarData* _cVd;

  //! Affected arguments bit-array.
  uint32_t _args;
};

// ============================================================================
// [asmjit::HLStream]
// ============================================================================

// TODO:

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_HLSTREAM_H
