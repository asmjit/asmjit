// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_STRINGUTILS_H
#define _ASMJIT_CORE_STRINGUTILS_H

// [Dependencies]
#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::StaticString]
// ============================================================================

template<size_t N>
union StaticString {
  enum : uint32_t {
    kNumU32 = uint32_t((N + sizeof(uint32_t) - 1) / sizeof(uint32_t))
  };

  inline bool test(const char* other) const noexcept { return std::strcmp(str, other) == 0; }

  char str[kNumU32 * sizeof(uint32_t)];
  uint32_t u32[kNumU32];
};

// ============================================================================
// [asmjit::StringUtils]
// ============================================================================

namespace StringUtils {
  // \internal
  static constexpr uint32_t hashRound(uint32_t hash, uint32_t c) noexcept { return hash * 65599 + c; }

  // Get a hash of the given string `data` of size `size`. Size must be valid
  // as this function doesn't check for a null terminator and allows it in the
  // middle of the string.
  static inline uint32_t hashString(const char* data, size_t size) noexcept {
    uint32_t hashCode = 0;
    for (uint32_t i = 0; i < size; i++)
      hashCode = hashRound(hashCode, uint8_t(data[i]));
    return hashCode;
  }

  static ASMJIT_INLINE size_t strLen(const char* s, size_t maxSize) noexcept {
    size_t i = 0;
    while (i < maxSize && s[i] != '\0')
      i++;
    return i;
  }

  static ASMJIT_INLINE const char* findPackedString(const char* p, uint32_t id) noexcept {
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
  //! `b` is a non-null terminated instruction name passed to `Inst::idByName()`.
  static ASMJIT_INLINE int cmpInstName(const char* a, const char* b, size_t size) noexcept {
    for (size_t i = 0; i < size; i++) {
      int c = int(uint8_t(a[i])) - int(uint8_t(b[i]));
      if (c != 0) return c;
    }
    return int(uint8_t(a[size]));
  }
}

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_STRINGUTILS_H
