// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_ZONEVECTOR_H_INCLUDED
#define ASMJIT_CORE_ZONEVECTOR_H_INCLUDED

#include "../core/support.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_zone
//! \{

//! Base class used by \ref ZoneVector template.
class ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVectorBase)

  //! \name Types (C++ compatibility)
  //! \{

  using size_type = uint32_t;
  using difference_type = ptrdiff_t;

  //! \}

  //! \name Members
  //! \{

  //! Vector data (untyped).
  void* _data = nullptr;
  //! Size of the vector.
  size_type _size = 0;
  //! Capacity of the vector.
  size_type _capacity = 0;

  //! \}

protected:
  //! \name Construction & Destruction
  //! \{

  //! Creates a new instance of `ZoneVectorBase`.
  ASMJIT_INLINE_NODEBUG ZoneVectorBase() noexcept {}

  ASMJIT_INLINE_NODEBUG ZoneVectorBase(ZoneVectorBase&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) { other.reset(); }

  //! \}

  //! \cond INTERNAL
  //! \name Internal
  //! \{

  inline void _release(ZoneAllocator* allocator, uint32_t sizeOfT) noexcept {
    if (_data != nullptr) {
      allocator->release(_data, _capacity * sizeOfT);
      reset();
    }
  }

  ASMJIT_INLINE_NODEBUG void _moveFrom(ZoneVectorBase&& other) noexcept {
    void* data = other._data;
    size_type size = other._size = 0;
    size_type capacity = other._capacity = 0;

    other._data = nullptr;
    other._size = 0;
    other._capacity = 0;

    _data = data;
    _size = size;
    _capacity = capacity;
  }

  ASMJIT_API Error _grow(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;
  ASMJIT_API Error _resize(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;
  ASMJIT_API Error _reserve(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;
  ASMJIT_API Error _growingReserve(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;

  ASMJIT_INLINE_NODEBUG void _swap(ZoneVectorBase& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  //! \}
  //! \endcond

public:
  //! \name Accessors
  //! \{

  //! Tests whether the vector is empty.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool empty() const noexcept { return _size == 0; }

  //! Returns the vector size.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_type size() const noexcept { return _size; }

  //! Returns the vector capacity.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_type capacity() const noexcept { return _capacity; }

  //! \}

  //! \name Utilities
  //! \{

  //! Makes the vector empty (won't change the capacity or data pointer).
  ASMJIT_INLINE_NODEBUG void clear() noexcept { _size = 0; }
  //! Resets the vector data and set its `size` to zero.
  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }

  //! Truncates the vector to at most `n` items.
  ASMJIT_INLINE_NODEBUG void truncate(size_type n) noexcept {
    _size = Support::min(_size, n);
  }

  //! Sets size of the vector to `n`. Used internally by some algorithms.
  inline void _setSize(size_type n) noexcept {
    ASMJIT_ASSERT(n <= _capacity);
    _size = n;
  }

  //! \}
};

//! Template used to store and manage array of Zone allocated data.
//!
//! This template has these advantages over other std::vector<>:
//! - Always non-copyable (designed to be non-copyable, we want it).
//! - Optimized for working only with POD types.
//! - Uses ZoneAllocator, thus small vectors are almost for free.
//! - Explicit allocation, ZoneAllocator is not part of the data.
template <typename T>
class ZoneVector : public ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVector)

  //! \name Types (C++ compatibility)
  //! \{

  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

  using iterator = T*;
  using const_iterator = const T*;
  using reverse_iterator = Support::ArrayReverseIterator<T>;
  using const_reverse_iterator = Support::ArrayReverseIterator<const T>;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a default constructed ZoneVector (data pointer is null, and both length/capacity is zero).
  ASMJIT_INLINE_NODEBUG ZoneVector() noexcept : ZoneVectorBase() {}

  //! Moves an existing zone vector into this instance and resets the `other` instance.
  ASMJIT_INLINE_NODEBUG ZoneVector(ZoneVector&& other) noexcept
    : ZoneVectorBase(std::move(other)) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  //! Implements a move assignment operator. The `other` instance is reset before this instance is set.
  //!
  //! \note It's recommended to first release the memory of the destination vector as there is no way
  //! how to do it after the move, unless it's guaranteed that the destination vector is default
  //! constructed.
  ASMJIT_INLINE_NODEBUG ZoneVector& operator=(ZoneVector&& other) noexcept {
    _moveFrom(other);
    return *this;
  }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns vector data (mutable).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T* data() noexcept { return static_cast<T*>(_data); }

  //! Returns vector data (const)
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* data() const noexcept { return static_cast<const T*>(_data); }

  //! Returns vector data (const)
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T* cdata() const noexcept { return static_cast<const T*>(_data); }

  //! Returns item at the given index `i` (const).
  [[nodiscard]]
  inline const T& at(size_t i) const noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }

  inline void _setEndPtr(T* p) noexcept {
    ASMJIT_ASSERT(p >= data() && p <= data() + _capacity);
    _setSize(uint32_t((uintptr_t)(p - data())));
  }

  //! \}

  //! \name STL Compatibility (Iterators)
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG iterator begin() noexcept { return iterator(data()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator begin() const noexcept { return const_iterator(data()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG iterator end() noexcept { return iterator(data() + _size); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator end() const noexcept { return const_iterator(data() + _size); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG reverse_iterator rbegin() noexcept { return reverse_iterator(end()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG reverse_iterator rend() noexcept { return reverse_iterator(begin()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator cbegin() const noexcept { return const_iterator(data()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_iterator cend() const noexcept { return const_iterator(data() + _size); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); };

  //! \}

  //! \name Utilities
  //! \{

  //! Swaps this vector with `other`.
  ASMJIT_INLINE void swap(ZoneVector<T>& other) noexcept { _swap(other); }

  //! Prepends `item` to the vector.
  ASMJIT_INLINE Error prepend(ZoneAllocator* allocator, const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_size == _capacity)) {
      ASMJIT_PROPAGATE(grow(allocator, 1));
    }

    memmove(static_cast<void*>(static_cast<T*>(_data) + 1),
            static_cast<const void*>(_data),
            size_t(_size) * sizeof(T));

    memcpy(static_cast<void*>(_data),
           static_cast<const void*>(&item),
           sizeof(T));

    _size++;
    return kErrorOk;
  }

  //! Inserts an `item` at the specified `index`.
  ASMJIT_INLINE Error insert(ZoneAllocator* allocator, size_t index, const T& item) noexcept {
    ASMJIT_ASSERT(index <= _size);

    if (ASMJIT_UNLIKELY(_size == _capacity)) {
      ASMJIT_PROPAGATE(grow(allocator, 1));
    }

    T* dst = static_cast<T*>(_data) + index;
    memmove(static_cast<void*>(dst + 1),
            static_cast<const void*>(dst),
            size_t(_size - index) * sizeof(T));

    memcpy(static_cast<void*>(dst),
           static_cast<const void*>(&item),
           sizeof(T));

    _size++;
    return kErrorOk;
  }

  //! Appends `item` to the vector.
  ASMJIT_INLINE Error append(ZoneAllocator* allocator, const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_size == _capacity)) {
      ASMJIT_PROPAGATE(grow(allocator, 1));
    }

    memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
           static_cast<const void*>(&item),
           sizeof(T));

    _size++;
    return kErrorOk;
  }

  //! Appends `other` vector at the end of this vector.
  ASMJIT_INLINE Error concat(ZoneAllocator* allocator, const ZoneVector<T>& other) noexcept {
    uint32_t size = other._size;
    if (_capacity - _size < size) {
      ASMJIT_PROPAGATE(grow(allocator, size));
    }

    if (size) {
      memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
             static_cast<const void*>(other._data),
             size_t(size) * sizeof(T));
      _size += size;
    }

    return kErrorOk;
  }

  //! Prepends `item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns `kErrorOk` then N elements
  //! can be added to the vector without checking if there is a place for them. Used mostly internally.
  ASMJIT_INLINE void prependUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);
    T* data = static_cast<T*>(_data);

    if (_size) {
      memmove(static_cast<void*>(data + 1),
              static_cast<const void*>(data),
              size_t(_size) * sizeof(T));
    }

    memcpy(static_cast<void*>(data),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
  }

  //! Append s`item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns `kErrorOk` then N elements
  //! can be added to the vector without checking if there is a place for them. Used mostly internally.
  ASMJIT_INLINE void appendUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);

    memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
           static_cast<const void*>(&item),
           sizeof(T));
    _size++;
  }

  //! Inserts an `item` at the specified `index` (unsafe case).
  ASMJIT_INLINE void insertUnsafe(size_t index, const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);
    ASMJIT_ASSERT(index <= _size);

    T* dst = static_cast<T*>(_data) + index;
    memmove(static_cast<void*>(dst + 1),
            static_cast<const void*>(dst),
            size_t(_size - index) * sizeof(T));

    memcpy(static_cast<void*>(dst),
           static_cast<const void*>(&item),
           sizeof(T));

    _size++;
  }

  //! Concatenates all items of `other` at the end of the vector.
  ASMJIT_INLINE void concatUnsafe(const ZoneVector<T>& other) noexcept {
    uint32_t size = other._size;
    ASMJIT_ASSERT(_capacity - _size >= size);

    if (size) {
      memcpy(static_cast<void*>(static_cast<T*>(_data) + _size),
             static_cast<const void*>(other._data),
             size_t(size) * sizeof(T));
      _size += size;
    }
  }

  //! Returns index of the given `val` or `Globals::kNotFound` if it doesn't exist.
  ASMJIT_INLINE uint32_t indexOf(const T& val) const noexcept {
    const T* data = static_cast<const T*>(_data);
    uint32_t size = _size;

    for (uint32_t i = 0; i < size; i++)
      if (data[i] == val)
        return i;
    return Globals::kNotFound;
  }

  //! Tests whether the vector contains `val`.
  inline bool contains(const T& val) const noexcept {
    return indexOf(val) != Globals::kNotFound;
  }

  //! Removes item at index `i`.
  inline void removeAt(size_t i) noexcept {
    ASMJIT_ASSERT(i < _size);

    T* data = static_cast<T*>(_data) + i;
    size_t size = --_size - i;

    if (size) {
      memmove(static_cast<void*>(data),
              static_cast<const void*>(data + 1),
              size_t(size) * sizeof(T));
    }
  }

  //! Pops the last element from the vector and returns it.
  [[nodiscard]]
  inline T pop() noexcept {
    ASMJIT_ASSERT(_size > 0);

    uint32_t index = --_size;
    return data()[index];
  }

  template<typename CompareT = Support::Compare<Support::SortOrder::kAscending>>
  inline void sort(const CompareT& cmp = CompareT()) noexcept {
    Support::qSort<T, CompareT>(data(), size(), cmp);
  }

  //! Returns item at index `i`.
  [[nodiscard]]
  inline T& operator[](size_t i) noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }

  //! Returns item at index `i`.
  [[nodiscard]]
  inline const T& operator[](size_t i) const noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }

  //! Returns a reference to the first element of the vector.
  //!
  //! \note The vector must have at least one element. Attempting to use `first()` on empty vector will trigger
  //! an assertion failure in debug builds.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG T& first() noexcept { return operator[](0); }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const T& first() const noexcept { return operator[](0); }

  //! Returns a reference to the last element of the vector.
  //!
  //! \note The vector must have at least one element. Attempting to use `last()` on empty vector will trigger
  //! an assertion failure in debug builds.
  [[nodiscard]]
  inline T& last() noexcept { return operator[](_size - 1); }

  //! \overload
  [[nodiscard]]
  inline const T& last() const noexcept { return operator[](_size - 1); }

  //! \}

  //! \name Memory Management
  //! \{

  //! Releases the memory held by `ZoneVector<T>` back to the `allocator`.
  inline void release(ZoneAllocator* allocator) noexcept {
    _release(allocator, sizeof(T));
  }

  //! Called to grow the buffer to fit at least `n` elements more.
  [[nodiscard]]
  inline Error grow(ZoneAllocator* allocator, uint32_t n) noexcept {
    return ZoneVectorBase::_grow(allocator, sizeof(T), n);
  }

  //! Resizes the vector to hold `n` elements.
  //!
  //! If `n` is greater than the current size then the additional elements' content will be initialized to zero.
  //! If `n` is less than the current size then the vector will be truncated to exactly `n` elements.
  [[nodiscard]]
  inline Error resize(ZoneAllocator* allocator, uint32_t n) noexcept {
    return ZoneVectorBase::_resize(allocator, sizeof(T), n);
  }

  //! Reallocates the internal array to fit at least `n` items.
  [[nodiscard]]
  inline Error reserve(ZoneAllocator* allocator, uint32_t n) noexcept {
    if (ASMJIT_UNLIKELY(n > _capacity)) {
      return ZoneVectorBase::_reserve(allocator, sizeof(T), n);
    }
    else {
      return Error(kErrorOk);
    }
  }

  //! Reallocates the internal array to fit at least `n` items with growing semantics.
  //!
  //! If the vector is smaller than `n` the same growing calculations will be used as if N items were appended
  //! to an empty vector, which means reserving additional space for more append operations that could follow.
  [[nodiscard]]
  inline Error growingReserve(ZoneAllocator* allocator, uint32_t n) noexcept {
    if (ASMJIT_UNLIKELY(n > _capacity)) {
      return ZoneVectorBase::_growingReserve(allocator, sizeof(T), n);
    }
    else {
      return Error(kErrorOk);
    }
  }

  inline Error willGrow(ZoneAllocator* allocator, uint32_t n = 1) noexcept {
    return _capacity - _size < n ? grow(allocator, n) : Error(kErrorOk);
  }

  //! \}
};

