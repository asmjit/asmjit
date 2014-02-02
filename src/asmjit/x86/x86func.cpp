// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../base/assert.h"
#include "../base/intutil.h"
#include "../base/string.h"
#include "../x86/x86defs.h"
#include "../x86/x86func.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

// ============================================================================
// [asmjit::X86X64FuncDecl - FindArgByReg]
// ============================================================================

uint32_t X86X64FuncDecl::findArgByReg(uint32_t rClass, uint32_t rIndex) const {
  for (uint32_t i = 0; i < _argCount; i++) {
    const FuncInOut& arg = getArg(i);
    if (arg.getRegIndex() == rIndex && x86VarTypeToClass(arg.getVarType()) == rClass)
      return i;
  }

  return kInvalidValue;
}

// ============================================================================
// [asmjit::X86X64FuncDecl - SetPrototype]
// ============================================================================

#define R(_Index_) kRegIndex##_Index_
static uint32_t X86X64FuncDecl_initConv(X86X64FuncDecl* self, uint32_t arch, uint32_t conv) {
  uint32_t i;

  // Setup defaults.
  self->_argStackSize = 0;
  self->_redZoneSize = 0;
  self->_spillZoneSize = 0;

  self->_convention = static_cast<uint8_t>(conv);
  self->_calleePopsStack = false;
  self->_direction = kFuncDirRtl;

  self->_passed.reset();
  self->_preserved.reset();

  for (i = 0; i < ASMJIT_ARRAY_SIZE(self->_passedOrderGp); i++) {
    self->_passedOrderGp[i] = kInvalidReg;
  }

  for (i = 0; i < ASMJIT_ARRAY_SIZE(self->_passedOrderXmm); i++) {
    self->_passedOrderXmm[i] = kInvalidReg;
  }

  // --------------------------------------------------------------------------
  // [X86 Support]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86) {
    self->_preserved.set(kRegClassGp, IntUtil::mask(R(Bx), R(Sp), R(Bp), R(Si), R(Di)));

    switch (conv) {
      case kFuncConvCDecl:
        break;

      case kFuncConvStdCall:
        self->_calleePopsStack = true;
        break;

      case kFuncConvMsThisCall:
        self->_calleePopsStack = true;
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Cx)));
        self->_passedOrderGp[0] = R(Cx);
        break;

      case kFuncConvMsFastCall:
        self->_calleePopsStack = true;
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Cx), R(Cx)));
        self->_passedOrderGp[0] = R(Cx);
        self->_passedOrderGp[1] = R(Dx);
        break;

      case kFuncConvBorlandFastCall:
        self->_calleePopsStack = true;
        self->_direction = kFuncDirLtr;
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Ax), R(Dx), R(Cx)));
        self->_passedOrderGp[0] = R(Ax);
        self->_passedOrderGp[1] = R(Dx);
        self->_passedOrderGp[2] = R(Cx);
        break;

      case kFuncConvGccFastCall:
        self->_calleePopsStack = true;
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Cx), R(Dx)));
        self->_passedOrderGp[0] = R(Cx);
        self->_passedOrderGp[1] = R(Dx);
        break;

      case kFuncConvGccRegParm1:
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Ax)));
        self->_passedOrderGp[0] = R(Ax);
        break;

      case kFuncConvGccRegParm2:
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Ax), R(Dx)));
        self->_passedOrderGp[0] = R(Ax);
        self->_passedOrderGp[1] = R(Dx);
        break;

      case kFuncConvGccRegParm3:
        self->_passed.set(kRegClassGp, IntUtil::mask(R(Ax), R(Dx), R(Cx)));
        self->_passedOrderGp[0] = R(Ax);
        self->_passedOrderGp[1] = R(Dx);
        self->_passedOrderGp[2] = R(Cx);
        break;

      default:
        ASMJIT_ASSERT(!"Reached");
    }

    return kErrorOk;
  }
