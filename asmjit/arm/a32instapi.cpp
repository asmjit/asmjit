// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH32)

#include <asmjit/support/support_p.h>
#include <asmjit/arm/a32instapi_p.h>
#include <asmjit/arm/a32instdb_p.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

namespace InstInternal {

// a32::InstAPI - Text
// ===================

#ifndef ASMJIT_NO_TEXT
Error inst_id_to_string(InstId inst_id, InstStringifyOptions options, String& output) noexcept {
  uint32_t real_id = inst_id & uint32_t(InstIdParts::kRealId);
  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(real_id)))
    return make_error(Error::kInvalidInstruction);

  return InstNameUtils::decode(InstDB::_inst_name_index_table[real_id], options, InstDB::_inst_name_string_table, output);
}

InstId string_to_inst_id(const char* s, size_t len) noexcept {
  return InstNameUtils::find_instruction(s, len, InstDB::_inst_name_index_table, InstDB::_inst_name_string_table, InstDB::inst_name_index);
}
#endif // !ASMJIT_NO_TEXT

// a32::InstAPI - Validation
// =========================

#ifndef ASMJIT_NO_VALIDATION
Error validate(const BaseInst& inst, const Operand_* operands, size_t op_count, ValidationFlags validation_flags) noexcept {
  // TODO: AArch32 tooling.
  Support::maybe_unused(inst, operands, op_count, validation_flags);

  return kErrorOk;
}
#endif // !ASMJIT_NO_VALIDATION

// a32::InstAPI - Introspection
// ============================

#ifndef ASMJIT_NO_INTROSPECTION

struct InstRWInfoRecord {
  //! RWX information for each operand.
  uint8_t rwx[Globals::kMaxOpCount];
  //! Index to InstRWFlagsRecord table.
  uint8_t rw_flags_index;
};

struct InstRWFlagsRecord {
  //! Read flags.
  CpuRWFlags r;
  //! Written flags.
  CpuRWFlags w;
};

// ${a32::RWInfo:Begin}
// ------------------- Automatically generated, do not edit -------------------
static const constexpr InstRWInfoRecord inst_rw_info_data[] = {
  #define R uint8_t(OpRWFlags::kRead)
  #define W uint8_t(OpRWFlags::kWrite)
  #define X uint8_t(OpRWFlags::kRW)

  {{ R, R, R, R, R, R }, 0}, // #0 [ref=105x]
  {{ W, R, R, R, R, R }, 1}, // #1 [ref=3x]
  {{ W, R, R, R, R, R }, 2}, // #2 [ref=1x]
  {{ W, R, R, R, R, R }, 0}, // #3 [ref=243x]
  {{ W, R, R, R, R, R }, 3}, // #4 [ref=5x]
  {{ X, R, R, R, R, R }, 0}, // #5 [ref=32x]
  {{ W, R, R, R, R, R }, 4}, // #6 [ref=11x]
  {{ R, R, R, R, R, R }, 3}, // #7 [ref=2x]
  {{ W, W, R, R, R, R }, 0}, // #8 [ref=6x]
  {{ W, R, R, R, R, R }, 5}, // #9 [ref=2x]
  {{ R, R, W, R, R, R }, 0}, // #10 [ref=2x]
  {{ R, R, W, W, R, R }, 0}, // #11 [ref=2x]
  {{ W, R, R, R, R, R }, 6}, // #12 [ref=20x]
  {{ W, R, R, R, R, R }, 7}, // #13 [ref=12x]
  {{ W, R, R, R, R, R }, 8}, // #14 [ref=1x]
  {{ X, X, R, R, R, R }, 0}, // #15 [ref=13x]
  {{ X, X, R, R, R, R }, 5}, // #16 [ref=2x]
  {{ X, X, R, R, R, R }, 6}, // #17 [ref=2x]
  {{ W, W, R, R, R, R }, 5}, // #18 [ref=2x]
  {{ R, R, R, R, R, R }, 4}, // #19 [ref=2x]
  {{ W, W, W, R, R, R }, 0}, // #20 [ref=2x]
  {{ W, W, W, W, R, R }, 0}  // #21 [ref=2x]

  #undef R
  #undef W
  #undef X
};

