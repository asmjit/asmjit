// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_STRINGBUILDER_H
#define _ASMJIT_CORE_STRINGBUILDER_H

// [Dependencies]
#include "../core/memmgr.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::StringBuilder]
// ============================================================================

//! String builder.
//!
//! String builder was designed to be able to build a string using append like
//! operation to append numbers, other strings, or signle characters. It can
//! allocate it's own buffer or use a buffer created on the stack.
//!
//! String builder contains method specific to AsmJit functionality, used for
//! logging or HTML output.
class StringBuilder {
public:
  ASMJIT_NONCOPYABLE(StringBuilder)

  //! \internal
  //!
  //! String operation.
  enum StringOp : uint32_t {
    kStringOpSet           = 0,           //!< Replace the current string by a given content.
    kStringOpAppend        = 1            //!< Append a given content to the current string.
  };

  //! \internal
  //!
  //! String format flags.
  enum StringFormatFlags : uint32_t {
    kStringFormatShowSign  = 0x00000001u,
    kStringFormatShowSpace = 0x00000002u,
    kStringFormatAlternate = 0x00000004u,
    kStringFormatSigned    = 0x80000000u
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline StringBuilder() noexcept
    : _data(_embedded),
      _size(0),
      _capacity(0),
      _embeddedUInt(0) {}

  inline StringBuilder(Globals::Init_, size_t embeddedCapacity) noexcept
    : _data(_embedded),
      _size(0),
      _capacity(embeddedCapacity),
      _embeddedUInt(0) {}

  inline ~StringBuilder() noexcept {
    if (!isEmbedded())
      MemMgr::release(_data);
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get null-terminated string data.
  inline char* data() noexcept { return _data; }
  //! Get null-terminated string data (const).
  inline const char* data() const noexcept { return _data; }

  //! Get whether the string is empty.
  inline bool empty() const noexcept { return _size == 0; }
  //! Get size.
  inline size_t size() const noexcept { return _size; }
  //! Get capacity.
  inline size_t capacity() const noexcept { return _capacity; }

  //! Get whether the string is using a small embedded buffer which is not dynamically allocated.
  inline bool isEmbedded() const noexcept { return _data == _embedded; }

  // --------------------------------------------------------------------------
  // [Prepare / Reserve]
  // --------------------------------------------------------------------------

  //! Prepare to set/append.
  ASMJIT_API char* prepare(uint32_t op, size_t size) noexcept;

  //! Reserve `to` bytes in string builder.
  ASMJIT_API Error reserve(size_t to) noexcept;

  // --------------------------------------------------------------------------
  // [Clear / Truncate]
  // --------------------------------------------------------------------------

  //! Clear the content in String builder.
  inline void clear() noexcept {
    _size = 0;
    _data[0] = '\0';
  }

  //! Truncate the string to `maxLen` characters.
  inline void truncate(size_t maxLen) noexcept {
    _size = std::min<size_t>(_size, maxLen);
  }

  // --------------------------------------------------------------------------
  // [Op]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _opString(uint32_t op, const char* str, size_t size = Globals::kNullTerminated) noexcept;
  ASMJIT_API Error _opVFormat(uint32_t op, const char* fmt, std::va_list ap) noexcept;
  ASMJIT_API Error _opChar(uint32_t op, char c) noexcept;
  ASMJIT_API Error _opChars(uint32_t op, char c, size_t n) noexcept;
  ASMJIT_API Error _opNumber(uint32_t op, uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) noexcept;
  ASMJIT_API Error _opHex(uint32_t op, const void* data, size_t size, char separator = '\0') noexcept;

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  //! Replace the current string with `str` having `size` characters (or possibly null terminated).
  inline Error setString(const char* str, size_t size = Globals::kNullTerminated) noexcept {
    return _opString(kStringOpSet, str, size);
  }

  //! Replace the current content by a formatted string `fmt`.
  inline Error setFormat(const char* fmt, ...) noexcept {
    Error result;
    std::va_list ap;

    va_start(ap, fmt);
    result = _opVFormat(kStringOpSet, fmt, ap);
    va_end(ap);

    return result;
  }

  //! Replace the current content by a formatted string `fmt` (va_list version).
  inline Error setFormatVA(const char* fmt, std::va_list ap) noexcept {
    return _opVFormat(kStringOpSet, fmt, ap);
  }

  //! Replace the current content by a single `c` character.
  inline Error setChar(char c) noexcept {
    return _opChar(kStringOpSet, c);
  }

  //! Replace the current content by `c` character `n` times.
  inline Error setChars(char c, size_t n) noexcept {
    return _opChars(kStringOpSet, c, n);
  }

  //! Replace the current content by a formatted integer `i` (signed).
  inline Error setInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) noexcept {
    return _opNumber(kStringOpSet, i, base, width, flags | kStringFormatSigned);
  }

