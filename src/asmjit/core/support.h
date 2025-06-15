// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_SUPPORT_H_INCLUDED
#define ASMJIT_CORE_SUPPORT_H_INCLUDED

#include "../core/globals.h"

#if defined(_MSC_VER)
  #include <intrin.h>
#endif

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_utilities
//! \{

//! Contains support classes and functions that may be used by AsmJit source and header files. Anything defined
//! here is considered internal and should not be used outside of AsmJit and related projects like AsmTK.
namespace Support {

// Support - Basic Traits
// ======================

#if ASMJIT_ARCH_X86
using FastUInt8 = uint8_t;
#else
using FastUInt8 = uint32_t;
#endif

//! \cond INTERNAL
namespace Internal {
  template<typename T, size_t Alignment>
  struct AliasedUInt {};

  template<> struct AliasedUInt<uint8_t, 1> { typedef uint8_t Type; };
  template<> struct AliasedUInt<uint16_t, 2> { typedef uint16_t ASMJIT_MAY_ALIAS Type; };
  template<> struct AliasedUInt<uint32_t, 4> { typedef uint32_t ASMJIT_MAY_ALIAS Type; };
  template<> struct AliasedUInt<uint64_t, 8> { typedef uint64_t ASMJIT_MAY_ALIAS Type; };

  template<> struct AliasedUInt<uint16_t, 1> { typedef uint16_t ASMJIT_MAY_ALIAS ASMJIT_ALIGN_TYPE(1, Type); };
  template<> struct AliasedUInt<uint32_t, 1> { typedef uint32_t ASMJIT_MAY_ALIAS ASMJIT_ALIGN_TYPE(1, Type); };
  template<> struct AliasedUInt<uint32_t, 2> { typedef uint32_t ASMJIT_MAY_ALIAS ASMJIT_ALIGN_TYPE(2, Type); };
  template<> struct AliasedUInt<uint64_t, 1> { typedef uint64_t ASMJIT_MAY_ALIAS ASMJIT_ALIGN_TYPE(1, Type); };
  template<> struct AliasedUInt<uint64_t, 2> { typedef uint64_t ASMJIT_MAY_ALIAS ASMJIT_ALIGN_TYPE(2, Type); };
  template<> struct AliasedUInt<uint64_t, 4> { typedef uint64_t ASMJIT_MAY_ALIAS ASMJIT_ALIGN_TYPE(4, Type); };

  // StdInt    - Make an int-type by size (signed or unsigned) that is the
  //             same as types defined by <stdint.h>.
  // Int32Or64 - Make an int-type that has at least 32 bits: [u]int[32|64]_t.

  template<size_t Size, unsigned Unsigned>
  struct StdInt {}; // Fail if not specialized.

  template<> struct StdInt<1, 0> { using Type = int8_t;   };
  template<> struct StdInt<1, 1> { using Type = uint8_t;  };
  template<> struct StdInt<2, 0> { using Type = int16_t;  };
  template<> struct StdInt<2, 1> { using Type = uint16_t; };
  template<> struct StdInt<4, 0> { using Type = int32_t;  };
  template<> struct StdInt<4, 1> { using Type = uint32_t; };
  template<> struct StdInt<8, 0> { using Type = int64_t;  };
  template<> struct StdInt<8, 1> { using Type = uint64_t; };

  template<typename T, int Unsigned = std::is_unsigned_v<T>>
  struct Int32Or64 : public StdInt<sizeof(T) <= 4 ? size_t(4) : sizeof(T), Unsigned> {};
}
//! \endcond

//! Casts an integer `x` to either `int32_t` or `int64_t` depending on `T`.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR typename Internal::Int32Or64<T, 0>::Type asInt(const T& x) noexcept {
  return (typename Internal::Int32Or64<T, 0>::Type)x;
}

//! Casts an integer `x` to either `uint32_t` or `uint64_t` depending on `T`.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR typename Internal::Int32Or64<T, 1>::Type asUInt(const T& x) noexcept {
  return (typename Internal::Int32Or64<T, 1>::Type)x;
}

//! Casts an integer `x` to either `int32_t`, uint32_t`, `int64_t`, or `uint64_t` depending on `T`.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR typename Internal::Int32Or64<T>::Type asNormalized(const T& x) noexcept {
  return (typename Internal::Int32Or64<T>::Type)x;
}

//! Casts an integer `x` to the same type as defined by `<stdint.h>`.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR typename Internal::StdInt<sizeof(T), std::is_unsigned_v<T>>::Type asStdInt(const T& x) noexcept {
  return (typename Internal::StdInt<sizeof(T), std::is_unsigned_v<T>>::Type)x;
}

//! A helper class that can be used to iterate over enum values.
template<typename T, T from = (T)0, T to = T::kMaxValue>
struct EnumValues {
  using ValueType = std::underlying_type_t<T>;

  struct Iterator {
    ValueType value;

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG T operator*() const { return (T)value; }
    ASMJIT_INLINE_NODEBUG void operator++() { ++value; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool operator==(const Iterator& other) const noexcept { return value == other.value; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool operator!=(const Iterator& other) const noexcept { return value != other.value; }
  };

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Iterator begin() const noexcept { return Iterator{ValueType(from)}; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG Iterator end() const noexcept { return Iterator{ValueType(to) + 1}; }
};

// Support - Pointer Operations
// ============================

template<typename Dst, typename Src, typename Offset>
static ASMJIT_INLINE_NODEBUG Dst* offsetPtr(Src* ptr, const Offset& n) noexcept {
  return static_cast<Dst*>(
    static_cast<void*>(static_cast<char*>(static_cast<void*>(ptr)) + n)
  );
}

template<typename Dst, typename Src, typename Offset>
static ASMJIT_INLINE_NODEBUG const Dst* offsetPtr(const Src* ptr, const Offset& n) noexcept {
  return static_cast<const Dst*>(
    static_cast<const void*>(static_cast<const char*>(static_cast<const void*>(ptr)) + n)
  );
}

// Support - Boolean Operations
// ============================

namespace Internal {
  static ASMJIT_INLINE_CONSTEXPR unsigned unsigned_from_bool(bool b) noexcept {
    return unsigned(b);
  }
}

template<typename... Args>
static ASMJIT_INLINE_CONSTEXPR bool bool_and(Args&&... args) noexcept {
  return bool( (... & Internal::unsigned_from_bool(args)) );
}

template<typename... Args>
static ASMJIT_INLINE_CONSTEXPR bool bool_or(Args&&... args) noexcept {
  return bool( (... | Internal::unsigned_from_bool(args)) );
}

// Support - BitCast
// =================

//! \cond
namespace Internal {
  template<typename DstT, typename SrcT>
  union BitCastUnion {
    ASMJIT_INLINE_NODEBUG BitCastUnion(SrcT src) noexcept : src(src) {}
    SrcT src;
    DstT dst;
  };
}
//! \endcond

//! Bit-casts from `Src` type to `Dst` type.
//!
//! Useful to bit-cast between integers and floating points.
template<typename Dst, typename Src>
static ASMJIT_INLINE_NODEBUG Dst bitCast(const Src& x) noexcept {
  static_assert(sizeof(Dst) == sizeof(Src), "bitCast can only be used to cast types of same size");

  if constexpr (std::is_integral_v<Dst> && std::is_integral_v<Src>) {
    return Dst(x);
  }
  else {
    return Internal::BitCastUnion<Dst, Src>(x).dst;
  }
}

// Support - BitOps
// ================

//! Storage used to store a pack of bits (should by compatible with a machine word).
using BitWord = Internal::StdInt<sizeof(uintptr_t), 1>::Type;

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t bitSizeOf() noexcept { return uint32_t(sizeof(T) * 8u); }

//! Number of bits stored in a single `BitWord`.
static constexpr uint32_t kBitWordSizeInBits = bitSizeOf<BitWord>();

//! Returns `0 - x` in a safe way (no undefined behavior), works for unsigned numbers as well.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T neg(const T& x) noexcept {
  using U = std::make_unsigned_t<T>;
  return T(U(0) - U(x));
}

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T allOnes() noexcept { return neg<T>(T(1)); }

//! Returns `x << y` (shift left logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X shl(const X& x, const Y& y) noexcept {
  using U = std::make_unsigned_t<X>;
  return X(U(x) << y);
}

//! Returns `x >> y` (shift right logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X shr(const X& x, const Y& y) noexcept {
  using U = std::make_unsigned_t<X>;
  return X(U(x) >> y);
}

//! Returns `x >> y` (shift right arithmetic) by explicitly casting `x` to a signed type and back.
template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X sar(const X& x, const Y& y) noexcept {
  using S = std::make_signed_t<X>;
  return X(S(x) >> y);
}

