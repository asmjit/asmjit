// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

#include "../core/ralocal_p.h"
#include "../core/rapass_p.h"
#include "../core/support.h"
#include "../core/type.h"
#include "../core/zonestack.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::RABlock - Control Flow]
// ============================================================================

Error RABlock::appendSuccessor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->_successors.contains(successor))
    return kErrorOk;
  ASMJIT_ASSERT(!successor->_predecessors.contains(predecessor));

  ASMJIT_PROPAGATE(successor->_predecessors.willGrow(allocator()));
  ASMJIT_PROPAGATE(predecessor->_successors.willGrow(allocator()));

  predecessor->_successors.appendUnsafe(successor);
  successor->_predecessors.appendUnsafe(predecessor);

  return kErrorOk;
}

Error RABlock::prependSuccessor(RABlock* successor) noexcept {
  RABlock* predecessor = this;

  if (predecessor->_successors.contains(successor))
    return kErrorOk;
  ASMJIT_ASSERT(!successor->_predecessors.contains(predecessor));

  ASMJIT_PROPAGATE(successor->_predecessors.willGrow(allocator()));
  ASMJIT_PROPAGATE(predecessor->_successors.willGrow(allocator()));

  predecessor->_successors.prependUnsafe(successor);
  successor->_predecessors.prependUnsafe(predecessor);

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Construction / Destruction]
// ============================================================================

RAPass::RAPass() noexcept
  : FuncPass("RAPass"),
    _allocator(),
    _logger(nullptr),
    _debugLogger(nullptr),
    _loggerFlags(0),
    _func(nullptr),
    _stop(nullptr),
    _extraBlock(nullptr),
    _blocks(),
    _exits(),
    _pov(),
    _instructionCount(0),
    _createdBlockCount(0),
    _lastTimestamp(0),
    _archRegsInfo(nullptr),
    _archTraits(),
    _physRegIndex(),
    _physRegCount(),
    _physRegTotal(0),
    _availableRegs(),
    _availableRegCount(),
    _clobberedRegs(),
    _globalMaxLiveCount(),
    _globalLiveSpans {},
    _temporaryMem(),
    _sp(),
    _fp(),
    _stackAllocator(),
    _argsAssignment(),
    _numStackArgsToStackSlots(0),
    _maxWorkRegNameSize(0) {}
RAPass::~RAPass() noexcept {}

// ============================================================================
// [asmjit::RAPass - RunOnFunction]
// ============================================================================

static void RAPass_reset(RAPass* self, FuncDetail* funcDetail) noexcept {
  ZoneAllocator* allocator = self->allocator();

  self->_blocks.reset();
  self->_exits.reset();
  self->_pov.reset();
  self->_workRegs.reset();
  self->_instructionCount = 0;
  self->_createdBlockCount = 0;
  self->_lastTimestamp = 0;

  self->_archRegsInfo = nullptr;
  self->_archTraits.reset();
  self->_physRegIndex.reset();
  self->_physRegCount.reset();
  self->_physRegTotal = 0;

  self->_availableRegs.reset();
  self->_availableRegCount.reset();
  self->_clobberedRegs.reset();

  self->_workRegs.reset();
  for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++) {
    self->_workRegsOfGroup[group].reset();
    self->_strategy[group].reset();
    self->_globalLiveSpans[group] = nullptr;
  }
  self->_globalMaxLiveCount.reset();
  self->_temporaryMem.reset();

  self->_stackAllocator.reset(allocator);
  self->_argsAssignment.reset(funcDetail);
  self->_numStackArgsToStackSlots = 0;
  self->_maxWorkRegNameSize = 0;
}

static void RAPass_resetVirtRegData(RAPass* self) noexcept {
  // Zero everything so it cannot be used by accident.
  for (RAWorkReg* wReg : self->_workRegs) {
    VirtReg* vReg = wReg->virtReg();
    vReg->_workReg = nullptr;
  }
}

Error RAPass::runOnFunction(Zone* zone, Logger* logger, FuncNode* func) noexcept {
  _allocator.reset(zone);

  #ifndef ASMJIT_DISABLE_LOGGING
  _logger = logger;
  _debugLogger = nullptr;

  if (logger) {
    _loggerFlags = logger->flags();
    if (_loggerFlags & FormatOptions::kFlagDebugPasses)
      _debugLogger = logger;
  }
  #endif

  // Initialize all core structures to use `zone` and `func`.
  BaseNode* end = func->endNode();
  _func = func;
  _stop = end->next();
  _extraBlock = end;

  RAPass_reset(this, &_func->_funcDetail);

  // Initialize architecture-specific members.
  onInit();

  // Perform all allocation steps required.
  Error err = onPerformAllSteps();

  // Must be called regardless of the allocation status.
  onDone();

  // TODO: I don't like this...
  // Reset possible connections introduced by the register allocator.
  RAPass_resetVirtRegData(this);

  // Reset all core structures and everything that depends on the passed `Zone`.
  RAPass_reset(this, nullptr);
  _allocator.reset(nullptr);

  #ifndef ASMJIT_DISABLE_LOGGING
  _logger = nullptr;
  _debugLogger = nullptr;
  _loggerFlags = 0;
  #endif

  _func = nullptr;
  _stop = nullptr;
  _extraBlock = nullptr;

  // Reset `Zone` as nothing should persist between `runOnFunction()` calls.
  zone->reset();

  // We alter the compiler cursor, because it doesn't make sense to reference
  // it after the compilation - some nodes may disappear and the old cursor
  // can go out anyway.
  cc()->_setCursor(cc()->lastNode());

  return err;
}

Error RAPass::onPerformAllSteps() noexcept {
  ASMJIT_PROPAGATE(buildCFG());
  ASMJIT_PROPAGATE(buildViews());
  ASMJIT_PROPAGATE(removeUnreachableBlocks());

  ASMJIT_PROPAGATE(buildDominators());
  ASMJIT_PROPAGATE(buildLiveness());
  ASMJIT_PROPAGATE(assignArgIndexToWorkRegs());

  #ifndef ASMJIT_DISABLE_LOGGING
  if (logger() && logger()->hasFlag(FormatOptions::kFlagAnnotations))
    ASMJIT_PROPAGATE(annotateCode());
  #endif

  ASMJIT_PROPAGATE(runGlobalAllocator());
  ASMJIT_PROPAGATE(runLocalAllocator());

  ASMJIT_PROPAGATE(updateStackFrame());
  ASMJIT_PROPAGATE(insertPrologEpilog());

  ASMJIT_PROPAGATE(rewrite());

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - CFG - Basic Block Management]
// ============================================================================

RABlock* RAPass::newBlock(BaseNode* initialNode) noexcept {
  RABlock* block = zone()->newT<RABlock>(this);
  if (ASMJIT_UNLIKELY(!block))
    return nullptr;

  block->setFirst(initialNode);
  block->setLast(initialNode);

  _createdBlockCount++;
  return block;
}

