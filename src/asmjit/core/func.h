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
  enum {
    //! Doesn't have variable number of arguments (`...`).
    kNoVarArgs = 0xFF
  };

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Initialize the function signature.
  inline void init(uint32_t ccId, uint32_t ret, const uint8_t* args, uint32_t argCount) noexcept {
    ASMJIT_ASSERT(ccId <= 0xFF);
    ASMJIT_ASSERT(argCount <= 0xFF);

    _callConv = uint8_t(ccId);
    _argCount = uint8_t(argCount);
    _vaIndex = kNoVarArgs;
    _ret = uint8_t(ret);
    _args = args;
  }

  inline void reset() noexcept { std::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the function's calling convention.
  inline uint32_t callConv() const noexcept { return _callConv; }

  //! Get whether the function has variable number of arguments (...).
  inline bool hasVarArgs() const noexcept { return _vaIndex != kNoVarArgs; }
  //! Get the variable arguments (...) index, `kNoVarArgs` if none.
  inline uint32_t vaIndex() const noexcept { return _vaIndex; }

  //! Get the number of function arguments.
  inline uint32_t argCount() const noexcept { return _argCount; }

  inline bool hasRet() const noexcept { return _ret != Type::kIdVoid; }
  //! Get the return value type.
  inline uint32_t ret() const noexcept { return _ret; }

  //! Get the type of the argument at index `i`.
  inline uint32_t arg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _argCount);
    return _args[i];
  }
  //! Get the array of function arguments' types.
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
  inline FuncSignatureT(uint32_t ccId = CallConv::kIdHost) noexcept {
    static const uint8_t ret_args[] = { (uint8_t(Type::IdOfT<RET_ARGS>::kTypeId))... };
    init(ccId, ret_args[0], ret_args + 1, uint32_t(ASMJIT_ARRAY_SIZE(ret_args) - 1));
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

  inline FuncSignatureBuilder(uint32_t ccId = CallConv::kIdHost) noexcept {
    init(ccId, Type::kIdVoid, _builderArgList, 0);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline void setCallConv(uint32_t ccId) noexcept { _callConv = uint8_t(ccId); }

  //! Set the return type to `retType`.
  inline void setRet(uint32_t retType) noexcept { _ret = uint8_t(retType); }
  //! Set the return type based on `T`.
  template<typename T>
  inline void setRetT() noexcept { setRet(Type::IdOfT<T>::kTypeId); }

  //! Set the argument at index `index` to `argType`.
  inline void setArg(uint32_t index, uint32_t argType) noexcept {
    ASMJIT_ASSERT(index < _argCount);
    _builderArgList[index] = uint8_t(argType);
  }
  //! Set the argument at index `i` to the type based on `T`.
  template<typename T>
  inline void setArgT(uint32_t index) noexcept { setArg(index, Type::IdOfT<T>::kTypeId); }

  //! Append an argument of `type` to the function prototype.
  inline void addArg(uint32_t type) noexcept {
    ASMJIT_ASSERT(_argCount < kFuncArgCount);
    _builderArgList[_argCount++] = uint8_t(type);
  }
  //! Append an argument of type based on `T` to the function prototype.
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

  //! Initialize this in/out by a given `typeId`.
  inline void initTypeId(uint32_t typeId) noexcept {
    _data = typeId << kTypeIdShift;
  }

  inline void initReg(uint32_t regType, uint32_t regId, uint32_t typeId, uint32_t flags = 0) noexcept {
    _data = (regType << kRegTypeShift) | (regId << kRegIdShift) | (typeId << kTypeIdShift) | kFlagIsReg | flags;
  }

  inline void initStack(int32_t offset, uint32_t typeId) noexcept {
    _data = (uint32_t(offset) << kStackOffsetShift) | (typeId << kTypeIdShift) | kFlagIsStack;
  }

  //! Reset the value to its unassigned state.
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

  //! Get whether the `FuncValue` has a flag `flag` set.
  inline bool hasFlag(uint32_t flag) const noexcept { return (_data & flag) != 0; }
  //! Add `flags` to `FuncValue`.
  inline void addFlags(uint32_t flags) noexcept { _data |= flags; }
  //! Clear `flags` of `FuncValue`.
  inline void clearFlags(uint32_t flags) noexcept { _data &= ~flags; }

  //! Get whether this value is initialized (i.e. contains a valid data).
  inline bool isInitialized() const noexcept { return _data != 0; }
  //! Get whether this argument is passed by register.
  inline bool isReg() const noexcept { return hasFlag(kFlagIsReg); }
  //! Get whether this argument is passed by stack.
  inline bool isStack() const noexcept { return hasFlag(kFlagIsStack); }
  //! Get whether this argument is passed by register.
  inline bool isAssigned() const noexcept { return hasFlag(kFlagIsReg | kFlagIsStack); }
  //! Get whether this argument is passed through a pointer (used by WIN64 to pass XMM|YMM|ZMM).
  inline bool isIndirect() const noexcept { return hasFlag(kFlagIsIndirect); }

  inline bool isDone() const noexcept { return hasFlag(kFlagIsDone); }

  //! Get a register type of the register used to pass function argument or return value.
  inline uint32_t regType() const noexcept { return (_data & kRegTypeMask) >> kRegTypeShift; }
  //! Set a register type of the register used to pass function argument or return value.
  inline void setRegType(uint32_t regType) noexcept { _replaceValue(kRegTypeMask, regType << kRegTypeShift); }

  //! Get a physical id of the register used to pass function argument or return value.
  inline uint32_t regId() const noexcept { return (_data & kRegIdMask) >> kRegIdShift; }
  //! Set a physical id of the register used to pass function argument or return value.
  inline void setRegId(uint32_t regId) noexcept { _replaceValue(kRegIdMask, regId << kRegIdShift); }

  //! Get a stack offset of this argument.
  inline int32_t stackOffset() const noexcept { return int32_t(_data & kStackOffsetMask) >> kStackOffsetShift; }
  //! Set a stack offset of this argument.
  inline void setStackOffset(int32_t offset) noexcept { _replaceValue(kStackOffsetMask, uint32_t(offset) << kStackOffsetShift); }

  //! Get a TypeId of this argument or return value.
  inline bool hasTypeId() const noexcept { return (_data & kTypeIdMask) != 0; }
  //! Get a TypeId of this argument or return value.
  inline uint32_t typeId() const noexcept { return (_data & kTypeIdMask) >> kTypeIdShift; }
  //! Set a TypeId of this argument or return value.
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
//! Function details is architecture and OS dependent representation of function.
//! It contains calling convention and expanded function signature so all
//! arguments have assigned either register type & id or stack address.
class FuncDetail {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline FuncDetail() noexcept { reset(); }
  inline FuncDetail(const FuncDetail& other) noexcept = default;

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  //! Initialize this `FuncDetail` to the given signature.
  ASMJIT_API Error init(const FuncSignature& sign);
  inline void reset() noexcept { std::memset(this, 0, sizeof(*this)); }

  // --------------------------------------------------------------------------
  // [Accessors - Calling Convention]
  // --------------------------------------------------------------------------

  //! Get the function's calling convention, see `CallConv`.
  inline const CallConv& callConv() const noexcept { return _callConv; }

  //! Get CallConv flags, see `CallConv::Flags`.
  inline uint32_t flags() const noexcept { return _callConv.flags(); }
  //! Check if a CallConv `flag` is set, see `CallConv::Flags`.
  inline bool hasFlag(uint32_t ccFlag) const noexcept { return _callConv.hasFlag(ccFlag); }

  // --------------------------------------------------------------------------
  // [Accessors - Arguments and Return]
  // --------------------------------------------------------------------------

  //! Get count of function return values.
  inline uint32_t retCount() const noexcept { return _retCount; }
  //! Get the number of function arguments.
  inline uint32_t argCount() const noexcept { return _argCount; }

  //! Get whether the function has a return value.
  inline bool hasRet() const noexcept { return _retCount != 0; }
  //! Get function return value.
  inline FuncValue& ret(uint32_t index = 0) noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_rets));
    return _rets[index];
  }
  //! Get function return value (const).
  inline const FuncValue& ret(uint32_t index = 0) const noexcept {
    ASMJIT_ASSERT(index < ASMJIT_ARRAY_SIZE(_rets));
    return _rets[index];
  }

  //! Get function arguments array.
  inline FuncValue* args() noexcept { return _args; }
  //! Get function arguments array (const).
  inline const FuncValue* args() const noexcept { return _args; }

  inline bool hasArg(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index].isInitialized();
  }

  //! Get function argument at index `index`.
  inline FuncValue& arg(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index];
  }

  //! Get function argument at index `index`.
  inline const FuncValue& arg(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    return _args[index];
  }

  inline void resetArg(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < kFuncArgCountLoHi);
    _args[index].reset();
  }

  //! Get whether the function passes one or more argument by stack.
  inline bool hasStackArgs() const noexcept { return _argStackSize != 0; }
  //! Get stack size needed for function arguments passed on the stack.
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
  uint16_t _reserved;                    //!< Reserved.
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
//!   +-----------------------------+-----------------------------+
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
    kAttrHasPreservedFP   = 0x00000001u, //!< Preserve frame pointer (don't omit FP).
    kAttrHasFuncCalls     = 0x00000002u, //!< Function calls other functions (is not leaf).

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
    std::memset(this, 0, sizeof(FuncFrame));
    _spRegId = BaseReg::kIdBad;
    _saRegId = BaseReg::kIdBad;
    _daOffset = kTagInvalidOffset;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the target architecture of the function frame.
  inline uint32_t archId() const noexcept { return _archId; }

  //! Get FuncFrame attributes, see `Attributes`.
  inline uint32_t attributes() const noexcept { return _attributes; }
  //! Check if the FuncFame contains an attribute `attr`.
  inline bool hasAttribute(uint32_t attr) const noexcept { return (_attributes & attr) != 0; }
  //! Add attributes `attrs` to the FuncFrame.
  inline void addAttributes(uint32_t attrs) noexcept { _attributes |= attrs; }
  //! Clear attributes `attrs` from the FrameFrame.
  inline void clearAttributes(uint32_t attrs) noexcept { _attributes &= ~attrs; }

  //! Get whether the function preserves frame pointer (EBP|ESP on X86).
  inline bool hasPreservedFP() const noexcept { return hasAttribute(kAttrHasPreservedFP); }
  //! Enable preserved frame pointer.
  inline void setPreservedFP() noexcept { addAttributes(kAttrHasPreservedFP); }
  //! Disable preserved frame pointer.
  inline void resetPreservedFP() noexcept { clearAttributes(kAttrHasPreservedFP); }

  //! Get whether the function calls other functions.
  inline bool hasFuncCalls() const noexcept { return hasAttribute(kAttrHasFuncCalls); }
  //! Set `kFlagHasCalls` to true.
  inline void setFuncCalls() noexcept { addAttributes(kAttrHasFuncCalls); }
  //! Set `kFlagHasCalls` to false.
  inline void resetFuncCalls() noexcept { clearAttributes(kAttrHasFuncCalls); }

  //! Get whether the function contains AVX cleanup - 'vzeroupper' instruction in epilog.
  inline bool hasAvxCleanup() const noexcept { return hasAttribute(kAttrX86AvxCleanup); }
  //! Enable AVX cleanup.
  inline void setAvxCleanup() noexcept { addAttributes(kAttrX86AvxCleanup); }
  //! Disable AVX cleanup.
  inline void resetAvxCleanup() noexcept { clearAttributes(kAttrX86AvxCleanup); }

  //! Get whether the function contains AVX cleanup - 'vzeroupper' instruction in epilog.
  inline bool isAvxEnabled() const noexcept { return hasAttribute(kAttrX86AvxEnabled); }
  //! Enable AVX cleanup.
  inline void setAvxEnabled() noexcept { addAttributes(kAttrX86AvxEnabled); }
  //! Disable AVX cleanup.
  inline void resetAvxEnabled() noexcept { clearAttributes(kAttrX86AvxEnabled); }

  //! Get whether the function contains MMX cleanup - 'emms' instruction in epilog.
  inline bool hasMmxCleanup() const noexcept { return hasAttribute(kAttrX86MmxCleanup); }
  //! Enable MMX cleanup.
  inline void setMmxCleanup() noexcept { addAttributes(kAttrX86MmxCleanup); }
  //! Disable MMX cleanup.
  inline void resetMmxCleanup() noexcept { clearAttributes(kAttrX86MmxCleanup); }

  //! Get whether the function uses call stack.
  inline bool hasCallStack() const noexcept { return _callStackSize != 0; }
  //! Get whether the function uses local stack.
  inline bool hasLocalStack() const noexcept { return _localStackSize != 0; }
  //! Get whether vector registers can be saved and restored by using aligned reads and writes.
  inline bool hasAlignedVecSR() const noexcept { return hasAttribute(kAttrAlignedVecSR); }
  //! Get whether the function has to align stack dynamically.
  inline bool hasDynamicAlignment() const noexcept { return _finalStackAlignment >= _minDynamicAlignment; }

  //! Get whether this calling convention specifies 'RedZone'.
  inline bool hasRedZone() const noexcept { return _redZoneSize != 0; }
  //! Get whether this calling convention specifies 'SpillZone'.
  inline bool hasSpillZone() const noexcept { return _spillZoneSize != 0; }

  //! Get size of 'RedZone'.
  inline uint32_t redZoneSize() const noexcept { return _redZoneSize; }
  //! Get size of 'SpillZone'.
  inline uint32_t spillZoneSize() const noexcept { return _spillZoneSize; }
  //! Get natural stack alignment (guaranteed stack alignment upon entry).
  inline uint32_t naturalStackAlignment() const noexcept { return _naturalStackAlignment; }
  //! Get natural stack alignment (guaranteed stack alignment upon entry).
  inline uint32_t minDynamicAlignment() const noexcept { return _minDynamicAlignment; }

  //! Get whether the callee must adjust SP before returning (X86-STDCALL only)
  inline bool hasCalleeStackCleanup() const noexcept { return _calleeStackCleanup != 0; }
  //! Get home many bytes of the stack the the callee must adjust before returning (X86-STDCALL only)
  inline uint32_t calleeStackCleanup() const noexcept { return _calleeStackCleanup; }

  //! Get call stack alignment.
  inline uint32_t callStackAlignment() const noexcept { return _callStackAlignment; }
  //! Get local stack alignment.
  inline uint32_t localStackAlignment() const noexcept { return _localStackAlignment; }
  //! Get final stack alignment (the maximum value of call, local, and natural stack alignments).
  inline uint32_t finalStackAlignment() const noexcept { return _finalStackAlignment; }

  //! Set call stack alignment.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void setCallStackAlignment(uint32_t alignment) noexcept {
    _callStackAlignment = uint8_t(alignment);
    _finalStackAlignment = std::max(_naturalStackAlignment, std::max(_callStackAlignment, _localStackAlignment));
  }

  //! Set local stack alignment.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void setLocalStackAlignment(uint32_t value) noexcept {
    _localStackAlignment = uint8_t(value);
    _finalStackAlignment = std::max(_naturalStackAlignment, std::max(_callStackAlignment, _localStackAlignment));
  }

  //! Combine call stack alignment with `alignment`, updating it to the greater value.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void updateCallStackAlignment(uint32_t alignment) noexcept {
    _callStackAlignment = uint8_t(std::max<uint32_t>(_callStackAlignment, alignment));
    _finalStackAlignment = std::max(_finalStackAlignment, _callStackAlignment);
  }

  //! Combine local stack alignment with `alignment`, updating it to the greater value.
  //!
  //! NOTE: This also updates the final stack alignment.
  inline void updateLocalStackAlignment(uint32_t alignment) noexcept {
    _localStackAlignment = uint8_t(std::max<uint32_t>(_localStackAlignment, alignment));
    _finalStackAlignment = std::max(_finalStackAlignment, _localStackAlignment);
  }

  //! Get call stack size.
  inline uint32_t callStackSize() const noexcept { return _callStackSize; }
  //! Get local stack size.
  inline uint32_t localStackSize() const noexcept { return _localStackSize; }

  //! Set call stack size.
  inline void setCallStackSize(uint32_t size) noexcept { _callStackSize = size; }
  //! Set local stack size.
  inline void setLocalStackSize(uint32_t size) noexcept { _localStackSize = size; }

  //! Combine call stack size with `size`, updating it to the greater value.
  inline void updateCallStackSize(uint32_t size) noexcept { _callStackSize = std::max(_callStackSize, size); }
  //! Combine local stack size with `size`, updating it to the greater value.
  inline void updateLocalStackSize(uint32_t size) noexcept { _localStackSize = std::max(_localStackSize, size); }

  //! Get final stack size (only valid after the FuncFrame is finalized).
  inline uint32_t finalStackSize() const noexcept { return _finalStackSize; }

  //! Get an offset to access the local stack (non-zero only if call stack is used).
  inline uint32_t localStackOffset() const noexcept { return _localStackOffset; }

  //! Get whether the function prolog/epilog requires a memory slot for storing unaligned SP.
  inline bool hasDAOffset() const noexcept { return _daOffset != kTagInvalidOffset; }
  //! Get a memory offset used to store DA (dynamic alignment) slot (relative to SP).
  inline uint32_t daOffset() const noexcept { return _daOffset; }

  inline uint32_t saOffset(uint32_t regId) const noexcept {
    return regId == _spRegId ? saOffsetFromSP()
                             : saOffsetFromSA();
  }

  inline uint32_t saOffsetFromSP() const noexcept { return _saOffsetFromSP; }
  inline uint32_t saOffsetFromSA() const noexcept { return _saOffsetFromSA; }

  //! Get which registers (by `group`) are saved/restored in prolog/epilog, respectively.
  inline uint32_t dirtyRegs(uint32_t group) const noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    return _dirtyRegs[group];
  }

  //! Set which registers (by `group`) are saved/restored in prolog/epilog, respectively.
  inline void setDirtyRegs(uint32_t group, uint32_t regs) noexcept {
    ASMJIT_ASSERT(group < BaseReg::kGroupVirt);
    _dirtyRegs[group] = regs;
  }

  //! Add registers (by `group`) to saved/restored registers.
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

  //! Get stack size required to save GP registers.
  inline uint32_t gpSaveSize() const noexcept { return _gpSaveSize; }
  //! Get stack size required to save other than GP registers (MM, XMM|YMM|ZMM, K, VFP, etc...).
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
  uint8_t _minDynamicAlignment;      //!< Minimum stack alignment to turn on dynamic alignment.

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
    std::memcpy(this, &other, sizeof(*this));
  }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline void reset(const FuncDetail* fd = nullptr) noexcept {
    _funcDetail = fd;
    _saRegId = uint8_t(BaseReg::kIdBad);
    std::memset(_reserved, 0, sizeof(_reserved));
    std::memset(_args, 0, sizeof(_args));
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
