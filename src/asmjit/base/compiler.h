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
#include "../base/podvector.h"
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
  //! X86/X64 Specific
  //! ----------------
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
// [asmjit::VarInfo]
// ============================================================================

struct VarInfo {
  // ============================================================================
  // [Flags]
  // ============================================================================

  //! \internal
  //!
  //! Variable flags.
  ASMJIT_ENUM(Flags) {
    //! Variable contains one or more single-precision floating point.
    kFlagSP = 0x10,
    //! Variable contains one or more double-precision floating point.
    kFlagDP = 0x20,
    //! Variable is a vector, contains packed data.
    kFlagSIMD = 0x80
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get type id.
  ASMJIT_INLINE uint32_t getTypeId() const noexcept { return _typeId; }
  //! Get type name.
  ASMJIT_INLINE const char* getTypeName() const noexcept { return _typeName; }

  //! Get register size in bytes.
  ASMJIT_INLINE uint32_t getSize() const noexcept { return _size; }
  //! Get variable class, see \ref RegClass.
  ASMJIT_INLINE uint32_t getRegClass() const noexcept { return _regClass; }
  //! Get register type, see `X86RegType`.
  ASMJIT_INLINE uint32_t getRegType() const noexcept { return _regType; }
  //! Get type flags, see `VarFlag`.
  ASMJIT_INLINE uint32_t getFlags() const noexcept { return _flags; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Variable type id.
  uint8_t _typeId;
  //! Variable and register size (in bytes).
  uint8_t _size;
  //! Register class, see `RegClass`.
  uint8_t _regClass;
  //! Register type the variable is mapped to.
  uint8_t _regType;

  //! Variable info flags, see \ref Flags.
  uint32_t _flags;

  //! Variable type name.
  char _typeName[8];
};

// ============================================================================
// [asmjit::Compiler]
// ============================================================================

//! Compiler interface.
//!
//! \sa Assembler.
struct ASMJIT_VIRTAPI Compiler : public ExternalTool {
  ASMJIT_NO_COPY(Compiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `Compiler` instance.
  ASMJIT_API Compiler() noexcept;
  //! Destroy the `Compiler` instance.
  ASMJIT_API virtual ~Compiler() noexcept;

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! \override
  ASMJIT_API virtual void reset(bool releaseMemory) noexcept;

  // --------------------------------------------------------------------------
  // [Compiler Features]
  // --------------------------------------------------------------------------

  //! Get code-generator features.
  ASMJIT_INLINE uint32_t getFeatures() const noexcept {
    return _features;
  }
  //! Set code-generator features.
  ASMJIT_INLINE void setFeatures(uint32_t features) noexcept {
    _features = features;
  }

  //! Get code-generator `feature`.
  ASMJIT_INLINE bool hasFeature(uint32_t feature) const noexcept {
    ASMJIT_ASSERT(feature < 32);
    return (_features & (1 << feature)) != 0;
  }

  //! Set code-generator `feature` to `value`.
  ASMJIT_INLINE void setFeature(uint32_t feature, bool value) noexcept {
    ASMJIT_ASSERT(feature < 32);
    feature = static_cast<uint32_t>(value) << feature;
    _features = (_features & ~feature) | feature;
  }

  //! Get maximum look ahead.
  ASMJIT_INLINE uint32_t getMaxLookAhead() const noexcept {
    return _maxLookAhead;
  }
  //! Set maximum look ahead to `val`.
  ASMJIT_INLINE void setMaxLookAhead(uint32_t val) noexcept {
    _maxLookAhead = val;
  }

  // --------------------------------------------------------------------------
  // [Token ID]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Reset the token-id generator.
  ASMJIT_INLINE void _resetTokenGenerator() noexcept {
    _tokenGenerator = 0;
  }

  //! \internal
  //!
  //! Generate a new unique token id.
  ASMJIT_INLINE uint32_t _generateUniqueToken() noexcept {
    return ++_tokenGenerator;
  }

  // --------------------------------------------------------------------------
  // [Instruction Options]
  // --------------------------------------------------------------------------

  //! Get options of the next instruction.
  ASMJIT_INLINE uint32_t getInstOptions() const noexcept {
    return _instOptions;
  }
  //! Set options of the next instruction.
  ASMJIT_INLINE void setInstOptions(uint32_t instOptions) noexcept {
    _instOptions = instOptions;
  }

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
  ASMJIT_INLINE T* newNode() noexcept {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this);
  }

  //! \internal
  template<typename T, typename P0>
  ASMJIT_INLINE T* newNode(P0 p0) noexcept {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0);
  }

  //! \internal
  template<typename T, typename P0, typename P1>
  ASMJIT_INLINE T* newNode(P0 p0, P1 p1) noexcept {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0, p1);
  }

