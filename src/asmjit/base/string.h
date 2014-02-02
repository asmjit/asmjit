// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_STRING_H
#define _ASMJIT_BASE_STRING_H

// [Dependencies - AsmJit]
#include "../base/assert.h"
#include "../base/defs.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_base
//! @{

// ============================================================================
// [asmjit::kStringOp]
// ============================================================================

//! @brief String operation.
ASMJIT_ENUM(kStringOp) {
  //! @brief Replace the current string by a given content.
  kStringOpSet = 0,
  //! @brief Append a given content to the current string.
  kStringOpAppend = 1
};

// ============================================================================
// [asmjit::kStringFormat]
// ============================================================================

//! @brief String format flags.
ASMJIT_ENUM(kStringFormat) {
  kStringFormatShowSign  = 0x00000001,
  kStringFormatShowSpace = 0x00000002,
  kStringFormatAlternate = 0x00000004,
  kStringFormatSigned    = 0x80000000
};

// ============================================================================
// [asmjit::StringUtil]
// ============================================================================

//! @brief String utilities.
struct StringUtil {
  static ASMJIT_INLINE size_t nlen(const char* s, size_t maxlen) {
    size_t i;
    for (i = 0; i < maxlen; i++)
      if (!s[i])
        break;
    return i;
  }
};

// ============================================================================
// [asmjit::StringBuilder]
// ============================================================================

//! @brief String builder.
//!
//! String builder was designed to be able to build a string using append like
//! operation to append numbers, other strings, or signle characters. It can
//! allocate it's own buffer or use a buffer created on the stack.
//!
//! String builder contains method specific to AsmJit functionality, used for
//! logging or HTML output.
struct StringBuilder {
  ASMJIT_NO_COPY(StringBuilder)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API StringBuilder();
  ASMJIT_API ~StringBuilder();

  ASMJIT_INLINE StringBuilder(const _DontInitialize&) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get string builder capacity.
  ASMJIT_INLINE size_t getCapacity() const { return _capacity; }
  //! @brief Get length.
  ASMJIT_INLINE size_t getLength() const { return _length; }

  //! @brief Get null-terminated string data.
  ASMJIT_INLINE char* getData() { return _data; }
  //! @brief Get null-terminated string data (const).
  ASMJIT_INLINE const char* getData() const { return _data; }

  // --------------------------------------------------------------------------
  // [Prepare / Reserve]
  // --------------------------------------------------------------------------

  //! @brief Prepare to set/append.
  ASMJIT_API char* prepare(uint32_t op, size_t len);

  //! @brief Reserve @a to bytes in string builder.
  ASMJIT_API bool reserve(size_t to);

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  //! @brief Clear the content in String builder.
  ASMJIT_API void clear();

  // --------------------------------------------------------------------------
  // [Op]
  // --------------------------------------------------------------------------

  ASMJIT_API bool _opString(uint32_t op, const char* str, size_t len = kInvalidIndex);
  ASMJIT_API bool _opVFormat(uint32_t op, const char* fmt, va_list ap);
  ASMJIT_API bool _opChar(uint32_t op, char c);
  ASMJIT_API bool _opChars(uint32_t op, char c, size_t len);
  ASMJIT_API bool _opNumber(uint32_t op, uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0);
  ASMJIT_API bool _opHex(uint32_t op, const void* data, size_t len);

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  //! @brief Replace the current content by @a str of @a len.
  ASMJIT_INLINE bool setString(const char* str, size_t len = kInvalidIndex)
  { return _opString(kStringOpSet, str, len); }

  //! @brief Replace the current content by formatted string @a fmt.
  ASMJIT_INLINE bool setVFormat(const char* fmt, va_list ap)
  { return _opVFormat(kStringOpSet, fmt, ap); }

  //! @brief Replace the current content by formatted string @a fmt.
  ASMJIT_API bool setFormat(const char* fmt, ...);

  //! @brief Replace the current content by @a c character.
  ASMJIT_INLINE bool setChar(char c)
  { return _opChar(kStringOpSet, c); }

  //! @brief Replace the current content by @a c of @a len.
  ASMJIT_INLINE bool setChars(char c, size_t len)
  { return _opChars(kStringOpSet, c, len); }

