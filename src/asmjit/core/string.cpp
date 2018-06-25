// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/string.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::String - Globals]
// ============================================================================

static const char String_baseN[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

constexpr size_t kMinAllocSize = 64;
constexpr size_t kMaxAllocSize = std::numeric_limits<size_t>::max() - Globals::kAllocThreshold;

// ============================================================================
// [asmjit::String]
// ============================================================================

Error String::reset() noexcept {
  if (type == kTypeLarge)
    ::free(large.data);

  _resetToEmbedded();
  return kErrorOk;
}

Error String::clear() noexcept {
  if (isLarge()) {
    large.size = 0;
    large.data[0] = '\0';
  }
  else {
    uptr[0] = 0;
  }

  return kErrorOk;
}

Error String::truncate(size_t newSize) noexcept {
  if (isLarge()) {
    if (newSize < large.size) {
      large.data[newSize] = '\0';
      large.size = newSize;
    }
  }
  else {
    if (newSize < type) {
      small.data[newSize] = '\0';
      small.type = uint8_t(newSize);
    }
  }

  return kErrorOk;
}

char* String::prepare(uint32_t op, size_t size) noexcept {
  char* curData;
  size_t curSize;
  size_t curCapacity;

  if (isLarge()) {
    curData = this->large.data;
    curSize = this->large.size;
    curCapacity = this->large.capacity;
  }
  else {
    curData = this->small.data;
    curSize = this->small.type;
    curCapacity = kSSOCapacity;
  }

  if (op == kOpAssign) {
    if (size > curCapacity) {
      // Prevent arithmetic overflow.
      if (ASMJIT_UNLIKELY(size >= kMaxAllocSize))
        return nullptr;

      size_t newCapacity = Support::alignUp<size_t>(size + 1, kMinAllocSize);
      char* newData = static_cast<char*>(::malloc(newCapacity));

      if (ASMJIT_UNLIKELY(!newData))
        return nullptr;

      if (type == kTypeLarge)
        ::free(curData);

      large.type = kTypeLarge;
      large.size = size;
      large.capacity = newCapacity - 1;
      large.data = newData;

      newData[size] = '\0';
      return newData;
    }
    else {
      _setSize(size);
      curData[size] = '\0';
      return curData;
    }
  }
  else {
    // Prevent arithmetic overflow.
    if (ASMJIT_UNLIKELY(size >= kMaxAllocSize - curSize))
      return nullptr;

    size_t newSize = size + curSize;
    size_t newSizePlusOne = newSize + 1;

    if (newSizePlusOne > curCapacity) {
      size_t newCapacity = Support::max<size_t>(curCapacity + 1, kMinAllocSize);

      if (newCapacity < newSizePlusOne && newCapacity < Globals::kAllocThreshold)
        newCapacity = Support::alignUpPowerOf2(newCapacity);

      if (newCapacity < newSizePlusOne)
        newCapacity = Support::alignUp(newSizePlusOne, Globals::kAllocThreshold);

      if (ASMJIT_UNLIKELY(newCapacity < newSizePlusOne))
        return nullptr;

      char* newData = static_cast<char*>(::malloc(newCapacity));
      if (ASMJIT_UNLIKELY(!newData))
        return nullptr;

      ::memcpy(newData, curData, curSize);

      if (type == kTypeLarge)
        ::free(curData);

      large.type = kTypeLarge;
      large.size = newSize;
      large.capacity = newCapacity - 1;
      large.data = newData;

      newData[newSize] = '\0';
      return newData + curSize;
    }
    else {
      _setSize(newSize);
      curData[newSize] = '\0';
      return curData + curSize;
    }
  }
}

bool String::eq(const char* other, size_t size) const noexcept {
  const char* aData = data();
  const char* bData = other;

  size_t aSize = this->size();
  size_t bSize = size;

  if (bSize == SIZE_MAX) {
    size_t i;
    for (i = 0; i < aSize; i++)
      if (aData[i] != bData[i] || bData[i] == 0)
        return false;
    return bData[i] == 0;
  }
  else {
    if (aSize != bSize)
      return false;
    return ::memcmp(aData, bData, aSize) == 0;
  }
}

Error String::assignString(const char* data, size_t size) noexcept {
  char* dst = nullptr;

  // Null terminated string without `size` specified.
  if (size == SIZE_MAX)
    size = data ? ::strlen(data) : size_t(0);

  if (isLarge()) {
    if (size <= large.capacity) {
      dst = large.data;
      large.size = size;
    }
    else {
      size_t capacityPlusOne = Support::alignUp(size + 1, 32);
      if (ASMJIT_UNLIKELY(capacityPlusOne < size))
        return DebugUtils::errored(kErrorOutOfMemory);

      dst = static_cast<char*>(::malloc(capacityPlusOne));
      if (ASMJIT_UNLIKELY(!dst))
        return DebugUtils::errored(kErrorOutOfMemory);

      if (!isExternal())
        ::free(large.data);

      large.type = kTypeLarge;
      large.data = dst;
      large.size = size;
      large.capacity = capacityPlusOne - 1;
    }
  }
  else {
    if (size <= kSSOCapacity) {
      ASMJIT_ASSERT(size < 0xFFu);

      dst = small.data;
      small.type = uint8_t(size);
    }
    else {
      dst = static_cast<char*>(::malloc(size + 1));
      if (ASMJIT_UNLIKELY(!dst))
        return DebugUtils::errored(kErrorOutOfMemory);

      large.type = kTypeLarge;
      large.data = dst;
      large.size = size;
      large.capacity = size;
    }
  }

  // Optionally copy data from `data` and null-terminate.
  if (data && size) {
    // NOTE: It's better to use `memmove()`. If, for any reason, somebody uses
    // this function to substring the same string it would work as expected.
    ::memmove(dst, data, size);
  }

  dst[size] = '\0';
  return kErrorOk;
}

// ============================================================================
// [asmjit::String - Operations]
// ============================================================================

Error String::_opString(uint32_t op, const char* str, size_t size) noexcept {
  if (size == SIZE_MAX)
    size = str ? ::strlen(str) : size_t(0);

  if (!size)
    return kErrorOk;

  char* p = prepare(op, size);
  if (!p) return DebugUtils::errored(kErrorOutOfMemory);

  ::memcpy(p, str, size);
  return kErrorOk;
}

Error String::_opChar(uint32_t op, char c) noexcept {
  char* p = prepare(op, 1);
  if (!p) return DebugUtils::errored(kErrorOutOfMemory);

  *p = c;
  return kErrorOk;
}

Error String::_opChars(uint32_t op, char c, size_t n) noexcept {
  if (!n)
    return kErrorOk;

  char* p = prepare(op, n);
  if (!p) return DebugUtils::errored(kErrorOutOfMemory);

  ::memset(p, c, n);
  return kErrorOk;
}

Error String::padEnd(size_t n, char c) noexcept {
  size_t size = this->size();
  if (n <= size)
    return kErrorOk;

  return appendChars(c, n - size);
}

Error String::_opNumber(uint32_t op, uint64_t i, uint32_t base, size_t width, uint32_t flags) noexcept {
  if (base < 2 || base > 36)
    base = 10;

  char buf[128];
  char* p = buf + ASMJIT_ARRAY_SIZE(buf);

  uint64_t orig = i;
  char sign = '\0';

  // --------------------------------------------------------------------------
  // [Sign]
  // --------------------------------------------------------------------------

  if ((flags & kFormatSigned) != 0 && int64_t(i) < 0) {
    i = uint64_t(-int64_t(i));
    sign = '-';
  }
  else if ((flags & kFormatShowSign) != 0) {
    sign = '+';
  }
  else if ((flags & kFormatShowSpace) != 0) {
    sign = ' ';
  }

  // --------------------------------------------------------------------------
  // [Number]
  // --------------------------------------------------------------------------

  do {
    uint64_t d = i / base;
    uint64_t r = i % base;

    *--p = String_baseN[r];
    i = d;
  } while (i);

  size_t numberSize = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p);

  // --------------------------------------------------------------------------
  // [Alternate Form]
  // --------------------------------------------------------------------------

  if ((flags & kFormatAlternate) != 0) {
    if (base == 8) {
      if (orig != 0)
        *--p = '0';
    }
    if (base == 16) {
      *--p = 'x';
      *--p = '0';
    }
  }

  // --------------------------------------------------------------------------
  // [Width]
  // --------------------------------------------------------------------------

  if (sign != 0)
    *--p = sign;

  if (width > 256)
    width = 256;

  if (width <= numberSize)
    width = 0;
  else
    width -= numberSize;

  // --------------------------------------------------------------------------
  // Write]
  // --------------------------------------------------------------------------

  size_t prefixSize = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p) - numberSize;
  char* data = prepare(op, prefixSize + width + numberSize);

  if (!data)
    return DebugUtils::errored(kErrorOutOfMemory);

  ::memcpy(data, p, prefixSize);
  data += prefixSize;

  ::memset(data, '0', width);
  data += width;

  ::memcpy(data, p + prefixSize, numberSize);
  return kErrorOk;
}

