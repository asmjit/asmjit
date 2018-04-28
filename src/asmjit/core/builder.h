// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_BUILDER_H
#define _ASMJIT_CORE_BUILDER_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/assembler.h"
#include "../core/codeholder.h"
#include "../core/constpool.h"
#include "../core/inst.h"
#include "../core/operand.h"
#include "../core/stringbuilder.h"
#include "../core/support.h"
#include "../core/zone.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [Forward Declarations]
// ============================================================================

class BaseNode;
class InstNode;
class LabelNode;
class AlignNode;
class EmbedDataNode;
class LabelDataNode;
class ConstPoolNode;
class CommentNode;
class SentinelNode;
class Pass;

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::BaseBuilder]
// ============================================================================

class ASMJIT_VIRTAPI BaseBuilder : public BaseEmitter {
public:
  ASMJIT_NONCOPYABLE(BaseBuilder)
  typedef BaseEmitter Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `BaseBuilder` instance.
  ASMJIT_API BaseBuilder() noexcept;
  //! Destroy the `BaseBuilder` instance.
  ASMJIT_API virtual ~BaseBuilder() noexcept;

  // --------------------------------------------------------------------------
  // [Node Management]
  // --------------------------------------------------------------------------

  //! Get the first node.
  inline BaseNode* firstNode() const noexcept { return _firstNode; }
  //! Get the last node.
  inline BaseNode* lastNode() const noexcept { return _lastNode; }

  //! \internal
  template<typename T>
  inline T* newNodeT() noexcept {
    return _allocator.newT<T>(this);
  }

  //! \internal
  template<typename T, typename... ArgsT>
  inline T* newNodeT(ArgsT&&... args) noexcept {
    return _allocator.newT<T>(this, std::forward<ArgsT>(args)...);
  }

  //! Create a new `LabelNode`.
  ASMJIT_API LabelNode* newLabelNode() noexcept;
  //! Create a new `AlignNode`.
  ASMJIT_API AlignNode* newAlignNode(uint32_t alignMode, uint32_t alignment) noexcept;
  //! Create a new `EmbedDataNode`.
  ASMJIT_API EmbedDataNode* newEmbedDataNode(const void* data, uint32_t size) noexcept;
  //! Create a new `ConstPoolNode`.
  ASMJIT_API ConstPoolNode* newConstPoolNode() noexcept;
  //! Create a new `CommentNode`.
  ASMJIT_API CommentNode* newCommentNode(const char* data, size_t size) noexcept;

