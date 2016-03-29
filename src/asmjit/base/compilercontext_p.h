// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_COMPILERCONTEXT_P_H
#define _ASMJIT_BASE_COMPILERCONTEXT_P_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/compiler.h"
#include "../base/podvector.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::VarAttrFlags]
// ============================================================================

//! \internal
//!
//! Variable attribute flags.
ASMJIT_ENUM(VarAttrFlags) {
  //! Read from register.
  kVarAttrRReg = 0x00000001,
  //! Write to register.
  kVarAttrWReg = 0x00000002,
  //! Read/Write from/to register.
  kVarAttrXReg = 0x00000003,

  //! Read from memory.
  kVarAttrRMem = 0x00000004,
  //! Write to memory.
  kVarAttrWMem = 0x00000008,
  //! Read/Write from/to memory.
  kVarAttrXMem = 0x0000000C,

  //! Register allocator can decide if input will be in register or memory.
  kVarAttrRDecide = 0x00000010,
  //! Register allocator can decide if output will be in register or memory.
  kVarAttrWDecide = 0x00000020,
  //! Register allocator can decide if in/out will be in register or memory.
  kVarAttrXDecide = 0x00000030,

  //! Variable is converted to other type/class on the input.
  kVarAttrRConv = 0x00000040,
  //! Variable is converted from other type/class on the output.
  kVarAttrWConv = 0x00000080,
  //! Combination of `kVarAttrRConv` and `kVarAttrWConv`.
  kVarAttrXConv = 0x000000C0,

  //! Variable is a function call operand.
  kVarAttrRCall = 0x00000100,
  //! Variable is a function argument passed in register.
  kVarAttrRFunc = 0x00000200,
  //! Variable is a function return value passed in register.
  kVarAttrWFunc = 0x00000400,

  //! Variable should be spilled.
  kVarAttrSpill = 0x00000800,
  //! Variable should be unused at the end of the instruction/node.
  kVarAttrUnuse = 0x00001000,

  //! All in-flags.
  kVarAttrRAll = kVarAttrRReg | kVarAttrRMem | kVarAttrRDecide | kVarAttrRCall | kVarAttrRFunc,
  //! All out-flags.
  kVarAttrWAll = kVarAttrWReg | kVarAttrWMem | kVarAttrWDecide | kVarAttrWFunc,

  //! Variable is already allocated on the input.
  kVarAttrAllocRDone = 0x00400000,
  //! Variable is already allocated on the output.
  kVarAttrAllocWDone = 0x00800000,

  kVarAttrX86GpbLo = 0x10000000,
  kVarAttrX86GpbHi = 0x20000000,
  kVarAttrX86Fld4  = 0x40000000,
  kVarAttrX86Fld8  = 0x80000000
};

// ============================================================================
// [asmjit::VarHint]
// ============================================================================

//! \internal
//!
//! Variable hint (used by `Compiler)`.
//!
//! \sa Compiler.
ASMJIT_ENUM(VarHint) {
  //! Alloc variable.
  kVarHintAlloc = 0,
  //! Spill variable.
  kVarHintSpill = 1,
  //! Save variable if modified.
  kVarHintSave = 2,
  //! Save variable if modified and mark it as unused.
  kVarHintSaveAndUnuse = 3,
  //! Mark variable as unused.
  kVarHintUnuse = 4
};

// ============================================================================
// [asmjit::kVarState]
// ============================================================================

// TODO: Rename `kVarState` or `VarState`.

//! \internal
//!
//! State of variable.
//!
//! \note Variable states are used only during register allocation.
ASMJIT_ENUM(kVarState) {
  //! Variable is currently not used.
  kVarStateNone = 0,
  //! Variable is currently allocated in register.
  kVarStateReg = 1,
  //! Variable is currently allocated in memory (or has been spilled).
  kVarStateMem = 2
};

// ============================================================================
// [asmjit::VarCell]
// ============================================================================

struct VarCell {
  ASMJIT_NO_COPY(VarCell)

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get cell offset.
  ASMJIT_INLINE int32_t getOffset() const { return _offset; }
  //! Set cell offset.
  ASMJIT_INLINE void setOffset(int32_t offset) { _offset = offset; }

