// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_SUPPORT_H
#define _ASMJIT_CORE_SUPPORT_H

#include "../core/globals.h"

#if defined(_MSC_VER)
  #include <intrin.h>
#endif

ASMJIT_BEGIN_NAMESPACE

//! \cond INTERNAL
//! \addtogroup asmjit_core_support
//! \{

//! Contains support classes and functions that may be used by AsmJit source
//! and header files. Anything defined here is considered internal and should
//! not be used outside of AsmJit and related projects like AsmTK.
namespace Support {

// ============================================================================
// [asmjit::Support - PlacementNew]
// ============================================================================

//! Helper to implement placement new/delete without relying on `<new>` header.
struct PlacementNew { void *ptr; };

// ============================================================================
// [asmjit::Support - Architecture Features & Constraints]
// ============================================================================

static constexpr bool kUnalignedAccess16 = ASMJIT_ARCH_X86 != 0;
static constexpr bool kUnalignedAccess32 = ASMJIT_ARCH_X86 != 0;
static constexpr bool kUnalignedAccess64 = ASMJIT_ARCH_X86 != 0;

// ============================================================================
// [asmjit::Support - Internal]
// ============================================================================

namespace Internal {
  template<typename T, size_t Alignment>
  struct AlignedInt {};

  template<> struct AlignedInt<uint16_t, 1> { typedef uint16_t ASMJIT_ALIGN_TYPE(T, 1); };
  template<> struct AlignedInt<uint16_t, 2> { typedef uint16_t T; };
  template<> struct AlignedInt<uint32_t, 1> { typedef uint32_t ASMJIT_ALIGN_TYPE(T, 1); };
  template<> struct AlignedInt<uint32_t, 2> { typedef uint32_t ASMJIT_ALIGN_TYPE(T, 2); };
  template<> struct AlignedInt<uint32_t, 4> { typedef uint32_t T; };
  template<> struct AlignedInt<uint64_t, 1> { typedef uint64_t ASMJIT_ALIGN_TYPE(T, 1); };
  template<> struct AlignedInt<uint64_t, 2> { typedef uint64_t ASMJIT_ALIGN_TYPE(T, 2); };
  template<> struct AlignedInt<uint64_t, 4> { typedef uint64_t ASMJIT_ALIGN_TYPE(T, 4); };
  template<> struct AlignedInt<uint64_t, 8> { typedef uint64_t T; };

  // IntBySize - Make an int-type by size (signed or unsigned) that is the
  //             same as types defined by <stdint.h>.
  // Int32Or64 - Make an int-type that has at least 32 bits: [u]int[32|64]_t.

  template<size_t SIZE, int IS_SIGNED>
  struct IntBySize {}; // Fail if not specialized.

  template<> struct IntBySize<1, 0> { typedef uint8_t  Type; };
  template<> struct IntBySize<1, 1> { typedef int8_t   Type; };
  template<> struct IntBySize<2, 0> { typedef uint16_t Type; };
  template<> struct IntBySize<2, 1> { typedef int16_t  Type; };
  template<> struct IntBySize<4, 0> { typedef uint32_t Type; };
  template<> struct IntBySize<4, 1> { typedef int32_t  Type; };
  template<> struct IntBySize<8, 0> { typedef uint64_t Type; };
  template<> struct IntBySize<8, 1> { typedef int64_t  Type; };

  template<typename T, int IS_SIGNED = std::is_signed<T>::value>
  struct Int32Or64 : public IntBySize<sizeof(T) <= 4 ? size_t(4) : sizeof(T), IS_SIGNED> {};
}

// ============================================================================
// [asmjit::Support - FastUInt8]
// ============================================================================

#if ASMJIT_ARCH_X86
typedef uint8_t FastUInt8;
#else
typedef unsigned int FastUInt8;
#endif

// ============================================================================
// [asmjit::Support - IntBySize / Int32Or64]
// ============================================================================

//! Casts an integer `x` to either `int32_t` or `int64_t` depending on `T`.
template<typename T>
static constexpr typename Internal::Int32Or64<T, 1>::Type asInt(T x) noexcept { return (typename Internal::Int32Or64<T, 1>::Type)x; }

//! Casts an integer `x` to either `uint32_t` or `uint64_t` depending on `T`.
template<typename T>
static constexpr typename Internal::Int32Or64<T, 0>::Type asUInt(T x) noexcept { return (typename Internal::Int32Or64<T, 0>::Type)x; }

//! Casts an integer `x` to either `int32_t`, uint32_t`, `int64_t`, or `uint64_t` depending on `T`.
template<typename T>
static constexpr typename Internal::Int32Or64<T>::Type asNormalized(T x) noexcept { return (typename Internal::Int32Or64<T>::Type)x; }

// ============================================================================
// [asmjit::Support - BitCast]
// ============================================================================

namespace Internal {
  template<typename DstT, typename SrcT>
  union BitCastUnion {
    ASMJIT_INLINE BitCastUnion(SrcT src) noexcept : src(src) {}
    SrcT src;
    DstT dst;
  };
}

//! Bit-casts from `Src` type to `Dst` type.
//!
//! Useful to bit-cast between integers and floating points.
template<typename Dst, typename Src>
static inline Dst bitCast(const Src& x) noexcept { return Internal::BitCastUnion<Dst, Src>(x).dst; }

// ============================================================================
// [asmjit::Support - BitOps]
// ============================================================================

//! Storage used to store a pack of bits (should by compatible with a machine word).
typedef Internal::IntBySize<sizeof(uintptr_t), 0>::Type BitWord;

template<typename T>
static constexpr uint32_t bitSizeOf() noexcept { return uint32_t(sizeof(T) * 8u); }

//! Number of bits stored in a single `BitWord`.
static constexpr uint32_t kBitWordSizeInBits = bitSizeOf<BitWord>();

//! Returns `0 - x` in a safe way (no undefined behavior), works for unsigned numbers as well.
template<typename T>
static constexpr T neg(const T& x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(0) - U(x));
}

template<typename T>
static constexpr T allOnes() noexcept { return neg<T>(T(1)); }

//! Returns `x << y` (shift left logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
static constexpr X shl(const X& x, const Y& y) noexcept {
  typedef typename std::make_unsigned<X>::type U;
  return X(U(x) << y);
}

//! Returns `x >> y` (shift right logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
static constexpr X shr(const X& x, const Y& y) noexcept {
  typedef typename std::make_unsigned<X>::type U;
  return X(U(x) >> y);
}

//! Returns `x >> y` (shift right arithmetic) by explicitly casting `x` to a signed type and back.
template<typename X, typename Y>
static constexpr X sar(const X& x, const Y& y) noexcept {
  typedef typename std::make_signed<X>::type S;
  return X(S(x) >> y);
}

//! Returns `x | (x >> y)` - helper used by some bit manipulation helpers.
template<typename X, typename Y>
static constexpr X or_shr(const X& x, const Y& y) noexcept { return X(x | shr(x, y)); }