  //! \internal
  template<typename T, typename P0, typename P1, typename P2>
  ASMJIT_INLINE T* newNode(P0 p0, P1 p1, P2 p2) noexcept {
    void* p = _zoneAllocator.alloc(sizeof(T));
    return new(p) T(this, p0, p1, p2);
  }

  //! \internal
  //!
  //! Create a new `HLData` node.
  ASMJIT_API HLData* newDataNode(const void* data, uint32_t size) noexcept;

  //! \internal
  //!
  //! Create a new `HLAlign` node.
  ASMJIT_API HLAlign* newAlignNode(uint32_t alignMode, uint32_t offset) noexcept;

  //! \internal
  //!
  //! Create a new `HLLabel` node.
  ASMJIT_API HLLabel* newLabelNode() noexcept;

  //! \internal
  //!
  //! Create a new `HLComment`.
  ASMJIT_API HLComment* newCommentNode(const char* str) noexcept;

  //! \internal
  //!
  //! Create a new `HLHint`.
  ASMJIT_API HLHint* newHintNode(Var& var, uint32_t hint, uint32_t value) noexcept;

  // --------------------------------------------------------------------------
  // [Code-Stream]
  // --------------------------------------------------------------------------

  //! Add a function `node` to the stream.
  ASMJIT_API HLNode* addFunc(HLFunc* func) noexcept;

  //! Add node `node` after current and set current to `node`.
  ASMJIT_API HLNode* addNode(HLNode* node) noexcept;
  //! Insert `node` before `ref`.
  ASMJIT_API HLNode* addNodeBefore(HLNode* node, HLNode* ref) noexcept;
  //! Insert `node` after `ref`.
  ASMJIT_API HLNode* addNodeAfter(HLNode* node, HLNode* ref) noexcept;
  //! Remove `node`.
  ASMJIT_API HLNode* removeNode(HLNode* node) noexcept;
  //! Remove multiple nodes.
  ASMJIT_API void removeNodes(HLNode* first, HLNode* last) noexcept;

  //! Get the first node.
  ASMJIT_INLINE HLNode* getFirstNode() const noexcept { return _firstNode; }
  //! Get the last node.
  ASMJIT_INLINE HLNode* getLastNode() const noexcept { return _lastNode; }

  //! Get current node.
  //!
  //! \note If this method returns `nullptr` it means that nothing has been
  //! emitted yet.
  ASMJIT_INLINE HLNode* getCursor() const noexcept { return _cursor; }
  //! \internal
  //!
  //! Set the current node without returning the previous node.
  ASMJIT_INLINE void _setCursor(HLNode* node) noexcept { _cursor = node; }
  //! Set the current node to `node` and return the previous one.
  ASMJIT_API HLNode* setCursor(HLNode* node) noexcept;

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! Get current function.
  ASMJIT_INLINE HLFunc* getFunc() const noexcept { return _func; }

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! Align target buffer to the `offset` specified.
  //!
  //! The sequence that is used to fill the gap between the aligned location
  //! and the current depends on `alignMode`, see \ref AlignMode.
  ASMJIT_API Error align(uint32_t alignMode, uint32_t offset) noexcept;

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! Get `HLLabel` by `id`.
  //!
  //! NOTE: The label has to be valid, see `isLabelValid()`.
  ASMJIT_API HLLabel* getHLLabel(uint32_t id) const noexcept;

  //! Get `HLLabel` by `label`.
  //!
  //! NOTE: The label has to be valid, see `isLabelValid()`.
  ASMJIT_INLINE HLLabel* getHLLabel(const Label& label) noexcept {
    return getHLLabel(label.getId());
  }

  //! Get whether the label `id` is valid.
  ASMJIT_API bool isLabelValid(uint32_t id) const noexcept;
  //! Get whether the `label` is valid.
  ASMJIT_INLINE bool isLabelValid(const Label& label) const noexcept {
    return isLabelValid(label.getId());
  }

  //! \internal
  //!
  //! Create a new label and return its ID.
  ASMJIT_API uint32_t _newLabelId() noexcept;

  //! Create and return a new `Label`.
  ASMJIT_INLINE Label newLabel() noexcept { return Label(_newLabelId()); }

