// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_COMPILERFUNC_H
#define _ASMJIT_BASE_COMPILERFUNC_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/operand.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::FuncHint]
// ============================================================================

//! Function hints.
//!
//! For a platform specific calling conventions, see:
//!   - `X86FuncHint` - X86/X64 function hints.
ASMJIT_ENUM(FuncHint) {
  //! Generate a naked function by omitting its prolog and epilog (default true).
  //!
  //! Naked functions should always result in less code required for function's
  //! prolog and epilog. In addition, on X86/64 naked functions save one register
  //! (ebp or rbp), which can be used by the function instead.
  kFuncHintNaked = 0,

  //! Generate a compact function prolog/epilog if possible (default true).
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! Use shorter, but possible slower prolog/epilog sequence to save/restore
  //! registers. At the moment this only enables emitting `leave` in function's
  //! epilog to make the code shorter, however, the counterpart `enter` is not
  //! used in function's prolog for performance reasons.
  kFuncHintCompact = 1,

  //! Emit `emms` instruction in the function's epilog.
  kFuncHintX86Emms = 17,
  //! Emit `sfence` instruction in the function's epilog.
  kFuncHintX86SFence = 18,
  //! Emit `lfence` instruction in the function's epilog.
  kFuncHintX86LFence = 19
};

// ============================================================================
// [asmjit::FuncFlags]
// ============================================================================

//! Function flags.
ASMJIT_ENUM(FuncFlags) {
  //! Whether the function is using naked (minimal) prolog / epilog.
  kFuncFlagIsNaked = 0x00000001,

  //! Whether an another function is called from this function.
  kFuncFlagIsCaller = 0x00000002,

  //! Whether the stack is not aligned to the required stack alignment,
  //! thus it has to be aligned manually.
  kFuncFlagIsStackMisaligned = 0x00000004,

  //! Whether the stack pointer is adjusted by the stack size needed
  //! to save registers and function variables.
  //!
  //! X86/X64 Specific
  //! ----------------
  //!
  //! Stack pointer (ESP/RSP) is adjusted by 'sub' instruction in prolog and by
  //! 'add' instruction in epilog (only if function is not naked). If function
  //! needs to perform manual stack alignment more instructions are used to
  //! adjust the stack (like "and zsp, -Alignment").
  kFuncFlagIsStackAdjusted = 0x00000008,

  //! Whether the function is finished using `Compiler::endFunc()`.
  kFuncFlagIsFinished = 0x80000000,

  //! Whether to emit `leave` instead of two instructions in case that the
  //! function saves and restores the frame pointer.
  kFuncFlagX86Leave = 0x00010000,

  //! Whether it's required to move arguments to a new stack location,
  //! because of manual aligning.
  kFuncFlagX86MoveArgs = 0x00040000,

  //! Whether to emit `emms` instruction in epilog (auto-detected).
  kFuncFlagX86Emms = 0x01000000,

  //! Whether to emit `sfence` instruction in epilog (auto-detected).
  //!
  //! `kFuncFlagX86SFence` with `kFuncFlagX86LFence` results in emitting `mfence`.
  kFuncFlagX86SFence = 0x02000000,

  //! Whether to emit `lfence` instruction in epilog (auto-detected).
  //!
  //! `kFuncFlagX86SFence` with `kFuncFlagX86LFence` results in emitting `mfence`.
  kFuncFlagX86LFence = 0x04000000
};

// ============================================================================
// [asmjit::FuncDir]
// ============================================================================

//! Function arguments direction.
ASMJIT_ENUM(FuncDir) {
  //! Arguments are passed left to right.
  //!
  //! This arguments direction is unusual in C, however it's used in Pascal.
  kFuncDirLTR = 0,

  //! Arguments are passed right ro left
  //!
  //! This is the default argument direction in C.
  kFuncDirRTL = 1
};

// ============================================================================
// [asmjit::FuncMisc]
// ============================================================================

enum {
  //! Function doesn't have variable number of arguments (`...`) (default).
  kFuncNoVarArgs = 0xFF,
  //! Invalid stack offset in function or function parameter.
  kFuncStackInvalid = -1
};

// ============================================================================
// [asmjit::FuncArgIndex]
// ============================================================================

