// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_COMPILER_H
#define _ASMJIT_BASE_COMPILER_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/compilerfunc.h"
#include "../base/constpool.h"
#include "../base/containers.h"
#include "../base/hlstream.h"
#include "../base/operand.h"
#include "../base/utils.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct VarAttr;
struct VarData;
struct VarMap;
struct VarState;

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::CompilerFeatures]
// ============================================================================

ASMJIT_ENUM(CompilerFeatures) {
  //! Schedule instructions so they can be executed faster (`Compiler` only).
  //!
  //! Default `false` - has to be explicitly enabled as the scheduler needs
  //! some time to run.
  //!
  //! X86/X64
  //! -------
  //!
  //! If scheduling is enabled AsmJit will try to reorder instructions to
  //! minimize the dependency chain. Scheduler always runs after the registers
  //! are allocated so it doesn't change count of register allocs/spills.
  //!
  //! This feature is highly experimental and untested.
  kCompilerFeatureEnableScheduler = 0
};

// ============================================================================
// [asmjit::ConstScope]
// ============================================================================

//! Scope of the constant.
ASMJIT_ENUM(ConstScope) {
  //! Local constant, always embedded right after the current function.
  kConstScopeLocal = 0,
  //! Global constant, embedded at the end of the currently compiled code.
  kConstScopeGlobal = 1
};

// ============================================================================
// [asmjit::Compiler]
// ============================================================================

//! Compiler interface.
//!
//! \sa Assembler.
struct ASMJIT_VIRTAPI Compiler : public CodeGen {
  ASMJIT_NO_COPY(Compiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `Compiler` instance.
  ASMJIT_API Compiler();
  //! Destroy the `Compiler` instance.
  ASMJIT_API virtual ~Compiler();

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! \override
  ASMJIT_API virtual void reset(bool releaseMemory);

  // --------------------------------------------------------------------------
  // [Compiler Features]
  // --------------------------------------------------------------------------

  //! Get code-generator features.
  ASMJIT_INLINE uint32_t getFeatures() const { return _features; }
  //! Set code-generator features.
  ASMJIT_INLINE void setFeatures(uint32_t features) { _features = features; }

  //! Get code-generator `feature`.
  ASMJIT_INLINE bool hasFeature(uint32_t feature) const {
    ASMJIT_ASSERT(feature < 32);
    return (_features & (1 << feature)) != 0;
  }

  //! Set code-generator `feature` to `value`.
  ASMJIT_INLINE void setFeature(uint32_t feature, bool value) {
    ASMJIT_ASSERT(feature < 32);
    feature = static_cast<uint32_t>(value) << feature;
    _features = (_features & ~feature) | feature;
  }

  //! Get maximum look ahead.
  ASMJIT_INLINE uint32_t getMaxLookAhead() const { return _maxLookAhead; }
  //! Set maximum look ahead to `val`.
  ASMJIT_INLINE void setMaxLookAhead(uint32_t val) { _maxLookAhead = val; }

  // --------------------------------------------------------------------------
  // [Token ID]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Reset the token-id generator.
  ASMJIT_INLINE void _resetTokenGenerator() { _tokenGenerator = 0; }

  //! \internal
  //!
  //! Generate a new unique token id.
  ASMJIT_INLINE uint32_t _generateUniqueToken() { return ++_tokenGenerator; }

  // --------------------------------------------------------------------------
  // [Instruction Options]
  // --------------------------------------------------------------------------

  //! Get options of the next instruction.
  ASMJIT_INLINE uint32_t getInstOptions() const { return _instOptions; }
  //! Set options of the next instruction.
  ASMJIT_INLINE void setInstOptions(uint32_t instOptions) { _instOptions = instOptions; }

  //! Get options of the next instruction and reset them.
  ASMJIT_INLINE uint32_t getInstOptionsAndReset() {
    uint32_t instOptions = _instOptions;
    _instOptions = 0;
    return instOptions;
  };

  // --------------------------------------------------------------------------
  // [Node-Factory]
  // --------------------------------------------------------------------------

  //! \internal
  template<typename T>
  ASMJIT_INLINE T* newNode() {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this);
  }

