// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_FUNC_H
#define _ASMJIT_CORE_FUNC_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/callconv.h"
#include "../core/operand.h"
#include "../core/type.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_func
//! \{

// ============================================================================
// [asmjit::FuncArgIndex]
// ============================================================================

//! Function argument index (lo/hi).
enum FuncArgIndex : uint32_t {
  //! Maximum number of function arguments supported by AsmJit.
  kFuncArgCount = Globals::kMaxFuncArgs,
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
// [asmjit::FuncSignature]
// ============================================================================

//! Function signature.
//!
//! Contains information about function return type, count of arguments and
//! their TypeIds. Function signature is a low level structure which doesn't
//! contain platform specific or calling convention specific information.
struct FuncSignature {
  enum : uint8_t {
    //! Doesn't have variable number of arguments (`...`).
    kNoVarArgs = 0xFF
  };

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Initializes the function signature.
  inline void init(uint32_t ccId, uint32_t vaIndex, uint32_t ret, const uint8_t* args, uint32_t argCount) noexcept {
    ASMJIT_ASSERT(ccId <= 0xFF);
    ASMJIT_ASSERT(argCount <= 0xFF);

    _callConv = uint8_t(ccId);
    _argCount = uint8_t(argCount);
    _vaIndex = uint8_t(vaIndex);
    _ret = uint8_t(ret);
    _args = args;
  }

  inline void reset() noexcept { ::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets the calling convention.
  inline uint32_t callConv() const noexcept { return _callConv; }
  //! Sets the calling convention to `ccId`;
  inline void setCallConv(uint32_t ccId) noexcept { _callConv = uint8_t(ccId); }

  //! Gets whether the function has variable number of arguments (...).
  inline bool hasVarArgs() const noexcept { return _vaIndex != kNoVarArgs; }
  //! Gets the variable arguments (...) index, `kNoVarArgs` if none.
  inline uint32_t vaIndex() const noexcept { return _vaIndex; }
  //! Sets the variable arguments (...) index to `index`.
  inline void setVaIndex(uint32_t index) noexcept { _vaIndex = uint8_t(index); }
  //! Resets the variable arguments index (making it a non-va function).
  inline void resetVaIndex() noexcept { _vaIndex = kNoVarArgs; }

  //! Gets the number of function arguments.
  inline uint32_t argCount() const noexcept { return _argCount; }

  inline bool hasRet() const noexcept { return _ret != Type::kIdVoid; }
  //! Gets the return value type.
  inline uint32_t ret() const noexcept { return _ret; }

  //! Gets the type of the argument at index `i`.
  inline uint32_t arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _argCount);
    return _args[i];
  }
  //! Gets the array of function arguments' types.
  inline const uint8_t* args() const noexcept { return _args; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _callConv;                     //!< Calling convention id.
  uint8_t _argCount;                     //!< Count of arguments.
  uint8_t _vaIndex;                      //!< Index of a first VA or `kNoVarArgs`.
  uint8_t _ret;                          //!< Return value TypeId.
  const uint8_t* _args;                  //!< Function arguments TypeIds.
};

// ============================================================================
// [asmjit::FuncSignatureT]
// ============================================================================

template<typename... RET_ARGS>
class FuncSignatureT : public FuncSignature {
public:
  inline FuncSignatureT(uint32_t ccId = CallConv::kIdHost, uint32_t vaIndex = kNoVarArgs) noexcept {
    static const uint8_t ret_args[] = { (uint8_t(Type::IdOfT<RET_ARGS>::kTypeId))... };
    init(ccId, vaIndex, ret_args[0], ret_args + 1, uint32_t(ASMJIT_ARRAY_SIZE(ret_args) - 1));
  }
};

// ============================================================================
// [asmjit::FuncSignatureBuilder]
// ============================================================================

//! Function signature builder.
class FuncSignatureBuilder : public FuncSignature {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline FuncSignatureBuilder(uint32_t ccId = CallConv::kIdHost, uint32_t vaIndex = kNoVarArgs) noexcept {
    init(ccId, vaIndex, Type::kIdVoid, _builderArgList, 0);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Sets the return type to `retType`.
  inline void setRet(uint32_t retType) noexcept { _ret = uint8_t(retType); }
  //! Sets the return type based on `T`.
  template<typename T>
  inline void setRetT() noexcept { setRet(Type::IdOfT<T>::kTypeId); }

  //! Sets the argument at index `index` to `argType`.
  inline void setArg(uint32_t index, uint32_t argType) noexcept {
    ASMJIT_ASSERT(index < _argCount);
    _builderArgList[index] = uint8_t(argType);
  }
  //! Sets the argument at index `i` to the type based on `T`.
  template<typename T>
  inline void setArgT(uint32_t index) noexcept { setArg(index, Type::IdOfT<T>::kTypeId); }