  //! Get cell size.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }
  //! Set cell size.
  ASMJIT_INLINE void setSize(uint32_t size) { _size = size; }

  //! Get cell alignment.
  ASMJIT_INLINE uint32_t getAlignment() const { return _alignment; }
  //! Set cell alignment.
  ASMJIT_INLINE void setAlignment(uint32_t alignment) { _alignment = alignment; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Next active cell.
  VarCell* _next;

  //! Offset, relative to base-offset.
  int32_t _offset;
  //! Size.
  uint32_t _size;
  //! Alignment.
  uint32_t _alignment;
};

// ============================================================================
// [asmjit::VarData]
// ============================================================================

//! HL variable data (base).
struct VarData {
  // --------------------------------------------------------------------------
  // [Accessors - Base]
  // --------------------------------------------------------------------------

  //! Get variable name.
  ASMJIT_INLINE const char* getName() const { return _name; }
  //! Get variable id.
  ASMJIT_INLINE uint32_t getId() const { return _id; }
  //! Get variable type.
  ASMJIT_INLINE uint32_t getType() const { return _type; }
  //! Get variable class.
  ASMJIT_INLINE uint32_t getClass() const { return _class; }

  // --------------------------------------------------------------------------
  // [Accessors - LocalId]
  // --------------------------------------------------------------------------

  //! Get whether the variable has a local id.
  ASMJIT_INLINE bool hasLocalId() const { return _localId != kInvalidValue; }
  //! Get a variable's local id.
  ASMJIT_INLINE uint32_t getLocalId() const { return _localId; }
  //! Set a variable's local id.
  ASMJIT_INLINE void setLocalId(uint32_t localId) { _localId = localId; }
  //! Reset a variable's local id.
  ASMJIT_INLINE void resetLocalId() { _localId = kInvalidValue; }

  // --------------------------------------------------------------------------
  // [Accessors - Priority]
  // --------------------------------------------------------------------------

  //! Get variable priority, used by compiler to decide which variable to spill.
  ASMJIT_INLINE uint32_t getPriority() const { return _priority; }
  //! Set variable priority.
  ASMJIT_INLINE void setPriority(uint32_t priority) {
    ASMJIT_ASSERT(priority <= 0xFF);
    _priority = static_cast<uint8_t>(priority);
  }

  // --------------------------------------------------------------------------
  // [Accessors - State]
  // --------------------------------------------------------------------------

  //! Get variable state, only used by `Context`.
  ASMJIT_INLINE uint32_t getState() const { return _state; }
  //! Set variable state, only used by `Context`.
  ASMJIT_INLINE void setState(uint32_t state) {
    ASMJIT_ASSERT(state <= 0xFF);
    _state = static_cast<uint8_t>(state);
  }

  // --------------------------------------------------------------------------
  // [Accessors - RegIndex]
  // --------------------------------------------------------------------------

  //! Get register index.
  ASMJIT_INLINE uint32_t getRegIndex() const { return _regIndex; }
  //! Set register index.
  ASMJIT_INLINE void setRegIndex(uint32_t regIndex) {
    ASMJIT_ASSERT(regIndex <= kInvalidReg);
    _regIndex = static_cast<uint8_t>(regIndex);
  }
  //! Reset register index.
  ASMJIT_INLINE void resetRegIndex() {
    _regIndex = static_cast<uint8_t>(kInvalidReg);
  }

  // --------------------------------------------------------------------------
  // [Accessors - HomeIndex/Mask]
  // --------------------------------------------------------------------------

  //! Get home registers mask.
  ASMJIT_INLINE uint32_t getHomeMask() const { return _homeMask; }
  //! Add a home register index to the home registers mask.
  ASMJIT_INLINE void addHomeIndex(uint32_t regIndex) { _homeMask |= Utils::mask(regIndex); }

  // --------------------------------------------------------------------------
  // [Accessors - Flags]
  // --------------------------------------------------------------------------

  //! Get variable flags.
  ASMJIT_INLINE uint32_t getFlags() const { return _flags; }