template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X ror(const X& x, const Y& y) noexcept {
  using U = std::make_unsigned_t<X>;
  return X((U(x) >> y) | (U(x) << (bitSizeOf<U>() - U(y))));
}

//! Returns `x | (x >> y)` - helper used by some bit manipulation helpers.
template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X or_shr(const X& x, const Y& y) noexcept { return X(x | shr(x, y)); }

//! Returns `x & -x` - extracts lowest set isolated bit (like BLSI instruction).
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T blsi(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  return T(U(x) & neg(U(x)));
}

//! Tests whether the given value `x` has `n`th bit set.
template<typename T, typename IndexT>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool bitTest(T x, IndexT n) noexcept {
  using U = std::make_unsigned_t<T>;
  return (U(x) & (U(1) << asStdInt(n))) != 0;
}

// Tests whether the given `value` is a consecutive mask of bits that starts at
// the least significant bit.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isLsbMask(const T& value) noexcept {
  using U = std::make_unsigned_t<T>;
  return value && ((U(value) + 1u) & U(value)) == 0;
}

// Tests whether the given value contains at least one bit or whether it's a
// bit-mask of consecutive bits.
//
// This function is similar to \ref isLsbMask(), but the mask doesn't have to
// start at a least significant bit.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isConsecutiveMask(const T& value) noexcept {
  using U = std::make_unsigned_t<T>;
  return value && isLsbMask((U(value) - 1u) | U(value));
}

//! Generates a trailing bit-mask that has `n` least significant (trailing) bits set.
template<typename T, typename CountT>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T lsbMask(const CountT& n) noexcept {
  using U = std::make_unsigned_t<T>;
  return (sizeof(U) < sizeof(uintptr_t))
    // Prevent undefined behavior by using a larger type than T.
    ? T(U((uintptr_t(1) << n) - uintptr_t(1)))
    // Prevent undefined behavior by checking `n` before shift.
    : n ? T(shr(allOnes<T>(), bitSizeOf<T>() - size_t(n))) : T(0);
}

//! Generates a leading bit-mask that has `n` most significant (leading) bits set.
template<typename T, typename CountT>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T msbMask(const CountT& n) noexcept {
  using U = std::make_unsigned_t<T>;
  return (sizeof(U) < sizeof(uintptr_t))
    // Prevent undefined behavior by using a larger type than T.
    ? T(allOnes<uintptr_t>() >> (bitSizeOf<uintptr_t>() - n))
    // Prevent undefined behavior by performing `n & (nBits - 1)` so it's always within the range.
    : T(sar(U(n != 0) << (bitSizeOf<U>() - 1), n ? uint32_t(n - 1) : uint32_t(0)));
}

//! Returns a bit-mask that has `x` bit set.
template<typename Index>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t bitMask(const Index& x) noexcept { return (1u << asUInt(x)); }

//! Returns a bit-mask that has `x` bit set (multiple arguments).
template<typename Index, typename... Args>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t bitMask(const Index& x, Args... args) noexcept { return bitMask(x) | bitMask(args...); }

//! Converts a boolean value `b` to zero or full mask (all bits set).
template<typename DstT, typename SrcT>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR DstT bitMaskFromBool(SrcT b) noexcept {
  using U = std::make_unsigned_t<DstT>;
  return DstT(U(0) - U(b));
}

//! Tests whether `a & b` is non-zero.
template<typename A, typename B>
[[nodiscard]]
static inline constexpr bool test(A a, B b) noexcept { return (asUInt(a) & asUInt(b)) != 0; }

//! \cond
namespace Internal {
  // Fills all trailing bits right from the first most significant bit set.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint8_t fillTrailingBitsImpl(uint8_t x) noexcept { return or_shr(or_shr(or_shr(x, 1), 2), 4); }

  // Fills all trailing bits right from the first most significant bit set.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint16_t fillTrailingBitsImpl(uint16_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8); }

  // Fills all trailing bits right from the first most significant bit set.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint32_t fillTrailingBitsImpl(uint32_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8), 16); }

  // Fills all trailing bits right from the first most significant bit set.
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR uint64_t fillTrailingBitsImpl(uint64_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8), 16), 32); }
}
//! \endcond

// Fills all trailing bits right from the first most significant bit set.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T fillTrailingBits(const T& x) noexcept {
  using U = std::make_unsigned_t<T>;
  return T(Internal::fillTrailingBitsImpl(U(x)));
}

// Support - Count Leading/Trailing Zeros
// ======================================