//! Returns `x & -x` - extracts lowest set isolated bit (like BLSI instruction).
template<typename T>
static constexpr T blsi(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(x) & neg(U(x)));
}

//! Generate a trailing bit-mask that has `n` least significant (trailing) bits set.
template<typename T, typename CountT>
static constexpr T lsbMask(CountT n) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return (sizeof(U) < sizeof(uintptr_t))
    ? T(U((uintptr_t(1) << n) - uintptr_t(1)))
    // Shifting more bits than the type provides is UNDEFINED BEHAVIOR.
    // In such case we trash the result by ORing it with a mask that has
    // all bits set and discards the UNDEFINED RESULT of the shift.
    : T(((U(1) << n) - U(1u)) | neg(U(n >= CountT(bitSizeOf<T>()))));
}

//! Gets whether `x` has Nth bit set.
template<typename T, typename IndexT>
static constexpr bool bitTest(T x, IndexT n) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return (U(x) & (U(1) << n)) != 0;
}

//! Return a bit-mask that has `x` bit set.
template<typename T>
static constexpr uint32_t bitMask(T x) noexcept { return (1u << x); }

//! Return a bit-mask that has `x` bit set (multiple arguments).
template<typename T, typename... ArgsT>
static constexpr uint32_t bitMask(T x, ArgsT... args) noexcept { return bitMask(x) | bitMask(args...); }

//! Convert a boolean value `b` to zero or full mask (all bits set).
template<typename DstT, typename SrcT>
static constexpr DstT bitMaskFromBool(SrcT b) noexcept {
  typedef typename std::make_unsigned<DstT>::type U;
  return DstT(U(0) - U(b));
}

namespace Internal {
  // Fills all trailing bits right from the first most significant bit set.
  static constexpr uint8_t fillTrailingBitsImpl(uint8_t x) noexcept { return or_shr(or_shr(or_shr(x, 1), 2), 4); }
  // Fills all trailing bits right from the first most significant bit set.
  static constexpr uint16_t fillTrailingBitsImpl(uint16_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8); }
  // Fills all trailing bits right from the first most significant bit set.
  static constexpr uint32_t fillTrailingBitsImpl(uint32_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8), 16); }
  // Fills all trailing bits right from the first most significant bit set.
  static constexpr uint64_t fillTrailingBitsImpl(uint64_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8), 16), 32); }
}

// Fills all trailing bits right from the first most significant bit set.
template<typename T>
static constexpr T fillTrailingBits(const T& x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(Internal::fillTrailingBitsImpl(U(x)));
}

// ============================================================================
// [asmjit::Support - CTZ]
// ============================================================================

namespace Internal {
  static constexpr uint32_t constCtzImpl(uint32_t xAndNegX) noexcept {
    return 31 - ((xAndNegX & 0x0000FFFFu) ? 16 : 0)
              - ((xAndNegX & 0x00FF00FFu) ?  8 : 0)
              - ((xAndNegX & 0x0F0F0F0Fu) ?  4 : 0)
              - ((xAndNegX & 0x33333333u) ?  2 : 0)
              - ((xAndNegX & 0x55555555u) ?  1 : 0);
  }

  static constexpr uint32_t constCtzImpl(uint64_t xAndNegX) noexcept {
    return 63 - ((xAndNegX & 0x00000000FFFFFFFFu) ? 32 : 0)
              - ((xAndNegX & 0x0000FFFF0000FFFFu) ? 16 : 0)
              - ((xAndNegX & 0x00FF00FF00FF00FFu) ?  8 : 0)
              - ((xAndNegX & 0x0F0F0F0F0F0F0F0Fu) ?  4 : 0)
              - ((xAndNegX & 0x3333333333333333u) ?  2 : 0)
              - ((xAndNegX & 0x5555555555555555u) ?  1 : 0);
  }

  template<typename T>
  static constexpr uint32_t constCtz(T x) noexcept {
    return constCtzImpl(x & neg(x));
  }

  static ASMJIT_INLINE uint32_t ctz(uint32_t x) noexcept {
  #if defined(__GNUC__)
    return uint32_t(__builtin_ctz(x));
  #elif defined(_MSC_VER) && (ASMJIT_ARCH_X86 || ASMJIT_ARCH_ARM)
    unsigned long i;
    _BitScanForward(&i, x);
    return uint32_t(i);
  #else
    return constCtz(x);
  #endif
  }

  static ASMJIT_INLINE uint32_t ctz(uint64_t x) noexcept {
  #if defined(__GNUC__)
    return uint32_t(__builtin_ctzll(x));
  #elif defined(_MSC_VER) && (ASMJIT_ARCH_X86 == 64 || ASMJIT_ARCH_ARM == 64)
    unsigned long i;
    _BitScanForward64(&i, x);
    return uint32_t(i);
  #else
    return constCtz(x);
  #endif
  }
}

//! Count trailing zeros in `x` (constant expression).
template<typename T>
static constexpr uint32_t constCtz(T x) noexcept { return Internal::constCtz(asUInt(x)); }

//! Count trailing zeros in `x` (returns a position of a first bit set in `x`).
//!
//! NOTE: The input MUST NOT be zero, otherwise the result is undefined.
template<typename T>
static inline uint32_t ctz(T x) noexcept { return Internal::ctz(asUInt(x)); }

// ============================================================================
// [asmjit::Support - PopCnt]
// ============================================================================

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

