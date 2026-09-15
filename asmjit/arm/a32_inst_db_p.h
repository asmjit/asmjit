// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32_INST_DB_H_P_INCLUDED
#define ASMJIT_ARM_A32_INST_DB_H_P_INCLUDED

#include <asmjit/core/code_holder.h>
#include <asmjit/core/inst_db_p.h>
#include <asmjit/arm/a32_operand.h>

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \cond INTERNAL
//! \addtogroup asmjit_a32
//! \{

namespace InstDB {

// a32::InstDB - Tables
// ====================

#ifndef ASMJIT_NO_TEXT
extern const InstNameIndex inst_name_index;
extern const char _inst_name_string_table[];
extern const uint32_t _inst_name_index_table[];
#endif // !ASMJIT_NO_TEXT

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_ARM_A32_INST_DB_H_P_INCLUDED