  //! Appends an argument of `type` to the function prototype.
  inline void addArg(uint32_t type) noexcept {
    ASMJIT_ASSERT(_argCount < kFuncArgCount);
    _builderArgList[_argCount++] = uint8_t(type);
  }
  //! Appends an argument of type based on `T` to the function prototype.
  template<typename T>
  inline void addArgT() noexcept { addArg(Type::IdOfT<T>::kTypeId); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _builderArgList[kFuncArgCount];
};

// ============================================================================
// [asmjit::FuncValue]
// ============================================================================

//! Argument or return value as defined by `FuncSignature`, but with register
//! or stack address (and other metadata) assigned to it.
struct FuncValue {
  enum Parts : uint32_t {
    kTypeIdShift      = 0,             //!< TypeId shift.
    kTypeIdMask       = 0x000000FFu,   //!< TypeId mask.

    kFlagIsReg        = 0x00000100u,   //!< Passed by register.
    kFlagIsStack      = 0x00000200u,   //!< Passed by stack.
    kFlagIsIndirect   = 0x00000400u,   //!< Passed indirectly by reference (internally a pointer).
    kFlagIsDone       = 0x00000800u,   //!< Used internally by arguments allocator.

    kStackOffsetShift = 12,            //!< Stack offset shift.
    kStackOffsetMask  = 0xFFFFF000u,   //!< Stack offset mask (must occupy MSB bits).

    kRegIdShift       = 16,            //!< RegId shift.
    kRegIdMask        = 0x00FF0000u,   //!< RegId mask.

    kRegTypeShift     = 24,            //!< RegType shift.
    kRegTypeMask      = 0xFF000000u    //!< RegType mask.
  };

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  // These initialize the whole `FuncValue` to either register or stack. Useful
  // when you know all of these properties and wanna just set it up.

  //! Initializes the `typeId` of this `FuncValue`.
  inline void initTypeId(uint32_t typeId) noexcept {
    _data = typeId << kTypeIdShift;
  }

  inline void initReg(uint32_t regType, uint32_t regId, uint32_t typeId, uint32_t flags = 0) noexcept {
    _data = (regType << kRegTypeShift) | (regId << kRegIdShift) | (typeId << kTypeIdShift) | kFlagIsReg | flags;
  }

  inline void initStack(int32_t offset, uint32_t typeId) noexcept {
    _data = (uint32_t(offset) << kStackOffsetShift) | (typeId << kTypeIdShift) | kFlagIsStack;
  }

  //! Resets the value to its unassigned state.
  inline void reset() noexcept { _data = 0; }

  // --------------------------------------------------------------------------
  // [Assign]
  // --------------------------------------------------------------------------

  // These initialize only part of `FuncValue`, useful when building `FuncValue`
  // incrementally. The caller should first init the type-id by caliing `initTypeId`
  // and then continue building either register or stack.

  inline void assignRegData(uint32_t regType, uint32_t regId) noexcept {
    ASMJIT_ASSERT((_data & (kRegTypeMask | kRegIdMask)) == 0);
    _data |= (regType << kRegTypeShift) | (regId << kRegIdShift) | kFlagIsReg;
  }

