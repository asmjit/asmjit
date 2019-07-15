// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86COMPILER_H
#define _ASMJIT_X86_X86COMPILER_H

#include "../core/build.h"
#ifndef ASMJIT_NO_COMPILER

#include "../core/compiler.h"
#include "../core/datatypes.h"
#include "../core/type.h"
#include "../x86/x86emitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::Compiler]
// ============================================================================

//! Architecture-dependent asm-compiler (X86).
class ASMJIT_VIRTAPI Compiler
  : public BaseCompiler,
    public EmitterExplicitT<Compiler> {
public:
  ASMJIT_NONCOPYABLE(Compiler)
  typedef BaseCompiler Base;

  //! \name Construction & Destruction
  //! \{

  ASMJIT_API explicit Compiler(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Compiler() noexcept;

  //! \}

  //! \name Virtual Registers
  //! \{

#ifndef ASMJIT_NO_LOGGING
# define ASMJIT_NEW_REG(OUT, PARAM, NAME_FMT)                 \
    va_list ap;                                               \
    va_start(ap, NAME_FMT);                                   \
    _newReg(OUT, PARAM, NAME_FMT, ap);                        \
    va_end(ap)
#else
# define ASMJIT_NEW_REG(OUT, PARAM, NAME_FMT)                 \
    ASMJIT_UNUSED(NAME_FMT);                                  \
    _newReg(OUT, PARAM, nullptr)
#endif

#define ASMJIT_NEW_REG_USER(FUNC, REG)                        \
    inline REG FUNC(uint32_t typeId) {                        \
      REG reg(Globals::NoInit);                               \
      _newReg(reg, typeId, nullptr);                          \
      return reg;                                             \
    }                                                         \
                                                              \
    inline REG FUNC(uint32_t typeId, const char* fmt, ...) {  \
      REG reg(Globals::NoInit);                               \
      ASMJIT_NEW_REG(reg, typeId, fmt);                       \
      return reg;                                             \
    }

#define ASMJIT_NEW_REG_AUTO(FUNC, REG, TYPE_ID)               \
    inline REG FUNC() {                                       \
      REG reg(Globals::NoInit);                               \
      _newReg(reg, TYPE_ID, nullptr);                         \
      return reg;                                             \
    }                                                         \
                                                              \
    inline REG FUNC(const char* fmt, ...) {                   \
      REG reg(Globals::NoInit);                               \
      ASMJIT_NEW_REG(reg, TYPE_ID, fmt);                      \
      return reg;                                             \
    }

  template<typename RegT>
  inline RegT newSimilarReg(const RegT& ref) {
    RegT reg(Globals::NoInit);
    _newReg(reg, ref, nullptr);
    return reg;
  }

  template<typename RegT>
  inline RegT newSimilarReg(const RegT& ref, const char* fmt, ...) {
    RegT reg(Globals::NoInit);
    ASMJIT_NEW_REG(reg, ref, fmt);
    return reg;
  }

  ASMJIT_NEW_REG_USER(newReg    , Reg )
  ASMJIT_NEW_REG_USER(newGp     , Gp  )
  ASMJIT_NEW_REG_USER(newVec    , Vec )
  ASMJIT_NEW_REG_USER(newK      , KReg)

  ASMJIT_NEW_REG_AUTO(newI8     , Gp  , Type::kIdI8     )
  ASMJIT_NEW_REG_AUTO(newU8     , Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_AUTO(newI16    , Gp  , Type::kIdI16    )
  ASMJIT_NEW_REG_AUTO(newU16    , Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_AUTO(newI32    , Gp  , Type::kIdI32    )
  ASMJIT_NEW_REG_AUTO(newU32    , Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_AUTO(newI64    , Gp  , Type::kIdI64    )
  ASMJIT_NEW_REG_AUTO(newU64    , Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_AUTO(newInt8   , Gp  , Type::kIdI8     )
  ASMJIT_NEW_REG_AUTO(newUInt8  , Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_AUTO(newInt16  , Gp  , Type::kIdI16    )
  ASMJIT_NEW_REG_AUTO(newUInt16 , Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_AUTO(newInt32  , Gp  , Type::kIdI32    )
  ASMJIT_NEW_REG_AUTO(newUInt32 , Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_AUTO(newInt64  , Gp  , Type::kIdI64    )
  ASMJIT_NEW_REG_AUTO(newUInt64 , Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_AUTO(newIntPtr , Gp  , Type::kIdIntPtr )
  ASMJIT_NEW_REG_AUTO(newUIntPtr, Gp  , Type::kIdUIntPtr)

  ASMJIT_NEW_REG_AUTO(newGpb    , Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_AUTO(newGpw    , Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_AUTO(newGpd    , Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_AUTO(newGpq    , Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_AUTO(newGpz    , Gp  , Type::kIdUIntPtr)
  ASMJIT_NEW_REG_AUTO(newXmm    , Xmm , Type::kIdI32x4  )
  ASMJIT_NEW_REG_AUTO(newXmmSs  , Xmm , Type::kIdF32x1  )
  ASMJIT_NEW_REG_AUTO(newXmmSd  , Xmm , Type::kIdF64x1  )
  ASMJIT_NEW_REG_AUTO(newXmmPs  , Xmm , Type::kIdF32x4  )
  ASMJIT_NEW_REG_AUTO(newXmmPd  , Xmm , Type::kIdF64x2  )
  ASMJIT_NEW_REG_AUTO(newYmm    , Ymm , Type::kIdI32x8  )
  ASMJIT_NEW_REG_AUTO(newYmmPs  , Ymm , Type::kIdF32x8  )
  ASMJIT_NEW_REG_AUTO(newYmmPd  , Ymm , Type::kIdF64x4  )
  ASMJIT_NEW_REG_AUTO(newZmm    , Zmm , Type::kIdI32x16 )
  ASMJIT_NEW_REG_AUTO(newZmmPs  , Zmm , Type::kIdF32x16 )
  ASMJIT_NEW_REG_AUTO(newZmmPd  , Zmm , Type::kIdF64x8  )
  ASMJIT_NEW_REG_AUTO(newMm     , Mm  , Type::kIdMmx64  )
  ASMJIT_NEW_REG_AUTO(newKb     , KReg, Type::kIdMask8  )
  ASMJIT_NEW_REG_AUTO(newKw     , KReg, Type::kIdMask16 )
  ASMJIT_NEW_REG_AUTO(newKd     , KReg, Type::kIdMask32 )
  ASMJIT_NEW_REG_AUTO(newKq     , KReg, Type::kIdMask64 )

#undef ASMJIT_NEW_REG_AUTO
#undef ASMJIT_NEW_REG_USER
#undef ASMJIT_NEW_REG

  //! \}

  //! \name Stack
  //! \{

  //! Creates a new memory chunk allocated on the current function's stack.
  inline Mem newStack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    Mem m(Globals::NoInit);
    _newStack(m, size, alignment, name);
    return m;
  }

  //! \}

  //! \name Constants
  //! \{

  //! Put data to a constant-pool and get a memory reference to it.
  inline Mem newConst(uint32_t scope, const void* data, size_t size) {
    Mem m(Globals::NoInit);
    _newConst(m, scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool.
  inline Mem newByteConst(uint32_t scope, uint8_t val) noexcept { return newConst(scope, &val, 1); }
  //! Put a WORD `val` to a constant-pool.
  inline Mem newWordConst(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  inline Mem newDWordConst(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  inline Mem newQWordConst(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  inline Mem newInt16Const(uint32_t scope, int16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  inline Mem newUInt16Const(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  inline Mem newInt32Const(uint32_t scope, int32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  inline Mem newUInt32Const(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  inline Mem newInt64Const(uint32_t scope, int64_t val) noexcept { return newConst(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  inline Mem newUInt64Const(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  inline Mem newFloatConst(uint32_t scope, float val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  inline Mem newDoubleConst(uint32_t scope, double val) noexcept { return newConst(scope, &val, 8); }

  //! Put a MMX `val` to a constant-pool.
  inline Mem newMmConst(uint32_t scope, const Data64& val) noexcept { return newConst(scope, &val, 8); }
  //! Put a XMM `val` to a constant-pool.
  inline Mem newXmmConst(uint32_t scope, const Data128& val) noexcept { return newConst(scope, &val, 16); }
  //! Put a YMM `val` to a constant-pool.
  inline Mem newYmmConst(uint32_t scope, const Data256& val) noexcept { return newConst(scope, &val, 32); }

  //! \}

  //! \name Instruction Options
  //! \{

  //! Force the compiler to not follow the conditional or unconditional jump.
  inline Compiler& unfollow() noexcept { _instOptions |= Inst::kOptionUnfollow; return *this; }
  //! Tell the compiler that the destination variable will be overwritten.
  inline Compiler& overwrite() noexcept { _instOptions |= Inst::kOptionOverwrite; return *this; }

  //! \}

  //! \name Function Call & Ret Intrinsics
  //! \{

  //! Call a function.
  inline FuncCallNode* call(const Gp& dst, const FuncSignature& sign) { return addCall(Inst::kIdCall, dst, sign); }
  //! \overload
  inline FuncCallNode* call(const Mem& dst, const FuncSignature& sign) { return addCall(Inst::kIdCall, dst, sign); }
  //! \overload
  inline FuncCallNode* call(const Label& label, const FuncSignature& sign) { return addCall(Inst::kIdCall, label, sign); }
  //! \overload
  inline FuncCallNode* call(const Imm& dst, const FuncSignature& sign) { return addCall(Inst::kIdCall, dst, sign); }
  //! \overload
  inline FuncCallNode* call(uint64_t dst, const FuncSignature& sign) { return addCall(Inst::kIdCall, Imm(int64_t(dst)), sign); }

  //! Return.
  inline FuncRetNode* ret() { return addRet(Operand(), Operand()); }
  //! \overload
  inline FuncRetNode* ret(const BaseReg& o0) { return addRet(o0, Operand()); }
  //! \overload
  inline FuncRetNode* ret(const BaseReg& o0, const BaseReg& o1) { return addRet(o0, o1); }

  //! \}

  //! \name Finalize
  //! \{

  ASMJIT_API Error finalize() override;

  //! \}

  //! \name Events
  //! \{

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;

  //! \}
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_COMPILER
#endif // _ASMJIT_X86_X86COMPILER_H