static inline uint32_t _popcntGeneric(uint32_t x) noexcept {
  x = x - ((x >> 1) & 0x55555555u);
  x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
  return (((x + (x >> 4)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24;
}

static inline uint32_t _popcntGeneric(uint64_t x) noexcept {
  if (ASMJIT_ARCH_BITS >= 64) {
    x = x - ((x >> 1) & 0x5555555555555555u);
    x = (x & 0x3333333333333333u) + ((x >> 2) & 0x3333333333333333u);
    return uint32_t((((x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Fu) * 0x0101010101010101u) >> 56);
  }
  else {
    return _popcntGeneric(uint32_t(x >> 32)) +
           _popcntGeneric(uint32_t(x & 0xFFFFFFFFu));
  }
}

static inline uint32_t _popcntImpl(uint32_t x) noexcept {
#if defined(__GNUC__)
  return uint32_t(__builtin_popcount(x));
#else
  return _popcntGeneric(asUInt(x));
#endif
}

static inline uint32_t _popcntImpl(uint64_t x) noexcept {
#if defined(__GNUC__)
  return uint32_t(__builtin_popcountll(x));
#else
  return _popcntGeneric(asUInt(x));
#endif
}

//! Gets count of bits in `x`.
template<typename T>
static inline uint32_t popcnt(T x) noexcept { return _popcntImpl(asUInt(x)); }

// ============================================================================
// [asmjit::Support - Min/Max]
// ============================================================================

// NOTE: These are constexpr `min()` and `max()` implementations that are not
// exactly the same as `std::min()` and `std::max()`. The return value is not
// a reference to `a` or `b` but it's a new value instead.

template<typename T>
static constexpr T min(const T& a, const T& b) noexcept { return b < a ? b : a; }

template<typename T, typename... ArgsT>
static constexpr T min(const T& a, const T& b, ArgsT&&... args) noexcept { return min(min(a, b), std::forward<ArgsT>(args)...); }

template<typename T>
static constexpr T max(const T& a, const T& b) noexcept { return a < b ? b : a; }

template<typename T, typename... ArgsT>
static constexpr T max(const T& a, const T& b, ArgsT&&... args) noexcept { return max(max(a, b), std::forward<ArgsT>(args)...); }

// ============================================================================
// [asmjit::Support - Overflow Arithmetic]
// ============================================================================

namespace Internal {
  template<typename T>
  static ASMJIT_INLINE T addOverflowImpl(T x, T y, FastUInt8* of) noexcept {
    typedef typename std::make_unsigned<T>::type U;

    U result = U(x) + U(y);
    *of = FastUInt8(*of | FastUInt8(std::is_unsigned<T>::value ? result < U(x) : T((U(x) ^ ~U(y)) & (U(x) ^ result)) < 0));
    return T(result);
  }

  template<typename T>
  static ASMJIT_INLINE T subOverflowImpl(T x, T y, FastUInt8* of) noexcept {
    typedef typename std::make_unsigned<T>::type U;

    U result = U(x) - U(y);
    *of = FastUInt8(*of | FastUInt8(std::is_unsigned<T>::value ? result > U(x) : T((U(x) ^ U(y)) & (U(x) ^ result)) < 0));
    return T(result);
  }
}

template<typename T>
static ASMJIT_INLINE T addOverflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(Internal::addOverflowImpl(x, y, of)); }

template<typename T>
static ASMJIT_INLINE T subOverflow(const T& x, const T& y, FastUInt8* of) noexcept { return T(Internal::subOverflowImpl(x, y, of)); }

// ============================================================================
// [asmjit::Support - Alignment]
// ============================================================================

template<typename X, typename Y>
static constexpr bool isAligned(X base, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return ((U)base % (U)alignment) == 0;
}

//! Gets whether the `x` is a power of two (only one bit is set).
template<typename T>
static constexpr bool isPowerOf2(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return x && !(U(x) & (U(x) - U(1)));
}

template<typename X, typename Y>
static constexpr X alignUp(X x, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return (X)( ((U)x + ((U)(alignment) - 1u)) & ~((U)(alignment) - 1u) );
}

template<typename T>
static constexpr T alignUpPowerOf2(T x) noexcept {
  typedef typename Internal::IntBySize<sizeof(T), 0>::Type U;
  return (T)(fillTrailingBits(U(x) - 1u) + 1u);
}

//! Gets zero or a positive difference between `base` and `base` when aligned to `alignment`.
template<typename X, typename Y>
static constexpr typename Internal::IntBySize<sizeof(X), 0>::Type alignUpDiff(X base, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return alignUp(U(base), alignment) - U(base);
}

template<typename X, typename Y>
static constexpr X alignDown(X x, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return (X)( (U)x & ~((U)(alignment) - 1u) );
}

// ============================================================================
// [asmjit::Support - NumGranularized]
// ============================================================================

//! Calculates the number of elements that would be required if `base` is
//! granularized by `granularity`. This function can be used to calculate
//! the number of BitWords to represent N bits, for example.
template<typename X, typename Y>
static constexpr X numGranularized(X base, Y granularity) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return X((U(base) + U(granularity) - 1) / U(granularity));
}

// ============================================================================
// [asmjit::Support - IsBetween]
// ============================================================================

//! Checks whether `x` is greater than or equal to `a` and lesser than or equal to `b`.
template<typename T>
static constexpr bool isBetween(const T& x, const T& a, const T& b) noexcept {
  return x >= a && x <= b;
}

// ============================================================================
// [asmjit::Support - IsInt / IsUInt]
// ============================================================================

//! Checks whether the given integer `x` can be casted to a 4-bit signed integer.
template<typename T>
static constexpr bool isInt4(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? isBetween<S>(S(x), -8, 7)
                                  : U(x) <= U(7u);
}

//! Checks whether the given integer `x` can be casted to an 8-bit signed integer.
template<typename T>
static constexpr bool isInt8(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? sizeof(T) <= 1 || isBetween<S>(S(x), -128, 127)
                                  : U(x) <= U(127u);
}

//! Checks whether the given integer `x` can be casted to a 16-bit signed integer.
template<typename T>
static constexpr bool isInt16(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? sizeof(T) <= 2 || isBetween<S>(S(x), -32768, 32767)
                                  : sizeof(T) <= 1 || U(x) <= U(32767u);
}

//! Checks whether the given integer `x` can be casted to a 32-bit signed integer.
template<typename T>
static constexpr bool isInt32(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? sizeof(T) <= 4 || isBetween<S>(S(x), -2147483647 - 1, 2147483647)
                                  : sizeof(T) <= 2 || U(x) <= U(2147483647u);
}

//! Checks whether the given integer `x` can be casted to a 4-bit unsigned integer.
template<typename T>
static constexpr bool isUInt4(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? x >= T(0) && x <= T(15)
                                  : U(x) <= U(15u);
}

//! Checks whether the given integer `x` can be casted to an 8-bit unsigned integer.
template<typename T>
static constexpr bool isUInt8(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 1 || T(x) <= T(255)) && x >= T(0)
                                  : (sizeof(T) <= 1 || U(x) <= U(255u));
}

//! Checks whether the given integer `x` can be casted to a 12-bit unsigned integer (ARM specific).
template<typename T>
static constexpr bool isUInt12(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 1 || T(x) <= T(4095)) && x >= T(0)
                                  : (sizeof(T) <= 1 || U(x) <= U(4095u));
}

//! Checks whether the given integer `x` can be casted to a 16-bit unsigned integer.
template<typename T>
static constexpr bool isUInt16(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 2 || T(x) <= T(65535)) && x >= T(0)
                                  : (sizeof(T) <= 2 || U(x) <= U(65535u));
}

//! Checks whether the given integer `x` can be casted to a 32-bit unsigned integer.
template<typename T>
static constexpr bool isUInt32(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 4 || T(x) <= T(4294967295u)) && x >= T(0)
                                  : (sizeof(T) <= 4 || U(x) <= U(4294967295u));
}

//! Checks whether the given integer `x` can be casted to a 32-bit unsigned integer.
template<typename T>
static constexpr bool isIntOrUInt32(T x) noexcept {
  return sizeof(T) <= 4 ? true : (uint32_t(uint64_t(x) >> 32) + 1u) <= 1u;
}

