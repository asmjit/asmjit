// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_COMPILER_H
#define _ASMJIT_CORE_COMPILER_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/assembler.h"
#include "../core/builder.h"
#include "../core/constpool.h"
#include "../core/func.h"
#include "../core/inst.h"
#include "../core/operand.h"
#include "../core/support.h"
#include "../core/zone.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct RATiedReg;
class RAWorkReg;

class FuncNode;
class FuncRetNode;
class FuncCallNode;

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::VirtReg]
// ============================================================================

//! Virtual register data (BaseCompiler).
class VirtReg {
public:
  ASMJIT_NONCOPYABLE(VirtReg)

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline VirtReg(uint32_t id, uint32_t signature, uint32_t virtSize, uint32_t alignment, uint32_t typeId) noexcept
    : _id(id),
      _virtSize(virtSize),
      _alignment(uint8_t(alignment)),
      _typeId(uint8_t(typeId)),
      _weight(1),
      _isFixed(false),
      _isStack(false),
      _reserved(0),
      _name(),
      _workReg(nullptr) { _info._signature = signature; }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get VirtReg id.
  inline uint32_t id() const noexcept { return _id; }

  //! Get VirtReg name.
  inline const char* name() const noexcept { return _name.data(); }
  //! Get VirtReg name size.
  inline uint32_t nameSize() const noexcept { return _name.size(); }

  //! Get register information that wraps the register signature.
  inline const RegInfo& info() const noexcept { return _info; }
  //! Get a physical register type.
  inline uint32_t type() const noexcept { return _info.type(); }
  //! Get a physical register group.
  inline uint32_t group() const noexcept { return _info.group(); }
  //! Get a physical register size.
  inline uint32_t regSize() const noexcept { return _info.size(); }
  //! Get a register signature of this virtual register.
  inline uint32_t signature() const noexcept { return _info.signature(); }

  //! Get virtual-register's size.
  inline uint32_t virtSize() const noexcept { return _virtSize; }
  //! Get virtual-register's alignment.
  inline uint32_t alignment() const noexcept { return _alignment; }

  //! Get a register's TypeId, see `Type::Id`.
  inline uint32_t typeId() const noexcept { return _typeId; }

  //! Get VirtReg's weight - register allocator can use it as explicit hint for alloc/spill decisions.
  inline uint32_t weight() const noexcept { return _weight; }
  //! Set VirtReg's weight (0 to 255) - register allocator can use it as explicit hint for alloc/spill decisions.
  inline void setWeight(uint32_t weight) noexcept { _weight = uint8_t(weight); }

  //! Get whether the `VirtReg` is always allocated to a fixed physical register (and never reallocated).
  //!
  //! NOTE: This is only used for special purposes and it's mostly internal.
  inline bool isFixed() const noexcept { return bool(_isFixed); }

  //! Get whether the `VirtReg` is only memory allocated on the stack.
  //!
  //! NOTE: It's an error if stack is accessed as a register.
  inline bool isStack() const noexcept { return bool(_isStack); }

