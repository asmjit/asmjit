// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_INTUTILS_H
#define _ASMJIT_CORE_INTUTILS_H

// [Dependencies]
#include "../core/globals.h"

#if ASMJIT_CXX_MSC
  #include <intrin.h>
#endif

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

//! Utilities related to integers and bitwords.
namespace IntUtils {

// ============================================================================
// [asmjit::IntUtils::IntBySize / Int32Or64]
// ============================================================================

//! \internal
namespace Internal {
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

//! Cast an integer `x` to either `int32_t`, uint32_t`, `int64_t`, or `uint64_t` depending on `T`.
template<typename T>
constexpr typename Internal::Int32Or64<T>::Type asNormalized(T x) noexcept { return (typename Internal::Int32Or64<T>::Type)x; }

//! Cast an integer `x` to either `int32_t` or `int64_t` depending on `T`.
template<typename T>
constexpr typename Internal::Int32Or64<T, 1>::Type asInt(T x) noexcept { return (typename Internal::Int32Or64<T, 1>::Type)x; }

//! Cast an integer `x` to either `uint32_t` or `uint64_t` depending on `T`.
template<typename T>
constexpr typename Internal::Int32Or64<T, 0>::Type asUInt(T x) noexcept { return (typename Internal::Int32Or64<T, 0>::Type)x; }

// ============================================================================
// [asmjit::IntUtils::BitCast]
// ============================================================================

//! \internal
namespace Internal {
  template<typename SRC, typename DST>
  union BitCast {
    constexpr BitCast(SRC src) noexcept : src(src) {}
    SRC src;
    DST dst;
  };
}

//! Bit-cast `SRC` to `DST`.
//!
//! Useful to bitcast between integer and floating point.
template<typename DST, typename SRC>
constexpr DST bit_cast(const SRC& x) noexcept { return Internal::BitCast<DST, SRC>(x).dst; }

// ============================================================================
// [asmjit::IntUtils::BitSizeOf]
// ============================================================================

template<typename T>
constexpr uint32_t bitSizeOf() noexcept { return uint32_t(sizeof(T) * 8U); }

// ============================================================================
// [asmjit::IntUtils::BitUtilities]
// ============================================================================

//! Returns `0 - x` in a safe way (no undefined behavior), works for unsigned numbers as well.
template<typename T>
constexpr T neg(const T& x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(0) - U(x));
}

template<typename T>
constexpr T allOnes() noexcept { return neg<T>(T(1)); }

//! Returns `x << y` (shift left logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
constexpr X shl(const X& x, const Y& y) noexcept {
  typedef typename std::make_unsigned<X>::type U;
  return X(U(x) << y);
}

//! Returns `x >> y` (shift right logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
constexpr X shr(const X& x, const Y& y) noexcept {
  typedef typename std::make_unsigned<X>::type U;
  return X(U(x) >> y);
}

//! Returns `x >> y` (shift right arithmetic) by explicitly casting `x` to a signed type and back.
template<typename X, typename Y>
constexpr X sar(const X& x, const Y& y) noexcept {
  typedef typename std::make_signed<X>::type S;
  return X(S(x) >> y);
}

//! Returns `x | (x >> y)` - helper used by some bit manipulation helpers.
template<typename X, typename Y>
constexpr X or_shr(const X& x, const Y& y) noexcept { return X(x | shr(x, y)); }

//! Returns `x & -x` - extracts lowest set isolated bit (like BLSI instruction).
template<typename T>
constexpr T blsi(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(x) & neg(U(x)));
}

//! Returns `x & (x - 1)` - resets lowest set bit (like BLSR instruction).
template<typename T>
constexpr T blsr(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(x) & (U(x) - U(1)));
}

//! Generate a trailing bit-mask that has `n` least significant (trailing) bits set.
template<typename T, typename COUNT>
constexpr T lsbMask(COUNT n) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return (sizeof(U) < sizeof(uintptr_t))
    ? T(U((uintptr_t(1) << n) - uintptr_t(1)))
    // Shifting more bits than the type provides is UNDEFINED BEHAVIOR.
    // In such case we trash the result by ORing it with a mask that has
    // all bits set and discards the UNDEFINED RESULT of the shift.
    : T(((U(1) << n) - U(1U)) | neg(U(n >= COUNT(bitSizeOf<T>()))));
}

