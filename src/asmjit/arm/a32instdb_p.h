// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_ARM_A32INSTDB_H_P_INCLUDED
#define ASMJIT_ARM_A32INSTDB_H_P_INCLUDED

#include "../core/codeholder.h"
// #include "../arm/a32instdb.h"
#include "../arm/a32operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(a32)

//! \cond INTERNAL
//! \addtogroup asmjit_a32
//! \{

namespace InstDB {

// a32::InstDB - EncodingId
// ========================

// ${EncodingId:Begin}
// ------------------- Automatically generated, do not edit -------------------
enum EncodingId : uint32_t {
  kEncodingNone = 0
};
// ----------------------------------------------------------------------------
// ${EncodingId:End}

// a32::InstDB::EncodingData
// =========================

namespace EncodingData {

// ${EncodingDataForward:Begin}
// ${EncodingDataForward:End}

} // {EncodingData}

// a32::InstDB - InstNameIndex
// ===========================

// ${NameLimits:Begin}
// ------------------- Automatically generated, do not edit -------------------
enum : uint32_t { kMaxNameSize = 9 };
// ----------------------------------------------------------------------------
// ${NameLimits:End}

struct InstNameIndex {
  uint16_t start;
  uint16_t end;
};

// a32::InstDB - Tables
// ====================

#ifndef ASMJIT_NO_TEXT
extern const char _nameData[];
extern const InstNameIndex instNameIndex[26];
#endif // !ASMJIT_NO_TEXT

} // {InstDB}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_A32_ARMINSTDB_H_P_INCLUDED

