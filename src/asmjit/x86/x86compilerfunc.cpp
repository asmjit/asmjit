// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER) && (defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64))

// [Dependencies - AsmJit]
#include "../x86/x86compiler.h"
#include "../x86/x86compilerfunc.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::X86FuncDecl - Helpers]
// ============================================================================

static ASMJIT_INLINE bool x86ArgIsInt(uint32_t aType) {
  ASMJIT_ASSERT(aType < kX86VarTypeCount);
  return Utils::inInterval<uint32_t>(aType, _kVarTypeIntStart, _kVarTypeIntEnd);
}

static ASMJIT_INLINE bool x86ArgIsFp(uint32_t aType) {
  ASMJIT_ASSERT(aType < kX86VarTypeCount);
  return Utils::inInterval<uint32_t>(aType, _kVarTypeFpStart, _kVarTypeFpEnd);
}

static ASMJIT_INLINE uint32_t x86ArgTypeToXmmType(uint32_t aType) {
  if (aType == kVarTypeFp32) return kX86VarTypeXmmSs;
  if (aType == kVarTypeFp64) return kX86VarTypeXmmSd;
  return aType;
}

//! Get an architecture depending on the calling convention `callConv`.
//!
//! Returns `kArchNone`, `kArchX86`, or `kArchX64`.
static ASMJIT_INLINE uint32_t x86GetArchFromCConv(uint32_t callConv) {
  if (Utils::inInterval<uint32_t>(callConv, _kCallConvX86Start, _kCallConvX86End)) return kArchX86;
  if (Utils::inInterval<uint32_t>(callConv, _kCallConvX64Start, _kCallConvX64End)) return kArchX64;

  return kArchNone;
}

// ============================================================================
// [asmjit::X86FuncDecl - SetPrototype]
// ============================================================================

#define R(_Index_) kX86RegIndex##_Index_
static uint32_t X86FuncDecl_initConv(X86FuncDecl* self, uint32_t arch, uint32_t callConv) {
  // Setup defaults.
  self->_argStackSize = 0;
  self->_redZoneSize = 0;
  self->_spillZoneSize = 0;

  self->_callConv = static_cast<uint8_t>(callConv);
  self->_calleePopsStack = false;
  self->_argsDirection = kFuncDirRTL;

  self->_passed.reset();
  self->_preserved.reset();

  ::memset(self->_passedOrderGp, kInvalidReg, ASMJIT_ARRAY_SIZE(self->_passedOrderGp));
  ::memset(self->_passedOrderXyz, kInvalidReg, ASMJIT_ARRAY_SIZE(self->_passedOrderXyz));

  switch (arch) {
    // ------------------------------------------------------------------------
    // [X86 Support]
    // ------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X86)
    case kArchX86: {
      self->_preserved.set(kX86RegClassGp, Utils::mask(R(Bx), R(Sp), R(Bp), R(Si), R(Di)));

      switch (callConv) {
        case kCallConvX86CDecl:
          break;

        case kCallConvX86StdCall:
          self->_calleePopsStack = true;
          break;

        case kCallConvX86MsThisCall:
          self->_calleePopsStack = true;
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Cx)));
          self->_passedOrderGp[0] = R(Cx);
          break;

        case kCallConvX86MsFastCall:
          self->_calleePopsStack = true;
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Cx), R(Cx)));
          self->_passedOrderGp[0] = R(Cx);
          self->_passedOrderGp[1] = R(Dx);
          break;

        case kCallConvX86BorlandFastCall:
          self->_calleePopsStack = true;
          self->_argsDirection = kFuncDirLTR;
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Ax), R(Dx), R(Cx)));
          self->_passedOrderGp[0] = R(Ax);
          self->_passedOrderGp[1] = R(Dx);
          self->_passedOrderGp[2] = R(Cx);
          break;

        case kCallConvX86GccFastCall:
          self->_calleePopsStack = true;
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Cx), R(Dx)));
          self->_passedOrderGp[0] = R(Cx);
          self->_passedOrderGp[1] = R(Dx);
          break;

        case kCallConvX86GccRegParm1:
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Ax)));
          self->_passedOrderGp[0] = R(Ax);
          break;

        case kCallConvX86GccRegParm2:
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Ax), R(Dx)));
          self->_passedOrderGp[0] = R(Ax);
          self->_passedOrderGp[1] = R(Dx);
          break;

        case kCallConvX86GccRegParm3:
          self->_passed.set(kX86RegClassGp, Utils::mask(R(Ax), R(Dx), R(Cx)));
          self->_passedOrderGp[0] = R(Ax);
          self->_passedOrderGp[1] = R(Dx);
          self->_passedOrderGp[2] = R(Cx);
          break;

        default:
          return kErrorInvalidArgument;
      }

      return kErrorOk;
    }