//! Zone-allocated bit vector.
class ZoneBitVector {
public:
  ASMJIT_NONCOPYABLE(ZoneBitVector)

  //! \name Types
  //! \{

  using BitWord = Support::BitWord;

  //! \}

  //! \name Constants
  //! \{

  static inline constexpr uint32_t kBitWordSizeInBits = Support::kBitWordSizeInBits;

  //! \}

  //! \name Members
  //! \{

  //! Bits.
  BitWord* _data = nullptr;
  //! Size of the bit-vector (in bits).
  uint32_t _size = 0;
  //! Capacity of the bit-vector (in bits).
  uint32_t _capacity = 0;

  //! \}

  //! \cond INTERNAL
  //! \name Internal
  //! \{

  static ASMJIT_INLINE_NODEBUG uint32_t _wordsPerBits(uint32_t nBits) noexcept {
    return ((nBits + kBitWordSizeInBits - 1) / kBitWordSizeInBits);
  }

  static ASMJIT_INLINE_NODEBUG void _zeroBits(BitWord* dst, uint32_t nBitWords) noexcept {
    for (uint32_t i = 0; i < nBitWords; i++) {
      dst[i] = 0;
    }
  }

  static ASMJIT_INLINE_NODEBUG void _fillBits(BitWord* dst, uint32_t nBitWords) noexcept {
    for (uint32_t i = 0; i < nBitWords; i++) {
      dst[i] = ~BitWord(0);
    }
  }