  ASMJIT_API InstNode* newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0) noexcept;
  ASMJIT_API InstNode* newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1) noexcept;
  ASMJIT_API InstNode* newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept;
  ASMJIT_API InstNode* newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept;
  ASMJIT_API InstNode* newInstNodeRaw(uint32_t instId, uint32_t instOptions, uint32_t opCount) noexcept;

  //! Add `node` after the current and set current to `node`.
  ASMJIT_API BaseNode* addNode(BaseNode* node) noexcept;
  //! Insert the given `node` after `ref`.
  ASMJIT_API BaseNode* addAfter(BaseNode* node, BaseNode* ref) noexcept;
  //! Insert the given `node` before `ref`.
  ASMJIT_API BaseNode* addBefore(BaseNode* node, BaseNode* ref) noexcept;
  //! Remove the given `node`.
  ASMJIT_API BaseNode* removeNode(BaseNode* node) noexcept;
  //! Remove multiple nodes.
  ASMJIT_API void removeNodes(BaseNode* first, BaseNode* last) noexcept;

  //! Get current node.
  //!
  //! \note If this method returns null it means that nothing has been
  //! emitted yet.
  inline BaseNode* cursor() const noexcept { return _cursor; }
  //! Set the current node without returning the previous node.
  inline void _setCursor(BaseNode* node) noexcept { _cursor = node; }
  //! Set the current node to `node` and return the previous one.
  ASMJIT_API BaseNode* setCursor(BaseNode* node) noexcept;

  // --------------------------------------------------------------------------
  // [Label Management]
  // --------------------------------------------------------------------------

  //! Get a vector of LabelNode nodes.
  //!
  //! NOTE: If a label of some index is not associated with `BaseBuilder` it
  //! would be null, so always check for nulls if you iterate over the vector.
  inline const ZoneVector<LabelNode*>& labelNodes() const noexcept { return _labelNodes; }

  //! Get `LabelNode` by `id`.
  ASMJIT_API Error labelNodeOf(LabelNode** pOut, uint32_t id) noexcept;
  //! Get `LabelNode` by `label`.
  inline Error labelNodeOf(LabelNode** pOut, const Label& label) noexcept { return labelNodeOf(pOut, label.id()); }

  //! \internal
  ASMJIT_API Error registerLabelNode(LabelNode* node) noexcept;

  ASMJIT_API Label newLabel() override;
  ASMJIT_API Label newNamedLabel(
    const char* name,
    size_t nameSize = Globals::kNullTerminated,
    uint32_t type = Label::kTypeGlobal,
    uint32_t parentId = 0) override;
  ASMJIT_API Error bind(const Label& label) override;

  // --------------------------------------------------------------------------
  // [Pass Management]
  // --------------------------------------------------------------------------

  //! Get a vector of Pass objects that will be executed by `runPasses()`.
  inline const ZoneVector<Pass*>& passes() const noexcept { return _passes; }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T>
  inline T* newPassT() noexcept { return _codeZone.newT<T>(); }

  //! Like `new(std::nothrow) T(...)`, but allocated by `Zone`.
  template<typename T, typename... ArgsT>
  inline T* newPassT(ArgsT&&... args) noexcept { return _codeZone.newT<T>(std::forward<ArgsT>(args)...); }

  template<typename T>
  inline Error addPassT() noexcept { return addPass(newPassT<T>()); }

  template<typename T, typename... ArgsT>
  inline Error addPassT(ArgsT&&... args) noexcept { return addPass(newPassT<T, ArgsT...>(std::forward<ArgsT>(args)...)); }

  //! Get a `Pass` by name.
  ASMJIT_API Pass* passByName(const char* name) const noexcept;
  //! Add `pass` to the list of passes.
  ASMJIT_API Error addPass(Pass* pass) noexcept;
  //! Remove `pass` from the list of passes and delete it.
  ASMJIT_API Error deletePass(Pass* pass) noexcept;

  //! Run all passes in order.
  ASMJIT_API Error runPasses();

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) override;
  ASMJIT_API Error _emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) override;

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  ASMJIT_API Error align(uint32_t alignMode, uint32_t alignment) override;

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  ASMJIT_API Error embed(const void* data, uint32_t size) override;
  ASMJIT_API Error embedLabel(const Label& label) override;
  ASMJIT_API Error embedConstPool(const Label& label, const ConstPool& pool) override;

  // --------------------------------------------------------------------------
  // [Comment]
  // --------------------------------------------------------------------------

  ASMJIT_API Error comment(const char* data, size_t size = Globals::kNullTerminated) override;

  // --------------------------------------------------------------------------
  // [Serialize]
  // --------------------------------------------------------------------------

  //! Serialize everything the `BaseBuilder` or `BaseCompiler` contains to
  //! another `BaseEmitter`, which would usually be `BaseAssembler`.
  ASMJIT_API Error serialize(BaseEmitter* dst);

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_LOGGING
  ASMJIT_API Error dump(StringBuilder& sb, uint32_t flags = 0) const noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Zone _codeZone;                        //!< Base zone used to allocate nodes and `Pass`.
  Zone _dataZone;                        //!< Data zone used to allocate data and names.
  Zone _passZone;                        //!< Pass zone, passed to `Pass::run()`.
  ZoneAllocator _allocator;              //!< Allocator that uses `_codeZone`.

  ZoneVector<Pass*> _passes;             //!< Array of `Pass` objects.
  ZoneVector<LabelNode*> _labelNodes;    //!< Maps label indexes to `LabelNode` nodes.

  BaseNode* _firstNode;                  //!< First node of the current section.
  BaseNode* _lastNode;                   //!< Last node of the current section.
  BaseNode* _cursor;                     //!< Current node (cursor).

  uint32_t _nodeFlags;                   //!< Flags assigned to each new node.
};