#endif // ASMJIT_BUILD_X86

    // ------------------------------------------------------------------------
    // [X64 Support]
    // ------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X64)
    case kArchX64: {
      switch (callConv) {
        case kCallConvX64Win:
          self->_spillZoneSize = 32;

          self->_passed.set(kX86RegClassGp, Utils::mask(R(Cx), R(Dx), 8, 9));
          self->_passedOrderGp[0] = R(Cx);
          self->_passedOrderGp[1] = R(Dx);
          self->_passedOrderGp[2] = 8;
          self->_passedOrderGp[3] = 9;

          self->_passed.set(kX86RegClassXyz, Utils::mask(0, 1, 2, 3));
          self->_passedOrderXyz[0] = 0;
          self->_passedOrderXyz[1] = 1;
          self->_passedOrderXyz[2] = 2;
          self->_passedOrderXyz[3] = 3;

          self->_preserved.set(kX86RegClassGp , Utils::mask(R(Bx), R(Sp), R(Bp), R(Si), R(Di), 12, 13, 14, 15));
          self->_preserved.set(kX86RegClassXyz, Utils::mask(6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
          break;

        case kCallConvX64Unix:
          self->_redZoneSize = 128;

          self->_passed.set(kX86RegClassGp, Utils::mask(R(Di), R(Si), R(Dx), R(Cx), 8, 9));
          self->_passedOrderGp[0] = R(Di);
          self->_passedOrderGp[1] = R(Si);
          self->_passedOrderGp[2] = R(Dx);
          self->_passedOrderGp[3] = R(Cx);
          self->_passedOrderGp[4] = 8;
          self->_passedOrderGp[5] = 9;

          self->_passed.set(kX86RegClassXyz, Utils::mask(0, 1, 2, 3, 4, 5, 6, 7));
          self->_passedOrderXyz[0] = 0;
          self->_passedOrderXyz[1] = 1;
          self->_passedOrderXyz[2] = 2;
          self->_passedOrderXyz[3] = 3;
          self->_passedOrderXyz[4] = 4;
          self->_passedOrderXyz[5] = 5;
          self->_passedOrderXyz[6] = 6;
          self->_passedOrderXyz[7] = 7;

          self->_preserved.set(kX86RegClassGp, Utils::mask(R(Bx), R(Sp), R(Bp), 12, 13, 14, 15));
          break;

        default:
          return kErrorInvalidArgument;
      }

      return kErrorOk;
    }
#endif // ASMJIT_BUILD_X64

    default:
      return kErrorInvalidArgument;
  }
}
#undef R

static Error X86FuncDecl_initFunc(X86FuncDecl* self, uint32_t arch,
  uint32_t ret, const uint32_t* args, uint32_t numArgs) {

  ASMJIT_ASSERT(numArgs <= kFuncArgCount);

  uint32_t callConv = self->_callConv;
  uint32_t regSize = (arch == kArchX86) ? 4 : 8;

  int32_t i = 0;
  int32_t gpPos = 0;
  int32_t xmmPos = 0;
  int32_t stackOffset = 0;
  const uint8_t* varMapping = nullptr;

#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86)
    varMapping = _x86VarMapping;
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64)
    varMapping = _x64VarMapping;