  inline void assignStackOffset(int32_t offset) noexcept {
    ASMJIT_ASSERT((_data & kStackOffsetMask) == 0);
    _data |= (uint32_t(offset) << kStackOffsetShift) | kFlagIsStack;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline void _replaceValue(uint32_t mask, uint32_t value) noexcept { _data = (_data & ~mask) | value; }

  //! Gets whether the `FuncValue` has a flag `flag` set.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_data & flag) != 0; }
  //! Adds `flags` to `FuncValue`.
  inline void addFlags(uint32_t flags) noexcept { _data |= flags; }
  //! Clears `flags` of `FuncValue`.
  inline void clearFlags(uint32_t flags) noexcept { _data &= ~flags; }

  //! Gets whether this value is initialized (i.e. contains a valid data).
  inline bool isInitialized() const noexcept { return _data != 0; }
  //! Gets whether this argument is passed by register.
  inline bool isReg() const noexcept { return hasFlag(kFlagIsReg); }
  //! Gets whether this argument is passed by stack.
  inline bool isStack() const noexcept { return hasFlag(kFlagIsStack); }
  //! Gets whether this argument is passed by register.
  inline bool isAssigned() const noexcept { return hasFlag(kFlagIsReg | kFlagIsStack); }
  //! Gets whether this argument is passed through a pointer (used by WIN64 to pass XMM|YMM|ZMM).
  inline bool isIndirect() const noexcept { return hasFlag(kFlagIsIndirect); }

  //! Gets whether the argument was already processed (used internally).
  inline bool isDone() const noexcept { return hasFlag(kFlagIsDone); }

  //! Gets a register type of the register used to pass function argument or return value.
  inline uint32_t regType() const noexcept { return (_data & kRegTypeMask) >> kRegTypeShift; }
  //! Sets a register type of the register used to pass function argument or return value.
  inline void setRegType(uint32_t regType) noexcept { _replaceValue(kRegTypeMask, regType << kRegTypeShift); }

  //! Gets a physical id of the register used to pass function argument or return value.
  inline uint32_t regId() const noexcept { return (_data & kRegIdMask) >> kRegIdShift; }
  //! Sets a physical id of the register used to pass function argument or return value.
  inline void setRegId(uint32_t regId) noexcept { _replaceValue(kRegIdMask, regId << kRegIdShift); }

  //! Gets a stack offset of this argument.
  inline int32_t stackOffset() const noexcept { return int32_t(_data & kStackOffsetMask) >> kStackOffsetShift; }
  //! Sets a stack offset of this argument.
  inline void setStackOffset(int32_t offset) noexcept { _replaceValue(kStackOffsetMask, uint32_t(offset) << kStackOffsetShift); }

  //! Gets a TypeId of this argument or return value.
  inline bool hasTypeId() const noexcept { return (_data & kTypeIdMask) != 0; }
  //! Gets a TypeId of this argument or return value.
  inline uint32_t typeId() const noexcept { return (_data & kTypeIdMask) >> kTypeIdShift; }
  //! Sets a TypeId of this argument or return value.
  inline void setTypeId(uint32_t typeId) noexcept { _replaceValue(kTypeIdMask, typeId << kTypeIdShift); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _data;
};

// ============================================================================
// [asmjit::FuncDetail]
// ============================================================================

//! Function detail - CallConv and expanded FuncSignature.
//!
//! Function detail is architecture and OS dependent representation of a function.
//! It contains calling convention and expanded function signature so all
//! arguments have assigned either register type & id or stack address.
class FuncDetail {
public:
  enum : uint8_t {
    //! Doesn't have variable number of arguments (`...`).
    kNoVarArgs = 0xFF
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline FuncDetail() noexcept { reset(); }
  inline FuncDetail(const FuncDetail& other) noexcept = default;

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Initializes this `FuncDetail` to the given signature.
  ASMJIT_API Error init(const FuncSignature& sign);
  inline void reset() noexcept { ::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Accessors - Calling Convention]
  // --------------------------------------------------------------------------

  //! Gets the function's calling convention, see `CallConv`.
  inline const CallConv& callConv() const noexcept { return _callConv; }

  //! Gets CallConv flags, see `CallConv::Flags`.
  inline uint32_t flags() const noexcept { return _callConv.flags(); }
  //! Checks whether a CallConv `flag` is set, see `CallConv::Flags`.
  inline bool hasFlag(uint32_t ccFlag) const noexcept { return _callConv.hasFlag(ccFlag); }

  // --------------------------------------------------------------------------
  // [Accessors - Arguments and Return]
  // --------------------------------------------------------------------------

  //! Gets count of function return values.
  inline uint32_t retCount() const noexcept { return _retCount; }
  //! Gets the number of function arguments.
  inline uint32_t argCount() const noexcept { return _argCount; }

  //! Gets whether the function has a return value.
  inline bool hasRet() const noexcept { return _retCount != 0; }
  //! Gets function return value.
  inline FuncValue& ret(uint32_t index = 0) noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_rets));
    return _rets[index];
  }
  //! Gets function return value (const).
  inline const FuncValue& ret(uint32_t index = 0) const noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_rets));
    return _rets[index];
  }

  //! Gets function arguments array.
  inline FuncValue* args() noexcept { return _args; }
  //! Gets function arguments array (const).
  inline const FuncValue* args() const noexcept { return _args; }

