// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_RAPASS_P_H
#define _ASMJIT_CORE_RAPASS_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

#include "../core/raassignment_p.h"
#include "../core/radefs_p.h"
#include "../core/rastack_p.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core_ra
//! \{

// ============================================================================
// [asmjit::RABlock]
// ============================================================================

class RABlock {
public:
  ASMJIT_NONCOPYABLE(RABlock)

  enum Id : uint32_t {
    kUnassignedId         = 0xFFFFFFFFu
  };

  enum Flags : uint32_t {
    kFlagIsConstructed    = 0x00000001u, //!< Block has been constructed from nodes.
    kFlagIsReachable      = 0x00000002u, //!< Block is reachable (set by `buildViews()`).
    kFlagIsAllocated      = 0x00000004u, //!< Block has been allocated.
    kFlagIsFuncExit       = 0x00000008u, //!< Block is a function-exit.

    kFlagHasTerminator    = 0x00000010u, //!< Block has a terminator (jump, conditional jump, ret).
    kFlagHasConsecutive   = 0x00000020u, //!< Block naturally flows to the next block.
    kFlagHasFixedRegs     = 0x00000040u, //!< Block contains fixed registers (precolored).
    kFlagHasFuncCalls     = 0x00000080u  //!< Block contains function calls.
  };

  enum LiveType : uint32_t {
    kLiveIn               = 0,
    kLiveOut              = 1,
    kLiveGen              = 2,
    kLiveKill             = 3,
    kLiveCount            = 4
  };

  // --------------------------------------------------------------------------
  // [Typedefs]
  // --------------------------------------------------------------------------

  typedef RAAssignment::PhysToWorkMap PhysToWorkMap;
  typedef RAAssignment::WorkToPhysMap WorkToPhysMap;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline RABlock(RAPass* ra) noexcept
    : _ra(ra),
      _blockId(kUnassignedId),
      _flags(0),
      _first(nullptr),
      _last(nullptr),
      _firstPosition(0),
      _endPosition(0),
      _weight(0),
      _povOrder(kUnassignedId),
      _regsStats(),
      _maxLiveCount(),
      _timestamp(0),
      _idom(nullptr),
      _predecessors(),
      _successors(),
      _entryPhysToWorkMap(nullptr),
      _entryWorkToPhysMap(nullptr) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline RAPass* pass() const noexcept { return _ra; }
  inline ZoneAllocator* allocator() const noexcept;