  //! @brief Replace the current content by @a i..
  ASMJIT_INLINE bool setInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0)
  { return _opNumber(kStringOpSet, i, base, width, flags | kStringFormatSigned); }

  //! @brief Replace the current content by @a i..
  ASMJIT_INLINE bool setUInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0)
  { return _opNumber(kStringOpSet, i, base, width, flags); }

  //! @brief Replace the current content by the given @a data converted to a HEX string.
  ASMJIT_INLINE bool setHex(const void* data, size_t len)
  { return _opHex(kStringOpSet, data, len); }

  // --------------------------------------------------------------------------
  // [Append]
  // --------------------------------------------------------------------------

  //! @brief Append @a str of @a len.
  ASMJIT_INLINE bool appendString(const char* str, size_t len = kInvalidIndex)
  { return _opString(kStringOpAppend, str, len); }

  //! @brief Append a formatted string @a fmt to the current content.
  ASMJIT_INLINE bool appendVFormat(const char* fmt, va_list ap)
  { return _opVFormat(kStringOpAppend, fmt, ap); }

  //! @brief Append a formatted string @a fmt to the current content.
  ASMJIT_API bool appendFormat(const char* fmt, ...);

  //! @brief Append @a c character.
  ASMJIT_INLINE bool appendChar(char c)
  { return _opChar(kStringOpAppend, c); }

  //! @brief Append @a c of @a len.
  ASMJIT_INLINE bool appendChars(char c, size_t len)
  { return _opChars(kStringOpAppend, c, len); }

  //! @brief Append @a i.
  ASMJIT_INLINE bool appendInt(int64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0)
  { return _opNumber(kStringOpAppend, static_cast<uint64_t>(i), base, width, flags | kStringFormatSigned); }

  //! @brief Append @a i.
  ASMJIT_INLINE bool appendUInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0)
  { return _opNumber(kStringOpAppend, i, base, width, flags); }

  //! @brief Append the given @a data converted to a HEX string.
  ASMJIT_INLINE bool appendHex(const void* data, size_t len)
  { return _opHex(kStringOpAppend, data, len); }

  // --------------------------------------------------------------------------
  // [_Append]
  // --------------------------------------------------------------------------

  //! @brief Append @a str of @a len (inlined, without buffer overflow check).
  ASMJIT_INLINE void _appendString(const char* str, size_t len = kInvalidIndex) {
    // len should be a constant if we are inlining.
    if (len == kInvalidIndex) {
      char* p = &_data[_length];

      while (*str) {
        ASMJIT_ASSERT(p < _data + _capacity);
        *p++ = *str++;
      }

      *p = '\0';
      _length = (size_t)(p - _data);
    }
    else {
      ASMJIT_ASSERT(_capacity - _length >= len);

      char* p = &_data[_length];
      char* pEnd = p + len;

      while (p < pEnd)
        *p++ = *str++;

      *p = '\0';
      _length += len;
    }
  }

  //! @brief Append @a c character (inlined, without buffer overflow check).
  ASMJIT_INLINE void _appendChar(char c) {
    ASMJIT_ASSERT(_capacity - _length >= 1);

    _data[_length] = c;
    _length++;
    _data[_length] = '\0';
  }

  //! @brief Append @a c of @a len (inlined, without buffer overflow check).
  ASMJIT_INLINE void _appendChars(char c, size_t len) {
    ASMJIT_ASSERT(_capacity - _length >= len);

    char* p = &_data[_length];
    char* pEnd = p + len;

    while (p < pEnd)
      *p++ = c;

    *p = '\0';
    _length += len;
  }

  ASMJIT_INLINE void _appendUInt32(uint32_t i) {
    char buf_[32];

    char* pEnd = buf_ + ASMJIT_ARRAY_SIZE(buf_);
    char* pBuf = pEnd;

    do {
      uint32_t d = i / 10;
      uint32_t r = i % 10;

      *--pBuf = static_cast<uint8_t>(r + '0');
      i = d;
    } while (i);

    ASMJIT_ASSERT(_capacity - _length >= (size_t)(pEnd - pBuf));
    char* p = &_data[_length];

    do {
      *p++ = *pBuf;
    } while (++pBuf != pEnd);

    *p = '\0';
    _length = (size_t)(p - _data);
  }

  // --------------------------------------------------------------------------
  // [Eq]
  // --------------------------------------------------------------------------

  //! @brief Check for equality with other @a str.
  ASMJIT_API bool eq(const char* str, size_t len = kInvalidIndex) const;
  //! @brief Check for equality with StringBuilder @a other.
  ASMJIT_INLINE bool eq(const StringBuilder& other) const { return eq(other._data); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool operator==(const StringBuilder& other) const { return  eq(other); }
  ASMJIT_INLINE bool operator!=(const StringBuilder& other) const { return !eq(other); }

  ASMJIT_INLINE bool operator==(const char* str) const { return  eq(str); }
  ASMJIT_INLINE bool operator!=(const char* str) const { return !eq(str); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief String data.
  char* _data;
  //! @brief Length.
  size_t _length;
  //! @brief Capacity.
  size_t _capacity;
  //! @brief Whether the string can be freed.
  size_t _canFree;
};

// ============================================================================
// [asmjit::StringBuilderT]
// ============================================================================

template<size_t N>
struct StringBuilderT : public StringBuilder {
  ASMJIT_NO_COPY(StringBuilderT<N>)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE StringBuilderT() : StringBuilder(DontInitialize) {
    _data = _embeddedData;
    _data[0] = 0;

    _length = 0;
    _capacity = N;
    _canFree = false;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Embedded data.
  char _embeddedData[static_cast<size_t>(N + 1 + sizeof(intptr_t)) & ~static_cast<size_t>(sizeof(intptr_t) - 1)];
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

#endif // _ASMJIT_BASE_STRING_H
