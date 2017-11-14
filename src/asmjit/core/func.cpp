// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/arch.h"
#include "../core/func.h"
#include "../core/type.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86internal_p.h"
  #include "../x86/x86operand.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/arminternal_p.h"
  #include "../arm/armoperand.h"
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::FuncDetail - Init / Reset]
// ============================================================================

ASMJIT_FAVOR_SIZE Error FuncDetail::init(const FuncSignature& sign) {
  uint32_t ccId = sign.getCallConv();
  CallConv& cc = _callConv;

  uint32_t argCount = sign.getArgCount();
  if (ASMJIT_UNLIKELY(argCount > kFuncArgCount))
    return DebugUtils::errored(kErrorInvalidArgument);

  ASMJIT_PROPAGATE(cc.init(ccId));

  uint32_t gpSize = (cc.getArchType() == ArchInfo::kTypeX86) ? 4 : 8;
  uint32_t deabstractDelta = Type::deabstractDeltaOfSize(gpSize);

  const uint8_t* args = sign.getArgs();
  for (uint32_t i = 0; i < argCount; i++) {
    FuncValue& arg = _args[i];
    arg.initTypeId(Type::deabstract(args[i], deabstractDelta));
  }
  _argCount = uint8_t(argCount);

  uint32_t ret = sign.getRet();
  if (ret != Type::kIdVoid) {
    _rets[0].initTypeId(Type::deabstract(ret, deabstractDelta));
    _retCount = 1;
  }

  #ifdef ASMJIT_BUILD_X86
  if (CallConv::isX86Family(ccId))
    return X86Internal::initFuncDetail(*this, sign, gpSize);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (CallConv::isArmFamily(ccId))
    return ArmInternal::initFuncDetail(*this, sign, gpSize);
  #endif

  // We should never bubble here as if `cc.init()` succeeded then there has to
  // be an implementation for the current architecture. However, stay safe.
  return DebugUtils::errored(kErrorInvalidArgument);
}

// ============================================================================
// [asmjit::FuncFrame - Init / Reset / Finalize]
// ============================================================================

ASMJIT_FAVOR_SIZE Error FuncFrame::init(const FuncDetail& func) noexcept {
  uint32_t ccId = func.getCallConv().getId();

  #ifdef ASMJIT_BUILD_X86
  if (CallConv::isX86Family(ccId))
    return X86Internal::initFuncFrame(*this, func);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (CallConv::isArmFamily(ccId))
    return ArmInternal::initFuncFrame(*this, func);
  #endif

  return DebugUtils::errored(kErrorInvalidArgument);
}

ASMJIT_FAVOR_SIZE Error FuncFrame::finalize() noexcept {
  #ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(getArchType()))
    return X86Internal::finalizeFuncFrame(*this);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(getArchType()))
    return ArmInternal::finalizeFuncFrame(*this);
  #endif

  return DebugUtils::errored(kErrorInvalidArgument);
}

// ============================================================================
// [asmjit::FuncArgsAssignment]
// ============================================================================

ASMJIT_FAVOR_SIZE Error FuncArgsAssignment::updateFuncFrame(FuncFrame& frame) const noexcept {
  const FuncDetail* func = getFuncDetail();
  if (!func) return DebugUtils::errored(kErrorInvalidState);

  uint32_t ccId = func->getCallConv().getId();

  #ifdef ASMJIT_BUILD_X86
  if (CallConv::isX86Family(ccId))
    return X86Internal::argsToFuncFrame(*this, frame);
  #endif

  #ifdef ASMJIT_BUILD_ARM
  if (CallConv::isArmFamily(ccId))
    return ArmInternal::argsToFuncFrame(*this, frame);
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_END_NAMESPACE