//! \cond
namespace Internal {
namespace {

template<typename T>
struct BitScanData { T x; uint32_t n; };

template<typename T, uint32_t N>
struct BitScanCalc {
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR BitScanData<T> advanceLeft(const BitScanData<T>& data, uint32_t n) noexcept {
    return BitScanData<T> { data.x << n, data.n + n };
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR BitScanData<T> advanceRight(const BitScanData<T>& data, uint32_t n) noexcept {
    return BitScanData<T> { data.x >> n, data.n + n };
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR BitScanData<T> clz(const BitScanData<T>& data) noexcept {
    return BitScanCalc<T, N / 2>::clz(advanceLeft(data, data.x & (allOnes<T>() << (bitSizeOf<T>() - N)) ? uint32_t(0) : N));
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR BitScanData<T> ctz(const BitScanData<T>& data) noexcept {
    return BitScanCalc<T, N / 2>::ctz(advanceRight(data, data.x & (allOnes<T>() >> (bitSizeOf<T>() - N)) ? uint32_t(0) : N));
  }
};

template<typename T>
struct BitScanCalc<T, 0> {
  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR BitScanData<T> clz(const BitScanData<T>& ctx) noexcept {
    return BitScanData<T> { 0, ctx.n - uint32_t(ctx.x >> (bitSizeOf<T>() - 1)) };
  }

  [[nodiscard]]
  static ASMJIT_INLINE_CONSTEXPR BitScanData<T> ctz(const BitScanData<T>& ctx) noexcept {
    return BitScanData<T> { 0, ctx.n - uint32_t(ctx.x & 0x1) };
  }
};

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_CONSTEXPR uint32_t clzFallback(const T& x) noexcept {
  return BitScanCalc<T, bitSizeOf<T>() / 2u>::clz(BitScanData<T>{x, 1}).n;
}

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_CONSTEXPR uint32_t ctzFallback(const T& x) noexcept {
  return BitScanCalc<T, bitSizeOf<T>() / 2u>::ctz(BitScanData<T>{x, 1}).n;
}

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t clzImpl(const T& x) noexcept { return clzFallback(asUInt(x)); }

template<typename T>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t ctzImpl(const T& x) noexcept { return ctzFallback(asUInt(x)); }

#if !defined(ASMJIT_NO_INTRINSICS)
# if defined(__GNUC__)
template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t clzImpl(const uint32_t& x) noexcept { return uint32_t(__builtin_clz(x)); }

template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t clzImpl(const uint64_t& x) noexcept { return uint32_t(__builtin_clzll(x)); }

template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t ctzImpl(const uint32_t& x) noexcept { return uint32_t(__builtin_ctz(x)); }

template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t ctzImpl(const uint64_t& x) noexcept { return uint32_t(__builtin_ctzll(x)); }

# elif defined(_MSC_VER)
template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t clzImpl(const uint32_t& x) noexcept { unsigned long i; _BitScanReverse(&i, x); return uint32_t(i ^ 31); }

template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t ctzImpl(const uint32_t& x) noexcept { unsigned long i; _BitScanForward(&i, x); return uint32_t(i); }

#  if ASMJIT_ARCH_X86 == 64 || ASMJIT_ARCH_ARM == 64
template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t clzImpl(const uint64_t& x) noexcept { unsigned long i; _BitScanReverse64(&i, x); return uint32_t(i ^ 63); }

template<>
[[nodiscard]]
ASMJIT_INLINE_NODEBUG uint32_t ctzImpl(const uint64_t& x) noexcept { unsigned long i; _BitScanForward64(&i, x); return uint32_t(i); }
#  endif
# endif
#endif

} // {anonymous}
} // {Internal}
//! \endcond

//! Count leading zeros in `x` (returns a position of a first bit set in `x`).
//!
//! \note The input MUST NOT be zero, otherwise the result is undefined.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t clz(T x) noexcept { return Internal::clzImpl(asUInt(x)); }

//! Count trailing zeros in `x` (returns a position of a first bit set in `x`).
//!
//! \note The input MUST NOT be zero, otherwise the result is undefined.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t ctz(T x) noexcept { return Internal::ctzImpl(asUInt(x)); }

template<uint64_t kInput>
struct ConstCTZ {
  static inline constexpr uint32_t value =
    (kInput & (uint64_t(1) <<  0)) ?  0 :
    (kInput & (uint64_t(1) <<  1)) ?  1 :
    (kInput & (uint64_t(1) <<  2)) ?  2 :
    (kInput & (uint64_t(1) <<  3)) ?  3 :
    (kInput & (uint64_t(1) <<  4)) ?  4 :
    (kInput & (uint64_t(1) <<  5)) ?  5 :
    (kInput & (uint64_t(1) <<  6)) ?  6 :
    (kInput & (uint64_t(1) <<  7)) ?  7 :
    (kInput & (uint64_t(1) <<  8)) ?  8 :
    (kInput & (uint64_t(1) <<  9)) ?  9 :
    (kInput & (uint64_t(1) << 10)) ? 10 :
    (kInput & (uint64_t(1) << 11)) ? 11 :
    (kInput & (uint64_t(1) << 12)) ? 12 :
    (kInput & (uint64_t(1) << 13)) ? 13 :
    (kInput & (uint64_t(1) << 14)) ? 14 :
    (kInput & (uint64_t(1) << 15)) ? 15 :
    (kInput & (uint64_t(1) << 16)) ? 16 :
    (kInput & (uint64_t(1) << 17)) ? 17 :
    (kInput & (uint64_t(1) << 18)) ? 18 :
    (kInput & (uint64_t(1) << 19)) ? 19 :
    (kInput & (uint64_t(1) << 20)) ? 20 :
    (kInput & (uint64_t(1) << 21)) ? 21 :
    (kInput & (uint64_t(1) << 22)) ? 22 :
    (kInput & (uint64_t(1) << 23)) ? 23 :
    (kInput & (uint64_t(1) << 24)) ? 24 :
    (kInput & (uint64_t(1) << 25)) ? 25 :
    (kInput & (uint64_t(1) << 26)) ? 26 :
    (kInput & (uint64_t(1) << 27)) ? 27 :
    (kInput & (uint64_t(1) << 28)) ? 28 :
    (kInput & (uint64_t(1) << 29)) ? 29 :
    (kInput & (uint64_t(1) << 30)) ? 30 :
    (kInput & (uint64_t(1) << 31)) ? 31 :
    (kInput & (uint64_t(1) << 32)) ? 32 :
    (kInput & (uint64_t(1) << 33)) ? 33 :
    (kInput & (uint64_t(1) << 34)) ? 34 :
    (kInput & (uint64_t(1) << 35)) ? 35 :
    (kInput & (uint64_t(1) << 36)) ? 36 :
    (kInput & (uint64_t(1) << 37)) ? 37 :
    (kInput & (uint64_t(1) << 38)) ? 38 :
    (kInput & (uint64_t(1) << 39)) ? 39 :
    (kInput & (uint64_t(1) << 40)) ? 40 :
    (kInput & (uint64_t(1) << 41)) ? 41 :
    (kInput & (uint64_t(1) << 42)) ? 42 :
    (kInput & (uint64_t(1) << 43)) ? 43 :
    (kInput & (uint64_t(1) << 44)) ? 44 :
    (kInput & (uint64_t(1) << 45)) ? 45 :
    (kInput & (uint64_t(1) << 46)) ? 46 :
    (kInput & (uint64_t(1) << 47)) ? 47 :
    (kInput & (uint64_t(1) << 48)) ? 48 :
    (kInput & (uint64_t(1) << 49)) ? 49 :
    (kInput & (uint64_t(1) << 50)) ? 50 :
    (kInput & (uint64_t(1) << 51)) ? 51 :
    (kInput & (uint64_t(1) << 52)) ? 52 :
    (kInput & (uint64_t(1) << 53)) ? 53 :
    (kInput & (uint64_t(1) << 54)) ? 54 :
    (kInput & (uint64_t(1) << 55)) ? 55 :
    (kInput & (uint64_t(1) << 56)) ? 56 :
    (kInput & (uint64_t(1) << 57)) ? 57 :
    (kInput & (uint64_t(1) << 58)) ? 58 :
    (kInput & (uint64_t(1) << 59)) ? 59 :
    (kInput & (uint64_t(1) << 60)) ? 60 :
    (kInput & (uint64_t(1) << 61)) ? 61 :
    (kInput & (uint64_t(1) << 62)) ? 62 :
    (kInput & (uint64_t(1) << 63)) ? 63 : 64;
};

// Support - PopCnt
// ================

// Based on the following resource:
//   http://graphics.stanford.edu/~seander/bithacks.html
//
// Alternatively, for a very small number of bits in `x`:
//   uint32_t n = 0;
//   while (x) {
//     x &= x - 1;
//     n++;
//   }
//   return n;

//! \cond
namespace Internal {
  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG uint32_t constPopcntImpl(uint32_t x) noexcept {
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    return (((x + (x >> 4)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24;
  }

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG uint32_t constPopcntImpl(uint64_t x) noexcept {
#if ASMJIT_ARCH_BITS >= 64
    x = x - ((x >> 1) & 0x5555555555555555u);
    x = (x & 0x3333333333333333u) + ((x >> 2) & 0x3333333333333333u);
    return uint32_t((((x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Fu) * 0x0101010101010101u) >> 56);
#else
    return constPopcntImpl(uint32_t(x >> 32)) +
           constPopcntImpl(uint32_t(x & 0xFFFFFFFFu));
#endif
  }

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG uint32_t popcntImpl(uint32_t x) noexcept {
#if defined(__GNUC__)
    return uint32_t(__builtin_popcount(x));
#else
    return constPopcntImpl(asUInt(x));
#endif
  }

  [[nodiscard]]
  static ASMJIT_INLINE_NODEBUG uint32_t popcntImpl(uint64_t x) noexcept {
#if defined(__GNUC__)
    return uint32_t(__builtin_popcountll(x));
#else
    return constPopcntImpl(asUInt(x));
#endif
  }
}
//! \endcond

//! Calculates count of bits in `x`.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t popcnt(T x) noexcept { return Internal::popcntImpl(asUInt(x)); }

//! Calculates count of bits in `x` (useful in constant expressions).
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t constPopcnt(T x) noexcept { return Internal::constPopcntImpl(asUInt(x)); }

// Support - HasAtLeast2BitsSet
// ============================

//! Tests whether `x` has at least 2 bits set.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool hasAtLeast2BitsSet(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  return !(U(x) & U(U(x) - U(1)));
}

// Support - Min/Max
// =================

// NOTE: These are constexpr `min()` and `max()` implementations that are not
// exactly the same as `std::min()` and `std::max()`. The return value is not
// a reference to `a` or `b` but it's a new value instead.

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T min(const T& a, const T& b) noexcept { return b < a ? b : a; }

template<typename T, typename... Args>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T min(const T& a, const T& b, Args&&... args) noexcept { return min(min(a, b), std::forward<Args>(args)...); }

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T max(const T& a, const T& b) noexcept { return a < b ? b : a; }

template<typename T, typename... Args>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T max(const T& a, const T& b, Args&&... args) noexcept { return max(max(a, b), std::forward<Args>(args)...); }

// Support - Immediate Helpers
// ===========================

namespace Internal {
  template<typename T, bool IsFloat>
  struct ImmConv {
    static ASMJIT_INLINE_NODEBUG int64_t fromT(const T& x) noexcept { return int64_t(x); }
    static ASMJIT_INLINE_NODEBUG T toT(int64_t x) noexcept { return T(uint64_t(x) & Support::allOnes<std::make_unsigned_t<T>>()); }
  };

  template<typename T>
  struct ImmConv<T, true> {
    static ASMJIT_INLINE_NODEBUG int64_t fromT(const T& x) noexcept { return int64_t(bitCast<int64_t>(double(x))); }
    static ASMJIT_INLINE_NODEBUG T toT(int64_t x) noexcept { return T(bitCast<double>(x)); }
  };
}

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG int64_t immediateFromT(const T& x) noexcept { return Internal::ImmConv<T, std::is_floating_point_v<T>>::fromT(x); }

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T immediateToT(int64_t x) noexcept { return Internal::ImmConv<T, std::is_floating_point_v<T>>::toT(x); }

// Support - Overflow Arithmetic
// =============================

//! \cond
namespace Internal {
  template<typename T>
  inline T addOverflowFallback(T x, T y, FastUInt8* of) noexcept {
    using U = std::make_unsigned_t<T>;

    U result = U(U(x) + U(y));
    *of = FastUInt8(*of | FastUInt8(std::is_unsigned_v<T> ? result < U(x) : T((U(x) ^ ~U(y)) & (U(x) ^ result)) < 0));
    return T(result);
  }

  template<typename T>
  inline T subOverflowFallback(T x, T y, FastUInt8* of) noexcept {
    using U = std::make_unsigned_t<T>;

    U result = U(U(x) - U(y));
    *of = FastUInt8(*of | FastUInt8(std::is_unsigned_v<T> ? result > U(x) : T((U(x) ^ U(y)) & (U(x) ^ result)) < 0));
    return T(result);
  }

  template<typename T>
  inline T mulOverflowFallback(T x, T y, FastUInt8* of) noexcept {
    using I = typename Internal::StdInt<sizeof(T) * 2, std::is_unsigned_v<T>>::Type;
    using U = std::make_unsigned_t<I>;

    U mask = allOnes<U>();
    if constexpr (std::is_signed_v<T>) {
      U prod = U(I(x)) * U(I(y));
      *of = FastUInt8(*of | FastUInt8(I(prod) < I(std::numeric_limits<T>::lowest()) || I(prod) > I(std::numeric_limits<T>::max())));
      return T(I(prod & mask));
    }
    else {
      U prod = U(x) * U(y);
      *of = FastUInt8(*of | FastUInt8((prod & ~mask) != 0));
      return T(prod & mask);
    }
  }

  template<>
  inline int64_t mulOverflowFallback(int64_t x, int64_t y, FastUInt8* of) noexcept {
    int64_t result = int64_t(uint64_t(x) * uint64_t(y));
    *of = FastUInt8(*of | FastUInt8(x && (result / x != y)));
    return result;
  }

  template<>
  inline uint64_t mulOverflowFallback(uint64_t x, uint64_t y, FastUInt8* of) noexcept {
    uint64_t result = x * y;
    *of = FastUInt8(*of | FastUInt8(y != 0 && allOnes<uint64_t>() / y < x));
    return result;
  }

  // These can be specialized.
  template<typename T> inline T addOverflowImpl(const T& x, const T& y, FastUInt8* of) noexcept { return addOverflowFallback(x, y, of); }
  template<typename T> inline T subOverflowImpl(const T& x, const T& y, FastUInt8* of) noexcept { return subOverflowFallback(x, y, of); }
  template<typename T> inline T mulOverflowImpl(const T& x, const T& y, FastUInt8* of) noexcept { return mulOverflowFallback(x, y, of); }

#if defined(__GNUC__) && !defined(ASMJIT_NO_INTRINSICS)
#define ASMJIT_ARITH_OVERFLOW_SPECIALIZE(FUNC, T, RESULT_T, BUILTIN)     \
    template<>                                                             \
    inline T FUNC(const T& x, const T& y, FastUInt8* of) noexcept {        \
      RESULT_T result;                                                     \
      *of = FastUInt8(*of | (BUILTIN((RESULT_T)x, (RESULT_T)y, &result))); \
      return T(result);                                                    \
    }
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(addOverflowImpl, int32_t , int               , __builtin_sadd_overflow  )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(addOverflowImpl, uint32_t, unsigned int      , __builtin_uadd_overflow  )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(addOverflowImpl, int64_t , long long         , __builtin_saddll_overflow)
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(addOverflowImpl, uint64_t, unsigned long long, __builtin_uaddll_overflow)
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(subOverflowImpl, int32_t , int               , __builtin_ssub_overflow  )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(subOverflowImpl, uint32_t, unsigned int      , __builtin_usub_overflow  )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(subOverflowImpl, int64_t , long long         , __builtin_ssubll_overflow)
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(subOverflowImpl, uint64_t, unsigned long long, __builtin_usubll_overflow)
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(mulOverflowImpl, int32_t , int               , __builtin_smul_overflow  )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(mulOverflowImpl, uint32_t, unsigned int      , __builtin_umul_overflow  )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(mulOverflowImpl, int64_t , long long         , __builtin_smulll_overflow)
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(mulOverflowImpl, uint64_t, unsigned long long, __builtin_umulll_overflow)
#undef ASMJIT_ARITH_OVERFLOW_SPECIALIZE
#endif

  // There is a bug in MSVC that makes these specializations unusable, maybe in the future...
#if defined(_MSC_VER) && 0
#define ASMJIT_ARITH_OVERFLOW_SPECIALIZE(FUNC, T, ALT_T, BUILTIN)        \
    template<>                                                             \
    inline T FUNC(T x, T y, FastUInt8* of) noexcept {                      \
      ALT_T result;                                                        \
      *of = FastUInt8(*of | BUILTIN(0, (ALT_T)x, (ALT_T)y, &result));      \
      return T(result);                                                    \
    }
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(addOverflowImpl, uint32_t, unsigned int      , _addcarry_u32 )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(subOverflowImpl, uint32_t, unsigned int      , _subborrow_u32)
#if ARCH_BITS >= 64
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(addOverflowImpl, uint64_t, unsigned __int64  , _addcarry_u64 )
  ASMJIT_ARITH_OVERFLOW_SPECIALIZE(subOverflowImpl, uint64_t, unsigned __int64  , _subborrow_u64)
#endif
#undef ASMJIT_ARITH_OVERFLOW_SPECIALIZE
#endif
} // {Internal}
//! \endcond

template<typename T>
static inline T addOverflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(Internal::addOverflowImpl(asStdInt(x), asStdInt(y), of)); }

template<typename T>
static inline T subOverflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(Internal::subOverflowImpl(asStdInt(x), asStdInt(y), of)); }

template<typename T>
static inline T mulOverflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(Internal::mulOverflowImpl(asStdInt(x), asStdInt(y), of)); }

template<typename T>
static inline T maddOverflow(const T& x, const T& y, const T& addend, FastUInt8* of) noexcept {
  T v = T(Internal::mulOverflowImpl(asStdInt(x), asStdInt(y), of));
  return T(Internal::addOverflowImpl(asStdInt(v), asStdInt(addend), of));
}

// Support - IsPowerOf2
// ====================

//! Tests whether `x` is a power of two (only one bit is set).
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isPowerOf2(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  U x_minus_1 = U(U(x) - U(1));
  return U(U(x) ^ x_minus_1) > x_minus_1;
}

//! Tests whether `x` is a power of two up to `n`.
template<typename T, typename N>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isPowerOf2UpTo(T x, N n) noexcept {
  using U = std::make_unsigned_t<T>;
  U x_minus_1 = U(U(x) - U(1));
  return bool_and(x_minus_1 < U(n), !(U(x) & x_minus_1));
}

//! Tests whether `x` is either zero or a power of two (only one bit is set).
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isZeroOrPowerOf2(T x) noexcept {
  using U = std::make_unsigned_t<T>;
  return !(U(x) & (U(x) - U(1)));
}

//! Tests whether `x` is either zero or a power of two up to `n`.
template<typename T, typename N>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isZeroOrPowerOf2UpTo(T x, N n) noexcept {
  using U = std::make_unsigned_t<T>;
  return bool_and(U(x) <= U(n), !(U(x) & (U(x) - U(1))));
}

// Support - Alignment
// ===================

template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isAligned(X base, Y alignment) noexcept {
  using U = typename Internal::StdInt<sizeof(X), 1>::Type;
  return ((U)base % (U)alignment) == 0;
}

template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X alignUp(X x, Y alignment) noexcept {
  using U = typename Internal::StdInt<sizeof(X), 1>::Type;
  return (X)( ((U)x + ((U)(alignment) - 1u)) & ~((U)(alignment) - 1u) );
}

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T alignUpPowerOf2(T x) noexcept {
  using U = typename Internal::StdInt<sizeof(T), 1>::Type;
  return (T)(fillTrailingBits(U(x) - 1u) + 1u);
}

//! Returns either zero or a positive difference between `base` and `base` when aligned to `alignment`.
template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR typename Internal::StdInt<sizeof(X), 1>::Type alignUpDiff(X base, Y alignment) noexcept {
  using U = typename Internal::StdInt<sizeof(X), 1>::Type;
  return alignUp(U(base), alignment) - U(base);
}

template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X alignDown(X x, Y alignment) noexcept {
  using U = typename Internal::StdInt<sizeof(X), 1>::Type;
  return (X)( (U)x & ~((U)(alignment) - 1u) );
}

// Support - NumGranularized
// =========================

//! Calculates the number of elements that would be required if `base` is granularized by `granularity`.
//! This function can be used to calculate the number of BitWords to represent N bits, for example.
template<typename X, typename Y>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR X numGranularized(X base, Y granularity) noexcept {
  using U = typename Internal::StdInt<sizeof(X), 1>::Type;
  return X((U(base) + U(granularity) - 1) / U(granularity));
}

// Support - IsBetween
// ===================

//! Checks whether `x` is greater than or equal to `a` and lesser than or equal to `b`.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isBetween(const T& x, const T& a, const T& b) noexcept {
  return x >= a && x <= b;
}

// Support - IsInt & IsUInt
// ========================

//! Checks whether the given integer `x` can be casted to a 4-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt4(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? isBetween<S>(S(x), -8, 7) : U(x) <= U(7u);
}

//! Checks whether the given integer `x` can be casted to a 7-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt7(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? isBetween<S>(S(x), -64, 63) : U(x) <= U(63u);
}