//! Get whether `x` has Nth bit set.
template<typename T, typename INDEX>
constexpr bool bitTest(T x, INDEX n) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return (U(x) & (U(1) << n)) != 0;
}

//! Get whether the `x` is a power of two (only one bit is set).
template<typename T>
constexpr bool isPowerOf2(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return x && !(U(x) & (U(x) - U(1)));
}

//! Return a bit-mask that has `x` bit set.
template<typename T>
constexpr uint32_t mask(T x) noexcept { return (1U << x); }

//! Return a bit-mask that has `x` bit set (multiple arguments).
template<typename T, typename... Args>
constexpr uint32_t mask(T x, Args... args) noexcept { return mask(x) | mask(args...); }

//! Convert a boolean value `b` to zero or full mask (all bits set).
template<typename DST, typename SRC>
constexpr DST maskFromBool(SRC b) noexcept {
  typedef typename std::make_unsigned<DST>::type U;
  return DST(U(0) - U(b));
}

//! \internal
namespace Internal {
  // Fill all trailing bits right from the first most significant bit set.
  constexpr uint8_t fillTrailingBitsImpl(uint8_t x) noexcept { return or_shr(or_shr(or_shr(x, 1), 2), 4); }
  // Fill all trailing bits right from the first most significant bit set.
  constexpr uint16_t fillTrailingBitsImpl(uint16_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8); }
  // Fill all trailing bits right from the first most significant bit set.
  constexpr uint32_t fillTrailingBitsImpl(uint32_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8), 16); }
  // Fill all trailing bits right from the first most significant bit set.
  constexpr uint64_t fillTrailingBitsImpl(uint64_t x) noexcept { return or_shr(or_shr(or_shr(or_shr(or_shr(or_shr(x, 1), 2), 4), 8), 16), 32); }
}

// Fill all trailing bits right from the first most significant bit set.
template<typename T>
constexpr T fillTrailingBits(const T& x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(Internal::fillTrailingBitsImpl(U(x)));
}

// ============================================================================
// [asmjit::IntUtils::CTZ]
// ============================================================================

//! \internal
namespace Internal {
  constexpr uint32_t ctzGenericImpl(uint32_t xAndNegX) noexcept {
    return 31 - ((xAndNegX & 0x0000FFFFU) ? 16 : 0)
              - ((xAndNegX & 0x00FF00FFU) ?  8 : 0)
              - ((xAndNegX & 0x0F0F0F0FU) ?  4 : 0)
              - ((xAndNegX & 0x33333333U) ?  2 : 0)
              - ((xAndNegX & 0x55555555U) ?  1 : 0);
  }

  constexpr uint32_t ctzGenericImpl(uint64_t xAndNegX) noexcept {
    return 63 - ((xAndNegX & 0x00000000FFFFFFFFU) ? 32 : 0)
              - ((xAndNegX & 0x0000FFFF0000FFFFU) ? 16 : 0)
              - ((xAndNegX & 0x00FF00FF00FF00FFU) ?  8 : 0)
              - ((xAndNegX & 0x0F0F0F0F0F0F0F0FU) ?  4 : 0)
              - ((xAndNegX & 0x3333333333333333U) ?  2 : 0)
              - ((xAndNegX & 0x5555555555555555U) ?  1 : 0);
  }

  template<typename T>
  constexpr T ctzGeneric(T x) noexcept {
    return ctzGenericImpl(x & neg(x));
  }

  static ASMJIT_FORCEINLINE uint32_t ctz(uint32_t x) noexcept {
    #if ASMJIT_CXX_MSC && (ASMJIT_ARCH_X86 || ASMJIT_ARCH_ARM)
    unsigned long i;
    _BitScanForward(&i, x);
    return uint32_t(i);
    #elif ASMJIT_CXX_GNU
    return uint32_t(__builtin_ctz(x));
    #else
    return ctzGeneric(x);
    #endif
  }

  static ASMJIT_FORCEINLINE uint32_t ctz(uint64_t x) noexcept {
    #if ASMJIT_CXX_MSC && (ASMJIT_ARCH_X86 == 64 || ASMJIT_ARCH_ARM == 64)
    unsigned long i;
    _BitScanForward64(&i, x);
    return uint32_t(i);
    #elif ASMJIT_CXX_GNU
    return uint32_t(__builtin_ctzll(x));
    #else
    return ctzGeneric(x);
    #endif
  }
}

