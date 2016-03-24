// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_PODVECTOR_H
#define _ASMJIT_BASE_PODVECTOR_H

// [Dependencies - AsmJit]
#include "../base/globals.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::PodVectorBase]
// ============================================================================

//! \internal
struct PodVectorBase {
  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  //! \internal
  struct Data {
    //! Get data.
    ASMJIT_INLINE void* getData() const noexcept {
      return static_cast<void*>(const_cast<Data*>(this + 1));
    }

    //! Capacity of the vector.
    size_t capacity;
    //! Length of the vector.
    size_t length;
  };

  static ASMJIT_API const Data _nullData;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `PodVectorBase`.
  ASMJIT_INLINE PodVectorBase() noexcept : _d(const_cast<Data*>(&_nullData)) {}
  //! Destroy the `PodVectorBase` and its data.
  ASMJIT_INLINE ~PodVectorBase() noexcept { reset(true); }

protected:
  explicit ASMJIT_INLINE PodVectorBase(Data* d) noexcept : _d(d) {}

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

public:
  //! Reset the vector data and set its `length` to zero.
  //!
  //! If `releaseMemory` is true the vector buffer will be released to the
  //! system.
  ASMJIT_API void reset(bool releaseMemory = false) noexcept;

  // --------------------------------------------------------------------------
  // [Grow / Reserve]
  // --------------------------------------------------------------------------

protected:
  ASMJIT_API Error _grow(size_t n, size_t sizeOfT) noexcept;
  ASMJIT_API Error _reserve(size_t n, size_t sizeOfT) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

public:
  Data* _d;
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
  ASMJIT_INLINE PodVector() noexcept {}
  //! Destroy the `PodVector<T>` and its data.
  ASMJIT_INLINE ~PodVector() noexcept {}

protected:
  explicit ASMJIT_INLINE PodVector(Data* d) noexcept : PodVectorBase(d) {}

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

public:
  //! Get whether the vector is empty.
  ASMJIT_INLINE bool isEmpty() const noexcept { return _d->length == 0; }
  //! Get length.
  ASMJIT_INLINE size_t getLength() const noexcept { return _d->length; }
  //! Get capacity.
  ASMJIT_INLINE size_t getCapacity() const noexcept { return _d->capacity; }
  //! Get data.
  ASMJIT_INLINE T* getData() noexcept { return static_cast<T*>(_d->getData()); }
  //! \overload
  ASMJIT_INLINE const T* getData() const noexcept { return static_cast<const T*>(_d->getData()); }

  // --------------------------------------------------------------------------
  // [Grow / Reserve]
  // --------------------------------------------------------------------------

  //! Called to grow the buffer to fit at least `n` elements more.
  ASMJIT_INLINE Error _grow(size_t n) noexcept { return PodVectorBase::_grow(n, sizeof(T)); }
  //! Realloc internal array to fit at least `n` items.
  ASMJIT_INLINE Error _reserve(size_t n) noexcept { return PodVectorBase::_reserve(n, sizeof(T)); }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Prepend `item` to vector.
  Error prepend(const T& item) noexcept {
    Data* d = _d;

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
  Error insert(size_t index, const T& item) noexcept {
    Data* d = _d;
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
  Error append(const T& item) noexcept {
    Data* d = _d;

    if (d->length == d->capacity) {
      ASMJIT_PROPAGATE_ERROR(_grow(1));
      d = _d;
    }

    ::memcpy(static_cast<T*>(d->getData()) + d->length, &item, sizeof(T));

    d->length++;
    return kErrorOk;
  }

  //! Get index of `val` or `kInvalidIndex` if not found.
  size_t indexOf(const T& val) const noexcept {
    Data* d = _d;

    const T* data = static_cast<const T*>(d->getData());
    size_t len = d->length;

    for (size_t i = 0; i < len; i++)
      if (data[i] == val)
        return i;

    return kInvalidIndex;
  }

  //! Remove item at index `i`.
  void removeAt(size_t i) noexcept {
    Data* d = _d;
    ASMJIT_ASSERT(i < d->length);

    T* data = static_cast<T*>(d->getData()) + i;
    d->length--;
    ::memmove(data, data + 1, d->length - i);
  }

  //! Swap this pod-vector with `other`.
  void swap(PodVector<T>& other) noexcept {
    T* otherData = other._d;
    other._d = _d;
    _d = otherData;
  }

  //! Get item at index `i`.
  ASMJIT_INLINE T& operator[](size_t i) noexcept {
    ASMJIT_ASSERT(i < getLength());
    return getData()[i];
  }

  //! Get item at index `i`.
  ASMJIT_INLINE const T& operator[](size_t i) const noexcept {
    ASMJIT_ASSERT(i < getLength());
    return getData()[i];
  }
};

// ============================================================================
// [asmjit::PodVectorTmp<T>]
// ============================================================================

template<typename T, size_t N>
struct PodVectorTmp : public PodVector<T> {
  ASMJIT_NO_COPY(PodVectorTmp<T, N>)

  // --------------------------------------------------------------------------
  // [StaticData]
  // --------------------------------------------------------------------------

  struct StaticData : public PodVectorBase::Data {
    char data[sizeof(T) * N];
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `PodVectorTmp<T>`.
  ASMJIT_INLINE PodVectorTmp() noexcept : PodVector<T>(&_staticData) {
    _staticData.capacity = N;
    _staticData.length = 0;
  }
  //! Destroy the `PodVectorTmp<T>` and its data.
  ASMJIT_INLINE ~PodVectorTmp() noexcept {}

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  StaticData _staticData;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_PODVECTOR_H