  inline uint32_t blockId() const noexcept { return _blockId; }
  inline uint32_t flags() const noexcept { return _flags; }

  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }

  inline bool isAssigned() const noexcept { return _blockId != kUnassignedId; }

  inline bool isConstructed() const noexcept { return hasFlag(kFlagIsConstructed); }
  inline bool isReachable() const noexcept { return hasFlag(kFlagIsReachable); }
  inline bool isAllocated() const noexcept { return hasFlag(kFlagIsAllocated); }
  inline bool isFuncExit() const noexcept { return hasFlag(kFlagIsFuncExit); }

  inline void makeConstructed(const RARegsStats& regStats) noexcept {
    _flags |= kFlagIsConstructed;
    _regsStats.combineWith(regStats);
  }

  inline void makeReachable() noexcept { _flags |= kFlagIsReachable; }
  inline void makeAllocated() noexcept { _flags |= kFlagIsAllocated; }

  inline const RARegsStats& regsStats() const noexcept { return _regsStats; }

  inline bool hasTerminator() const noexcept { return hasFlag(kFlagHasTerminator); }
  inline bool hasConsecutive() const noexcept { return hasFlag(kFlagHasConsecutive); }

  inline bool hasPredecessors() const noexcept { return !_predecessors.empty(); }
  inline bool hasSuccessors() const noexcept { return !_successors.empty(); }

  inline const RABlocks& predecessors() const noexcept { return _predecessors; }
  inline const RABlocks& successors() const noexcept { return _successors; }

  inline BaseNode* first() const noexcept { return _first; }
  inline BaseNode* last() const noexcept { return _last; }

  inline void setFirst(BaseNode* node) noexcept { _first = node; }
  inline void setLast(BaseNode* node) noexcept { _last = node; }

  inline uint32_t firstPosition() const noexcept { return _firstPosition; }
  inline void setFirstPosition(uint32_t position) noexcept { _firstPosition = position; }

  inline uint32_t endPosition() const noexcept { return _endPosition; }
  inline void setEndPosition(uint32_t position) noexcept { _endPosition = position; }

  inline uint32_t povOrder() const noexcept { return _povOrder; }

  inline uint64_t timestamp() const noexcept { return _timestamp; }
  inline bool hasTimestamp(uint64_t ts) const noexcept { return _timestamp == ts; }
  inline void setTimestamp(uint64_t ts) const noexcept { _timestamp = ts; }
  inline void resetTimestamp() const noexcept { _timestamp = 0; }

  inline RABlock* consecutive() const noexcept { return hasConsecutive() ? _successors[0] : nullptr; }

  inline RABlock* iDom() noexcept { return _idom; }
  inline const RABlock* iDom() const noexcept { return _idom; }
  inline void setIDom(RABlock* block) noexcept { _idom = block; }

  inline ZoneBitVector& liveIn() noexcept { return _liveBits[kLiveIn]; }
  inline const ZoneBitVector& liveIn() const noexcept { return _liveBits[kLiveIn]; }

  inline ZoneBitVector& liveOut() noexcept { return _liveBits[kLiveOut]; }
  inline const ZoneBitVector& liveOut() const noexcept { return _liveBits[kLiveOut]; }

  inline ZoneBitVector& gen() noexcept { return _liveBits[kLiveGen]; }
  inline const ZoneBitVector& gen() const noexcept { return _liveBits[kLiveGen]; }

  inline ZoneBitVector& kill() noexcept { return _liveBits[kLiveKill]; }
  inline const ZoneBitVector& kill() const noexcept { return _liveBits[kLiveKill]; }

  inline Error resizeLiveBits(uint32_t size) noexcept {
    ASMJIT_PROPAGATE(_liveBits[kLiveIn  ].resize(allocator(), size));
    ASMJIT_PROPAGATE(_liveBits[kLiveOut ].resize(allocator(), size));
    ASMJIT_PROPAGATE(_liveBits[kLiveGen ].resize(allocator(), size));
    ASMJIT_PROPAGATE(_liveBits[kLiveKill].resize(allocator(), size));
    return kErrorOk;
  }

  inline bool hasEntryAssignment() const noexcept { return _entryPhysToWorkMap != nullptr; }
  inline WorkToPhysMap* entryWorkToPhysMap() const noexcept { return _entryWorkToPhysMap; }
  inline PhysToWorkMap* entryPhysToWorkMap() const noexcept { return _entryPhysToWorkMap; }

  inline void setEntryAssignment(PhysToWorkMap* physToWorkMap, WorkToPhysMap* workToPhysMap) noexcept {
    _entryPhysToWorkMap = physToWorkMap;
    _entryWorkToPhysMap = workToPhysMap;
  }

  // --------------------------------------------------------------------------
  // [Control Flow]
  // --------------------------------------------------------------------------

  //! Adds a successor to this block, and predecessor to `successor`, making
  //! connection on both sides.
  //!
  //! This API must be used to manage successors and predecessors, never manage
  //! it manually.
  Error appendSuccessor(RABlock* successor) noexcept;

  //! Similar to `appendSuccessor()`, but does prepend instead append.
  //!
  //! This function is used to add a natural flow (always first) to the block.
  Error prependSuccessor(RABlock* successor) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  RAPass* _ra;                           //!< Register allocator pass.

  uint32_t _blockId;                     //!< Block id (indexed from zero).
  uint32_t _flags;                       //!< Block flags, see `Flags`.

  BaseNode* _first;                      //!< First `BaseNode` of this block (inclusive).
  BaseNode* _last;                       //!< Last `BaseNode` of this block (inclusive).

  uint32_t _firstPosition;               //!< Initial position of this block (inclusive).
  uint32_t _endPosition;                 //!< End position of this block (exclusive).

  uint32_t _weight;                      //!< Weight of this block (default 0, each loop adds one).
  uint32_t _povOrder;                    //!< Post-order view order, used during POV construction.
  RARegsStats _regsStats;                //!< Basic statistics about registers.
  RALiveCount _maxLiveCount;             //!< Maximum live-count per register group.

  mutable uint64_t _timestamp;           //!< Timestamp (used by block visitors).
  RABlock* _idom;                        //!< Immediate dominator of this block.

  RABlocks _predecessors;                //!< Block predecessors.
  RABlocks _successors;                  //!< Block successors.

  // TODO: Used?
  RABlocks _doms;

  ZoneBitVector _liveBits[kLiveCount];   //!< Liveness in/out/use/kill.

  PhysToWorkMap* _entryPhysToWorkMap;    //!< Register assignment (PhysToWork) on entry.
  WorkToPhysMap* _entryWorkToPhysMap;    //!< Register assignment (WorkToPhys) on entry.
};

// ============================================================================
// [asmjit::RAInst]
// ============================================================================

//! Register allocator's data associated with each `InstNode`.
class RAInst {
public:
  ASMJIT_NONCOPYABLE(RAInst)

