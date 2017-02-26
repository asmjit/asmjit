// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_STRINGUTILS_H
#define _ASMJIT_CORE_STRINGUTILS_H

// [Dependencies]
#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE
namespace StringUtils {

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::StringUtils]
// ============================================================================

template<typename T>
static constexpr T toLower(T c) noexcept { return c ^ (T(c >= T('A') && c <= T('Z')) << 5); }

template<typename T>
static constexpr T toUpper(T c) noexcept { return c ^ (T(c >= T('a') && c <= T('z')) << 5); }

// \internal
static constexpr uint32_t hashRound(uint32_t hash, uint32_t c) noexcept { return hash * 65599 + c; }

// Get a hash of the given string `str` of `len` length. Length must be valid
// as this function doesn't check for a null terminator and allows it in the
// middle of the string.
static inline uint32_t hashString(const char* str, size_t len) noexcept {
  uint32_t hVal = 0;
  for (uint32_t i = 0; i < len; i++)
    hVal = hashRound(hVal, uint8_t(str[i]));
  return hVal;
}

static ASMJIT_FORCEINLINE size_t strLen(const char* s, size_t maxlen) noexcept {
  size_t i;
  for (i = 0; i < maxlen && s[i] != '\0'; i++)
    continue;
  return i;
}

static ASMJIT_FORCEINLINE const char* findPackedString(const char* p, uint32_t id) noexcept {
  uint32_t i = 0;
  while (i < id) {
    while (p[0])
      p++;
    p++;
    i++;
  }
  return p;
}

//! Compare two instruction names.
//!
//! `a` is a null terminated instruction name from arch-specific `nameData[]` table.
//! `b` is a non-null terminated instruction name passed to `???Inst::getIdByName()`.
static ASMJIT_FORCEINLINE int cmpInstName(const char* a, const char* b, size_t len) noexcept {
  for (size_t i = 0; i < len; i++) {
    int c = int(uint8_t(a[i])) - int(uint8_t(b[i]));
    if (c != 0) return c;
  }
  return int(uint8_t(a[len]));
}

//! \}

} // StringUtils namespace
ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_STRINGUTILS_H