static const constexpr InstRWFlagsRecord instRWFlagsData[] = {
  { CpuRWFlags::kNone, CpuRWFlags::kNone }, // #0 [ref=407x]
  { CpuRWFlags::kARM_C, CpuRWFlags::kNone }, // #1 [ref=3x]
  { CpuRWFlags::kARM_C, CpuRWFlags::kARM_C | CpuRWFlags::kARM_N | CpuRWFlags::kARM_V | CpuRWFlags::kARM_Z }, // #2 [ref=1x]
  { CpuRWFlags::kNone, CpuRWFlags::kARM_C | CpuRWFlags::kARM_N | CpuRWFlags::kARM_V | CpuRWFlags::kARM_Z }, // #3 [ref=7x]
  { CpuRWFlags::kNone, CpuRWFlags::kARM_C | CpuRWFlags::kARM_N | CpuRWFlags::kARM_Z }, // #4 [ref=13x]
  { CpuRWFlags::kNone, CpuRWFlags::kARM_N | CpuRWFlags::kARM_Z }, // #5 [ref=6x]
  { CpuRWFlags::kARM_Q, CpuRWFlags::kARM_Q }, // #6 [ref=22x]
  { CpuRWFlags::kNone, CpuRWFlags::kARM_GE }, // #7 [ref=12x]
  { CpuRWFlags::kARM_GE, CpuRWFlags::kNone }  // #8 [ref=1x]
};

static const constexpr uint8_t instRWInfoIndex[] {
  0, 1, 2, 3, 4, 3, 5, 5, 3, 3, 3, 6, 3, 6, 0, 3, 3, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 7, 7, 0, 0, 0, 3, 3, 3, 3, 3, 3,
  0, 0, 0, 0, 0, 0, 0, 3, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 8, 3, 3, 0, 0, 0, 0,
  3, 3, 3, 8, 3, 3, 8, 3, 3, 3, 3, 3, 3, 3, 3, 3, 6, 3, 6, 0, 0, 0, 0, 3, 9, 3, 0xFFu, 0xFFu, 5, 3, 10, 10, 11, 11, 3, 0,
  3, 9, 3, 6, 0, 3, 6, 3, 6, 3, 3, 0, 0, 0, 0xFFu, 0, 0, 12, 3, 3, 3, 12, 12, 3, 12, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 3, 6,
  1, 6, 3, 4, 3, 4, 13, 13, 13, 0, 1, 4, 3, 3, 14, 0, 0, 0, 0, 5, 3, 5, 5, 5, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 3, 0, 12, 12,
  12, 12, 15, 15, 15, 15, 15, 16, 15, 15, 12, 12, 12, 12, 12, 12, 17, 17, 3, 3, 3, 3, 3, 3, 12, 12, 3, 3, 8, 18, 3, 3, 3,
  3, 3, 3, 0, 0, 0, 0, 12, 12, 13, 0, 13, 13, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 3, 4, 0,
  3, 3, 3, 3, 3, 3, 0, 0, 19, 19, 13, 13, 13, 3, 0, 3, 3, 3, 3, 3, 3, 3, 15, 15, 16, 8, 18, 3, 3, 3, 3, 3, 3, 3, 3, 12,
  12, 13, 13, 13, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0xFFu, 0xFFu, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  5, 0, 0, 3, 0xFFu, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 5, 3, 5, 5, 5, 5, 3, 3, 3, 3, 0xFFu, 0xFFu, 0xFFu, 8, 20,
  20, 21, 21, 0, 0, 3, 3, 3, 3, 3, 5, 5, 5, 5, 3, 0xFFu, 3, 3, 3, 3, 3, 3, 3, 5, 5, 3, 0xFFu, 0xFFu, 5, 3, 3, 3, 3, 0, 0,
  3, 3, 5, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 3, 5, 5, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 15, 3, 3, 15, 3, 3, 3, 3, 3, 15, 15, 0,
  0, 0
};
// ----------------------------------------------------------------------------
// ${a32::RWInfo:End}

