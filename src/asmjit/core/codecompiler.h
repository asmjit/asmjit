// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_CODECOMPILER_H
#define _ASMJIT_CORE_CODECOMPILER_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/assembler.h"
#include "../core/codebuilder.h"
#include "../core/constpool.h"
#include "../core/func.h"
#include "../core/inst.h"
#include "../core/intutils.h"
#include "../core/operand.h"
#include "../core/zone.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct RATiedReg;
class RAWorkReg;

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::ConstScope]
// ============================================================================

// TODO: Move to some class instead, or to Globals namespace.
//! Scope of the constant.
enum ConstScope : uint32_t {
  //! Local constant, always embedded right after the current function.
  kConstScopeLocal = 0,
  //! Global constant, embedded at the end of the currently compiled code.
  kConstScopeGlobal = 1
};

// ============================================================================
// [asmjit::VirtReg]
// ============================================================================

//! Virtual register data (CodeCompiler).
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
  inline uint32_t getId() const noexcept { return _id; }

  //! Get VirtReg name string.
  inline const char* getName() const noexcept { return _name.getData(); }
  //! Get VirtReg name length.
  inline uint32_t getNameLength() const noexcept { return _name.getLength(); }

  //! Get register information that wraps the register signature.
  inline const RegInfo& getInfo() const noexcept { return _info; }
  //! Get a physical register type.
  inline uint32_t getType() const noexcept { return _info.getType(); }
  //! Get a physical register group.
  inline uint32_t getGroup() const noexcept { return _info.getGroup(); }
  //! Get a physical register size.
  inline uint32_t getRegSize() const noexcept { return _info.getSize(); }
  //! Get a register signature of this virtual register.
  inline uint32_t getSignature() const noexcept { return _info.getSignature(); }

  //! Get virtual-register's size.
  inline uint32_t getVirtSize() const noexcept { return _virtSize; }
  //! Get virtual-register's alignment.
  inline uint32_t getAlignment() const noexcept { return _alignment; }

  //! Get a register's TypeId, see \ref Type::Id.
  inline uint32_t getTypeId() const noexcept { return _typeId; }

  //! Get VirtReg's weight - register allocator can use it as explicit hint for alloc/spill decisions.
  inline uint32_t getWeight() const noexcept { return _weight; }
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
  inline RAWorkReg* getWorkReg() const noexcept { return _workReg; }
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

  SmallString<16> _name;                 //!< VirtReg name (user provided or automatically generated).

  // -------------------------------------------------------------------------
  // The following members are used exclusively by RAPass. They are initialized
  // when the VirtReg is created to NULL pointers and then changed during RAPass
  // execution. RAPass sets them back to NULL before it returns.
  // -------------------------------------------------------------------------

  RAWorkReg* _workReg;                   //!< Reference to `RAWorkReg`, used during register allocation.
};

// ============================================================================
// [asmjit::CCFunc]
// ============================================================================

//! Function entry (CodeCompiler).
class CCFunc : public CBLabel {
public:
  ASMJIT_NONCOPYABLE(CCFunc)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `CCFunc` instance.
  //!
  //! Always use `CodeCompiler::addFunc()` to create \ref CCFunc.
  ASMJIT_FORCEINLINE CCFunc(CodeBuilder* cb) noexcept
    : CBLabel(cb),
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

  //! Get function exit `CBLabel`.
  inline CBLabel* getExitNode() const noexcept { return _exitNode; }
  //! Get function exit label.
  inline Label getExitLabel() const noexcept { return _exitNode->getLabel(); }

  //! Get "End of Func" sentinel.
  inline CBSentinel* getEnd() const noexcept { return _end; }

  //! Get function declaration.
  inline FuncDetail& getDetail() noexcept { return _funcDetail; }
  //! Get function declaration.
  inline const FuncDetail& getDetail() const noexcept { return _funcDetail; }

  //! Get function frame.
  inline FuncFrame& getFrame() noexcept { return _frame; }
  //! Get function frame.
  inline const FuncFrame& getFrame() const noexcept { return _frame; }

  //! Get arguments count.
  inline uint32_t getArgCount() const noexcept { return _funcDetail.getArgCount(); }
  //! Get returns count.
  inline uint32_t getRetCount() const noexcept { return _funcDetail.getRetCount(); }

  //! Get arguments list.
  inline VirtReg** getArgs() const noexcept { return _args; }

  //! Get argument at `i`.
  inline VirtReg* getArg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < getArgCount());
    return _args[i];
  }

  //! Set argument at `i`.
  inline void setArg(uint32_t i, VirtReg* vReg) noexcept {
    ASMJIT_ASSERT(i < getArgCount());
    _args[i] = vReg;
  }

  //! Reset argument at `i`.
  inline void resetArg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < getArgCount());
    _args[i] = nullptr;
  }

  inline uint32_t getAttributes() const noexcept { return _frame.getAttributes(); }
  inline void addAttributes(uint32_t attrs) noexcept { _frame.addAttributes(attrs); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FuncDetail _funcDetail;                //!< Function detail.
  FuncFrame _frame;                      //!< Function frame.

  CBLabel* _exitNode;                    //!< Function exit.
  CBSentinel* _end;                      //!< Function end.

  VirtReg** _args;                       //!< Arguments array as `VirtReg`.
};