  //! Bind label to the current offset.
  //!
  //! \note Label can be bound only once!
  ASMJIT_API Error bind(const Label& label) noexcept;

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! Embed data.
  ASMJIT_API Error embed(const void* data, uint32_t size) noexcept;

  //! Embed a constant pool data, adding the following in order:
  //!   1. Data alignment.
  //!   2. Label.
  //!   3. Constant pool data.
  ASMJIT_API Error embedConstPool(const Label& label, const ConstPool& pool) noexcept;

  // --------------------------------------------------------------------------
  // [Comment]
  // --------------------------------------------------------------------------

  //! Emit a single comment line.
  ASMJIT_API Error comment(const char* fmt, ...) noexcept;

  // --------------------------------------------------------------------------
  // [Hint]
  // --------------------------------------------------------------------------

  //! Emit a new hint (purery informational node).
  ASMJIT_API Error _hint(Var& var, uint32_t hint, uint32_t value) noexcept;

  // --------------------------------------------------------------------------
  // [Vars]
  // --------------------------------------------------------------------------

  //! Get whether variable `var` is created.
  ASMJIT_INLINE bool isVarValid(const Var& var) const noexcept {
    return static_cast<size_t>(var.getId() & Operand::kIdIndexMask) < _varList.getLength();
  }

  //! \internal
  //!
  //! Get `VarData` by `var`.
  ASMJIT_INLINE VarData* getVd(const Var& var) const noexcept {
    return getVdById(var.getId());
  }

  //! \internal
  //!
  //! Get `VarData` by `id`.
  ASMJIT_INLINE VarData* getVdById(uint32_t id) const noexcept {
    ASMJIT_ASSERT(id != kInvalidValue);
    ASMJIT_ASSERT(static_cast<size_t>(id & Operand::kIdIndexMask) < _varList.getLength());

    return _varList[id & Operand::kIdIndexMask];
  }

  //! \internal
  //!
  //! Get an array of 'VarData*'.
  ASMJIT_INLINE VarData** _getVdArray() const noexcept {
    return const_cast<VarData**>(_varList.getData());
  }

  //! \internal
  //!
  //! Create a new `VarData`.
  ASMJIT_API VarData* _newVd(const VarInfo& vi, const char* name) noexcept;

  //! Alloc variable `var`.
  ASMJIT_API Error alloc(Var& var) noexcept;
  //! Alloc variable `var` using `regIndex` as a register index.
  ASMJIT_API Error alloc(Var& var, uint32_t regIndex) noexcept;
  //! Alloc variable `var` using `reg` as a register operand.
  ASMJIT_API Error alloc(Var& var, const Reg& reg) noexcept;
  //! Spill variable `var`.
  ASMJIT_API Error spill(Var& var) noexcept;
  //! Save variable `var` if the status is `modified` at this point.
  ASMJIT_API Error save(Var& var) noexcept;
  //! Unuse variable `var`.
  ASMJIT_API Error unuse(Var& var) noexcept;

  //! Get priority of variable `var`.
  ASMJIT_API uint32_t getPriority(Var& var) const noexcept;
  //! Set priority of variable `var` to `priority`.
  ASMJIT_API void setPriority(Var& var, uint32_t priority) noexcept;

  //! Get save-on-unuse `var` property.
  ASMJIT_API bool getSaveOnUnuse(Var& var) const noexcept;
  //! Set save-on-unuse `var` property to `value`.
  ASMJIT_API void setSaveOnUnuse(Var& var, bool value) noexcept;

  //! Rename variable `var` to `name`.
  //!
  //! \note Only new name will appear in the logger.
  ASMJIT_API void rename(Var& var, const char* fmt, ...) noexcept;

  // --------------------------------------------------------------------------
  // [Stack]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Create a new memory chunk allocated on the current function's stack.
  virtual Error _newStack(BaseMem* mem, uint32_t size, uint32_t alignment, const char* name) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Const]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Put data to a constant-pool and get a memory reference to it.
  virtual Error _newConst(BaseMem* mem, uint32_t scope, const void* data, size_t size) noexcept = 0;

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

ASMJIT_INLINE HLNode::HLNode(Compiler* compiler, uint32_t type) noexcept {
  _prev = nullptr;
  _next = nullptr;
  _type = static_cast<uint8_t>(type);
  _opCount = 0;
  _flags = static_cast<uint16_t>(compiler->_nodeFlags);
  _flowId = compiler->_nodeFlowId;
  _tokenId = 0;
  _comment = nullptr;
  _map = nullptr;
  _liveness = nullptr;
  _state = nullptr;
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_COMPILER_H
