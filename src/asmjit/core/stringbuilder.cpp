// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/memmgr.h"
#include "../core/stringbuilder.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::StringBuilder - Globals]
// ============================================================================

constexpr char StringBuilder_numbers[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

constexpr size_t kMinAllocSize = 256;
constexpr size_t kMaxAllocSize = std::numeric_limits<size_t>::max() - Globals::kAllocThreshold;

// ============================================================================
// [asmjit::StringBuilder - Prepare / Reserve]
// ============================================================================

ASMJIT_FAVOR_SIZE char* StringBuilder::prepare(uint32_t op, size_t size) noexcept {
  if (op == kStringOpSet) {
    if (size > _capacity) {
      // Prevent arithmetic overflow.
      if (ASMJIT_UNLIKELY(size >= kMaxAllocSize))
        return nullptr;

      size_t newCapacity = Support::alignUp<size_t>(size + 1, kMinAllocSize);
      char* newData = static_cast<char*>(MemMgr::alloc(newCapacity));

      if (ASMJIT_UNLIKELY(!newData))
        return nullptr;

      char* oldData = _data;
      _data = newData;
      _capacity = newCapacity - 1;

      if (oldData != _embedded)
        MemMgr::release(oldData);
    }

    _data[size] = 0;
    _size = size;

    ASMJIT_ASSERT(_size <= _capacity);
    return _data;
  }
  else {
    // Prevent arithmetic overflow.
    if (ASMJIT_UNLIKELY(size >= kMaxAllocSize - _size))
      return nullptr;

    size_t afterPlusOne = _size + size + 1;
    if (afterPlusOne > _capacity) {
      size_t newCapacity = std::max<size_t>(_capacity + 1, kMinAllocSize);

      if (newCapacity < afterPlusOne && newCapacity < Globals::kAllocThreshold)
        newCapacity = Support::alignUpPowerOf2(newCapacity);

      if (newCapacity < afterPlusOne)
        newCapacity = Support::alignUp(afterPlusOne, Globals::kAllocThreshold);

      if (ASMJIT_UNLIKELY(newCapacity < afterPlusOne))
        return nullptr;

      char* newData = static_cast<char*>(MemMgr::alloc(newCapacity));
      if (!newData) return nullptr;

      char* oldData = _data;
      std::memcpy(newData, oldData, _size);

      _data = newData;
      _capacity = newCapacity - 1;

      if (oldData != _embedded)
        MemMgr::release(oldData);
    }

    char* p = _data + _size;
    _size = afterPlusOne - 1;
    _data[afterPlusOne - 1] = '\0';
    ASMJIT_ASSERT(_size <= _capacity);

    return p;
  }
}

ASMJIT_FAVOR_SIZE Error StringBuilder::reserve(size_t to) noexcept {
  if (_capacity >= to)
    return kErrorOk;

  if (to >= std::numeric_limits<size_t>::max() - sizeof(intptr_t) * 2)
    return DebugUtils::errored(kErrorNoHeapMemory);

  to = Support::alignUp<size_t>(to, sizeof(intptr_t));
  char* newData = static_cast<char*>(MemMgr::alloc(to + sizeof(intptr_t)));

  if (!newData)
    return DebugUtils::errored(kErrorNoHeapMemory);

  std::memcpy(newData, _data, _size + 1);

  char* oldData = _data;
  _data = newData;
  _capacity = to + sizeof(intptr_t) - 1;

  if (oldData != _embedded)
    MemMgr::release(oldData);
  return kErrorOk;
}

// ============================================================================
// [asmjit::StringBuilder - Methods]
// ============================================================================

Error StringBuilder::_opString(uint32_t op, const char* str, size_t size) noexcept {
  if (size == Globals::kNullTerminated)
    size = str ? std::strlen(str) : size_t(0);

  if (!size)
    return kErrorOk;

  char* p = prepare(op, size);
  if (!p) return DebugUtils::errored(kErrorNoHeapMemory);

  std::memcpy(p, str, size);
  return kErrorOk;
}

Error StringBuilder::_opChar(uint32_t op, char c) noexcept {
  char* p = prepare(op, 1);
  if (!p) return DebugUtils::errored(kErrorNoHeapMemory);

  *p = c;
  return kErrorOk;
}

Error StringBuilder::_opChars(uint32_t op, char c, size_t n) noexcept {
  if (!n)
    return kErrorOk;

  char* p = prepare(op, n);
  if (!p) return DebugUtils::errored(kErrorNoHeapMemory);

  std::memset(p, c, n);
  return kErrorOk;
}

Error StringBuilder::padEnd(size_t n, char c) noexcept {
  size_t size = _size;
  if (n <= size)
    return kErrorOk;

  return appendChars(c, n - size);
}

Error StringBuilder::_opNumber(uint32_t op, uint64_t i, uint32_t base, size_t width, uint32_t flags) noexcept {
  if (base < 2 || base > 36)
    base = 10;

  char buf[128];
  char* p = buf + ASMJIT_ARRAY_SIZE(buf);

  uint64_t orig = i;
  char sign = '\0';

  // --------------------------------------------------------------------------
  // [Sign]
  // --------------------------------------------------------------------------

  if ((flags & kStringFormatSigned) != 0 && int64_t(i) < 0) {
    i = uint64_t(-int64_t(i));
    sign = '-';
  }
  else if ((flags & kStringFormatShowSign) != 0) {
    sign = '+';
  }
  else if ((flags & kStringFormatShowSpace) != 0) {
    sign = ' ';
  }

  // --------------------------------------------------------------------------
  // [Number]
  // --------------------------------------------------------------------------

  do {
    uint64_t d = i / base;
    uint64_t r = i % base;

    *--p = StringBuilder_numbers[r];
    i = d;
  } while (i);

  size_t numberSize = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p);

  // --------------------------------------------------------------------------
  // [Alternate Form]
  // --------------------------------------------------------------------------

  if ((flags & kStringFormatAlternate) != 0) {
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
    return DebugUtils::errored(kErrorNoHeapMemory);

  std::memcpy(data, p, prefixSize);
  data += prefixSize;

  std::memset(data, '0', width);
  data += width;

  std::memcpy(data, p + prefixSize, numberSize);
  return kErrorOk;
}

Error StringBuilder::_opHex(uint32_t op, const void* data, size_t size, char separator) noexcept {
  char* dst;
  const char* src = static_cast<const char*>(data);

  if (!size)
    return kErrorOk;

  if (separator) {
    if (ASMJIT_UNLIKELY(size >= std::numeric_limits<size_t>::max() / 3))
      return DebugUtils::errored(kErrorNoHeapMemory);

    dst = prepare(op, size * 3 - 1);
    if (ASMJIT_UNLIKELY(!dst))
      return DebugUtils::errored(kErrorNoHeapMemory);

    size_t i = 0;
    for (;;) {
      dst[0] = StringBuilder_numbers[(uint8_t(src[0]) >> 4) & 0xF];
      dst[1] = StringBuilder_numbers[(uint8_t(src[0])     ) & 0xF];
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
      return DebugUtils::errored(kErrorNoHeapMemory);

    dst = prepare(op, size * 2);
    if (ASMJIT_UNLIKELY(!dst))
      return DebugUtils::errored(kErrorNoHeapMemory);

    for (size_t i = 0; i < size; i++, dst += 2, src++) {
      dst[0] = StringBuilder_numbers[(uint8_t(src[0]) >> 4) & 0xF];
      dst[1] = StringBuilder_numbers[(uint8_t(src[0])     ) & 0xF];
    }
  }

  return kErrorOk;
}

Error StringBuilder::_opVFormat(uint32_t op, const char* fmt, std::va_list ap) noexcept {
  size_t startAt = (op == kStringOpSet) ? size_t(0) : _size;
  size_t remainingCapacity = _capacity - startAt;

  char buf[1024];
  int fmtResult;
  size_t outputSize;

  if (remainingCapacity >= 128) {
    fmtResult = std::vsnprintf(_data + startAt, remainingCapacity, fmt, ap);
    outputSize = size_t(fmtResult);

    if (ASMJIT_LIKELY(outputSize <= remainingCapacity)) {
      _size = startAt + outputSize;
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
    return DebugUtils::errored(kErrorNoHeapMemory);

  fmtResult = std::vsnprintf(p, outputSize + 1, fmt, ap);
  ASMJIT_ASSERT(size_t(fmtResult) == outputSize);

  return kErrorOk;
}

bool StringBuilder::eq(const char* data, size_t size) const noexcept {
  const char* aData = _data;
  const char* bData = data;

  size_t aSize = _size;
  size_t bSize = size;

  if (bSize == Globals::kNullTerminated) {
    size_t i;
    for (i = 0; i < aSize; i++)
      if (aData[i] != bData[i] || bData[i] == 0)
        return false;
    return bData[i] == 0;
  }
  else {
    if (aSize != bSize)
      return false;
    return std::memcmp(aData, bData, aSize) == 0;
  }
}

ASMJIT_END_NAMESPACE
