// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_AARCH64)

#include "../core/cpuinfo.h"
#include "../core/misc_p.h"
#include "../core/support_p.h"
#include "../arm/a64instapi_p.h"
#include "../arm/a64instdb_p.h"
#include "../arm/a64operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a64)

namespace InstInternal {

// a64::InstInternal - Text
// ========================

#ifndef ASMJIT_NO_TEXT
Error instIdToString(InstId instId, InstStringifyOptions options, String& output) noexcept {
  uint32_t realId = instId & uint32_t(InstIdParts::kRealId);
  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId))) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  return InstNameUtils::decode(InstDB::_instNameIndexTable[realId], options, InstDB::_instNameStringTable, output);
}

InstId stringToInstId(const char* s, size_t len) noexcept {
  if (ASMJIT_UNLIKELY(!s)) {
    return BaseInst::kIdNone;
  }

  if (len == SIZE_MAX) {
    len = strlen(s);
  }

  if (len == 0u || len > InstDB::instNameIndex.maxNameLength) {
    return BaseInst::kIdNone;
  }

  return InstNameUtils::findInstruction(s, len, InstDB::_instNameIndexTable, InstDB::_instNameStringTable, InstDB::instNameIndex);
}
#endif // !ASMJIT_NO_TEXT

// a64::InstInternal - Validate
// ============================

#ifndef ASMJIT_NO_VALIDATION
ASMJIT_FAVOR_SIZE Error validate(const BaseInst& inst, const Operand_* operands, size_t opCount, ValidationFlags validationFlags) noexcept {
  // TODO:
  DebugUtils::unused(inst, operands, opCount, validationFlags);
  return kErrorOk;
}
#endif // !ASMJIT_NO_VALIDATION

// a64::InstInternal - QueryRWInfo
// ===============================

#ifndef ASMJIT_NO_INTROSPECTION
struct InstRWInfoData {
  uint8_t rwx[Globals::kMaxOpCount];
};

static const InstRWInfoData instRWInfoData[] = {
  #define R uint8_t(OpRWFlags::kRead)
  #define W uint8_t(OpRWFlags::kWrite)
  #define X uint8_t(OpRWFlags::kRW)

  {{ R, R, R, R, R, R }}, // kRWI_R
  {{ R, W, R, R, R, R }}, // kRWI_RW
  {{ R, X, R, R, R, R }}, // kRWI_RX
  {{ R, R, W, R, R, R }}, // kRWI_RRW
  {{ R, W, X, R, R, R }}, // kRWI_RWX
  {{ W, R, R, R, R, R }}, // kRWI_W
  {{ W, R, W, R, R, R }}, // kRWI_WRW
  {{ W, R, X, R, R, R }}, // kRWI_WRX
  {{ W, R, R, W, R, R }}, // kRWI_WRRW
  {{ W, R, R, X, R, R }}, // kRWI_WRRX
  {{ W, W, R, R, R, R }}, // kRWI_WW
  {{ X, R, R, R, R, R }}, // kRWI_X
  {{ X, R, X, R, R, R }}, // kRWI_XRX
  {{ X, X, R, R, X, R }}, // kRWI_XXRRX

  {{ W, R, R, R, R, R }}, // kRWI_LDn
  {{ R, W, R, R, R, R }}, // kRWI_STn
  {{ R, R, R, R, R, R }}  // kRWI_TODO

  #undef R
  #undef W
  #undef X
};

static const uint8_t elementTypeSize[8] = { 0, 1, 2, 4, 8, 4, 4, 0 };

