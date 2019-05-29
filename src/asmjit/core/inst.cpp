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
  #include "../x86/x86instdb_p.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/arminstdb.h"
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::BaseInst - Validate]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
Error BaseInst::validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept {
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
#endif

// ============================================================================
// [asmjit::BaseInst - QueryRWInfo]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
Error BaseInst::queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, InstRWInfo& out) noexcept {
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
#endif

// ============================================================================
// [asmjit::BaseInst - QueryFeatures]
// ============================================================================

#ifndef ASMJIT_DISABLE_INST_API
Error BaseInst::queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, BaseFeatures& out) noexcept {
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
#endif

ASMJIT_END_NAMESPACE

#endif // ASMJIT_BUILD_X86