  inline bool hasWorkReg() const noexcept { return _workReg != nullptr; }
  inline RAWorkReg* workReg() const noexcept { return _workReg; }
  inline void setWorkReg(RAWorkReg* workReg) noexcept { _workReg = workReg; }
  inline void resetWorkReg() noexcept { _workReg = nullptr; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _id;                          //!< VirtReg id.
  RegInfo _info;                         //!< VirtReg info (signature).
  uint32_t _virtSize;                    //!< VirtReg size (can be smaller than `regInfo._size`).
  uint8_t _alignment;                    //!< VirtReg alignment (for spilling).
  uint8_t _typeId;                       //!< Type-id.
  uint8_t _weight;                       //!< VirtReg weight for alloc/spill decisions.
  uint8_t _isFixed : 1;                  //!< True if this is a fixed register, never reallocated.
  uint8_t _isStack : 1;                  //!< True if the virtual register is only used as a stack (never accessed as register).
  uint8_t _reserved : 6;

  ZoneString<16> _name;                 //!< VirtReg name (user provided or automatically generated).

  // -------------------------------------------------------------------------
  // The following members are used exclusively by RAPass. They are initialized
  // when the VirtReg is created to NULL pointers and then changed during RAPass
  // execution. RAPass sets them back to NULL before it returns.
  // -------------------------------------------------------------------------

  RAWorkReg* _workReg;                   //!< Reference to `RAWorkReg`, used during register allocation.
};

// ============================================================================
// [asmjit::BaseCompiler]
// ============================================================================

//! Code emitter that uses virtual registers and performs register allocation.
//!
//! Compiler is a high-level code-generation tool that provides register
//! allocation and automatic handling of function calling conventions. It was
//! primarily designed for merging multiple parts of code into a function
//! without worrying about registers and function calling conventions.
//!
//! BaseCompiler can be used, with a minimum effort, to handle 32-bit and 64-bit
//! code at the same time.
//!
//! BaseCompiler is based on BaseBuilder and contains all the features it
//! provides. It means that the code it stores can be modified (removed, added,
//! injected) and analyzed. When the code is finalized the compiler can emit
//! the code into an Assembler to translate the abstract representation into a
//! machine code.
class ASMJIT_VIRTAPI BaseCompiler : public BaseBuilder {
public:
  ASMJIT_NONCOPYABLE(BaseCompiler)
  typedef BaseBuilder Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `BaseCompiler` instance.
  ASMJIT_API BaseCompiler() noexcept;
  //! Destroy the `BaseCompiler` instance.
  ASMJIT_API virtual ~BaseCompiler() noexcept;

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! Get the current function.
  inline FuncNode* func() const noexcept { return _func; }

  //! Create a new `FuncNode`.
  ASMJIT_API FuncNode* newFunc(const FuncSignature& sign) noexcept;
  //! Add a function `node` to the stream.
  ASMJIT_API FuncNode* addFunc(FuncNode* func);
  //! Add a new function.
  ASMJIT_API FuncNode* addFunc(const FuncSignature& sign);
  //! Emit a sentinel that marks the end of the current function.
  ASMJIT_API Error endFunc();

  // --------------------------------------------------------------------------
  // [Ret]
  // --------------------------------------------------------------------------

  //! Create a new `FuncRetNode`.
  ASMJIT_API FuncRetNode* newRet(const Operand_& o0, const Operand_& o1) noexcept;
  //! Add a new `FuncRetNode`.
  ASMJIT_API FuncRetNode* addRet(const Operand_& o0, const Operand_& o1) noexcept;

  // --------------------------------------------------------------------------
  // [Call]
  // --------------------------------------------------------------------------

  //! Create a new `FuncCallNode`.
  ASMJIT_API FuncCallNode* newCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept;
  //! Add a new `FuncCallNode`.
  ASMJIT_API FuncCallNode* addCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept;

  // --------------------------------------------------------------------------
  // [Args]
  // --------------------------------------------------------------------------

  //! Set a function argument at `argIndex` to `reg`.
  ASMJIT_API Error setArg(uint32_t argIndex, const BaseReg& reg);

  // --------------------------------------------------------------------------
  // [VirtReg / Stack]
  // --------------------------------------------------------------------------

  //! Create a new virtual register representing the given `typeId` and `signature`.
  ASMJIT_API VirtReg* newVirtReg(uint32_t typeId, uint32_t signature, const char* name) noexcept;

  ASMJIT_API Error _newReg(BaseReg& out, uint32_t typeId, const char* name = nullptr);
  ASMJIT_API Error _newReg(BaseReg& out, uint32_t typeId, const char* fmt, std::va_list ap);

  ASMJIT_API Error _newReg(BaseReg& out, const BaseReg& ref, const char* name = nullptr);
  ASMJIT_API Error _newReg(BaseReg& out, const BaseReg& ref, const char* fmt, std::va_list ap);

  ASMJIT_API Error _newStack(BaseMem& out, uint32_t size, uint32_t alignment, const char* name = nullptr);
  ASMJIT_API Error _newConst(BaseMem& out, uint32_t scope, const void* data, size_t size);

  //! Get whether the given `id` is a valid virtual register id.
  inline bool isVirtIdValid(uint32_t id) const noexcept {
    uint32_t index = Operand::unpackId(id);
    return index < _vRegArray.size();
  }
  //! Get whether the given `reg` is a valid virtual register having a valid id.
  inline bool isVirtRegValid(const BaseReg& reg) const noexcept {
    return isVirtIdValid(reg.id());
  }

