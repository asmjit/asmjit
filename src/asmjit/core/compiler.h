// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_COMPILER_H
#define _ASMJIT_CORE_COMPILER_H

#include "../core/build.h"
#ifndef ASMJIT_NO_COMPILER

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

//! \addtogroup asmjit_compiler
//! \{

// ============================================================================
// [asmjit::VirtReg]
// ============================================================================

//! Virtual register data (BaseCompiler).
class VirtReg {
public:
  ASMJIT_NONCOPYABLE(VirtReg)

  //! Virtual register id.
  uint32_t _id;
  //! Virtual register info (signature).
  RegInfo _info;
  //! Virtual register size (can be smaller than `regInfo._size`).
  uint32_t _virtSize;
  //! Virtual register alignment (for spilling).
  uint8_t _alignment;
  //! Type-id.
  uint8_t _typeId;
  //! Virtual register weight for alloc/spill decisions.
  uint8_t _weight;
  //! True if this is a fixed register, never reallocated.
  uint8_t _isFixed : 1;
  //! True if the virtual register is only used as a stack (never accessed as register).
  uint8_t _isStack : 1;
  uint8_t _reserved : 6;

  //! Virtual register name (user provided or automatically generated).
  ZoneString<16> _name;

  // -------------------------------------------------------------------------
  // The following members are used exclusively by RAPass. They are initialized
  // when the VirtReg is created to NULL pointers and then changed during RAPass
  // execution. RAPass sets them back to NULL before it returns.
  // -------------------------------------------------------------------------

  //! Reference to `RAWorkReg`, used during register allocation.
  RAWorkReg* _workReg;

  //! \name Construction & Destruction
  //! \{

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

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the virtual register id.
  inline uint32_t id() const noexcept { return _id; }

  //! Returns the virtual register name.
  inline const char* name() const noexcept { return _name.data(); }
  //! Returns the size of the virtual register name.
  inline uint32_t nameSize() const noexcept { return _name.size(); }

  //! Returns a register information that wraps the register signature.
  inline const RegInfo& info() const noexcept { return _info; }
  //! Returns a virtual register type (maps to the physical register type as well).
  inline uint32_t type() const noexcept { return _info.type(); }
  //! Returns a virtual register group (maps to the physical register group as well).
  inline uint32_t group() const noexcept { return _info.group(); }

  //! Returns a real size of the register this virtual register maps to.
  //!
  //! For example if this is a 128-bit SIMD register used for a scalar single
  //! precision floating point value then its virtSize would be 4, however, the
  //! `regSize` would still say 16 (128-bits), because it's the smallest size
  //! of that register type.
  inline uint32_t regSize() const noexcept { return _info.size(); }

  //! Returns a register signature of this virtual register.
  inline uint32_t signature() const noexcept { return _info.signature(); }

  //! Returns the virtual register size.
  //!
  //! The virtual register size describes how many bytes the virtual register
  //! needs to store its content. It can be smaller than the physical register
  //! size, see `regSize()`.
  inline uint32_t virtSize() const noexcept { return _virtSize; }

  //! Returns the virtual register alignment.
  inline uint32_t alignment() const noexcept { return _alignment; }

  //! Returns the virtual register type id, see `Type::Id`.
  inline uint32_t typeId() const noexcept { return _typeId; }

  //! Returns the virtual register weight - the register allocator can use it
  //! as explicit hint for alloc/spill decisions.
  inline uint32_t weight() const noexcept { return _weight; }
  //! Sets the virtual register weight (0 to 255) - the register allocator can
  //! use it as explicit hint for alloc/spill decisions and initial bin-packing.
  inline void setWeight(uint32_t weight) noexcept { _weight = uint8_t(weight); }

  //! Returns whether the virtual register is always allocated to a fixed
  //! physical register (and never reallocated).
  //!
  //! \note This is only used for special purposes and it's mostly internal.
  inline bool isFixed() const noexcept { return bool(_isFixed); }