// ============================================================================
// [asmjit::BaseNode]
// ============================================================================

//! Base node.
//!
//! Every node represents a building-block used by `BaseBuilder`. It can be
//! instruction, data, label, comment, directive, or any other high-level
//! representation that can be transformed to the building blocks mentioned.
//! Every class that inherits `BaseBuilder` can define its own nodes that it
//! can lower to basic nodes.
class BaseNode {
public:
  ASMJIT_NONCOPYABLE(BaseNode)

  //! Type of link, used to index `_link[2]` data.
  enum LinkType : uint32_t {
    kLinkPrev       = 0,                 //!< Previous node in a double-linked list.
    kLinkNext       = 1,                 //!< Next node in a double-linked list.
    kLinkCount      = 2                  //!< Count of node links (must be 2 as it's a double-linked list)
  };

  //! Type of `BaseNode`.
  enum NodeType : uint32_t {
    kNodeNone       = 0,                 //!< Invalid node (internal, don't use).

    // [BaseBuilder]
    kNodeInst       = 1,                 //!< Node is `InstNode` or `InstExNode`.
    kNodeLabel      = 2,                 //!< Node is `LabelNode`.
    kNodeAlign      = 3,                 //!< Node is `AlignNode`.
    kNodeEmbedData  = 4,                 //!< Node is `EmbedDataNode`.
    kNodeLabelData  = 5,                 //!< Node is `LabelDataNode`.
    kNodeConstPool  = 6,                 //!< Node is `ConstPoolNode`.
    kNodeComment    = 7,                 //!< Node is `CommentNode`.
    kNodeSentinel   = 8,                 //!< Node is `SentinelNode`.

    // [BaseCompiler]
    kNodeFunc       = 16,                //!< Node is `FuncNode`     (acts as LabelNode).
    kNodeFuncRet    = 17,                //!< Node is `FuncRetNode`  (acts as BaseNode).
    kNodeFuncCall   = 18,                //!< Node is `FuncCallNode` (acts as InstNode).

    // [UserDefined]
    kNodeUser       = 32                 //!< First id of a user-defined node.
  };

  //! Node flags, specify what the node is and/or does.
  enum Flags : uint32_t {
    kFlagIsCode          = 0x01u,        //!< Node is code that can be executed (instruction, label, align, etc...).
    kFlagIsData          = 0x02u,        //!< Node is data that cannot be executed (data, const-pool, etc...).
    kFlagIsInformative   = 0x04u,        //!< Node is informative, can be removed and ignored.
    kFlagIsRemovable     = 0x08u,        //!< Node can be safely removed if unreachable.
    kFlagHasNoEffect     = 0x20u,        //!< Node does nothing when executed (label, align, explicit nop).