  inline bool hasArg(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index].isInitialized();
  }

  //! Gets function argument at index `index`.
  inline FuncValue& arg(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index];
  }

  //! Gets function argument at index `index`.
  inline const FuncValue& arg(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index];
  }

  inline void resetArg(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    _args[index].reset();
  }

  inline bool hasVarArgs() const noexcept { return _vaIndex != kNoVarArgs; }
  inline uint32_t vaIndex() const noexcept { return _vaIndex; }

  //! Gets whether the function passes one or more argument by stack.
  inline bool hasStackArgs() const noexcept { return _argStackSize != 0; }
  //! Gets stack size needed for function arguments passed on the stack.
  inline uint32_t argStackSize() const noexcept { return _argStackSize; }

  inline uint32_t redZoneSize() const noexcept { return _callConv.redZoneSize(); }
  inline uint32_t spillZoneSize() const noexcept { return _callConv.spillZoneSize(); }
  inline uint32_t naturalStackAlignment() const noexcept { return _callConv.naturalStackAlignment(); }

  inline uint32_t passedRegs(uint32_t group) const noexcept { return _callConv.passedRegs(group); }
  inline uint32_t preservedRegs(uint32_t group) const noexcept { return _callConv.preservedRegs(group); }

  inline uint32_t usedRegs(uint32_t group) const noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    return _usedRegs[group];
  }

  inline void addUsedRegs(uint32_t group, uint32_t regs) noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    _usedRegs[group] |= regs;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  CallConv _callConv;                    //!< Calling convention.
  uint8_t _argCount;                     //!< Number of function arguments.
  uint8_t _retCount;                     //!< Number of function return values.
  uint8_t _vaIndex;                      //!< Variable arguments index of `kNoVarArgs`.
  uint8_t _reserved;                     //!< Reserved.
  uint32_t _usedRegs[BaseReg::kGroupVirt];//!< Registers that contains arguments.
  uint32_t _argStackSize;                //!< Size of arguments passed by stack.
  FuncValue _rets[2];                    //!< Function return values.
  FuncValue _args[kFuncArgCountLoHi];    //!< Function arguments.
};

// ============================================================================
// [asmjit::FuncFrame]
// ============================================================================

//! Function frame.
//!
//! Function frame is used directly by prolog and epilog insertion (PEI) utils.
//! It provides information necessary to insert a proper and ABI comforming
//! prolog and epilog. Function frame calculation is based on `CallConv` and
//! other function attributes.
//!
//! Function Frame Structure
//! ------------------------
//!
//! Various properties can contribute to the size and structure of the function
//! frame. The function frame in most cases won't use all of the properties
//! illustrated (for example Spill Zone and Red Zone are never used together).
//!
//!   +-----------------------------+
//!   | Arguments Passed by Stack   |
//!   +-----------------------------+
//!   | Spill Zone                  |
//!   +-----------------------------+ <- Stack offset (args) starts from here.
//!   | Return Address if Pushed    |
//!   +-----------------------------+ <- Stack pointer (SP) upon entry.
//!   | Save/Restore Stack.         |
//!   +-----------------------------+-----------------------------+
//!   | Local Stack                 |                             |
//!   +-----------------------------+          Final Stack        |
//!   | Call Stack                  |                             |
//!   +-----------------------------+-----------------------------+ <- SP after prolog.
//!   | Red Zone                    |
//!   +-----------------------------+
class FuncFrame {
public:
  enum Group : uint32_t {
    kGroupVirt = BaseReg::kGroupVirt
  };

  enum Tag : uint32_t {
    kTagInvalidOffset     = 0xFFFFFFFFu  //!< Tag used to inform that some offset is invalid.
  };

  //! Attributes are designed in a way that all are initially false, and user
  //! or FuncFrame finalizer adds them when necessary.
  enum Attributes : uint32_t {
    kAttrHasVarArgs       = 0x00000001u, //!< Function has variable number of arguments.
    kAttrHasPreservedFP   = 0x00000010u, //!< Preserve frame pointer (don't omit FP).
    kAttrHasFuncCalls     = 0x00000020u, //!< Function calls other functions (is not leaf).

    kAttrX86AvxEnabled    = 0x00010000u, //!< Use AVX instead of SSE for all operations (X86).
    kAttrX86AvxCleanup    = 0x00020000u, //!< Emit VZEROUPPER instruction in epilog (X86).
    kAttrX86MmxCleanup    = 0x00040000u, //!< Emit EMMS instruction in epilog (X86).

    kAttrAlignedVecSR     = 0x40000000u, //!< Function has aligned save/restore of vector registers.
    kAttrIsFinalized      = 0x80000000u  //!< FuncFrame is finalized and can be used by PEI.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline FuncFrame() noexcept { reset(); }
  inline FuncFrame(const FuncFrame& other) noexcept = default;

  // --------------------------------------------------------------------------
  // [Init / Reset / Finalize]
  // --------------------------------------------------------------------------

  ASMJIT_API Error init(const FuncDetail& func) noexcept;
  ASMJIT_API Error finalize() noexcept;

