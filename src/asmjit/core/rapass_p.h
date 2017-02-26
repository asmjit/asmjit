// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_RAPASS_P_H
#define _ASMJIT_CORE_RAPASS_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/raassignment_p.h"
#include "../core/radefs_p.h"
#include "../core/rastack_p.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_ra
//! \{

// ============================================================================
// [asmjit::RABlock]
// ============================================================================

class RABlock {
public:
  ASMJIT_NONCOPYABLE(RABlock)

  enum Id : uint32_t {
    kUnassignedId         = 0xFFFFFFFFU
  };

  enum Flags : uint32_t {
    kFlagIsConstructed    = 0x00000001U, //!< Block has been constructed from nodes.
    kFlagIsReachable      = 0x00000002U, //!< Block is reachable (set by `buildViews()`).
    kFlagIsAllocated      = 0x00000004U, //!< Block has been allocated.
    kFlagIsFuncExit       = 0x00000008U, //!< Block is a function-exit.

    kFlagHasTerminator    = 0x00000010U, //!< Block has a terminator (jump, conditional jump, ret).
    kFlagHasConsecutive   = 0x00000020U, //!< Block naturally flows to the next block.
    kFlagHasFixedRegs     = 0x00000040U, //!< Block contains fixed registers (precolored).
    kFlagHasFuncCalls     = 0x00000080U  //!< Block contains function calls.
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

  inline RAPass* getPass() const noexcept { return _ra; }
  inline ZoneAllocator* getAllocator() const noexcept;

  inline uint32_t getBlockId() const noexcept { return _blockId; }
  inline uint32_t getFlags() const noexcept { return _flags; }

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

  inline const RARegsStats& getRegsStats() const noexcept { return _regsStats; }

  inline bool hasTerminator() const noexcept { return hasFlag(kFlagHasTerminator); }
  inline bool hasConsecutive() const noexcept { return hasFlag(kFlagHasConsecutive); }

  inline bool hasPredecessors() const noexcept { return !_predecessors.isEmpty(); }
  inline bool hasSuccessors() const noexcept { return !_successors.isEmpty(); }

  inline const RABlocks& getPredecessors() const noexcept { return _predecessors; }
  inline const RABlocks& getSuccessors() const noexcept { return _successors; }

  inline CBNode* getFirst() const noexcept { return _first; }
  inline CBNode* getLast() const noexcept { return _last; }

  inline void setFirst(CBNode* node) noexcept { _first = node; }
  inline void setLast(CBNode* node) noexcept { _last = node; }

  inline uint32_t getFirstPosition() const noexcept { return _firstPosition; }
  inline void setFirstPosition(uint32_t position) noexcept { _firstPosition = position; }

  inline uint32_t getEndPosition() const noexcept { return _endPosition; }
  inline void setEndPosition(uint32_t position) noexcept { _endPosition = position; }

  inline uint32_t getPovOrder() const noexcept { return _povOrder; }

  inline bool hasTimestamp(uint64_t ts) const noexcept { return _timestamp == ts; }
  inline uint64_t getTimestamp() const noexcept { return _timestamp; }
  inline void setTimestamp(uint64_t ts) const noexcept { _timestamp = ts; }
  inline void resetTimestamp() const noexcept { _timestamp = 0; }

  inline RABlock* getConsecutive() const noexcept { return hasConsecutive() ? _successors[0] : nullptr; }

  inline bool hasIDom() const noexcept { return _idom != nullptr; }
  inline RABlock* getIDom() noexcept { return _idom; }
  inline const RABlock* getIDom() const noexcept { return _idom; }
  inline void setIDom(RABlock* block) noexcept { _idom = block; }

  inline ZoneBitVector& getLiveIn() noexcept { return _liveBits[kLiveIn]; }
  inline const ZoneBitVector& getLiveIn() const noexcept { return _liveBits[kLiveIn]; }

  inline ZoneBitVector& getLiveOut() noexcept { return _liveBits[kLiveOut]; }
  inline const ZoneBitVector& getLiveOut() const noexcept { return _liveBits[kLiveOut]; }

