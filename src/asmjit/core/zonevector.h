// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONEVECTOR_H
#define _ASMJIT_CORE_ZONEVECTOR_H

// [Dependencies]
#include "../core/algorithm.h"
#include "../core/intutils.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::ZoneVectorBase]
// ============================================================================

//! \internal
class ZoneVectorBase {
public:
  ASMJIT_NONCOPYABLE(ZoneVectorBase)

protected:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new instance of `ZoneVectorBase`.
  explicit inline ZoneVectorBase() noexcept
    : _data(nullptr),
      _length(0),
      _capacity(0) {}

  inline ZoneVectorBase(ZoneVectorBase&& other) noexcept
    : _data(other._data),
      _length(other._length),
      _capacity(other._capacity) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

public:
  //! Get whether the vector is empty.
  inline bool isEmpty() const noexcept { return _length == 0; }
  //! Get vector length.
  inline uint32_t getLength() const noexcept { return _length; }
  //! Get vector capacity.
  inline uint32_t getCapacity() const noexcept { return _capacity; }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Makes the vector empty (won't change the capacity or data pointer).
  inline void clear() noexcept { _length = 0; }
  //! Reset the vector data and set its `length` to zero.
  inline void reset() noexcept {
    _data = nullptr;
    _length = 0;
    _capacity = 0;
  }

  //! Truncate the vector to at most `n` items.
  inline void truncate(uint32_t n) noexcept {
    _length = std::min(_length, n);
  }

  //! Set length of the vector to `n`. Used internally by some algorithms.
  inline void _setLength(uint32_t n) noexcept {
    ASMJIT_ASSERT(n <= _capacity);
    _length = n;
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
    std::swap(_length, other._length);
    std::swap(_capacity, other._capacity);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

public:
  void* _data;                           //!< Vector data (untyped).
  uint32_t _length;                      //!< Length of the vector.
  uint32_t _capacity;                    //!< Capacity of the vector.
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

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneVector() noexcept : ZoneVectorBase() {}
  inline ZoneVector(ZoneVector&& other) noexcept : ZoneVector(other) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get data.
  inline T* getData() noexcept { return static_cast<T*>(_data); }
  //! \overload
  inline const T* getData() const noexcept { return static_cast<const T*>(_data); }

  //! Get item at index `i` (const).
  inline const T& getAt(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _length);
    return getData()[i];
  }

  inline void _setEndPtr(T* p) noexcept {
    ASMJIT_ASSERT(p >= getData() && p <= getData() + _capacity);
    _setLength(uint32_t((uintptr_t)(p - getData())));
  }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Prepend `item` to the vector.
  inline Error prepend(ZoneAllocator* allocator, const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_length == _capacity))
      ASMJIT_PROPAGATE(grow(allocator, 1));

    std::memmove(static_cast<T*>(_data) + 1, _data, size_t(_length) * sizeof(T));
    std::memcpy(_data, &item, sizeof(T));

