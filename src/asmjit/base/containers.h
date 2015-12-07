// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CONTAINERS_H
#define _ASMJIT_BASE_CONTAINERS_H

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::BitArray]
// ============================================================================

//! Fixed size bit-array.
//!
//! Used by variable liveness analysis.
struct BitArray {
  // --------------------------------------------------------------------------
  // [Enums]
  // --------------------------------------------------------------------------

  enum {
    kEntitySize = static_cast<int>(sizeof(uintptr_t)),
    kEntityBits = kEntitySize * 8
  };

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uintptr_t getBit(uint32_t index) const {
    return (data[index / kEntityBits] >> (index % kEntityBits)) & 1;
  }

  ASMJIT_INLINE void setBit(uint32_t index) {
    data[index / kEntityBits] |= static_cast<uintptr_t>(1) << (index % kEntityBits);
  }

  ASMJIT_INLINE void delBit(uint32_t index) {
    data[index / kEntityBits] &= ~(static_cast<uintptr_t>(1) << (index % kEntityBits));
  }

  // --------------------------------------------------------------------------
  // [Interface]
  // --------------------------------------------------------------------------

  //! Copy bits from `s0`, returns `true` if at least one bit is set in `s0`.
  ASMJIT_INLINE bool copyBits(const BitArray* s0, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool addBits(const BitArray* s0, uint32_t len) {
    return addBits(this, s0, len);
  }

  ASMJIT_INLINE bool addBits(const BitArray* s0, const BitArray* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] | s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool andBits(const BitArray* s1, uint32_t len) {
    return andBits(this, s1, len);
  }

  ASMJIT_INLINE bool andBits(const BitArray* s0, const BitArray* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] & s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool delBits(const BitArray* s1, uint32_t len) {
    return delBits(this, s1, len);
  }

  ASMJIT_INLINE bool delBits(const BitArray* s0, const BitArray* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t t = s0->data[i] & ~s1->data[i];
      data[i] = t;
      r |= t;
    }
    return r != 0;
  }

  ASMJIT_INLINE bool _addBitsDelSource(BitArray* s1, uint32_t len) {
    return _addBitsDelSource(this, s1, len);
  }

  ASMJIT_INLINE bool _addBitsDelSource(const BitArray* s0, BitArray* s1, uint32_t len) {
    uintptr_t r = 0;
    for (uint32_t i = 0; i < len; i++) {
      uintptr_t a = s0->data[i];
      uintptr_t b = s1->data[i];

      this->data[i] = a | b;
      b &= ~a;

      s1->data[i] = b;
      r |= b;
    }
    return r != 0;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uintptr_t data[1];
};

// ============================================================================
// [asmjit::PodVectorData]
// ============================================================================

//! \internal
struct PodVectorData {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get data.
  ASMJIT_INLINE void* getData() const { return (void*)(this + 1); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Capacity of the vector.
  size_t capacity;
  //! Length of the vector.
  size_t length;
};

// ============================================================================
// [asmjit::PodVectorBase]
// ============================================================================

//! \internal
struct PodVectorBase {
  static ASMJIT_API const PodVectorData _nullData;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `PodVectorBase`.
  ASMJIT_INLINE PodVectorBase() : _d(const_cast<PodVectorData*>(&_nullData)) {}
  //! Destroy the `PodVectorBase` and data.
  ASMJIT_INLINE ~PodVectorBase() { reset(true); }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  //! Reset the vector data and set its `length` to zero.
  //!
  //! If `releaseMemory` is true the vector buffer will be released to the
  //! system.
  ASMJIT_API void reset(bool releaseMemory = false);

