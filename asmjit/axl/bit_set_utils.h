// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_AXL_BIT_SET_UTILS_H_INCLUDED
#define ASMJIT_AXL_BIT_SET_UTILS_H_INCLUDED

#include <asmjit/axl/commons.h>
#include <asmjit/axl/span.h>

ASMJIT_BEGIN_SUB_NAMESPACE(axl)

//! \addtogroup asmjit_axl
//! \{

// BitWordIterator
// ===============

//! Iterates over each bit in a number which is set to 1.
//!
//! Example of use:
//!
//! ```
//! uint32_t bits_to_iterate = 0x110F;
//! axl::BitWordIterator<uint32_t> it(bits_to_iterate);
//!
//! while (it.has_next()) {
//!   uint32_t bit_index = it.next();
//!   std::printf("Bit at %u is set\n", unsigned(bit_index));
//! }
//! ```
template<typename T>
class BitWordIterator {
public:
  ASMJIT_INLINE_NODEBUG explicit BitWordIterator(T bit_word) noexcept
    : _bit_word(bit_word) {}

  ASMJIT_INLINE_NODEBUG void init(T bit_word) noexcept { _bit_word = bit_word; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_next() const noexcept { return _bit_word != 0; }

  [[nodiscard]]
  ASMJIT_INLINE uint32_t next() noexcept {
    ASMJIT_ASSERT(_bit_word != 0);
    uint32_t index = ctz(_bit_word);
    _bit_word &= T(_bit_word - 1);
    return index;
  }

  T _bit_word;
};

// BitVectorOps
// ============

//! \cond
namespace Internal {
  template<typename T, class OperatorT, class FullWordOpT>
  static ASMJIT_INLINE void bit_vector_op(T* buf, size_t index, size_t count) noexcept {
    if (count == 0) {
      return;
    }

    size_t word_index = index / bit_size_of<T>; // T[]
    size_t bit_index = index % bit_size_of<T>; // T[][]

    buf += word_index;

    // The first BitWord requires special handling to preserve bits outside the fill region.
    constexpr T fill_mask = bit_ones<T>;
    size_t first_n_bits = min<size_t>(bit_size_of<T> - bit_index, count);

    buf[0] = OperatorT::op(buf[0], (fill_mask >> (bit_size_of<T> - first_n_bits)) << bit_index);
    buf++;
    count -= first_n_bits;

    // All bits between the first and last affected BitWords can be just filled.
    while (count >= bit_size_of<T>) {
      buf[0] = FullWordOpT::op(buf[0], fill_mask);
      buf++;
      count -= bit_size_of<T>;
    }

    // The last BitWord requires special handling as well
    if (count) {
      buf[0] = OperatorT::op(buf[0], fill_mask >> (bit_size_of<T> - count));
    }
  }
}
//! \endcond

//! Sets bit in a bit-vector `buf` at `index`.
template<typename T>
static ASMJIT_INLINE_NODEBUG bool bit_vector_get_bit(T* buf, size_t index) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;

  return bool((buf[word_index] >> bit_index) & 0x1u);
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bit_vector_set_bit(T* buf, size_t index, bool value) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;

  T clear_mask = T(1u) << bit_index;
  T set_mask = T(value) << bit_index;

  buf[word_index] = T((buf[word_index] & ~clear_mask) | set_mask);
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bit_vector_or_bit(T* buf, size_t index, bool value) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;

  T bit_mask = T(value) << bit_index;
  buf[word_index] |= bit_mask;
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bit_vector_xor_bit(T* buf, size_t index, bool value) noexcept {
  size_t word_index = index / bit_size_of<T>;
  size_t bit_index = index % bit_size_of<T>;

  T bit_mask = T(value) << bit_index;
  buf[word_index] ^= bit_mask;
}

//! Fills `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bit_vector_fill(T* buf, size_t index, size_t count) noexcept { Internal::bit_vector_op<T, Or, Set>(buf, index, count); }

//! Clears `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bit_vector_clear(T* buf, size_t index, size_t count) noexcept { Internal::bit_vector_op<T, AndNot, SetNot>(buf, index, count); }

template<typename T>
static ASMJIT_INLINE size_t bit_vector_index_of(T* buf, size_t start, bool value) noexcept {
  size_t word_index = start / bit_size_of<T>; // T[]
  size_t bit_index = start % bit_size_of<T>; // T[][]

  T* p = buf + word_index;

  // We always look for zeros, if value is `true` we have to flip all bits before the search.
  const T fill_mask = bit_ones<T>;
  const T flip_mask = value ? T(0) : fill_mask;

  // The first BitWord requires special handling as there are some bits we want to ignore.
  T bits = (*p ^ flip_mask) & (fill_mask << bit_index);
  for (;;) {
    if (bits) {
      return (size_t)(p - buf) * bit_size_of<T> + ctz(bits);
    }
    bits = *++p ^ flip_mask;
  }
}

// BitVectorIterator
// =================

template<typename T>
class BitVectorIterator {
public:
  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;