// ============================================================================
// [asmjit::CCFuncRet]
// ============================================================================

//! Function return (CodeCompiler).
class CCFuncRet : public CBInst {
public:
  ASMJIT_NONCOPYABLE(CCFuncRet)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `CCFuncRet` instance.
  inline CCFuncRet(CodeBuilder* cb) noexcept : CBInst(cb, Inst::kIdAbstract, 0, 0) {
    _any._nodeType = kNodeFuncRet;
  }
};

// ============================================================================
// [asmjit::CCFuncCall]
// ============================================================================

//! Function call (CodeCompiler).
class CCFuncCall : public CBInst {
public:
  ASMJIT_NONCOPYABLE(CCFuncCall)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `CCFuncCall` instance.
  inline CCFuncCall(CodeBuilder* cb, uint32_t instId, uint32_t options) noexcept
    : CBInst(cb, instId, options, kBaseOpCapacity),
      _funcDetail(),
      _args(nullptr) {
    setType(kNodeFuncCall);
    _resetOps();
    _ret[0].reset();
    _ret[1].reset();
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
  inline FuncDetail& getDetail() noexcept { return _funcDetail; }
  //! Get function declaration.
  inline const FuncDetail& getDetail() const noexcept { return _funcDetail; }

  //! Get target operand.
  inline Operand& getTarget() noexcept { return _opArray[0].as<Operand>(); }
  //! \overload
  inline const Operand& getTarget() const noexcept { return _opArray[0].as<Operand>(); }

  //! Get number of function arguments.
  inline uint32_t getArgCount() const noexcept { return _funcDetail.getArgCount(); }
  //! Get number of function returns.
  inline uint32_t getRetCount() const noexcept { return _funcDetail.getRetCount(); }

  //! Get return at `i`.
  inline Operand& getRet(uint32_t i = 0) noexcept {
    ASMJIT_ASSERT(i < 2);
    return _ret[i].as<Operand>();
  }
  //! \overload
  inline const Operand& getRet(uint32_t i = 0) const noexcept {
    ASMJIT_ASSERT(i < 2);
    return _ret[i].as<Operand>();
  }

  //! Get argument at `i`.
  inline Operand& getArg(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i].as<Operand>();
  }
  //! \overload
  inline const Operand& getArg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < kFuncArgCountLoHi);
    return _args[i].as<Operand>();
  }

  //! Set argument at `i` to `op`.
  ASMJIT_API bool _setArg(uint32_t i, const Operand_& op) noexcept;
  //! Set return at `i` to `op`.
  ASMJIT_API bool _setRet(uint32_t i, const Operand_& op) noexcept;

  //! Set argument at `i` to `reg`.
  inline bool setArg(uint32_t i, const Reg& reg) noexcept { return _setArg(i, reg); }
  //! Set argument at `i` to `imm`.
  inline bool setArg(uint32_t i, const Imm& imm) noexcept { return _setArg(i, imm); }

  //! Set return at `i` to `var`.
  inline bool setRet(uint32_t i, const Reg& reg) noexcept { return _setRet(i, reg); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FuncDetail _funcDetail;                //!< Function detail.
  Operand_ _ret[2];                      //!< Return.
  Operand_* _args;                       //!< Arguments.
};

// ============================================================================
// [asmjit::CodeCompiler]
// ============================================================================

//! Code emitter that uses virtual registers and performs register allocation.
//!
//! Compiler is a high-level code-generation tool that provides register
//! allocation and automatic handling of function calling conventions. It was
//! primarily designed for merging multiple parts of code into a function
//! without worrying about registers and function calling conventions.
//!
//! CodeCompiler can be used, with a minimum effort, to handle 32-bit and 64-bit
//! code at the same time.
//!
//! CodeCompiler is based on CodeBuilder and contains all the features it
//! provides. It means that the code it stores can be modified (removed, added,
//! injected) and analyzed. When the code is finalized the compiler can emit
//! the code into an Assembler to translate the abstract representation into a
//! machine code.
class ASMJIT_VIRTAPI CodeCompiler : public CodeBuilder {
public:
  ASMJIT_NONCOPYABLE(CodeCompiler)
  typedef CodeBuilder Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `CodeCompiler` instance.
  ASMJIT_API CodeCompiler() noexcept;
  //! Destroy the `CodeCompiler` instance.
  ASMJIT_API virtual ~CodeCompiler() noexcept;

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! Get the current function.
  inline CCFunc* getFunc() const noexcept { return _func; }

  //! Create a new `CCFunc`.
  ASMJIT_API CCFunc* newFunc(const FuncSignature& sign) noexcept;
  //! Add a function `node` to the stream.
  ASMJIT_API CCFunc* addFunc(CCFunc* func);
  //! Add a new function.
  ASMJIT_API CCFunc* addFunc(const FuncSignature& sign);
  //! Emit a sentinel that marks the end of the current function.
  ASMJIT_API Error endFunc();

  // --------------------------------------------------------------------------
  // [Ret]
  // --------------------------------------------------------------------------

