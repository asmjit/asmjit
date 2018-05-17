// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONEVECTOR_H
#define _ASMJIT_CORE_ZONEVECTOR_H

// [Dependencies]
#include "../core/support.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_zone
//! \{

// ============================================================================
// [asmjit::ZoneVectorBase]
// ============================================================================

//! \internal
class ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVectorBase)

  // STL compatibility;
  typedef uint32_t size_type;
  typedef ptrdiff_t difference_type;

protected:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `ZoneVectorBase`.
  inline ZoneVectorBase() noexcept
    : _data(nullptr),
      _size(0),
      _capacity(0) {}

  inline ZoneVectorBase(ZoneVectorBase&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

public:
  //! Get whether the vector is empty.
  inline bool empty() const noexcept { return _size == 0; }
  //! Get vector size.
  inline size_type size() const noexcept { return _size; }
  //! Get vector capacity.
  inline size_type capacity() const noexcept { return _capacity; }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Makes the vector empty (won't change the capacity or data pointer).
  inline void clear() noexcept { _size = 0; }
  //! Reset the vector data and set its `size` to zero.
  inline void reset() noexcept {
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }

  //! Truncate the vector to at most `n` items.
  inline void truncate(size_type n) noexcept {
    _size = std::min(_size, n);
  }

  //! Set size of the vector to `n`. Used internally by some algorithms.
  inline void _setSize(size_type n) noexcept {
    ASMJIT_ASSERT(n <= _capacity);
    _size = n;
  }

  // --------------------------------------------------------------------------
  // [Memory Management]
  // --------------------------------------------------------------------------

protected:
  inline void _release(ZoneAllocator* allocator, uint32_t sizeOfT) noexcept {
    if (_data != nullptr) {
      allocator->release(_data, _capacity * sizeOfT);
      reset();
    }
  }

  ASMJIT_API Error _grow(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;
  ASMJIT_API Error _resize(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;
  ASMJIT_API Error _reserve(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept;

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneVectorBase& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

public:
  void* _data;                           //!< Vector data (untyped).
  size_type _size;                       //!< Size of the vector.
  size_type _capacity;                   //!< Capacity of the vector.
};

// ============================================================================
// [asmjit::ZoneVector<T>]
// ============================================================================

//! Template used to store and manage array of Zone allocated data.
//!
//! This template has these advantages over other std::vector<>:
//! - Always non-copyable (designed to be non-copyable, we want it).
//! - No copy-on-write (some implementations of STL can use it).
//! - Optimized for working only with POD types.
//! - Uses ZoneAllocator, thus small vectors are basically for free.
template <typename T>
class ZoneVector : public ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVector<T>)

  // STL compatibility;
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;

  typedef Support::Iterator<T> iterator;
  typedef Support::Iterator<const T> const_iterator;
  typedef Support::ReverseIterator<T> reverse_iterator;
  typedef Support::ReverseIterator<const T> const_reverse_iterator;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneVector() noexcept : ZoneVectorBase() {}
  inline ZoneVector(ZoneVector&& other) noexcept : ZoneVector(other) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get data.
  inline T* data() noexcept { return static_cast<T*>(_data); }
  //! \overload
  inline const T* data() const noexcept { return static_cast<const T*>(_data); }

  //! Get item at index `i` (const).
  inline const T& at(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }

  inline void _setEndPtr(T* p) noexcept {
    ASMJIT_ASSERT(p >= data() && p <= data() + _capacity);
    _setSize(uint32_t((uintptr_t)(p - data())));
  }

  // --------------------------------------------------------------------------
  // [Iterators]
  // --------------------------------------------------------------------------

  // STL compatibility.
  inline iterator begin() noexcept { return iterator(data()); };
  inline const_iterator begin() const noexcept { return const_iterator(data()); };

  inline iterator end() noexcept { return iterator(data() + _size); };
  inline const_iterator end() const noexcept { return const_iterator(data() + _size); };

  inline reverse_iterator rbegin() noexcept { return reverse_iterator(data()); };
  inline const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(data()); };

  inline reverse_iterator rend() noexcept { return reverse_iterator(data() + _size); };
  inline const_reverse_iterator rend() const noexcept { return const_reverse_iterator(data() + _size); };

  inline const_iterator cbegin() const noexcept { return const_iterator(data()); };
  inline const_iterator cend() const noexcept { return const_iterator(data() + _size); };

  inline const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(data()); };
  inline const_reverse_iterator crend() const noexcept { return const_reverse_iterator(data() + _size); };

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  //! Prepend `item` to the vector.
  inline Error prepend(ZoneAllocator* allocator, const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_size == _capacity))
      ASMJIT_PROPAGATE(grow(allocator, 1));

    std::memmove(static_cast<T*>(_data) + 1, _data, size_t(_size) * sizeof(T));
    std::memcpy(_data, &item, sizeof(T));

    _size++;
    return kErrorOk;
  }

  //! Insert an `item` at the specified `index`.
  inline Error insert(ZoneAllocator* allocator, uint32_t index, const T& item) noexcept {
    ASMJIT_ASSERT(index <= _size);

    if (ASMJIT_UNLIKELY(_size == _capacity))
      ASMJIT_PROPAGATE(grow(allocator, 1));

    T* dst = static_cast<T*>(_data) + index;
    std::memmove(dst + 1, dst, size_t(_size - index) * sizeof(T));
    std::memcpy(dst, &item, sizeof(T));
    _size++;

    return kErrorOk;
  }

  //! Append `item` to the vector.
  inline Error append(ZoneAllocator* allocator, const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_size == _capacity))
      ASMJIT_PROPAGATE(grow(allocator, 1));

    std::memcpy(static_cast<T*>(_data) + _size, &item, sizeof(T));
    _size++;

    return kErrorOk;
  }

  inline Error concat(ZoneAllocator* allocator, const ZoneVector<T>& other) noexcept {
    uint32_t size = other._size;
    if (_capacity - _size < size)
      ASMJIT_PROPAGATE(grow(allocator, size));

    if (size) {
      std::memcpy(static_cast<T*>(_data) + _size, other._data, size_t(size) * sizeof(T));
      _size += size;
    }

    return kErrorOk;
  }

  //! Prepend `item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns
  //! `kErrorOk` then N elements can be added to the vector without checking
  //! if there is a place for them. Used mostly internally.
  inline void prependUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);
    T* data = static_cast<T*>(_data);

    if (_size)
      std::memmove(data + 1, data, size_t(_size) * sizeof(T));

    std::memcpy(data, &item, sizeof(T));
    _size++;
  }

  //! Append `item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns
  //! `kErrorOk` then N elements can be added to the vector without checking
  //! if there is a place for them. Used mostly internally.
  inline void appendUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_size < _capacity);

    std::memcpy(static_cast<T*>(_data) + _size, &item, sizeof(T));
    _size++;
  }

  //! Concatenate all items of `other` at the end of the vector.
  inline void concatUnsafe(const ZoneVector<T>& other) noexcept {
    uint32_t size = other._size;
    ASMJIT_ASSERT(_capacity - _size >= size);

    if (size) {
      std::memcpy(static_cast<T*>(_data) + _size, other._data, size_t(size) * sizeof(T));
      _size += size;
    }
  }

  //! Get index of `val` or `Globals::kNotFound` if not found.
  inline uint32_t indexOf(const T& val) const noexcept {
    const T* data = static_cast<const T*>(_data);
    uint32_t size = _size;

    for (uint32_t i = 0; i < size; i++)
      if (data[i] == val)
        return i;
    return Globals::kNotFound;
  }

  //! Get whether the vector contains `val`.
  inline bool contains(const T& val) const noexcept {
    return indexOf(val) != Globals::kNotFound;
  }

  //! Remove item at index `i`.
  inline void removeAt(uint32_t i) noexcept {
    ASMJIT_ASSERT(i < _size);

    T* data = static_cast<T*>(_data) + i;
    uint32_t size = --_size - i;

    if (size)
      std::memmove(data, data + 1, size_t(size) * sizeof(T));
  }

  inline T pop() noexcept {
    ASMJIT_ASSERT(_size > 0);

    uint32_t index = --_size;
    return data()[index];
  }

  //! Swap this pod-vector with `other`.
  inline void swap(ZoneVector<T>& other) noexcept {
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_data, other._data);
  }

  template<typename CompareT = Support::Compare<Support::kSortAscending>>
  inline void sort(const CompareT& cmp = CompareT()) noexcept {
    Support::qSort<T, CompareT>(data(), size(), cmp);
  }

  //! Get item at index `i`.
  inline T& operator[](uint32_t i) noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }

  //! Get item at index `i`.
  inline const T& operator[](uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _size);
    return data()[i];
  }

  inline T& first() noexcept { return operator[](0); }
  inline const T& first() const noexcept { return operator[](0); }

  inline T& last() noexcept { return operator[](_size - 1); }
  inline const T& last() const noexcept { return operator[](_size - 1); }

  // --------------------------------------------------------------------------
  // [Memory Management]
  // --------------------------------------------------------------------------

  //! Release the memory held by `ZoneVector<T>` back to the `allocator`.
  inline void release(ZoneAllocator* allocator) noexcept {
    _release(allocator, sizeof(T));
  }

  //! Called to grow the buffer to fit at least `n` elements more.
  inline Error grow(ZoneAllocator* allocator, uint32_t n) noexcept {
    return ZoneVectorBase::_grow(allocator, sizeof(T), n);
  }

  //! Resize the vector to hold `n` elements.
  //!
  //! If `n` is greater than the current size then the additional elements'
  //! content will be initialized to zero. If `n` is less than the current
  //! size then the vector will be truncated to exactly `n` elements.
  inline Error resize(ZoneAllocator* allocator, uint32_t n) noexcept {
    return ZoneVectorBase::_resize(allocator, sizeof(T), n);
  }

  //! Realloc internal array to fit at least `n` items.
  inline Error reserve(ZoneAllocator* allocator, uint32_t n) noexcept {
    return n > _capacity ? ZoneVectorBase::_reserve(allocator, sizeof(T), n) : Error(kErrorOk);
  }

  inline Error willGrow(ZoneAllocator* allocator, uint32_t n = 1) noexcept {
    return _capacity - _size < n ? grow(allocator, n) : Error(kErrorOk);
  }

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneVector<T>& other) noexcept {
    ZoneVectorBase::swapWith(other);
  }
};