  enum Flags : uint32_t {
    kFlagIsTerminator = 0x00000001u
  };

  static inline size_t sizeOf(uint32_t tiedRegCount) noexcept {
    return sizeof(RAInst) - sizeof(RATiedReg) + tiedRegCount * sizeof(RATiedReg);
  }

  ASMJIT_INLINE RAInst(RABlock* block, uint32_t flags, uint32_t tiedTotal, const RARegMask& clobberedRegs) noexcept {
    _block = block;
    _flags = flags;
    _tiedTotal = tiedTotal;
    _tiedIndex.reset();
    _tiedCount.reset();
    _liveCount.reset();
    _usedRegs.reset();
    _clobberedRegs = clobberedRegs;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets the instruction flags.
  inline uint32_t flags() const noexcept { return _flags; }
  //! Gets whether the instruction has flag `flag`.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  //! Replaces the existing instruction flags with `flags`.
  inline void setFlags(uint32_t flags) noexcept { _flags = flags; }
  //! Adds instruction `flags` to this RAInst.
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
  //! Clears instruction `flags` from  this RAInst.
  inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

  //! Gets whether the node is code that can be executed.
  inline bool isTerminator() const noexcept { return hasFlag(kFlagIsTerminator); }

  //! Gets the associated block with this RAInst.
  inline RABlock* block() const noexcept { return _block; }

  //! Gets tied registers (all).
  inline RATiedReg* tiedRegs() const noexcept { return const_cast<RATiedReg*>(_tiedRegs); }
  //! Gets tied registers for a given `group`.
  inline RATiedReg* tiedRegs(uint32_t group) const noexcept { return const_cast<RATiedReg*>(_tiedRegs) + _tiedIndex.get(group); }

  //! Gets count of all tied registers.
  inline uint32_t tiedCount() const noexcept { return _tiedTotal; }
  //! Gets count of tied registers of a given `group`.
  inline uint32_t tiedCount(uint32_t group) const noexcept { return _tiedCount[group]; }

  //! Gets `RATiedReg` at the specified `index`.
  inline RATiedReg* tiedAt(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _tiedTotal);
    return tiedRegs() + index;
  }

  //! Gets `RATiedReg` at the specified index for a given register `group`.
  inline RATiedReg* tiedOf(uint32_t group, uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _tiedCount._regs[group]);
    return tiedRegs(group) + index;
  }

  inline void setTiedAt(uint32_t index, RATiedReg& tied) noexcept {
    ASMJIT_ASSERT(index < _tiedTotal);
    _tiedRegs[index] = tied;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  RABlock* _block;                       //!< Parent block.
  uint32_t _flags;                       //!< Flags.
  uint32_t _tiedTotal;                   //!< Total count of RATiedReg's.
  RARegIndex _tiedIndex;                 //!< Index of RATiedReg's per register group.
  RARegCount _tiedCount;                 //!< Count of RATiedReg's per register group.
  RALiveCount _liveCount;                //!< Number of live, and thus interfering VirtReg's at this point.
  RARegMask _usedRegs;                   //!< Fixed physical registers used.
  RARegMask _clobberedRegs;              //!< Clobbered registers (by a function call).
  RATiedReg _tiedRegs[1];                //!< Tied registers.
};

// ============================================================================
// [asmjit::RAInstBuilder]
// ============================================================================

//! A helper class that is used to build an array of RATiedReg items that are
//! then copied to `RAInst`.
class RAInstBuilder {
public:
  ASMJIT_NONCOPYABLE(RAInstBuilder)

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline RAInstBuilder() noexcept { reset(); }

  inline void init() noexcept { reset(); }
  inline void reset() noexcept {
    _count.reset();
    _stats.reset();
    _used.reset();
    _clobbered.reset();
    _cur = _tiedRegs;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline uint32_t flags() const noexcept { return _flags; }
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }

  //! Gets the number of tied registers added to the builder.
  inline uint32_t tiedRegCount() const noexcept { return uint32_t((size_t)(_cur - _tiedRegs)); }

  //! Gets a tied register at index `index`.
  inline RATiedReg* operator[](uint32_t index) noexcept {
    ASMJIT_ASSERT(index < tiedRegCount());
    return &_tiedRegs[index];
  }