  //! Replace the current content by a formatted integer `i` (unsigned).
  inline Error setUInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) noexcept {
    return _opNumber(kStringOpSet, i, base, width, flags);
  }

  //! Replace the current content by the given `data` converted to a HEX string.
  inline Error setHex(const void* data, size_t size, char separator = '\0') noexcept {
    return _opHex(kStringOpSet, data, size, separator);
  }

  // --------------------------------------------------------------------------
  // [Append]
  // --------------------------------------------------------------------------

  //! Append string `str` of size `size` (or possibly null terminated).
  inline Error appendString(const char* str, size_t size = Globals::kNullTerminated) noexcept {
    return _opString(kStringOpAppend, str, size);
  }

  //! Append a formatted string `fmt`.
  inline Error appendFormat(const char* fmt, ...) noexcept {
    Error result;
    std::va_list ap;

    va_start(ap, fmt);
    result = _opVFormat(kStringOpAppend, fmt, ap);
    va_end(ap);

    return result;
  }

  //! Append a formatted string `fmt` (va_list version).
  inline Error appendFormatVA(const char* fmt, std::va_list ap) noexcept {
    return _opVFormat(kStringOpAppend, fmt, ap);
  }

  //! Append a single `c` character.
  inline Error appendChar(char c) noexcept {
    return _opChar(kStringOpAppend, c);
  }

  //! Append `c` character `n` times.
  inline Error appendChars(char c, size_t n) noexcept {
    return _opChars(kStringOpAppend, c, n);
  }

  ASMJIT_API Error padEnd(size_t n, char c = ' ') noexcept;

  //! Append `i`.
  inline Error appendInt(int64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) noexcept {
    return _opNumber(kStringOpAppend, uint64_t(i), base, width, flags | kStringFormatSigned);
  }

  //! Append `i`.
  inline Error appendUInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) noexcept {
    return _opNumber(kStringOpAppend, i, base, width, flags);
  }

  //! Append the given `data` converted to a HEX string.
  inline Error appendHex(const void* data, size_t size, char separator = '\0') noexcept {
    return _opHex(kStringOpAppend, data, size, separator);
  }

  // --------------------------------------------------------------------------
  // [Eq]
  // --------------------------------------------------------------------------

  //! Check for equality with other `str` of size `size`.
  ASMJIT_API bool eq(const char* str, size_t size = Globals::kNullTerminated) const noexcept;
  //! Check for equality with `other`.
  inline bool eq(const StringBuilder& other) const noexcept { return eq(other._data, other._size); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline bool operator==(const StringBuilder& other) const noexcept { return  eq(other); }
  inline bool operator!=(const StringBuilder& other) const noexcept { return !eq(other); }

  inline bool operator==(const char* str) const noexcept { return  eq(str); }
  inline bool operator!=(const char* str) const noexcept { return !eq(str); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  char* _data;                           //!< String data.
  size_t _size;                          //!< String size.
  size_t _capacity;                      //!< String capacity.

  union {
    uintptr_t _embeddedUInt;             //!< Embedded data casted to `uintptr_t`.
    char _embedded[sizeof(uintptr_t)];   //!< Embedded data for very small strings.
  };
};

// ============================================================================
// [asmjit::StringBuilderTmp]
// ============================================================================

//! Temporary string builder, has statically allocated `N` bytes.
template<size_t N>
class StringBuilderTmp : public StringBuilder {
public:
  ASMJIT_NONCOPYABLE(StringBuilderTmp<N>)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline StringBuilderTmp() noexcept
    : StringBuilder(Globals::Init, ASMJIT_ARRAY_SIZE(_embedded) + ASMJIT_ARRAY_SIZE(_embeddedTmp)) {}

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Embedded data (temporary extension to the `_embedded` data).
  char _embeddedTmp[size_t(N + 1 + sizeof(intptr_t)) & ~size_t(sizeof(intptr_t) - 1)];
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_STRINGBUILDER_H