// ============================================================================
// [asmjit::Support - ByteSwap]
// ============================================================================

static constexpr uint32_t byteswap32(uint32_t x) noexcept {
  return (x << 24) | (x >> 24) | ((x << 8) & 0x00FF0000u) | ((x >> 8) & 0x0000FF00);
}

// ============================================================================
// [asmjit::Support - BytePack / Unpack]
// ============================================================================

//! Pack four 8-bit integer into a 32-bit integer as it is an array of `{b0,b1,b2,b3}`.
static constexpr uint32_t bytepack32_4x8(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
  return ASMJIT_ARCH_LE ? (a | (b << 8) | (c << 16) | (d << 24))
                        : (d | (c << 8) | (b << 16) | (a << 24));
}

template<typename T>
static constexpr uint32_t unpackU32At0(T x) noexcept { return ASMJIT_ARCH_LE ? uint32_t(uint64_t(x) & 0xFFFFFFFFu) : uint32_t(uint64_t(x) >> 32); }
template<typename T>
static constexpr uint32_t unpackU32At1(T x) noexcept { return ASMJIT_ARCH_BE ? uint32_t(uint64_t(x) & 0xFFFFFFFFu) : uint32_t(uint64_t(x) >> 32); }

// ============================================================================
// [asmjit::Support - Position of byte (in bit-shift)]
// ============================================================================

static inline uint32_t byteShiftOfDWordStruct(uint32_t index) noexcept {
  return ASMJIT_ARCH_LE ? index * 8 : (uint32_t(sizeof(uint32_t)) - 1u - index) * 8;
}

// ============================================================================
// [asmjit::Support - String Utilities]
// ============================================================================

template<typename T>
static constexpr T asciiToLower(T c) noexcept { return c ^ (T(c >= T('A') && c <= T('Z')) << 5); }

template<typename T>
static constexpr T asciiToUpper(T c) noexcept { return c ^ (T(c >= T('a') && c <= T('z')) << 5); }

static ASMJIT_INLINE size_t strLen(const char* s, size_t maxSize) noexcept {
  size_t i = 0;
  while (i < maxSize && s[i] != '\0')
    i++;
  return i;
}

static constexpr uint32_t hashRound(uint32_t hash, uint32_t c) noexcept { return hash * 65599 + c; }

// Gets a hash of the given string `data` of size `size`. Size must be valid
// as this function doesn't check for a null terminator and allows it in the
// middle of the string.
static inline uint32_t hashString(const char* data, size_t size) noexcept {
  uint32_t hashCode = 0;
  for (uint32_t i = 0; i < size; i++)
    hashCode = hashRound(hashCode, uint8_t(data[i]));
  return hashCode;
}

static ASMJIT_INLINE const char* findPackedString(const char* p, uint32_t id) noexcept {
  uint32_t i = 0;
  while (i < id) {
    while (p[0])
      p++;
    p++;
    i++;
  }
  return p;
}

//! Compares two instruction names.
//!
//! `a` is a null terminated instruction name from arch-specific `nameData[]` table.
//! `b` is a non-null terminated instruction name passed to `Inst::idByName()`.
static ASMJIT_INLINE int cmpInstName(const char* a, const char* b, size_t size) noexcept {
  for (size_t i = 0; i < size; i++) {
    int c = int(uint8_t(a[i])) - int(uint8_t(b[i]));
    if (c != 0) return c;
  }
  return int(uint8_t(a[size]));
}

// ============================================================================
// [asmjit::Support - Read / Write]
// ============================================================================

static inline uint32_t readU8(const void* p) noexcept { return uint32_t(static_cast<const uint8_t*>(p)[0]); }
static inline int32_t readI8(const void* p) noexcept { return int32_t(static_cast<const int8_t*>(p)[0]); }

template<uint32_t BO, size_t Alignment>
static inline uint32_t readU16x(const void* p) noexcept {
  if (BO == ByteOrder::kNative && (kUnalignedAccess16 || Alignment >= 2)) {
    typedef typename Internal::AlignedInt<uint16_t, Alignment>::T U16AlignedToN;
    return uint32_t(static_cast<const U16AlignedToN*>(p)[0]);
  }
  else {
    uint32_t hi = readU8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 1 : 0));
    uint32_t lo = readU8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 0 : 1));
    return shl(hi, 8) | lo;
  }
}

template<uint32_t BO, size_t Alignment>
static inline int32_t readI16x(const void* p) noexcept {
  if (BO == ByteOrder::kNative && (kUnalignedAccess16 || Alignment >= 2)) {
    typedef typename Internal::AlignedInt<uint16_t, Alignment>::T U16AlignedToN;
    return int32_t(int16_t(static_cast<const U16AlignedToN*>(p)[0]));
  }
  else {
    int32_t hi = readI8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 1 : 0));
    uint32_t lo = readU8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 0 : 1));
    return shl(hi, 8) | int32_t(lo);
  }
}

template<uint32_t BO = ByteOrder::kNative>
static inline uint32_t readU24u(const void* p) noexcept {
  uint32_t b0 = readU8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 2 : 0));
  uint32_t b1 = readU8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 1 : 1));
  uint32_t b2 = readU8(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 0 : 2));
  return shl(b0, 16) | shl(b1, 8) | b2;
}

template<uint32_t BO, size_t Alignment>
static inline uint32_t readU32x(const void* p) noexcept {
  if (kUnalignedAccess32 || Alignment >= 4) {
    typedef typename Internal::AlignedInt<uint32_t, Alignment>::T U32AlignedToN;
    uint32_t x = static_cast<const U32AlignedToN*>(p)[0];
    return BO == ByteOrder::kNative ? x : byteswap32(x);
  }
  else {
    uint32_t hi = readU16x<BO, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 2 : 0));
    uint32_t lo = readU16x<BO, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 0 : 2));
    return shl(hi, 16) | lo;
  }
}

template<uint32_t BO, size_t Alignment>
static inline uint64_t readU64x(const void* p) noexcept {
  if (BO == ByteOrder::kNative && (kUnalignedAccess64 || Alignment >= 8)) {
    typedef typename Internal::AlignedInt<uint64_t, Alignment>::T U64AlignedToN;
    return static_cast<const U64AlignedToN*>(p)[0];
  }
  else {
    uint32_t hi = readU32x<BO, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 4 : 0));
    uint32_t lo = readU32x<BO, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<const uint8_t*>(p) + (BO == ByteOrder::kLE ? 0 : 4));
    return shl(uint64_t(hi), 32) | lo;
  }
}

template<uint32_t BO, size_t Alignment>
static inline int32_t readI32x(const void* p) noexcept { return int32_t(readU32x<BO, Alignment>(p)); }

template<uint32_t BO, size_t Alignment>
static inline int64_t readI64x(const void* p) noexcept { return int64_t(readU64x<BO, Alignment>(p)); }