  // --------------------------------------------------------------------------
  // [Grow / Reserve]
  // --------------------------------------------------------------------------

protected:
  ASMJIT_API Error _grow(size_t n, size_t sizeOfT);
  ASMJIT_API Error _reserve(size_t n, size_t sizeOfT);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

public:
  PodVectorData* _d;
};

// ============================================================================
// [asmjit::PodVector<T>]
// ============================================================================

//! Template used to store and manage array of POD data.
//!
//! This template has these adventages over other vector<> templates:
//! - Non-copyable (designed to be non-copyable, we want it)
//! - No copy-on-write (some implementations of stl can use it)
//! - Optimized for working only with POD types
//! - Uses ASMJIT_... memory management macros
template <typename T>
struct PodVector : PodVectorBase {
  ASMJIT_NO_COPY(PodVector<T>)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `PodVector<T>`.
  ASMJIT_INLINE PodVector() {}
  //! Destroy the `PodVector<>` and data.
  ASMJIT_INLINE ~PodVector() {}

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  //! Get whether the vector is empty.
  ASMJIT_INLINE bool isEmpty() const { return _d->length == 0; }
  //! Get length.
  ASMJIT_INLINE size_t getLength() const { return _d->length; }
  //! Get capacity.
  ASMJIT_INLINE size_t getCapacity() const { return _d->capacity; }
  //! Get data.
  ASMJIT_INLINE T* getData() { return static_cast<T*>(_d->getData()); }
  //! \overload
  ASMJIT_INLINE const T* getData() const { return static_cast<const T*>(_d->getData()); }

  // --------------------------------------------------------------------------
  // [Grow / Reserve]
  // --------------------------------------------------------------------------

  //! Called to grow the buffer to fit at least `n` elements more.
  ASMJIT_INLINE Error _grow(size_t n) { return PodVectorBase::_grow(n, sizeof(T)); }
  //! Realloc internal array to fit at least `n` items.
  ASMJIT_INLINE Error _reserve(size_t n) { return PodVectorBase::_reserve(n, sizeof(T)); }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Prepend `item` to vector.
  Error prepend(const T& item) {
    PodVectorData* d = _d;

    if (d->length == d->capacity) {
      ASMJIT_PROPAGATE_ERROR(_grow(1));
      _d = d;
    }

    ::memmove(static_cast<T*>(d->getData()) + 1, d->getData(), d->length * sizeof(T));
    ::memcpy(d->getData(), &item, sizeof(T));

    d->length++;
    return kErrorOk;
  }

  //! Insert an `item` at the `index`.
  Error insert(size_t index, const T& item) {
    PodVectorData* d = _d;
    ASMJIT_ASSERT(index <= d->length);

    if (d->length == d->capacity) {
      ASMJIT_PROPAGATE_ERROR(_grow(1));
      d = _d;
    }

    T* dst = static_cast<T*>(d->getData()) + index;
    ::memmove(dst + 1, dst, d->length - index);
    ::memcpy(dst, &item, sizeof(T));

    d->length++;
    return kErrorOk;
  }

  //! Append `item` to vector.
  Error append(const T& item) {
    PodVectorData* d = _d;

    if (d->length == d->capacity) {
      ASMJIT_PROPAGATE_ERROR(_grow(1));
      d = _d;
    }

    ::memcpy(static_cast<T*>(d->getData()) + d->length, &item, sizeof(T));

    d->length++;
    return kErrorOk;
  }

  //! Get index of `val` or `kInvalidIndex` if not found.
  size_t indexOf(const T& val) const {
    PodVectorData* d = _d;

    const T* data = static_cast<const T*>(d->getData());
    size_t len = d->length;

    for (size_t i = 0; i < len; i++)
      if (data[i] == val)
        return i;

    return kInvalidIndex;
  }

  //! Remove item at index `i`.
  void removeAt(size_t i) {
    PodVectorData* d = _d;
    ASMJIT_ASSERT(i < d->length);

    T* data = static_cast<T*>(d->getData()) + i;
    d->length--;
    ::memmove(data, data + 1, d->length - i);
  }

  //! Swap this pod-vector with `other`.
  void swap(PodVector<T>& other) {
    T* otherData = other._d;
    other._d = _d;
    _d = otherData;
  }

  //! Get item at index `i`.
  ASMJIT_INLINE T& operator[](size_t i) {
    ASMJIT_ASSERT(i < getLength());
    return getData()[i];
  }