RABlock* RAPass::newBlockOrExistingAt(LabelNode* cbLabel, BaseNode** stoppedAt) noexcept {
  if (cbLabel->hasPassData())
    return cbLabel->passData<RABlock>();

  FuncNode* func = this->func();
  BaseNode* node = cbLabel->prev();
  RABlock* block = nullptr;

  // Try to find some label, but terminate the loop on any code. We try hard to
  // coalesce code that contains two consecutive labels or a combination of
  // non-code nodes between 2 or more labels.
  //
  // Possible cases that would share the same basic block:
  //
  //   1. Two or more consecutive labels:
  //     Label1:
  //     Label2:
  //
  //   2. Two or more labels separated by non-code nodes:
  //     Label1:
  //     ; Some comment...
  //     .align 16
  //     Label2:
  size_t nPendingLabels = 0;

  while (node) {
    if (node->type() == BaseNode::kNodeLabel) {
      // Function has a different NodeType, just make sure this was not messed
      // up as we must never associate BasicBlock with a `func` itself.
      ASMJIT_ASSERT(node != func);

      block = node->passData<RABlock>();
      if (block) {
        // Exit node has always a block associated with it. If we went here it
        // means that `cbLabel` passed here is after the end of the function
        // and cannot be merged with the function exit block.
        if (node == func->exitNode())
          block = nullptr;
        break;
      }

      nPendingLabels++;
    }
    else if (node->type() == BaseNode::kNodeAlign) {
      // Align node is fine.
    }
    else {
      break;
    }

    node = node->prev();
  }

  if (stoppedAt)
    *stoppedAt = node;

  if (!block) {
    block = newBlock();
    if (ASMJIT_UNLIKELY(!block))
      return nullptr;
  }

  cbLabel->setPassData<RABlock>(block);
  node = cbLabel;

  while (nPendingLabels) {
    node = node->prev();
    for (;;) {
      if (node->type() == BaseNode::kNodeLabel) {
        node->setPassData<RABlock>(block);
        nPendingLabels--;
        break;
      }

      node = node->prev();
      ASMJIT_ASSERT(node != nullptr);
    }
  }

  if (!block->first()) {
    block->setFirst(node);
    block->setLast(cbLabel);
  }

  return block;
}

Error RAPass::addBlock(RABlock* block) noexcept {
  ASMJIT_PROPAGATE(_blocks.willGrow(allocator()));

  block->_blockId = blockCount();
  _blocks.appendUnsafe(block);
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - CFG - Views Order]
// ============================================================================

class RABlockVisitItem {
public:
  inline RABlockVisitItem(RABlock* block, uint32_t index) noexcept
    : _block(block),
      _index(index) {}

  inline RABlockVisitItem(const RABlockVisitItem& other) noexcept
    : _block(other._block),
      _index(other._index) {}

  inline RABlock* block() const noexcept { return _block; }
  inline uint32_t index() const noexcept { return _index; }

  RABlock* _block;
  uint32_t _index;
};

Error RAPass::buildViews() noexcept {
  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = debugLogger();
  #endif

  ASMJIT_RA_LOG_FORMAT("[RAPass::BuildViews]\n");

  uint32_t count = blockCount();
  if (ASMJIT_UNLIKELY(!count)) return kErrorOk;

  ASMJIT_PROPAGATE(_pov.reserve(allocator(), count));

  ZoneStack<RABlockVisitItem> stack;
  ASMJIT_PROPAGATE(stack.init(allocator()));

  ZoneBitVector visited;
  ASMJIT_PROPAGATE(visited.resize(allocator(), count));

  RABlock* current = _blocks[0];
  uint32_t i = 0;

  for (;;) {
    for (;;) {
      if (i >= current->successors().size())
        break;

      // Skip if already visited.
      RABlock* child = current->successors()[i++];
      if (visited.bitAt(child->blockId()))
        continue;

      // Mark as visited to prevent visiting the same block multiple times.
      visited.setBit(child->blockId(), true);

      // Add the current block on the stack, we will get back to it later.
      ASMJIT_PROPAGATE(stack.append(RABlockVisitItem(current, i)));
      current = child;
      i = 0;
    }

    current->makeReachable();
    current->_povOrder = _pov.size();
    _pov.appendUnsafe(current);

    if (stack.empty())
      break;

    RABlockVisitItem top = stack.pop();
    current = top.block();
    i = top.index();
  }

  visited.release(allocator());
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - CFG - Dominators]
// ============================================================================

static ASMJIT_INLINE RABlock* intersectBlocks(RABlock* b1, RABlock* b2) noexcept {
  while (b1 != b2) {
    while (b2->povOrder() > b1->povOrder()) b1 = b1->iDom();
    while (b1->povOrder() > b2->povOrder()) b2 = b2->iDom();
  }
  return b1;
}

// Based on "A Simple, Fast Dominance Algorithm".
Error RAPass::buildDominators() noexcept {
  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = debugLogger();
  #endif

  ASMJIT_RA_LOG_FORMAT("[RAPass::BuildDominators]\n");

  if (_blocks.empty())
    return kErrorOk;

  RABlock* entryBlock = this->entryBlock();
  entryBlock->setIDom(entryBlock);

  bool changed = true;
  uint32_t nIters = 0;

  while (changed) {
    nIters++;
    changed = false;

    uint32_t i = _pov.size();
    while (i) {
      RABlock* block = _pov[--i];
      if (block == entryBlock)
        continue;

      RABlock* iDom = nullptr;
      const RABlocks& preds = block->predecessors();

      uint32_t j = preds.size();
      while (j) {
        RABlock* p = preds[--j];
        if (!p->iDom()) continue;
        iDom = !iDom ? p : intersectBlocks(iDom, p);
      }

      if (block->iDom() != iDom) {
        ASMJIT_RA_LOG_FORMAT("  IDom of #%u -> #%u\n", block->blockId(), iDom->blockId());
        block->setIDom(iDom);
        changed = true;
      }
    }
  }

  ASMJIT_RA_LOG_FORMAT("  Done (%u iterations)\n", nIters);
  return kErrorOk;
}

bool RAPass::_strictlyDominates(const RABlock* a, const RABlock* b) const noexcept {
  ASMJIT_ASSERT(a != nullptr); // There must be at least one block if this function is
  ASMJIT_ASSERT(b != nullptr); // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != b);       // Checked by `dominates()` and `strictlyDominates()`.

  // Nothing strictly dominates the entry block.
  const RABlock* entryBlock = this->entryBlock();
  if (a == entryBlock)
    return false;

  const RABlock* iDom = b->iDom();
  while (iDom != a && iDom != entryBlock)
    iDom = iDom->iDom();

  return iDom != entryBlock;
}

const RABlock* RAPass::_nearestCommonDominator(const RABlock* a, const RABlock* b) const noexcept {
  ASMJIT_ASSERT(a != nullptr); // There must be at least one block if this function is
  ASMJIT_ASSERT(b != nullptr); // called, as both `a` and `b` must be valid blocks.
  ASMJIT_ASSERT(a != b);       // Checked by `dominates()` and `properlyDominates()`.

  if (a == b)
    return a;

  // If `a` strictly dominates `b` then `a` is the nearest common dominator.
  if (_strictlyDominates(a, b))
    return a;

  // If `b` strictly dominates `a` then `b` is the nearest common dominator.
  if (_strictlyDominates(b, a))
    return b;

  const RABlock* entryBlock = this->entryBlock();
  uint64_t timestamp = nextTimestamp();

  // Mark all A's dominators.
  const RABlock* block = a->iDom();
  while (block != entryBlock) {
    block->setTimestamp(timestamp);
    block = block->iDom();
  }

  // Check all B's dominators against marked dominators of A.
  block = b->iDom();
  while (block != entryBlock) {
    if (block->hasTimestamp(timestamp))
      return block;
    block = block->iDom();
  }

  return entryBlock;
}

// ============================================================================
// [asmjit::RAPass - CFG - Utilities]
// ============================================================================