//! Count trailing zeros in `x` (returns a position of a first bit set in `x`).
//!
//! NOTE: The input MUST NOT be zero, otherwise the result is undefined.
template<typename T>
static inline uint32_t ctz(T x) noexcept { return Internal::ctz(asUInt(x)); }

template<uint64_t N>
struct StaticCtz {
  enum {
    _kTmp1 = 0      + (((N            ) & uint64_t(0xFFFFFFFFU)) == 0 ? 32 : 0),
    _kTmp2 = _kTmp1 + (((N >> (_kTmp1)) & uint64_t(0x0000FFFFU)) == 0 ? 16 : 0),
    _kTmp3 = _kTmp2 + (((N >> (_kTmp2)) & uint64_t(0x000000FFU)) == 0 ?  8 : 0),
    _kTmp4 = _kTmp3 + (((N >> (_kTmp3)) & uint64_t(0x0000000FU)) == 0 ?  4 : 0),
    _kTmp5 = _kTmp4 + (((N >> (_kTmp4)) & uint64_t(0x00000003U)) == 0 ?  2 : 0),
    kValue = _kTmp5 + (((N >> (_kTmp5)) & uint64_t(0x00000001U)) == 0 ?  1 : 0)
  };
};

template<>
struct StaticCtz<0> {}; // Undefined.

template<uint64_t N>
constexpr uint32_t staticCtz() noexcept { return StaticCtz<N>::kValue; }

// ============================================================================
// [asmjit::IntUtils::Popcnt]
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

//! \internal
static inline uint32_t _popcntGeneric(uint32_t x) noexcept {
  x = x - ((x >> 1) & 0x55555555U);
  x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
  return (((x + (x >> 4)) & 0x0F0F0F0FU) * 0x01010101U) >> 24;
}