//! Function argument index (lo/hi).
ASMJIT_ENUM(FuncArgIndex) {
  //! Maxumum number of function arguments supported by AsmJit.
  kFuncArgCount = 16,
  //! Extended maximum number of arguments (used internally).
  kFuncArgCountLoHi = kFuncArgCount * 2,

  //! Index to the LO part of function argument (default).
  //!
  //! This value is typically omitted and added only if there is HI argument
  //! accessed.
  kFuncArgLo = 0,

  //! Index to the HI part of function argument.
  //!
  //! HI part of function argument depends on target architecture. On x86 it's
  //! typically used to transfer 64-bit integers (they form a pair of 32-bit
  //! integers).
  kFuncArgHi = kFuncArgCount
};

// ============================================================================
// [asmjit::FuncRet]
// ============================================================================

//! Function return value (lo/hi) specification.
ASMJIT_ENUM(FuncRet) {
  //! Index to the LO part of function return value.
  kFuncRetLo = 0,
  //! Index to the HI part of function return value.
  kFuncRetHi = 1
};

// ============================================================================
// [asmjit::TypeId]
// ============================================================================

//! Function builder's `void` type.
struct Void {};

//! Function builder's `int8_t` type.
struct Int8Type {};
//! Function builder's `uint8_t` type.
struct UInt8Type {};

//! Function builder's `int16_t` type.
struct Int16Type {};
//! Function builder's `uint16_t` type.
struct UInt16Type {};

//! Function builder's `int32_t` type.
struct Int32Type {};
//! Function builder's `uint32_t` type.
struct UInt32Type {};

//! Function builder's `int64_t` type.
struct Int64Type {};
//! Function builder's `uint64_t` type.
struct UInt64Type {};

//! Function builder's `intptr_t` type.
struct IntPtrType {};
//! Function builder's `uintptr_t` type.
struct UIntPtrType {};

//! Function builder's `float` type.
struct FloatType {};
//! Function builder's `double` type.
struct DoubleType {};

#if !defined(ASMJIT_DOCGEN)
template<typename T>
struct TypeId {
  // Let it fail here if `T` was not specialized.
};

template<typename T>
struct TypeId<T*> {
  enum { kId = kVarTypeIntPtr };
};

template<typename T>
struct TypeIdOfInt {
  enum { kId = (sizeof(T) == 1) ? (int)(IntTraits<T>::kIsSigned ? kVarTypeInt8  : kVarTypeUInt8 ) :
               (sizeof(T) == 2) ? (int)(IntTraits<T>::kIsSigned ? kVarTypeInt16 : kVarTypeUInt16) :
               (sizeof(T) == 4) ? (int)(IntTraits<T>::kIsSigned ? kVarTypeInt32 : kVarTypeUInt32) :
               (sizeof(T) == 8) ? (int)(IntTraits<T>::kIsSigned ? kVarTypeInt64 : kVarTypeUInt64) : (int)kInvalidVar
  };
};

#define ASMJIT_TYPE_ID(T, ID) \
  template<> struct TypeId<T> { enum { kId = ID }; }

ASMJIT_TYPE_ID(void              , kInvalidVar);
ASMJIT_TYPE_ID(signed char       , TypeIdOfInt<signed char>::kId);
ASMJIT_TYPE_ID(unsigned char     , TypeIdOfInt<unsigned char>::kId);
ASMJIT_TYPE_ID(short             , TypeIdOfInt<short>::kId);
ASMJIT_TYPE_ID(unsigned short    , TypeIdOfInt<unsigned short>::kId);
ASMJIT_TYPE_ID(int               , TypeIdOfInt<int>::kId);
ASMJIT_TYPE_ID(unsigned int      , TypeIdOfInt<unsigned int>::kId);
ASMJIT_TYPE_ID(long              , TypeIdOfInt<long>::kId);
ASMJIT_TYPE_ID(unsigned long     , TypeIdOfInt<unsigned long>::kId);
ASMJIT_TYPE_ID(float             , kVarTypeFp32);
ASMJIT_TYPE_ID(double            , kVarTypeFp64);

