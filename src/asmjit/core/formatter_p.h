// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FORMATTER_P_H_INCLUDED
#define ASMJIT_CORE_FORMATTER_P_H_INCLUDED

#include "../core/compilerdefs.h"
#include "../core/formatter.h"
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_logging
//! \{

namespace Formatter {

[[maybe_unused]]
static ASMJIT_INLINE size_t paddingFromOptions(const FormatOptions& formatOptions, FormatPaddingGroup group) noexcept {
  static constexpr uint16_t _defaultPaddingTable[uint32_t(FormatPaddingGroup::kMaxValue) + 1] = { 44, 26 };
  static_assert(uint32_t(FormatPaddingGroup::kMaxValue) + 1 == 2, "If a new group is defined it must be added here");

  size_t padding = formatOptions.padding(group);
  return padding ? padding : size_t(_defaultPaddingTable[uint32_t(group)]);
}

Error formatVirtRegName(String& sb, const VirtReg* vReg) noexcept;
Error formatVirtRegNameWithPrefix(String& sb, const char* prefix, size_t prefixSize, const VirtReg* vReg) noexcept;

} // {Formatter}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FORMATTER_H_P_INCLUDED