template<size_t Alignment> static inline int32_t readI16xLE(const void* p) noexcept { return readI16x<ByteOrder::kLE, Alignment>(p); }
template<size_t Alignment> static inline int32_t readI16xBE(const void* p) noexcept { return readI16x<ByteOrder::kBE, Alignment>(p); }
template<size_t Alignment> static inline uint32_t readU16xLE(const void* p) noexcept { return readU16x<ByteOrder::kLE, Alignment>(p); }
template<size_t Alignment> static inline uint32_t readU16xBE(const void* p) noexcept { return readU16x<ByteOrder::kBE, Alignment>(p); }
template<size_t Alignment> static inline int32_t readI32xLE(const void* p) noexcept { return readI32x<ByteOrder::kLE, Alignment>(p); }
template<size_t Alignment> static inline int32_t readI32xBE(const void* p) noexcept { return readI32x<ByteOrder::kBE, Alignment>(p); }
template<size_t Alignment> static inline uint32_t readU32xLE(const void* p) noexcept { return readU32x<ByteOrder::kLE, Alignment>(p); }
template<size_t Alignment> static inline uint32_t readU32xBE(const void* p) noexcept { return readU32x<ByteOrder::kBE, Alignment>(p); }
template<size_t Alignment> static inline int64_t readI64xLE(const void* p) noexcept { return readI64x<ByteOrder::kLE, Alignment>(p); }
template<size_t Alignment> static inline int64_t readI64xBE(const void* p) noexcept { return readI64x<ByteOrder::kBE, Alignment>(p); }
template<size_t Alignment> static inline uint64_t readU64xLE(const void* p) noexcept { return readU64x<ByteOrder::kLE, Alignment>(p); }
template<size_t Alignment> static inline uint64_t readU64xBE(const void* p) noexcept { return readU64x<ByteOrder::kBE, Alignment>(p); }

static inline int32_t readI16a(const void* p) noexcept { return readI16x<ByteOrder::kNative, 2>(p); }
static inline int32_t readI16u(const void* p) noexcept { return readI16x<ByteOrder::kNative, 1>(p); }
static inline uint32_t readU16a(const void* p) noexcept { return readU16x<ByteOrder::kNative, 2>(p); }
static inline uint32_t readU16u(const void* p) noexcept { return readU16x<ByteOrder::kNative, 1>(p); }

static inline int32_t readI16aLE(const void* p) noexcept { return readI16xLE<2>(p); }
static inline int32_t readI16uLE(const void* p) noexcept { return readI16xLE<1>(p); }
static inline uint32_t readU16aLE(const void* p) noexcept { return readU16xLE<2>(p); }
static inline uint32_t readU16uLE(const void* p) noexcept { return readU16xLE<1>(p); }

static inline int32_t readI16aBE(const void* p) noexcept { return readI16xBE<2>(p); }
static inline int32_t readI16uBE(const void* p) noexcept { return readI16xBE<1>(p); }
static inline uint32_t readU16aBE(const void* p) noexcept { return readU16xBE<2>(p); }
static inline uint32_t readU16uBE(const void* p) noexcept { return readU16xBE<1>(p); }

static inline uint32_t readU24uLE(const void* p) noexcept { return readU24u<ByteOrder::kLE>(p); }
static inline uint32_t readU24uBE(const void* p) noexcept { return readU24u<ByteOrder::kBE>(p); }

static inline int32_t readI32a(const void* p) noexcept { return readI32x<ByteOrder::kNative, 4>(p); }
static inline int32_t readI32u(const void* p) noexcept { return readI32x<ByteOrder::kNative, 1>(p); }
static inline uint32_t readU32a(const void* p) noexcept { return readU32x<ByteOrder::kNative, 4>(p); }
static inline uint32_t readU32u(const void* p) noexcept { return readU32x<ByteOrder::kNative, 1>(p); }

static inline int32_t readI32aLE(const void* p) noexcept { return readI32xLE<4>(p); }
static inline int32_t readI32uLE(const void* p) noexcept { return readI32xLE<1>(p); }
static inline uint32_t readU32aLE(const void* p) noexcept { return readU32xLE<4>(p); }
static inline uint32_t readU32uLE(const void* p) noexcept { return readU32xLE<1>(p); }

static inline int32_t readI32aBE(const void* p) noexcept { return readI32xBE<4>(p); }
static inline int32_t readI32uBE(const void* p) noexcept { return readI32xBE<1>(p); }
static inline uint32_t readU32aBE(const void* p) noexcept { return readU32xBE<4>(p); }
static inline uint32_t readU32uBE(const void* p) noexcept { return readU32xBE<1>(p); }

static inline int64_t readI64a(const void* p) noexcept { return readI64x<ByteOrder::kNative, 8>(p); }
static inline int64_t readI64u(const void* p) noexcept { return readI64x<ByteOrder::kNative, 1>(p); }
static inline uint64_t readU64a(const void* p) noexcept { return readU64x<ByteOrder::kNative, 8>(p); }
static inline uint64_t readU64u(const void* p) noexcept { return readU64x<ByteOrder::kNative, 1>(p); }

static inline int64_t readI64aLE(const void* p) noexcept { return readI64xLE<8>(p); }
static inline int64_t readI64uLE(const void* p) noexcept { return readI64xLE<1>(p); }
static inline uint64_t readU64aLE(const void* p) noexcept { return readU64xLE<8>(p); }
static inline uint64_t readU64uLE(const void* p) noexcept { return readU64xLE<1>(p); }

static inline int64_t readI64aBE(const void* p) noexcept { return readI64xBE<8>(p); }
static inline int64_t readI64uBE(const void* p) noexcept { return readI64xBE<1>(p); }
static inline uint64_t readU64aBE(const void* p) noexcept { return readU64xBE<8>(p); }
static inline uint64_t readU64uBE(const void* p) noexcept { return readU64xBE<1>(p); }

static inline void writeU8(void* p, uint32_t x) noexcept { static_cast<uint8_t*>(p)[0] = uint8_t(x & 0xFFu); }
static inline void writeI8(void* p, int32_t x) noexcept { static_cast<uint8_t*>(p)[0] = uint8_t(x & 0xFF); }

template<uint32_t BO = ByteOrder::kNative, size_t Alignment = 1>
static inline void writeU16x(void* p, uint32_t x) noexcept {
  if (BO == ByteOrder::kNative && (kUnalignedAccess16 || Alignment >= 2)) {
    typedef typename Internal::AlignedInt<uint16_t, Alignment>::T U16AlignedToN;
    static_cast<U16AlignedToN*>(p)[0] = uint16_t(x & 0xFFFFu);
  }
  else {
    static_cast<uint8_t*>(p)[0] = uint8_t((x >> (BO == ByteOrder::kLE ? 0 : 8)) & 0xFFu);
    static_cast<uint8_t*>(p)[1] = uint8_t((x >> (BO == ByteOrder::kLE ? 8 : 0)) & 0xFFu);
  }
}