  //! Get item at index `i`.
  ASMJIT_INLINE const T& operator[](size_t i) const {
    ASMJIT_ASSERT(i < getLength());
    return getData()[i];
  }
};

// ============================================================================
// [asmjit::PodList<T>]
// ============================================================================

//! \internal
template <typename T>
struct PodList {
  ASMJIT_NO_COPY(PodList<T>)

  // --------------------------------------------------------------------------
  // [Link]
  // --------------------------------------------------------------------------

  struct Link {
    // --------------------------------------------------------------------------
    // [Accessors]
    // --------------------------------------------------------------------------

    //! Get next node.
    ASMJIT_INLINE Link* getNext() const { return _next; }

    //! Get value.
    ASMJIT_INLINE T getValue() const { return _value; }
    //! Set value to `value`.
    ASMJIT_INLINE void setValue(const T& value) { _value = value; }

    // --------------------------------------------------------------------------
    // [Members]
    // --------------------------------------------------------------------------

    Link* _next;
    T _value;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE PodList() : _first(NULL), _last(NULL) {}
  ASMJIT_INLINE ~PodList() {}

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isEmpty() const { return _first != NULL; }

  ASMJIT_INLINE Link* getFirst() const { return _first; }
  ASMJIT_INLINE Link* getLast() const { return _last; }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() {
    _first = NULL;
    _last = NULL;
  }

  ASMJIT_INLINE void prepend(Link* link) {
    link->_next = _first;
    if (_first == NULL)
      _last = link;
    _first = link;
  }

  ASMJIT_INLINE void append(Link* link) {
    link->_next = NULL;
    if (_first == NULL)
      _first = link;
    else
      _last->_next = link;
    _last = link;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Link* _first;
  Link* _last;
};

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
struct StringBuilder {
  ASMJIT_NO_COPY(StringBuilder)

  // --------------------------------------------------------------------------
  // [Enums]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! String operation.
  ASMJIT_ENUM(StringOp) {
    //! Replace the current string by a given content.
    kStringOpSet = 0,
    //! Append a given content to the current string.
    kStringOpAppend = 1
  };

  //! \internal
  //!
  //! String format flags.
  ASMJIT_ENUM(StringFormatFlags) {
    kStringFormatShowSign  = 0x00000001,
    kStringFormatShowSpace = 0x00000002,
    kStringFormatAlternate = 0x00000004,
    kStringFormatSigned    = 0x80000000
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API StringBuilder();
  ASMJIT_API ~StringBuilder();

  ASMJIT_INLINE StringBuilder(const _NoInit&) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get string builder capacity.
  ASMJIT_INLINE size_t getCapacity() const { return _capacity; }
  //! Get length.
  ASMJIT_INLINE size_t getLength() const { return _length; }

  //! Get null-terminated string data.
  ASMJIT_INLINE char* getData() { return _data; }
  //! Get null-terminated string data (const).
  ASMJIT_INLINE const char* getData() const { return _data; }

  // --------------------------------------------------------------------------
  // [Prepare / Reserve]
  // --------------------------------------------------------------------------

  //! Prepare to set/append.
  ASMJIT_API char* prepare(uint32_t op, size_t len);

  //! Reserve `to` bytes in string builder.
  ASMJIT_API bool reserve(size_t to);

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  //! Clear the content in String builder.
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

  //! Replace the current content by `str` of `len`.
  ASMJIT_INLINE bool setString(const char* str, size_t len = kInvalidIndex) {
    return _opString(kStringOpSet, str, len);
  }

  //! Replace the current content by formatted string `fmt`.
  ASMJIT_INLINE bool setVFormat(const char* fmt, va_list ap) {
    return _opVFormat(kStringOpSet, fmt, ap);
  }

  //! Replace the current content by formatted string `fmt`.
  ASMJIT_API bool setFormat(const char* fmt, ...);

  //! Replace the current content by `c` character.
  ASMJIT_INLINE bool setChar(char c) {
    return _opChar(kStringOpSet, c);
  }

  //! Replace the current content by `c` of `len`.
  ASMJIT_INLINE bool setChars(char c, size_t len) {
    return _opChars(kStringOpSet, c, len);
  }

  //! Replace the current content by formatted integer `i`.
  ASMJIT_INLINE bool setInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) {
    return _opNumber(kStringOpSet, i, base, width, flags | kStringFormatSigned);
  }