Error String::_opHex(uint32_t op, const void* data, size_t size, char separator) noexcept {
  char* dst;
  const uint8_t* src = static_cast<const uint8_t*>(data);

  if (!size)
    return kErrorOk;

  if (separator) {
    if (ASMJIT_UNLIKELY(size >= std::numeric_limits<size_t>::max() / 3))
      return DebugUtils::errored(kErrorOutOfMemory);

    dst = prepare(op, size * 3 - 1);
    if (ASMJIT_UNLIKELY(!dst))
      return DebugUtils::errored(kErrorOutOfMemory);

    size_t i = 0;
    for (;;) {
      dst[0] = String_baseN[(src[0] >> 4) & 0xF];
      dst[1] = String_baseN[(src[0]     ) & 0xF];
      if (++i == size)
        break;
      // This makes sure that the separator is only put between two hexadecimal bytes.
      dst[2] = separator;
      dst += 3;
      src++;
    }
  }
  else {
    if (ASMJIT_UNLIKELY(size >= std::numeric_limits<size_t>::max() / 2))
      return DebugUtils::errored(kErrorOutOfMemory);

    dst = prepare(op, size * 2);
    if (ASMJIT_UNLIKELY(!dst))
      return DebugUtils::errored(kErrorOutOfMemory);

    for (size_t i = 0; i < size; i++, dst += 2, src++) {
      dst[0] = String_baseN[(src[0] >> 4) & 0xF];
      dst[1] = String_baseN[(src[0]     ) & 0xF];
    }
  }

  return kErrorOk;
}