  inline void reset() noexcept {
    ::memset(this, 0, sizeof(FuncFrame));
    _spRegId = BaseReg::kIdBad;
    _saRegId = BaseReg::kIdBad;
    _daOffset = kTagInvalidOffset;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets the target architecture of the function frame.
  inline uint32_t archId() const noexcept { return _archId; }

  //! Gets FuncFrame attributes, see `Attributes`.
  inline uint32_t attributes() const noexcept { return _attributes; }
  //! Checks whether the FuncFame contains an attribute `attr`.
  inline bool hasAttribute(uint32_t attr) const noexcept { return (_attributes & attr) != 0; }
  //! Adds attributes `attrs` to the FuncFrame.
  inline void addAttributes(uint32_t attrs) noexcept { _attributes |= attrs; }
  //! Clears attributes `attrs` from the FrameFrame.
  inline void clearAttributes(uint32_t attrs) noexcept { _attributes &= ~attrs; }

  //! Gets whether the function has variable number of arguments.
  inline bool hasVarArgs() const noexcept { return hasAttribute(kAttrHasVarArgs); }
  //! Sets the variable arguments flag.
  inline void setVarArgs() noexcept { addAttributes(kAttrHasVarArgs); }
  //! Resets variable arguments flag.
  inline void resetVarArgs() noexcept { clearAttributes(kAttrHasVarArgs); }

  //! Gets whether the function preserves frame pointer (EBP|ESP on X86).
  inline bool hasPreservedFP() const noexcept { return hasAttribute(kAttrHasPreservedFP); }
  //! Enables preserved frame pointer.
  inline void setPreservedFP() noexcept { addAttributes(kAttrHasPreservedFP); }
  //! Disables preserved frame pointer.
  inline void resetPreservedFP() noexcept { clearAttributes(kAttrHasPreservedFP); }

  //! Gets whether the function calls other functions.
  inline bool hasFuncCalls() const noexcept { return hasAttribute(kAttrHasFuncCalls); }
  //! Sets `kFlagHasCalls` to true.
  inline void setFuncCalls() noexcept { addAttributes(kAttrHasFuncCalls); }
  //! Sets `kFlagHasCalls` to false.
  inline void resetFuncCalls() noexcept { clearAttributes(kAttrHasFuncCalls); }

  //! Gets whether the function contains AVX cleanup - 'vzeroupper' instruction in epilog.
  inline bool hasAvxCleanup() const noexcept { return hasAttribute(kAttrX86AvxCleanup); }
  //! Enables AVX cleanup.
  inline void setAvxCleanup() noexcept { addAttributes(kAttrX86AvxCleanup); }
  //! Disables AVX cleanup.
  inline void resetAvxCleanup() noexcept { clearAttributes(kAttrX86AvxCleanup); }

  //! Gets whether the function contains AVX cleanup - 'vzeroupper' instruction in epilog.
  inline bool isAvxEnabled() const noexcept { return hasAttribute(kAttrX86AvxEnabled); }
  //! Enables AVX cleanup.
  inline void setAvxEnabled() noexcept { addAttributes(kAttrX86AvxEnabled); }
  //! Disables AVX cleanup.
  inline void resetAvxEnabled() noexcept { clearAttributes(kAttrX86AvxEnabled); }

  //! Gets whether the function contains MMX cleanup - 'emms' instruction in epilog.
  inline bool hasMmxCleanup() const noexcept { return hasAttribute(kAttrX86MmxCleanup); }
  //! Enables MMX cleanup.
  inline void setMmxCleanup() noexcept { addAttributes(kAttrX86MmxCleanup); }
  //! Disables MMX cleanup.
  inline void resetMmxCleanup() noexcept { clearAttributes(kAttrX86MmxCleanup); }

  //! Gets whether the function uses call stack.
  inline bool hasCallStack() const noexcept { return _callStackSize != 0; }
  //! Gets whether the function uses local stack.
  inline bool hasLocalStack() const noexcept { return _localStackSize != 0; }
  //! Gets whether vector registers can be saved and restored by using aligned reads and writes.
  inline bool hasAlignedVecSR() const noexcept { return hasAttribute(kAttrAlignedVecSR); }
  //! Gets whether the function has to align stack dynamically.
  inline bool hasDynamicAlignment() const noexcept { return _finalStackAlignment >= _minDynamicAlignment; }

  //! Gets whether this calling convention specifies 'RedZone'.
  inline bool hasRedZone() const noexcept { return _redZoneSize != 0; }
  //! Gets whether this calling convention specifies 'SpillZone'.
  inline bool hasSpillZone() const noexcept { return _spillZoneSize != 0; }

  //! Gets size of 'RedZone'.
  inline uint32_t redZoneSize() const noexcept { return _redZoneSize; }
  //! Gets size of 'SpillZone'.
  inline uint32_t spillZoneSize() const noexcept { return _spillZoneSize; }
  //! Gets natural stack alignment (guaranteed stack alignment upon entry).
  inline uint32_t naturalStackAlignment() const noexcept { return _naturalStackAlignment; }
  //! Gets natural stack alignment (guaranteed stack alignment upon entry).
  inline uint32_t minDynamicAlignment() const noexcept { return _minDynamicAlignment; }

  //! Gets whether the callee must adjust SP before returning (X86-STDCALL only)
  inline bool hasCalleeStackCleanup() const noexcept { return _calleeStackCleanup != 0; }
  //! Gets home many bytes of the stack the the callee must adjust before returning (X86-STDCALL only)
  inline uint32_t calleeStackCleanup() const noexcept { return _calleeStackCleanup; }

  //! Gets call stack alignment.
  inline uint32_t callStackAlignment() const noexcept { return _callStackAlignment; }
  //! Gets local stack alignment.
  inline uint32_t localStackAlignment() const noexcept { return _localStackAlignment; }
  //! Gets final stack alignment (the maximum value of call, local, and natural stack alignments).
  inline uint32_t finalStackAlignment() const noexcept { return _finalStackAlignment; }

  //! Sets call stack alignment.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void setCallStackAlignment(uint32_t alignment) noexcept {
    _callStackAlignment = uint8_t(alignment);
    _finalStackAlignment = Support::max(_naturalStackAlignment, _callStackAlignment, _localStackAlignment);
  }