template<uint32_t BO = ByteOrder::kNative>
static inline void writeU24u(void* p, uint32_t v) noexcept {
  static_cast<uint8_t*>(p)[0] = uint8_t((v >> (BO == ByteOrder::kLE ?  0 : 16)) & 0xFFu);
  static_cast<uint8_t*>(p)[1] = uint8_t((v >> (BO == ByteOrder::kLE ?  8 :  8)) & 0xFFu);
  static_cast<uint8_t*>(p)[2] = uint8_t((v >> (BO == ByteOrder::kLE ? 16 :  0)) & 0xFFu);
}

template<uint32_t BO = ByteOrder::kNative, size_t Alignment = 1>
static inline void writeU32x(void* p, uint32_t x) noexcept {
  if (kUnalignedAccess32 || Alignment >= 4) {
    typedef typename Internal::AlignedInt<uint32_t, Alignment>::T U32AlignedToN;
    static_cast<U32AlignedToN*>(p)[0] = (BO == ByteOrder::kNative) ? x : Support::byteswap32(x);
  }
  else {
    writeU16x<BO, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<uint8_t*>(p) + 0, x >> (BO == ByteOrder::kLE ?  0 : 16));
    writeU16x<BO, Alignment >= 2 ? size_t(2) : Alignment>(static_cast<uint8_t*>(p) + 2, x >> (BO == ByteOrder::kLE ? 16 :  0));
  }
}

template<uint32_t BO = ByteOrder::kNative, size_t Alignment = 1>
static inline void writeU64x(void* p, uint64_t x) noexcept {
  if (BO == ByteOrder::kNative && (kUnalignedAccess64 || Alignment >= 8)) {
    typedef typename Internal::AlignedInt<uint64_t, Alignment>::T U64AlignedToN;
    static_cast<U64AlignedToN*>(p)[0] = x;
  }
  else {
    writeU32x<BO, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<uint8_t*>(p) + 0, uint32_t((x >> (BO == ByteOrder::kLE ?  0 : 32)) & 0xFFFFFFFFu));
    writeU32x<BO, Alignment >= 4 ? size_t(4) : Alignment>(static_cast<uint8_t*>(p) + 4, uint32_t((x >> (BO == ByteOrder::kLE ? 32 :  0)) & 0xFFFFFFFFu));
  }
}

template<uint32_t BO = ByteOrder::kNative, size_t Alignment = 1> static inline void writeI16x(void* p, int32_t x) noexcept { writeU16x<BO, Alignment>(p, uint32_t(x)); }
template<uint32_t BO = ByteOrder::kNative, size_t Alignment = 1> static inline void writeI32x(void* p, int32_t x) noexcept { writeU32x<BO, Alignment>(p, uint32_t(x)); }
template<uint32_t BO = ByteOrder::kNative, size_t Alignment = 1> static inline void writeI64x(void* p, int64_t x) noexcept { writeU64x<BO, Alignment>(p, uint64_t(x)); }