  inline ZoneBitVector& getGen() noexcept { return _liveBits[kLiveGen]; }
  inline const ZoneBitVector& getGen() const noexcept { return _liveBits[kLiveGen]; }

  inline ZoneBitVector& getKill() noexcept { return _liveBits[kLiveKill]; }
  inline const ZoneBitVector& getKill() const noexcept { return _liveBits[kLiveKill]; }

  inline Error resizeLiveBits(uint32_t size) noexcept {
    ZoneAllocator* allocator = getAllocator();
    ASMJIT_PROPAGATE(_liveBits[kLiveIn  ].resize(allocator, size));
    ASMJIT_PROPAGATE(_liveBits[kLiveOut ].resize(allocator, size));
    ASMJIT_PROPAGATE(_liveBits[kLiveGen ].resize(allocator, size));
    ASMJIT_PROPAGATE(_liveBits[kLiveKill].resize(allocator, size));
    return kErrorOk;
  }

  inline bool hasEntryAssignment() const noexcept { return _entryPhysToWorkMap != nullptr; }
  inline WorkToPhysMap* getEntryWorkToPhysMap() const noexcept { return _entryWorkToPhysMap; }
  inline PhysToWorkMap* getEntryPhysToWorkMap() const noexcept { return _entryPhysToWorkMap; }

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
  uint32_t _flags;                       //!< Block flags, see \ref Flags.

  CBNode* _first;                        //!< First `CBNode` of this block (inclusive).
  CBNode* _last;                         //!< Last `CBNode` of this block (inclusive).

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

//! Register allocator's data associated with each \ref CBInst.
class RAInst {
public:
  ASMJIT_NONCOPYABLE(RAInst)

  enum Flags : uint32_t {
    kFlagIsTerminator = 0x00000001U
  };

  static inline size_t sizeOf(uint32_t tiedRegCount) noexcept {
    return sizeof(RAInst) - sizeof(RATiedReg) + tiedRegCount * sizeof(RATiedReg);
  }

  ASMJIT_FORCEINLINE RAInst(RABlock* block, uint32_t flags, uint32_t tiedTotal, const RARegMask& clobberedRegs) noexcept {
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

  //! Get the instruction flags.
  inline uint32_t getFlags() const noexcept { return _flags; }
  //! Get whether the instruction has flag `flag`.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  //! Set instruction flags to `flags`.
  inline void setFlags(uint32_t flags) noexcept { _flags = flags; }
  //! Add instruction `flags`.
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
  //! Clear instruction `flags`.
  inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

  //! Get whether the node is code that can be executed.
  inline bool isTerminator() const noexcept { return hasFlag(kFlagIsTerminator); }

  inline RABlock* getBlock() const noexcept { return _block; }

  //! Get tied registers (all).
  inline RATiedReg* getTiedRegs() const noexcept { return const_cast<RATiedReg*>(_tiedRegs); }
  //! Get tied registers for a given `group`.
  inline RATiedReg* getTiedRegs(uint32_t group) const noexcept { return const_cast<RATiedReg*>(_tiedRegs) + _tiedIndex.get(group); }

   //! Get count of all tied registers.
  inline uint32_t getTiedCount() const noexcept { return _tiedTotal; }
  //! Get count of tied registers of a given `group`.
  inline uint32_t getTiedCount(uint32_t group) const noexcept { return _tiedCount[group]; }

  //! Get `RATiedReg` at the specified `index`.
  inline RATiedReg* getTiedAt(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _tiedTotal);
    return getTiedRegs() + index;
  }

  //! Get `RATiedReg` at the specified index for a given register `group`.
  inline RATiedReg* getTiedOf(uint32_t group, uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _tiedCount._regs[group]);
    return getTiedRegs(group) + index;
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

  inline uint32_t getFlags() const noexcept { return _flags; }
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }

  //! Get the number of tied registers added to the builder.
  inline uint32_t getTiedRegCount() const noexcept {
    return uint32_t((size_t)(_cur - _tiedRegs));
  }

