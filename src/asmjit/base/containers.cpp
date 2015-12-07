// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/containers.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::PodVectorBase - NullData]
// ============================================================================

const PodVectorData PodVectorBase::_nullData = { 0, 0 };

// ============================================================================
// [asmjit::PodVectorBase - Reset]
// ============================================================================

//! Clear vector data and free internal buffer.
void PodVectorBase::reset(bool releaseMemory) {
  PodVectorData* d = _d;

  if (d == &_nullData)
    return;

  if (releaseMemory) {
    ASMJIT_FREE(d);
    _d = const_cast<PodVectorData*>(&_nullData);
    return;
  }

  d->length = 0;
}

// ============================================================================
// [asmjit::PodVectorBase - Helpers]
// ============================================================================

Error PodVectorBase::_grow(size_t n, size_t sizeOfT) {
  PodVectorData* d = _d;

  size_t threshold = kMemAllocGrowMax / sizeOfT;
  size_t capacity = d->capacity;
  size_t after = d->length;

  if (IntTraits<size_t>::maxValue() - n < after)
    return kErrorNoHeapMemory;

  after += n;

  if (capacity >= after)
    return kErrorOk;

  // PodVector is used as a linear array for some data structures used by
  // AsmJit code generation. The purpose of this agressive growing schema
  // is to minimize memory reallocations, because AsmJit code generation
  // classes live short life and will be freed or reused soon.
  if (capacity < 32)
    capacity = 32;
  else if (capacity < 128)
    capacity = 128;
  else if (capacity < 512)
    capacity = 512;

  while (capacity < after) {
    if (capacity < threshold)
      capacity *= 2;
    else
      capacity += threshold;
  }

  return _reserve(capacity, sizeOfT);
}

Error PodVectorBase::_reserve(size_t n, size_t sizeOfT) {
  PodVectorData* d = _d;

  if (d->capacity >= n)
    return kErrorOk;

  size_t nBytes = sizeof(PodVectorData) + n * sizeOfT;
  if (nBytes < n)
    return kErrorNoHeapMemory;

  if (d == &_nullData) {
    d = static_cast<PodVectorData*>(ASMJIT_ALLOC(nBytes));
    if (d == NULL)
      return kErrorNoHeapMemory;
    d->length = 0;
  }
  else {
    d = static_cast<PodVectorData*>(ASMJIT_REALLOC(d, nBytes));
    if (d == NULL)
      return kErrorNoHeapMemory;
  }

  d->capacity = n;
  _d = d;

  return kErrorOk;
}

// Should be placed in read-only memory.
static const char StringBuilder_empty[4] = { 0 };

// ============================================================================
// [asmjit::StringBuilder - Construction / Destruction]
// ============================================================================

StringBuilder::StringBuilder()
  : _data(const_cast<char*>(StringBuilder_empty)),
    _length(0),
    _capacity(0),
    _canFree(false) {}

StringBuilder::~StringBuilder() {
  if (_canFree)
    ASMJIT_FREE(_data);
}

// ============================================================================
// [asmjit::StringBuilder - Prepare / Reserve]
// ============================================================================

char* StringBuilder::prepare(uint32_t op, size_t len) {
  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  if (op == kStringOpSet) {
    // We don't care here, but we can't return a NULL pointer since it indicates
    // failure in memory allocation.
    if (len == 0) {
      if (_data != StringBuilder_empty)
        _data[0] = 0;

      _length = 0;
      return _data;
    }

    if (_capacity < len) {
      if (len >= IntTraits<size_t>::maxValue() - sizeof(intptr_t) * 2)
        return NULL;

      size_t to = Utils::alignTo<size_t>(len, sizeof(intptr_t));
      if (to < 256 - sizeof(intptr_t))
        to = 256 - sizeof(intptr_t);

      char* newData = static_cast<char*>(ASMJIT_ALLOC(to + sizeof(intptr_t)));
      if (newData == NULL) {
        clear();
        return NULL;
      }

      if (_canFree)
        ASMJIT_FREE(_data);

      _data = newData;
      _capacity = to + sizeof(intptr_t) - 1;
      _canFree = true;
    }

    _data[len] = 0;
    _length = len;

    ASMJIT_ASSERT(_length <= _capacity);
    return _data;
  }

  // --------------------------------------------------------------------------
  // [Append]
  // --------------------------------------------------------------------------

  else {
    // We don't care here, but we can't return a NULL pointer since it indicates
    // failure in memory allocation.
    if (len == 0)
      return _data + _length;

    // Overflow.
    if (IntTraits<size_t>::maxValue() - sizeof(intptr_t) * 2 - _length < len)
      return NULL;

    size_t after = _length + len;
    if (_capacity < after) {
      size_t to = _capacity;

      if (to < 256)
        to = 256;

      while (to < 1024 * 1024 && to < after)
        to *= 2;

      if (to < after) {
        to = after;
        if (to < (IntTraits<size_t>::maxValue() - 1024 * 32))
          to = Utils::alignTo<size_t>(to, 1024 * 32);
      }

      to = Utils::alignTo<size_t>(to, sizeof(intptr_t));
      char* newData = static_cast<char*>(ASMJIT_ALLOC(to + sizeof(intptr_t)));

      if (newData == NULL)
        return NULL;

      ::memcpy(newData, _data, _length);
      if (_canFree)
        ASMJIT_FREE(_data);

      _data = newData;
      _capacity = to + sizeof(intptr_t) - 1;
      _canFree = true;
    }

    char* ret = _data + _length;
    _data[after] = 0;
    _length = after;

    ASMJIT_ASSERT(_length <= _capacity);
    return ret;
  }
}