// ============================================================================
// [asmjit::ZoneBitVector]
// ============================================================================

class ZoneBitVector {
public:
  ASMJIT_NONCOPYABLE(ZoneBitVector)

  typedef Support::BitWord BitWord;
  static constexpr uint32_t kBitWordSizeInBits = Support::kBitWordSizeInBits;

  static inline uint32_t _wordsPerBits(uint32_t nBits) noexcept {
    return ((nBits + kBitWordSizeInBits - 1) / kBitWordSizeInBits);
  }

  static inline void _zeroBits(BitWord* dst, uint32_t nBitWords) noexcept {
    for (uint32_t i = 0; i < nBitWords; i++)
      dst[i] = 0;
  }

  static inline void _fillBits(BitWord* dst, uint32_t nBitWords) noexcept {
    for (uint32_t i = 0; i < nBitWords; i++)
      dst[i] = ~BitWord(0);
  }

  static inline void _copyBits(BitWord* dst, const BitWord* src, uint32_t nBitWords) noexcept {
    for (uint32_t i = 0; i < nBitWords; i++)
      dst[i] = src[i];
  }

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneBitVector() noexcept
    : _data(nullptr),
      _size(0),
      _capacity(0) {}

  inline ZoneBitVector(ZoneBitVector&& other) noexcept
    : _data(other._data),
      _size(other._size),
      _capacity(other._capacity) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get whether the bit-vector is empty (has no bits).
  inline bool empty() const noexcept { return _size == 0; }
  //! Get a size of this bit-vector (in bits).
  inline uint32_t size() const noexcept { return _size; }
  //! Get a capacity of this bit-vector (in bits).
  inline uint32_t capacity() const noexcept { return _capacity; }

