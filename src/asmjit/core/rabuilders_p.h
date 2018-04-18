// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_RABUILDERS_P_H
#define _ASMJIT_CORE_RABUILDERS_P_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/rapass_p.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_ra
//! \{

// ============================================================================
// [asmjit::RACFGBuilder]
// ============================================================================

template<typename This>
class RACFGBuilder {
public:
  inline RACFGBuilder(RAPass* pass) noexcept
    : _pass(pass),
      _cc(pass->cc()),
      _curBlock(nullptr),
      _retBlock(nullptr) {}

  inline BaseCompiler* cc() const noexcept { return _cc; }

  Error run() noexcept {
    ASMJIT_RA_LOG_INIT(
      Logger* logger = _pass->debugLogger();
      uint32_t flags = FormatOptions::kFlagPositions;
      RABlock* lastPrintedBlock = nullptr;
      StringBuilderTmp<512> sb;
    );
    ASMJIT_RA_LOG_FORMAT("[RAPass::BuildCFG]\n");

    FuncNode* func = _pass->func();
    BaseNode* node = nullptr;

    // Create entry and exit blocks.
    _retBlock = _pass->newBlockOrExistingAt(func->exitNode(), &node);
    if (ASMJIT_UNLIKELY(!_retBlock))
      return DebugUtils::errored(kErrorNoHeapMemory);
    ASMJIT_PROPAGATE(_pass->addExitBlock(_retBlock));

    if (node != func) {
      _curBlock = _pass->newBlock();
      if (ASMJIT_UNLIKELY(!_curBlock))
        return DebugUtils::errored(kErrorNoHeapMemory);
    }
    else {
      // Function that has no code at all.
      _curBlock = _retBlock;
    }

    ASMJIT_PROPAGATE(_pass->addBlock(_curBlock));

    RARegsStats blockRegStats;
    blockRegStats.reset();
    RAInstBuilder ib;

    bool hasCode = false;
    uint32_t exitLabelId = func->exitNode()->id();

    ASMJIT_RA_LOG_COMPLEX({
      flags |= logger->flags();

      Logging::formatNode(sb, flags, cc(), func);
      logger->logf("  %s\n", sb.data());

      lastPrintedBlock = _curBlock;
      logger->logf("  {#%u}\n", lastPrintedBlock->blockId());
    });

    node = func->next();
    if (ASMJIT_UNLIKELY(!node))
      return DebugUtils::errored(kErrorInvalidState);

    _curBlock->setFirst(node);
    _curBlock->setLast(node);

    for (;;) {
      BaseNode* next = node->next();
      ASMJIT_ASSERT(!node->hasPosition());

      if (node->isInst()) {
        if (ASMJIT_UNLIKELY(!_curBlock)) {
          // If this code is unreachable then it has to be removed.
          ASMJIT_RA_LOG_COMPLEX({
            sb.clear();
            Logging::formatNode(sb, flags, cc(), node);
            logger->logf("  <Removed> %s\n", sb.data());
          });
          cc()->removeNode(node);
          node = next;
          continue;
        }
        else {
          // Handle `InstNode`, `FuncCallNode`, and `FuncRetNode`. All of
          // these share the `InstNode` interface and contain operands.
          hasCode = true;

          ASMJIT_RA_LOG_COMPLEX({
            sb.clear();
            Logging::formatNode(sb, flags, cc(), node);
            logger->logf("    %s\n", sb.data());
          });

          InstNode* inst = node->as<InstNode>();
          uint32_t controlType = BaseInst::kControlNone;

          ib.reset();
          ASMJIT_PROPAGATE(static_cast<This*>(this)->onInst(inst, controlType, ib));

          uint32_t nodeType = inst->type();
          if (nodeType != BaseNode::kNodeInst) {
            if (nodeType == BaseNode::kNodeFuncCall) {
              ASMJIT_PROPAGATE(static_cast<This*>(this)->onCall(inst->as<FuncCallNode>(), ib));
            }
            else if (nodeType == BaseNode::kNodeFuncRet) {
              ASMJIT_PROPAGATE(static_cast<This*>(this)->onRet(inst->as<FuncRetNode>(), ib));
              controlType = BaseInst::kControlReturn;
            }
            else {
              return DebugUtils::errored(kErrorInvalidInstruction);
            }
          }

          ASMJIT_PROPAGATE(_pass->assignRAInst(inst, _curBlock, ib));
          blockRegStats.combineWith(ib._stats);

          if (controlType != BaseInst::kControlNone) {
            // Support for conditional and unconditional jumps.
            if (controlType == BaseInst::kControlJump || controlType == BaseInst::kControlBranch) {
              // Jmp/Jcc/Call/Loop/etc...
              uint32_t opCount = inst->opCount();
              const Operand* opArray = inst->operands();

              // The last operand must be label (this supports also instructions
              // like jecx in explicit form).
              if (ASMJIT_UNLIKELY(opCount == 0 || !opArray[opCount - 1].isLabel()))
                return DebugUtils::errored(kErrorInvalidState);

              LabelNode* cbLabel;
              ASMJIT_PROPAGATE(cc()->labelNodeOf(&cbLabel, opArray[opCount - 1].as<Label>()));

              RABlock* targetBlock = _pass->newBlockOrExistingAt(cbLabel);
              if (ASMJIT_UNLIKELY(!targetBlock))
                return DebugUtils::errored(kErrorNoHeapMemory);

              _curBlock->setLast(node);
              _curBlock->addFlags(RABlock::kFlagHasTerminator);
              _curBlock->makeConstructed(blockRegStats);
              ASMJIT_PROPAGATE(_curBlock->appendSuccessor(targetBlock));

              if (controlType == BaseInst::kControlJump) {
                // Unconditional jump makes the code after the jump unreachable,
                // which will be removed instantly during the CFG construction;
                // as we cannot allocate registers for instructions that are not
                // part of any block. Of course we can leave these instructions
                // as they are, however, that would only postpone the problem as
                // assemblers can't encode instructions that use virtual registers.
                _curBlock = nullptr;
              }
              else {
                node = next;
                if (ASMJIT_UNLIKELY(!node))
                  return DebugUtils::errored(kErrorInvalidState);

                RABlock* consecutiveBlock;
                if (node->type() == BaseNode::kNodeLabel) {
                  if (node->hasPassData()) {
                    consecutiveBlock = node->passData<RABlock>();
                  }
                  else {
                    consecutiveBlock = _pass->newBlock(node);
                    if (ASMJIT_UNLIKELY(!consecutiveBlock))
                      return DebugUtils::errored(kErrorNoHeapMemory);
                    node->setPassData<RABlock>(consecutiveBlock);
                  }
                }
                else {
                  consecutiveBlock = _pass->newBlock(node);
                  if (ASMJIT_UNLIKELY(!consecutiveBlock))
                    return DebugUtils::errored(kErrorNoHeapMemory);
                }

                _curBlock->addFlags(RABlock::kFlagHasConsecutive);
                ASMJIT_PROPAGATE(_curBlock->prependSuccessor(consecutiveBlock));

                _curBlock = consecutiveBlock;
                hasCode = false;
                blockRegStats.reset();

                if (_curBlock->isConstructed())
                  break;
                ASMJIT_PROPAGATE(_pass->addBlock(consecutiveBlock));

                ASMJIT_RA_LOG_COMPLEX({
                  lastPrintedBlock = _curBlock;
                  logger->logf("  {#%u}\n", lastPrintedBlock->blockId());
                });

                continue;
              }
            }

            if (controlType == BaseInst::kControlReturn) {
              _curBlock->setLast(node);
              _curBlock->makeConstructed(blockRegStats);
              ASMJIT_PROPAGATE(_curBlock->appendSuccessor(_retBlock));

              _curBlock = nullptr;
            }
          }
        }
      }
      else if (node->type() == BaseNode::kNodeLabel) {
        if (!_curBlock) {
          // If the current code is unreachable the label makes it reachable again.
          _curBlock = node->passData<RABlock>();
          if (_curBlock) {
            // If the label has a block assigned we can either continue with
            // it or skip it if the block has been constructed already.
            if (_curBlock->isConstructed())
              break;
          }
          else {
            // No block assigned, to create a new one, and assign it.
            _curBlock = _pass->newBlock(node);
            if (ASMJIT_UNLIKELY(!_curBlock))
              return DebugUtils::errored(kErrorNoHeapMemory);
            node->setPassData<RABlock>(_curBlock);
          }

          hasCode = false;
          blockRegStats.reset();
          ASMJIT_PROPAGATE(_pass->addBlock(_curBlock));
        }
        else {
          if (node->hasPassData()) {
            RABlock* consecutive = node->passData<RABlock>();
            if (_curBlock == consecutive) {
              // The label currently processed is part of the current block. This
              // is only possible for multiple labels that are right next to each
              // other, or are separated by non-code nodes like directives and comments.
              if (ASMJIT_UNLIKELY(hasCode))
                return DebugUtils::errored(kErrorInvalidState);
            }
            else {
              // Label makes the current block constructed. There is a chance that the
              // Label is not used, but we don't know that at this point. In the worst
              // case there would be two blocks next to each other, it's just fine.
              ASMJIT_ASSERT(_curBlock->last() != node);
              _curBlock->setLast(node->prev());
              _curBlock->addFlags(RABlock::kFlagHasConsecutive);
              _curBlock->makeConstructed(blockRegStats);

              ASMJIT_PROPAGATE(_curBlock->appendSuccessor(consecutive));
              ASMJIT_PROPAGATE(_pass->addBlock(consecutive));

              _curBlock = consecutive;
              hasCode = false;
              blockRegStats.reset();
            }
          }
          else {
            // First time we see this label.
            if (hasCode) {
              // Cannot continue the current block if it already contains some
              // code. We need to create a new block and make it a successor.
              ASMJIT_ASSERT(_curBlock->last() != node);
              _curBlock->setLast(node->prev());
              _curBlock->addFlags(RABlock::kFlagHasConsecutive);
              _curBlock->makeConstructed(blockRegStats);

              RABlock* consecutive = _pass->newBlock(node);
              if (ASMJIT_UNLIKELY(!consecutive))
                return DebugUtils::errored(kErrorNoHeapMemory);

              ASMJIT_PROPAGATE(_curBlock->appendSuccessor(consecutive));
              ASMJIT_PROPAGATE(_pass->addBlock(consecutive));

              _curBlock = consecutive;
              hasCode = false;
              blockRegStats.reset();
            }

            node->setPassData<RABlock>(_curBlock);
          }
        }

        ASMJIT_RA_LOG_COMPLEX({
          if (_curBlock && _curBlock != lastPrintedBlock) {
            lastPrintedBlock = _curBlock;
            logger->logf("  {#%u}\n", lastPrintedBlock->blockId());
          }

          sb.clear();
          Logging::formatNode(sb, flags, cc(), node);
          logger->logf("  %s\n", sb.data());
        });

        // Unlikely: Assume that the exit label is reached only once per function.
        if (ASMJIT_UNLIKELY(node->as<LabelNode>()->id() == exitLabelId)) {
          _curBlock->setLast(node);
          _curBlock->makeConstructed(blockRegStats);
          ASMJIT_PROPAGATE(_pass->addExitBlock(_curBlock));

          _curBlock = nullptr;
        }
      }
      else {
        ASMJIT_RA_LOG_COMPLEX({
          sb.clear();
          Logging::formatNode(sb, flags, cc(), node);
          logger->logf("    %s\n", sb.data());
        });

        if (node->type() == BaseNode::kNodeSentinel) {
          if (node == func->endNode()) {
            // Make sure we didn't flow here if this is the end of the function sentinel.
            if (ASMJIT_UNLIKELY(_curBlock))
              return DebugUtils::errored(kErrorInvalidState);
            break;
          }
        }
        else if (node->type() == BaseNode::kNodeFunc) {
          // RAPass can only compile a single function at a time. If we
          // encountered a function it must be the current one, bail if not.
          if (ASMJIT_UNLIKELY(node != func))
            return DebugUtils::errored(kErrorInvalidState);
          // PASS if this is the first node.
        }
        else {
          // PASS if this is a non-interesting or unknown node.
        }
      }

      // Advance to the next node.
      node = next;

      // NOTE: We cannot encounter a NULL node, because every function must be
      // terminated by a sentinel (`stop`) node. If we encountered a NULL node it
      // means that something went wrong and this node list is corrupted; bail in
      // such case.
      if (ASMJIT_UNLIKELY(!node))
        return DebugUtils::errored(kErrorInvalidState);
    }

    if (_pass->hasDanglingBlocks())
      return DebugUtils::errored(kErrorInvalidState);

    return kErrorOk;
  }

  RAPass* _pass;
  BaseCompiler* _cc;
  RABlock* _curBlock;
  RABlock* _retBlock;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_CORE_RABUILDERS_P_H