  //! Get `VirtReg` associated with the given `id`.
  inline VirtReg* virtRegById(uint32_t id) const noexcept {
    uint32_t index = Operand::unpackId(id);
    ASMJIT_ASSERT(index < _vRegArray.size());
    return _vRegArray[index];
  }
  //! Get `VirtReg` associated with the given `reg`.
  inline VirtReg* virtRegByReg(const BaseReg& reg) const noexcept { return virtRegById(reg.id()); }
  //! Get `VirtReg` associated with the given `index`.
  inline VirtReg* virtRegByIndex(uint32_t index) const noexcept { return _vRegArray[index]; }

  //! Get an array of all virtual registers managed by `BaseCompiler`.
  inline const ZoneVector<VirtReg*>& virtRegs() const noexcept { return _vRegArray; }

  //! Rename variable `reg` to `name`.
  //!
  //! NOTE: Only new name will appear in the logger.
  ASMJIT_API void rename(BaseReg& reg, const char* fmt, ...);

  // TODO:
  inline void alloc(BaseReg& reg) { ASMJIT_UNUSED(reg); }
  inline void spill(BaseReg& reg) { ASMJIT_UNUSED(reg); }

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FuncNode* _func;                       //!< Current function.

  Zone _vRegZone;                        //!< Allocates `VirtReg` objects.
  ZoneVector<VirtReg*> _vRegArray;       //!< Stores array of `VirtReg` pointers.

  ConstPoolNode* _localConstPool;        //!< Local constant pool, flushed at the end of each function.
  ConstPoolNode* _globalConstPool;       //!< Global constant pool, flushed by `finalize()`.
};

// ============================================================================
// [asmjit::FuncNode]
// ============================================================================

//! Function entry (BaseCompiler).
class FuncNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(FuncNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `FuncNode` instance.
  //!
  //! Always use `BaseCompiler::addFunc()` to create `FuncNode`.
  ASMJIT_INLINE FuncNode(BaseBuilder* cb) noexcept
    : LabelNode(cb),
      _funcDetail(),
      _frame(),
      _exitNode(nullptr),
      _end(nullptr),
      _args(nullptr) {
    setType(kNodeFunc);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get function exit `LabelNode`.
  inline LabelNode* exitNode() const noexcept { return _exitNode; }
  //! Get function exit label.
  inline Label exitLabel() const noexcept { return _exitNode->label(); }

  //! Get "End of Func" sentinel.
  inline SentinelNode* endNode() const noexcept { return _end; }

  //! Get function declaration.
  inline FuncDetail& detail() noexcept { return _funcDetail; }
  //! Get function declaration.
  inline const FuncDetail& detail() const noexcept { return _funcDetail; }

  //! Get function frame.
  inline FuncFrame& frame() noexcept { return _frame; }
  //! Get function frame.
  inline const FuncFrame& frame() const noexcept { return _frame; }

  //! Get arguments count.
  inline uint32_t argCount() const noexcept { return _funcDetail.argCount(); }
  //! Get returns count.
  inline uint32_t retCount() const noexcept { return _funcDetail.retCount(); }

  //! Get arguments list.
  inline VirtReg** args() const noexcept { return _args; }

  //! Get argument at `i`.
  inline VirtReg* arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < argCount());
    return _args[i];
  }

  //! Set argument at `i`.
  inline void setArg(uint32_t i, VirtReg* vReg) noexcept {
    ASMJIT_ASSERT(i < argCount());
    _args[i] = vReg;
  }

  //! Reset argument at `i`.
  inline void resetArg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < argCount());
    _args[i] = nullptr;
  }

  inline uint32_t attributes() const noexcept { return _frame.attributes(); }
  inline void addAttributes(uint32_t attrs) noexcept { _frame.addAttributes(attrs); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FuncDetail _funcDetail;                //!< Function detail.
  FuncFrame _frame;                      //!< Function frame.

  LabelNode* _exitNode;                  //!< Function exit (label).
  SentinelNode* _end;                    //!< Function end (sentinel).

  VirtReg** _args;                       //!< Arguments array as `VirtReg`.
};