#if ASMJIT_CC_HAS_NATIVE_CHAR
ASMJIT_TYPE_ID(char              , TypeIdOfInt<char>::kId);
#endif
#if ASMJIT_CC_HAS_NATIVE_WCHAR_T
ASMJIT_TYPE_ID(wchar_t           , TypeIdOfInt<wchar_t>::kId);
#endif
#if ASMJIT_CC_HAS_NATIVE_CHAR16_T
ASMJIT_TYPE_ID(char16_t          , TypeIdOfInt<char16_t>::kId);
#endif
#if ASMJIT_CC_HAS_NATIVE_CHAR32_T
ASMJIT_TYPE_ID(char32_t          , TypeIdOfInt<char32_t>::kId);
#endif

#if ASMJIT_CC_MSC && !ASMJIT_CC_MSC_GE(16, 0, 0)
ASMJIT_TYPE_ID(__int64           , TypeIdOfInt<__int64>::kId);
ASMJIT_TYPE_ID(unsigned __int64  , TypeIdOfInt<unsigned __int64>::kId);
#else
ASMJIT_TYPE_ID(long long         , TypeIdOfInt<long long>::kId);
ASMJIT_TYPE_ID(unsigned long long, TypeIdOfInt<unsigned long long>::kId);
#endif

ASMJIT_TYPE_ID(Void              , kInvalidVar);
ASMJIT_TYPE_ID(Int8Type          , kVarTypeInt8);
ASMJIT_TYPE_ID(UInt8Type         , kVarTypeUInt8);
ASMJIT_TYPE_ID(Int16Type         , kVarTypeInt16);
ASMJIT_TYPE_ID(UInt16Type        , kVarTypeUInt16);
ASMJIT_TYPE_ID(Int32Type         , kVarTypeInt32);
ASMJIT_TYPE_ID(UInt32Type        , kVarTypeUInt32);
ASMJIT_TYPE_ID(Int64Type         , kVarTypeInt64);
ASMJIT_TYPE_ID(UInt64Type        , kVarTypeUInt64);
ASMJIT_TYPE_ID(IntPtrType        , kVarTypeIntPtr);
ASMJIT_TYPE_ID(UIntPtrType       , kVarTypeUIntPtr);
ASMJIT_TYPE_ID(FloatType         , kVarTypeFp32);
ASMJIT_TYPE_ID(DoubleType        , kVarTypeFp64);
#endif // !ASMJIT_DOCGEN

// ============================================================================
// [asmjit::FuncInOut]
// ============================================================================

//! Function in/out - argument or return value translated from `FuncPrototype`.
struct FuncInOut {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint32_t getVarType() const noexcept { return _varType; }

  ASMJIT_INLINE bool hasRegIndex() const noexcept { return _regIndex != kInvalidReg; }
  ASMJIT_INLINE uint32_t getRegIndex() const noexcept { return _regIndex; }

  ASMJIT_INLINE bool hasStackOffset() const noexcept { return _stackOffset != kFuncStackInvalid; }
  ASMJIT_INLINE int32_t getStackOffset() const noexcept { return static_cast<int32_t>(_stackOffset); }

  //! Get whether the argument / return value is assigned.
  ASMJIT_INLINE bool isSet() const noexcept {
    return (_regIndex != kInvalidReg) | (_stackOffset != kFuncStackInvalid);
  }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the function argument to "unassigned state".
  ASMJIT_INLINE void reset() noexcept { _packed = 0xFFFFFFFFU; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      //! Variable type, see \ref VarType.
      uint8_t _varType;
      //! Register index if argument / return value is a register.
      uint8_t _regIndex;
      //! Stack offset if argument / return value is on the stack.
      int16_t _stackOffset;
    };

    //! All members packed into single 32-bit integer.
    uint32_t _packed;
  };
};

// ============================================================================
// [asmjit::FuncPrototype]
// ============================================================================

//! Function prototype.
//!
//! Function prototype contains information about function return type, count
//! of arguments and their types. Function prototype is a low level structure
//! which doesn't contain platform specific or calling convention specific
//! information. Function prototype is used to create a `FuncDecl`.
struct FuncPrototype {
  // --------------------------------------------------------------------------
  // [Setup]
  // --------------------------------------------------------------------------