  //! Returns whether the virtual register is indeed a stack that only uses
  //! the virtual register id for making it accessible.
  //!
  //! \note It's an error if a stack is accessed as a register.
  inline bool isStack() const noexcept { return bool(_isStack); }

  inline bool hasWorkReg() const noexcept { return _workReg != nullptr; }
  inline RAWorkReg* workReg() const noexcept { return _workReg; }
  inline void setWorkReg(RAWorkReg* workReg) noexcept { _workReg = workReg; }
  inline void resetWorkReg() noexcept { _workReg = nullptr; }

  //! \}
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

  //! Current function.
  FuncNode* _func;
  //! Allocates `VirtReg` objects.
  Zone _vRegZone;
  //! Stores array of `VirtReg` pointers.
  ZoneVector<VirtReg*> _vRegArray;

  //! Local constant pool, flushed at the end of each function.
  ConstPoolNode* _localConstPool;
  //! Global constant pool, flushed by `finalize()`.
  ConstPoolNode* _globalConstPool;

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `BaseCompiler` instance.
  ASMJIT_API BaseCompiler() noexcept;
  //! Destroys the `BaseCompiler` instance.
  ASMJIT_API virtual ~BaseCompiler() noexcept;

  //! \}

  //! \name Function API
  //! \{

  //! Returns the current function.
  inline FuncNode* func() const noexcept { return _func; }

  //! Creates a new `FuncNode`.
  ASMJIT_API FuncNode* newFunc(const FuncSignature& sign) noexcept;
  //! Adds a function `node` to the stream.
  ASMJIT_API FuncNode* addFunc(FuncNode* func);
  //! Adds a new function.
  ASMJIT_API FuncNode* addFunc(const FuncSignature& sign);
  //! Emits a sentinel that marks the end of the current function.
  ASMJIT_API Error endFunc();

  //! Sets a function argument at `argIndex` to `reg`.
  ASMJIT_API Error setArg(uint32_t argIndex, const BaseReg& reg);

  //! Creates a new `FuncRetNode`.
  ASMJIT_API FuncRetNode* newRet(const Operand_& o0, const Operand_& o1) noexcept;
  //! Adds a new `FuncRetNode`.
  ASMJIT_API FuncRetNode* addRet(const Operand_& o0, const Operand_& o1) noexcept;

  //! \}

  //! \name Function Calls
  //! \{

  //! Creates a new `FuncCallNode`.
  ASMJIT_API FuncCallNode* newCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept;
  //! Adds a new `FuncCallNode`.
  ASMJIT_API FuncCallNode* addCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept;

  //! \}

  //! \name Virtual Registers
  //! \{

  //! Creates a new virtual register representing the given `typeId` and `signature`.
  ASMJIT_API VirtReg* newVirtReg(uint32_t typeId, uint32_t signature, const char* name) noexcept;

  ASMJIT_API Error _newReg(BaseReg& out, uint32_t typeId, const char* name = nullptr);
  ASMJIT_API Error _newReg(BaseReg& out, uint32_t typeId, const char* fmt, va_list ap);

  ASMJIT_API Error _newReg(BaseReg& out, const BaseReg& ref, const char* name = nullptr);
  ASMJIT_API Error _newReg(BaseReg& out, const BaseReg& ref, const char* fmt, va_list ap);

  //! Tests whether the given `id` is a valid virtual register id.
  inline bool isVirtIdValid(uint32_t id) const noexcept {
    uint32_t index = Operand::virtIdToIndex(id);
    return index < _vRegArray.size();
  }
  //! Tests whether the given `reg` is a virtual register having a valid id.
  inline bool isVirtRegValid(const BaseReg& reg) const noexcept {
    return isVirtIdValid(reg.id());
  }