    kFlagActsAsInst      = 0x40u,        //!< Node is an instruction or acts as it.
    kFlagActsAsLabel     = 0x80u         //!< Node is a label or acts as it.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `BaseNode` - always use `BaseBuilder` to allocate nodes.
  ASMJIT_INLINE BaseNode(BaseBuilder* cb, uint32_t type, uint32_t flags = 0) noexcept {
    _link[kLinkPrev] = nullptr;
    _link[kLinkNext] = nullptr;
    _any._nodeType = uint8_t(type);
    _any._nodeFlags = uint8_t(flags | cb->_nodeFlags);
    _any._reserved0 = 0;
    _any._reserved1 = 0;
    _position = 0;
    _passData = nullptr;
    _inlineComment = nullptr;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline BaseNode* link(size_t where) const noexcept {
    ASMJIT_ASSERT(where < kLinkCount);
    return _link[where];
  }

  //! Get previous node.
  inline BaseNode* prev() const noexcept { return _link[kLinkPrev]; }
  //! Get next node.
  inline BaseNode* next() const noexcept { return _link[kLinkNext]; }

  //! Set previous node (internal, use public BaseBuilder API to manage nodes).
  inline void _setPrev(BaseNode* node) noexcept { _link[kLinkPrev] = node; }
  //! Set next node (internal, use public BaseBuilder API to do manage nodes).
  inline void _setNext(BaseNode* node) noexcept { _link[kLinkNext] = node; }

  //! Get the node type, see `NodeType`.
  inline uint32_t type() const noexcept { return _any._nodeType; }
  //! Set the node type, see `NodeType` (internal).
  inline void setType(uint32_t type) noexcept { _any._nodeType = uint8_t(type); }

  inline bool isInst() const noexcept { return hasFlag(kFlagActsAsInst); }
  inline bool isLabel() const noexcept { return hasFlag(kFlagActsAsLabel); }
  inline bool isAlign() const noexcept { return type() == kNodeAlign; }
  inline bool isEmbedData() const noexcept { return type() == kNodeEmbedData; }
  inline bool isLabelData() const noexcept { return type() == kNodeLabelData; }
  inline bool isConstPool() const noexcept { return type() == kNodeConstPool; }
  inline bool isComment() const noexcept { return type() == kNodeComment; }
  inline bool isSentinel() const noexcept { return type() == kNodeSentinel; }

  inline bool isFunc() const noexcept { return type() == kNodeFunc; }
  inline bool isFuncRet() const noexcept { return type() == kNodeFuncRet; }
  inline bool isFuncCall() const noexcept { return type() == kNodeFuncCall; }

  //! Get the node flags.
  inline uint32_t flags() const noexcept { return _any._nodeFlags; }
  //! Get whether the instruction has flag `flag`.
  inline bool hasFlag(uint32_t flag) const noexcept { return (uint32_t(_any._nodeFlags) & flag) != 0; }
  //! Set node flags to `flags`.
  inline void setFlags(uint32_t flags) noexcept { _any._nodeFlags = uint8_t(flags); }
  //! Add instruction `flags`.
  inline void addFlags(uint32_t flags) noexcept { _any._nodeFlags = uint8_t(_any._nodeFlags | flags); }
  //! Clear instruction `flags`.
  inline void clearFlags(uint32_t flags) noexcept { _any._nodeFlags = uint8_t(_any._nodeFlags & (flags ^ 0xFF)); }

  //! Get whether the node is code that can be executed.
  inline bool isCode() const noexcept { return hasFlag(kFlagIsCode); }
  //! Get whether the node is data that cannot be executed.
  inline bool isData() const noexcept { return hasFlag(kFlagIsData); }
  //! Get whether the node is informative only (is never encoded like comment, etc...).
  inline bool isInformative() const noexcept { return hasFlag(kFlagIsInformative); }
  //! Get whether the node is removable if it's in an unreachable code block.
  inline bool isRemovable() const noexcept { return hasFlag(kFlagIsRemovable); }

  //! The node has no effect when executed (label, .align, nop, ...).
  inline bool hasNoEffect() const noexcept { return hasFlag(kFlagHasNoEffect); }

  //! Get whether the node has a position.
  inline bool hasPosition() const noexcept { return _position != 0; }
  //! Get node position.
  inline uint32_t position() const noexcept { return _position; }
  //! Set node position.
  inline void setPosition(uint32_t position) noexcept { _position = position; }

  //! Get whether the node has associated work-data.
  inline bool hasPassData() const noexcept { return _passData != nullptr; }
  //! Get work-data - data used during processing & transformations.
  template<typename T>
  inline T* passData() const noexcept { return (T*)_passData; }
  //! Set work-data to `data`.
  template<typename T>
  inline void setPassData(T* data) noexcept { _passData = (void*)data; }
  //! Reset work-data to null.
  inline void resetPassData() noexcept { _passData = nullptr; }

  //! Get whether the node has an inline comment.
  inline bool hasInlineComment() const noexcept { return _inlineComment != nullptr; }
  //! Get an inline comment string.
  inline const char* inlineComment() const noexcept { return _inlineComment; }
  //! Set an inline comment string to `s`.
  inline void setInlineComment(const char* s) noexcept { _inlineComment = s; }
  //! Set an inline comment string to null.
  inline void resetInlineComment() noexcept { _inlineComment = nullptr; }

  //! Cast this node to `T*`.
  template<typename T>
  inline T* as() noexcept { return static_cast<T*>(this); }
  //! Cast this node to `const T*`.
  template<typename T>
  inline const T* as() const noexcept { return static_cast<const T*>(this); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BaseNode* _link[2];                    //!< Links (previous and next nodes).

  struct AnyData {
    uint8_t _nodeType;                   //!< Node type, see `NodeType`.
    uint8_t _nodeFlags;                  //!< Node flags.
    uint8_t _reserved0;
    uint8_t _reserved1;
  };

  struct InstData {
    uint8_t _nodeType;                   //!< Node type, see `NodeType`.
    uint8_t _nodeFlags;                  //!< Node flags.
    uint8_t _opCount;                    //!< Number of operands.
    uint8_t _opCapacity;                 //!< Maximum number of operands (capacity).
  };

  struct SentinelData {
    uint8_t _nodeType;                   //!< Node type, see `NodeType`.
    uint8_t _nodeFlags;                  //!< Node flags.
    uint8_t _sentinelType;               //!< Sentinel type.
    uint8_t _reserved0;
  };

  union {
    AnyData _any;
    InstData _inst;
    SentinelData _sentinel;
  };

  uint32_t _position;                    //!< Node position in code (should be unique).
  void* _passData;                       //!< Data used exclusively by the current `Pass`.
  const char* _inlineComment;            //!< Inline comment or null if not used.
};

// ============================================================================
// [asmjit::InstNode]
// ============================================================================

//! Instruction node.
//!
//! Wraps an instruction with its options and operands.
class InstNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(InstNode)

  enum : uint32_t {
    //! Count of embedded operands per `InstNode` that are always allocated as
    //! a part of the instruction. Minimum embedded operands is 4, but in 32-bit
    //! more pointers are smaller and we can embed 5. The rest (up to 6 operands)
    //! is always stored in `InstExNode`.
    kBaseOpCapacity = uint32_t((128 - sizeof(BaseNode) - sizeof(BaseInst)) / sizeof(Operand_))
  };

  static inline uint32_t capacityOfOpCount(uint32_t opCount) noexcept {
    return opCount <= kBaseOpCapacity ? kBaseOpCapacity : Globals::kMaxOpCount;
  }

  static inline size_t nodeSizeOfOpCapacity(uint32_t opCapacity) noexcept {
    size_t base = sizeof(InstNode) - kBaseOpCapacity * sizeof(Operand);
    return base + opCapacity * sizeof(Operand);
  }

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `InstNode` instance.
  ASMJIT_INLINE InstNode(BaseBuilder* cb, uint32_t instId, uint32_t options, uint32_t opCount, uint32_t opCapacity = kBaseOpCapacity) noexcept
    : BaseNode(cb, kNodeInst, kFlagIsCode | kFlagIsRemovable | kFlagActsAsInst),
      _baseInst(instId, options) {
    _inst._opCapacity = uint8_t(opCapacity);
    _inst._opCount = uint8_t(opCount);
  }

  // --------------------------------------------------------------------------
  // [Init]
  // --------------------------------------------------------------------------

  //! Reset all built-in operands, including `extraReg`.
  inline void _resetOps() noexcept {
    _baseInst.resetExtraReg();
    for (uint32_t i = 0, count = opCapacity(); i < count; i++)
      _opArray[i].reset();
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline BaseInst& baseInst() noexcept { return _baseInst; }
  inline const BaseInst& baseInst() const noexcept { return _baseInst; }

  //! Get the instruction id, see `BaseInst::Id`.
  inline uint32_t id() const noexcept { return _baseInst.id(); }
  //! Set the instruction id to `id`, see `BaseInst::Id`.
  inline void setId(uint32_t id) noexcept { _baseInst.setId(id); }

  //! Get instruction options.
  inline uint32_t instOptions() const noexcept { return _baseInst.options(); }
  //! Set instruction options.
  inline void setInstOptions(uint32_t options) noexcept { _baseInst.setOptions(options); }
  //! Add instruction options.
  inline void addInstOptions(uint32_t options) noexcept { _baseInst.addOptions(options); }
  //! Clear instruction options.
  inline void clearInstOptions(uint32_t options) noexcept { _baseInst.clearOptions(options); }

  //! Get whether the node has an extra register operand.
  inline bool hasExtraReg() const noexcept { return _baseInst.hasExtraReg(); }
  //! Get extra register operand.
  inline RegOnly& extraReg() noexcept { return _baseInst.extraReg(); }
  //! \overload
  inline const RegOnly& extraReg() const noexcept { return _baseInst.extraReg(); }
  //! Set extra register operand to `reg`.
  inline void setExtraReg(const BaseReg& reg) noexcept { _baseInst.setExtraReg(reg); }
  //! Set extra register operand to `reg`.
  inline void setExtraReg(const RegOnly& reg) noexcept { _baseInst.setExtraReg(reg); }
  //! Reset extra register operand.
  inline void resetExtraReg() noexcept { _baseInst.resetExtraReg(); }

  //! Get operands count.
  inline uint32_t opCount() const noexcept { return _inst._opCount; }
  //! Get operands capacity.
  inline uint32_t opCapacity() const noexcept { return _inst._opCapacity; }

  //! Set operands count.
  inline void setOpCount(uint32_t opCount) noexcept { _inst._opCount = uint8_t(opCount); }

  //! Get operands array.
  inline Operand* operands() noexcept { return (Operand*)_opArray; }
  //! Get operands array (const).
  inline const Operand* operands() const noexcept { return (const Operand*)_opArray; }

  inline Operand& opType(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < opCapacity());
    return _opArray[index].as<Operand>();
  }

  inline const Operand& opType(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < opCapacity());
    return _opArray[index].as<Operand>();
  }