    _length++;
    return kErrorOk;
  }

  //! Insert an `item` at the specified `index`.
  inline Error insert(ZoneAllocator* allocator, uint32_t index, const T& item) noexcept {
    ASMJIT_ASSERT(index <= _length);

    if (ASMJIT_UNLIKELY(_length == _capacity))
      ASMJIT_PROPAGATE(grow(allocator, 1));

    T* dst = static_cast<T*>(_data) + index;
    std::memmove(dst + 1, dst, size_t(_length - index) * sizeof(T));
    std::memcpy(dst, &item, sizeof(T));
    _length++;

    return kErrorOk;
  }

  //! Append `item` to the vector.
  inline Error append(ZoneAllocator* allocator, const T& item) noexcept {
    if (ASMJIT_UNLIKELY(_length == _capacity))
      ASMJIT_PROPAGATE(grow(allocator, 1));

    std::memcpy(static_cast<T*>(_data) + _length, &item, sizeof(T));
    _length++;

    return kErrorOk;
  }

  inline Error concat(ZoneAllocator* allocator, const ZoneVector<T>& other) noexcept {
    uint32_t count = other._length;
    if (_capacity - _length < count)
      ASMJIT_PROPAGATE(grow(allocator, count));

    if (count) {
      std::memcpy(static_cast<T*>(_data) + _length, other._data, size_t(count) * sizeof(T));
      _length += count;
    }

    return kErrorOk;
  }

  //! Prepend `item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns
  //! `kErrorOk` then N elements can be added to the vector without checking
  //! if there is a place for them. Used mostly internally.
  inline void prependUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_length < _capacity);
    T* data = static_cast<T*>(_data);

    if (_length)
      std::memmove(data + 1, data, size_t(_length) * sizeof(T));

    std::memcpy(data, &item, sizeof(T));
    _length++;
  }

  //! Append `item` to the vector (unsafe case).
  //!
  //! Can only be used together with `willGrow()`. If `willGrow(N)` returns
  //! `kErrorOk` then N elements can be added to the vector without checking
  //! if there is a place for them. Used mostly internally.
  inline void appendUnsafe(const T& item) noexcept {
    ASMJIT_ASSERT(_length < _capacity);

    std::memcpy(static_cast<T*>(_data) + _length, &item, sizeof(T));
    _length++;
  }

  //! Concatenate all items of `other` at the end of the vector.
  inline void concatUnsafe(const ZoneVector<T>& other) noexcept {
    uint32_t count = other._length;
    ASMJIT_ASSERT(_capacity - _length >= count);

    if (count) {
      std::memcpy(static_cast<T*>(_data) + _length, other._data, size_t(count) * sizeof(T));
      _length += count;
    }
  }

  //! Get index of `val` or `Globals::kNotFound` if not found.
  inline uint32_t indexOf(const T& val) const noexcept {
    const T* data = static_cast<const T*>(_data);
    uint32_t length = _length;

    for (uint32_t i = 0; i < length; i++)
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
    ASMJIT_ASSERT(i < _length);

    T* data = static_cast<T*>(_data) + i;
    uint32_t count = --_length - i;

    if (count)
      std::memmove(data, data + 1, size_t(count) * sizeof(T));
  }

  inline T pop() noexcept {
    ASMJIT_ASSERT(_length > 0);

    uint32_t index = --_length;
    return getData()[index];
  }

  //! Swap this pod-vector with `other`.
  inline void swap(ZoneVector<T>& other) noexcept {
    std::swap(_length, other._length);
    std::swap(_capacity, other._capacity);
    std::swap(_data, other._data);
  }

  template<typename CMP = Algorithm::Compare<Algorithm::kOrderAscending>>
  inline void sort(const CMP& cmp = CMP()) noexcept {
    Algorithm::qSort<T, CMP>(getData(), getLength(), cmp);
  }

  //! Get item at index `i`.
  inline T& operator[](uint32_t i) noexcept {
    ASMJIT_ASSERT(i < _length);
    return getData()[i];
  }

  //! Get item at index `i`.
  inline const T& operator[](uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _length);
    return getData()[i];
  }

  inline T& getFirst() noexcept { return operator[](0); }
  inline const T& getFirst() const noexcept { return operator[](0); }

  inline T& getLast() noexcept { return operator[](_length - 1); }
  inline const T& getLast() const noexcept { return operator[](_length - 1); }

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
  //! If `n` is greater than the current length then the additional elements'
  //! content will be initialized to zero. If `n` is less than the current
  //! length then the vector will be truncated to exactly `n` elements.
  inline Error resize(ZoneAllocator* allocator, uint32_t n) noexcept {
    return ZoneVectorBase::_resize(allocator, sizeof(T), n);
  }

  //! Realloc internal array to fit at least `n` items.
  inline Error reserve(ZoneAllocator* allocator, uint32_t n) noexcept {
    return n > _capacity ? ZoneVectorBase::_reserve(allocator, sizeof(T), n) : Error(kErrorOk);
  }

  inline Error willGrow(ZoneAllocator* allocator, uint32_t n = 1) noexcept {
    return _capacity - _length < n ? grow(allocator, n) : Error(kErrorOk);
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

  typedef Globals::BitWord BitWord;
  static constexpr uint32_t kBitWordSize = Globals::kBitWordSize;

  static inline uint32_t _wordsPerBits(uint32_t nBits) noexcept {
    return ((nBits + kBitWordSize - 1) / kBitWordSize);
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

  explicit inline ZoneBitVector() noexcept
    : _data(nullptr),
      _length(0),
      _capacity(0) {}

  inline ZoneBitVector(ZoneBitVector&& other) noexcept
    : _data(other._data),
      _length(other._length),
      _capacity(other._capacity) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get whether the bit-vector is empty (has no bits).
  inline bool isEmpty() const noexcept { return _length == 0; }
  //! Get a length of this bit-vector (in bits).
  inline uint32_t getLength() const noexcept { return _length; }
  //! Get a capacity of this bit-vector (in bits).
  inline uint32_t getCapacity() const noexcept { return _capacity; }

  //! Get a count of `BitWord[]` array need to store all bits.
  inline uint32_t getLengthInBitWords() const noexcept { return _wordsPerBits(_length); }
  //! Get a count of `BitWord[]` array need to store all bits.
  inline uint32_t getCapacityInBitWords() const noexcept { return _wordsPerBits(_capacity); }

  //! Get data.
  inline BitWord* getData() noexcept { return _data; }
  //! \overload
  inline const BitWord* getData() const noexcept { return _data; }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  inline void clear() noexcept {
    _length = 0;
  }

  inline void reset() noexcept {
    _data = nullptr;
    _length = 0;
    _capacity = 0;
  }

  inline void truncate(uint32_t newLength) noexcept {
    _length = std::min(_length, newLength);
    _clearUnusedBits();
  }

  inline bool getBit(uint32_t index) const noexcept {
    ASMJIT_ASSERT(index < _length);
    return IntUtils::bitVectorGetBit(_data, index);
  }

  inline void setBit(uint32_t index, bool value) noexcept {
    ASMJIT_ASSERT(index < _length);
    IntUtils::bitVectorSetBit(_data, index, value);
  }

  inline void flipBit(uint32_t index) noexcept {
    ASMJIT_ASSERT(index < _length);
    IntUtils::bitVectorFlipBit(_data, index);
  }

  ASMJIT_FORCEINLINE Error append(ZoneAllocator* allocator, bool value) noexcept {
    uint32_t index = _length;
    if (ASMJIT_UNLIKELY(index >= _capacity))
      return _append(allocator, value);

    uint32_t idx = index / kBitWordSize;
    uint32_t bit = index % kBitWordSize;

    if (bit == 0)
      _data[idx] = BitWord(value) << bit;
    else
      _data[idx] |= BitWord(value) << bit;

    _length++;
    return kErrorOk;
  }

  ASMJIT_API Error copyFrom(ZoneAllocator* allocator, const ZoneBitVector& other) noexcept;

  inline void clearAll() noexcept {
    _zeroBits(_data, _wordsPerBits(_length));
  }

  inline void fillAll() noexcept {
    _fillBits(_data, _wordsPerBits(_length));
    _clearUnusedBits();
  }

  inline void clearBits(uint32_t start, uint32_t count) noexcept {
    ASMJIT_ASSERT(start <= _length);
    ASMJIT_ASSERT(_length - start >= count);

    IntUtils::bitVectorClear(_data, start, count);
  }

  inline void fillBits(uint32_t start, uint32_t count) noexcept {
    ASMJIT_ASSERT(start <= _length);
    ASMJIT_ASSERT(_length - start >= count);

    IntUtils::bitVectorFill(_data, start, count);
  }

  //! Perform a logical bitwise AND between bits specified in this array and bits
  //! in `other`. If `other` has less bits than `this` then all remaining bits are
  //! set to zero.
  //!
  //! NOTE: The length of the BitVector is unaffected by this operation.
  inline void and_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t thisBitWordCount = getLengthInBitWords();
    uint32_t otherBitWordCount = other.getLengthInBitWords();
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
  //! NOTE: The length of the BitVector is unaffected by this operation.
  inline void andNot(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t commonBitWordCount = _wordsPerBits(std::min(_length, other._length));
    for (uint32_t i = 0; i < commonBitWordCount; i++)
      dst[i] = dst[i] & ~src[i];
  }

  //! Perform a logical bitwise OP between bits specified in this array and bits
  //! in `other`. If `other` has less bits than `this` then all remaining bits
  //! are kept intact.
  //!
  //! NOTE: The length of the BitVector is unaffected by this operation.
  inline void or_(const ZoneBitVector& other) noexcept {
    BitWord* dst = _data;
    const BitWord* src = other._data;

    uint32_t commonBitWordCount = _wordsPerBits(std::min(_length, other._length));
    for (uint32_t i = 0; i < commonBitWordCount; i++)
      dst[i] = dst[i] | src[i];
    _clearUnusedBits();
  }

  inline void _clearUnusedBits() noexcept {
    uint32_t idx = _length / kBitWordSize;
    uint32_t bit = _length % kBitWordSize;

    if (!bit) return;
    _data[idx] &= (BitWord(1) << bit) - 1U;
  }

  inline bool eq(const ZoneBitVector& other) const noexcept {
    uint32_t len = _length;

    if (len != other._length)
      return false;

    const BitWord* aData = _data;
    const BitWord* bData = other._data;

    uint32_t numBitWords = _wordsPerBits(len);
    for (uint32_t i = 0; i < numBitWords; i++) {
      if (aData[i] != bData[i])
        return false;
    }
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

  inline Error resize(ZoneAllocator* allocator, uint32_t newLength, bool newBitsValue = false) noexcept {
    return _resize(allocator, newLength, newLength, newBitsValue);
  }

  ASMJIT_API Error _resize(ZoneAllocator* allocator, uint32_t newLength, uint32_t idealCapacity, bool newBitsValue) noexcept;
  ASMJIT_API Error _append(ZoneAllocator* allocator, bool value) noexcept;

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneBitVector& other) noexcept {
    std::swap(_data, other._data);
    std::swap(_length, other._length);
    std::swap(_capacity, other._capacity);
  }

  // --------------------------------------------------------------------------
  // [Iterators]
  // --------------------------------------------------------------------------

  class ForEachBitSet : public IntUtils::BitVectorIterator<BitWord> {
  public:
    explicit ASMJIT_FORCEINLINE ForEachBitSet(const ZoneBitVector& bitVector) noexcept
      : IntUtils::BitVectorIterator<BitWord>(bitVector.getData(), bitVector.getLengthInBitWords()) {}
  };

  template<class Operator>
  class ForEachBitOp : public IntUtils::BitVectorOpIterator<BitWord, Operator> {
  public:
    ASMJIT_FORCEINLINE ForEachBitOp(const ZoneBitVector& a, const ZoneBitVector& b) noexcept
      : IntUtils::BitVectorOpIterator<BitWord, Operator>(a.getData(), b.getData(), a.getLengthInBitWords()) {
      ASMJIT_ASSERT(a.getLength() == b.getLength());
    }
  };

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BitWord* _data;                        //!< Bits.
  uint32_t _length;                      //!< Length of the bit-vector (in bits).
  uint32_t _capacity;                    //!< Capacity of the bit-vector (in bits).
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONEVECTOR_H