  //! Sets local stack alignment.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void setLocalStackAlignment(uint32_t value) noexcept {
    _localStackAlignment = uint8_t(value);
    _finalStackAlignment = Support::max(_naturalStackAlignment, _callStackAlignment, _localStackAlignment);
  }

  //! Combines call stack alignment with `alignment`, updating it to the greater value.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void updateCallStackAlignment(uint32_t alignment) noexcept {
    _callStackAlignment = uint8_t(Support::max<uint32_t>(_callStackAlignment, alignment));
    _finalStackAlignment = Support::max(_finalStackAlignment, _callStackAlignment);
  }

  //! Combines local stack alignment with `alignment`, updating it to the greater value.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void updateLocalStackAlignment(uint32_t alignment) noexcept {
    _localStackAlignment = uint8_t(Support::max<uint32_t>(_localStackAlignment, alignment));
    _finalStackAlignment = Support::max(_finalStackAlignment, _localStackAlignment);
  }

  //! Gets call stack size.
  inline uint32_t callStackSize() const noexcept { return _callStackSize; }
  //! Gets local stack size.
  inline uint32_t localStackSize() const noexcept { return _localStackSize; }

  //! Sets call stack size.
  inline void setCallStackSize(uint32_t size) noexcept { _callStackSize = size; }
  //! Sets local stack size.
  inline void setLocalStackSize(uint32_t size) noexcept { _localStackSize = size; }

  //! Combines call stack size with `size`, updating it to the greater value.
  inline void updateCallStackSize(uint32_t size) noexcept { _callStackSize = Support::max(_callStackSize, size); }
  //! Combines local stack size with `size`, updating it to the greater value.
  inline void updateLocalStackSize(uint32_t size) noexcept { _localStackSize = Support::max(_localStackSize, size); }

  //! Gets final stack size (only valid after the FuncFrame is finalized).
  inline uint32_t finalStackSize() const noexcept { return _finalStackSize; }

  //! Gets an offset to access the local stack (non-zero only if call stack is used).
  inline uint32_t localStackOffset() const noexcept { return _localStackOffset; }

  //! Gets whether the function prolog/epilog requires a memory slot for storing unaligned SP.
  inline bool hasDAOffset() const noexcept { return _daOffset != kTagInvalidOffset; }
  //! Gets a memory offset used to store DA (dynamic alignment) slot (relative to SP).
  inline uint32_t daOffset() const noexcept { return _daOffset; }

  inline uint32_t saOffset(uint32_t regId) const noexcept {
    return regId == _spRegId ? saOffsetFromSP()
                             : saOffsetFromSA();
  }

  inline uint32_t saOffsetFromSP() const noexcept { return _saOffsetFromSP; }
  inline uint32_t saOffsetFromSA() const noexcept { return _saOffsetFromSA; }

  //! Gets which registers (by `group`) are saved/restored in prolog/epilog, respectively.
  inline uint32_t dirtyRegs(uint32_t group) const noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    return _dirtyRegs[group];
  }

  //! Sets which registers (by `group`) are saved/restored in prolog/epilog, respectively.
  inline void setDirtyRegs(uint32_t group, uint32_t regs) noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    _dirtyRegs[group] = regs;
  }

  //! Adds registers (by `group`) to saved/restored registers.
  inline void addDirtyRegs(uint32_t group, uint32_t regs) noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    _dirtyRegs[group] |= regs;
  }

  inline void setAllDirty() noexcept {
    _dirtyRegs[0] = 0xFFFFFFFFu;
    _dirtyRegs[1] = 0xFFFFFFFFu;
    _dirtyRegs[2] = 0xFFFFFFFFu;
    _dirtyRegs[3] = 0xFFFFFFFFu;
  }

  inline void setAllDirty(uint32_t group) noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    _dirtyRegs[group] = 0xFFFFFFFFu;
  }

  inline uint32_t savedRegs(uint32_t group) const noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    return _dirtyRegs[group] & _preservedRegs[group];
  }

  inline uint32_t preservedRegs(uint32_t group) const noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    return _preservedRegs[group];
  }

  inline bool hasSARegId() const noexcept { return _saRegId != BaseReg::kIdBad; }
  inline uint32_t saRegId() const noexcept { return _saRegId; }
  inline void setSARegId(uint32_t regId) { _saRegId = uint8_t(regId); }
  inline void resetSARegId() { setSARegId(BaseReg::kIdBad); }

  //! Gets stack size required to save GP registers.
  inline uint32_t gpSaveSize() const noexcept { return _gpSaveSize; }
  //! Gets stack size required to save other than GP registers (MM, XMM|YMM|ZMM, K, VFP, etc...).
  inline uint32_t nonGpSaveSize() const noexcept { return _nonGpSaveSize; }

  inline uint32_t gpSaveOffset() const noexcept { return _gpSaveOffset; }
  inline uint32_t nonGpSaveOffset() const noexcept { return _nonGpSaveOffset; }

  inline bool hasStackAdjustment() const noexcept { return _stackAdjustment != 0; }
  inline uint32_t stackAdjustment() const noexcept { return _stackAdjustment; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint32_t _attributes;                  //!< Function attributes.

  uint8_t _archId;                       //!< Architecture ID.
  uint8_t _spRegId;                      //!< SP register ID (to access call stack and local stack).
  uint8_t _saRegId;                      //!< SA register ID (to access stack arguments).

  uint8_t _redZoneSize;                  //!< Red zone size (copied from CallConv).
  uint8_t _spillZoneSize;                //!< Spill zone size (copied from CallConv).
  uint8_t _naturalStackAlignment;        //!< Natural stack alignment (copied from CallConv).
  uint8_t _minDynamicAlignment;          //!< Minimum stack alignment to turn on dynamic alignment.

  uint8_t _callStackAlignment;           //!< Call stack alignment.
  uint8_t _localStackAlignment;          //!< Local stack alignment.
  uint8_t _finalStackAlignment;          //!< Final stack alignment.

  uint16_t _calleeStackCleanup;          //!< Adjustment of the stack before returning (X86-STDCALL).

  uint32_t _callStackSize;               //!< Call stack size.
  uint32_t _localStackSize;              //!< Local stack size.
  uint32_t _finalStackSize;              //!< Final stack size (sum of call stack and local stack).

  uint32_t _localStackOffset;            //!< Local stack offset (non-zero only if call stack is used).
  uint32_t _daOffset;                    //!< Offset relative to SP that contains previous SP (before alignment).
  uint32_t _saOffsetFromSP;              //!< Offset of the first stack argument relative to SP.
  uint32_t _saOffsetFromSA;              //!< Offset of the first stack argument relative to SA (_saRegId or FP).

  uint32_t _stackAdjustment;             //!< Local stack adjustment in prolog/epilog.

  uint32_t _dirtyRegs[BaseReg::kGroupVirt];     //!< Registers that are dirty.
  uint32_t _preservedRegs[BaseReg::kGroupVirt]; //!< Registers that must be preserved (copied from CallConv).

  uint16_t _gpSaveSize;                  //!< Final stack size required to save GP regs.
  uint16_t _nonGpSaveSize;               //!< Final Stack size required to save other than GP regs.
  uint32_t _gpSaveOffset;                //!< Final offset where saved GP regs are stored.
  uint32_t _nonGpSaveOffset;             //!< Final offset where saved other than GP regs are stored.
};

