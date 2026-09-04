// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_AXL_SUPPORT_P_H_INCLUDED
#define ASMJIT_AXL_SUPPORT_P_H_INCLUDED

#include <asmjit/axl/arena_bit_set_p.h>
#include <asmjit/axl/bit_set_utils.h>
#include <asmjit/axl/commons.h>

ASMJIT_BEGIN_SUB_NAMESPACE(axl)

//! \cond INTERNAL
//! \addtogroup asmjit_axl
//! \{

class BitWordMutator {
public:
  BitWord _bit_word;

  ASMJIT_INLINE explicit BitWordMutator(Span<BitWord> span) noexcept {
    ASMJIT_ASSERT(span.size() == 1u);
    _bit_word = span[0];
  }

  [[nodiscard]]
  ASMJIT_INLINE BitWord bit_word([[maybe_unused]] size_t index) const noexcept {
    ASMJIT_ASSERT(index == 0u);
    return _bit_word;
  }

  ASMJIT_INLINE void set_bit_word([[maybe_unused]] size_t index, BitWord bw) noexcept {
    ASMJIT_ASSERT(index == 0u);
    _bit_word = bw;
  }

  template<typename Index>
  [[nodiscard]]
  ASMJIT_INLINE bool bit_at(const Index& index) const noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);
    return (_bit_word & (BitWord(1) << size_t(index))) != 0u;
  }

  template<typename Index>
  ASMJIT_INLINE void set_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord clear_mask = BitWord(1u) << size_t(index);
    BitWord bit_mask = BitWord(value) << size_t(index);

    _bit_word = (_bit_word & ~clear_mask) | bit_mask;
  }

  template<typename Index>
  ASMJIT_INLINE void add_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord bit_mask = BitWord(value) << size_t(index);
    _bit_word |= bit_mask;
  }

  template<typename Index>
  ASMJIT_INLINE void clear_bit(const Index& index) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord bit_mask = BitWord(1) << size_t(index);
    _bit_word &= ~bit_mask;
  }

  template<typename Index>
  ASMJIT_INLINE void xor_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < bit_size_of<BitWord>);

    BitWord bit_mask = BitWord(value) << size_t(index);
    _bit_word ^= bit_mask;
  }

  ASMJIT_INLINE void clear_bits(const BitWordMutator& other) noexcept {
    _bit_word &= ~other._bit_word;
  }

  ASMJIT_INLINE void commit(Span<BitWord> span) const noexcept {
    span[0] = _bit_word;
  }
};

class BitVectorMutator {
public:
  template<typename T>
  static ASMJIT_INLINE T bit_word_count_of(const T& n) noexcept {
    return T((n + bit_size_of<BitWord> - 1u) / bit_size_of<BitWord>);
  }

  BitWord* _data;
  size_t _count;

  ASMJIT_INLINE BitVectorMutator(BitWord* data, size_t count) noexcept
    : _data(data),
      _count(count) {}

  ASMJIT_INLINE BitVectorMutator(Span<BitWord> span) noexcept
    : _data(span.data_mut()),
      _count(span.size()) {}

  [[nodiscard]]
  ASMJIT_INLINE BitWord bit_word(size_t index) const noexcept {
    ASMJIT_ASSERT(index < _count);
    return _data[index];
  }

  ASMJIT_INLINE void set_bit_word(size_t index, BitWord bw) noexcept {
    ASMJIT_ASSERT(index < _count);
    _data[index] = bw;
  }