  //! Get whether the VarData is only memory allocated on the stack.
  ASMJIT_INLINE bool isStack() const { return static_cast<bool>(_isStack); }
  //! Get whether the variable is a function argument passed through memory.
  ASMJIT_INLINE bool isMemArg() const { return static_cast<bool>(_isMemArg); }

  //! Get variable content can be calculated by a simple instruction.
  ASMJIT_INLINE bool isCalculated() const { return static_cast<bool>(_isCalculated); }
  //! Get whether to save variable when it's unused (spill).
  ASMJIT_INLINE bool saveOnUnuse() const { return static_cast<bool>(_saveOnUnuse); }

  //! Get whether the variable was changed.
  ASMJIT_INLINE bool isModified() const { return static_cast<bool>(_modified); }
  //! Set whether the variable was changed.
  ASMJIT_INLINE void setModified(bool modified) { _modified = modified; }

  //! Get variable alignment.
  ASMJIT_INLINE uint32_t getAlignment() const { return _alignment; }
  //! Get variable size.
  ASMJIT_INLINE uint32_t getSize() const { return _size; }

  //! Get home memory offset.
  ASMJIT_INLINE int32_t getMemOffset() const { return _memOffset; }
  //! Set home memory offset.
  ASMJIT_INLINE void setMemOffset(int32_t offset) { _memOffset = offset; }

  //! Get home memory cell.
  ASMJIT_INLINE VarCell* getMemCell() const { return _memCell; }
  //! Set home memory cell.
  ASMJIT_INLINE void setMemCell(VarCell* cell) { _memCell = cell; }

  // --------------------------------------------------------------------------
  // [Accessors - Temporary Usage]
  // --------------------------------------------------------------------------