  //! Gets a tied register at index `index` (const).
  inline const RATiedReg* operator[](uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < tiedRegCount());
    return &_tiedRegs[index];
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE Error add(RAWorkReg* workReg, uint32_t flags, uint32_t allocable, uint32_t useId, uint32_t useRewriteMask, uint32_t outId, uint32_t outRewriteMask) noexcept {
    uint32_t group = workReg->group();
    RATiedReg* tiedReg = workReg->tiedReg();

    if (useId != BaseReg::kIdBad) {
      _stats.makeFixed(group);
      _used[group] |= Support::bitMask(useId);
      flags |= RATiedReg::kUseFixed;
    }

    if (outId != BaseReg::kIdBad) {
      _clobbered[group] |= Support::bitMask(outId);
      flags |= RATiedReg::kOutFixed;
    }

    _flags |= flags;
    _stats.makeUsed(group);

    if (!tiedReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(tiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg->workId(), flags, allocable, useId, useRewriteMask, outId, outRewriteMask);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      if (useId != BaseReg::kIdBad) {
        if (ASMJIT_UNLIKELY(tiedReg->hasUseId()))
          return DebugUtils::errored(kErrorOverlappedRegs);
        tiedReg->setUseId(useId);
      }

      if (outId != BaseReg::kIdBad) {
        if (ASMJIT_UNLIKELY(tiedReg->hasOutId()))
          return DebugUtils::errored(kErrorOverlappedRegs);
        tiedReg->setOutId(outId);
        // TODO: ? _used[group] |= Support::bitMask(outId);
      }

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      tiedReg->_allocableRegs &= allocable;
      tiedReg->_useRewriteMask |= useRewriteMask;
      tiedReg->_outRewriteMask |= outRewriteMask;
      return kErrorOk;
    }
  }

  ASMJIT_INLINE Error addCallArg(RAWorkReg* workReg, uint32_t useId) noexcept {
    ASMJIT_ASSERT(useId != BaseReg::kIdBad);

    uint32_t flags = RATiedReg::kUse | RATiedReg::kRead | RATiedReg::kUseFixed;
    uint32_t group = workReg->group();
    uint32_t allocable = Support::bitMask(useId);

    _flags |= flags;
    _used[group] |= allocable;
    _stats.makeFixed(group);
    _stats.makeUsed(group);

    RATiedReg* tiedReg = workReg->tiedReg();
    if (!tiedReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(tiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg->workId(), flags, allocable, useId, 0, BaseReg::kIdBad, 0);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      if (tiedReg->hasUseId()) {
        flags |= RATiedReg::kDuplicate;
        tiedReg->_allocableRegs |= allocable;
      }
      else {
        tiedReg->setUseId(useId);
        tiedReg->_allocableRegs &= allocable;
      }

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      return kErrorOk;
    }
  }

  ASMJIT_INLINE Error addCallRet(RAWorkReg* workReg, uint32_t outId) noexcept {
    ASMJIT_ASSERT(outId != BaseReg::kIdBad);

    uint32_t flags = RATiedReg::kOut | RATiedReg::kWrite | RATiedReg::kOutFixed;
    uint32_t group = workReg->group();
    uint32_t allocable = Support::bitMask(outId);

    _flags |= flags;
    _used[group] |= allocable;
    _stats.makeFixed(group);
    _stats.makeUsed(group);

    RATiedReg* tiedReg = workReg->tiedReg();
    if (!tiedReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(tiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg->workId(), flags, allocable, BaseReg::kIdBad, 0, outId, 0);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      if (tiedReg->hasOutId())
        return DebugUtils::errored(kErrorOverlappedRegs);

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      tiedReg->setOutId(outId);
      return kErrorOk;
    }
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Flags combined from all RATiedReg's.
  uint32_t _flags;
  RARegCount _count;
  RARegsStats _stats;

  RARegMask _used;
  RARegMask _clobbered;

  //! Current tied register in `_tiedRegs`.
  RATiedReg* _cur;
  //! Array of temporary tied registers.
  RATiedReg _tiedRegs[128];
};

// ============================================================================
// [asmjit::RAPass]
// ============================================================================

//! Register allocation pass used by `BaseCompiler`.
class RAPass : public FuncPass {
public:
  ASMJIT_NONCOPYABLE(RAPass)
  typedef FuncPass Base;

  enum Weights : uint32_t {
    kCallArgWeight = 80
  };

  // --------------------------------------------------------------------------
  // [Typedefs]
  // --------------------------------------------------------------------------

  typedef RAAssignment::PhysToWorkMap PhysToWorkMap;
  typedef RAAssignment::WorkToPhysMap WorkToPhysMap;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  RAPass() noexcept;
  virtual ~RAPass() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets `Logger` passed to `runOnFunction()`.
  inline Logger* logger() const noexcept { return _logger; }
  //! Gets `Logger` passed to `runOnFunction()` or null if `kOptionDebugPasses` is not set.
  inline Logger* debugLogger() const noexcept { return _debugLogger; }