//! \internal
static inline uint32_t _popcntGeneric(uint64_t x) noexcept {
  if (ASMJIT_ARCH_BITS == 64) {
    x = x - ((x >> 1) & 0x5555555555555555U);
    x = (x & 0x3333333333333333U) + ((x >> 2) & 0x3333333333333333U);
    return uint32_t((((x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FU) * 0x0101010101010101U) >> 56);
  }
  else {
    return _popcntGeneric(uint32_t(x >> 32)) +
           _popcntGeneric(uint32_t(x & 0xFFFFFFFFU));
  }
}

//! \internal
static inline uint32_t _popcntImpl(uint32_t x) noexcept {
  #if ASMJIT_CXX_GNU
  return uint32_t(__builtin_popcount(x));
  #else
  return _popcntGeneric(asUInt(x));
  #endif
}

//! \internal
static inline uint32_t _popcntImpl(uint64_t x) noexcept {
  #if ASMJIT_CXX_GNU
  return uint32_t(__builtin_popcountll(x));
  #else
  return _popcntGeneric(asUInt(x));
  #endif
}

//! Get count of bits in `x`.
template<typename T>
static inline uint32_t popcnt(T x) noexcept { return _popcntImpl(asUInt(x)); }

// ============================================================================
// [asmjit::IntUtils::SignExtend]
// ============================================================================

template<typename T>
constexpr T signExtendI8(T imm) noexcept { return int64_t(int8_t(imm & T(0xFF))); }

template<typename T>
constexpr T signExtendI16(T imm) noexcept { return int64_t(int16_t(imm & T(0xFFFF))); }

template<typename T>
constexpr T signExtendI32(T imm) noexcept { return int64_t(int32_t(imm & T(0xFFFFFFFF))); }

// ============================================================================
// [asmjit::IntUtils::Alignment]
// ============================================================================

template<typename X, typename Y>
constexpr bool isAligned(X base, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return ((U)base % (U)alignment) == 0;
}

template<typename X, typename Y>
constexpr X alignUp(X x, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return (X)( ((U)x + ((U)(alignment) - 1U)) & ~((U)(alignment) - 1U) );
}

template<typename X, typename Y>
constexpr X alignDown(X x, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return (X)( (U)x & ~((U)(alignment) - 1U) );
}

//! Get zero or a positive difference between `base` and `base` when aligned to `alignment`.
template<typename X, typename Y>
constexpr typename Internal::IntBySize<sizeof(X), 0>::Type alignUpDiff(X base, Y alignment) noexcept {
  typedef typename Internal::IntBySize<sizeof(X), 0>::Type U;
  return alignUp(U(base), alignment) - U(base);
}

template<typename T>
constexpr T alignUpPowerOf2(T x) noexcept {
  typedef typename Internal::IntBySize<sizeof(T), 0>::Type U;
  return (T)(fillTrailingBits(U(x) - 1U) + 1U);
}

// ============================================================================
// [asmjit::IntUtils::IsBetween]
// ============================================================================

//! Get whether `x` is greater than or equal to `a` and lesses than or equal to `b`.
template<typename T>
constexpr bool isBetween(const T& x, const T& a, const T& b) noexcept {
  return x >= a && x <= b;
}

// ============================================================================
// [asmjit::IntUtils::IsInt / IsUInt]
// ============================================================================

//! Get whether the given integer `x` can be casted to a 4-bit signed integer.
template<typename T>
constexpr bool isInt4(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? isBetween<S>(S(x), -8, 7)
                                  : U(x) <= U(7U);
}

//! Get whether the given integer `x` can be casted to an 8-bit signed integer.
template<typename T>
constexpr bool isInt8(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? sizeof(T) <= 1 || isBetween<S>(S(x), -128, 127)
                                  : U(x) <= U(127U);
}

//! Get whether the given integer `x` can be casted to a 16-bit signed integer.
template<typename T>
constexpr bool isInt16(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? sizeof(T) <= 2 || isBetween<S>(S(x), -32768, 32767)
                                  : sizeof(T) <= 1 || U(x) <= U(32767U);
}

//! Get whether the given integer `x` can be casted to a 32-bit signed integer.
template<typename T>
constexpr bool isInt32(T x) noexcept {
  typedef typename std::make_signed<T>::type S;
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? sizeof(T) <= 4 || isBetween<S>(S(x), -2147483647 - 1, 2147483647)
                                  : sizeof(T) <= 2 || U(x) <= U(2147483647U);
}

//! Get whether the given integer `x` can be casted to a 4-bit unsigned integer.
template<typename T>
constexpr bool isUInt4(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? x >= T(0) && x <= T(15)
                                  : U(x) <= U(15U);
}

//! Get whether the given integer `x` can be casted to an 8-bit unsigned integer.
template<typename T>
constexpr bool isUInt8(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 1 || T(x) <= T(255)) && x >= T(0)
                                  : (sizeof(T) <= 1 || U(x) <= U(255U));
}

//! Get whether the given integer `x` can be casted to a 12-bit unsigned integer (ARM specific).
template<typename T>
constexpr bool isUInt12(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 1 || T(x) <= T(4095)) && x >= T(0)
                                  : (sizeof(T) <= 1 || U(x) <= U(4095U));
}

//! Get whether the given integer `x` can be casted to a 16-bit unsigned integer.
template<typename T>
constexpr bool isUInt16(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 2 || T(x) <= T(65535)) && x >= T(0)
                                  : (sizeof(T) <= 2 || U(x) <= U(65535U));
}

//! Get whether the given integer `x` can be casted to a 32-bit unsigned integer.
template<typename T>
constexpr bool isUInt32(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;

  return std::is_signed<T>::value ? (sizeof(T) <= 4 || T(x) <= T(4294967295U)) && x >= T(0)
                                  : (sizeof(T) <= 4 || U(x) <= U(4294967295U));
}

// ============================================================================
// [asmjit::IntUtils::ByteSwap]
// ============================================================================

static inline uint32_t byteswap32(uint32_t x) noexcept {
  #if ASMJIT_CXX_MSC
    return uint32_t(_byteswap_ulong(x));
  #elif ASMJIT_CXX_GNU
    return __builtin_bswap32(x);
  #else
    return (x << 24) | (x >> 24) | ((x << 8) & 0x00FF0000U) | ((x >> 8) & 0x0000FF00);
  #endif
}

// ============================================================================
// [asmjit::IntUtils::BytePack / Unpack]
// ============================================================================