Error RAPass::removeUnreachableBlocks() noexcept {
  uint32_t numAllBlocks = blockCount();
  uint32_t numReachableBlocks = reachableBlockCount();

  // All reachable -> nothing to do.
  if (numAllBlocks == numReachableBlocks)
    return kErrorOk;

  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = debugLogger();
  #endif
  ASMJIT_RA_LOG_FORMAT("[RAPass::RemoveUnreachableBlocks (%u of %u unreachable)]\n", numAllBlocks - numReachableBlocks, numAllBlocks);

  for (uint32_t i = 0; i < numAllBlocks; i++) {
    RABlock* block = _blocks[i];
    if (block->isReachable())
      continue;

    ASMJIT_RA_LOG_FORMAT("  Removing block {%u}\n", i);
    BaseNode* first = block->first();
    BaseNode* last = block->last();

    BaseNode* beforeFirst = first->prev();
    BaseNode* afterLast = last->next();

    // Skip labels as they can be used as reference points.
    while (first->isLabel() && first != afterLast)
      first = first->next();

    // Just to control flow.
    for (;;) {
      if (first == afterLast)
        break;

      // Don't know a better way atm, .align nodes before labels should be preserved.
      if (last->type() == BaseNode::kNodeAlign) {
        if (first == last)
          break;
        last = last->prev();
      }

      bool wholeBlockGone = (first == block->first() && last == block->last());
      cc()->removeNodes(first, last);

      if (wholeBlockGone) {
        block->setFirst(nullptr);
        block->setLast(nullptr);
      }
      else {
        block->setFirst(beforeFirst->next());
        block->setLast(afterLast->prev());
      }
      break;
    }
  }

  return kErrorOk;
}

BaseNode* RAPass::findSuccessorStartingAt(BaseNode* node) noexcept {
  while (node && (node->isInformative() || node->hasNoEffect()))
    node = node->next();
  return node;
}

bool RAPass::isNextTo(BaseNode* node, BaseNode* target) noexcept {
  for (;;) {
    node = node->next();
    if (node == target)
      return true;

    if (!node)
      return false;

    if (node->isCode() || node->isData())
      return false;
  }
}

// ============================================================================
// [asmjit::RAPass - ?]
// ============================================================================

Error RAPass::_asWorkReg(VirtReg* vReg, RAWorkReg** out) noexcept {
  // Checked by `asWorkReg()` - must be true.
  ASMJIT_ASSERT(vReg->_workReg == nullptr);

  uint32_t group = vReg->group();
  ASMJIT_ASSERT(group < BaseReg::kGroupVirt);

  RAWorkRegs& wRegs = workRegs();
  RAWorkRegs& wRegsByGroup = workRegs(group);

  ASMJIT_PROPAGATE(wRegs.willGrow(allocator()));
  ASMJIT_PROPAGATE(wRegsByGroup.willGrow(allocator()));

  RAWorkReg* wReg = zone()->newT<RAWorkReg>(vReg, wRegs.size());
  if (ASMJIT_UNLIKELY(!wReg))
    return DebugUtils::errored(kErrorOutOfMemory);

  vReg->setWorkReg(wReg);
  wRegs.appendUnsafe(wReg);
  wRegsByGroup.appendUnsafe(wReg);

  // Only used by RA logging.
  _maxWorkRegNameSize = Support::max(_maxWorkRegNameSize, vReg->nameSize());

  *out = wReg;
  return kErrorOk;
}

RAAssignment::WorkToPhysMap* RAPass::newWorkToPhysMap() noexcept {
  uint32_t count = workRegCount();
  size_t size = WorkToPhysMap::sizeOf(count);

  // If no registers are used it could be zero, in that case return a dummy
  // map instead of NULL.
  if (ASMJIT_UNLIKELY(!size)) {
    static const RAAssignment::WorkToPhysMap nullMap = {{ 0 }};
    return const_cast<RAAssignment::WorkToPhysMap*>(&nullMap);
  }

  WorkToPhysMap* map = zone()->allocT<WorkToPhysMap>(size);
  if (ASMJIT_UNLIKELY(!map))
    return nullptr;

  map->reset(count);
  return map;
}

RAAssignment::PhysToWorkMap* RAPass::newPhysToWorkMap() noexcept {
  uint32_t count = physRegTotal();
  size_t size = PhysToWorkMap::sizeOf(count);

  PhysToWorkMap* map = zone()->allocT<PhysToWorkMap>(size);
  if (ASMJIT_UNLIKELY(!map))
    return nullptr;

  map->reset(count);
  return map;
}

// ============================================================================
// [asmjit::RAPass - Registers - Liveness Analysis and Statistics]
// ============================================================================

namespace LiveOps {
  typedef ZoneBitVector::BitWord BitWord;

  struct In {
    static ASMJIT_INLINE BitWord op(BitWord dst, BitWord out, BitWord gen, BitWord kill) noexcept {
      ASMJIT_UNUSED(dst);
      return (out | gen) & ~kill;
    }
  };

  template<typename Operator>
  static ASMJIT_INLINE bool op(BitWord* dst, const BitWord* a, uint32_t n) noexcept {
    BitWord changed = 0;

    for (uint32_t i = 0; i < n; i++) {
      BitWord before = dst[i];
      BitWord after = Operator::op(before, a[i]);

      dst[i] = after;
      changed |= (before ^ after);
    }

    return changed != 0;
  }

  template<typename Operator>
  static ASMJIT_INLINE bool op(BitWord* dst, const BitWord* a, const BitWord* b, uint32_t n) noexcept {
    BitWord changed = 0;

    for (uint32_t i = 0; i < n; i++) {
      BitWord before = dst[i];
      BitWord after = Operator::op(before, a[i], b[i]);

      dst[i] = after;
      changed |= (before ^ after);
    }

    return changed != 0;
  }

  template<typename Operator>
  static ASMJIT_INLINE bool op(BitWord* dst, const BitWord* a, const BitWord* b, const BitWord* c, uint32_t n) noexcept {
    BitWord changed = 0;

    for (uint32_t i = 0; i < n; i++) {
      BitWord before = dst[i];
      BitWord after = Operator::op(before, a[i], b[i], c[i]);

      dst[i] = after;
      changed |= (before ^ after);
    }

    return changed != 0;
  }

  static ASMJIT_INLINE bool recalcInOut(RABlock* block, uint32_t numBitWords, bool initial = false) noexcept {
    bool changed = initial;

    const RABlocks& successors = block->successors();
    uint32_t numSuccessors = successors.size();

    // Calculate `OUT` based on `IN` of all successors.
    for (uint32_t i = 0; i < numSuccessors; i++)
      changed |= op<Support::Or>(block->liveOut().data(), successors[i]->liveIn().data(), numBitWords);

    // Calculate `IN` based on `OUT`, `GEN`, and `KILL` bits.
    if (changed)
      changed = op<In>(block->liveIn().data(), block->liveOut().data(), block->gen().data(), block->kill().data(), numBitWords);

    return changed;
  }
}