  //! Gets `Zone` passed to `runOnFunction()`.
  inline Zone* zone() const noexcept { return _allocator.zone(); }
  //! Gets `ZoneAllocator` used by the register allocator.
  inline ZoneAllocator* allocator() const noexcept { return const_cast<ZoneAllocator*>(&_allocator); }

  //! Gets the current function node.
  inline FuncNode* func() const noexcept { return _func; }
  //! Gets the stop of the current function.
  inline BaseNode* stop() const noexcept { return _stop; }

  //! Gets an extra block.
  inline BaseNode* extraBlock() const noexcept { return _extraBlock; }
  //! Sets an extra block.
  inline void setExtraBlock(BaseNode* node) noexcept { _extraBlock = node; }

  inline uint32_t endPosition() const noexcept { return _instructionCount * 2; }

  inline const RARegMask& availableRegs() const noexcept { return _availableRegs; }
  inline const RARegMask& cloberredRegs() const noexcept { return _clobberedRegs; }

  inline void makeUnavailable(uint32_t group, uint32_t regId) noexcept {
    _availableRegs[group] &= ~Support::bitMask(regId);
    _availableRegCount[group]--;
  }

  // --------------------------------------------------------------------------
  // [RunOnFunction / RunAllocation]
  // --------------------------------------------------------------------------

  //! Runs the register allocator for the given `func`.
  Error runOnFunction(Zone* zone, Logger* logger, FuncNode* func) noexcept override;

  //! Performs all allocation steps sequentially, called by `runOnFunction()`.
  Error onPerformAllSteps() noexcept;

  // --------------------------------------------------------------------------
  // [OnInit / OnDone]
  // --------------------------------------------------------------------------

  //! Called by `runOnFunction()` before the register allocation to initialize
  //! architecture-specific data and constraints.
  virtual void onInit() noexcept = 0;

  //! Called by `runOnFunction()` after register allocation to clean everything
  //! up. Called even if the register allocation failed.
  virtual void onDone() noexcept = 0;

  // --------------------------------------------------------------------------
  // [CFG - Basic Block Management]
  // --------------------------------------------------------------------------

  //! Gets entry block of the current function.
  inline RABlock* entryBlock() noexcept {
    ASMJIT_ASSERT(!_blocks.empty());
    return _blocks[0];
  }

  //! \overload
  inline const RABlock* entryBlock() const noexcept {
    ASMJIT_ASSERT(!_blocks.empty());
    return _blocks[0];
  }

  //! Gets count of basic blocks (returns size of `_blocks` array).
  inline uint32_t blockCount() const noexcept { return _blocks.size(); }
  //! Gets count of reachable basic blocks (returns size of `_pov` array).
  inline uint32_t reachableBlockCount() const noexcept { return _pov.size(); }

  //! Gets whether the CFG has dangling blocks - these were created by `newBlock()`,
  //! but not added to CFG through `addBlocks()`. If `true` is returned and the
  //! CFG is constructed it means that something is missing and it's incomplete.
  //!
  //! NOTE: This is only used to check if the number of created blocks matches
  //! the number of added blocks.
  inline bool hasDanglingBlocks() const noexcept { return _createdBlockCount != blockCount(); }

  //! Gest a next timestamp to be used to mark CFG blocks.
  inline uint64_t nextTimestamp() const noexcept { return ++_lastTimestamp; }

  //! Createss a new `RABlock` instance.
  //!
  //! NOTE: New blocks don't have ID assigned until they are added to the block
  //! array by calling `addBlock()`.
  RABlock* newBlock(BaseNode* initialNode = nullptr) noexcept;

  //! Tries to find a neighboring LabelNode (without going through code) that is
  //! already connected with `RABlock`. If no label is found then a new RABlock
  //! is created and assigned to all possible labels in a backward direction.
  RABlock* newBlockOrExistingAt(LabelNode* cbLabel, BaseNode** stoppedAt = nullptr) noexcept;

  //! Adds the given `block` to the block list and assign it a unique block id.
  Error addBlock(RABlock* block) noexcept;

  inline Error addExitBlock(RABlock* block) noexcept {
    block->addFlags(RABlock::kFlagIsFuncExit);
    return _exits.append(allocator(), block);
  }

  ASMJIT_INLINE RAInst* newRAInst(RABlock* block, uint32_t flags, uint32_t tiedRegCount, const RARegMask& clobberedRegs) noexcept {
    void* p = zone()->alloc(RAInst::sizeOf(tiedRegCount));
    if (ASMJIT_UNLIKELY(!p))
      return nullptr;
    return new(p) RAInst(block, flags, tiedRegCount, clobberedRegs);
  }