Error queryRWInfo(const BaseInst& inst, const Operand_* operands, size_t opCount, InstRWInfo* out) noexcept {
  // Get the instruction data.
  uint32_t realId = inst.id() & uint32_t(InstIdParts::kRealId);

  if (ASMJIT_UNLIKELY(!Inst::isDefinedId(realId))) {
    return DebugUtils::errored(kErrorInvalidInstruction);
  }

  out->_instFlags = InstRWFlags::kNone;
  out->_opCount = uint8_t(opCount);
  out->_rmFeature = 0;
  out->_extraReg.reset();
  out->_readFlags = CpuRWFlags::kNone; // TODO: [ARM] Read PSTATUS.
  out->_writeFlags = CpuRWFlags::kNone; // TODO: [ARM] Write PSTATUS

  const InstDB::InstInfo& instInfo = InstDB::_instInfoTable[realId];
  const InstRWInfoData& rwInfo = instRWInfoData[instInfo.rwInfoIndex()];

  if (instInfo.hasFlag(InstDB::kInstFlagConsecutive) && opCount > 2) {
    for (uint32_t i = 0; i < opCount; i++) {
      OpRWInfo& op = out->_operands[i];
      const Operand_& srcOp = operands[i];

      if (!srcOp.isRegOrMem()) {
        op.reset();
        continue;
      }

      OpRWFlags rwFlags = i < opCount - 1 ? (OpRWFlags)rwInfo.rwx[0] : (OpRWFlags)rwInfo.rwx[1];

      op._opFlags = rwFlags & ~(OpRWFlags::kZExt);
      op._physId = Reg::kIdBad;
      op._rmSize = 0;
      op._resetReserved();

      uint64_t rByteMask = op.isRead() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
      uint64_t wByteMask = op.isWrite() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;

      op._readByteMask = rByteMask;
      op._writeByteMask = wByteMask;
      op._extendByteMask = 0;
      op._consecutiveLeadCount = 0;

      if (srcOp.isReg()) {
        if (i == 0) {
          op._consecutiveLeadCount = uint8_t(opCount - 1);
        }
        else {
          op.addOpFlags(OpRWFlags::kConsecutive);
        }
      }
      else {
        const Mem& memOp = srcOp.as<Mem>();

        if (memOp.hasBase()) {
          op.addOpFlags(OpRWFlags::kMemBaseRead);
          if ((memOp.hasIndex() || memOp.hasOffset()) && memOp.isPreOrPost()) {
            op.addOpFlags(OpRWFlags::kMemBaseWrite);
          }
        }

        if (memOp.hasIndex()) {
          op.addOpFlags(OpRWFlags::kMemIndexRead);
        }
      }
    }
  }
  else {
    for (uint32_t i = 0; i < opCount; i++) {
      OpRWInfo& op = out->_operands[i];
      const Operand_& srcOp = operands[i];

      if (!srcOp.isRegOrMem()) {
        op.reset();
        continue;
      }

      OpRWFlags rwFlags = (OpRWFlags)rwInfo.rwx[i];

      op._opFlags = rwFlags & ~(OpRWFlags::kZExt);
      op._physId = Reg::kIdBad;
      op._rmSize = 0;
      op._resetReserved();

      uint64_t rByteMask = op.isRead() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
      uint64_t wByteMask = op.isWrite() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;

      op._readByteMask = rByteMask;
      op._writeByteMask = wByteMask;
      op._extendByteMask = 0;
      op._consecutiveLeadCount = 0;

      if (srcOp.isReg()) {
        if (srcOp.as<Vec>().hasElementIndex()) {
          // Only part of the vector is accessed if element index [] is used.
          VecElementType elementType = srcOp.as<Vec>().elementType();
          uint32_t elementIndex = srcOp.as<Vec>().elementIndex();

          uint32_t elementSize = elementTypeSize[size_t(elementType)];
          uint64_t accessMask = uint64_t(Support::lsbMask<uint32_t>(elementSize)) << (elementIndex * elementSize);

          op._readByteMask &= accessMask;
          op._writeByteMask &= accessMask;
        }

        // TODO: [ARM] RW info is not finished.
      }
      else {
        const Mem& memOp = srcOp.as<Mem>();

        if (memOp.hasBase()) {
          op.addOpFlags(OpRWFlags::kMemBaseRead);
          if ((memOp.hasIndex() || memOp.hasOffset()) && memOp.isPreOrPost()) {
            op.addOpFlags(OpRWFlags::kMemBaseWrite);
          }
        }

        if (memOp.hasIndex()) {
          op.addOpFlags(OpRWFlags::kMemIndexRead);
        }
      }
    }
  }

  return kErrorOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

// a64::InstInternal - QueryFeatures
// =================================

#ifndef ASMJIT_NO_INTROSPECTION
Error queryFeatures(const BaseInst& inst, const Operand_* operands, size_t opCount, CpuFeatures* out) noexcept {
  // TODO: [ARM] QueryFeatures not implemented yet.
  DebugUtils::unused(inst, operands, opCount, out);
  return kErrorOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

// a64::InstInternal - Unit
// ========================

#if defined(ASMJIT_TEST)
UNIT(arm_inst_api_text) {
  // TODO:
}
#endif

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH64