bool StringBuilder::reserve(size_t to) {
  if (_capacity >= to)
    return true;

  if (to >= IntTraits<size_t>::maxValue() - sizeof(intptr_t) * 2)
    return false;

  to = Utils::alignTo<size_t>(to, sizeof(intptr_t));

  char* newData = static_cast<char*>(ASMJIT_ALLOC(to + sizeof(intptr_t)));
  if (newData == NULL)
    return false;

  ::memcpy(newData, _data, _length + 1);
  if (_canFree)
    ASMJIT_FREE(_data);

  _data = newData;
  _capacity = to + sizeof(intptr_t) - 1;
  _canFree = true;
  return true;
}

// ============================================================================
// [asmjit::StringBuilder - Clear]
// ============================================================================

void StringBuilder::clear() {
  if (_data != StringBuilder_empty)
    _data[0] = 0;
  _length = 0;
}

// ============================================================================
// [asmjit::StringBuilder - Methods]
// ============================================================================

bool StringBuilder::_opString(uint32_t op, const char* str, size_t len) {
  if (len == kInvalidIndex)
    len = str != NULL ? ::strlen(str) : static_cast<size_t>(0);

  char* p = prepare(op, len);
  if (p == NULL)
    return false;

  ::memcpy(p, str, len);
  return true;
}

bool StringBuilder::_opChar(uint32_t op, char c) {
  char* p = prepare(op, 1);
  if (p == NULL)
    return false;

  *p = c;
  return true;
}

bool StringBuilder::_opChars(uint32_t op, char c, size_t len) {
  char* p = prepare(op, len);
  if (p == NULL)
    return false;

  ::memset(p, c, len);
  return true;
}

static const char StringBuilder_numbers[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool StringBuilder::_opNumber(uint32_t op, uint64_t i, uint32_t base, size_t width, uint32_t flags) {
  if (base < 2 || base > 36)
    base = 10;

  char buf[128];
  char* p = buf + ASMJIT_ARRAY_SIZE(buf);

  uint64_t orig = i;
  char sign = '\0';

  // --------------------------------------------------------------------------
  // [Sign]
  // --------------------------------------------------------------------------

  if ((flags & kStringFormatSigned) != 0 && static_cast<int64_t>(i) < 0) {
    i = static_cast<uint64_t>(-static_cast<int64_t>(i));
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

  size_t numberLength = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p);

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

  if (width <= numberLength)
    width = 0;
  else
    width -= numberLength;

  // --------------------------------------------------------------------------
  // Write]
  // --------------------------------------------------------------------------

  size_t prefixLength = (size_t)(buf + ASMJIT_ARRAY_SIZE(buf) - p) - numberLength;
  char* data = prepare(op, prefixLength + width + numberLength);

  if (data == NULL)
    return false;

  ::memcpy(data, p, prefixLength);
  data += prefixLength;

  ::memset(data, '0', width);
  data += width;

  ::memcpy(data, p + prefixLength, numberLength);
  return true;
}

bool StringBuilder::_opHex(uint32_t op, const void* data, size_t len) {
  if (len >= IntTraits<size_t>::maxValue() / 2)
    return false;

  char* dst = prepare(op, len * 2);
  if (dst == NULL)
    return false;

  const char* src = static_cast<const char*>(data);
  for (size_t i = 0; i < len; i++, dst += 2, src += 1)
  {
    dst[0] = StringBuilder_numbers[(src[0] >> 4) & 0xF];
    dst[1] = StringBuilder_numbers[(src[0]     ) & 0xF];
  }

  return true;
}

bool StringBuilder::_opVFormat(uint32_t op, const char* fmt, va_list ap) {
  char buf[1024];

  vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
  buf[ASMJIT_ARRAY_SIZE(buf) - 1] = '\0';

  return _opString(op, buf);
}

bool StringBuilder::setFormat(const char* fmt, ...) {
  bool result;

  va_list ap;
  va_start(ap, fmt);
  result = _opVFormat(kStringOpSet, fmt, ap);
  va_end(ap);

  return result;
}

bool StringBuilder::appendFormat(const char* fmt, ...) {
  bool result;

  va_list ap;
  va_start(ap, fmt);
  result = _opVFormat(kStringOpAppend, fmt, ap);
  va_end(ap);

  return result;
}

bool StringBuilder::eq(const char* str, size_t len) const {
  const char* aData = _data;
  const char* bData = str;

  size_t aLength = _length;
  size_t bLength = len;

  if (bLength == kInvalidIndex) {
    size_t i;
    for (i = 0; i < aLength; i++) {
      if (aData[i] != bData[i] || bData[i] == 0)
        return false;
    }

    return bData[i] == 0;
  }
  else {
    if (aLength != bLength)
      return false;

    return ::memcmp(aData, bData, aLength) == 0;
  }
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
