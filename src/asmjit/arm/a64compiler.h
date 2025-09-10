// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A64COMPILER_H_INCLUDED
#define ASMJIT_ARM_A64COMPILER_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/type.h"
#include "../arm/a64emitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

//! \addtogroup asmjit_a64
//! \{

//! AArch64 compiler implementation.
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  using Base = BaseCompiler;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API ~Compiler() noexcept override;

  //! \}

  //! \name Virtual Registers
  //! \{

  //! \cond INTERNAL
  template<typename RegT, typename Type>
  ASMJIT_INLINE_NODEBUG RegT _new_reg_internal(const Type& type) {
    RegT reg(Globals::NoInit);
    _new_reg(Out<Reg>{reg}, type, nullptr);
    return reg;
  }

  template<typename RegT, typename Type>
  ASMJIT_INLINE_NODEBUG RegT _new_reg_internal(const Type& type, const char* s) {
#ifndef ASMJIT_NO_LOGGING
    RegT reg(Globals::NoInit);
    _new_reg(Out<Reg>{reg}, type, s);
    return reg;
#else
    Support::maybe_unused(s);
    return _new_reg_internal<RegT>(type);
#endif
  }

  template<typename RegT, typename Type, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT _new_reg_internal(const Type& type, const char* s, Args&&... args) {
#ifndef ASMJIT_NO_LOGGING
    RegT reg(Globals::NoInit);
    _new_reg_fmt(Out<Reg>{reg}, type, s, std::forward<Args>(args)...);
    return reg;
#else
    Support::maybe_unused(s, std::forward<Args>(args)...);
    return _new_reg_internal<RegT>(type);
#endif
  }
  //! \endcond

  template<typename RegT, typename... Args>
  ASMJIT_INLINE_NODEBUG RegT new_similar_reg(const RegT& ref, Args&&... args) {
    return _new_reg_internal<RegT>(ref, std::forward<Args>(args)...);
  }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Reg new_reg(TypeId type_id, Args&&... args) { return _new_reg_internal<Reg>(type_id, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp(TypeId type_id, Args&&... args) { return _new_reg_internal<Gp>(type_id, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp32(Args&&... args) { return _new_reg_internal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp64(Args&&... args) { return _new_reg_internal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpw(Args&&... args) { return _new_reg_internal<Gp>(TypeId::kUInt32, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpx(Args&&... args) { return _new_reg_internal<Gp>(TypeId::kUInt64, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gpz(Args&&... args) { return _new_reg_internal<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }
  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Gp new_gp_ptr(Args&&... args) { return _new_reg_internal<Gp>(TypeId::kUIntPtr, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec(TypeId type_id, Args&&... args) { return _new_reg_internal<Vec>(type_id, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec_s(Args&&... args) { return _new_reg_internal<Vec>(TypeId::kFloat32, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec_d(Args&&... args) { return _new_reg_internal<Vec>(TypeId::kFloat64, std::forward<Args>(args)...); }

  template<typename... Args>
  ASMJIT_INLINE_NODEBUG Vec new_vec_q(Args&&... args) { return _new_reg_internal<Vec>(TypeId::kUInt8x16, std::forward<Args>(args)...); }

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new stack and returns a \ref Mem operand that can be used to address it.
  ASMJIT_INLINE_NODEBUG Mem new_stack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _new_stack(Out<BaseMem>(m), size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  ASMJIT_INLINE_NODEBUG Mem new_const(ConstPoolScope scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _new_const(Out<BaseMem>(m), scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool (8 bits).
  ASMJIT_INLINE_NODEBUG Mem new_byte_const(ConstPoolScope scope, uint8_t val) noexcept { return new_const(scope, &val, 1); }
  //! Put a HWORD `val` to a constant-pool (16 bits).
  ASMJIT_INLINE_NODEBUG Mem new_half_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool (32 bits).
  ASMJIT_INLINE_NODEBUG Mem new_word_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool (64 bits).
  ASMJIT_INLINE_NODEBUG Mem new_dword_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int16_const(ConstPoolScope scope, int16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint16_const(ConstPoolScope scope, uint16_t val) noexcept { return new_const(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int32_const(ConstPoolScope scope, int32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint32_const(ConstPoolScope scope, uint32_t val) noexcept { return new_const(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_int64_const(ConstPoolScope scope, int64_t val) noexcept { return new_const(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_uint64_const(ConstPoolScope scope, uint64_t val) noexcept { return new_const(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_float_const(ConstPoolScope scope, float val) noexcept { return new_const(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  ASMJIT_INLINE_NODEBUG Mem new_double_const(ConstPoolScope scope, double val) noexcept { return new_const(scope, &val, 8); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Force the compiler to not follow the conditional or unconditional jump.
  ASMJIT_INLINE_NODEBUG Compiler& unfollow() noexcept { _inst_options |= InstOptions::kUnfollow; return *this; }

  //! \}

  //! \name Compiler specific
  //! \{

  //! Special pseudo-instruction that can be used to load a memory address into `o0` GP register.
  //!
  //! \note At the moment this instruction is only useful to load a stack allocated address into a GP register
  //! for further use. It makes very little sense to use it for anything else. The semantics of this instruction
  //! is the same as X86 `LEA` (load effective address) instruction.
  ASMJIT_INLINE_NODEBUG Error load_address_of(const Gp& o0, const Mem& o1) { return _emitter()->_emitI(Inst::kIdAdr, o0, o1); }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  //! Invoke a function call without `target` type enforcement.
  ASMJIT_INLINE_NODEBUG Error invoke_(Out<InvokeNode*> out, const Operand_& target, const FuncSignature& signature) {
    return add_invoke_node(out, Inst::kIdBlr, target, signature);
  }

  //! Invoke a function call of the given `target` and `signature` and store the added node to `out`.
  //!
  //! Creates a new \ref InvokeNode, initializes all the necessary members to match the given function `signature`,
  //! adds the node to the compiler, and stores its pointer to `out`. The operation is atomic, if anything fails
  //! nullptr is stored in `out` and error code is returned.
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Gp& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Mem& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Label& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, const Imm& target, const FuncSignature& signature) { return invoke_(out, target, signature); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error invoke(Out<InvokeNode*> out, uint64_t target, const FuncSignature& signature) { return invoke_(out, Imm(int64_t(target)), signature); }

  //! Return.
  ASMJIT_INLINE_NODEBUG Error ret() { return add_ret(Operand(), Operand()); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0) { return add_ret(o0, Operand()); }
  //! \overload
  ASMJIT_INLINE_NODEBUG Error ret(const Reg& o0, const Reg& o1) { return add_ret(o0, o1); }

  //! \}

  //! \name Jump Tables Support
  //! \{

  using EmitterExplicitT<Compiler>::br;

  //! Adds a jump to the given `target` with the provided jump `annotation`.
  ASMJIT_INLINE_NODEBUG Error br(const Reg& target, JumpAnnotation* annotation) { return emit_annotated_jump(Inst::kIdBr, target, annotation); }

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error on_attach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_detach(CodeHolder& code) noexcept override;
  ASMJIT_API Error on_reinit(CodeHolder& code) noexcept override;

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // ASMJIT_ARM_A64COMPILER_H_INCLUDED