#endif // ASMJIT_BUILD_X64

  ASMJIT_ASSERT(varMapping != nullptr);
  self->_numArgs = static_cast<uint8_t>(numArgs);
  self->_retCount = 0;

  for (i = 0; i < static_cast<int32_t>(numArgs); i++) {
    FuncInOut& arg = self->getArg(i);
    arg._varType = static_cast<uint8_t>(varMapping[args[i]]);
    arg._regIndex = kInvalidReg;
    arg._stackOffset = kFuncStackInvalid;
  }

  for (; i < kFuncArgCount; i++) {
    self->_args[i].reset();
  }

  self->_rets[0].reset();
  self->_rets[1].reset();
  self->_argStackSize = 0;
  self->_used.reset();

  if (ret != kInvalidVar) {
    ret = varMapping[ret];
    switch (ret) {
      case kVarTypeInt64:
      case kVarTypeUInt64:
        // 64-bit value is returned in EDX:EAX on x86.
#if defined(ASMJIT_BUILD_X86)
        if (arch == kArchX86) {
          self->_retCount = 2;
          self->_rets[0]._varType = kVarTypeUInt32;
          self->_rets[0]._regIndex = kX86RegIndexAx;
          self->_rets[1]._varType = static_cast<uint8_t>(ret - 2);
          self->_rets[1]._regIndex = kX86RegIndexDx;
        }
        ASMJIT_FALLTHROUGH;
#endif // ASMJIT_BUILD_X86

      case kVarTypeInt8:
      case kVarTypeUInt8:
      case kVarTypeInt16:
      case kVarTypeUInt16:
      case kVarTypeInt32:
      case kVarTypeUInt32:
        self->_retCount = 1;
        self->_rets[0]._varType = static_cast<uint8_t>(ret);
        self->_rets[0]._regIndex = kX86RegIndexAx;
        break;

      case kX86VarTypeMm:
        self->_retCount = 1;
        self->_rets[0]._varType = static_cast<uint8_t>(ret);
        self->_rets[0]._regIndex = 0;
        break;

      case kVarTypeFp32:
        self->_retCount = 1;
        if (arch == kArchX86) {
          self->_rets[0]._varType = kVarTypeFp32;
          self->_rets[0]._regIndex = 0;
        }
        else {
          self->_rets[0]._varType = kX86VarTypeXmmSs;
          self->_rets[0]._regIndex = 0;
        }
        break;

      case kVarTypeFp64:
        self->_retCount = 1;
        if (arch == kArchX86) {
          self->_rets[0]._varType = kVarTypeFp64;
          self->_rets[0]._regIndex = 0;
        }
        else {
          self->_rets[0]._varType = kX86VarTypeXmmSd;
          self->_rets[0]._regIndex = 0;
          break;
        }
        break;

      case kX86VarTypeXmm:
      case kX86VarTypeXmmSs:
      case kX86VarTypeXmmSd:
      case kX86VarTypeXmmPs:
      case kX86VarTypeXmmPd:
        self->_retCount = 1;
        self->_rets[0]._varType = static_cast<uint8_t>(ret);
        self->_rets[0]._regIndex = 0;
        break;
    }
  }

  if (self->_numArgs == 0)
    return kErrorOk;

#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86) {
    // Register arguments (Integer), always left-to-right.
    for (i = 0; i != static_cast<int32_t>(numArgs); i++) {
      FuncInOut& arg = self->getArg(i);
      uint32_t varType = varMapping[arg.getVarType()];

      if (!x86ArgIsInt(varType) || gpPos >= ASMJIT_ARRAY_SIZE(self->_passedOrderGp))
        continue;

      if (self->_passedOrderGp[gpPos] == kInvalidReg)
        continue;

      arg._regIndex = self->_passedOrderGp[gpPos++];
      self->_used.or_(kX86RegClassGp, Utils::mask(arg.getRegIndex()));
    }

    // Stack arguments.
    int32_t iStart = static_cast<int32_t>(numArgs - 1);
    int32_t iEnd   = -1;
    int32_t iStep  = -1;

    if (self->_argsDirection == kFuncDirLTR) {
      iStart = 0;
      iEnd   = static_cast<int32_t>(numArgs);
      iStep  = 1;
    }

    for (i = iStart; i != iEnd; i += iStep) {
      FuncInOut& arg = self->getArg(i);
      uint32_t varType = varMapping[arg.getVarType()];

      if (arg.hasRegIndex())
        continue;

      if (x86ArgIsInt(varType)) {
        stackOffset -= 4;
        arg._stackOffset = static_cast<int16_t>(stackOffset);
      }
      else if (x86ArgIsFp(varType)) {
        int32_t size = static_cast<int32_t>(_x86VarInfo[varType].getSize());
        stackOffset -= size;
        arg._stackOffset = static_cast<int16_t>(stackOffset);
      }
    }
  }
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64) {
    if (callConv == kCallConvX64Win) {
      int32_t argMax = Utils::iMin<int32_t>(numArgs, 4);

      // Register arguments (Gp/Xmm), always left-to-right.
      for (i = 0; i != argMax; i++) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (x86ArgIsInt(varType) && i < ASMJIT_ARRAY_SIZE(self->_passedOrderGp)) {
          arg._regIndex = self->_passedOrderGp[i];
          self->_used.or_(kX86RegClassGp, Utils::mask(arg.getRegIndex()));
          continue;
        }

        if (x86ArgIsFp(varType) && i < ASMJIT_ARRAY_SIZE(self->_passedOrderXyz)) {
          arg._varType = static_cast<uint8_t>(x86ArgTypeToXmmType(varType));
          arg._regIndex = self->_passedOrderXyz[i];
          self->_used.or_(kX86RegClassXyz, Utils::mask(arg.getRegIndex()));
        }
      }

      // Stack arguments (always right-to-left).
      for (i = numArgs - 1; i != -1; i--) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (arg.hasRegIndex())
          continue;

        if (x86ArgIsInt(varType)) {
          stackOffset -= 8; // Always 8 bytes.
          arg._stackOffset = stackOffset;
        }
        else if (x86ArgIsFp(varType)) {
          stackOffset -= 8; // Always 8 bytes (float/double).
          arg._stackOffset = stackOffset;
        }
      }

      // 32 bytes shadow space (X64W calling convention specific).
      stackOffset -= 4 * 8;
    }
    else {
      // Register arguments (Gp), always left-to-right.
      for (i = 0; i != static_cast<int32_t>(numArgs); i++) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (!x86ArgIsInt(varType) || gpPos >= ASMJIT_ARRAY_SIZE(self->_passedOrderGp))
          continue;

        if (self->_passedOrderGp[gpPos] == kInvalidReg)
          continue;

        arg._regIndex = self->_passedOrderGp[gpPos++];
        self->_used.or_(kX86RegClassGp, Utils::mask(arg.getRegIndex()));
      }

      // Register arguments (Xmm), always left-to-right.
      for (i = 0; i != static_cast<int32_t>(numArgs); i++) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (x86ArgIsFp(varType)) {
          arg._varType = static_cast<uint8_t>(x86ArgTypeToXmmType(varType));
          arg._regIndex = self->_passedOrderXyz[xmmPos++];
          self->_used.or_(kX86RegClassXyz, Utils::mask(arg.getRegIndex()));
        }
      }

      // Stack arguments.
      for (i = numArgs - 1; i != -1; i--) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (arg.hasRegIndex())
          continue;

        if (x86ArgIsInt(varType)) {
          stackOffset -= 8;
          arg._stackOffset = static_cast<int16_t>(stackOffset);
        }
        else if (x86ArgIsFp(varType)) {
          int32_t size = static_cast<int32_t>(_x86VarInfo[varType].getSize());

          stackOffset -= size;
          arg._stackOffset = static_cast<int16_t>(stackOffset);
        }
      }
    }
  }