  //! Get a tied register at index `index`.
  inline RATiedReg* operator[](uint32_t index) noexcept {
    ASMJIT_ASSERT(index < getTiedRegCount());
    return &_tiedRegs[index];
  }

  //! Get a tied register at index `index` (const).
  inline const RATiedReg* operator[](uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < getTiedRegCount());
    return &_tiedRegs[index];
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_FORCEINLINE Error add(RAWorkReg* workReg, uint32_t flags, uint32_t allocable, uint32_t useId, uint32_t useRewriteMask, uint32_t outId, uint32_t outRewriteMask) noexcept {
    uint32_t group = workReg->getGroup();
    RATiedReg* tiedReg = workReg->getTiedReg();

    if (useId != Reg::kIdBad) {
      _stats.makeFixed(group);
      _used[group] |= IntUtils::mask(useId);
      flags |= RATiedReg::kUseFixed;
    }

    if (outId != Reg::kIdBad) {
      _clobbered[group] |= IntUtils::mask(outId);
      flags |= RATiedReg::kOutFixed;
    }

    _flags |= flags;
    _stats.makeUsed(group);

    if (!tiedReg) {
      // Could happen when the builder is not reset properly after each instruction.
      ASMJIT_ASSERT(getTiedRegCount() < ASMJIT_ARRAY_SIZE(_tiedRegs));

      tiedReg = _cur++;
      tiedReg->init(workReg->getWorkId(), flags, allocable, useId, useRewriteMask, outId, outRewriteMask);
      workReg->setTiedReg(tiedReg);

      _count.add(group);
      return kErrorOk;
    }
    else {
      // TODO: What about `useId`, in that case we should perform a move outside and ban coalescing.
      if (ASMJIT_UNLIKELY(outId != Reg::kIdBad)) {
        if (ASMJIT_UNLIKELY(tiedReg->hasOutId()))
          return DebugUtils::errored(kErrorOverlappedRegs);

        tiedReg->setOutId(outId);
        // TODO: ? _used[group] |= IntUtils::mask(outId);
      }

      tiedReg->addRefCount();
      tiedReg->addFlags(flags);
      tiedReg->_allocableRegs &= allocable;
      tiedReg->_useRewriteMask |= useRewriteMask;
      tiedReg->_outRewriteMask |= outRewriteMask;
      return kErrorOk;
    }
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _flags;                       //!< Flags combined from all RATiedReg's.
  RARegCount _count;
  RARegsStats _stats;

  RARegMask _used;
  RARegMask _clobbered;

  RATiedReg* _cur;                       //!< Current tied register in `_tiedRegs`.
  RATiedReg _tiedRegs[128];              //!< Array of temporary tied registers.
};

// ============================================================================
// [asmjit::RAPass]
// ============================================================================

//! Register allocation pass (abstract) used by \ref CodeCompiler.
class RAPass : public CCFuncPass {
public:
  ASMJIT_NONCOPYABLE(RAPass)
  typedef CCFuncPass Base;

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

  //! Get whether the logging is enabled, in that case `getLogger()` returns a valid `Logger` instance.
  inline bool hasLogger() const noexcept { return _logger != nullptr; }
  //! Get `Logger` passed to `runOnFunction()`.
  inline Logger* getLogger() const noexcept { return _logger; }

  //! Get whether the logging is enabled and `kOptionDebugPasses` is set.
  inline bool hasDebugLogger() const noexcept { return _debugLogger != nullptr; }
  //! Get `Logger` passed to `runOnFunction()` or null if `kOptionDebugPasses` is not set.
  inline Logger* getDebugLogger() const noexcept { return _debugLogger; }

  //! Get `Zone` passed to `runOnFunction()`.
  inline Zone* getZone() const { return _allocator.getZone(); }
  //! Get `ZoneAllocator` used by the register allocator.
  inline ZoneAllocator* getAllocator() const { return const_cast<ZoneAllocator*>(&_allocator); }

  //! Get function node.
  inline CCFunc* getFunc() const noexcept { return _func; }
  //! Get stop node.
  inline CBNode* getStop() const noexcept { return _stop; }