  static ASMJIT_INLINE_NODEBUG void _copyBits(BitWord* dst, const BitWord* src, uint32_t nBitWords) noexcept {
    for (uint32_t i = 0; i < nBitWords; i++) {
      dst[i] = src[i];
    }
  }

  //! \}
  //! \endcond

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_NODEBUG ZoneBitVector() noexcept {}

  ASMJIT_INLINE_NODEBUG ZoneBitVector(ZoneBitVector&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator==(const ZoneBitVector& other) const noexcept { return  equals(other); }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool operator!=(const ZoneBitVector& other) const noexcept { return !equals(other); }

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the bit-vector is empty (has no bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool empty() const noexcept { return _size == 0; }

  //! Returns the size of this bit-vector (in bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t size() const noexcept { return _size; }

  //! Returns the capacity of this bit-vector (in bits).
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t capacity() const noexcept { return _capacity; }

  //! Returns the size of the `BitWord[]` array in `BitWord` units.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t sizeInBitWords() const noexcept { return _wordsPerBits(_size); }

  //! Returns the capacity of the `BitWord[]` array in `BitWord` units.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t capacityInBitWords() const noexcept { return _wordsPerBits(_capacity); }

  //! Returns bit-vector data as `BitWord[]`.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG BitWord* data() noexcept { return _data; }

  //! \overload
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG const BitWord* data() const noexcept { return _data; }

  //! \}

  //! \name Utilities
  //! \{

  ASMJIT_INLINE_NODEBUG void swap(ZoneBitVector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  ASMJIT_INLINE_NODEBUG void clear() noexcept {
    _size = 0;
  }

  ASMJIT_INLINE_NODEBUG void reset() noexcept {
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }

  ASMJIT_INLINE_NODEBUG void truncate(uint32_t newSize) noexcept {
    _size = Support::min(_size, newSize);
    _clearUnusedBits();
  }

  [[nodiscard]]
  inline bool bitAt(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return Support::bitVectorGetBit(_data, index);
  }

  inline void setBit(uint32_t index, bool value) noexcept {
    ASMJIT_ASSERT(index < _size);
    Support::bitVectorSetBit(_data, index, value);
  }

  inline void flipBit(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    Support::bitVectorFlipBit(_data, index);
  }

  ASMJIT_INLINE Error append(ZoneAllocator* allocator, bool value) noexcept {
    uint32_t index = _size;
    if (ASMJIT_UNLIKELY(index >= _capacity))
      return _append(allocator, value);

    uint32_t idx = index / kBitWordSizeInBits;
    uint32_t bit = index % kBitWordSizeInBits;

    if (bit == 0)
      _data[idx] = BitWord(value) << bit;
    else
      _data[idx] |= BitWord(value) << bit;

    _size++;
    return kErrorOk;
  }

  ASMJIT_API Error copyFrom(ZoneAllocator* allocator, const ZoneBitVector& other) noexcept;

  ASMJIT_INLINE void clearAll() noexcept {
    _zeroBits(_data, _wordsPerBits(_size));
  }

  ASMJIT_INLINE void fillAll() noexcept {
    _fillBits(_data, _wordsPerBits(_size));
    _clearUnusedBits();
  }

  ASMJIT_INLINE void clearBits(uint32_t start, uint32_t count) noexcept {
    ASMJIT_ASSERT(start <= _size);
    ASMJIT_ASSERT(_size - start >= count);

    Support::bitVectorClear(_data, start, count);
  }

  ASMJIT_INLINE void fillBits(uint32_t start, uint32_t count) noexcept {
    ASMJIT_ASSERT(start <= _size);
    ASMJIT_ASSERT(_size - start >= count);

    Support::bitVectorFill(_data, start, count);
  }

  //! Performs a logical bitwise AND between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are set to zero.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void and_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t thisBitWordCount = sizeInBitWords();
    uint32_t otherBitWordCount = other.sizeInBitWords();
    uint32_t commonBitWordCount = Support::min(thisBitWordCount, otherBitWordCount);

    uint32_t i = 0;
    while (i < commonBitWordCount) {
      dst[i] = dst[i] & src[i];
      i++;
    }

    while (i < thisBitWordCount) {
      dst[i] = 0;
      i++;
    }
  }

  //! Performs a logical bitwise AND between bits specified in this array and negated bits in `other`. If `other`
  //! has less bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void andNot(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t commonBitWordCount = _wordsPerBits(Support::min(_size, other._size));
    for (uint32_t i = 0; i < commonBitWordCount; i++) {
      dst[i] = dst[i] & ~src[i];
    }
  }

  //! Performs a logical bitwise OP between bits specified in this array and bits in `other`. If `other` has less
  //! bits than `this` then all remaining bits are kept intact.
  //!
  //! \note The size of the BitVector is unaffected by this operation.
  ASMJIT_INLINE void or_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t commonBitWordCount = _wordsPerBits(Support::min(_size, other._size));
    for (uint32_t i = 0; i < commonBitWordCount; i++) {
      dst[i] = dst[i] | src[i];
    }
    _clearUnusedBits();
  }