  template<typename Index>
  [[nodiscard]]
  ASMJIT_INLINE bool bit_at(const Index& index) const noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    return bit_vector_get_bit(_data, size_t(index));
  }

  template<typename Index>
  ASMJIT_INLINE void set_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bit_vector_set_bit(_data, size_t(index), value);
  }

  template<typename Index>
  ASMJIT_INLINE void add_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bit_vector_or_bit(_data, size_t(index), value);
  }

  template<typename Index>
  ASMJIT_INLINE void clear_bit(const Index& index) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bit_vector_set_bit(_data, size_t(index), false);
  }

  template<typename Index>
  ASMJIT_INLINE void xor_bit(const Index& index, bool value) noexcept {
    ASMJIT_ASSERT(size_t(index) < _count * bit_size_of<BitWord>);
    bit_vector_xor_bit(_data, size_t(index), value);
  }

  ASMJIT_INLINE void clear_bits(const BitVectorMutator& other) noexcept {
    ASMJIT_ASSERT(_count == other._count);

    size_t n = _count;
    const BitWord* other_data = other._data;

    for (size_t i = 0u; i < n; i++) {
      _data[i] &= ~other_data[i];
    }
  }

  ASMJIT_INLINE void commit(Span<BitWord> span) const noexcept {
    // Does nothing - each operation is written to memory.
    axl::maybe_unused(span);
  }
};

template<typename T, uint32_t B>
class BitVectorRangeIterator {
public:
  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _bit_word;

  static inline constexpr uint32_t kBitWordSize = axl::bit_size_of<T>;
  static inline constexpr T kXorMask = B == 0 ? axl::bit_ones<T> : T(0);

  ASMJIT_INLINE BitVectorRangeIterator(const T* data, size_t bit_word_count) noexcept {
    init(data, bit_word_count);
  }

  ASMJIT_INLINE BitVectorRangeIterator(const T* data, size_t bit_word_count, size_t start, size_t end) noexcept {
    init(data, bit_word_count, start, end);
  }

  ASMJIT_INLINE void init(const T* data, size_t bit_word_count) noexcept {
    init(data, bit_word_count, 0, bit_word_count * kBitWordSize);
  }

  ASMJIT_INLINE void init(const T* data, size_t bit_word_count, size_t start, size_t end) noexcept {
    ASMJIT_ASSERT(bit_word_count >= (end + kBitWordSize - 1) / kBitWordSize);
    axl::maybe_unused(bit_word_count);

    size_t idx = axl::align_down(start, kBitWordSize);
    const T* ptr = data + (idx / kBitWordSize);

    T bit_word = 0;
    if (idx < end) {
      bit_word = (*ptr ^ kXorMask) & (axl::bit_ones<T> << (start % kBitWordSize));
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _bit_word = bit_word;
  }

  ASMJIT_INLINE bool next_range(Out<size_t> range_start, Out<size_t> range_end, size_t range_hint = max_value<size_t>()) noexcept {
    // Skip all empty BitWords.
    while (_bit_word == 0) {
      _idx += kBitWordSize;
      if (_idx >= _end) {
        return false;
      }
      _bit_word = (*++_ptr) ^ kXorMask;
    }

    size_t i = axl::ctz(_bit_word);

    *range_start = _idx + i;
    _bit_word = ~(_bit_word ^ ~(axl::bit_ones<T> << i));

    if (_bit_word == 0) {
      *range_end = axl::min(_idx + kBitWordSize, _end);
      while (*range_end - *range_start < range_hint) {
        _idx += kBitWordSize;
        if (_idx >= _end) {
          break;
        }

        _bit_word = (*++_ptr) ^ kXorMask;
        if (_bit_word != axl::bit_ones<T>) {
          size_t j = axl::ctz(~_bit_word);
          *range_end = axl::min(_idx + j, _end);
          _bit_word = _bit_word ^ ~(axl::bit_ones<T> << j);
          break;
        }

        *range_end = axl::min(_idx + kBitWordSize, _end);
        _bit_word = 0;
        continue;
      }

      return true;
    }
    else {
      size_t j = axl::ctz(_bit_word);
      *range_end = axl::min(_idx + j, _end);

      _bit_word = ~(_bit_word ^ ~(axl::bit_ones<T> << j));
      return true;
    }
  }
};

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_AXL_SUPPORT_P_H_INCLUDED