  //! \internal
  template<typename T, typename P0>
  ASMJIT_INLINE T* newNode(P0 p0) {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0);
  }

  //! \internal
  template<typename T, typename P0, typename P1>
  ASMJIT_INLINE T* newNode(P0 p0, P1 p1) {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0, p1);
  }

  //! \internal
  template<typename T, typename P0, typename P1, typename P2>
  ASMJIT_INLINE T* newNode(P0 p0, P1 p1, P2 p2) {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0, p1, p2);
  }

  //! \internal
  //!
  //! Create a new `HLData` node.
  ASMJIT_API HLData* newDataNode(const void* data, uint32_t size);

  //! \internal
  //!
  //! Create a new `HLAlign` node.
  ASMJIT_API HLAlign* newAlignNode(uint32_t alignMode, uint32_t offset);

  //! \internal
  //!
  //! Create a new `HLLabel` node.
  ASMJIT_API HLLabel* newLabelNode();

  //! \internal
  //!
  //! Create a new `HLComment`.
  ASMJIT_API HLComment* newCommentNode(const char* str);

  //! \internal
  //!
  //! Create a new `HLHint`.
  ASMJIT_API HLHint* newHintNode(Var& var, uint32_t hint, uint32_t value);

  // --------------------------------------------------------------------------
  // [Code-Stream]
  // --------------------------------------------------------------------------

  //! Add node `node` after current and set current to `node`.
  ASMJIT_API HLNode* addNode(HLNode* node);
  //! Insert `node` before `ref`.
  ASMJIT_API HLNode* addNodeBefore(HLNode* node, HLNode* ref);
  //! Insert `node` after `ref`.
  ASMJIT_API HLNode* addNodeAfter(HLNode* node, HLNode* ref);
  //! Remove `node`.
  ASMJIT_API HLNode* removeNode(HLNode* node);
  //! Remove multiple nodes.
  ASMJIT_API void removeNodes(HLNode* first, HLNode* last);

  //! Get the first node.
  ASMJIT_INLINE HLNode* getFirstNode() const { return _firstNode; }
  //! Get the last node.
  ASMJIT_INLINE HLNode* getLastNode() const { return _lastNode; }

  //! Get current node.
  //!
  //! \note If this method returns `NULL` it means that nothing has been emitted
  //! yet.
  ASMJIT_INLINE HLNode* getCursor() const { return _cursor; }
  //! \internal
  //!
  //! Set the current node without returning the previous node.
  ASMJIT_INLINE void _setCursor(HLNode* node) { _cursor = node; }
  //! Set the current node to `node` and return the previous one.
  ASMJIT_API HLNode* setCursor(HLNode* node);

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! Get current function.
  ASMJIT_INLINE HLFunc* getFunc() const { return _func; }

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! Align target buffer to the `offset` specified.
  //!
  //! The sequence that is used to fill the gap between the aligned location
  //! and the current depends on `alignMode`, see \ref AlignMode.
  ASMJIT_API Error align(uint32_t alignMode, uint32_t offset);

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! Get `HLLabel` by `id`.
  //!
  //! NOTE: The label has to be valid, see `isLabelValid()`.
  ASMJIT_API HLLabel* getHLLabel(uint32_t id) const;

  //! Get `HLLabel` by `label`.
  //!
  //! NOTE: The label has to be valid, see `isLabelValid()`.
  ASMJIT_INLINE HLLabel* getHLLabel(const Label& label) { return getHLLabel(label.getId()); }

  //! Get whether the label `id` is valid.
  ASMJIT_API bool isLabelValid(uint32_t id) const;
  //! Get whether the `label` is valid.
  ASMJIT_INLINE bool isLabelValid(const Label& label) const { return isLabelValid(label.getId()); }

  //! \internal
  //!
  //! Create a new label and return its ID.
  ASMJIT_API uint32_t _newLabelId();

  //! Create and return a new `Label`.
  ASMJIT_INLINE Label newLabel() { return Label(_newLabelId()); }

  //! Bind label to the current offset.
  //!
  //! \note Label can be bound only once!
  ASMJIT_API Error bind(const Label& label);

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! Embed data.
  ASMJIT_API Error embed(const void* data, uint32_t size);

  //! Embed a constant pool data, adding the following in order:
  //!   1. Data alignment.
  //!   2. Label.
  //!   3. Constant pool data.
  ASMJIT_API Error embedConstPool(const Label& label, const ConstPool& pool);

  // --------------------------------------------------------------------------
  // [Comment]
  // --------------------------------------------------------------------------

  //! Emit a single comment line.
  ASMJIT_API Error comment(const char* fmt, ...);

  // --------------------------------------------------------------------------
  // [Hint]
  // --------------------------------------------------------------------------

  //! Emit a new hint (purery informational node).
  ASMJIT_API Error _hint(Var& var, uint32_t hint, uint32_t value);

  // --------------------------------------------------------------------------
  // [Vars]
  // --------------------------------------------------------------------------

  //! Get whether variable `var` is created.
  ASMJIT_INLINE bool isVarValid(const Var& var) const {
    return static_cast<size_t>(var.getId() & kOperandIdNum) < _varList.getLength();
  }

  //! \internal
  //!
  //! Get `VarData` by `var`.
  ASMJIT_INLINE VarData* getVd(const Var& var) const {
    return getVdById(var.getId());
  }

  //! \internal
  //!
  //! Get `VarData` by `id`.
  ASMJIT_INLINE VarData* getVdById(uint32_t id) const {
    ASMJIT_ASSERT(id != kInvalidValue);
    ASMJIT_ASSERT(static_cast<size_t>(id & kOperandIdNum) < _varList.getLength());

    return _varList[id & kOperandIdNum];
  }

  //! \internal
  //!
  //! Get an array of 'VarData*'.
  ASMJIT_INLINE VarData** _getVdArray() const {
    return const_cast<VarData**>(_varList.getData());
  }

  //! \internal
  //!
  //! Create a new `VarData`.
  ASMJIT_API VarData* _newVd(uint32_t type, uint32_t size, uint32_t c, const char* name);

  //! Create a new `Var`.
  virtual Error _newVar(Var* var, uint32_t vType, const char* name, va_list ap) = 0;

  //! Alloc variable `var`.
  ASMJIT_API Error alloc(Var& var);
  //! Alloc variable `var` using `regIndex` as a register index.
  ASMJIT_API Error alloc(Var& var, uint32_t regIndex);
  //! Alloc variable `var` using `reg` as a register operand.
  ASMJIT_API Error alloc(Var& var, const Reg& reg);
  //! Spill variable `var`.
  ASMJIT_API Error spill(Var& var);
  //! Save variable `var` if the status is `modified` at this point.
  ASMJIT_API Error save(Var& var);
  //! Unuse variable `var`.
  ASMJIT_API Error unuse(Var& var);

  //! Get priority of variable `var`.
  ASMJIT_API uint32_t getPriority(Var& var) const;
  //! Set priority of variable `var` to `priority`.
  ASMJIT_API void setPriority(Var& var, uint32_t priority);

  //! Get save-on-unuse `var` property.
  ASMJIT_API bool getSaveOnUnuse(Var& var) const;
  //! Set save-on-unuse `var` property to `value`.
  ASMJIT_API void setSaveOnUnuse(Var& var, bool value);

  //! Rename variable `var` to `name`.
  //!
  //! \note Only new name will appear in the logger.
  ASMJIT_API void rename(Var& var, const char* fmt, ...);

  // --------------------------------------------------------------------------
  // [Stack]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Create a new memory chunk allocated on the current function's stack.
  virtual Error _newStack(BaseMem* mem, uint32_t size, uint32_t alignment, const char* name) = 0;

  // --------------------------------------------------------------------------
  // [Const]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Put data to a constant-pool and get a memory reference to it.
  virtual Error _newConst(BaseMem* mem, uint32_t scope, const void* data, size_t size) = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Code-Generation features, used by \ref hasFeature() and \ref setFeature().
  uint32_t _features;
  //! Maximum count of nodes to look ahead when allocating/spilling
  //! registers.
  uint32_t _maxLookAhead;

  //! Options affecting the next instruction.
  uint32_t _instOptions;
  //! Processing token generator.
  //!
  //! Used to get a unique token that is then used to process `HLNode`s. See
  //! `Compiler::_getUniqueToken()` for more details.
  uint32_t _tokenGenerator;

  //! Flow id added to each node created (used only by `Context)`.
  uint32_t _nodeFlowId;
  //! Flags added to each node created (used only by `Context)`.
  uint32_t _nodeFlags;

  //! Variable mapping (translates incoming VarType into target).
  const uint8_t* _targetVarMapping;

  //! First node.
  HLNode* _firstNode;
  //! Last node.
  HLNode* _lastNode;

  //! Current node.
  HLNode* _cursor;
  //! Current function.
  HLFunc* _func;

  //! General purpose zone allocator.
  Zone _zoneAllocator;
  //! Variable zone.
  Zone _varAllocator;
  //! String/data zone.
  Zone _stringAllocator;
  //! Local constant pool zone.
  Zone _constAllocator;

  //! VarData list.
  PodVector<VarData*> _varList;

  //! Local constant pool, flushed at the end of each function.
  ConstPool _localConstPool;
  //! Global constant pool, flushed at the end of the compilation.
  ConstPool _globalConstPool;

  //! Label to start of the local constant pool.
  Label _localConstPoolLabel;
  //! Label to start of the global constant pool.
  Label _globalConstPoolLabel;
};

//! \}

// ============================================================================
// [Defined-Later]
// ============================================================================

ASMJIT_INLINE Label::Label(Compiler& c) : Operand(NoInit) {
  reset();
  _label.id = c._newLabelId();
}

ASMJIT_INLINE HLNode::HLNode(Compiler* compiler, uint32_t type) {
  _prev = NULL;
  _next = NULL;
  _type = static_cast<uint8_t>(type);
  _opCount = 0;
  _flags = static_cast<uint16_t>(compiler->_nodeFlags);
  _flowId = compiler->_nodeFlowId;
  _tokenId = 0;
  _comment = NULL;
  _map = NULL;
  _liveness = NULL;
  _state = NULL;
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_COMPILER_H