Error query_rw_info(const BaseInst& inst, const Operand_* operands, size_t op_count, InstRWInfo* out) noexcept {
  uint32_t inst_id = inst.inst_id();
  uint32_t real_id = inst_id & uint32_t(InstIdParts::kRealId);

  if (ASMJIT_UNLIKELY(!Inst::is_defined_id(real_id)))
    return make_error(Error::kInvalidInstruction);

  out->_inst_flags = InstRWFlags::kNone;
  out->_op_count = uint8_t(op_count);
  out->_rm_feature = 0;
  out->_extra_reg.reset();

  out->_read_flags = CpuRWFlags::kNone;
  out->_write_flags = CpuRWFlags::kNone;

  size_t index = instRWInfoIndex[real_id];
  if (index < 0xFFu) {
    const InstRWInfoRecord& rwInfo = inst_rw_info_data[index];
    size_t rw_flags_index = rwInfo.rw_flags_index;

    out->_read_flags = instRWFlagsData[rw_flags_index].r;
    out->_write_flags = instRWFlagsData[rw_flags_index].w;

    for (size_t i = 0; i < op_count; i++) {
      uint32_t access = rwInfo.rwx[i];

      OpRWInfo& rwOp = out->_operands[i];
      const Operand_& srcOp = operands[i];

      if (!srcOp.is_reg_or_reg_list_or_mem()) {
        rwOp.reset();
        continue;
      }

      rwOp._op_flags = OpRWFlags(access);
      rwOp._phys_id = Reg::kIdBad;
      rwOp._rm_size = 0;
      rwOp._reset_reserved();

      uint64_t r_byte_mask = rwOp.is_read() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;
      uint64_t w_byte_mask = rwOp.is_write() ? 0xFFFFFFFFFFFFFFFFu : 0x0000000000000000u;

      rwOp._read_byte_mask = r_byte_mask;
      rwOp._write_byte_mask = w_byte_mask;
      rwOp._extend_byte_mask = 0;
      rwOp._consecutive_lead_count = 0;

      if (srcOp.is_mem()) {
        const Mem& mem_op = srcOp.as<Mem>();

        if (mem_op.has_base()) {
          rwOp.add_op_flags(OpRWFlags::kMemBaseRead);
        }

        if (mem_op.has_index()) {
          rwOp.add_op_flags(mem_op.is_pre_or_post() ? OpRWFlags::kMemIndexRW : OpRWFlags::kMemIndexRead);
        }
      }
      else if (srcOp.as<Vec>().has_element_index()) {
        // Only part of the vector is accessed if element index [] is used.
        uint32_t elementSize = data_type_size(inst.arm_dt());
        uint32_t element_index = srcOp.as<Vec>().element_index();

        // NOTE: DataType must be present otherwise it's impossible to calculate the access flags.
        if (!elementSize)
          return make_error(Error::kInvalidInstruction);

        uint64_t accessMask = uint64_t(Support::lsb_mask<uint32_t>(elementSize)) << (element_index * elementSize);
        rwOp._read_byte_mask &= accessMask;
        rwOp._write_byte_mask &= accessMask;
      }
    }
    return kErrorOk;
  }
  else {
    // TODO: [ARM] Not finished introspection.
    return make_error(Error::kInvalidState);
  }
}

Error query_features(const BaseInst& inst, const Operand_* operands, size_t op_count, CpuFeatures* out) noexcept {
  // TODO: AArch32 tooling.
  Support::maybe_unused(inst, operands, op_count, out);

  return kErrorOk;
}
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH32