//! Checks whether the given integer `x` can be casted to an 8-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt8(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? sizeof(T) <= 1 || isBetween<S>(S(x), -128, 127) : U(x) <= U(127u);
}

//! Checks whether the given integer `x` can be casted to a 9-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt9(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? sizeof(T) <= 1 || isBetween<S>(S(x), -256, 255)
                             : sizeof(T) <= 1 || U(x) <= U(255u);
}

//! Checks whether the given integer `x` can be casted to a 10-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt10(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? sizeof(T) <= 1 || isBetween<S>(S(x), -512, 511)
                             : sizeof(T) <= 1 || U(x) <= U(511u);
}

//! Checks whether the given integer `x` can be casted to a 16-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt16(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? sizeof(T) <= 2 || isBetween<S>(S(x), -32768, 32767)
                             : sizeof(T) <= 1 || U(x) <= U(32767u);
}

//! Checks whether the given integer `x` can be casted to a 32-bit signed integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isInt32(T x) noexcept {
  using S = std::make_signed_t<T>;
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? sizeof(T) <= 4 || isBetween<S>(S(x), -2147483647 - 1, 2147483647)
                             : sizeof(T) <= 2 || U(x) <= U(2147483647u);
}

//! Checks whether the given integer `x` can be casted to a 4-bit unsigned integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isUInt4(T x) noexcept {
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? x >= T(0) && x <= T(15)
                             : U(x) <= U(15u);
}