template<size_t Alignment = 1> static inline void writeI16xLE(void* p, int32_t x) noexcept { writeI16x<ByteOrder::kLE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeI16xBE(void* p, int32_t x) noexcept { writeI16x<ByteOrder::kBE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeU16xLE(void* p, uint32_t x) noexcept { writeU16x<ByteOrder::kLE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeU16xBE(void* p, uint32_t x) noexcept { writeU16x<ByteOrder::kBE, Alignment>(p, x); }

template<size_t Alignment = 1> static inline void writeI32xLE(void* p, int32_t x) noexcept { writeI32x<ByteOrder::kLE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeI32xBE(void* p, int32_t x) noexcept { writeI32x<ByteOrder::kBE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeU32xLE(void* p, uint32_t x) noexcept { writeU32x<ByteOrder::kLE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeU32xBE(void* p, uint32_t x) noexcept { writeU32x<ByteOrder::kBE, Alignment>(p, x); }

template<size_t Alignment = 1> static inline void writeI64xLE(void* p, int64_t x) noexcept { writeI64x<ByteOrder::kLE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeI64xBE(void* p, int64_t x) noexcept { writeI64x<ByteOrder::kBE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeU64xLE(void* p, uint64_t x) noexcept { writeU64x<ByteOrder::kLE, Alignment>(p, x); }
template<size_t Alignment = 1> static inline void writeU64xBE(void* p, uint64_t x) noexcept { writeU64x<ByteOrder::kBE, Alignment>(p, x); }

static inline void writeI16a(void* p, int32_t x) noexcept { writeI16x<ByteOrder::kNative, 2>(p, x); }
static inline void writeI16u(void* p, int32_t x) noexcept { writeI16x<ByteOrder::kNative, 1>(p, x); }
static inline void writeU16a(void* p, uint32_t x) noexcept { writeU16x<ByteOrder::kNative, 2>(p, x); }
static inline void writeU16u(void* p, uint32_t x) noexcept { writeU16x<ByteOrder::kNative, 1>(p, x); }

static inline void writeI16aLE(void* p, int32_t x) noexcept { writeI16xLE<2>(p, x); }
static inline void writeI16uLE(void* p, int32_t x) noexcept { writeI16xLE<1>(p, x); }
static inline void writeU16aLE(void* p, uint32_t x) noexcept { writeU16xLE<2>(p, x); }
static inline void writeU16uLE(void* p, uint32_t x) noexcept { writeU16xLE<1>(p, x); }

static inline void writeI16aBE(void* p, int32_t x) noexcept { writeI16xBE<2>(p, x); }
static inline void writeI16uBE(void* p, int32_t x) noexcept { writeI16xBE<1>(p, x); }
static inline void writeU16aBE(void* p, uint32_t x) noexcept { writeU16xBE<2>(p, x); }
static inline void writeU16uBE(void* p, uint32_t x) noexcept { writeU16xBE<1>(p, x); }

static inline void writeU24uLE(void* p, uint32_t v) noexcept { writeU24u<ByteOrder::kLE>(p, v); }
static inline void writeU24uBE(void* p, uint32_t v) noexcept { writeU24u<ByteOrder::kBE>(p, v); }

static inline void writeI32a(void* p, int32_t x) noexcept { writeI32x<ByteOrder::kNative, 4>(p, x); }
static inline void writeI32u(void* p, int32_t x) noexcept { writeI32x<ByteOrder::kNative, 1>(p, x); }
static inline void writeU32a(void* p, uint32_t x) noexcept { writeU32x<ByteOrder::kNative, 4>(p, x); }
static inline void writeU32u(void* p, uint32_t x) noexcept { writeU32x<ByteOrder::kNative, 1>(p, x); }

static inline void writeI32aLE(void* p, int32_t x) noexcept { writeI32xLE<4>(p, x); }
static inline void writeI32uLE(void* p, int32_t x) noexcept { writeI32xLE<1>(p, x); }
static inline void writeU32aLE(void* p, uint32_t x) noexcept { writeU32xLE<4>(p, x); }
static inline void writeU32uLE(void* p, uint32_t x) noexcept { writeU32xLE<1>(p, x); }

static inline void writeI32aBE(void* p, int32_t x) noexcept { writeI32xBE<4>(p, x); }
static inline void writeI32uBE(void* p, int32_t x) noexcept { writeI32xBE<1>(p, x); }
static inline void writeU32aBE(void* p, uint32_t x) noexcept { writeU32xBE<4>(p, x); }
static inline void writeU32uBE(void* p, uint32_t x) noexcept { writeU32xBE<1>(p, x); }

static inline void writeI64a(void* p, int64_t x) noexcept { writeI64x<ByteOrder::kNative, 8>(p, x); }
static inline void writeI64u(void* p, int64_t x) noexcept { writeI64x<ByteOrder::kNative, 1>(p, x); }
static inline void writeU64a(void* p, uint64_t x) noexcept { writeU64x<ByteOrder::kNative, 8>(p, x); }
static inline void writeU64u(void* p, uint64_t x) noexcept { writeU64x<ByteOrder::kNative, 1>(p, x); }

static inline void writeI64aLE(void* p, int64_t x) noexcept { writeI64xLE<8>(p, x); }
static inline void writeI64uLE(void* p, int64_t x) noexcept { writeI64xLE<1>(p, x); }
static inline void writeU64aLE(void* p, uint64_t x) noexcept { writeU64xLE<8>(p, x); }
static inline void writeU64uLE(void* p, uint64_t x) noexcept { writeU64xLE<1>(p, x); }

static inline void writeI64aBE(void* p, int64_t x) noexcept { writeI64xBE<8>(p, x); }
static inline void writeI64uBE(void* p, int64_t x) noexcept { writeI64xBE<1>(p, x); }
static inline void writeU64aBE(void* p, uint64_t x) noexcept { writeU64xBE<8>(p, x); }
static inline void writeU64uBE(void* p, uint64_t x) noexcept { writeU64xBE<1>(p, x); }

// ============================================================================
// [asmjit::Support - Operators]
// ============================================================================

struct Set    { template<typename T> static inline T op(T x, T y) noexcept { ASMJIT_UNUSED(x); return  y; } };
struct SetNot { template<typename T> static inline T op(T x, T y) noexcept { ASMJIT_UNUSED(x); return ~y; } };
struct And    { template<typename T> static inline T op(T x, T y) noexcept { return  x &  y; } };
struct AndNot { template<typename T> static inline T op(T x, T y) noexcept { return  x & ~y; } };
struct NotAnd { template<typename T> static inline T op(T x, T y) noexcept { return ~x &  y; } };
struct Or     { template<typename T> static inline T op(T x, T y) noexcept { return  x |  y; } };
struct Xor    { template<typename T> static inline T op(T x, T y) noexcept { return  x ^  y; } };
struct Add    { template<typename T> static inline T op(T x, T y) noexcept { return  x +  y; } };
struct Sub    { template<typename T> static inline T op(T x, T y) noexcept { return  x -  y; } };
struct Min    { template<typename T> static inline T op(T x, T y) noexcept { return min<T>(x, y); } };
struct Max    { template<typename T> static inline T op(T x, T y) noexcept { return max<T>(x, y); } };

// ============================================================================
// [asmjit::Support - BitWordIterator]
// ============================================================================

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
  inline explicit BitWordIterator(T bitWord) noexcept
    : _bitWord(bitWord) {}

  inline void init(T bitWord) noexcept { _bitWord = bitWord; }
  inline bool hasNext() const noexcept { return _bitWord != 0; }

  inline uint32_t next() noexcept {
    ASMJIT_ASSERT(_bitWord != 0);
    uint32_t index = ctz(_bitWord);
    _bitWord ^= T(1u) << index;
    return index;
  }

  T _bitWord;
};

// ============================================================================
// [asmjit::Support - BitVectorOps]
// ============================================================================

namespace Internal {
  template<typename T, class OperatorT, class FullWordOpT>
  static inline void bitVectorOp(T* buf, size_t index, size_t count) noexcept {
    if (count == 0)
      return;

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
    if (count)
      buf[0] = OperatorT::op(buf[0], kFillMask >> (kTSizeInBits - count));
  }
}

//! Sets bit in a bit-vector `buf` at `index`.
template<typename T>
static inline bool bitVectorGetBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  return bool((buf[vecIndex] >> bitIndex) & 0x1u);
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static inline void bitVectorSetBit(T* buf, size_t index, bool value) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1u) << bitIndex;
  if (value)
    buf[vecIndex] |= bitMask;
  else
    buf[vecIndex] &= ~bitMask;
}

//! Sets bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static inline void bitVectorFlipBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1u) << bitIndex;
  buf[vecIndex] ^= bitMask;
}

//! Fills `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static inline void bitVectorFill(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, Or, Set>(buf, index, count); }

//! Clears `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static inline void bitVectorClear(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, AndNot, SetNot>(buf, index, count); }

template<typename T>
static inline size_t bitVectorIndexOf(T* buf, size_t start, bool value) noexcept {
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
    if (bits)
      return (size_t)(p - buf) * kTSizeInBits + ctz(bits);
    bits = *++p ^ kFlipMask;
  }
}

// ============================================================================
// [asmjit::Support - BitVectorIterator]
// ============================================================================

template<typename T>
class BitVectorIterator {
public:
  ASMJIT_INLINE BitVectorIterator(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    init(data, numBitWords, start);
  }

  ASMJIT_INLINE void init(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    const T* ptr = data + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = *ptr++ & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += bitSizeOf<T>()) < end)
        bitWord = *ptr++;
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bitWord;
  }

  ASMJIT_INLINE bool hasNext() const noexcept {
    return _current != T(0);
  }

  ASMJIT_INLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1u) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++;

    _current = bitWord;
    return n;
  }

  ASMJIT_INLINE size_t peekNext() const noexcept {
    ASMJIT_ASSERT(_current != T(0));
    return _idx + ctz(_current);
  }

  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
};

// ============================================================================
// [asmjit::Support - BitVectorOpIterator]
// ============================================================================

template<typename T, class OperatorT>
class BitVectorOpIterator {
public:
  static constexpr uint32_t kTSizeInBits = bitSizeOf<T>();

  ASMJIT_INLINE BitVectorOpIterator(const T* aData, const T* bData, size_t numBitWords, size_t start = 0) noexcept {
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
      while (!bitWord && (idx += kTSizeInBits) < end)
        bitWord = OperatorT::op(*aPtr++, *bPtr++);
    }