  ASMJIT_INLINE Error assignRAInst(BaseNode* node, RABlock* block, RAInstBuilder& ib) noexcept {
    uint32_t tiedRegCount = ib.tiedRegCount();
    RAInst* raInst = newRAInst(block, ib.flags(), tiedRegCount, ib._clobbered);

    if (ASMJIT_UNLIKELY(!raInst))
      return DebugUtils::errored(kErrorOutOfMemory);

    RARegIndex index;
    index.buildIndexes(ib._count);

    raInst->_tiedIndex = index;
    raInst->_tiedCount = ib._count;

    for (uint32_t i = 0; i < tiedRegCount; i++) {
      RATiedReg* tiedReg = ib[i];
      RAWorkReg* workReg = workRegById(tiedReg->workId());

      workReg->resetTiedReg();
      uint32_t group = workReg->group();

      if (tiedReg->hasUseId()) {
        block->addFlags(RABlock::kFlagHasFixedRegs);
        raInst->_usedRegs[group] |= Support::bitMask(tiedReg->useId());
      }

      if (tiedReg->hasOutId()) {
        block->addFlags(RABlock::kFlagHasFixedRegs);
      }

      RATiedReg& dst = raInst->_tiedRegs[index[group]++];
      dst = *tiedReg;
      if (!tiedReg->isDuplicate())
        dst._allocableRegs &= ~ib._used[group];
    }

    node->setPassData<RAInst>(raInst);
    return kErrorOk;
  }

  // --------------------------------------------------------------------------
  // [CFG - Build CFG]
  // --------------------------------------------------------------------------

  //! Traverse the whole function and do the following:
  //!
  //!   1. Construct CFG (represented by `RABlock`) by populating `_blocks` and
  //!      `_exits`. Blocks describe the control flow of the function and contain
  //!      some additional information that is used by the register allocator.
  //!
  //!   2. Remove unreachable code immediately. This is not strictly necessary
  //!      for BaseCompiler itself as the register allocator cannot reach such
  //!      nodes, but keeping instructions that use virtual registers would fail
  //!      during instruction encoding phase (Assembler).
  //!
  //!   3. `RAInst` is created for each `InstNode` or compatible. It contains
  //!      information that is essential for further analysis and register
  //!      allocation.
  //!
  //! Use `RACFGBuilder` template that provides the necessary boilerplate.
  virtual Error buildCFG() noexcept = 0;

  // --------------------------------------------------------------------------
  // [CFG - Views Order]
  // --------------------------------------------------------------------------

  //! Constructs CFG views (only POV at the moment).
  Error buildViews() noexcept;

  // --------------------------------------------------------------------------
  // [CFG - Dominators]
  // --------------------------------------------------------------------------

  // Terminology:
  //   - A node `X` dominates a node `Z` if any path from the entry point to
  //     `Z` has to go through `X`.
  //   - A node `Z` post-dominates a node `X` if any path from `X` to the end
  //     of the graph has to go through `Z`.

  //! Constructs a dominator-tree from CFG.
  Error buildDominators() noexcept;

  bool _strictlyDominates(const RABlock* a, const RABlock* b) const noexcept;
  const RABlock* _nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept;