  //! Returns `VirtReg` associated with the given `id`.
  inline VirtReg* virtRegById(uint32_t id) const noexcept {
    ASMJIT_ASSERT(isVirtIdValid(id));
    return _vRegArray[Operand::virtIdToIndex(id)];
  }
  //! Returns `VirtReg` associated with the given `reg`.
  inline VirtReg* virtRegByReg(const BaseReg& reg) const noexcept { return virtRegById(reg.id()); }
  //! Returns `VirtReg` associated with the given `index`.
  inline VirtReg* virtRegByIndex(uint32_t index) const noexcept { return _vRegArray[index]; }

  //! Returns an array of all virtual registers managed by the Compiler.
  inline const ZoneVector<VirtReg*>& virtRegs() const noexcept { return _vRegArray; }

  //! \name Stack
  //! \{

  ASMJIT_API Error _newStack(BaseMem& out, uint32_t size, uint32_t alignment, const char* name = nullptr);

  //! \}

  //! \name Constants
  //! \{

  ASMJIT_API Error _newConst(BaseMem& out, uint32_t scope, const void* data, size_t size);

  //! \}

  //! \name Miscellaneous
  //! \{

  //! Rename the given virtual register `reg` to a formatted string `fmt`.
  //!
  //! \note Only new name will appear in the logger.
  ASMJIT_API void rename(BaseReg& reg, const char* fmt, ...);

  //! \}

  // TODO: These should be removed
  inline void alloc(BaseReg& reg) { ASMJIT_UNUSED(reg); }
  inline void spill(BaseReg& reg) { ASMJIT_UNUSED(reg); }

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  //! \}
};

// ============================================================================
// [asmjit::FuncNode]
// ============================================================================

//! Function entry (BaseCompiler).
class FuncNode : public LabelNode {
public:
  ASMJIT_NONCOPYABLE(FuncNode)

  //! Function detail.
  FuncDetail _funcDetail;
  //! Function frame.
  FuncFrame _frame;
  //! Function exit (label).
  LabelNode* _exitNode;
  //! Function end (sentinel).
  SentinelNode* _end;
  //! Arguments array as `VirtReg`.
  VirtReg** _args;

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FuncNode` instance.
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

  //! \}

  //! \{
  //! \name Accessors

  //! Returns function exit `LabelNode`.
  inline LabelNode* exitNode() const noexcept { return _exitNode; }
  //! Returns function exit label.
  inline Label exitLabel() const noexcept { return _exitNode->label(); }

  //! Returns "End of Func" sentinel.
  inline SentinelNode* endNode() const noexcept { return _end; }

  //! Returns function declaration.
  inline FuncDetail& detail() noexcept { return _funcDetail; }
  //! Returns function declaration.
  inline const FuncDetail& detail() const noexcept { return _funcDetail; }

  //! Returns function frame.
  inline FuncFrame& frame() noexcept { return _frame; }
  //! Returns function frame.
  inline const FuncFrame& frame() const noexcept { return _frame; }

  //! Returns arguments count.
  inline uint32_t argCount() const noexcept { return _funcDetail.argCount(); }
  //! Returns returns count.
  inline uint32_t retCount() const noexcept { return _funcDetail.retCount(); }

  //! Returns arguments list.
  inline VirtReg** args() const noexcept { return _args; }

  //! Returns argument at `i`.
  inline VirtReg* arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < argCount());
    return _args[i];
  }

  //! Sets argument at `i`.
  inline void setArg(uint32_t i, VirtReg* vReg) noexcept {
    ASMJIT_ASSERT(i < argCount());
    _args[i] = vReg;
  }

  //! Resets argument at `i`.
  inline void resetArg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < argCount());
    _args[i] = nullptr;
  }

  inline uint32_t attributes() const noexcept { return _frame.attributes(); }
  inline void addAttributes(uint32_t attrs) noexcept { _frame.addAttributes(attrs); }

  //! \}
};

// ============================================================================
// [asmjit::FuncRetNode]
// ============================================================================

//! Function return (BaseCompiler).
class FuncRetNode : public InstNode {
public:
  ASMJIT_NONCOPYABLE(FuncRetNode)

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FuncRetNode` instance.
  inline FuncRetNode(BaseBuilder* cb) noexcept : InstNode(cb, BaseInst::kIdAbstract, 0, 0) {
    _any._nodeType = kNodeFuncRet;
  }