  ASMJIT_INLINE void _clearUnusedBits() noexcept {
    uint32_t idx = _size / kBitWordSizeInBits;
    uint32_t bit = _size % kBitWordSizeInBits;

    if (!bit) {
      return;
    }

    _data[idx] &= (BitWord(1) << bit) - 1u;
  }

  [[nodiscard]]
  ASMJIT_INLINE bool equals(const ZoneBitVector& other) const noexcept {
    if (_size != other._size) {
      return false;
    }

    const BitWord* aData = _data;
    const BitWord* bData = other._data;
    uint32_t numBitWords = _wordsPerBits(_size);

    for (uint32_t i = 0; i < numBitWords; i++) {
      if (aData[i] != bData[i]) {
        return false;
      }
    }
    return true;
  }

  //! \}

  //! \name Memory Management
  //! \{

  inline void release(ZoneAllocator* allocator) noexcept {
    if (!_data) {
      return;
    }

    allocator->release(_data, _capacity / 8);
    reset();
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Error resize(ZoneAllocator* allocator, uint32_t newSize, bool newBitsValue = false) noexcept {
    return _resize(allocator, newSize, newSize, newBitsValue);
  }

  ASMJIT_API Error _resize(ZoneAllocator* allocator, uint32_t newSize, uint32_t idealCapacity, bool newBitsValue) noexcept;
  ASMJIT_API Error _append(ZoneAllocator* allocator, bool value) noexcept;

  //! \}

  //! \name Iterators
  //! \{

  class ForEachBitSet : public Support::BitVectorIterator<BitWord> {
  public:
    inline explicit ForEachBitSet(const ZoneBitVector& bitVector) noexcept
      : Support::BitVectorIterator<BitWord>(bitVector.data(), bitVector.sizeInBitWords()) {}
  };

  template<class Operator>
  class ForEachBitOp : public Support::BitVectorOpIterator<BitWord, Operator> {
  public:
    inline ForEachBitOp(const ZoneBitVector& a, const ZoneBitVector& b) noexcept
      : Support::BitVectorOpIterator<BitWord, Operator>(a.data(), b.data(), a.sizeInBitWords()) {
      ASMJIT_ASSERT(a.size() == b.size());
    }
  };

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_ZONEVECTOR_H_INCLUDED
