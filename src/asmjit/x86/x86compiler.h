// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86COMPILER_H
#define _ASMJIT_X86_X86COMPILER_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/codecompiler.h"
#include "../core/simdtypes.h"
#include "../core/type.h"
#include "../x86/x86emitter.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::X86Compiler]
// ============================================================================

//! Architecture-dependent \ref CodeCompiler targeting X86 and X64.
class ASMJIT_VIRTAPI X86Compiler
  : public CodeCompiler,
    public X86EmitterExplicitT<X86Compiler> {
public:
  ASMJIT_NONCOPYABLE(X86Compiler)
  typedef CodeCompiler Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `X86Compiler` instance.
  ASMJIT_API explicit X86Compiler(CodeHolder* code = nullptr) noexcept;
  //! Destroy the `X86Compiler` instance.
  ASMJIT_API virtual ~X86Compiler() noexcept;

  // --------------------------------------------------------------------------
  // [Finalize]
  // --------------------------------------------------------------------------

  ASMJIT_API Error finalize() override;

  // --------------------------------------------------------------------------
  // [VirtReg]
  // --------------------------------------------------------------------------

#ifndef ASMJIT_DISABLE_LOGGING
# define ASMJIT_NEW_REG(OUT, PARAM, NAME_FMT)                 \
    std::va_list ap;                                          \
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

  ASMJIT_NEW_REG_USER(newReg    , X86Reg )
  ASMJIT_NEW_REG_USER(newGpReg  , X86Gp  )
  ASMJIT_NEW_REG_USER(newMmReg  , X86Mm  )
  ASMJIT_NEW_REG_USER(newKReg   , X86KReg)
  ASMJIT_NEW_REG_USER(newVecReg , X86Vec )
  ASMJIT_NEW_REG_USER(newXmmReg , X86Xmm )
  ASMJIT_NEW_REG_USER(newYmmReg , X86Ymm )
  ASMJIT_NEW_REG_USER(newZmmReg , X86Zmm )

  ASMJIT_NEW_REG_AUTO(newI8     , X86Gp  , Type::kIdI8     )
  ASMJIT_NEW_REG_AUTO(newU8     , X86Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_AUTO(newI16    , X86Gp  , Type::kIdI16    )
  ASMJIT_NEW_REG_AUTO(newU16    , X86Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_AUTO(newI32    , X86Gp  , Type::kIdI32    )
  ASMJIT_NEW_REG_AUTO(newU32    , X86Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_AUTO(newI64    , X86Gp  , Type::kIdI64    )
  ASMJIT_NEW_REG_AUTO(newU64    , X86Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_AUTO(newInt8   , X86Gp  , Type::kIdI8     )
  ASMJIT_NEW_REG_AUTO(newUInt8  , X86Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_AUTO(newInt16  , X86Gp  , Type::kIdI16    )
  ASMJIT_NEW_REG_AUTO(newUInt16 , X86Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_AUTO(newInt32  , X86Gp  , Type::kIdI32    )
  ASMJIT_NEW_REG_AUTO(newUInt32 , X86Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_AUTO(newInt64  , X86Gp  , Type::kIdI64    )
  ASMJIT_NEW_REG_AUTO(newUInt64 , X86Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_AUTO(newIntPtr , X86Gp  , Type::kIdIntPtr )
  ASMJIT_NEW_REG_AUTO(newUIntPtr, X86Gp  , Type::kIdUIntPtr)

  ASMJIT_NEW_REG_AUTO(newGpb    , X86Gp  , Type::kIdU8     )
  ASMJIT_NEW_REG_AUTO(newGpw    , X86Gp  , Type::kIdU16    )
  ASMJIT_NEW_REG_AUTO(newGpd    , X86Gp  , Type::kIdU32    )
  ASMJIT_NEW_REG_AUTO(newGpq    , X86Gp  , Type::kIdU64    )
  ASMJIT_NEW_REG_AUTO(newGpz    , X86Gp  , Type::kIdUIntPtr)
  ASMJIT_NEW_REG_AUTO(newKb     , X86KReg, Type::kIdMask8  )
  ASMJIT_NEW_REG_AUTO(newKw     , X86KReg, Type::kIdMask16 )
  ASMJIT_NEW_REG_AUTO(newKd     , X86KReg, Type::kIdMask32 )
  ASMJIT_NEW_REG_AUTO(newKq     , X86KReg, Type::kIdMask64 )
  ASMJIT_NEW_REG_AUTO(newMm     , X86Mm  , Type::kIdMmx64  )
  ASMJIT_NEW_REG_AUTO(newXmm    , X86Xmm , Type::kIdI32x4  )
  ASMJIT_NEW_REG_AUTO(newXmmSs  , X86Xmm , Type::kIdF32x1  )
  ASMJIT_NEW_REG_AUTO(newXmmSd  , X86Xmm , Type::kIdF64x1  )
  ASMJIT_NEW_REG_AUTO(newXmmPs  , X86Xmm , Type::kIdF32x4  )
  ASMJIT_NEW_REG_AUTO(newXmmPd  , X86Xmm , Type::kIdF64x2  )
  ASMJIT_NEW_REG_AUTO(newYmm    , X86Ymm , Type::kIdI32x8  )
  ASMJIT_NEW_REG_AUTO(newYmmPs  , X86Ymm , Type::kIdF32x8  )
  ASMJIT_NEW_REG_AUTO(newYmmPd  , X86Ymm , Type::kIdF64x4  )
  ASMJIT_NEW_REG_AUTO(newZmm    , X86Zmm , Type::kIdI32x16 )
  ASMJIT_NEW_REG_AUTO(newZmmPs  , X86Zmm , Type::kIdF32x16 )
  ASMJIT_NEW_REG_AUTO(newZmmPd  , X86Zmm , Type::kIdF64x8  )

#undef ASMJIT_NEW_REG_AUTO
#undef ASMJIT_NEW_REG_USER
#undef ASMJIT_NEW_REG

  // --------------------------------------------------------------------------
  // [Stack]
  // --------------------------------------------------------------------------

  //! Create a new memory chunk allocated on the current function's stack.
  inline X86Mem newStack(uint32_t size, uint32_t alignment, const char* name = nullptr) {
    X86Mem m(Globals::NoInit);
    _newStack(m, size, alignment, name);
    return m;
  }

  // --------------------------------------------------------------------------
  // [Const]
  // --------------------------------------------------------------------------

  //! Put data to a constant-pool and get a memory reference to it.
  inline X86Mem newConst(uint32_t scope, const void* data, size_t size) {
    X86Mem m(Globals::NoInit);
    _newConst(m, scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool.
  inline X86Mem newByteConst(uint32_t scope, uint8_t val) noexcept { return newConst(scope, &val, 1); }
  //! Put a WORD `val` to a constant-pool.
  inline X86Mem newWordConst(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  inline X86Mem newDWordConst(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  inline X86Mem newQWordConst(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  inline X86Mem newInt16Const(uint32_t scope, int16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  inline X86Mem newUInt16Const(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  inline X86Mem newInt32Const(uint32_t scope, int32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  inline X86Mem newUInt32Const(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  inline X86Mem newInt64Const(uint32_t scope, int64_t val) noexcept { return newConst(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  inline X86Mem newUInt64Const(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  inline X86Mem newFloatConst(uint32_t scope, float val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  inline X86Mem newDoubleConst(uint32_t scope, double val) noexcept { return newConst(scope, &val, 8); }

  //! Put a MMX `val` to a constant-pool.
  inline X86Mem newMmConst(uint32_t scope, const Data64& val) noexcept { return newConst(scope, &val, 8); }
  //! Put a XMM `val` to a constant-pool.
  inline X86Mem newXmmConst(uint32_t scope, const Data128& val) noexcept { return newConst(scope, &val, 16); }
  //! Put a YMM `val` to a constant-pool.
  inline X86Mem newYmmConst(uint32_t scope, const Data256& val) noexcept { return newConst(scope, &val, 32); }

  // --------------------------------------------------------------------------
  // [Instruction Options]
  // --------------------------------------------------------------------------

  //! Force the compiler to not follow the conditional or unconditional jump.
  inline X86Compiler& unfollow() noexcept { _instOptions |= Inst::kOptionUnfollow; return *this; }
  //! Tell the compiler that the destination variable will be overwritten.
  inline X86Compiler& overwrite() noexcept { _instOptions |= Inst::kOptionOverwrite; return *this; }

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  //! Call a function.
  inline CCFuncCall* call(const X86Gp& dst, const FuncSignature& sign) { return addCall(X86Inst::kIdCall, dst, sign); }
  //! \overload
  inline CCFuncCall* call(const X86Mem& dst, const FuncSignature& sign) { return addCall(X86Inst::kIdCall, dst, sign); }
  //! \overload
  inline CCFuncCall* call(const Label& label, const FuncSignature& sign) { return addCall(X86Inst::kIdCall, label, sign); }
  //! \overload
  inline CCFuncCall* call(const Imm& dst, const FuncSignature& sign) { return addCall(X86Inst::kIdCall, dst, sign); }
  //! \overload
  inline CCFuncCall* call(uint64_t dst, const FuncSignature& sign) { return addCall(X86Inst::kIdCall, Imm(int64_t(dst)), sign); }

  //! Return.
  inline CCFuncRet* ret() { return addRet(Operand(), Operand()); }
  //! \overload
  inline CCFuncRet* ret(const Reg& o0) { return addRet(o0, Operand()); }
  //! \overload
  inline CCFuncRet* ret(const Reg& o0, const Reg& o1) { return addRet(o0, o1); }

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_X86_X86COMPILER_H
