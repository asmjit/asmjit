// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CONTEXT_P_H
#define _ASMJIT_BASE_CONTEXT_P_H

// [Dependencies - AsmJit]
#include "../base/compiler.h"
#include "../base/zone.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::BaseContext]
// ============================================================================

struct BaseContext {
  ASMJIT_NO_COPY(BaseContext)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  BaseContext(BaseCompiler* compiler);
  virtual ~BaseContext();

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! @brief Reset the whole context.
  virtual void reset();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get compiler.
  ASMJIT_INLINE BaseCompiler* getCompiler() const { return _compiler; }

  //! @brief Get function.
  ASMJIT_INLINE FuncNode* getFunc() const { return _func; }
  //! @brief Get stop node.
  ASMJIT_INLINE BaseNode* getStop() const { return _stop; }

  //! @brief Get start of the current scope.
  ASMJIT_INLINE BaseNode* getStart() const { return _start; }
  //! @brief Get end of the current scope.
  ASMJIT_INLINE BaseNode* getEnd() const { return _end; }

  //! @brief Get extra block.
  ASMJIT_INLINE BaseNode* getExtraBlock() const { return _extraBlock; }
  //! @brief Set extra block.
  ASMJIT_INLINE void setExtraBlock(BaseNode* node) { _extraBlock = node; }

  // --------------------------------------------------------------------------
  // [Error]
  // --------------------------------------------------------------------------

  //! @brief Get the last error code.
  ASMJIT_INLINE Error getError() const {
    return getCompiler()->getError();
  }

  //! @brief Set the last error code and propagate it through the error handler.
  ASMJIT_INLINE Error setError(Error error, const char* message = NULL) {
    return getCompiler()->setError(error, message);
  }

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  //! @brief Get current state.
  ASMJIT_INLINE BaseVarState* getState() const { return _state; }

  //! @brief Load current state from @a target state.
  virtual void loadState(BaseVarState* src) = 0;
  //! @brief Save current state, returning new @ref BaseVarState instance.
  virtual BaseVarState* saveState() = 0;

  //! @brief Change the current state to @a target state.
  virtual void switchState(BaseVarState* src) = 0;

  //! @brief Change the current state to the intersection of two states @a a
  //! and @a b.
  virtual void intersectStates(BaseVarState* a, BaseVarState* b) = 0;

  // --------------------------------------------------------------------------
  // [Mem]
  // --------------------------------------------------------------------------

  MemCell* _newVarCell(VarData* vd);
  MemCell* _newStackCell(uint32_t size, uint32_t alignment);

  ASMJIT_INLINE MemCell* getVarCell(VarData* vd) {
    MemCell* cell = vd->getMemCell();
    return cell ? cell : _newVarCell(vd);
  }

  virtual Error resolveCellOffsets();

  // --------------------------------------------------------------------------
  // [Bits]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE VarBits* newBits(uint32_t len) {
    return static_cast<VarBits*>(
      _zoneAllocator.calloc(static_cast<size_t>(len) * VarBits::kEntitySize));
  }

  ASMJIT_INLINE VarBits* copyBits(const VarBits* src, uint32_t len) {
    return static_cast<VarBits*>(
      _zoneAllocator.dup(src, static_cast<size_t>(len) * VarBits::kEntitySize));
  }

  // --------------------------------------------------------------------------
  // [Fetch]
  // --------------------------------------------------------------------------

  //! @brief Fetch.
  //!
  //! Fetch iterates over all nodes and gathers information about all variables
  //! used. The process generates information required by register allocator,
  //! variable liveness analysis and translator.
  virtual Error fetch() = 0;

  // --------------------------------------------------------------------------
  // [RemoveUnreachableCode]
  // --------------------------------------------------------------------------

  //! @brief Remove unreachable code.
  virtual Error removeUnreachableCode();

  // --------------------------------------------------------------------------
  // [Analyze]
  // --------------------------------------------------------------------------

  //! @brief Preform variable liveness analysis.
  //!
  //! Analysis phase iterates over nodes in reverse order and generates a bit
  //! array describing variables that are alive at every node in the function.
  //! When the analysis start all variables are assumed dead. When a read or
  //! read/write operations of a variable is detected the variable becomes
  //! alive; when only write operation is detected the variable becomes dead.
  //!
  //! When a label is found all jumps to that label are followed and analysis
  //! repeats until all variables are resolved.
  virtual Error analyze() = 0;

  // --------------------------------------------------------------------------
  // [Translate]
  // --------------------------------------------------------------------------

  //! @brief Translate code by allocating registers and handling state changes.
  virtual Error translate() = 0;

  // --------------------------------------------------------------------------
  // [Cleanup]
  // --------------------------------------------------------------------------

  virtual void cleanup();

  // --------------------------------------------------------------------------
  // [Compile]
  // --------------------------------------------------------------------------

  virtual Error compile(FuncNode* func);

  // --------------------------------------------------------------------------
  // [Serialize]
  // --------------------------------------------------------------------------

  virtual Error serialize(BaseAssembler* assembler, BaseNode* start, BaseNode* stop) = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Compiler.
  BaseCompiler* _compiler;
  //! @brief Function.
  FuncNode* _func;

  //! @brief Zone allocator.
  Zone _zoneAllocator;

  //! @brief Start of the current active scope.
  BaseNode* _start;
  //! @brief End of the current active scope.
  BaseNode* _end;

  //! @brief Node that is used to insert extra code after the function body.
  BaseNode* _extraBlock;
  //! @brief Stop node.
  BaseNode* _stop;

  //! @brief Unreachable nodes.
  PodList<BaseNode*> _unreachableList;
  //! @brief Jump nodes.
  PodList<BaseNode*> _jccList;

  //! @brief All variables used by the current function.
  PodVector<VarData*> _contextVd;

  //! @brief Memory used to spill variables.
  MemCell* _memVarCells;
  //! @brief Memory used to alloc memory on the stack.
  MemCell* _memStackCells;

  //! @brief Count of 1-byte cells.
  uint32_t _mem1ByteVarsUsed;
  //! @brief Count of 2-byte cells.
  uint32_t _mem2ByteVarsUsed;
  //! @brief Count of 4-byte cells.
  uint32_t _mem4ByteVarsUsed;
  //! @brief Count of 8-byte cells.
  uint32_t _mem8ByteVarsUsed;
  //! @brief Count of 16-byte cells.
  uint32_t _mem16ByteVarsUsed;
  //! @brief Count of 32-byte cells.
  uint32_t _mem32ByteVarsUsed;
  //! @brief Count of 64-byte cells.
  uint32_t _mem64ByteVarsUsed;
  //! @brief Count of stack memory cells.
  uint32_t _memStackCellsUsed;

  //! @brief Maximum memory alignment used by the function.
  uint32_t _memMaxAlign;
  //! @brief Count of bytes used by variables.
  uint32_t _memVarTotal;
  //! @brief Count of bytes used by stack.
  uint32_t _memStackTotal;
  //! @brief Count of bytes used by variables and stack after alignment.
  uint32_t _memAllTotal;

  //! @brief Current state (used by register allocator).
  BaseVarState* _state;
};

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CONTEXT_P_H