    _aPtr = aPtr;
    _bPtr = bPtr;
    _idx = idx;
    _end = end;
    _current = bitWord;
  }

  ASMJIT_INLINE bool hasNext() noexcept {
    return _current != T(0);
  }

  ASMJIT_INLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1u) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += kTSizeInBits) < _end)
      bitWord = OperatorT::op(*_aPtr++, *_bPtr++);

    _current = bitWord;
    return n;
  }

  const T* _aPtr;
  const T* _bPtr;
  size_t _idx;
  size_t _end;
  T _current;
};

// ============================================================================
// [asmjit::Support - Sorting]
// ============================================================================

//! Sort order.
enum SortOrder : uint32_t {
  kSortAscending  = 0, //!< Ascending.
  kSortDescending = 1  //!< Descending.
};

//! A helper class that provides comparison of any user-defined type that
//! implements `<` and `>` operators (primitive types are supported as well).
template<uint32_t Order = kSortAscending>
struct Compare {
  template<typename A, typename B>
  inline int operator()(const A& a, const B& b) const noexcept {
    return (Order == kSortAscending) ? (a < b ? -1 : a > b ?  1 : 0)
                                     : (a < b ?  1 : a > b ? -1 : 0);
  }
};

//! Insertion sort.
template<typename T, typename CompareT = Compare<kSortAscending>>
static inline void iSort(T* base, size_t size, const CompareT& cmp = CompareT()) noexcept {
  for (T* pm = base + 1; pm < base + size; pm++)
    for (T* pl = pm; pl > base && cmp(pl[-1], pl[0]) > 0; pl--)
      std::swap(pl[-1], pl[0]);
}

namespace Internal {
  //! Quick-sort implementation.
  template<typename T, class CompareT>
  struct QSortImpl {
    static constexpr size_t kStackSize = 64 * 2;
    static constexpr size_t kISortThreshold = 7;

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

          if (cmp(*pi  , *pj  ) > 0) std::swap(*pi  , *pj  );
          if (cmp(*base, *pj  ) > 0) std::swap(*base, *pj  );
          if (cmp(*pi  , *base) > 0) std::swap(*pi  , *base);

          // Now we have the median for pivot element, entering main loop.
          for (;;) {
            while (pi < pj   && cmp(*++pi, *base) < 0) continue; // Move `i` right until `*i >= pivot`.
            while (pj > base && cmp(*--pj, *base) > 0) continue; // Move `j` left  until `*j <= pivot`.

            if (pi > pj) break;
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
          iSort(base, (size_t)(end - base), cmp);
          if (stackptr == stack)
            break;
          end = *--stackptr;
          base = *--stackptr;
        }
      }
    }
  };
}

//! Quick sort implementation.
//!
//! The main reason to provide a custom qsort implementation is that we needed
//! something that will never throw `bad_alloc` exception. This implementation
//! doesn't use dynamic memory allocation.
template<typename T, class CompareT = Compare<kSortAscending>>
static inline void qSort(T* base, size_t size, const CompareT& cmp = CompareT()) noexcept {
  Internal::QSortImpl<T, CompareT>::sort(base, size, cmp);
}

// ============================================================================
// [asmjit::Support - Iterators]
// ============================================================================

template<typename T>
class Iterator {
public:
  constexpr Iterator(T* p) noexcept : _p(p) {}
  constexpr Iterator(const Iterator& other) noexcept = default;

  inline Iterator& operator=(const Iterator& other) noexcept = default;

  inline Iterator operator+(size_t n) const noexcept { return Iterator(_p + n); }
  inline Iterator operator-(size_t n) const noexcept { return Iterator(_p - n); }

  inline Iterator& operator+=(size_t n) noexcept { _p += n; return *this; }
  inline Iterator& operator-=(size_t n) noexcept { _p -= n; return *this; }

  inline Iterator& operator++() noexcept { return operator+=(1); }
  inline Iterator& operator--() noexcept { return operator-=(1); }

  inline Iterator operator++(int) noexcept { T* prev = _p; operator+=(1); return Iterator(prev); }
  inline Iterator operator--(int) noexcept { T* prev = _p; operator-=(1); return Iterator(prev); }

  inline bool operator==(const Iterator& other) noexcept { return _p == other._p; }
  inline bool operator!=(const Iterator& other) noexcept { return _p != other._p; }

  inline T& operator*() const noexcept { return _p[0]; }

  T* _p;
};

template<typename T>
class ReverseIterator {
public:
  constexpr ReverseIterator(T* p) noexcept : _p(p) {}
  constexpr ReverseIterator(const ReverseIterator& other) noexcept = default;

  inline ReverseIterator& operator=(const ReverseIterator& other) noexcept = default;

  inline ReverseIterator operator+(size_t n) const noexcept { return ReverseIterator(_p + n); }
  inline ReverseIterator operator-(size_t n) const noexcept { return ReverseIterator(_p - n); }

  inline ReverseIterator& operator+=(size_t n) noexcept { _p -= n; return *this; }
  inline ReverseIterator& operator-=(size_t n) noexcept { _p += n; return *this; }

  inline ReverseIterator& operator++() noexcept { return operator+=(1); }
  inline ReverseIterator& operator--() noexcept { return operator-=(1); }

  inline ReverseIterator operator++(int) noexcept { T* prev = _p; operator+=(1); return ReverseIterator(prev); }
  inline ReverseIterator operator--(int) noexcept { T* prev = _p; operator-=(1); return ReverseIterator(prev); }

  inline bool operator==(const ReverseIterator& other) noexcept { return _p == other._p; }
  inline bool operator!=(const ReverseIterator& other) noexcept { return _p != other._p; }

  inline T& operator*() const noexcept { return _p[-1]; }

  T* _p;
};

// ============================================================================
// [asmjit::Support::Temporary]
// ============================================================================

//! Used to pass a temporary buffer to:
//!
//!   - Containers that use user-passed buffer as an initial storage (still can grow).
//!   - Zone allocator that would use the temporary buffer as a first block.
struct Temporary {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  constexpr Temporary(const Temporary& other) noexcept = default;
  constexpr Temporary(void* data, size_t size) noexcept
    : _data(data),
      _size(size) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets the storage.
  template<typename T = void>
  constexpr T* data() const noexcept { return static_cast<T*>(_data); }
  //! Gets the storage size (capacity).
  constexpr size_t size() const noexcept { return _size; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline Temporary& operator=(const Temporary& other) noexcept = default;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  void* _data;
  size_t _size;
};

} // {Support}

//! \}
//! \endcond

ASMJIT_END_NAMESPACE

// ============================================================================
// [asmjit::Support - Placement New / Delete]
// ============================================================================

//! Implementation of placement new so we don't have to depend on `<new>`.
inline void* operator new(std::size_t, const asmjit::Support::PlacementNew& p) { return p.ptr; }
//! Implementation of placement delete so we don't have to depend on `<new>`.
inline void operator delete(void*, const asmjit::Support::PlacementNew&) noexcept {}

#endif // _ASMJIT_CORE_SUPPORT_H