//! Checks whether the given integer `x` can be casted to an 8-bit unsigned integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isUInt8(T x) noexcept {
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? (sizeof(T) <= 1 || T(x) <= T(255)) && x >= T(0)
                             : (sizeof(T) <= 1 || U(x) <= U(255u));
}

//! Checks whether the given integer `x` can be casted to a 12-bit unsigned integer (ARM specific).
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isUInt12(T x) noexcept {
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? (sizeof(T) <= 1 || T(x) <= T(4095)) && x >= T(0)
                             : (sizeof(T) <= 1 || U(x) <= U(4095u));
}

//! Checks whether the given integer `x` can be casted to a 16-bit unsigned integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isUInt16(T x) noexcept {
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? (sizeof(T) <= 2 || T(x) <= T(65535)) && x >= T(0)
                             : (sizeof(T) <= 2 || U(x) <= U(65535u));
}

//! Checks whether the given integer `x` can be casted to a 32-bit unsigned integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isUInt32(T x) noexcept {
  using U = std::make_unsigned_t<T>;

  return std::is_signed_v<T> ? (sizeof(T) <= 4 || T(x) <= T(4294967295u)) && x >= T(0)
                             : (sizeof(T) <= 4 || U(x) <= U(4294967295u));
}

//! Checks whether the given integer `x` can be casted to a 32-bit unsigned integer.
template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR bool isIntOrUInt32(T x) noexcept {
  return sizeof(T) <= 4 ? true : (uint32_t(uint64_t(x) >> 32) + 1u) <= 1u;
}

[[nodiscard]]
static bool ASMJIT_INLINE_NODEBUG isEncodableOffset32(int32_t offset, uint32_t nBits) noexcept {
  uint32_t nRev = 32 - nBits;
  return Support::sar(Support::shl(offset, nRev), nRev) == offset;
}

[[nodiscard]]
static bool ASMJIT_INLINE_NODEBUG isEncodableOffset64(int64_t offset, uint32_t nBits) noexcept {
  uint32_t nRev = 64 - nBits;
  return Support::sar(Support::shl(offset, nRev), nRev) == offset;
}

// Support - ByteSwap
// ==================

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint16_t byteswap16(uint16_t x) noexcept {
  return uint16_t(((x >> 8) & 0xFFu) | ((x & 0xFFu) << 8));
}

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t byteswap32(uint32_t x) noexcept {
  return (x << 24) | (x >> 24) | ((x << 8) & 0x00FF0000u) | ((x >> 8) & 0x0000FF00);
}

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint64_t byteswap64(uint64_t x) noexcept {
#if (defined(__GNUC__) || defined(__clang__)) && !defined(ASMJIT_NO_INTRINSICS)
  return uint64_t(__builtin_bswap64(uint64_t(x)));
#elif defined(_MSC_VER) && !defined(ASMJIT_NO_INTRINSICS)
  return uint64_t(_byteswap_uint64(uint64_t(x)));
#else
  return (uint64_t(byteswap32(uint32_t(uint64_t(x) >> 32        )))      ) |
         (uint64_t(byteswap32(uint32_t(uint64_t(x) & 0xFFFFFFFFu))) << 32) ;
#endif
}

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T byteswap(T x) noexcept {
  static_assert(std::is_integral_v<T>, "byteswap() expects the given type to be integral");
  if constexpr (sizeof(T) == 8) {
    return T(byteswap64(uint64_t(x)));
  }
  else if constexpr (sizeof(T) == 4) {
    return T(byteswap32(uint32_t(x)));
  }
  else if constexpr (sizeof(T) == 2) {
    return T(byteswap16(uint16_t(x)));
  }
  else {
    static_assert(sizeof(T) == 1, "byteswap() can be used with a type of size 1, 2, 4, or 8");
    return x;
  }
}

// Support - BytePack & Unpack
// ===========================

//! Pack four 8-bit integer into a 32-bit integer as it is an array of `{b0,b1,b2,b3}`.
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t bytepack32_4x8(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
  return ASMJIT_ARCH_LE ? (a | (b << 8) | (c << 16) | (d << 24))
                        : (d | (c << 8) | (b << 16) | (a << 24));
}

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t unpackU32At0(T x) noexcept { return ASMJIT_ARCH_LE ? uint32_t(uint64_t(x) & 0xFFFFFFFFu) : uint32_t(uint64_t(x) >> 32); }

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t unpackU32At1(T x) noexcept { return ASMJIT_ARCH_BE ? uint32_t(uint64_t(x) & 0xFFFFFFFFu) : uint32_t(uint64_t(x) >> 32); }

// Support - Position of byte (in bit-shift)
// =========================================

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t byteShiftOfDWordStruct(uint32_t index) noexcept {
  return ASMJIT_ARCH_LE ? index * 8 : (uint32_t(sizeof(uint32_t)) - 1u - index) * 8;
}

// Support - String Utilities
// ==========================

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T asciiToLower(T c) noexcept { return T(c ^ T(T(c >= T('A') && c <= T('Z')) << 5)); }

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR T asciiToUpper(T c) noexcept { return T(c ^ T(T(c >= T('a') && c <= T('z')) << 5)); }

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG size_t strLen(const char* s, size_t maxSize) noexcept {
  size_t i = 0;
  while (i < maxSize && s[i] != '\0')
    i++;
  return i;
}

[[nodiscard]]
static ASMJIT_INLINE_CONSTEXPR uint32_t hashRound(uint32_t hash, uint32_t c) noexcept { return hash * 65599 + c; }

// Gets a hash of the given string `data` of size `size`. Size must be valid
// as this function doesn't check for a null terminator and allows it in the
// middle of the string.
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG uint32_t hashString(const char* data, size_t size) noexcept {
  uint32_t hashCode = 0;
  for (uint32_t i = 0; i < size; i++)
    hashCode = hashRound(hashCode, uint8_t(data[i]));
  return hashCode;
}

[[nodiscard]]
static ASMJIT_INLINE_NODEBUG const char* findPackedString(const char* p, uint32_t id) noexcept {
  uint32_t i = 0;
  while (i < id) {
    while (p[0])
      p++;
    p++;
    i++;
  }
  return p;
}

//! Compares two string views.
[[nodiscard]]
static ASMJIT_INLINE int compareStringViews(const char* aData, size_t aSize, const char* bData, size_t bSize) noexcept {
  size_t size = Support::min(aSize, bSize);

  for (size_t i = 0; i < size; i++) {
    int c = int(uint8_t(aData[i])) - int(uint8_t(bData[i]));
    if (c != 0)
      return c;
  }

  return int(aSize) - int(bSize);
}

// Support - Aligned / Unaligned Memory Read Access
// ================================================

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T loada(const void* p) noexcept {
  static_assert(std::is_integral_v<T>, "loada() expects the data-type to be integral");

  return *static_cast<const T*>(p);
}

template<typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T loadu(const void* p) noexcept {
  static_assert(std::is_integral_v<T>, "loadu() expects the data-type to be integral");

  using UnsignedType = typename Internal::StdInt<sizeof(T), 1>::Type;
  using UnalignedType = typename Internal::AliasedUInt<UnsignedType, 1>::Type;

  return T(*static_cast<const UnalignedType*>(p));
}

template<ByteOrder BO, typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T loada(const void* p) noexcept {
  T v = loada<T>(p);
  if constexpr (BO != ByteOrder::kNative) {
    v = byteswap(v);
  }
  return v;
}