  //! Replace the current content by formatted integer `i`.
  ASMJIT_INLINE bool setUInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) {
    return _opNumber(kStringOpSet, i, base, width, flags);
  }

  //! Replace the current content by the given `data` converted to a HEX string.
  ASMJIT_INLINE bool setHex(const void* data, size_t len) {
    return _opHex(kStringOpSet, data, len);
  }

  // --------------------------------------------------------------------------
  // [Append]
  // --------------------------------------------------------------------------

  //! Append `str` of `len`.
  ASMJIT_INLINE bool appendString(const char* str, size_t len = kInvalidIndex) {
    return _opString(kStringOpAppend, str, len);
  }

  //! Append a formatted string `fmt` to the current content.
  ASMJIT_INLINE bool appendVFormat(const char* fmt, va_list ap) {
    return _opVFormat(kStringOpAppend, fmt, ap);
  }

  //! Append a formatted string `fmt` to the current content.
  ASMJIT_API bool appendFormat(const char* fmt, ...);

  //! Append `c` character.
  ASMJIT_INLINE bool appendChar(char c) {
    return _opChar(kStringOpAppend, c);
  }

  //! Append `c` of `len`.
  ASMJIT_INLINE bool appendChars(char c, size_t len) {
    return _opChars(kStringOpAppend, c, len);
  }

  //! Append `i`.
  ASMJIT_INLINE bool appendInt(int64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) {
    return _opNumber(kStringOpAppend, static_cast<uint64_t>(i), base, width, flags | kStringFormatSigned);
  }

  //! Append `i`.
  ASMJIT_INLINE bool appendUInt(uint64_t i, uint32_t base = 0, size_t width = 0, uint32_t flags = 0) {
    return _opNumber(kStringOpAppend, i, base, width, flags);
  }

  //! Append the given `data` converted to a HEX string.
  ASMJIT_INLINE bool appendHex(const void* data, size_t len) {
    return _opHex(kStringOpAppend, data, len);
  }

  // --------------------------------------------------------------------------
  // [_Append]
  // --------------------------------------------------------------------------

  //! Append `str` of `len`, inlined, without buffer overflow check.
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

  //! Append `c` character, inlined, without buffer overflow check.
  ASMJIT_INLINE void _appendChar(char c) {
    ASMJIT_ASSERT(_capacity - _length >= 1);

    _data[_length] = c;
    _length++;
    _data[_length] = '\0';
  }

  //! Append `c` of `len`, inlined, without buffer overflow check.
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

  //! Check for equality with other `str` of `len`.
  ASMJIT_API bool eq(const char* str, size_t len = kInvalidIndex) const;
  //! Check for equality with `other`.
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

  //! String data.
  char* _data;
  //! Length.
  size_t _length;
  //! Capacity.
  size_t _capacity;
  //! Whether the string can be freed.
  size_t _canFree;
};

// ============================================================================
// [asmjit::StringBuilderTmp]
// ============================================================================

//! Temporary string builder, has statically allocated `N` bytes.
template<size_t N>
struct StringBuilderTmp : public StringBuilder {
  ASMJIT_NO_COPY(StringBuilderTmp<N>)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE StringBuilderTmp() : StringBuilder(NoInit) {
    _data = _embeddedData;
    _data[0] = 0;

    _length = 0;
    _capacity = N;
    _canFree = false;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Embedded data.
  char _embeddedData[static_cast<size_t>(
    N + 1 + sizeof(intptr_t)) & ~static_cast<size_t>(sizeof(intptr_t) - 1)];
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CONTAINERS_H
