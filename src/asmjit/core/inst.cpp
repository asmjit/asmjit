// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/build.h"
#ifdef ASMJIT_BUILD_X86

#include "../core/arch.h"
#include "../core/inst.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86instapi_p.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/arminstapi_p.h"
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::InstAPI - Text]
// ============================================================================

#ifndef ASMJIT_NO_TEXT
Error InstAPI::instIdToString(uint32_t archId, uint32_t instId, String& output) noexcept {
#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::InstInternal::instIdToString(archId, instId, output);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::InstInternal::instIdToString(archId, instId, output);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}

uint32_t InstAPI::stringToInstId(uint32_t archId, const char* s, size_t len) noexcept {
#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::InstInternal::stringToInstId(archId, s, len);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::InstInternal::stringToInstId(archId, s, len);
#endif

  return 0;
}
#endif // !ASMJIT_NO_TEXT

// ============================================================================
// [asmjit::InstAPI - Validate]
// ============================================================================

#ifndef ASMJIT_NO_VALIDATION
Error InstAPI::validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept {
#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::InstInternal::validate(archId, inst, operands, opCount);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::InstInternal::validate(archId, inst, operands, opCount);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}
#endif // !ASMJIT_NO_VALIDATION

// ============================================================================
// [asmjit::InstAPI - QueryRWInfo]
// ============================================================================

#ifndef ASMJIT_NO_INTROSPECTION
Error InstAPI::queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, InstRWInfo& out) noexcept {
  if (ASMJIT_UNLIKELY(opCount > 6))
    return DebugUtils::errored(kErrorInvalidArgument);

#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::InstInternal::queryRWInfo(archId, inst, operands, opCount, out);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::InstInternal::queryRWInfo(archId, inst, operands, opCount, out);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}
#endif // !ASMJIT_NO_INTROSPECTION

// ============================================================================
// [asmjit::InstAPI - QueryFeatures]
// ============================================================================

#ifndef ASMJIT_NO_INTROSPECTION
Error InstAPI::queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, BaseFeatures& out) noexcept {
#ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archId))
    return x86::InstInternal::queryFeatures(archId, inst, operands, opCount, out);
#endif

#ifdef ASMJIT_BUILD_ARM
  if (ArchInfo::isArmFamily(archId))
    return arm::InstInternal::queryFeatures(archId, inst, operands, opCount, out);
#endif

  return DebugUtils::errored(kErrorInvalidArch);
}
#endif // !ASMJIT_NO_INTROSPECTION

ASMJIT_END_NAMESPACE

#endif // ASMJIT_BUILD_X86