  ASMJIT_INLINE_NODEBUG BitVectorIterator(const BitVectorIterator& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG BitVectorIterator(Span<const T> data, size_t start = 0) noexcept {
    init(data, start);
  }

  ASMJIT_INLINE void init(Span<const T> data, size_t start = 0) noexcept {
    const T* ptr = data.data() + (start / bit_size_of<T>);
    size_t idx = align_down(start, bit_size_of<T>);
    size_t end = data.size() * bit_size_of<T>;

    T bit_word = T(0);
    if (idx < end) {
      bit_word = *ptr++ & (bit_ones<T> << (start % bit_size_of<T>));
      while (!bit_word && (idx += bit_size_of<T>) < end) {
        bit_word = *ptr++;
      }
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bit_word;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_next() const noexcept {
    return _current != T(0);
  }

  [[nodiscard]]
  ASMJIT_INLINE size_t next() noexcept {
    T bit_word = _current;
    ASMJIT_ASSERT(bit_word != T(0));

    uint32_t bit = ctz(bit_word);
    bit_word &= T(bit_word - 1u);

    size_t n = _idx + bit;
    while (!bit_word && (_idx += bit_size_of<T>) < _end) {
      bit_word = *_ptr++;
    }

    _current = bit_word;
    return n;
  }

  [[nodiscard]]
  ASMJIT_INLINE size_t peek_next() const noexcept {
    ASMJIT_ASSERT(_current != T(0));
    return _idx + ctz(_current);
  }
};

// BitVectorOpIterator
// ===================

template<typename T, class OperatorT>
class BitVectorOpIterator {
public:
  const T* _a_ptr;
  const T* _b_ptr;
  size_t _idx;
  size_t _end;
  T _current;

  ASMJIT_INLINE_NODEBUG BitVectorOpIterator(const T* a_data, const T* b_data, size_t bit_word_count, size_t start = 0) noexcept {
    init(a_data, b_data, bit_word_count, start);
  }

  ASMJIT_INLINE_NODEBUG BitVectorOpIterator(Span<const T> a, Span<const T> b, size_t start = 0) noexcept {
    ASMJIT_ASSERT(a.size() == b.size());
    init(a.data(), b.data(), a.size(), start);
  }

  ASMJIT_INLINE void init(const T* a_data, const T* b_data, size_t bit_word_count, size_t start = 0) noexcept {
    const T* a_ptr = a_data + (start / bit_size_of<T>);
    const T* b_ptr = b_data + (start / bit_size_of<T>);
    size_t idx = align_down(start, bit_size_of<T>);
    size_t end = bit_word_count * bit_size_of<T>;

    T bit_word = T(0);
    if (idx < end) {
      bit_word = OperatorT::op(*a_ptr++, *b_ptr++) & (bit_ones<T> << (start % bit_size_of<T>));
      while (!bit_word && (idx += bit_size_of<T>) < end) {
        bit_word = OperatorT::op(*a_ptr++, *b_ptr++);
      }
    }

    _a_ptr = a_ptr;
    _b_ptr = b_ptr;
    _idx = idx;
    _end = end;
    _current = bit_word;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool has_next() noexcept {
    return _current != T(0);
  }

  [[nodiscard]]
  ASMJIT_INLINE size_t next() noexcept {
    T bit_word = _current;
    ASMJIT_ASSERT(bit_word != T(0));

    uint32_t bit = ctz(bit_word);
    bit_word &= T(bit_word - 1u);

    size_t n = _idx + bit;
    while (!bit_word && (_idx += bit_size_of<T>) < _end) {
      bit_word = OperatorT::op(*_a_ptr++, *_b_ptr++);
    }

    _current = bit_word;
    return n;
  }
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // ASMJIT_AXL_BIT_SET_UTILS_H_INCLUDED