ASMJIT_FAVOR_SPEED Error RAPass::buildLiveness() noexcept {
  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = debugLogger();
  StringTmp<512> sb;
  #endif

  ASMJIT_RA_LOG_FORMAT("[RAPass::BuildLiveness]\n");

  uint32_t i;

  uint32_t numAllBlocks = blockCount();
  uint32_t numReachableBlocks = reachableBlockCount();

  uint32_t numVisits = numReachableBlocks;
  uint32_t numWorkRegs = workRegCount();
  uint32_t numBitWords = ZoneBitVector::_wordsPerBits(numWorkRegs);

  if (!numWorkRegs) {
    ASMJIT_RA_LOG_FORMAT("  Done (no virtual registers)\n");
    return kErrorOk;
  }

  ZoneVector<uint32_t> nUsesPerWorkReg; // Number of USEs of each RAWorkReg.
  ZoneVector<uint32_t> nOutsPerWorkReg; // Number of OUTs of each RAWorkReg.
  ZoneVector<uint32_t> nInstsPerBlock;  // Number of instructions of each RABlock.

  ASMJIT_PROPAGATE(nUsesPerWorkReg.resize(allocator(), numWorkRegs));
  ASMJIT_PROPAGATE(nOutsPerWorkReg.resize(allocator(), numWorkRegs));
  ASMJIT_PROPAGATE(nInstsPerBlock.resize(allocator(), numAllBlocks));

  // --------------------------------------------------------------------------
  // Calculate GEN/KILL of each block.
  // --------------------------------------------------------------------------

  for (i = 0; i < numReachableBlocks; i++) {
    RABlock* block = _pov[i];
    ASMJIT_PROPAGATE(block->resizeLiveBits(numWorkRegs));

    BaseNode* node = block->last();
    BaseNode* stop = block->first();

    uint32_t nInsts = 0;
    for (;;) {
      if (node->isInst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* raInst = inst->passData<RAInst>();
        ASMJIT_ASSERT(raInst != nullptr);

        RATiedReg* tiedRegs = raInst->tiedRegs();
        uint32_t count = raInst->tiedCount();

        for (uint32_t j = 0; j < count; j++) {
          RATiedReg* tiedReg = &tiedRegs[j];
          uint32_t workId = tiedReg->workId();

          // Update `nUses` and `nOuts`.
          nUsesPerWorkReg[workId] += 1u;
          nOutsPerWorkReg[workId] += uint32_t(tiedReg->isWrite());

          // Mark as:
          //   KILL - if this VirtReg is killed afterwards.
          //   LAST - if this VirtReg is last in this basic block.
          if (block->kill().bitAt(workId))
            tiedReg->addFlags(RATiedReg::kKill);
          else if (!block->gen().bitAt(workId))
            tiedReg->addFlags(RATiedReg::kLast);

          if (tiedReg->isWriteOnly()) {
            // KILL.
            block->kill().setBit(workId, true);
          }
          else {
            // GEN.
            block->kill().setBit(workId, false);
            block->gen().setBit(workId, true);
          }
        }

        nInsts++;
      }

      if (node == stop)
        break;

      node = node->prev();
      ASMJIT_ASSERT(node != nullptr);
    }

    nInstsPerBlock[block->blockId()] = nInsts;
  }

  // --------------------------------------------------------------------------
  // Calculate IN/OUT of each block.
  // --------------------------------------------------------------------------

  {
    ZoneStack<RABlock*> workList;
    ZoneBitVector workBits;

    ASMJIT_PROPAGATE(workList.init(allocator()));
    ASMJIT_PROPAGATE(workBits.resize(allocator(), blockCount(), true));

    for (i = 0; i < numReachableBlocks; i++) {
      RABlock* block = _pov[i];
      LiveOps::recalcInOut(block, numBitWords, true);
      ASMJIT_PROPAGATE(workList.append(block));
    }

    while (!workList.empty()) {
      RABlock* block = workList.popFirst();
      uint32_t blockId = block->blockId();

      workBits.setBit(blockId, false);
      if (LiveOps::recalcInOut(block, numBitWords)) {
        const RABlocks& predecessors = block->predecessors();
        uint32_t numPredecessors = predecessors.size();

        for (uint32_t j = 0; j < numPredecessors; j++) {
          RABlock* pred = predecessors[j];
          if (!workBits.bitAt(pred->blockId())) {
            workBits.setBit(pred->blockId(), true);
            ASMJIT_PROPAGATE(workList.append(pred));
          }
        }
      }
      numVisits++;
    }

    workList.reset();
    workBits.release(allocator());
  }

  ASMJIT_RA_LOG_COMPLEX({
    logger->logf("  LiveIn/Out Done (%u visits)\n", numVisits);
    for (i = 0; i < numAllBlocks; i++) {
      RABlock* block = _blocks[i];

      ASMJIT_PROPAGATE(sb.assignFormat("  {#%u}\n", block->blockId()));
      ASMJIT_PROPAGATE(_dumpBlockLiveness(sb, block));

      logger->log(sb);
    }
  });

  // --------------------------------------------------------------------------
  // Reserve the space in each `RAWorkReg` for references.
  // --------------------------------------------------------------------------

  for (i = 0; i < numWorkRegs; i++) {
    RAWorkReg* workReg = workRegById(i);
    ASMJIT_PROPAGATE(workReg->_refs.reserve(allocator(), nUsesPerWorkReg[i]));
    ASMJIT_PROPAGATE(workReg->_writes.reserve(allocator(), nOutsPerWorkReg[i]));
  }

  // --------------------------------------------------------------------------
  // Assign block and instruction positions, build LiveCount and LiveSpans.
  // --------------------------------------------------------------------------

  uint32_t position = 2;
  for (i = 0; i < numAllBlocks; i++) {
    RABlock* block = _blocks[i];
    if (!block->isReachable())
      continue;

    BaseNode* node = block->first();
    BaseNode* stop = block->last();

    uint32_t endPosition = position + nInstsPerBlock[i] * 2;
    block->setFirstPosition(position);
    block->setEndPosition(endPosition);

    RALiveCount curLiveCount;
    RALiveCount maxLiveCount;

    // Process LIVE-IN.
    ZoneBitVector::ForEachBitSet it(block->liveIn());
    while (it.hasNext()) {
      RAWorkReg* workReg = _workRegs[uint32_t(it.next())];
      curLiveCount[workReg->group()]++;
      ASMJIT_PROPAGATE(workReg->liveSpans().openAt(allocator(), position, endPosition));
    }

    for (;;) {
      if (node->isInst()) {
        InstNode* inst = node->as<InstNode>();
        RAInst* raInst = inst->passData<RAInst>();
        ASMJIT_ASSERT(raInst != nullptr);

        RATiedReg* tiedRegs = raInst->tiedRegs();
        uint32_t count = raInst->tiedCount();

        inst->setPosition(position);
        raInst->_liveCount = curLiveCount;

        for (uint32_t j = 0; j < count; j++) {
          RATiedReg* tiedReg = &tiedRegs[j];
          uint32_t workId = tiedReg->workId();

          // Create refs and writes.
          RAWorkReg* workReg = workRegById(workId);
          workReg->_refs.appendUnsafe(node);
          if (tiedReg->isWrite())
            workReg->_writes.appendUnsafe(node);

          // We couldn't calculate this in previous steps, but since we know all LIVE-OUT
          // at this point it becomes trivial. If this is the last instruction that uses
          // this `workReg` and it's not LIVE-OUT then it is KILLed here.
          if (tiedReg->isLast() && !block->liveOut().bitAt(workId))
            tiedReg->addFlags(RATiedReg::kKill);

          LiveRegSpans& liveSpans = workReg->liveSpans();
          bool wasOpen;
          ASMJIT_PROPAGATE(liveSpans.openAt(allocator(), position + !tiedReg->isRead(), endPosition, wasOpen));

          uint32_t group = workReg->group();
          if (!wasOpen) {
            curLiveCount[group]++;
            raInst->_liveCount[group]++;
          }

          if (tiedReg->isKill()) {
            liveSpans.closeAt(position + !tiedReg->isRead() + 1);
            curLiveCount[group]--;
          }

          // Update `RAWorkReg::hintRegId`.
          if (tiedReg->hasUseId() && !workReg->hasHintRegId()) {
            uint32_t useId = tiedReg->useId();
            if (!(raInst->_clobberedRegs[group] & Support::bitMask(useId)))
              workReg->setHintRegId(useId);
          }

          // Update `RAWorkReg::clobberedSurvivalMask`.
          if (raInst->_clobberedRegs[group] && !tiedReg->isOutOrKill())
            workReg->addClobberSurvivalMask(raInst->_clobberedRegs[group]);
        }

        position += 2;
        maxLiveCount.op<Support::Max>(raInst->_liveCount);
      }

      if (node == stop)
        break;

      node = node->next();
      ASMJIT_ASSERT(node != nullptr);
    }

    block->_maxLiveCount = maxLiveCount;
    _globalMaxLiveCount.op<Support::Max>(maxLiveCount);
    ASMJIT_ASSERT(position == block->endPosition());
  }

  // --------------------------------------------------------------------------
  // Calculate WorkReg statistics.
  // --------------------------------------------------------------------------

  for (i = 0; i < numWorkRegs; i++) {
    RAWorkReg* workReg = _workRegs[i];

    LiveRegSpans& spans = workReg->liveSpans();
    uint32_t width = spans.width();
    float freq = width ? float(double(workReg->_refs.size()) / double(width)) : float(0);

    RALiveStats& stats = workReg->liveStats();
    stats._width = width;
    stats._freq = freq;
    stats._priority = freq + float(int(workReg->virtReg()->weight())) * 0.01f;
  }

  ASMJIT_RA_LOG_COMPLEX({
    sb.clear();
    _dumpLiveSpans(sb);
    logger->log(sb);
  });

  nUsesPerWorkReg.release(allocator());
  nOutsPerWorkReg.release(allocator());
  nInstsPerBlock.release(allocator());

  return kErrorOk;
}