//! Pack four 8-bit integer into a 32-bit integer as it is an array of `{b0,b1,b2,b3}`.
constexpr uint32_t bytepack32_4x8(uint32_t a, uint32_t b, uint32_t c, uint32_t d) noexcept {
  return ASMJIT_ARCH_LE ? (a | (b << 8) | (c << 16) | (d << 24))
                        : (d | (c << 8) | (b << 16) | (a << 24)) ;
}

template<typename T>
constexpr uint32_t unpackU32At0(T x) noexcept { return ASMJIT_ARCH_LE ? uint32_t(uint64_t(x) & 0xFFFFFFFFU) : uint32_t(uint64_t(x) >> 32); }
template<typename T>
constexpr uint32_t unpackU32At1(T x) noexcept { return ASMJIT_ARCH_BE ? uint32_t(uint64_t(x) & 0xFFFFFFFFU) : uint32_t(uint64_t(x) >> 32); }

// ============================================================================
// [asmjit::IntUtils::Position of byte (in bit-shift)]
// ============================================================================

static inline uint32_t byteShiftOfDWordStruct(uint32_t index) noexcept {
  return ASMJIT_ARCH_LE ? index * 8 : (uint32_t(sizeof(uint32_t)) - 1U - index) * 8;
}

// ============================================================================
// [asmjit::IntUtils::Operators]
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
struct Min    { template<typename T> static inline T op(T x, T y) noexcept { return std::min<T>(x, y); } };
struct Max    { template<typename T> static inline T op(T x, T y) noexcept { return std::max<T>(x, y); } };

// ============================================================================
// [asmjit::IntUtils::BitWordIterator]
// ============================================================================

//! Iterates over each bit in a number which is set to 1.
//!
//! Example of use:
//!
//! ```
//! uint32_t bitsToIterate = 0x110F;
//! IntUtils::BitWordIterator<uint32_t> it(bitsToIterate);
//!
//! while (it.hasNext()) {
//!   uint32_t bitIndex = it.next();
//!   std::printf("Bit at %u is set\n", unsigned(bitIndex));
//! }
//! ```
template<typename T>
class BitWordIterator {
public:
  explicit constexpr BitWordIterator(T bitWord) noexcept
    : _bitWord(bitWord) {}

  inline void init(T bitWord) noexcept { _bitWord = bitWord; }
  inline bool hasNext() const noexcept { return _bitWord != 0; }

  inline uint32_t next() noexcept {
    ASMJIT_ASSERT(_bitWord != 0);
    uint32_t index = ctz(_bitWord);
    _bitWord ^= T(1U) << index;
    return index;
  }

  T _bitWord;
};

// ============================================================================
// [asmjit::IntUtils::BitVectorOps]
// ============================================================================

namespace Internal {
  template<typename T, class OPERATOR, class FULL_WORD_OP>
  static inline void bitVectorOp(T* buf, size_t index, size_t count) noexcept {
    if (count == 0)
      return;

    const size_t kTSizeInBits = bitSizeOf<T>();
    size_t vecIndex = index / kTSizeInBits; // T[]
    size_t bitIndex = index % kTSizeInBits; // T[][]

    buf += vecIndex;

    // The first BitWord requires special handling to preserve bits outside the fill region.
    const T kFillMask = allOnes<T>();
    size_t firstNBits = std::min<size_t>(kTSizeInBits - bitIndex, count);

    buf[0] = OPERATOR::op(buf[0], (kFillMask >> (kTSizeInBits - firstNBits)) << bitIndex);
    buf++;
    count -= firstNBits;

    // All bits between the first and last affected BitWords can be just filled.
    while (count >= kTSizeInBits) {
      buf[0] = FULL_WORD_OP::op(buf[0], kFillMask);
      buf++;
      count -= kTSizeInBits;
    }

    // The last BitWord requires special handling as well
    if (count)
      buf[0] = OPERATOR::op(buf[0], kFillMask >> (kTSizeInBits - count));
  }
}

//! Set bit in a bit-vector `buf` at `index`.
template<typename T>
static inline bool bitVectorGetBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  return bool((buf[vecIndex] >> bitIndex) & 0x1U);
}