  //! Get temporary VarAttr.
  ASMJIT_INLINE VarAttr* getVa() const { return _va; }
  //! Set temporary VarAttr.
  ASMJIT_INLINE void setVa(VarAttr* va) { _va = va; }
  //! Reset temporary VarAttr.
  ASMJIT_INLINE void resetVa() { _va = nullptr; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Variable name.
  const char* _name;

  //! Variable id.
  uint32_t _id;
  //! Variable's local id (initially `kInvalidValue`).
  uint32_t _localId;

  //! Variable type.
  uint8_t _type;
  //! Variable class.
  uint8_t _class;
  //! Variable flags.
  uint8_t _flags;
  //! Variable priority.
  uint8_t _priority;

  //! Variable state (connected with actual `VarState)`.
  uint8_t _state;
  //! Actual register index (only used by `Context)`, during translate.
  uint8_t _regIndex;

  //! Whether the variable is only used as memory allocated on the stack.
  uint8_t _isStack : 1;
  //! Whether the variable is a function argument passed through memory.
  uint8_t _isMemArg : 1;
  //! Whether variable content can be calculated by a simple instruction.
  //!
  //! This is used mainly by MMX and SSE2 code. This flag indicates that
  //! register allocator should never reserve memory for this variable, because
  //! the content can be generated by a single instruction (for example PXOR).
  uint8_t _isCalculated : 1;
  //! Save on unuse (at end of the variable scope).
  uint8_t _saveOnUnuse : 1;
  //! Whether variable was changed (connected with actual `VarState)`.
  uint8_t _modified : 1;
  //! \internal
  uint8_t _reserved0 : 3;
  //! Variable natural alignment.
  uint8_t _alignment;

  //! Variable size.
  uint32_t _size;

  //! Mask of all registers variable has been allocated to.
  uint32_t _homeMask;

  //! Home memory offset.
  int32_t _memOffset;
  //! Home memory cell, used by `Context` (initially nullptr).
  VarCell* _memCell;

  //! Register read access statistics.
  uint32_t rReadCount;
  //! Register write access statistics.
  uint32_t rWriteCount;

  //! Memory read statistics.
  uint32_t mReadCount;
  //! Memory write statistics.
  uint32_t mWriteCount;

  // --------------------------------------------------------------------------
  // [Members - Temporary Usage]
  // --------------------------------------------------------------------------

  // These variables are only used during register allocation. They are
  // initialized by init() phase and reset by cleanup() phase.

  union {
    //! Temporary link to VarAttr* used by the `Context` used in
    //! various phases, but always set back to nullptr when finished.
    //!
    //! This temporary data is designed to be used by algorithms that need to
    //! store some data into variables themselves during compilation. But it's
    //! expected that after variable is compiled & translated the data is set
    //! back to zero/null. Initial value is nullptr.
    VarAttr* _va;

    //! \internal
    //!
    //! Same as `_va` just provided as `uintptr_t`.
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
    _inRegIndex = kInvalidReg;
    _outRegIndex = kInvalidReg;
    _reserved = 0;
    _inRegs = inRegs;
    _allocableRegs = allocableRegs;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get VarData.
  ASMJIT_INLINE VarData* getVd() const { return _vd; }
  //! Set VarData.
  ASMJIT_INLINE void setVd(VarData* vd) { _vd = vd; }

  //! Get flags.
  ASMJIT_INLINE uint32_t getFlags() const { return _flags; }
  //! Set flags.
  ASMJIT_INLINE void setFlags(uint32_t flags) { _flags = flags; }

  //! Get whether `flag` is on.
  ASMJIT_INLINE bool hasFlag(uint32_t flag) { return (_flags & flag) != 0; }
  //! Add `flags`.
  ASMJIT_INLINE void orFlags(uint32_t flags) { _flags |= flags; }
  //! Mask `flags`.
  ASMJIT_INLINE void andFlags(uint32_t flags) { _flags &= flags; }
  //! Clear `flags`.
  ASMJIT_INLINE void andNotFlags(uint32_t flags) { _flags &= ~flags; }

  //! Get how many times the variable is used by the instruction/node.
  ASMJIT_INLINE uint32_t getVarCount() const { return _varCount; }
  //! Set how many times the variable is used by the instruction/node.
  ASMJIT_INLINE void setVarCount(uint32_t count) { _varCount = static_cast<uint8_t>(count); }
  //! Add how many times the variable is used by the instruction/node.
  ASMJIT_INLINE void addVarCount(uint32_t count = 1) { _varCount += static_cast<uint8_t>(count); }

  //! Get whether the variable has to be allocated in a specific input register.
  ASMJIT_INLINE uint32_t hasInRegIndex() const { return _inRegIndex != kInvalidReg; }
  //! Get the input register index or `kInvalidReg`.
  ASMJIT_INLINE uint32_t getInRegIndex() const { return _inRegIndex; }
  //! Set the input register index.
  ASMJIT_INLINE void setInRegIndex(uint32_t index) { _inRegIndex = static_cast<uint8_t>(index); }
  //! Reset the input register index.
  ASMJIT_INLINE void resetInRegIndex() { _inRegIndex = kInvalidReg; }

  //! Get whether the variable has to be allocated in a specific output register.
  ASMJIT_INLINE uint32_t hasOutRegIndex() const { return _outRegIndex != kInvalidReg; }
  //! Get the output register index or `kInvalidReg`.
  ASMJIT_INLINE uint32_t getOutRegIndex() const { return _outRegIndex; }
  //! Set the output register index.
  ASMJIT_INLINE void setOutRegIndex(uint32_t index) { _outRegIndex = static_cast<uint8_t>(index); }
  //! Reset the output register index.
  ASMJIT_INLINE void resetOutRegIndex() { _outRegIndex = kInvalidReg; }

  //! Get whether the mandatory input registers are in used.
  ASMJIT_INLINE bool hasInRegs() const { return _inRegs != 0; }
  //! Get mandatory input registers (mask).
  ASMJIT_INLINE uint32_t getInRegs() const { return _inRegs; }
  //! Set mandatory input registers (mask).
  ASMJIT_INLINE void setInRegs(uint32_t mask) { _inRegs = mask; }
  //! Add mandatory input registers (mask).
  ASMJIT_INLINE void addInRegs(uint32_t mask) { _inRegs |= mask; }
  //! And mandatory input registers (mask).
  ASMJIT_INLINE void andInRegs(uint32_t mask) { _inRegs &= mask; }
  //! Clear mandatory input registers (mask).
  ASMJIT_INLINE void delInRegs(uint32_t mask) { _inRegs &= ~mask; }

  //! Get allocable input registers (mask).
  ASMJIT_INLINE uint32_t getAllocableRegs() const { return _allocableRegs; }
  //! Set allocable input registers (mask).
  ASMJIT_INLINE void setAllocableRegs(uint32_t mask) { _allocableRegs = mask; }
  //! Add allocable input registers (mask).
  ASMJIT_INLINE void addAllocableRegs(uint32_t mask) { _allocableRegs |= mask; }
  //! And allocable input registers (mask).
  ASMJIT_INLINE void andAllocableRegs(uint32_t mask) { _allocableRegs &= mask; }
  //! Clear allocable input registers (mask).
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
  //! Flags.
  uint32_t _flags;

  union {
    struct {
      //! How many times the variable is used by the instruction/node.
      uint8_t _varCount;
      //! Input register index or `kInvalidReg` if it's not given.
      //!
      //! Even if the input register index is not given (i.e. it may by any
      //! register), register allocator should assign an index that will be
      //! used to persist a variable into this specific index. It's helpful
      //! in situations where one variable has to be allocated in multiple
      //! registers to determine the register which will be persistent.
      uint8_t _inRegIndex;
      //! Output register index or `kInvalidReg` if it's not given.
      //!
      //! Typically `kInvalidReg` if variable is only used on input.
      uint8_t _outRegIndex;
      //! \internal
      uint8_t _reserved;
    };

    //! \internal
    //!
    //! Packed data #0.
    uint32_t _packed;
  };

  //! Mandatory input registers.
  //!
  //! Mandatory input registers are required by the instruction even if
  //! there are duplicates. This schema allows us to allocate one variable
  //! in one or more register when needed. Required mostly by instructions
  //! that have implicit register operands (imul, cpuid, ...) and function
  //! call.
  uint32_t _inRegs;

  //! Allocable input registers.
  //!
  //! Optional input registers is a mask of all allocable registers for a given
  //! variable where we have to pick one of them. This mask is usually not used
  //! when _inRegs is set. If both masks are used then the register
  //! allocator tries first to find an intersection between these and allocates
  //! an extra slot if not found.
  uint32_t _allocableRegs;
};

// ============================================================================
// [asmjit::VarMap]
// ============================================================================

//! Variables' map related to a single node (instruction / other node).
struct VarMap {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get count of variables (all).
  ASMJIT_INLINE uint32_t getVaCount() const {
    return _vaCount;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Variables count.
  uint32_t _vaCount;
};

// ============================================================================
// [asmjit::VarState]
// ============================================================================

//! Variables' state.
struct VarState {};

// ============================================================================
// [asmjit::Context]
// ============================================================================

//! \internal
//!
//! Code generation context is the logic behind `Compiler`. The context is
//! used to compile the code stored in `Compiler`.
struct Context {
  ASMJIT_NO_COPY(Context)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  Context(Compiler* compiler);
  virtual ~Context();

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the whole context.
  virtual void reset(bool releaseMemory = false);

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get compiler.
  ASMJIT_INLINE Compiler* getCompiler() const { return _compiler; }

  //! Get function.
  ASMJIT_INLINE HLFunc* getFunc() const { return _func; }
  //! Get stop node.
  ASMJIT_INLINE HLNode* getStop() const { return _stop; }

  //! Get start of the current scope.
  ASMJIT_INLINE HLNode* getStart() const { return _start; }
  //! Get end of the current scope.
  ASMJIT_INLINE HLNode* getEnd() const { return _end; }

  //! Get extra block.
  ASMJIT_INLINE HLNode* getExtraBlock() const { return _extraBlock; }
  //! Set extra block.
  ASMJIT_INLINE void setExtraBlock(HLNode* node) { _extraBlock = node; }

  // --------------------------------------------------------------------------
  // [Error]
  // --------------------------------------------------------------------------

  //! Get the last error code.
  ASMJIT_INLINE Error getLastError() const {
    return getCompiler()->getLastError();
  }

  //! Set the last error code and propagate it through the error handler.
  ASMJIT_INLINE Error setLastError(Error error, const char* message = nullptr) {
    return getCompiler()->setLastError(error, message);
  }

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! Get current state.
  ASMJIT_INLINE VarState* getState() const { return _state; }

  //! Load current state from `target` state.
  virtual void loadState(VarState* src) = 0;

  //! Save current state, returning new `VarState` instance.
  virtual VarState* saveState() = 0;

  //! Change the current state to `target` state.
  virtual void switchState(VarState* src) = 0;

  //! Change the current state to the intersection of two states `a` and `b`.
  virtual void intersectStates(VarState* a, VarState* b) = 0;

  // --------------------------------------------------------------------------
  // [Context]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error _registerContextVar(VarData* vd) {
    if (vd->hasLocalId())
      return kErrorOk;

    uint32_t cid = static_cast<uint32_t>(_contextVd.getLength());
    ASMJIT_PROPAGATE_ERROR(_contextVd.append(vd));

    vd->setLocalId(cid);
    return kErrorOk;
  }

  // --------------------------------------------------------------------------
  // [Mem]
  // --------------------------------------------------------------------------

  VarCell* _newVarCell(VarData* vd);
  VarCell* _newStackCell(uint32_t size, uint32_t alignment);

  ASMJIT_INLINE VarCell* getVarCell(VarData* vd) {
    VarCell* cell = vd->getMemCell();
    return cell ? cell : _newVarCell(vd);
  }

  virtual Error resolveCellOffsets();

  // --------------------------------------------------------------------------
  // [Bits]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE BitArray* newBits(uint32_t len) {
    return static_cast<BitArray*>(
      _zoneAllocator.allocZeroed(static_cast<size_t>(len) * BitArray::kEntitySize));
  }

  ASMJIT_INLINE BitArray* copyBits(const BitArray* src, uint32_t len) {
    return static_cast<BitArray*>(
      _zoneAllocator.dup(src, static_cast<size_t>(len) * BitArray::kEntitySize));
  }

  // --------------------------------------------------------------------------
  // [Fetch]
  // --------------------------------------------------------------------------

  //! Fetch.
  //!
  //! Fetch iterates over all nodes and gathers information about all variables
  //! used. The process generates information required by register allocator,
  //! variable liveness analysis and translator.
  virtual Error fetch() = 0;

  // --------------------------------------------------------------------------
  // [Unreachable Code]
  // --------------------------------------------------------------------------

  //! Add unreachable-flow data to the unreachable flow list.
  ASMJIT_INLINE Error addUnreachableNode(HLNode* node) {
    PodList<HLNode*>::Link* link = _zoneAllocator.allocT<PodList<HLNode*>::Link>();
    if (link == nullptr)
      return setLastError(kErrorNoHeapMemory);

    link->setValue(node);
    _unreachableList.append(link);

    return kErrorOk;
  }

  //! Remove unreachable code.
  virtual Error removeUnreachableCode();

  // --------------------------------------------------------------------------
  // [Code-Flow]
  // --------------------------------------------------------------------------

  //! Add returning node (i.e. node that returns and where liveness analysis
  //! should start).
  ASMJIT_INLINE Error addReturningNode(HLNode* node) {
    PodList<HLNode*>::Link* link = _zoneAllocator.allocT<PodList<HLNode*>::Link>();
    if (link == nullptr)
      return setLastError(kErrorNoHeapMemory);

    link->setValue(node);
    _returningList.append(link);

    return kErrorOk;
  }

  //! Add jump-flow data to the jcc flow list.
  ASMJIT_INLINE Error addJccNode(HLNode* node) {
    PodList<HLNode*>::Link* link = _zoneAllocator.allocT<PodList<HLNode*>::Link>();
    if (link == nullptr)
      return setLastError(kErrorNoHeapMemory);

    link->setValue(node);
    _jccList.append(link);

    return kErrorOk;
  }

  // --------------------------------------------------------------------------
  // [Analyze]
  // --------------------------------------------------------------------------

  //! Perform variable liveness analysis.
  //!
  //! Analysis phase iterates over nodes in reverse order and generates a bit
  //! array describing variables that are alive at every node in the function.
  //! When the analysis start all variables are assumed dead. When a read or
  //! read/write operations of a variable is detected the variable becomes
  //! alive; when only write operation is detected the variable becomes dead.
  //!
  //! When a label is found all jumps to that label are followed and analysis
  //! repeats until all variables are resolved.
  virtual Error livenessAnalysis();

  // --------------------------------------------------------------------------
  // [Annotate]
  // --------------------------------------------------------------------------

  virtual Error annotate() = 0;
  virtual Error formatInlineComment(StringBuilder& dst, HLNode* node);

  // --------------------------------------------------------------------------
  // [Translate]
  // --------------------------------------------------------------------------

  //! Translate code by allocating registers and handling state changes.
  virtual Error translate() = 0;

  // --------------------------------------------------------------------------
  // [Cleanup]
  // --------------------------------------------------------------------------

  virtual void cleanup();

  // --------------------------------------------------------------------------
  // [Compile]
  // --------------------------------------------------------------------------

  virtual Error compile(HLFunc* func);

  // --------------------------------------------------------------------------
  // [Serialize]
  // --------------------------------------------------------------------------

  virtual Error serialize(Assembler* assembler, HLNode* start, HLNode* stop) = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Compiler.
  Compiler* _compiler;
  //! Function.
  HLFunc* _func;

  //! Zone allocator.
  Zone _zoneAllocator;

  //! \internal
  typedef void (ASMJIT_CDECL* TraceNodeFunc)(Context* self, HLNode* node_, const char* prefix);
  //! \internal
  //!
  //! Only non-NULL when ASMJIT_TRACE is enabled.
  TraceNodeFunc _traceNode;

  //! \internal
  //!
  //! Offset (how many bytes to add) to `VarMap` to get `VarAttr` array. Used
  //! by liveness analysis shared across all backends. This is needed because
  //! `VarMap` is a base class for a specialized version that liveness analysis
  //! doesn't use, it just needs `VarAttr` array.
  uint32_t _varMapToVaListOffset;

  //! Start of the current active scope.
  HLNode* _start;
  //! End of the current active scope.
  HLNode* _end;

  //! Node that is used to insert extra code after the function body.
  HLNode* _extraBlock;
  //! Stop node.
  HLNode* _stop;

  //! Unreachable nodes.
  PodList<HLNode*> _unreachableList;
  //! Returning nodes.
  PodList<HLNode*> _returningList;
  //! Jump nodes.
  PodList<HLNode*> _jccList;

  //! All variables used by the current function.
  PodVector<VarData*> _contextVd;

  //! Memory used to spill variables.
  VarCell* _memVarCells;
  //! Memory used to alloc memory on the stack.
  VarCell* _memStackCells;

  //! Count of 1-byte cells.
  uint32_t _mem1ByteVarsUsed;
  //! Count of 2-byte cells.
  uint32_t _mem2ByteVarsUsed;
  //! Count of 4-byte cells.
  uint32_t _mem4ByteVarsUsed;
  //! Count of 8-byte cells.
  uint32_t _mem8ByteVarsUsed;
  //! Count of 16-byte cells.
  uint32_t _mem16ByteVarsUsed;
  //! Count of 32-byte cells.
  uint32_t _mem32ByteVarsUsed;
  //! Count of 64-byte cells.
  uint32_t _mem64ByteVarsUsed;
  //! Count of stack memory cells.
  uint32_t _memStackCellsUsed;

  //! Maximum memory alignment used by the function.
  uint32_t _memMaxAlign;
  //! Count of bytes used by variables.
  uint32_t _memVarTotal;
  //! Count of bytes used by stack.
  uint32_t _memStackTotal;
  //! Count of bytes used by variables and stack after alignment.
  uint32_t _memAllTotal;

  //! Default lenght of annotated instruction.
  uint32_t _annotationLength;

  //! Current state (used by register allocator).
  VarState* _state;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_COMPILERCONTEXT_P_H