  //! Get a count of `BitWord[]` array need to store all bits.
  inline uint32_t sizeInBitWords() const noexcept { return _wordsPerBits(_size); }
  //! Get a count of `BitWord[]` array need to store all bits.
  inline uint32_t capacityInBitWords() const noexcept { return _wordsPerBits(_capacity); }

  //! Get data.
  inline BitWord* data() noexcept { return _data; }
  //! \overload
  inline const BitWord* data() const noexcept { return _data; }

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  inline void clear() noexcept {
    _size = 0;
  }

  inline void reset() noexcept {
    _data = nullptr;
    _size = 0;
    _capacity = 0;
  }

  inline void truncate(uint32_t newSize) noexcept {
    _size = std::min(_size, newSize);
    _clearUnusedBits();
  }

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

  inline void clearAll() noexcept {
    _zeroBits(_data, _wordsPerBits(_size));
  }

  inline void fillAll() noexcept {
    _fillBits(_data, _wordsPerBits(_size));
    _clearUnusedBits();
  }

  inline void clearBits(uint32_t start, uint32_t count) noexcept {
    ASMJIT_ASSERT(start <= _size);
    ASMJIT_ASSERT(_size - start >= count);

    Support::bitVectorClear(_data, start, count);
  }

  inline void fillBits(uint32_t start, uint32_t count) noexcept {
    ASMJIT_ASSERT(start <= _size);
    ASMJIT_ASSERT(_size - start >= count);

    Support::bitVectorFill(_data, start, count);
  }