#endif // ASMJIT_BUILD_X64

  // Modify the stack offset, thus in result all parameters would have positive
  // non-zero stack offset.
  for (i = 0; i < static_cast<int32_t>(numArgs); i++) {
    FuncInOut& arg = self->getArg(i);
    if (!arg.hasRegIndex()) {
      arg._stackOffset += static_cast<uint16_t>(static_cast<int32_t>(regSize) - stackOffset);
    }
  }

  self->_argStackSize = static_cast<uint32_t>(-stackOffset);
  return kErrorOk;
}

Error X86FuncDecl::setPrototype(const FuncPrototype& p) {
  uint32_t callConv = p.getCallConv();
  uint32_t arch = x86GetArchFromCConv(callConv);

  if (arch == kArchNone)
    return kErrorInvalidArgument;

  if (p.getNumArgs() > kFuncArgCount)
    return kErrorInvalidArgument;

  // Validate that the required convention is supported by the current asmjit
  // configuration, if only one target is compiled.
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64)
    return kErrorInvalidState;
#endif // ASMJIT_BUILD_X86 && !ASMJIT_BUILD_X64

#if !defined(ASMJIT_BUILD_X86) && defined(ASMJIT_BUILD_X64)
  if (arch == kArchX86)
    return kErrorInvalidState;
#endif // !ASMJIT_BUILD_X86 && ASMJIT_BUILD_X64

  ASMJIT_PROPAGATE_ERROR(X86FuncDecl_initConv(this, arch, callConv));
  ASMJIT_PROPAGATE_ERROR(X86FuncDecl_initFunc(this, arch, p.getRet(), p.getArgs(), p.getNumArgs()));

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86FuncDecl - Reset]
// ============================================================================

void X86FuncDecl::reset() {
  uint32_t i;

  _callConv = kCallConvNone;
  _calleePopsStack = false;
  _argsDirection = kFuncDirRTL;
  _reserved0 = 0;

  _numArgs = 0;
  _retCount = 0;

  _argStackSize = 0;
  _redZoneSize = 0;
  _spillZoneSize = 0;

  for (i = 0; i < ASMJIT_ARRAY_SIZE(_args); i++)
    _args[i].reset();

  _rets[0].reset();
  _rets[1].reset();

  _used.reset();
  _passed.reset();
  _preserved.reset();

  ::memset(_passedOrderGp, kInvalidReg, ASMJIT_ARRAY_SIZE(_passedOrderGp));
  ::memset(_passedOrderXyz, kInvalidReg, ASMJIT_ARRAY_SIZE(_passedOrderXyz));
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER && (ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64)