  //! Get extra block.
  inline CBNode* getExtraBlock() const noexcept { return _extraBlock; }
  //! Set extra block.
  inline void setExtraBlock(CBNode* node) noexcept { _extraBlock = node; }

  inline uint32_t getEndPosition() const noexcept { return _instructionCount * 2; }

  inline const RARegMask& getAvailableRegs() const noexcept { return _availableRegs; }
  inline const RARegMask& getCloberredRegs() const noexcept { return _clobberedRegs; }

  inline void makeUnavailable(uint32_t group, uint32_t regId) noexcept {
    _availableRegs[group] &= ~IntUtils::mask(regId);
    _availableRegCount[group]--;
  }

  // --------------------------------------------------------------------------
  // [RunOnFunction / RunAllocation]
  // --------------------------------------------------------------------------

  //! Run the register allocator for the given `func`.
  Error runOnFunction(Zone* zone, Logger* logger, CCFunc* func) noexcept override;

  //! Perform all allocation steps sequentially, called by `runOnFunction()`.
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

  //! Get entry block.
  inline RABlock* getEntryBlock() noexcept {
    ASMJIT_ASSERT(!_blocks.isEmpty());
    return _blocks[0];
  }

  //! Get entry block (const).
  inline const RABlock* getEntryBlock() const noexcept {
    ASMJIT_ASSERT(!_blocks.isEmpty());
    return _blocks[0];
  }

  //! Get count of basic blocks (returns length of `_blocks` array).
  inline uint32_t getBlockCount() const noexcept { return _blocks.getLength(); }
  //! Get count of reachable basic blocks (returns length of `_pov` array).
  inline uint32_t getReachableBlockCount() const noexcept { return _pov.getLength(); }

  //! Get whether the CFG has dangling blocks - these were created by `newBlock()`,
  //! but not added to CFG through `addBlocks()`. If `true` is returned and the
  //! CFG is constructed it means that something is missing and it's incomplete.
  //!
  //! NOTE: This is only used to check if the number of created blocks matches
  //! the number of added blocks.
  inline bool hasDanglingBlocks() const noexcept { return _createdBlockCount != getBlockCount(); }

  //! Get a next timestamp to be used to mark CFG blocks.
  inline uint64_t nextTimestamp() const noexcept { return ++_lastTimestamp; }

  //! Creates a new `RABlock` instance.
  //!
  //! NOTE: New blocks don't have ID assigned until they are added to the block
  //! array by calling `addBlock()`.
  RABlock* newBlock(CBNode* initialNode = nullptr) noexcept;

  //! Tries to find a neighboring CBLabel (without going through code) that is
  //! already connected with `RABlock`. If no label is found then a new RABlock
  //! is created and assigned to all possible labels in a backward direction.
  RABlock* newBlockOrExistingAt(CBLabel* cbLabel, CBNode** stoppedAt = nullptr) noexcept;

  //! Add the given `block` to the block list and assign it a unique block id.
  Error addBlock(RABlock* block) noexcept;

  inline Error addExitBlock(RABlock* block) noexcept {
    block->addFlags(RABlock::kFlagIsFuncExit);
    return _exits.append(getAllocator(), block);
  }

  ASMJIT_FORCEINLINE RAInst* newRAInst(RABlock* block, uint32_t flags, uint32_t tiedRegCount, const RARegMask& clobberedRegs) noexcept {
    return new(getZone()->alloc(RAInst::sizeOf(tiedRegCount))) RAInst(block, flags, tiedRegCount, clobberedRegs);
  }