  //! Setup the prototype.
  ASMJIT_INLINE void setup(
    uint32_t callConv,
    uint32_t ret,
    const uint32_t* args, uint32_t numArgs) noexcept {

    ASMJIT_ASSERT(callConv <= 0xFF);
    ASMJIT_ASSERT(numArgs <= 0xFF);

    _callConv = static_cast<uint8_t>(callConv);
    _varArgs = kFuncNoVarArgs;
    _numArgs = static_cast<uint8_t>(numArgs);
    _reserved = 0;

    _ret = ret;
    _args = args;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the function's calling convention.
  ASMJIT_INLINE uint32_t getCallConv() const noexcept { return _callConv; }
  //! Get the variable arguments `...` index, `kFuncNoVarArgs` if none.
  ASMJIT_INLINE uint32_t getVarArgs() const noexcept { return _varArgs; }
  //! Get the number of function arguments.
  ASMJIT_INLINE uint32_t getNumArgs() const noexcept { return _numArgs; }

  //! Get the return value type.
  ASMJIT_INLINE uint32_t getRet() const noexcept { return _ret; }
  //! Get the type of the argument at index `i`.
  ASMJIT_INLINE uint32_t getArg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _numArgs);
    return _args[i];
  }
  //! Get the array of function arguments' types.
  ASMJIT_INLINE const uint32_t* getArgs() const noexcept { return _args; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _callConv;
  uint8_t _varArgs;
  uint8_t _numArgs;
  uint8_t _reserved;

  uint32_t _ret;
  const uint32_t* _args;
};

// ============================================================================
// [asmjit::FuncBuilderX]
// ============================================================================

// TODO: Rename to `DynamicFuncBuilder`
//! Custom function builder for up to 32 function arguments.
struct FuncBuilderX : public FuncPrototype {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE FuncBuilderX(uint32_t callConv = kCallConvHost) noexcept {
    setup(callConv, kInvalidVar, _builderArgList, 0);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void setCallConv(uint32_t callConv) noexcept {
    ASMJIT_ASSERT(callConv <= 0xFF);
    _callConv = static_cast<uint8_t>(callConv);
  }

  //! Set the return type to `retType`.
  ASMJIT_INLINE void setRet(uint32_t retType) noexcept {
    _ret = retType;
  }
  //! Set the return type based on `T`.
  template<typename T>
  ASMJIT_INLINE void setRetT() noexcept { setRet(TypeId<T>::kId); }

  //! Set the argument at index `i` to the `type`
  ASMJIT_INLINE void setArg(uint32_t i, uint32_t type) noexcept {
    ASMJIT_ASSERT(i < _numArgs);
    _builderArgList[i] = type;
  }
  //! Set the argument at index `i` to the type based on `T`.
  template<typename T>
  ASMJIT_INLINE void setArgT(uint32_t i) noexcept { setArg(i, TypeId<T>::kId); }