Error RAPass::assignArgIndexToWorkRegs() noexcept {
  ZoneBitVector& liveIn = entryBlock()->liveIn();
  uint32_t argCount = func()->argCount();

  for (uint32_t i = 0; i < argCount; i++) {
    // Unassigned argument.
    VirtReg* virtReg = func()->arg(i);
    if (!virtReg) continue;

    // Unreferenced argument.
    RAWorkReg* workReg = virtReg->workReg();
    if (!workReg) continue;

    // Overwritten argument.
    uint32_t workId = workReg->workId();
    if (!liveIn.bitAt(workId))
      continue;

    workReg->setArgIndex(i);

    const FuncValue& arg = func()->detail().arg(i);
    if (arg.isReg() && _archRegsInfo->regInfo[arg.regType()].group() == workReg->group()) {
      workReg->setHintRegId(arg.regId());
    }
  }

  return kErrorOk;
}
// ============================================================================
// [asmjit::RAPass - Allocation - Global]
// ============================================================================

static void RAPass_dumpSpans(String& sb, uint32_t index, const LiveRegSpans& liveSpans) noexcept {
  sb.appendFormat("  %02u: ", index);

  for (uint32_t i = 0; i < liveSpans.size(); i++) {
    const LiveRegSpan& liveSpan = liveSpans[i];
    if (i) sb.appendString(", ");
    sb.appendFormat("[%u:%u@%u]", liveSpan.a, liveSpan.b, liveSpan.id);
  }

  sb.appendChar('\n');
}