#endif // ASMJIT_BUILD_X86

  // --------------------------------------------------------------------------
  // [X64 Support]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X64)
  switch (conv) {
    case kFuncConvX64W:
      self->_spillZoneSize = 32;

      self->_passed.set(kRegClassGp, IntUtil::mask(R(Cx), R(Dx), R(R8), R(R9)));
      self->_passedOrderGp[0] = R(Cx);
      self->_passedOrderGp[1] = R(Dx);
      self->_passedOrderGp[2] = R(R8);
      self->_passedOrderGp[3] = R(R9);

      self->_passed.set(kRegClassXy, IntUtil::mask(0, 1, 2, 3));
      self->_passedOrderXmm[0] = R(Xmm0);
      self->_passedOrderXmm[1] = R(Xmm1);
      self->_passedOrderXmm[2] = R(Xmm2);
      self->_passedOrderXmm[3] = R(Xmm3);

      self->_preserved.set(kRegClassGp, IntUtil::mask(R(Bx), R(Sp), R(Bp), R(Si), R(Di), R(R12), R(R13), R(R14), R(R15)));
      self->_preserved.set(kRegClassXy, IntUtil::mask(R(Xmm6), R(Xmm7), R(Xmm8), R(Xmm9), R(Xmm10), R(Xmm11), R(Xmm12), R(Xmm13), R(Xmm14), R(Xmm15)));
      break;

    case kFuncConvX64U:
      self->_redZoneSize = 128;

      self->_passed.set(kRegClassGp, IntUtil::mask(R(Di), R(Si), R(Dx), R(Cx), R(R8), R(R9)));
      self->_passedOrderGp[0] = R(Di);
      self->_passedOrderGp[1] = R(Si);
      self->_passedOrderGp[2] = R(Dx);
      self->_passedOrderGp[3] = R(Cx);
      self->_passedOrderGp[4] = R(R8);
      self->_passedOrderGp[5] = R(R9);

      self->_passed.set(kRegClassXy, IntUtil::mask(R(Xmm0), R(Xmm1), R(Xmm2), R(Xmm3), R(Xmm4), R(Xmm5), R(Xmm6), R(Xmm7)));
      self->_passedOrderXmm[0] = R(Xmm0);
      self->_passedOrderXmm[1] = R(Xmm1);
      self->_passedOrderXmm[2] = R(Xmm2);
      self->_passedOrderXmm[3] = R(Xmm3);
      self->_passedOrderXmm[4] = R(Xmm4);
      self->_passedOrderXmm[5] = R(Xmm5);
      self->_passedOrderXmm[6] = R(Xmm6);
      self->_passedOrderXmm[7] = R(Xmm7);

      self->_preserved.set(kRegClassGp, IntUtil::mask(R(Bx), R(Sp), R(Bp), R(R12), R(R13), R(R14), R(R15)));
      break;

    default:
      ASMJIT_ASSERT(!"Reached");
  }
#endif // ASMJIT_BUILD_X64

  return kErrorOk;
}
#undef R

static Error X86X64FuncDecl_initFunc(X86X64FuncDecl* self, uint32_t arch,
  uint32_t ret, const uint32_t* argList, uint32_t argCount) {

  ASMJIT_ASSERT(argCount <= kFuncArgCount);

  uint32_t conv = self->_convention;
  uint32_t regSize = (arch == kArchX86) ? 4 : 8;

  int32_t i = 0;
  int32_t gpPos = 0;
  int32_t xmmPos = 0;
  int32_t stackOffset = 0;

  const uint8_t* varMapping;

#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86)
    varMapping = x86::_varMapping;
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64)
    varMapping = x64::_varMapping;