  ASMJIT_FORCEINLINE Error assignRAInst(CBNode* node, RABlock* block, RAInstBuilder& ib) noexcept {
    uint32_t tiedRegCount = ib.getTiedRegCount();
    RAInst* raInst = newRAInst(block, ib.getFlags(), tiedRegCount, ib._clobbered);

    if (ASMJIT_UNLIKELY(!raInst))
      return DebugUtils::errored(kErrorNoHeapMemory);

    RARegIndex index;
    index.buildIndexes(ib._count);

    raInst->_tiedIndex = index;
    raInst->_tiedCount = ib._count;

    for (uint32_t i = 0; i < tiedRegCount; i++) {
      RATiedReg* tiedReg = ib[i];
      RAWorkReg* workReg = getWorkReg(tiedReg->getWorkId());

      workReg->resetTiedReg();
      uint32_t group = workReg->getGroup();

      if (tiedReg->hasUseId()) {
        block->addFlags(RABlock::kFlagHasFixedRegs);
        raInst->_usedRegs[group] |= IntUtils::mask(tiedReg->getUseId());
      }

      if (tiedReg->hasOutId()) {
        block->addFlags(RABlock::kFlagHasFixedRegs);
      }

      RATiedReg& dst = raInst->_tiedRegs[index[group]++];
      dst = *tiedReg;
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
  //!      for CodeCompiler itself as the register allocator cannot reach such
  //!      nodes, but keeping instructions that use virtual registers would fail
  //!      during instruction encoding phase (Assembler).
  //!
  //!   3. `RAInst` is created for each `CBInst` or compatible. It contains
  //!      information that is essential for further analysis and register
  //!      allocation.
  //!
  //! Use `RACFGBuilder` template that provides the necessary boilerplate.
  virtual Error buildCFG() noexcept = 0;

  // --------------------------------------------------------------------------
  // [CFG - Views Order]
  // --------------------------------------------------------------------------

  //! Construct CFG views (only POV at the moment).
  Error buildViews() noexcept;

  // --------------------------------------------------------------------------
  // [CFG - Dominators]
  // --------------------------------------------------------------------------

  // Terminology:
  //   - A node `X` dominates a node `Z` if any path from the entry point to
  //     `Z` has to go through `X`.
  //   - A node `Z` post-dominates a node `X` if any path from `X` to the end
  //     of the graph has to go through `Z`.

  //! Construct a dominator-tree from CFG.
  Error buildDominators() noexcept;

  //! \internal
  bool _strictlyDominates(const RABlock* a, const RABlock* b) const noexcept;
  //! \internal
  const RABlock* _nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept;

  //! Get whether basic block `a` dominates `b` - non-strict, returns true when `a == b`.
  inline bool dominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? true : _strictlyDominates(a, b); }
  //! Get whether basic block `a` dominates `b` - strict dominance check, returns false when `a == b`.
  inline bool strictlyDominates(const RABlock* a, const RABlock* b) const noexcept { return a == b ? false : _strictlyDominates(a, b); }

  //! Get a nearest common dominator of `a` and `b`.
  inline RABlock* nearestCommonDominator(RABlock* a, RABlock* b) const noexcept { return const_cast<RABlock*>(_nearestCommonDominator(a, b)); }
  //! Get a nearest common dominator of `a` and `b` (const).
  inline const RABlock* nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept { return _nearestCommonDominator(a, b); }

  // --------------------------------------------------------------------------
  // [CFG - Utilities]
  // --------------------------------------------------------------------------

  Error removeUnreachableBlocks() noexcept;

  //! Returns `node` or some node after that is ideal for beginning a new block.
  //! This function is mostly used after a conditional or unconditional jump to
  //! select the successor node. In some cases the next node could be a label,
  //! which means it could have assigned some block already.
  CBNode* findSuccessorStartingAt(CBNode* node) noexcept;

  //! Returns `true` of the `node` can flow to `target` without reaching code
  //! nor data. It's used to eliminate jumps to labels that are next right to
  //! them.
  bool isNextTo(CBNode* node, CBNode* target) noexcept;

  // --------------------------------------------------------------------------
  // [Registers - Management]
  // --------------------------------------------------------------------------

  //! Get a native size of a general-purpose register.
  inline uint32_t getGpSize() const noexcept { return _sp.getSize(); }
  inline uint32_t getAvailableRegCount(uint32_t group) const noexcept { return _availableRegCount[group]; }

  inline RAWorkReg* getWorkReg(uint32_t workId) const noexcept { return _workRegs[workId]; }

  inline RAWorkRegs& getWorkRegs() noexcept { return _workRegs; }
  inline RAWorkRegs& getWorkRegs(uint32_t group) noexcept { return _workRegsOfGroup[group]; }

  inline const RAWorkRegs& getWorkRegs() const noexcept { return _workRegs; }
  inline const RAWorkRegs& getWorkRegs(uint32_t group) const noexcept { return _workRegsOfGroup[group]; }

  inline uint32_t getWorkRegCount() const noexcept { return _workRegs.getLength(); }
  inline uint32_t getWorkRegCount(uint32_t group) const noexcept { return _workRegsOfGroup[group].getLength(); }

  inline void _buildPhysIndex() noexcept {
    _physRegIndex.buildIndexes(_physRegCount);
    _physRegTotal = uint32_t(_physRegIndex[Reg::kGroupVirt - 1]) +
                    uint32_t(_physRegCount[Reg::kGroupVirt - 1]) ;
  }
  inline uint32_t getPhysRegIndex(uint32_t group) const noexcept { return _physRegIndex[group]; }
  inline uint32_t getPhysRegTotal() const noexcept { return _physRegTotal; }

  Error _asWorkReg(VirtReg* vReg, RAWorkReg** out) noexcept;

  //! Creates `RAWorkReg` data for the given `vReg`. The function does nothing
  //! if `vReg` already contains link to `RAWorkReg`. Called by `constructBlocks()`.
  inline Error asWorkReg(VirtReg* vReg, RAWorkReg** out) noexcept {
    *out = vReg->getWorkReg();
    return *out ? kErrorOk : _asWorkReg(vReg, out);
  }

  inline Error virtIndexAsWorkReg(uint32_t vIndex, RAWorkReg** out) noexcept {
    const ZoneVector<VirtReg*>& virtRegs = cc()->getVirtRegs();
    if (ASMJIT_UNLIKELY(vIndex >= virtRegs.getLength()))
      return DebugUtils::errored(kErrorInvalidVirtId);
    return asWorkReg(virtRegs[vIndex], out);
  }

  inline RAStackSlot* getOrCreateStackSlot(RAWorkReg* workReg) noexcept {
    RAStackSlot* slot = workReg->getStackSlot();
    if (slot) return slot;

    slot = _stackAllocator.newSlot(_sp.getId(), workReg->getVirtReg()->getVirtSize(), workReg->getVirtReg()->getAlignment(), 0);
    workReg->_stackSlot = slot;
    workReg->markStackUsed();
    return slot;
  }

  inline Mem workRegAsMem(RAWorkReg* workReg) noexcept {
    getOrCreateStackSlot(workReg);
    return Mem(Globals::Init, _sp.getType(), workReg->getVirtId(), Reg::kRegNone, 0, 0, 0, Mem::kSignatureMemRegHomeFlag);
  }

  WorkToPhysMap* newWorkToPhysMap() noexcept;
  PhysToWorkMap* newPhysToWorkMap() noexcept;

  inline PhysToWorkMap* clonePhysToWorkMap(const PhysToWorkMap* map) noexcept {
    size_t size = PhysToWorkMap::sizeOf(_physRegTotal);
    return static_cast<PhysToWorkMap*>(getZone()->dupAligned(map, size, sizeof(uint32_t)));
  }

  inline WorkToPhysMap* cloneWorkToPhysMap(const WorkToPhysMap* map) noexcept {
    size_t size = WorkToPhysMap::sizeOf(_workRegs.getLength());
    if (ASMJIT_UNLIKELY(size == 0))
      return const_cast<WorkToPhysMap*>(map);
    return static_cast<WorkToPhysMap*>(getZone()->dup(map, size));
  }

  // --------------------------------------------------------------------------
  // [Registers - Liveness Analysis and Statistics]
  // --------------------------------------------------------------------------

  //! 1. Calculate GEN/KILL/IN/OUT of each block.
  //! 2. Calculate live spans and basic statistics of each work register.
  Error buildLiveness() noexcept;

  // --------------------------------------------------------------------------
  // [Allocation - Global]
  // --------------------------------------------------------------------------

  //! Run a global register allocator.
  Error runGlobalAllocator() noexcept;

  Error binPack(uint32_t group) noexcept;

  // --------------------------------------------------------------------------
  // [Allocation - Local]
  // --------------------------------------------------------------------------

  //! Run a local register allocator.
  Error runLocalAllocator() noexcept;
  Error setBlockEntryAssignment(RABlock* block, const RABlock* fromBlock, const RAAssignment& fromAssignment) noexcept;

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
  Error _rewrite(CBNode* first, CBNode* stop) noexcept;

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  #ifndef ASMJIT_DISABLE_LOGGING
  Error annotateCode() noexcept;

  Error _logBlockIds(const RABlocks& blocks) noexcept;
  Error _dumpBlockLiveness(StringBuilder& sb, const RABlock* block) noexcept;
  Error _dumpLiveSpans(StringBuilder& sb) noexcept;
  #endif

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  virtual Error onEmitMove(uint32_t workId, uint32_t dstPhysId, uint32_t srcPhysId) noexcept = 0;
  virtual Error onEmitSwap(uint32_t aWorkId, uint32_t aPhysId, uint32_t bWorkId, uint32_t bPhysId) noexcept = 0;

  virtual Error onEmitLoad(uint32_t workId, uint32_t dstPhysId) noexcept = 0;
  virtual Error onEmitSave(uint32_t workId, uint32_t srcPhysId) noexcept = 0;

  virtual Error onEmitJump(const Label& label) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ZoneAllocator _allocator;              //!< Allocator that uses zone passed to `runOnFunction()`.
  Logger* _logger;                       //!< Logger, disabled if null.
  Logger* _debugLogger;                  //!< Debug logger, non-null only if `kOptionDebugPasses` option is set.
  uint32_t _loggerOptions;               //!< Logger options.

  CCFunc* _func;                         //!< Function being processed.
  CBNode* _stop;                         //!< Stop node.
  CBNode* _extraBlock;                   //!< Node that is used to insert extra code after the function body.

  RABlocks _blocks;                      //!< Blocks (first block is the entry, always exists).
  RABlocks _exits;                       //!< Function exit blocks (usually one, but can contain more).
  RABlocks _pov;                         //!< Post order view (POV).

  uint32_t _instructionCount;            //!< Number of instruction nodes.
  uint32_t _createdBlockCount;           //!< Number of created blocks (internal).
  mutable uint64_t _lastTimestamp;       //!< Timestamp generator (incremental).

  RAArchTraits _archTraits;              //!< Architecture traits.
  RARegIndex _physRegIndex;              //!< Index to physical registers in `RAAssignment::PhysToWorkMap`.
  RARegCount _physRegCount;              //!< Count of physical registers in `RAAssignment::PhysToWorkMap`.
  uint32_t _physRegTotal;                //!< Total number of physical registers.

  RARegMask _availableRegs;              //!< Registers available for allocation.
  RARegCount _availableRegCount;         //!< Count of physical registers per group.

  RARegMask _clobberedRegs;              //!< Registers clobbered by the function.

  RAWorkRegs _workRegs;                  //!< Work registers (registers used by the function).
  RAWorkRegs _workRegsOfGroup[Reg::kGroupVirt];

  RAStrategy _strategy[Reg::kGroupVirt]; //!< Register allocation strategy.
  RALiveCount _globalMaxLiveCount;       //!< Global max live-count (from all blocks).

  Reg _sp;                               //!< Stack pointer.
  Reg _fp;                               //!< Frame pointer.
  RAStackAllocator _stackAllocator;      //!< Stack manager.
  FuncArgsAssignment _argsAssignment;    //!< Function arguments mapper.
  uint32_t _numStackArgsToStackSlots;    //!< Some StackArgs have to be assigned to StackSlots.

  StringBuilderTmp<80> _tmpString;       //!< Temporary string builder used to format comments.
  uint32_t _maxWorkRegNameLength;        //!< Maximum length computed from all WorkReg's.
};

inline ZoneAllocator* RABlock::getAllocator() const noexcept { return _ra->getAllocator(); }

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_CORE_RAPASS_P_H
