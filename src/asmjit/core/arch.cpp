// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/arch.h"
#include "../core/intutils.h"
#include "../core/type.h"

#ifdef ASMJIT_BUILD_X86
  #include "../x86/x86operand.h"
#endif

#ifdef ASMJIT_BUILD_ARM
  #include "../arm/armoperand.h"
#endif

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ArchInfo]
// ============================================================================

static const uint32_t archInfoTable[] = {
  // <--------------------+---------------------+-----------------------+-------+
  //                      | Type                | SubType               | GPInfo|
  // <--------------------+---------------------+-----------------------+-------+
  IntUtils::bytepack32_4x8(ArchInfo::kTypeNone  , ArchInfo::kSubTypeNone, 0,  0),
  IntUtils::bytepack32_4x8(ArchInfo::kTypeX86   , ArchInfo::kSubTypeNone, 4,  8),
  IntUtils::bytepack32_4x8(ArchInfo::kTypeX64   , ArchInfo::kSubTypeNone, 8, 16),
  IntUtils::bytepack32_4x8(ArchInfo::kTypeA32   , ArchInfo::kSubTypeNone, 4, 16),
  IntUtils::bytepack32_4x8(ArchInfo::kTypeA64   , ArchInfo::kSubTypeNone, 8, 32)
};

ASMJIT_FAVOR_SIZE void ArchInfo::init(uint32_t type, uint32_t subType) noexcept {
  uint32_t index = type < ASMJIT_ARRAY_SIZE(archInfoTable) ? type : uint32_t(0);

  // Make sure the `archInfoTable` array is correctly indexed.
  _signature = archInfoTable[index];
  ASMJIT_ASSERT(_type == index);

  // Even if the architecture is not known we setup its type and sub-type,
  // however, such architecture is not really useful.
  _type = uint8_t(type);
  _subType = uint8_t(subType);
}

// ============================================================================
// [asmjit::ArchUtils]
// ============================================================================

ASMJIT_FAVOR_SIZE Error ArchUtils::typeIdToRegInfo(uint32_t archType, uint32_t& typeIdInOut, RegInfo& regInfo) noexcept {
  uint32_t typeId = typeIdInOut;

  // Zero the signature so it's clear in case that typeId is not invalid.
  regInfo._signature = 0;

  // TODO: Move to X86 backend.
  #ifdef ASMJIT_BUILD_X86
  if (ArchInfo::isX86Family(archType)) {
    // Passed RegType instead of TypeId?
    if (typeId <= Reg::kRegMax)
      typeId = x86OpData.archRegs.regTypeToTypeId[typeId];

    if (ASMJIT_UNLIKELY(!Type::isValid(typeId)))
      return DebugUtils::errored(kErrorInvalidTypeId);

    // First normalize architecture dependent types.
    if (Type::isAbstract(typeId)) {
      if (typeId == Type::kIdIntPtr)
        typeId = (archType == ArchInfo::kTypeX86) ? Type::kIdI32 : Type::kIdI64;
      else
        typeId = (archType == ArchInfo::kTypeX86) ? Type::kIdU32 : Type::kIdU64;
    }

    // Type size helps to construct all groupss of registers. If the size is zero
    // then the TypeId is invalid.
    uint32_t size = Type::sizeOf(typeId);
    if (ASMJIT_UNLIKELY(!size))
      return DebugUtils::errored(kErrorInvalidTypeId);

    if (ASMJIT_UNLIKELY(typeId == Type::kIdF80))
      return DebugUtils::errored(kErrorInvalidUseOfF80);

    uint32_t regType = 0;

    switch (typeId) {
      case Type::kIdI8:
      case Type::kIdU8:
        regType = X86Reg::kRegGpbLo;
        break;

      case Type::kIdI16:
      case Type::kIdU16:
        regType = X86Reg::kRegGpw;
        break;

      case Type::kIdI32:
      case Type::kIdU32:
        regType = X86Reg::kRegGpd;
        break;

      case Type::kIdI64:
      case Type::kIdU64:
        if (archType == ArchInfo::kTypeX86)
          return DebugUtils::errored(kErrorInvalidUseOfGpq);

        regType = X86Reg::kRegGpq;
        break;

      // F32 and F64 are always promoted to use vector registers.
      case Type::kIdF32:
        typeId = Type::kIdF32x1;
        regType = X86Reg::kRegXmm;
        break;

      case Type::kIdF64:
        typeId = Type::kIdF64x1;
        regType = X86Reg::kRegXmm;
        break;

      // Mask registers {k}.
      case Type::kIdMask8:
      case Type::kIdMask16:
      case Type::kIdMask32:
      case Type::kIdMask64:
        regType = X86Reg::kRegK;
        break;

      // MMX registers.
      case Type::kIdMmx32:
      case Type::kIdMmx64:
        regType = X86Reg::kRegMm;
        break;

      // XMM|YMM|ZMM registers.
      default:
        if (size <= 16)
          regType = X86Reg::kRegXmm;
        else if (size == 32)
          regType = X86Reg::kRegYmm;
        else
          regType = X86Reg::kRegZmm;
        break;
    }

    typeIdInOut = typeId;
    regInfo._signature = x86OpData.archRegs.regInfo[regType].getSignature();
    return kErrorOk;
  }
  #endif

  return DebugUtils::errored(kErrorInvalidArch);
}

ASMJIT_END_NAMESPACE