#endif // ASMJIT_BUILD_X64

  self->_argCount = static_cast<uint8_t>(argCount);
  self->_retCount = 0;

  for (i = 0; i < static_cast<int32_t>(argCount); i++) {
    FuncInOut& arg = self->getArg(i);
    arg._varType = static_cast<uint8_t>(argList[i]);
    arg._regIndex = kInvalidReg;
    arg._stackOffset = kFuncStackInvalid;
  }

  for (; i < kFuncArgCount; i++) {
    self->_argList[i].reset();
  }

  self->_retList[0].reset();
  self->_retList[1].reset();
  self->_argStackSize = 0;
  self->_used.reset();

  if (ret != kVarTypeInvalid) {
    ret = varMapping[ret];
    switch (ret) {
      case kVarTypeInt64:
      case kVarTypeUInt64:
        // 64-bit value is returned in EDX:EAX on x86.
#if defined(ASMJIT_BUILD_X86)
        if (arch == kArchX86) {
          self->_retCount = 2;
          self->_retList[0]._varType = kVarTypeUInt32;
          self->_retList[0]._regIndex = kRegIndexAx;
          self->_retList[1]._varType = static_cast<uint8_t>(ret - 2);
          self->_retList[1]._regIndex = kRegIndexDx;
        }
#endif // ASMJIT_BUILD_X86
        // ... Fall through ...

      case kVarTypeInt8:
      case kVarTypeUInt8:
      case kVarTypeInt16:
      case kVarTypeUInt16:
      case kVarTypeInt32:
      case kVarTypeUInt32:
        self->_retCount = 1;
        self->_retList[0]._varType = static_cast<uint8_t>(ret);
        self->_retList[0]._regIndex = kRegIndexAx;
        break;

      case kVarTypeMm:
        self->_retCount = 1;
        self->_retList[0]._varType = static_cast<uint8_t>(ret);
        self->_retList[0]._regIndex = kRegIndexMm0;
        break;

      case kVarTypeFp32:
        self->_retCount = 1;
        if (arch == kArchX86) {
          self->_retList[0]._varType = kVarTypeFp32;
          self->_retList[0]._regIndex = kRegIndexFp0;
        }
        else {
          self->_retList[0]._varType = kVarTypeXmmSs;
          self->_retList[0]._regIndex = kRegIndexXmm0;
        }
        break;

      case kVarTypeFp64:
        self->_retCount = 1;
        if (arch == kArchX86) {
          self->_retList[0]._varType = kVarTypeFp64;
          self->_retList[0]._regIndex = kRegIndexFp0;
        }
        else {
          self->_retList[0]._varType = kVarTypeXmmSd;
          self->_retList[0]._regIndex = kRegIndexXmm0;
          break;
        }
        break;

      case kVarTypeFpEx:
        self->_retCount = 1;
        self->_retList[0]._varType = static_cast<uint8_t>(ret);
        self->_retList[0]._regIndex = kRegIndexFp0;
        break;

      case kVarTypeXmm:
      case kVarTypeXmmSs:
      case kVarTypeXmmSd:
      case kVarTypeXmmPs:
      case kVarTypeXmmPd:
        self->_retCount = 1;
        self->_retList[0]._varType = static_cast<uint8_t>(ret);
        self->_retList[0]._regIndex = kRegIndexXmm0;
        break;
    }
  }

  if (self->_argCount == 0)
    return kErrorOk;

#if defined(ASMJIT_BUILD_X86)
  if (arch == kArchX86) {
    // Register arguments (Integer), always left-to-right.
    for (i = 0; i != static_cast<int32_t>(argCount); i++) {
      FuncInOut& arg = self->getArg(i);
      uint32_t varType = varMapping[arg.getVarType()];

      if (x86VarIsInt(varType) && gpPos < 16 && self->_passedOrderGp[gpPos] != kInvalidReg) {
        arg._regIndex = self->_passedOrderGp[gpPos++];
        self->_used.add(kRegClassGp, IntUtil::mask(arg.getRegIndex()));
      }
    }

    // Stack arguments.
    int32_t iStart = static_cast<int32_t>(argCount - 1);
    int32_t iEnd   = -1;
    int32_t iStep  = -1;

    if (self->_direction == kFuncDirLtr) {
      iStart = 0;
      iEnd   = static_cast<int32_t>(argCount);
      iStep  = 1;
    }

    for (i = iStart; i != iEnd; i += iStep) {
      FuncInOut& arg = self->getArg(i);
      uint32_t varType = varMapping[arg.getVarType()];

      if (arg.hasRegIndex())
        continue;

      if (x86VarIsInt(varType)) {
        stackOffset -= 4;
        arg._stackOffset = static_cast<int16_t>(stackOffset);
      }
      else if (x86VarIsFloat(varType)) {
        int32_t size = static_cast<int32_t>(_varInfo[varType].getSize());
        stackOffset -= size;
        arg._stackOffset = static_cast<int16_t>(stackOffset);
      }
    }
  }
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64) {
    if (conv == kFuncConvX64W) {
      int32_t argMax = argCount < 4 ? argCount : 4;

      // Register arguments (Gp/Xmm), always left-to-right.
      for (i = 0; i != argMax; i++) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (x86VarIsInt(varType)) {
          arg._regIndex = self->_passedOrderGp[i];
          self->_used.add(kRegClassGp, IntUtil::mask(arg.getRegIndex()));
        }
        else if (x86VarIsFloat(varType)) {
          arg._regIndex = self->_passedOrderXmm[i];
          self->_used.add(kRegClassXy, IntUtil::mask(arg.getRegIndex()));
        }
      }

      // Stack arguments (always right-to-left).
      for (i = argCount - 1; i != -1; i--) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (arg.hasRegIndex())
          continue;

        if (x86VarIsInt(varType)) {
          stackOffset -= 8; // Always 8 bytes.
          arg._stackOffset = stackOffset;
        }
        else if (x86VarIsFloat(varType)) {
          int32_t size = static_cast<int32_t>(_varInfo[varType].getSize());
          stackOffset -= size;
          arg._stackOffset = stackOffset;
        }
      }

      // 32 bytes shadow space (X64W calling convention specific).
      stackOffset -= 4 * 8;
    }
    else {
      // Register arguments (Gp), always left-to-right.
      for (i = 0; i != static_cast<int32_t>(argCount); i++) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (x86VarIsInt(varType) && gpPos < 32 && self->_passedOrderGp[gpPos] != kInvalidReg) {
          arg._regIndex = self->_passedOrderGp[gpPos++];
          self->_used.add(kRegClassGp, IntUtil::mask(arg.getRegIndex()));
        }
      }

      // Register arguments (Xmm), always left-to-right.
      for (i = 0; i != static_cast<int32_t>(argCount); i++) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (x86VarIsFloat(varType)) {
          arg._regIndex = self->_passedOrderXmm[xmmPos++];
          self->_used.add(kRegClassXy, IntUtil::mask(arg.getRegIndex()));
        }
      }

      // Stack arguments.
      for (i = argCount - 1; i != -1; i--) {
        FuncInOut& arg = self->getArg(i);
        uint32_t varType = varMapping[arg.getVarType()];

        if (arg.hasRegIndex())
          continue;

        if (x86VarIsInt(varType)) {
          stackOffset -= 8;
          arg._stackOffset = static_cast<int16_t>(stackOffset);
        }
        else if (x86VarIsFloat(varType)) {
          int32_t size = static_cast<int32_t>(_varInfo[varType].getSize());

          stackOffset -= size;
          arg._stackOffset = static_cast<int16_t>(stackOffset);
        }
      }
    }
  }
