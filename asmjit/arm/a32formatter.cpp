// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api-build_p.h>
#if !defined(ASMJIT_NO_AARCH32) && !defined(ASMJIT_NO_LOGGING)

#include <asmjit/core/misc_p.h>
#include <asmjit/support/support.h>
#include <asmjit/arm/a32formatter_p.h>
#include <asmjit/arm/a32instapi_p.h>
#include <asmjit/arm/a32instdb_p.h>
#include <asmjit/arm/a32operand.h>

#ifndef ASMJIT_NO_COMPILER
  #include <asmjit/core/compiler.h>
#endif

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

// a32::FormatterInternal - Format Instruction
// ===========================================

ASMJIT_FAVOR_SIZE Error FormatterInternal::format_instruction(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept {

  Support::maybe_unused(arch);

  // Format instruction options and instruction mnemonic.
  InstId inst_id = inst.real_id();
  if (inst_id < Inst::_kIdCount) {
    InstStringifyOptions stringifyOptions =
      Support::test(format_flags, FormatFlags::kShowAliases)
        ? InstStringifyOptions::kAliases
        : InstStringifyOptions::kNone;
    ASMJIT_PROPAGATE(InstInternal::inst_id_to_string(inst_id, stringifyOptions, sb));
  }
  else {
    ASMJIT_PROPAGATE(sb.append_format("[InstId=#%u]", unsigned(inst_id)));
  }

  CondCode cc = inst.arm_cond_code();
  if (cc != CondCode::kAL) {
    ASMJIT_PROPAGATE(sb.append('.'));
    ASMJIT_PROPAGATE(format_cond_code(sb, cc));
  }

  // Format instruction operands.
  for (size_t i = 0; i < operands.size(); i++) {
    const Operand_& op = operands[i];
    if (op.is_none())
      break;

    ASMJIT_PROPAGATE(sb.append(i == 0 ? " " : ", "));
    ASMJIT_PROPAGATE(format_operand(sb, format_flags, emitter, arch, op));
  }

  return kErrorOk;
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_AARCH32 && !ASMJIT_NO_LOGGING