  //! Perform a logical bitwise AND between bits specified in this array and bits
  //! in `other`. If `other` has less bits than `this` then all remaining bits are
  //! set to zero.
  //!
  //! NOTE: The size of the BitVector is unaffected by this operation.
  inline void and_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t thisBitWordCount = sizeInBitWords();
    uint32_t otherBitWordCount = other.sizeInBitWords();
    uint32_t commonBitWordCount = std::min(thisBitWordCount, otherBitWordCount);

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

  //! Perform a logical bitwise AND between bits specified in this array and
  //! negated bits in `other`. If `other` has less bits than `this` then all
  //! remaining bits are kept intact.
  //!
  //! NOTE: The size of the BitVector is unaffected by this operation.
  inline void andNot(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t commonBitWordCount = _wordsPerBits(std::min(_size, other._size));
    for (uint32_t i = 0; i < commonBitWordCount; i++)
      dst[i] = dst[i] & ~src[i];
  }

  //! Perform a logical bitwise OP between bits specified in this array and bits
  //! in `other`. If `other` has less bits than `this` then all remaining bits
  //! are kept intact.
  //!
  //! NOTE: The size of the BitVector is unaffected by this operation.
  inline void or_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t commonBitWordCount = _wordsPerBits(std::min(_size, other._size));
    for (uint32_t i = 0; i < commonBitWordCount; i++)
      dst[i] = dst[i] | src[i];
    _clearUnusedBits();
  }

  inline void _clearUnusedBits() noexcept {
    uint32_t idx = _size / kBitWordSizeInBits;
    uint32_t bit = _size % kBitWordSizeInBits;

    if (!bit) return;
    _data[idx] &= (BitWord(1) << bit) - 1u;
  }

  inline bool eq(const ZoneBitVector& other) const noexcept {
    if (_size != other._size)
      return false;

    const BitWord* aData = _data;
    const BitWord* bData = other._data;
    uint32_t numBitWords = _wordsPerBits(_size);

    for (uint32_t i = 0; i < numBitWords; i++)
      if (aData[i] != bData[i])
        return false;
    return true;
  }

  inline bool operator==(const ZoneBitVector& other) const noexcept { return  eq(other); }
  inline bool operator!=(const ZoneBitVector& other) const noexcept { return !eq(other); }

  // --------------------------------------------------------------------------
  // [Memory Management]
  // --------------------------------------------------------------------------

  inline void release(ZoneAllocator* allocator) noexcept {
    if (!_data) return;
    allocator->release(_data, _capacity / 8);
    reset();
  }

  inline Error resize(ZoneAllocator* allocator, uint32_t newSize, bool newBitsValue = false) noexcept {
    return _resize(allocator, newSize, newSize, newBitsValue);
  }

  ASMJIT_API Error _resize(ZoneAllocator* allocator, uint32_t newSize, uint32_t idealCapacity, bool newBitsValue) noexcept;
  ASMJIT_API Error _append(ZoneAllocator* allocator, bool value) noexcept;

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneBitVector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
  }

  // --------------------------------------------------------------------------
  // [Iterators]
  // --------------------------------------------------------------------------

  class ForEachBitSet : public Support::BitVectorIterator<BitWord> {
  public:
    ASMJIT_INLINE explicit ForEachBitSet(const ZoneBitVector& bitVector) noexcept
      : Support::BitVectorIterator<BitWord>(bitVector.data(), bitVector.sizeInBitWords()) {}
  };

  template<class Operator>
  class ForEachBitOp : public Support::BitVectorOpIterator<BitWord, Operator> {
  public:
    ASMJIT_INLINE ForEachBitOp(const ZoneBitVector& a, const ZoneBitVector& b) noexcept
      : Support::BitVectorOpIterator<BitWord, Operator>(a.data(), b.data(), a.sizeInBitWords()) {
      ASMJIT_ASSERT(a.size() == b.size());
    }
  };

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BitWord* _data;                        //!< Bits.
  uint32_t _size;                        //!< Size of the bit-vector (in bits).
  uint32_t _capacity;                    //!< Capacity of the bit-vector (in bits).
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONEVECTOR_H
