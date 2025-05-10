// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_INSTDB_P_H_INCLUDED
#define ASMJIT_CORE_INSTDB_P_H_INCLUDED

#include "../core/inst.h"
#include "../core/string.h"

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_instruction_db
//! \{

struct InstNameIndex {
  struct Span {
    uint16_t start;
    uint16_t end;
  };

  Span data[26];
  uint16_t maxNameLength;
};

namespace InstNameUtils {

Error decode(uint32_t nameValue, InstStringifyOptions options, const char* stringTable, String& output) noexcept;
InstId findInstruction(const char* s, size_t len, const uint32_t* nameTable, const char* stringTable, const InstNameIndex& nameIndex) noexcept;
uint32_t findAlias(const char* s, size_t len, const uint32_t* nameTable, const char* stringTable, uint32_t aliasNameCount) noexcept;

} // {InstNameUtils}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_INSTDB_P_H_INCLUDED