// ============================================================================
// [asmjit::FuncRetNode]
// ============================================================================

//! Function return (BaseCompiler).
class FuncRetNode : public InstNode {
public:
  ASMJIT_NONCOPYABLE(FuncRetNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `FuncRetNode` instance.
  inline FuncRetNode(BaseBuilder* cb) noexcept : InstNode(cb, BaseInst::kIdAbstract, 0, 0) {
    _any._nodeType = kNodeFuncRet;
  }
};

// ============================================================================
// [asmjit::FuncCallNode]
// ============================================================================

//! Function call (BaseCompiler).
class FuncCallNode : public InstNode {
public:
  ASMJIT_NONCOPYABLE(FuncCallNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `FuncCallNode` instance.
  inline FuncCallNode(BaseBuilder* cb, uint32_t instId, uint32_t options) noexcept
    : InstNode(cb, instId, options, kBaseOpCapacity),
      _funcDetail(),
      _args(nullptr) {
    setType(kNodeFuncCall);
    _resetOps();
    _rets[0].reset();
    _rets[1].reset();
    addFlags(kFlagIsRemovable);
  }

  // --------------------------------------------------------------------------
  // [Signature]
  // --------------------------------------------------------------------------

  //! Set function signature.
  inline Error setSignature(const FuncSignature& sign) noexcept {
    return _funcDetail.init(sign);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get function declaration.
  inline FuncDetail& detail() noexcept { return _funcDetail; }
  //! Get function declaration.
  inline const FuncDetail& detail() const noexcept { return _funcDetail; }

  //! Get target operand.
  inline Operand& target() noexcept { return _opArray[0].as<Operand>(); }
  //! \overload
  inline const Operand& target() const noexcept { return _opArray[0].as<Operand>(); }

  //! Get number of function arguments.
  inline uint32_t argCount() const noexcept { return _funcDetail.argCount(); }
  //! Get number of function returns.
  inline uint32_t retCount() const noexcept { return _funcDetail.retCount(); }

  //! Get return at `i`.
  inline Operand& ret(uint32_t i = 0) noexcept {
    ASMJIT_ASSERT(i < 2);
    return _rets[i].as<Operand>();
  }
  //! \overload
  inline const Operand& ret(uint32_t i = 0) const noexcept {
    ASMJIT_ASSERT(i < 2);
    return _rets[i].as<Operand>();
  }

  //! Get argument at `i`.
  inline Operand& arg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i].as<Operand>();
  }
  //! \overload
  inline const Operand& arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i].as<Operand>();
  }

  //! Set argument at `i` to `op`.
  ASMJIT_API bool _setArg(uint32_t i, const Operand_& op) noexcept;
  //! Set return at `i` to `op`.
  ASMJIT_API bool _setRet(uint32_t i, const Operand_& op) noexcept;

  //! Set argument at `i` to `reg`.
  inline bool setArg(uint32_t i, const BaseReg& reg) noexcept { return _setArg(i, reg); }
  //! Set argument at `i` to `imm`.
  inline bool setArg(uint32_t i, const Imm& imm) noexcept { return _setArg(i, imm); }

  //! Set return at `i` to `var`.
  inline bool setRet(uint32_t i, const BaseReg& reg) noexcept { return _setRet(i, reg); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FuncDetail _funcDetail;                //!< Function detail.
  Operand_ _rets[2];                     //!< Returns.
  Operand_* _args;                       //!< Arguments.
};

// ============================================================================
// [asmjit::FuncPass]
// ============================================================================

class ASMJIT_VIRTAPI FuncPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(FuncPass)
  typedef Pass Base;

  ASMJIT_API FuncPass(const char* name) noexcept;

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  //! Calls `runOnFunction()` on each `FuncNode` node found.
  ASMJIT_API Error run(Zone* zone, Logger* logger) noexcept override;

  //! Called once per `FuncNode`.
  virtual Error runOnFunction(Zone* zone, Logger* logger, FuncNode* func) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the associated `BaseCompiler`.
  inline BaseCompiler* cc() const noexcept { return static_cast<BaseCompiler*>(_cb); }
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_CORE_COMPILER_H