Error String::_opFormat(uint32_t op, const char* fmt, ...) noexcept {
  Error err;
  va_list ap;

  va_start(ap, fmt);
  err = _opVFormat(op, fmt, ap);
  va_end(ap);

  return err;
}

Error String::_opVFormat(uint32_t op, const char* fmt, va_list ap) noexcept {
  size_t startAt = (op == kOpAssign) ? size_t(0) : size();
  size_t remainingCapacity = capacity() - startAt;

  char buf[1024];
  int fmtResult;
  size_t outputSize;

  if (remainingCapacity >= 128) {
    fmtResult = std::vsnprintf(data() + startAt, remainingCapacity, fmt, ap);
    outputSize = size_t(fmtResult);

    if (ASMJIT_LIKELY(outputSize <= remainingCapacity)) {
      _setSize(startAt + outputSize);
      return kErrorOk;
    }
  }
  else {
    fmtResult = std::vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
    outputSize = size_t(fmtResult);

    if (ASMJIT_LIKELY(outputSize < ASMJIT_ARRAY_SIZE(buf)))
      return _opString(op, buf, outputSize);
  }

  if (ASMJIT_UNLIKELY(fmtResult < 0))
    return DebugUtils::errored(kErrorInvalidState);

  char* p = prepare(op, outputSize);
  if (ASMJIT_UNLIKELY(!p))
    return DebugUtils::errored(kErrorOutOfMemory);

  fmtResult = std::vsnprintf(p, outputSize + 1, fmt, ap);
  ASMJIT_ASSERT(size_t(fmtResult) == outputSize);

  return kErrorOk;
}

// ============================================================================
// [asmjit::Support - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(asmjit_core_string) {
  String s;

  EXPECT(s.isLarge() == false);
  EXPECT(s.isExternal() == false);

  EXPECT(s.assignChar('a') == kErrorOk);
  EXPECT(s.size() == 1);
  EXPECT(s.capacity() == String::kSSOCapacity);
  EXPECT(s.data()[0] == 'a');
  EXPECT(s.data()[1] == '\0');
  EXPECT(s.eq("a") == true);
  EXPECT(s.eq("a", 1) == true);

  EXPECT(s.assignChars('b', 4) == kErrorOk);
  EXPECT(s.size() == 4);
  EXPECT(s.capacity() == String::kSSOCapacity);
  EXPECT(s.data()[0] == 'b');
  EXPECT(s.data()[1] == 'b');
  EXPECT(s.data()[2] == 'b');
  EXPECT(s.data()[3] == 'b');
  EXPECT(s.data()[4] == '\0');
  EXPECT(s.eq("bbbb") == true);
  EXPECT(s.eq("bbbb", 4) == true);

  EXPECT(s.assignString("abc") == kErrorOk);
  EXPECT(s.size() == 3);
  EXPECT(s.capacity() == String::kSSOCapacity);
  EXPECT(s.data()[0] == 'a');
  EXPECT(s.data()[1] == 'b');
  EXPECT(s.data()[2] == 'c');
  EXPECT(s.data()[3] == '\0');
  EXPECT(s.eq("abc") == true);
  EXPECT(s.eq("abc", 3) == true);

  const char* large = "Large string that will not fit into SSO buffer";
  EXPECT(s.assignString(large) == kErrorOk);
  EXPECT(s.isLarge() == true);
  EXPECT(s.size() == ::strlen(large));
  EXPECT(s.capacity() > String::kSSOCapacity);
  EXPECT(s.eq(large) == true);
  EXPECT(s.eq(large, ::strlen(large)) == true);

  const char* additional = " (additional content)";
  EXPECT(s.isLarge() == true);
  EXPECT(s.appendString(additional) == kErrorOk);
  EXPECT(s.size() == ::strlen(large) + ::strlen(additional));

  EXPECT(s.clear() == kErrorOk);
  EXPECT(s.size() == 0);
  EXPECT(s.empty() == true);
  EXPECT(s.data()[0] == '\0');
  EXPECT(s.isLarge() == true); // Clear should never release the memory.

  EXPECT(s.appendUInt(1234) == kErrorOk);
  EXPECT(s.eq("1234") == true);

  StringTmp<64> sTmp;
  EXPECT(sTmp.isLarge());
  EXPECT(sTmp.isExternal());
  EXPECT(sTmp.appendChars(' ', 1000) == kErrorOk);
  EXPECT(!sTmp.isExternal());
}
#endif

ASMJIT_END_NAMESPACE