  //! Append an argument of `type` to the function prototype.
  ASMJIT_INLINE void addArg(uint32_t type) noexcept {
    ASMJIT_ASSERT(_numArgs < kFuncArgCount);
    _builderArgList[_numArgs++] = type;
  }
  //! Append an argument of type based on `T` to the function prototype.
  template<typename T>
  ASMJIT_INLINE void addArgT() noexcept { addArg(TypeId<T>::kId); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _builderArgList[kFuncArgCount];
};

//! \internal
#define T(_Type_) TypeId<_Type_>::kId

//! Function prototype (no args).
template<typename RET>
struct FuncBuilder0 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder0(uint32_t callConv = kCallConvHost) noexcept {
    setup(callConv, T(RET), nullptr, 0);
  }
};

//! Function prototype (1 argument).
template<typename RET, typename P0>
struct FuncBuilder1 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder1(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (2 arguments).
template<typename RET, typename P0, typename P1>
struct FuncBuilder2 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder2(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (3 arguments).
template<typename RET, typename P0, typename P1, typename P2>
struct FuncBuilder3 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder3(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (4 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3>
struct FuncBuilder4 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder4(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (5 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3, typename P4>
struct FuncBuilder5 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder5(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3), T(P4) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (6 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
struct FuncBuilder6 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder6(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3), T(P4), T(P5) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (7 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct FuncBuilder7 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder7(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3), T(P4), T(P5), T(P6) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (8 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct FuncBuilder8 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder8(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3), T(P4), T(P5), T(P6), T(P7) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (9 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncBuilder9 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder9(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3), T(P4), T(P5), T(P6), T(P7), T(P8) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};

//! Function prototype (10 arguments).
template<typename RET, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
struct FuncBuilder10 : public FuncPrototype {
  ASMJIT_INLINE FuncBuilder10(uint32_t callConv = kCallConvHost) noexcept {
    static const uint32_t args[] = { T(P0), T(P1), T(P2), T(P3), T(P4), T(P5), T(P6), T(P7), T(P8), T(P9) };
    setup(callConv, T(RET), args, ASMJIT_ARRAY_SIZE(args));
  }
};
#undef T

// ============================================================================
// [asmjit::FuncDecl]
// ============================================================================

//! Function declaration.
struct FuncDecl {
  // --------------------------------------------------------------------------
  // [Accessors - Calling Convention]
  // --------------------------------------------------------------------------

  //! Get the function's calling convention, see `CallConv`.
  ASMJIT_INLINE uint32_t getCallConv() const noexcept { return _callConv; }

  //! Get whether the callee pops the stack.
  ASMJIT_INLINE uint32_t getCalleePopsStack() const noexcept { return _calleePopsStack; }

  //! Get direction of arguments passed on the stack.
  //!
  //! Direction should be always `kFuncDirRTL`.
  //!
  //! \note This is related to used calling convention, it's not affected by
  //! number of function arguments or their types.
  ASMJIT_INLINE uint32_t getArgsDirection() const noexcept { return _argsDirection; }

  //! Get stack size needed for function arguments passed on the stack.
  ASMJIT_INLINE uint32_t getArgStackSize() const noexcept { return _argStackSize; }
  //! Get size of "Red Zone".
  ASMJIT_INLINE uint32_t getRedZoneSize() const noexcept { return _redZoneSize; }
  //! Get size of "Spill Zone".
  ASMJIT_INLINE uint32_t getSpillZoneSize() const noexcept { return _spillZoneSize; }

  // --------------------------------------------------------------------------
  // [Accessors - Arguments and Return]
  // --------------------------------------------------------------------------

  //! Get whether the function has a return value.
  ASMJIT_INLINE bool hasRet() const noexcept { return _retCount != 0; }
  //! Get count of function return values.
  ASMJIT_INLINE uint32_t getRetCount() const noexcept { return _retCount; }

  //! Get function return value.
  ASMJIT_INLINE FuncInOut& getRet(uint32_t index = kFuncRetLo) noexcept { return _rets[index]; }
  //! Get function return value.
  ASMJIT_INLINE const FuncInOut& getRet(uint32_t index = kFuncRetLo) const noexcept { return _rets[index]; }

  //! Get the number of function arguments.
  ASMJIT_INLINE uint32_t getNumArgs() const noexcept { return _numArgs; }

  //! Get function arguments array.
  ASMJIT_INLINE FuncInOut* getArgs() noexcept { return _args; }
  //! Get function arguments array (const).
  ASMJIT_INLINE const FuncInOut* getArgs() const noexcept { return _args; }

  //! Get function argument at index `index`.
  ASMJIT_INLINE FuncInOut& getArg(size_t index) noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index];
  }

  //! Get function argument at index `index`.
  ASMJIT_INLINE const FuncInOut& getArg(size_t index) const noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index];
  }

  ASMJIT_INLINE void resetArg(size_t index) noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    _args[index].reset();
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Calling convention.
  uint8_t _callConv;
  //! Whether a callee pops stack.
  uint8_t _calleePopsStack : 1;
  //! Direction for arguments passed on the stack, see `FuncDir`.
  uint8_t _argsDirection : 1;
  //! Reserved #0 (alignment).
  uint8_t _reserved0 : 6;

  //! Number of function arguments.
  uint8_t _numArgs;
  //! Number of function return values.
  uint8_t _retCount;

  //! Count of bytes consumed by arguments on the stack (aligned).
  uint32_t _argStackSize;

  //! Size of "Red Zone".
  //!
  //! \note Used by AMD64-ABI (128 bytes).
  uint16_t _redZoneSize;

  //! Size of "Spill Zone".
  //!
  //! \note Used by WIN64-ABI (32 bytes).
  uint16_t _spillZoneSize;

  //! Function arguments (LO & HI) mapped to physical registers and stack.
  FuncInOut _args[kFuncArgCountLoHi];

  //! Function return value(s).
  FuncInOut _rets[2];
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_BASE_COMPILERFUNC_H