  //! Create a new `CCFuncRet`.
  ASMJIT_API CCFuncRet* newRet(const Operand_& o0, const Operand_& o1) noexcept;
  //! Add a new `CCFuncRet`.
  ASMJIT_API CCFuncRet* addRet(const Operand_& o0, const Operand_& o1) noexcept;

  // --------------------------------------------------------------------------
  // [Call]
  // --------------------------------------------------------------------------

  //! Create a new `CCFuncCall`.
  ASMJIT_API CCFuncCall* newCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept;
  //! Add a new `CCFuncCall`.
  ASMJIT_API CCFuncCall* addCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept;

  // --------------------------------------------------------------------------
  // [Args]
  // --------------------------------------------------------------------------

  //! Set a function argument at `argIndex` to `reg`.
  ASMJIT_API Error setArg(uint32_t argIndex, const Reg& reg);

  // --------------------------------------------------------------------------
  // [VirtReg / Stack]
  // --------------------------------------------------------------------------

  //! Create a new virtual register representing the given `vti` and `signature`.
  //!
  //! This function accepts either register type representing a machine-specific
  //! register, like `X86Reg`, or RegTag representation, which represents
  //! machine independent register, and from the machine-specific register
  //! is deduced.
  ASMJIT_API VirtReg* newVirtReg(uint32_t typeId, uint32_t signature, const char* name) noexcept;

  ASMJIT_API Error _newReg(Reg& out, uint32_t typeId, const char* name = nullptr);
  ASMJIT_API Error _newReg(Reg& out, uint32_t typeId, const char* fmt, std::va_list ap);

  ASMJIT_API Error _newReg(Reg& out, const Reg& ref, const char* name = nullptr);
  ASMJIT_API Error _newReg(Reg& out, const Reg& ref, const char* fmt, std::va_list ap);

  ASMJIT_API Error _newStack(Mem& out, uint32_t size, uint32_t alignment, const char* name = nullptr);
  ASMJIT_API Error _newConst(Mem& out, uint32_t scope, const void* data, size_t size);

  //! Get whether the given `id` is a valid virtual register id.
  inline bool isVirtIdValid(uint32_t id) const noexcept {
    uint32_t index = Operand::unpackId(id);
    return index < _vRegArray.getLength();
  }
  //! Get whether the given `reg` is a valid virtual register having a valid id.
  inline bool isVirtRegValid(const Reg& reg) const noexcept {
    return isVirtIdValid(reg.getId());
  }

  //! Get \ref VirtReg associated with the given `reg`.
  inline VirtReg* getVirtReg(const Reg& reg) const noexcept {
    return getVirtRegById(reg.getId());
  }
  //! Get \ref VirtReg associated with the given `id`.
  inline VirtReg* getVirtRegById(uint32_t id) const noexcept {
    uint32_t index = Operand::unpackId(id);
    ASMJIT_ASSERT(index < _vRegArray.getLength());
    return _vRegArray[index];
  }

  //! Get \ref VirtReg associated with the given `index`.
  inline VirtReg* getVirtRegAt(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _vRegArray.getLength());
    return _vRegArray[index];
  }

  //! Get an array of all virtual registers managed by `CodeCompiler`.
  inline const ZoneVector<VirtReg*>& getVirtRegs() const noexcept { return _vRegArray; }

  //! Rename variable `reg` to `name`.
  //!
  //! NOTE: Only new name will appear in the logger.
  ASMJIT_API void rename(Reg& reg, const char* fmt, ...);

  // TODO:
  inline void alloc(Reg& reg) { ASMJIT_UNUSED(reg); }
  inline void spill(Reg& reg) { ASMJIT_UNUSED(reg); }

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
  ASMJIT_API Error onDetach(CodeHolder* code) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  CCFunc* _func;                         //!< Current function.

  Zone _vRegZone;                        //!< Allocates \ref VirtReg objects.
  ZoneVector<VirtReg*> _vRegArray;       //!< Stores array of \ref VirtReg pointers.

  CBConstPool* _localConstPool;          //!< Local constant pool, flushed at the end of each function.
  CBConstPool* _globalConstPool;         //!< Global constant pool, flushed by `finalize()`.
};

// ============================================================================
// [asmjit::CCFuncPass]
// ============================================================================

class ASMJIT_VIRTAPI CCFuncPass : public CBPass {
public:
  ASMJIT_NONCOPYABLE(CCFuncPass)
  typedef CBPass Base;

  ASMJIT_API CCFuncPass(const char* name) noexcept;

  // --------------------------------------------------------------------------
  // [Run]
  // --------------------------------------------------------------------------

  //! Calls `runOnFunction()` on each `CCFunc` node found.
  ASMJIT_API Error run(Zone* zone, Logger* logger) noexcept override;

  //! Called once per `CCFunc`.
  virtual Error runOnFunction(Zone* zone, Logger* logger, CCFunc* func) noexcept = 0;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the associated `CodeCompiler`.
  inline CodeCompiler* cc() const noexcept { return static_cast<CodeCompiler*>(_cb); }
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_CORE_CODECOMPILER_H