#endif // ASMJIT_BUILD_X64

  // Modify the stack offset, thus in result all parameters would have positive
  // non-zero stack offset.
  for (i = 0; i < static_cast<int32_t>(argCount); i++) {
    FuncInOut& arg = self->getArg(i);
    if (!arg.hasRegIndex()) {
      arg._stackOffset += static_cast<uint16_t>(static_cast<int32_t>(regSize) - stackOffset);
    }
  }

  self->_argStackSize = static_cast<uint32_t>(-stackOffset);
  return kErrorOk;
}

Error X86X64FuncDecl::setPrototype(uint32_t conv, const FuncPrototype& p) {
  if (conv == kFuncConvNone || conv >= _kFuncConvCount)
    return kErrorInvalidArgument;

  if (p.getArgCount() > kFuncArgCount)
    return kErrorInvalidArgument;

  // Validate that the required convention is supported by the current asmjit
  // configuration, if only one target is compiled.
  uint32_t arch = x86GetArchFromCConv(conv);
#if defined(ASMJIT_BUILD_X86) && !defined(ASMJIT_BUILD_X64)
  if (arch == kArchX64)
    return kErrorInvalidState;
#endif // ASMJIT_BUILD_X86 && !ASMJIT_BUILD_X64

#if !defined(ASMJIT_BUILD_X86) && defined(ASMJIT_BUILD_X64)
  if (arch == kArchX86)
    return kErrorInvalidState;
#endif // !ASMJIT_BUILD_X86 && ASMJIT_BUILD_X64

  ASMJIT_PROPAGATE_ERROR(X86X64FuncDecl_initConv(this, arch, conv));
  ASMJIT_PROPAGATE_ERROR(X86X64FuncDecl_initFunc(this, arch, p.getRet(), p.getArgList(), p.getArgCount()));

  return kErrorOk;
}

// ============================================================================
// [asmjit::X86X64FuncDecl - Reset]
// ============================================================================

void X86X64FuncDecl::reset() {
  uint32_t i;

  _convention = kFuncConvNone;
  _calleePopsStack = false;
  _direction = kFuncDirRtl;
  _reserved0 = 0;

  _argCount = 0;
  _retCount = 0;

  _argStackSize = 0;
  _redZoneSize = 0;
  _spillZoneSize = 0;

  for (i = 0; i < ASMJIT_ARRAY_SIZE(_argList); i++) {
    _argList[i].reset();
  }

  _retList[0].reset();
  _retList[1].reset();

  _used.reset();
  _passed.reset();
  _preserved.reset();

  for (i = 0; i < ASMJIT_ARRAY_SIZE(_passedOrderGp); i++) {
    _passedOrderGp[i] = kInvalidReg;
  }

  for (i = 0; i < ASMJIT_ARRAY_SIZE(_passedOrderXmm); i++) {
    _passedOrderXmm[i] = kInvalidReg;
  }
}

} // x86x64 namespace
} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