  //! Gets whether basic block `a` dominates `b` - non-strict, returns true when `a == b`.
  inline bool dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? true : _strictlyDominates(a, b); }
  //! Gets whether basic block `a` dominates `b` - strict dominance check, returns false when `a == b`.
  inline bool strictlyDominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? false : _strictlyDominates(a, b); }

  //! Gets a nearest common dominator of `a` and `b`.
  inline RABlock* nearestCommonDominator(RABlock* a, RABlock* b) const noexcept { return const_cast<RABlock*>(_nearestCommonDominator(a, b)); }
  //! Gets a nearest common dominator of `a` and `b` (const).
  inline const RABlock* nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept { return _nearestCommonDominator(a, b); }

  // --------------------------------------------------------------------------
  // [CFG - Utilities]
  // --------------------------------------------------------------------------

  Error removeUnreachableBlocks() noexcept;

  //! Returns `node` or some node after that is ideal for beginning a new block.
  //! This function is mostly used after a conditional or unconditional jump to
  //! select the successor node. In some cases the next node could be a label,
  //! which means it could have assigned some block already.
  BaseNode* findSuccessorStartingAt(BaseNode* node) noexcept;

  //! Returns `true` of the `node` can flow to `target` without reaching code
  //! nor data. It's used to eliminate jumps to labels that are next right to
  //! them.
  bool isNextTo(BaseNode* node, BaseNode* target) noexcept;

  // --------------------------------------------------------------------------
  // [Registers - Management]
  // --------------------------------------------------------------------------

  //! Gets a native size of a general-purpose register.
  inline uint32_t gpSize() const noexcept { return _sp.size(); }
  inline uint32_t availableRegCount(uint32_t group) const noexcept { return _availableRegCount[group]; }

  inline RAWorkReg* workRegById(uint32_t workId) const noexcept { return _workRegs[workId]; }

  inline RAWorkRegs& workRegs() noexcept { return _workRegs; }
  inline RAWorkRegs& workRegs(uint32_t group) noexcept { return _workRegsOfGroup[group]; }

  inline const RAWorkRegs& workRegs() const noexcept { return _workRegs; }
  inline const RAWorkRegs& workRegs(uint32_t group) const noexcept { return _workRegsOfGroup[group]; }

  inline uint32_t workRegCount() const noexcept { return _workRegs.size(); }
  inline uint32_t workRegCount(uint32_t group) const noexcept { return _workRegsOfGroup[group].size(); }

  inline void _buildPhysIndex() noexcept {
    _physRegIndex.buildIndexes(_physRegCount);
    _physRegTotal = uint32_t(_physRegIndex[BaseReg::kGroupVirt - 1]) +
                    uint32_t(_physRegCount[BaseReg::kGroupVirt - 1]) ;
  }
  inline uint32_t physRegIndex(uint32_t group) const noexcept { return _physRegIndex[group]; }
  inline uint32_t physRegTotal() const noexcept { return _physRegTotal; }

  Error _asWorkReg(VirtReg* vReg, RAWorkReg** out) noexcept;

  //! Creates `RAWorkReg` data for the given `vReg`. The function does nothing
  //! if `vReg` already contains link to `RAWorkReg`. Called by `constructBlocks()`.
  inline Error asWorkReg(VirtReg* vReg, RAWorkReg** out) noexcept {
    *out = vReg->workReg();
    return *out ? kErrorOk : _asWorkReg(vReg, out);
  }

  inline Error virtIndexAsWorkReg(uint32_t vIndex, RAWorkReg** out) noexcept {
    const ZoneVector<VirtReg*>& virtRegs = cc()->virtRegs();
    if (ASMJIT_UNLIKELY(vIndex >= virtRegs.size()))
      return DebugUtils::errored(kErrorInvalidVirtId);
    return asWorkReg(virtRegs[vIndex], out);
  }

  inline RAStackSlot* getOrCreateStackSlot(RAWorkReg* workReg) noexcept {
    RAStackSlot* slot = workReg->stackSlot();
    if (slot) return slot;

    slot = _stackAllocator.newSlot(_sp.id(), workReg->virtReg()->virtSize(), workReg->virtReg()->alignment(), 0);
    workReg->_stackSlot = slot;
    workReg->markStackUsed();
    return slot;
  }

  inline BaseMem workRegAsMem(RAWorkReg* workReg) noexcept {
    getOrCreateStackSlot(workReg);
    return BaseMem(BaseMem::Decomposed { _sp.type(), workReg->virtId(), BaseReg::kTypeNone, 0, 0, 0, BaseMem::kSignatureMemRegHomeFlag });
  }

  WorkToPhysMap* newWorkToPhysMap() noexcept;
  PhysToWorkMap* newPhysToWorkMap() noexcept;

  inline PhysToWorkMap* clonePhysToWorkMap(const PhysToWorkMap* map) noexcept {
    size_t size = PhysToWorkMap::sizeOf(_physRegTotal);
    return static_cast<PhysToWorkMap*>(zone()->dupAligned(map, size, sizeof(uint32_t)));
  }

  inline WorkToPhysMap* cloneWorkToPhysMap(const WorkToPhysMap* map) noexcept {
    size_t size = WorkToPhysMap::sizeOf(_workRegs.size());
    if (ASMJIT_UNLIKELY(size == 0))
      return const_cast<WorkToPhysMap*>(map);
    return static_cast<WorkToPhysMap*>(zone()->dup(map, size));
  }

  // --------------------------------------------------------------------------
  // [Registers - Liveness Analysis and Statistics]
  // --------------------------------------------------------------------------

  //! 1. Calculates GEN/KILL/IN/OUT of each block.
  //! 2. Calculates live spans and basic statistics of each work register.
  Error buildLiveness() noexcept;

  //! Assigns argIndex to WorkRegs. Must be called after the liveness analysis
  //! finishes as it checks whether the argument is live upon entry.
  Error assignArgIndexToWorkRegs() noexcept;

  // --------------------------------------------------------------------------
  // [Allocation - Global]
  // --------------------------------------------------------------------------

  //! Runs a global register allocator.
  Error runGlobalAllocator() noexcept;

  //! Initializes data structures used for global live spans.
  Error initGlobalLiveSpans() noexcept;

  Error binPack(uint32_t group) noexcept;

  // --------------------------------------------------------------------------
  // [Allocation - Local]
  // --------------------------------------------------------------------------

  //! Runs a local register allocator.
  Error runLocalAllocator() noexcept;
  Error setBlockEntryAssignment(RABlock* block, const RABlock* fromBlock, const RAAssignment& fromAssignment) noexcept;

  // --------------------------------------------------------------------------
  // [Allocation - Utilities]
  // --------------------------------------------------------------------------

  Error useTemporaryMem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept;

  // --------------------------------------------------------------------------
  // [Allocation - Prolog / Epilog]
  // --------------------------------------------------------------------------

  Error updateStackFrame() noexcept;
  Error _markStackArgsToKeep() noexcept;
  Error _updateStackArgs() noexcept;
  Error insertPrologEpilog() noexcept;

  // --------------------------------------------------------------------------
  // [Rewriter]
  // --------------------------------------------------------------------------

  Error rewrite() noexcept;
  Error _rewrite(BaseNode* first, BaseNode* stop) noexcept;

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_LOGGING
  Error annotateCode() noexcept;

  Error _logBlockIds(const RABlocks& blocks) noexcept;
  Error _dumpBlockLiveness(String& sb, const RABlock* block) noexcept;
  Error _dumpLiveSpans(String& sb) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  virtual Error onEmitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept = 0;
  virtual Error onEmitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept = 0;

  virtual Error onEmitLoad(uint32_t workId, uint32_t dstPhysId) noexcept = 0;
  virtual Error onEmitSave(uint32_t workId, uint32_t srcPhysId) noexcept = 0;

  virtual Error onEmitJump(const Label& label) noexcept = 0;
  virtual Error onEmitPreCall(FuncCallNode* call) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Allocator that uses zone passed to `runOnFunction()`.
  ZoneAllocator _allocator;
  //! Logger, disabled if null.
  Logger* _logger;
  //! Debug logger, non-null only if `kOptionDebugPasses` option is set.
  Logger* _debugLogger;
  //! Logger flags.
  uint32_t _loggerFlags;

  //! Function being processed.
  FuncNode* _func;
  //! Stop node.
  BaseNode* _stop;
  //! Node that is used to insert extra code after the function body.
  BaseNode* _extraBlock;

  //! Blocks (first block is the entry, always exists).
  RABlocks _blocks;
  //! Function exit blocks (usually one, but can contain more).
  RABlocks _exits;
  //! Post order view (POV).
  RABlocks _pov;

  //! Number of instruction nodes.
  uint32_t _instructionCount;
  //! Number of created blocks (internal).
  uint32_t _createdBlockCount;
  //! Timestamp generator (incremental).
  mutable uint64_t _lastTimestamp;

  //!< Architecture registers information.
  const ArchRegs* _archRegsInfo;
  //! Architecture traits.
  RAArchTraits _archTraits;
  //! Index to physical registers in `RAAssignment::PhysToWorkMap`.
  RARegIndex _physRegIndex;
  //! Count of physical registers in `RAAssignment::PhysToWorkMap`.
  RARegCount _physRegCount;
  //! Total number of physical registers.
  uint32_t _physRegTotal;

  //! Registers available for allocation.
  RARegMask _availableRegs;
  //! Count of physical registers per group.
  RARegCount _availableRegCount;
  //! Registers clobbered by the function.
  RARegMask _clobberedRegs;

  //! Work registers (registers used by the function).
  RAWorkRegs _workRegs;
  //! Work registers per register group.
  RAWorkRegs _workRegsOfGroup[BaseReg::kGroupVirt];

  //! Register allocation strategy per register group.
  RAStrategy _strategy[BaseReg::kGroupVirt];
  //! Global max live-count (from all blocks) per register group.
  RALiveCount _globalMaxLiveCount;
  //! Global live spans per register group.
  LiveRegSpans* _globalLiveSpans[BaseReg::kGroupVirt];
  //! Temporary stack slot.
  Operand _temporaryMem;

  //! Stack pointer.
  BaseReg _sp;
  //! Frame pointer.
  BaseReg _fp;
  //! Stack manager.
  RAStackAllocator _stackAllocator;
  //! Function arguments assignment.
  FuncArgsAssignment _argsAssignment;
  //! Some StackArgs have to be assigned to StackSlots.
  uint32_t _numStackArgsToStackSlots;

  //! Maximum name-size computed from all WorkRegs.
  uint32_t _maxWorkRegNameSize;
  //! Temporary string builder used to format comments.
  StringTmp<80> _tmpString;
};

inline ZoneAllocator* RABlock::allocator() const noexcept { return _ra->allocator(); }

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_CORE_RAPASS_P_H