//! Set bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static inline void bitVectorSetBit(T* buf, size_t index, bool value) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1U) << bitIndex;
  if (value)
    buf[vecIndex] |= bitMask;
  else
    buf[vecIndex] &= ~bitMask;
}

//! Set bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static inline void bitVectorFlipBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1U) << bitIndex;
  buf[vecIndex] ^= bitMask;
}

//! Fill `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static inline void bitVectorFill(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, Or, Set>(buf, index, count); }

//! Clear `count` bits in bit-vector `buf` starting at bit-index `index`.
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
// [asmjit::IntUtils::BitVectorIterator]
// ============================================================================

template<typename T>
class BitVectorIterator {
public:
  ASMJIT_FORCEINLINE BitVectorIterator(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    init(data, numBitWords, start);
  }

  ASMJIT_FORCEINLINE void init(const T* data, size_t numBitWords, size_t start = 0) noexcept {
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

  ASMJIT_FORCEINLINE bool hasNext() const noexcept {
    return _current != T(0);
  }

  ASMJIT_FORCEINLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1U) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++;

    _current = bitWord;
    return n;
  }

  ASMJIT_FORCEINLINE size_t peekNext() const noexcept {
    ASMJIT_ASSERT(_current != T(0));
    return _idx + ctz(_current);
  }

  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
};

// ============================================================================
// [asmjit::IntUtils::BitVectorFlipIterator]
// ============================================================================

template<typename T>
class BitVectorFlipIterator {
public:
  ASMJIT_FORCEINLINE BitVectorFlipIterator(const T* data, size_t numBitWords, size_t start = 0, T xorMask = 0) noexcept {
    init(data, numBitWords, start, xorMask);
  }

  ASMJIT_FORCEINLINE void init(const T* data, size_t numBitWords, size_t start = 0, T xorMask = 0) noexcept {
    const T* ptr = data + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = (*ptr++ ^ xorMask) & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += bitSizeOf<T>()) < end)
        bitWord = *ptr++ ^ xorMask;
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bitWord;
    _xorMask = xorMask;
  }

  ASMJIT_FORCEINLINE bool hasNext() const noexcept {
    return _current != T(0);
  }

  ASMJIT_FORCEINLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1U) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++ ^ _xorMask;

    _current = bitWord;
    return n;
  }

  ASMJIT_FORCEINLINE size_t nextAndFlip() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= allOnes<T>() << bit;
    _xorMask ^= allOnes<T>();

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++ ^ _xorMask;

    _current = bitWord;
    return n;
  }

  ASMJIT_FORCEINLINE size_t peekNext() const noexcept {
    ASMJIT_ASSERT(_current != T(0));
    return _idx + ctz(_current);
  }

  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
  T _xorMask;
};

// ============================================================================
// [asmjit::IntUtils::BitVectorIterator]
// ============================================================================

template<typename T, class OPERATOR>
class BitVectorOpIterator {
public:
  static constexpr uint32_t kTSizeInBits = bitSizeOf<T>();

  ASMJIT_FORCEINLINE BitVectorOpIterator(const T* aData, const T* bData, size_t numBitWords, size_t start = 0) noexcept {
    init(aData, bData, numBitWords, start);
  }

  ASMJIT_FORCEINLINE void init(const T* aData, const T* bData, size_t numBitWords, size_t start = 0) noexcept {
    const T* aPtr = aData + (start / bitSizeOf<T>());
    const T* bPtr = bData + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = OPERATOR::op(*aPtr++, *bPtr++) & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += kTSizeInBits) < end)
        bitWord = OPERATOR::op(*aPtr++, *bPtr++);
    }

    _aPtr = aPtr;
    _bPtr = bPtr;
    _idx = idx;
    _end = end;
    _current = bitWord;
  }

  ASMJIT_FORCEINLINE bool hasNext() noexcept {
    return _current != T(0);
  }

  ASMJIT_FORCEINLINE size_t next() noexcept {
    T bitWord = _current;
    ASMJIT_ASSERT(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1U) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += kTSizeInBits) < _end)
      bitWord = OPERATOR::op(*_aPtr++, *_bPtr++);

    _current = bitWord;
    return n;
  }

  const T* _aPtr;
  const T* _bPtr;
  size_t _idx;
  size_t _end;
  T _current;
};

} // IntUtils namespace

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_INTUTILS_H
