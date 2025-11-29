// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32FORMATTER_P_H_INCLUDED
#define ASMJIT_ARM_A32FORMATTER_P_H_INCLUDED

#include <asmjit/core/api-config.h>
#ifndef ASMJIT_NO_LOGGING

#include <asmjit/core/formatter.h>
#include <asmjit/core/string.h>
#include <asmjit/arm/armformatter_p.h>
#include <asmjit/arm/a32globals.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \cond INTERNAL
//! \addtogroup asmjit_a32
//! \{

namespace FormatterInternal {

using namespace arm::FormatterInternal;

Error ASMJIT_CDECL format_instruction(
  String& sb,
  FormatFlags format_flags,
  const BaseEmitter* emitter,
  Arch arch,
  const BaseInst& inst, Span<const Operand_> operands) noexcept;

} // {FormatterInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
#endif // ASMJIT_ARM_A32FORMATTER_P_H_INCLUDED