  //! \}
};

// ============================================================================
// [asmjit::FuncCallNode]
// ============================================================================

//! Function call (BaseCompiler).
class FuncCallNode : public InstNode {
public:
  ASMJIT_NONCOPYABLE(FuncCallNode)

  //! Function detail.
  FuncDetail _funcDetail;
  //! Returns.
  Operand_ _rets[2];
  //! Arguments.
  Operand_* _args;

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FuncCallNode` instance.
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

  //! \}

  //! \name Accessors
  //! \{

  //! Sets the function signature.
  inline Error setSignature(const FuncSignature& sign) noexcept {
    return _funcDetail.init(sign);
  }

  //! Returns the function detail.
  inline FuncDetail& detail() noexcept { return _funcDetail; }
  //! Returns the function detail.
  inline const FuncDetail& detail() const noexcept { return _funcDetail; }

  //! Returns the target operand.
  inline Operand& target() noexcept { return _opArray[0].as<Operand>(); }
  //! \overload
  inline const Operand& target() const noexcept { return _opArray[0].as<Operand>(); }

  //! Returns the number of function arguments.
  inline uint32_t argCount() const noexcept { return _funcDetail.argCount(); }
  //! Returns the number of function return values.
  inline uint32_t retCount() const noexcept { return _funcDetail.retCount(); }

  //! Returns the return value at `i`.
  inline Operand& ret(uint32_t i = 0) noexcept {
    ASMJIT_ASSERT(i < 2);
    return _rets[i].as<Operand>();
  }
  //! \overload
  inline const Operand& ret(uint32_t i = 0) const noexcept {
    ASMJIT_ASSERT(i < 2);
    return _rets[i].as<Operand>();
  }

  //! Returns the function argument at `i`.
  inline Operand& arg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i].as<Operand>();
  }
  //! \overload
  inline const Operand& arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i].as<Operand>();
  }

  //! Sets the function argument at `i` to `op`.
  ASMJIT_API bool _setArg(uint32_t i, const Operand_& op) noexcept;
  //! Sets the function return value at `i` to `op`.
  ASMJIT_API bool _setRet(uint32_t i, const Operand_& op) noexcept;

  //! Sets the function argument at `i` to `reg`.
  inline bool setArg(uint32_t i, const BaseReg& reg) noexcept { return _setArg(i, reg); }
  //! Sets the function argument at `i` to `imm`.
  inline bool setArg(uint32_t i, const Imm& imm) noexcept { return _setArg(i, imm); }

  //! Sets the function return value at `i` to `var`.
  inline bool setRet(uint32_t i, const BaseReg& reg) noexcept { return _setRet(i, reg); }

  //! \}
};

// ============================================================================
// [asmjit::FuncPass]
// ============================================================================

class ASMJIT_VIRTAPI FuncPass : public Pass {
public:
  ASMJIT_NONCOPYABLE(FuncPass)
  typedef Pass Base;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API FuncPass(const char* name) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the associated `BaseCompiler`.
  inline BaseCompiler* cc() const noexcept { return static_cast<BaseCompiler*>(_cb); }

  //! \}

  //! \name Run
  //! \{

  //! Calls `runOnFunction()` on each `FuncNode` node found.
  ASMJIT_API Error run(Zone* zone, Logger* logger) noexcept override;

  //! Called once per `FuncNode`.
  virtual Error runOnFunction(Zone* zone, Logger* logger, FuncNode* func) noexcept = 0;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // _ASMJIT_CORE_COMPILER_H