  inline void setOp(uint32_t index, const Operand_& op) noexcept {
    ASMJIT_ASSERT(index < opCapacity());
    _opArray[index].copyFrom(op);
  }

  inline void resetOp(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < opCapacity());
    _opArray[index].reset();
  }

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  inline bool hasOpType(uint32_t opType) const noexcept {
    for (uint32_t i = 0, count = opCount(); i < count; i++)
      if (_opArray[i].opType() == opType)
        return true;
    return false;
  }

  inline bool hasRegOp() const noexcept { return hasOpType(Operand::kOpReg); }
  inline bool hasMemOp() const noexcept { return hasOpType(Operand::kOpMem); }
  inline bool hasImmOp() const noexcept { return hasOpType(Operand::kOpImm); }
  inline bool hasLabelOp() const noexcept { return hasOpType(Operand::kOpLabel); }

  inline uint32_t indexOfOpType(uint32_t opType) const noexcept {
    uint32_t i = 0;
    uint32_t count = opCount();

    while (i < count) {
      if (_opArray[i].opType() == opType)
        break;
      i++;
    }

    return i;
  }

  inline uint32_t indexOfMemOp() const noexcept { return indexOfOpType(Operand::kOpMem); }
  inline uint32_t indexOfImmOp() const noexcept { return indexOfOpType(Operand::kOpImm); }
  inline uint32_t indexOfLabelOp() const noexcept { return indexOfOpType(Operand::kOpLabel); }