// ============================================================================
// [asmjit::FuncArgsAssignment]
// ============================================================================

//! A helper class that can be used to assign a physical register for each
//! function argument. Use with `BaseEmitter::emitArgsAssignment()`.
class FuncArgsAssignment {
public:
  enum {
    kArgCount = kFuncArgCountLoHi
  };

  inline explicit FuncArgsAssignment(const FuncDetail* fd = nullptr) noexcept { reset(fd); }

  inline FuncArgsAssignment(const FuncArgsAssignment& other) noexcept {
    ::memcpy(this, &other, sizeof(*this));
  }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline void reset(const FuncDetail* fd = nullptr) noexcept {
    _funcDetail = fd;
    _saRegId = uint8_t(BaseReg::kIdBad);
    ::memset(_reserved, 0, sizeof(_reserved));
    ::memset(_args, 0, sizeof(_args));
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline const FuncDetail* funcDetail() const noexcept { return _funcDetail; }
  inline void setFuncDetail(const FuncDetail* fd) noexcept { _funcDetail = fd; }

  inline bool hasSARegId() const noexcept { return _saRegId != BaseReg::kIdBad; }
  inline uint32_t saRegId() const noexcept { return _saRegId; }
  inline void setSARegId(uint32_t regId) { _saRegId = uint8_t(regId); }
  inline void resetSARegId() { _saRegId = uint8_t(BaseReg::kIdBad); }

  inline FuncValue& arg(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_args));
    return _args[index];
  }
  inline const FuncValue& arg(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_args));
    return _args[index];
  }

  inline bool isAssigned(uint32_t argIndex) const noexcept {
    ASMJIT_ASSERT(argIndex < ASMJIT_ARRAY_SIZE(_args));
    return _args[argIndex].isAssigned();
  }

  inline void assignReg(uint32_t argIndex, const BaseReg& reg, uint32_t typeId = Type::kIdVoid) noexcept {
    ASMJIT_ASSERT(argIndex < ASMJIT_ARRAY_SIZE(_args));
    ASMJIT_ASSERT(reg.isPhysReg());
    _args[argIndex].initReg(reg.type(), reg.id(), typeId);
  }

  inline void assignReg(uint32_t argIndex, uint32_t regType, uint32_t regId, uint32_t typeId = Type::kIdVoid) noexcept {
    ASMJIT_ASSERT(argIndex < ASMJIT_ARRAY_SIZE(_args));
    _args[argIndex].initReg(regType, regId, typeId);
  }

  inline void assignStack(uint32_t argIndex, int32_t offset, uint32_t typeId = Type::kIdVoid) {
    ASMJIT_ASSERT(argIndex < ASMJIT_ARRAY_SIZE(_args));
    _args[argIndex].initStack(offset, typeId);
  }

  // NOTE: All `assignAll()` methods are shortcuts to assign all arguments at
  // once, however, since registers are passed all at once these initializers
  // don't provide any way to pass TypeId and/or to keep any argument between
  // the arguments passed unassigned.
  inline void assignAll(const BaseReg& a0) noexcept {
    assignReg(0, a0);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1, const BaseReg& a2) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
    assignReg(2, a2);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1, const BaseReg& a2, const BaseReg& a3) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
    assignReg(2, a2);
    assignReg(3, a3);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1, const BaseReg& a2, const BaseReg& a3, const BaseReg& a4) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
    assignReg(2, a2);
    assignReg(3, a3);
    assignReg(4, a4);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1, const BaseReg& a2, const BaseReg& a3, const BaseReg& a4, const BaseReg& a5) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
    assignReg(2, a2);
    assignReg(3, a3);
    assignReg(4, a4);
    assignReg(5, a5);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1, const BaseReg& a2, const BaseReg& a3, const BaseReg& a4, const BaseReg& a5, const BaseReg& a6) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
    assignReg(2, a2);
    assignReg(3, a3);
    assignReg(4, a4);
    assignReg(5, a5);
    assignReg(6, a6);
  }

  inline void assignAll(const BaseReg& a0, const BaseReg& a1, const BaseReg& a2, const BaseReg& a3, const BaseReg& a4, const BaseReg& a5, const BaseReg& a6, const BaseReg& a7) noexcept {
    assignReg(0, a0);
    assignReg(1, a1);
    assignReg(2, a2);
    assignReg(3, a3);
    assignReg(4, a4);
    assignReg(5, a5);
    assignReg(6, a6);
    assignReg(7, a7);
  }

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  //! Update `FuncFrame` based on function's arguments assignment.
  //!
  //! NOTE: You MUST call this in orher to use `BaseEmitter::emitArgsAssignment()`,
  //! otherwise the FuncFrame would not contain the information necessary to
  //! assign all arguments into the registers and/or stack specified.
  ASMJIT_API Error updateFuncFrame(FuncFrame& frame) const noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  const FuncDetail* _funcDetail;         //!< Function detail.
  uint8_t _saRegId;                      //!< Register that can be used to access arguments passed by stack.
  uint8_t _reserved[3];                  //!< \internal
  FuncValue _args[kArgCount];            //!< Mapping of each function argument.
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_FUNC_H