Error RAPass::runGlobalAllocator() noexcept {
  ASMJIT_PROPAGATE(initGlobalLiveSpans());

  for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++) {
    ASMJIT_PROPAGATE(binPack(group));
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SPEED Error RAPass::initGlobalLiveSpans() noexcept {
  for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++) {
    size_t physCount = _physRegCount[group];
    LiveRegSpans* liveSpans = allocator()->allocT<LiveRegSpans>(physCount * sizeof(LiveRegSpans));

    if (ASMJIT_UNLIKELY(!liveSpans))
      return DebugUtils::errored(kErrorOutOfMemory);

    for (size_t physId = 0; physId < physCount; physId++)
      new(Support::PlacementNew { &liveSpans[physId] }) LiveRegSpans();

    _globalLiveSpans[group] = liveSpans;
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SPEED Error RAPass::binPack(uint32_t group) noexcept {
  if (workRegCount(group) == 0)
    return kErrorOk;

  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = debugLogger();
  StringTmp<512> sb;
  #endif

  ASMJIT_RA_LOG_FORMAT("[RAPass::BinPack] Available=%u (0x%08X) Count=%u\n",
    Support::popcnt(_availableRegs[group]),
    _availableRegs[group],
    workRegCount(group));

  uint32_t i;
  uint32_t physCount = _physRegCount[group];

  RAWorkRegs workRegs;
  LiveRegSpans tmpSpans;

  ASMJIT_PROPAGATE(workRegs.concat(allocator(), this->workRegs(group)));
  workRegs.sort([](const RAWorkReg* a, const RAWorkReg* b) noexcept {
    return b->liveStats().priority() - a->liveStats().priority();
  });

  uint32_t numWorkRegs = workRegs.size();
  uint32_t availableRegs = _availableRegs[group];

  // First try to pack everything that provides register-id hint as these are
  // most likely function arguments and fixed (precolored) virtual registers.
  if (!workRegs.empty()) {
    uint32_t dstIndex = 0;

    for (i = 0; i < numWorkRegs; i++) {
      RAWorkReg* workReg = workRegs[i];
      if (workReg->hasHintRegId()) {
        uint32_t physId = workReg->hintRegId();
        if (availableRegs & Support::bitMask(physId)) {
          LiveRegSpans& live = _globalLiveSpans[group][physId];
          Error err = tmpSpans.nonOverlappingUnionOf(allocator(), live, workReg->liveSpans(), LiveRegData(workReg->virtId()));

          if (err == kErrorOk) {
            workReg->setHomeRegId(physId);
            live.swap(tmpSpans);
            continue;
          }

          if (ASMJIT_UNLIKELY(err != 0xFFFFFFFFu))
            return err;
        }
      }

      workRegs[dstIndex++] = workReg;
    }

    workRegs._setSize(dstIndex);
    numWorkRegs = dstIndex;
  }

  // Try to pack the rest.
  if (!workRegs.empty()) {
    uint32_t dstIndex = 0;

    for (i = 0; i < numWorkRegs; i++) {
      RAWorkReg* workReg = workRegs[i];
      uint32_t physRegs = availableRegs;

      while (physRegs) {
        uint32_t physId = Support::ctz(physRegs);
        if (workReg->clobberSurvivalMask()) {
          uint32_t preferredMask = physRegs & workReg->clobberSurvivalMask();
          if (preferredMask)
            physId = Support::ctz(preferredMask);
        }

        LiveRegSpans& live = _globalLiveSpans[group][physId];
        Error err = tmpSpans.nonOverlappingUnionOf(allocator(), live, workReg->liveSpans(), LiveRegData(workReg->virtId()));

        if (err == kErrorOk) {
          workReg->setHomeRegId(physId);
          live.swap(tmpSpans);
          break;
        }

        if (ASMJIT_UNLIKELY(err != 0xFFFFFFFFu))
          return err;

        physRegs ^= Support::bitMask(physId);
      }

      // Keep it in `workRegs` if it was not allocated.
      if (!physRegs)
        workRegs[dstIndex++] = workReg;
    }

    workRegs._setSize(dstIndex);
    numWorkRegs = dstIndex;
  }

  ASMJIT_RA_LOG_COMPLEX({
    for (uint32_t physId = 0; physId < physCount; physId++) {
      LiveRegSpans& live = _globalLiveSpans[group][physId];
      if (live.empty())
        continue;

      sb.clear();
      RAPass_dumpSpans(sb, physId, live);
      logger->log(sb);
    }
  });

  if (workRegs.empty()) {
    ASMJIT_RA_LOG_FORMAT("  Completed.\n");
  }
  else {
    _strategy[group].setType(RAStrategy::kStrategyComplex);
    for (RAWorkReg* workReg : workRegs)
      workReg->markStackPreferred();

    ASMJIT_RA_LOG_COMPLEX({
      uint32_t numWorkRegs = workRegs.size();
      sb.clear();
      sb.appendFormat("  Unassigned (%u): ", numWorkRegs);
      for (i = 0; i < numWorkRegs; i++) {
        RAWorkReg* workReg = workRegs[i];
        if (i) sb.appendString(", ");
        sb.appendString(workReg->name());
      }
      sb.appendChar('\n');
      logger->log(sb);
    });
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Allocation - Local]
// ============================================================================

Error RAPass::runLocalAllocator() noexcept {
  RALocalAllocator lra(this);
  ASMJIT_PROPAGATE(lra.init());

  if (!blockCount())
    return kErrorOk;

  // The allocation is done when this reaches zero.
  uint32_t blocksRemaining = reachableBlockCount();

  // Current block.
  uint32_t blockId = 0;
  RABlock* block = _blocks[blockId];

  // The first block (entry) must always be reachable.
  ASMJIT_ASSERT(block->isReachable());

  // Assign function arguments for the initial block. The `lra` is valid now.
  lra.makeInitialAssignment();
  ASMJIT_PROPAGATE(setBlockEntryAssignment(block, block, lra._curAssignment));

  // The loop starts from the first block and iterates blocks in order, however,
  // the algorithm also allows to jump to any other block when finished if it's
  // a jump target. In-order iteration just makes sure that all blocks are visited.
  for (;;) {
    BaseNode* first = block->first();
    BaseNode* last = block->last();
    BaseNode* terminator = block->hasTerminator() ? last : nullptr;

    BaseNode* beforeFirst = first->prev();
    BaseNode* afterLast = last->next();

    bool unconditionalJump = false;
    RABlock* consecutive = nullptr;

    if (block->hasSuccessors())
      consecutive = block->successors()[0];

    lra.setBlock(block);
    block->makeAllocated();

    for (BaseNode* node = first; node != afterLast; node = node->next()) {
      if (node->isInst()) {
        InstNode* inst = node->as<InstNode>();

        if (ASMJIT_UNLIKELY(inst == terminator)) {
          const RABlocks& successors = block->successors();
          if (block->hasConsecutive()) {
            ASMJIT_PROPAGATE(lra.allocBranch(inst, successors.last(), successors.first()));
            continue;
          }
          else if (successors.size() > 1) {
            // TODO: Jump table.
            ASMJIT_ASSERT(false);
          }
          else {
            // Otherwise this is an unconditional jump, special handling isn't required.
            unconditionalJump = true;
          }
        }

        ASMJIT_PROPAGATE(lra.allocInst(inst));
        if (inst->type() == BaseNode::kNodeFuncCall)
          ASMJIT_PROPAGATE(onEmitPreCall(inst->as<FuncCallNode>()));
      }
    }

    if (consecutive) {
      if (consecutive->hasEntryAssignment()) {
        BaseNode* prev = afterLast ? afterLast->prev() : cc()->lastNode();
        cc()->_setCursor(unconditionalJump ? prev->prev() : prev);

        ASMJIT_PROPAGATE(
          lra.switchToAssignment(
            consecutive->entryPhysToWorkMap(),
            consecutive->entryWorkToPhysMap(),
            consecutive->liveIn(),
            consecutive->isAllocated(),
            false));
      }
      else {
        ASMJIT_PROPAGATE(setBlockEntryAssignment(consecutive, block, lra._curAssignment));
        lra._curAssignment.copyFrom(consecutive->entryPhysToWorkMap(), consecutive->entryWorkToPhysMap());
      }
    }

    // Important as the local allocator can insert instructions before
    // and after any instruction within the basic block.
    block->setFirst(beforeFirst->next());
    block->setLast(afterLast ? afterLast->prev() : cc()->lastNode());

    if (--blocksRemaining == 0)
      break;

    // Switch to the next consecutive block, if any.
    if (consecutive) {
      block = consecutive;
      if (!block->isAllocated())
        continue;
    }

    // Get the next block.
    for (;;) {
      if (++blockId >= blockCount())
        blockId = 0;

      block = _blocks[blockId];
      if (!block->isReachable() || block->isAllocated() || !block->hasEntryAssignment())
        continue;

      break;
    }

    // If we switched to some block we have to update `lra`.
    lra.replaceAssignment(block->entryPhysToWorkMap(), block->entryWorkToPhysMap());
  }

  _clobberedRegs.op<Support::Or>(lra._clobberedRegs);
  return kErrorOk;
}

Error RAPass::setBlockEntryAssignment(RABlock* block, const RABlock* fromBlock, const RAAssignment& fromAssignment) noexcept {
  PhysToWorkMap* physToWorkMap = clonePhysToWorkMap(fromAssignment.physToWorkMap());
  WorkToPhysMap* workToPhysMap = cloneWorkToPhysMap(fromAssignment.workToPhysMap());

  if (ASMJIT_UNLIKELY(!physToWorkMap || !workToPhysMap))
    return DebugUtils::errored(kErrorOutOfMemory);

  block->setEntryAssignment(physToWorkMap, workToPhysMap);

  // Must be first block, otherwise impossible.
  if (block == fromBlock)
    return kErrorOk;

  const ZoneBitVector& liveOut = fromBlock->liveOut();
  const ZoneBitVector& liveIn  = block->liveIn();

  RAAssignment as;
  as.initLayout(_physRegCount, workRegs());
  as.initMaps(physToWorkMap, workToPhysMap);

  // It's possible that `fromBlock` has LIVE-OUT regs that `block` doesn't
  // have in LIVE-IN, these have to be unassigned.
  {
    ZoneBitVector::ForEachBitOp<Support::AndNot> it(liveOut, liveIn);
    while (it.hasNext()) {
      uint32_t workId = uint32_t(it.next());
      RAWorkReg* workReg = workRegById(workId);

      uint32_t group = workReg->group();
      uint32_t physId = as.workToPhysId(group, workId);

      if (physId != RAAssignment::kPhysNone)
        as.unassign(group, workId, physId);
    }
  }

  // Complex allocation strategy: Record register assignments upon block entry.
  {
    for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++) {
      if (_strategy[group].isComplex()) {
        Support::BitWordIterator<uint32_t> it(as.assigned(group));
        while (it.hasNext()) {
          uint32_t physId = it.next();
          uint32_t workId = as.physToWorkId(group, physId);

          RAWorkReg* workReg = workRegById(workId);
          workReg->addAllocatedMask(Support::bitMask(physId));
        }
      }
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Allocation - Utilities]
// ============================================================================

Error RAPass::useTemporaryMem(BaseMem& out, uint32_t size, uint32_t alignment) noexcept {
  ASMJIT_ASSERT(alignment <= 64);

  if (_temporaryMem.isNone()) {
    ASMJIT_PROPAGATE(cc()->_newStack(_temporaryMem.as<BaseMem>(), size, alignment));
  }
  else {
    ASMJIT_ASSERT(_temporaryMem.as<BaseMem>().isRegHome());
    uint32_t virtId = _temporaryMem.as<BaseMem>().baseId();

    VirtReg* virtReg = cc()->virtRegById(virtId);
    virtReg->_virtSize = Support::max(virtReg->virtSize(), size);
    virtReg->_alignment = uint8_t(Support::max(virtReg->alignment(), alignment));
  }

  out = _temporaryMem.as<BaseMem>();
  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Allocation - Prolog / Epilog]
// ============================================================================

Error RAPass::updateStackFrame() noexcept {
  // Update some StackFrame information that we updated during allocation. The
  // only information we don't have at the moment is final local stack size,
  // which is calculated last.
  FuncFrame& frame = func()->frame();
  for (uint32_t group = 0; group < BaseReg::kGroupVirt; group++)
    frame.addDirtyRegs(group, _clobberedRegs[group]);
  frame.setLocalStackAlignment(_stackAllocator.alignment());

  // If there are stack arguments that are not assigned to registers upon entry
  // and the function doesn't require dynamic stack alignment we keep these
  // arguments where they are. This will also mark all stack slots that match
  // these arguments as allocated.
  if (_numStackArgsToStackSlots)
    ASMJIT_PROPAGATE(_markStackArgsToKeep());

  // Calculate offsets of all stack slots and update StackSize to reflect the calculated local stack size.
  ASMJIT_PROPAGATE(_stackAllocator.calculateStackFrame());
  frame.setLocalStackSize(_stackAllocator.stackSize());

  // Update the stack frame based on `_argsAssignment` and finalize it.
  // Finalization means to apply final calculation to the stack layout.
  ASMJIT_PROPAGATE(_argsAssignment.updateFuncFrame(frame));
  ASMJIT_PROPAGATE(frame.finalize());

  // StackAllocator allocates all stots starting from [0], adjust them when necessary.
  if (frame.localStackOffset() != 0)
    ASMJIT_PROPAGATE(_stackAllocator.adjustSlotOffsets(int32_t(frame.localStackOffset())));

  // Again, if there are stack arguments allocated in function's stack we have
  // to handle them. This handles all cases (either regular or dynamic stack
  // alignment).
  if (_numStackArgsToStackSlots)
    ASMJIT_PROPAGATE(_updateStackArgs());

  return kErrorOk;
}

Error RAPass::_markStackArgsToKeep() noexcept {
  FuncFrame& frame = func()->frame();
  bool hasSAReg = frame.hasPreservedFP() || !frame.hasDynamicAlignment();

  RAWorkRegs& workRegs = _workRegs;
  uint32_t numWorkRegs = workRegCount();

  for (uint32_t workId = 0; workId < numWorkRegs; workId++) {
    RAWorkReg* workReg = workRegs[workId];
    if (workReg->hasFlag(RAWorkReg::kFlagStackArgToStack)) {
      ASMJIT_ASSERT(workReg->hasArgIndex());
      const FuncValue& srcArg = _func->detail().arg(workReg->argIndex());

      // If the register doesn't have stack slot then we failed. It doesn't
      // make much sense as it was marked as `kFlagStackArgToStack`, which
      // requires the WorkReg was live-in upon function entry.
      RAStackSlot* slot = workReg->stackSlot();
      if (ASMJIT_UNLIKELY(!slot))
        return DebugUtils::errored(kErrorInvalidState);

      if (hasSAReg && srcArg.isStack() && !srcArg.isIndirect()) {
        uint32_t typeSize = Type::sizeOf(srcArg.typeId());
        if (typeSize == slot->size()) {
          slot->addFlags(RAStackSlot::kFlagStackArg);
          continue;
        }
      }

      // NOTE: Update StackOffset here so when `_argsAssignment.updateFuncFrame()`
      // is called it will take into consideration moving to stack slots. Without
      // this we may miss some scratch registers later.
      FuncValue& dstArg = _argsAssignment.arg(workReg->argIndex());
      dstArg.assignStackOffset(0);
    }
  }

  return kErrorOk;
}

Error RAPass::_updateStackArgs() noexcept {
  FuncFrame& frame = func()->frame();
  RAWorkRegs& workRegs = _workRegs;
  uint32_t numWorkRegs = workRegCount();

  for (uint32_t workId = 0; workId < numWorkRegs; workId++) {
    RAWorkReg* workReg = workRegs[workId];
    if (workReg->hasFlag(RAWorkReg::kFlagStackArgToStack)) {
      ASMJIT_ASSERT(workReg->hasArgIndex());
      RAStackSlot* slot = workReg->stackSlot();

      if (ASMJIT_UNLIKELY(!slot))
        return DebugUtils::errored(kErrorInvalidState);

      if (slot->isStackArg()) {
        const FuncValue& srcArg = _func->detail().arg(workReg->argIndex());
        if (frame.hasPreservedFP()) {
          slot->setBaseRegId(_fp.id());
          slot->setOffset(int32_t(frame.saOffsetFromSA()) + srcArg.stackOffset());
        }
        else {
          slot->setOffset(int32_t(frame.saOffsetFromSP()) + srcArg.stackOffset());
        }
      }
      else {
        FuncValue& dstArg = _argsAssignment.arg(workReg->argIndex());
        dstArg.setStackOffset(slot->offset());
      }
    }
  }

  return kErrorOk;
}

Error RAPass::insertPrologEpilog() noexcept {
  FuncFrame& frame = _func->frame();

  cc()->_setCursor(func());
  ASMJIT_PROPAGATE(cc()->emitProlog(frame));
  ASMJIT_PROPAGATE(cc()->emitArgsAssignment(frame, _argsAssignment));

  cc()->_setCursor(func()->exitNode());
  ASMJIT_PROPAGATE(cc()->emitEpilog(frame));

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Rewriter]
// ============================================================================

Error RAPass::rewrite() noexcept {
  #ifndef ASMJIT_DISABLE_LOGGING
  Logger* logger = debugLogger();
  #endif

  ASMJIT_RA_LOG_FORMAT("[RAPass::Rewrite]\n");
  return _rewrite(_func, _stop);
}

ASMJIT_FAVOR_SPEED Error RAPass::_rewrite(BaseNode* first, BaseNode* stop) noexcept {
  uint32_t virtCount = cc()->_vRegArray.size();

  BaseNode* node = first;
  while (node != stop) {
    BaseNode* next = node->next();
    if (node->isInst()) {
      InstNode* inst = node->as<InstNode>();
      RAInst* raInst = node->passData<RAInst>();

      Operand* operands = inst->operands();
      uint32_t opCount = inst->opCount();
      uint32_t i;

      // Rewrite virtual registers into physical registers.
      if (ASMJIT_LIKELY(raInst)) {
        // If the instruction contains pass data (raInst) then it was a subject
        // for register allocation and must be rewritten to use physical regs.
        RATiedReg* tiedRegs = raInst->tiedRegs();
        uint32_t tiedCount = raInst->tiedCount();

        for (i = 0; i < tiedCount; i++) {
          RATiedReg* tiedReg = &tiedRegs[i];

          Support::BitWordIterator<uint32_t> useIt(tiedReg->useRewriteMask());
          uint32_t useId = tiedReg->useId();
          while (useIt.hasNext()) inst->rewriteIdAtIndex(useIt.next(), useId);

          Support::BitWordIterator<uint32_t> outIt(tiedReg->outRewriteMask());
          uint32_t outId = tiedReg->outId();
          while (outIt.hasNext()) inst->rewriteIdAtIndex(outIt.next(), outId);
        }

        // This data is allocated by Zone passed to `runOnFunction()`, which
        // will be reset after the RA pass finishes. So reset this data to
        // prevent having a dead pointer after RA pass is complete.
        node->resetPassData();

        if (ASMJIT_UNLIKELY(node->type() != BaseNode::kNodeInst)) {
          // FuncRet terminates the flow, it must either be removed if the exit
          // label is next to it (optimization) or patched to an architecture
          // dependent jump instruction that jumps to the function's exit before
          // the epilog.
          if (node->type() == BaseNode::kNodeFuncRet) {
            RABlock* block = raInst->block();
            if (!isNextTo(node, _func->exitNode())) {
              cc()->_setCursor(node->prev());
              ASMJIT_PROPAGATE(onEmitJump(_func->exitNode()->label()));
            }

            BaseNode* prev = node->prev();
            cc()->removeNode(node);
            block->setLast(prev);
          }
        }
      }

      // Rewrite stack slot addresses.
      for (i = 0; i < opCount; i++) {
        Operand& op = operands[i];
        if (op.isMem()) {
          BaseMem& mem = op.as<BaseMem>();
          if (mem.isRegHome()) {
            uint32_t virtIndex = Operand::virtIdToIndex(mem.baseId());
            if (ASMJIT_UNLIKELY(virtIndex >= virtCount))
              return DebugUtils::errored(kErrorInvalidVirtId);

            VirtReg* virtReg = cc()->virtRegByIndex(virtIndex);
            RAWorkReg* workReg = virtReg->workReg();
            ASMJIT_ASSERT(workReg != nullptr);

            RAStackSlot* slot = workReg->stackSlot();
            int32_t offset = slot->offset();

            mem._setBase(_sp.type(), slot->baseRegId());
            mem.clearRegHome();
            mem.addOffsetLo32(offset);
          }
        }
      }
    }

    node = next;
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::RAPass - Logging]
// ============================================================================

#ifndef ASMJIT_DISABLE_LOGGING
static void RAPass_dumpRAInst(RAPass* pass, String& sb, const RAInst* raInst) noexcept {
  const RATiedReg* tiedRegs = raInst->tiedRegs();
  uint32_t tiedCount = raInst->tiedCount();

  for (uint32_t i = 0; i < tiedCount; i++) {
    const RATiedReg& tiedReg = tiedRegs[i];

    if (i != 0) sb.appendChar(' ');

    sb.appendFormat("%s{", pass->workRegById(tiedReg.workId())->name());
    sb.appendChar(tiedReg.isReadWrite() ? 'X' :
                  tiedReg.isRead()      ? 'R' :
                  tiedReg.isWrite()     ? 'W' : '?');

    if (tiedReg.hasUseId())
      sb.appendFormat("|Use=%u", tiedReg.useId());
    else if (tiedReg.isUse())
      sb.appendString("|Use");

    if (tiedReg.hasOutId())
      sb.appendFormat("|Out=%u", tiedReg.outId());
    else if (tiedReg.isOut())
      sb.appendString("|Out");

    if (tiedReg.isLast()) sb.appendString("|Last");
    if (tiedReg.isKill()) sb.appendString("|Kill");

    sb.appendString("}");
  }
}

ASMJIT_FAVOR_SIZE Error RAPass::annotateCode() noexcept {
  uint32_t loggerFlags = _loggerFlags;
  StringTmp<1024> sb;

  for (const RABlock* block : _blocks) {
    BaseNode* node = block->first();
    if (!node) continue;

    BaseNode* last = block->last();
    for (;;) {
      sb.clear();
      Logging::formatNode(sb, loggerFlags, cc(), node);

      if ((loggerFlags & FormatOptions::kFlagDebugRA) != 0 && node->isInst() && node->hasPassData()) {
        const RAInst* raInst = node->passData<RAInst>();
        if (raInst->tiedCount() > 0) {
          sb.padEnd(40);
          sb.appendString(" | ");
          RAPass_dumpRAInst(this, sb, raInst);
        }
      }

      node->setInlineComment(
        static_cast<char*>(
          cc()->_dataZone.dup(sb.data(), sb.size(), true)));

      if (node == last)
        break;
      node = node->next();
    }
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error RAPass::_logBlockIds(const RABlocks& blocks) noexcept {
  // Can only be called if the `Logger` is present.
  ASMJIT_ASSERT(debugLogger());

  StringTmp<1024> sb;
  sb.appendString("  [Succ] {");

  for (uint32_t i = 0, size = blocks.size(); i < size; i++) {
    const RABlock* block = blocks[i];
    if (i != 0) sb.appendString(", ");
    sb.appendFormat("#%u", block->blockId());
  }

  sb.appendString("}\n");
  return debugLogger()->log(sb.data(), sb.size());
}

ASMJIT_FAVOR_SIZE Error RAPass::_dumpBlockLiveness(String& sb, const RABlock* block) noexcept {
  for (uint32_t liveType = 0; liveType < RABlock::kLiveCount; liveType++) {
    const char* bitsName = liveType == RABlock::kLiveIn  ? "IN  " :
                           liveType == RABlock::kLiveOut ? "OUT " :
                           liveType == RABlock::kLiveGen ? "GEN " : "KILL";

    const ZoneBitVector& bits = block->_liveBits[liveType];
    uint32_t size = bits.size();
    ASMJIT_ASSERT(size <= workRegCount());

    uint32_t n = 0;
    for (uint32_t workId = 0; workId < size; workId++) {
      if (bits.bitAt(workId)) {
        RAWorkReg* wReg = workRegById(workId);

        if (!n)
          sb.appendFormat("    %s [", bitsName);
        else
          sb.appendString(", ");

        sb.appendString(wReg->name());
        n++;
      }
    }

    if (n)
      sb.appendString("]\n");
  }

  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error RAPass::_dumpLiveSpans(String& sb) noexcept {
  uint32_t numWorkRegs = _workRegs.size();
  uint32_t maxSize = _maxWorkRegNameSize;

  for (uint32_t workId = 0; workId < numWorkRegs; workId++) {
    RAWorkReg* workReg = _workRegs[workId];

    sb.appendString("  ");

    size_t oldSize = sb.size();
    sb.appendString(workReg->name());
    sb.padEnd(oldSize + maxSize);

    RALiveStats& stats = workReg->liveStats();
    sb.appendFormat(" {id:%04u width: %-4u freq: %0.4f priority=%0.4f}",
      workReg->virtId(),
      stats.width(),
      stats.freq(),
      stats.priority());
    sb.appendString(": ");

    LiveRegSpans& liveSpans = workReg->liveSpans();
    for (uint32_t x = 0; x < liveSpans.size(); x++) {
      const LiveRegSpan& liveSpan = liveSpans[x];
      if (x) sb.appendString(", ");
      sb.appendFormat("[%u:%u]", liveSpan.a, liveSpan.b);
    }

    sb.appendChar('\n');
  }

  return kErrorOk;
}
#endif

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_DISABLE_COMPILER