  // --------------------------------------------------------------------------
  // [Rewrite]
  // --------------------------------------------------------------------------

  inline uint32_t* _getRewriteArray() noexcept { return &_baseInst._extraReg._id; }
  inline const uint32_t* _getRewriteArray() const noexcept { return &_baseInst._extraReg._id; }

  ASMJIT_INLINE uint32_t getRewriteIndex(const uint32_t* id) const noexcept {
    const uint32_t* array = _getRewriteArray();
    ASMJIT_ASSERT(array <= id);

    size_t index = (size_t)(id - array);
    ASMJIT_ASSERT(index < 32);

    return uint32_t(index);
  }

  ASMJIT_INLINE void rewriteIdAtIndex(uint32_t index, uint32_t id) noexcept {
    uint32_t* array = _getRewriteArray();
    array[index] = id;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BaseInst _baseInst;                    //!< Base instruction data.
  Operand_ _opArray[kBaseOpCapacity];    //!< First 4 or 5 operands (indexed from 0).
};

// ============================================================================
// [asmjit::InstExNode]
// ============================================================================

class InstExNode : public InstNode {
public:
  ASMJIT_NONCOPYABLE(InstExNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `InstExNode` instance.
  inline InstExNode(BaseBuilder* cb, uint32_t instId, uint32_t options, uint32_t opCapacity = Globals::kMaxOpCount) noexcept
    : InstNode(cb, instId, options, opCapacity) {}

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Continued `_opArray[]` to hold up to `kMaxOpCount` operands.
  Operand_ _opArrayEx[Globals::kMaxOpCount - kBaseOpCapacity];
};

// ============================================================================
// [asmjit::LabelNode]
// ============================================================================

//! Label node.
class LabelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(LabelNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `LabelNode` instance.
  inline LabelNode(BaseBuilder* cb, uint32_t id = 0) noexcept
    : BaseNode(cb, kNodeLabel, kFlagHasNoEffect | kFlagActsAsLabel),
      _id(id) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the label id.
  inline uint32_t id() const noexcept { return _id; }
  //! Get the label as `Label` operand.
  inline Label label() const noexcept { return Label(_id); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _id;
};

// ============================================================================
// [asmjit::AlignNode]
// ============================================================================

//! Align directive (BaseBuilder).
//!
//! Wraps `.align` directive.
class AlignNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(AlignNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `AlignNode` instance.
  inline AlignNode(BaseBuilder* cb, uint32_t alignMode, uint32_t alignment) noexcept
    : BaseNode(cb, kNodeAlign, kFlagIsCode | kFlagHasNoEffect),
      _alignMode(alignMode),
      _alignment(alignment) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get align mode.
  inline uint32_t alignMode() const noexcept { return _alignMode; }
  //! Set align mode to `alignMode`.
  inline void setAlignMode(uint32_t alignMode) noexcept { _alignMode = alignMode; }

  //! Get align offset in bytes.
  inline uint32_t alignment() const noexcept { return _alignment; }
  //! Set align offset in bytes to `offset`.
  inline void setAlignment(uint32_t alignment) noexcept { _alignment = alignment; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _alignMode;                   //!< Align mode, see `AlignMode`.
  uint32_t _alignment;                   //!< Alignment (in bytes).
};

// ============================================================================
// [asmjit::EmbedDataNode]
// ============================================================================

//! Embed data node.
//!
//! Wraps `.data` directive. The node contains data that will be placed at the
//! node's position in the assembler stream. The data is considered to be RAW;
//! no analysis nor byte-order conversion is performed on RAW data.
class EmbedDataNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(EmbedDataNode)

  enum : uint32_t {
    kInlineBufferSize = uint32_t(64 - sizeof(BaseNode) - 4)
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `EmbedDataNode` instance.
  inline EmbedDataNode(BaseBuilder* cb, void* data, uint32_t size) noexcept
    : BaseNode(cb, kNodeEmbedData, kFlagIsData) {

    if (size <= kInlineBufferSize) {
      if (data)
        std::memcpy(_buf, data, size);
    }
    else {
      _externalPtr = static_cast<uint8_t*>(data);
    }
    _size = size;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get pointer to the data.
  inline uint8_t* data() const noexcept { return _size <= kInlineBufferSize ? const_cast<uint8_t*>(_buf) : _externalPtr; }
  //! Get size of the data.
  inline uint32_t size() const noexcept { return _size; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      uint8_t _buf[kInlineBufferSize];   //!< Embedded data buffer.
      uint32_t _size;                    //!< Size of the data.
    };
    struct {
      uint8_t* _externalPtr;             //!< Pointer to external data.
    };
  };
};

// ============================================================================
// [asmjit::LabelDataNode]
// ============================================================================

//! Label data node.
class LabelDataNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(LabelDataNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `LabelDataNode` instance.
  inline LabelDataNode(BaseBuilder* cb, uint32_t id = 0) noexcept
    : BaseNode(cb, kNodeLabelData, kFlagIsData),
      _id(id) {}

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  //! Get the label id.
  inline uint32_t id() const noexcept { return _id; }
  //! Set the label id (use with caution, improper use can break a lot of things).
  inline void setId(uint32_t id) noexcept { _id = id; }

  //! Get the label as `Label` operand.
  inline Label label() const noexcept { return Label(_id); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _id;
};

// ============================================================================
// [asmjit::ConstPoolNode]
// ============================================================================

class ConstPoolNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(ConstPoolNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `ConstPoolNode` instance.
  inline ConstPoolNode(BaseBuilder* cb, uint32_t id = 0) noexcept
    : LabelNode(cb, id),
      _constPool(&cb->_codeZone) {

    setType(kNodeConstPool);
    addFlags(kFlagIsData);
    clearFlags(kFlagIsCode | kFlagHasNoEffect);
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  //! Get whether the constant-pool is empty.
  inline bool empty() const noexcept { return _constPool.empty(); }
  //! Get the size of the constant-pool in bytes.
  inline size_t size() const noexcept { return _constPool.size(); }
  //! Get minimum alignment.
  inline size_t alignment() const noexcept { return _constPool.alignment(); }

  //! Get the wrapped `ConstPool` instance.
  inline ConstPool& constPool() noexcept { return _constPool; }
  //! Get the wrapped `ConstPool` instance (const).
  inline const ConstPool& constPool() const noexcept { return _constPool; }

  //! See `ConstPool::add()`.
  inline Error add(const void* data, size_t size, size_t& dstOffset) noexcept {
    return _constPool.add(data, size, dstOffset);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ConstPool _constPool;
};

// ============================================================================
// [asmjit::CommentNode]
// ============================================================================

//! Comment node.
class CommentNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(CommentNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `CommentNode` instance.
  inline CommentNode(BaseBuilder* cb, const char* comment) noexcept
    : BaseNode(cb, kNodeComment, kFlagIsInformative | kFlagHasNoEffect | kFlagIsRemovable) {
    _inlineComment = comment;
  }
};

// ============================================================================
// [asmjit::SentinelNode]
// ============================================================================

//! Sentinel node.
//!
//! Sentinel is a marker that is completely ignored by the code builder. It's
//! used to remember a position in a code as it never gets removed by any pass.
class SentinelNode : public BaseNode {
public:
  ASMJIT_NONCOPYABLE(SentinelNode)

  //! Type of the sentinel (purery informative purpose).
  enum SentinelType : uint32_t {
    kSentinelUnknown = 0u,
    kSentinelFuncEnd = 1u
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `SentinelNode` instance.
  inline SentinelNode(BaseBuilder* cb, uint32_t sentinelType = kSentinelUnknown) noexcept
    : BaseNode(cb, kNodeSentinel, kFlagIsInformative | kFlagHasNoEffect) {

    _sentinel._sentinelType = uint8_t(sentinelType);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline uint32_t sentinelType() const noexcept { return _sentinel._sentinelType; }
  inline void setSentinelType(uint32_t type) noexcept { _sentinel._sentinelType = uint8_t(type); }
};

// ============================================================================
// [asmjit::Pass]
// ============================================================================

//! Pass can be used to implement code transformations, analysis, and lowering.
class ASMJIT_VIRTAPI Pass {
public:
  ASMJIT_NONCOPYABLE(Pass);

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API Pass(const char* name) noexcept;
  ASMJIT_API virtual ~Pass() noexcept;

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  //! Process the code stored in BaseBuilder.
  //!
  //! This is the only function that is called by the `BaseBuilder` to process
  //! the code. It passes `zone`, which will be reset after the `run()` returns
  //! by `BaseBuilder`.
  virtual Error run(Zone* zone, Logger* logger) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline const BaseBuilder* cb() const noexcept { return _cb; }
  inline const char* name() const noexcept { return _name; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BaseBuilder* _cb;                      //!< BaseBuilder this pass is assigned to.
  const char* _name;                     //!< Name of the pass.
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
#endif // _ASMJIT_CORE_BUILDER_H