template<ByteOrder BO, typename T>
[[nodiscard]]
static ASMJIT_INLINE_NODEBUG T loadu(const void* p) noexcept {
  T v = loadu<T>(p);
  if constexpr (BO != ByteOrder::kNative) {
    v = byteswap(v);
  }
  return v;
}

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int8_t load_i8(const void* p) noexcept { return *static_cast<const int8_t*>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint8_t load_u8(const void* p) noexcept { return *static_cast<const uint8_t*>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int16_t loada_i16(const void* p) noexcept { return loada<int16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int16_t loadu_i16(const void* p) noexcept { return loadu<int16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint16_t loada_u16(const void* p) noexcept { return loada<uint16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint16_t loadu_u16(const void* p) noexcept { return loadu<uint16_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int16_t loada_i16_le(const void* p) noexcept { return loada<ByteOrder::kLE, int16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int16_t loadu_i16_le(const void* p) noexcept { return loadu<ByteOrder::kLE, int16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint16_t loada_u16_le(const void* p) noexcept { return loada<ByteOrder::kLE, uint16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint16_t loadu_u16_le(const void* p) noexcept { return loadu<ByteOrder::kLE, uint16_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int16_t loada_i16_be(const void* p) noexcept { return loada<ByteOrder::kBE, int16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int16_t loadu_i16_be(const void* p) noexcept { return loadu<ByteOrder::kBE, int16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint16_t loada_u16_be(const void* p) noexcept { return loada<ByteOrder::kBE, uint16_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint16_t loadu_u16_be(const void* p) noexcept { return loadu<ByteOrder::kBE, uint16_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int32_t loada_i32(const void* p) noexcept { return loada<int32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int32_t loadu_i32(const void* p) noexcept { return loadu<int32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint32_t loada_u32(const void* p) noexcept { return loada<uint32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint32_t loadu_u32(const void* p) noexcept { return loadu<uint32_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int32_t loada_i32_le(const void* p) noexcept { return loada<ByteOrder::kLE, int32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int32_t loadu_i32_le(const void* p) noexcept { return loadu<ByteOrder::kLE, int32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint32_t loada_u32_le(const void* p) noexcept { return loada<ByteOrder::kLE, uint32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint32_t loadu_u32_le(const void* p) noexcept { return loadu<ByteOrder::kLE, uint32_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int32_t loada_i32_be(const void* p) noexcept { return loada<ByteOrder::kBE, int32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int32_t loadu_i32_be(const void* p) noexcept { return loadu<ByteOrder::kBE, int32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint32_t loada_u32_be(const void* p) noexcept { return loada<ByteOrder::kBE, uint32_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint32_t loadu_u32_be(const void* p) noexcept { return loadu<ByteOrder::kBE, uint32_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int64_t loada_i64(const void* p) noexcept { return loada<int64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int64_t loadu_i64(const void* p) noexcept { return loadu<int64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint64_t loada_u64(const void* p) noexcept { return loada<uint64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint64_t loadu_u64(const void* p) noexcept { return loadu<uint64_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int64_t loada_i64_le(const void* p) noexcept { return loada<ByteOrder::kLE, int64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int64_t loadu_i64_le(const void* p) noexcept { return loadu<ByteOrder::kLE, int64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint64_t loada_u64_le(const void* p) noexcept { return loada<ByteOrder::kLE, uint64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint64_t loadu_u64_le(const void* p) noexcept { return loadu<ByteOrder::kLE, uint64_t>(p); }

[[nodiscard]] static ASMJIT_INLINE_NODEBUG int64_t loada_i64_be(const void* p) noexcept { return loada<ByteOrder::kBE, int64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG int64_t loadu_i64_be(const void* p) noexcept { return loadu<ByteOrder::kBE, int64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint64_t loada_u64_be(const void* p) noexcept { return loada<ByteOrder::kBE, uint64_t>(p); }
[[nodiscard]] static ASMJIT_INLINE_NODEBUG uint64_t loadu_u64_be(const void* p) noexcept { return loadu<ByteOrder::kBE, uint64_t>(p); }

// Support - Aligned / Unaligned Memory Write Access
// =================================================

template<typename T>
static ASMJIT_INLINE_NODEBUG void storea(void* p, T x) noexcept {
  static_assert(std::is_integral_v<T>, "storea() expects its data-type to be integral");

  *static_cast<T*>(p) = x;
}

template<typename T>
static ASMJIT_INLINE_NODEBUG void storeu(void* p, T x) noexcept {
  static_assert(std::is_integral_v<T>, "storeu() expects its data-type to be integral");

  using UnsignedType = typename Internal::StdInt<sizeof(T), 1>::Type;
  using UnalignedType = typename Internal::AliasedUInt<UnsignedType, 1>::Type;

  *static_cast<UnalignedType*>(p) = UnsignedType(x);
}

template<ByteOrder BO, typename T>
static ASMJIT_INLINE_NODEBUG void storea(void* p, T x) noexcept {
  if constexpr (BO != ByteOrder::kNative) {
    x = byteswap(x);
  }
  storea<T>(p, x);
}

template<ByteOrder BO, typename T>
static ASMJIT_INLINE_NODEBUG void storeu(void* p, T x) noexcept {
  if constexpr (BO != ByteOrder::kNative) {
    x = byteswap(x);
  }
  storeu<T>(p, x);
}

static ASMJIT_INLINE_NODEBUG void store_i8(void* p, int8_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void store_u8(void* p, uint8_t x) noexcept { storea(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i16(void* p, int16_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i16(void* p, int16_t x) noexcept { storeu(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u16(void* p, uint16_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u16(void* p, uint16_t x) noexcept { storeu(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i16_le(void* p, int16_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i16_le(void* p, int16_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u16_le(void* p, uint16_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u16_le(void* p, uint16_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i16_be(void* p, int16_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i16_be(void* p, int16_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u16_be(void* p, uint16_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u16_be(void* p, uint16_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i32(void* p, int32_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i32(void* p, int32_t x) noexcept { storeu(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u32(void* p, uint32_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u32(void* p, uint32_t x) noexcept { storeu(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i32_le(void* p, int32_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i32_le(void* p, int32_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u32_le(void* p, uint32_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u32_le(void* p, uint32_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i32_be(void* p, int32_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i32_be(void* p, int32_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u32_be(void* p, uint32_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u32_be(void* p, uint32_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i64(void* p, int64_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i64(void* p, int64_t x) noexcept { storeu(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u64(void* p, uint64_t x) noexcept { storea(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u64(void* p, uint64_t x) noexcept { storeu(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i64_le(void* p, int64_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i64_le(void* p, int64_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u64_le(void* p, uint64_t x) noexcept { storea<ByteOrder::kLE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u64_le(void* p, uint64_t x) noexcept { storeu<ByteOrder::kLE>(p, x); }

static ASMJIT_INLINE_NODEBUG void storea_i64_be(void* p, int64_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_i64_be(void* p, int64_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storea_u64_be(void* p, uint64_t x) noexcept { storea<ByteOrder::kBE>(p, x); }
static ASMJIT_INLINE_NODEBUG void storeu_u64_be(void* p, uint64_t x) noexcept { storeu<ByteOrder::kBE>(p, x); }

// Support - Operators
// ===================

//! \cond INTERNAL
struct Set    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { DebugUtils::unused(x); return  y; } };
struct SetNot { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { DebugUtils::unused(x); return ~y; } };
struct And    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x &  y; } };
struct AndNot { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x & ~y; } };
struct NotAnd { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return ~x &  y; } };
struct Or     { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x |  y; } };
struct Xor    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x ^  y; } };
struct Add    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x +  y; } };
struct Sub    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return  x -  y; } };
struct Min    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return min<T>(x, y); } };
struct Max    { template<typename T> static ASMJIT_INLINE_NODEBUG T op(T x, T y) noexcept { return max<T>(x, y); } };
//! \endcond

// Support - BitWordIterator
// =========================

//! Iterates over each bit in a number which is set to 1.
//!
//! Example of use:
//!
//! ```
//! uint32_t bitsToIterate = 0x110F;
//! Support::BitWordIterator<uint32_t> it(bitsToIterate);
//!
//! while (it.hasNext()) {
//!   uint32_t bitIndex = it.next();
//!   std::printf("Bit at %u is set\n", unsigned(bitIndex));
//! }
//! ```
template<typename T>
class BitWordIterator {
public:
  ASMJIT_INLINE_NODEBUG explicit BitWordIterator(T bitWord) noexcept
    : _bitWord(bitWord) {}

  ASMJIT_INLINE_NODEBUG void init(T bitWord) noexcept { _bitWord = bitWord; }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasNext() const noexcept { return _bitWord != 0; }

  [[nodiscard]]
  ASMJIT_INLINE uint32_t next() noexcept {
    ASMJIT_ASSERT(_bitWord != 0);
    uint32_t index = ctz(_bitWord);
    _bitWord &= T(_bitWord - 1);
    return index;
  }

  T _bitWord;
};

// Support - BitVectorOps
// ======================

//! \cond
namespace Internal {
  template<typename T, class OperatorT, class FullWordOpT>
  static ASMJIT_INLINE void bitVectorOp(T* buf, size_t index, size_t count) noexcept {
    if (count == 0) {
      return;
    }

    const size_t kTSizeInBits = bitSizeOf<T>();
    size_t vecIndex = index / kTSizeInBits; // T[]
    size_t bitIndex = index % kTSizeInBits; // T[][]

    buf += vecIndex;

    // The first BitWord requires special handling to preserve bits outside the fill region.
    const T kFillMask = allOnes<T>();
    size_t firstNBits = min<size_t>(kTSizeInBits - bitIndex, count);

    buf[0] = OperatorT::op(buf[0], (kFillMask >> (kTSizeInBits - firstNBits)) << bitIndex);
    buf++;
    count -= firstNBits;

    // All bits between the first and last affected BitWords can be just filled.
    while (count >= kTSizeInBits) {
      buf[0] = FullWordOpT::op(buf[0], kFillMask);
      buf++;
      count -= kTSizeInBits;
    }

    // The last BitWord requires special handling as well
    if (count) {
      buf[0] = OperatorT::op(buf[0], kFillMask >> (kTSizeInBits - count));
    }
  }
}
//! \endcond

//! Sets bit in a bit-vector `buf` at `index`.
template<typename T>
static ASMJIT_INLINE_NODEBUG bool bitVectorGetBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  return bool((buf[vecIndex] >> bitIndex) & 0x1u);
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bitVectorSetBit(T* buf, size_t index, bool value) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1u) << bitIndex;
  if (value) {
    buf[vecIndex] |= bitMask;
  }
  else {
    buf[vecIndex] &= ~bitMask;
  }
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bitVectorFlipBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1u) << bitIndex;
  buf[vecIndex] ^= bitMask;
}

//! Fills `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bitVectorFill(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, Or, Set>(buf, index, count); }

//! Clears `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static ASMJIT_INLINE_NODEBUG void bitVectorClear(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, AndNot, SetNot>(buf, index, count); }

template<typename T>
static ASMJIT_INLINE size_t bitVectorIndexOf(T* buf, size_t start, bool value) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();
  size_t vecIndex = start / kTSizeInBits; // T[]
  size_t bitIndex = start % kTSizeInBits; // T[][]

  T* p = buf + vecIndex;

  // We always look for zeros, if value is `true` we have to flip all bits before the search.
  const T kFillMask = allOnes<T>();
  const T kFlipMask = value ? T(0) : kFillMask;

  // The first BitWord requires special handling as there are some bits we want to ignore.
  T bits = (*p ^ kFlipMask) & (kFillMask << bitIndex);
  for (;;) {
    if (bits) {
      return (size_t)(p - buf) * kTSizeInBits + ctz(bits);
    }
    bits = *++p ^ kFlipMask;
  }
}

// Support - BitVectorIterator
// ===========================

template<typename T>
class BitVectorIterator {
public:
  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;

  ASMJIT_INLINE_NODEBUG BitVectorIterator(const BitVectorIterator& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG BitVectorIterator(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    init(data, numBitWords, start);
  }

  ASMJIT_INLINE void init(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    const T* ptr = data + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = *ptr++ & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += bitSizeOf<T>()) < end) {
        bitWord = *ptr++;
      }
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bitWord;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasNext() const noexcept {
    return _current != T(0);
  }

  [[nodiscard]]
  ASMJIT_INLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord &= T(bitWord - 1u);

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end) {
      bitWord = *_ptr++;
    }

    _current = bitWord;
    return n;
  }

  [[nodiscard]]
  ASMJIT_INLINE size_t peekNext() const noexcept {
    ASMJIT_ASSERT(_current != T(0));
    return _idx + ctz(_current);
  }
};

// Support - BitVectorOpIterator
// =============================

template<typename T, class OperatorT>
class BitVectorOpIterator {
public:
  static inline constexpr uint32_t kTSizeInBits = bitSizeOf<T>();

  const T* _aPtr;
  const T* _bPtr;
  size_t _idx;
  size_t _end;
  T _current;

  ASMJIT_INLINE_NODEBUG BitVectorOpIterator(const T* aData, const T* bData, size_t numBitWords, size_t start = 0) noexcept {
    init(aData, bData, numBitWords, start);
  }

  ASMJIT_INLINE void init(const T* aData, const T* bData, size_t numBitWords, size_t start = 0) noexcept {
    const T* aPtr = aData + (start / bitSizeOf<T>());
    const T* bPtr = bData + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = OperatorT::op(*aPtr++, *bPtr++) & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += kTSizeInBits) < end) {
        bitWord = OperatorT::op(*aPtr++, *bPtr++);
      }
    }

    _aPtr = aPtr;
    _bPtr = bPtr;
    _idx = idx;
    _end = end;
    _current = bitWord;
  }

  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool hasNext() noexcept {
    return _current != T(0);
  }

  [[nodiscard]]
  ASMJIT_INLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord &= T(bitWord - 1u);

    size_t n = _idx + bit;
    while (!bitWord && (_idx += kTSizeInBits) < _end) {
      bitWord = OperatorT::op(*_aPtr++, *_bPtr++);
    }

    _current = bitWord;
    return n;
  }
};

// Support - Sorting
// =================

//! Sort order.
enum class SortOrder : uint32_t {
  //!< Ascending order.
  kAscending  = 0,
  //!< Descending order.
  kDescending = 1
};

//! A helper class that provides comparison of any user-defined type that
//! implements `<` and `>` operators (primitive types are supported as well).
template<SortOrder kOrder = SortOrder::kAscending>
struct Compare {
  template<typename A, typename B>
  ASMJIT_INLINE_NODEBUG int operator()(const A& a, const B& b) const noexcept {
    return kOrder == SortOrder::kAscending ? int(a > b) - int(a < b) : int(a < b) - int(a > b);
  }
};

//! Insertion sort.
template<typename T, typename CompareT = Compare<SortOrder::kAscending>>
static inline void iSort(T* base, size_t size, const CompareT& cmp = CompareT()) noexcept {
  for (T* pm = base + 1; pm < base + size; pm++) {
    for (T* pl = pm; pl > base && cmp(pl[-1], pl[0]) > 0; pl--) {
      std::swap(pl[-1], pl[0]);
    }
  }
}

//! \cond
namespace Internal {
  //! Quick-sort implementation.
  template<typename T, class CompareT>
  struct QSortImpl {
    static inline constexpr size_t kStackSize = 64u * 2u;
    static inline constexpr size_t kISortThreshold = 7u;

    // Based on "PDCLib - Public Domain C Library" and rewritten to C++.
    static void sort(T* base, size_t size, const CompareT& cmp) noexcept {
      T* end = base + size;
      T* stack[kStackSize];
      T** stackptr = stack;

      for (;;) {
        if ((size_t)(end - base) > kISortThreshold) {
          // We work from second to last - first will be pivot element.
          T* pi = base + 1;
          T* pj = end - 1;
          std::swap(base[(size_t)(end - base) / 2], base[0]);

          if (cmp(*pi  , *pj  ) > 0) { std::swap(*pi  , *pj  ); }
          if (cmp(*base, *pj  ) > 0) { std::swap(*base, *pj  ); }
          if (cmp(*pi  , *base) > 0) { std::swap(*pi  , *base); }

          // Now we have the median for pivot element, entering main loop.
          for (;;) {
            while (pi < pj   && cmp(*++pi, *base) < 0) continue; // Move `i` right until `*i >= pivot`.
            while (pj > base && cmp(*--pj, *base) > 0) continue; // Move `j` left  until `*j <= pivot`.

            if (pi > pj) {
              break;
            }
            std::swap(*pi, *pj);
          }

          // Move pivot into correct place.
          std::swap(*base, *pj);

          // Larger subfile base / end to stack, sort smaller.
          if (pj - base > end - pi) {
            // Left is larger.
            *stackptr++ = base;
            *stackptr++ = pj;
            base = pi;
          }
          else {
            // Right is larger.
            *stackptr++ = pi;
            *stackptr++ = end;
            end = pj;
          }
          ASMJIT_ASSERT(stackptr <= stack + kStackSize);
        }
        else {
          // UB sanitizer doesn't like applying offset to a nullptr base.
          if (base != end) {
            iSort(base, (size_t)(end - base), cmp);
          }

          if (stackptr == stack) {
            break;
          }

          end = *--stackptr;
          base = *--stackptr;
        }
      }
    }
  };
}
//! \endcond

//! Quick sort implementation.
//!
//! The main reason to provide a custom qsort implementation is that we needed something that will
//! never throw `bad_alloc` exception. This implementation doesn't use dynamic memory allocation.
template<typename T, class CompareT = Compare<SortOrder::kAscending>>
static ASMJIT_INLINE_NODEBUG void qSort(T* base, size_t size, const CompareT& cmp = CompareT()) noexcept {
  Internal::QSortImpl<T, CompareT>::sort(base, size, cmp);
}

// Support - ReverseIterator
// =========================

//! Reverse iterator to avoid including `<iterator>` header for iteration over arrays, specialized for
//! AsmJit use (noexcept by design).
template<typename T>
class ArrayReverseIterator {
public:
  //! \name Members
  //! \{

  T* _ptr {};

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_CONSTEXPR ArrayReverseIterator() noexcept = default;
  ASMJIT_INLINE_CONSTEXPR ArrayReverseIterator(const ArrayReverseIterator& other) noexcept = default;
  ASMJIT_INLINE_CONSTEXPR ArrayReverseIterator(T* ptr) noexcept : _ptr(ptr) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG ArrayReverseIterator& operator=(const ArrayReverseIterator& other) noexcept = default;

  ASMJIT_INLINE_NODEBUG bool operator==(const T* other) const noexcept { return _ptr == other; }
  ASMJIT_INLINE_NODEBUG bool operator==(const ArrayReverseIterator& other) const noexcept { return _ptr == other._ptr; }

  ASMJIT_INLINE_NODEBUG bool operator!=(const T* other) const noexcept { return _ptr != other; }
  ASMJIT_INLINE_NODEBUG bool operator!=(const ArrayReverseIterator& other) const noexcept { return _ptr != other._ptr; }

  ASMJIT_INLINE_NODEBUG bool operator<(const T* other) const noexcept { return _ptr < other; }
  ASMJIT_INLINE_NODEBUG bool operator<(const ArrayReverseIterator& other) const noexcept { return _ptr < other._ptr; }

  ASMJIT_INLINE_NODEBUG bool operator<=(const T* other) const noexcept { return _ptr <= other; }
  ASMJIT_INLINE_NODEBUG bool operator<=(const ArrayReverseIterator& other) const noexcept { return _ptr <= other._ptr; }

  ASMJIT_INLINE_NODEBUG bool operator>(const T* other) const noexcept { return _ptr > other; }
  ASMJIT_INLINE_NODEBUG bool operator>(const ArrayReverseIterator& other) const noexcept { return _ptr > other._ptr; }

  ASMJIT_INLINE_NODEBUG bool operator>=(const T* other) const noexcept { return _ptr >= other; }
  ASMJIT_INLINE_NODEBUG bool operator>=(const ArrayReverseIterator& other) const noexcept { return _ptr >= other._ptr; }

  ASMJIT_INLINE_NODEBUG ArrayReverseIterator& operator++() noexcept { _ptr--; return *this; }
  ASMJIT_INLINE_NODEBUG ArrayReverseIterator& operator--() noexcept { _ptr++; return *this; }

  ASMJIT_INLINE_NODEBUG ArrayReverseIterator operator++(int) noexcept { ArrayReverseIterator prev(*this); _ptr--; return prev; }
  ASMJIT_INLINE_NODEBUG ArrayReverseIterator operator--(int) noexcept { ArrayReverseIterator prev(*this); _ptr++; return prev; }

  template<typename Diff> ASMJIT_INLINE_NODEBUG ArrayReverseIterator operator+(const Diff& n) noexcept { return ArrayReverseIterator(_ptr -= n); }
  template<typename Diff> ASMJIT_INLINE_NODEBUG ArrayReverseIterator operator-(const Diff& n) noexcept { return ArrayReverseIterator(_ptr += n); }

  template<typename Diff> ASMJIT_INLINE_NODEBUG ArrayReverseIterator& operator+=(const Diff& n) noexcept { _ptr -= n; return *this; }
  template<typename Diff> ASMJIT_INLINE_NODEBUG ArrayReverseIterator& operator-=(const Diff& n) noexcept { _ptr += n; return *this; }

  ASMJIT_INLINE_CONSTEXPR T& operator*() const noexcept { return _ptr[-1]; }
  ASMJIT_INLINE_CONSTEXPR T* operator->() const noexcept { return &_ptr[-1]; }

  template<typename Diff> ASMJIT_INLINE_NODEBUG T& operator[](const Diff& n) noexcept { return *(_ptr - n - 1); }

  ASMJIT_INLINE_NODEBUG operator T*() const noexcept { return _ptr; }

  //! \}
};

// Support - Array
// ===============

//! Array type, similar to std::array<T, N>, with the possibility to use enums in operator[].
//!
//! \note The array has C semantics - the elements in the array are not initialized.
template<typename T, size_t N>
struct Array {
  //! \name Members
  //! \{

  //! The underlying array data, use \ref data() to access it.
  T _data[N];

  //! \}

  //! \cond
  // std compatibility.
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  using reference = value_type&;
  using const_reference = const value_type&;

  using pointer = value_type*;
  using const_pointer = const value_type*;

  using iterator = pointer;
  using const_iterator = const_pointer;
  //! \endcond

  //! \name Overloaded Operators
  //! \{

  template<typename Index>
  inline T& operator[](const Index& index) noexcept {
    using U = typename Internal::StdInt<sizeof(Index), 1>::Type;
    ASMJIT_ASSERT(U(index) < N);
    return _data[U(index)];
  }

  template<typename Index>
  inline const T& operator[](const Index& index) const noexcept {
    using U = typename Internal::StdInt<sizeof(Index), 1>::Type;
    ASMJIT_ASSERT(U(index) < N);
    return _data[U(index)];
  }

  constexpr inline bool operator==(const Array& other) const noexcept {
    for (size_t i = 0; i < N; i++) {
      if (_data[i] != other._data[i]) {
        return false;
      }
    }
    return true;
  }

  ASMJIT_INLINE_CONSTEXPR bool operator!=(const Array& other) const noexcept {
    return !operator==(other);
  }

  //! \}

  //! \name Accessors
  //! \{

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR bool empty() const noexcept { return false; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR size_t size() const noexcept { return N; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T* data() noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* data() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T& front() noexcept { return _data[0]; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T& front() const noexcept { return _data[0]; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T& back() noexcept { return _data[N - 1]; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T& back() const noexcept { return _data[N - 1]; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T* begin() noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T* end() noexcept { return _data + N; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* begin() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* end() const noexcept { return _data + N; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* cbegin() const noexcept { return _data; }

  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR const T* cend() const noexcept { return _data + N; }

  //! \}

  //! \name Utilities
  //! \{

  inline void swap(Array& other) noexcept {
    for (size_t i = 0; i < N; i++) {
      std::swap(_data[i], other._data[i]);
    }
  }

  inline void fill(const T& value) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = value;
    }
  }

  inline void copyFrom(const Array& other) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = other._data[i];
    }
  }

  template<typename Operator>
  inline void combine(const Array& other) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = Operator::op(_data[i], other._data[i]);
    }
  }

  template<typename Operator>
  inline T aggregate(T initialValue = T()) const noexcept {
    T value = initialValue;
    for (size_t i = 0; i < N; i++) {
      value = Operator::op(value, _data[i]);
    }
    return value;
  }

  template<typename Fn>
  inline void forEach(Fn&& fn) noexcept {
    for (size_t i = 0; i < N; i++) {
      fn(_data[i]);
    }
  }
  //! \}
};

// Support::Temporary
// ==================

//! Used to pass a temporary buffer to:
//!
//!   - Containers that use user-passed buffer as an initial storage (still can grow).
//!   - Zone allocator that would use the temporary buffer as a first block.
struct Temporary {
  //! \name Members
  //! \{

  void* _data;
  size_t _size;

  //! \}

  //! \name Construction & Destruction
  //! \{

  ASMJIT_INLINE_CONSTEXPR Temporary(const Temporary& other) noexcept = default;
  ASMJIT_INLINE_CONSTEXPR Temporary(void* data, size_t size) noexcept
    : _data(data),
      _size(size) {}

  //! \}

  //! \name Overloaded Operators
  //! \{

  ASMJIT_INLINE_NODEBUG Temporary& operator=(const Temporary& other) noexcept = default;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the data storage.
  template<typename T = void>
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR T* data() const noexcept { return static_cast<T*>(_data); }

  //! Returns the data storage size in bytes.
  [[nodiscard]]
  ASMJIT_INLINE_CONSTEXPR size_t size() const noexcept { return _size; }

  //! \}
};

} // {Support}

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_SUPPORT_H_INCLUDED
