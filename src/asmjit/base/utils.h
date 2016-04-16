// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_UTILS_H
#define _ASMJIT_BASE_UTILS_H

// [Dependencies - AsmJit]
#include "../base/globals.h"

#if ASMJIT_CC_MSC_GE(14, 0, 0)
# include <intrin.h>
#endif // ASMJIT_OS_WINDOWS

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::IntTraits]
// ============================================================================

//! \internal
//! \{
template<size_t Size, int IsSigned>
struct IntTraitsPrivate {
  // Let it fail if not specialized!
};

template<> struct IntTraitsPrivate<1, 0> { typedef int     IntType; typedef int8_t  SignedType; typedef uint8_t  UnsignedType; };
template<> struct IntTraitsPrivate<1, 1> { typedef int     IntType; typedef int8_t  SignedType; typedef uint8_t  UnsignedType; };

template<> struct IntTraitsPrivate<2, 0> { typedef int     IntType; typedef int16_t SignedType; typedef uint16_t UnsignedType; };
template<> struct IntTraitsPrivate<2, 1> { typedef int     IntType; typedef int16_t SignedType; typedef uint16_t UnsignedType; };

template<> struct IntTraitsPrivate<4, 0> { typedef int64_t IntType; typedef int32_t SignedType; typedef uint32_t UnsignedType; };
template<> struct IntTraitsPrivate<4, 1> { typedef int     IntType; typedef int32_t SignedType; typedef uint32_t UnsignedType; };

template<> struct IntTraitsPrivate<8, 0> { typedef int64_t IntType; typedef int64_t SignedType; typedef uint64_t UnsignedType; };
template<> struct IntTraitsPrivate<8, 1> { typedef int64_t IntType; typedef int64_t SignedType; typedef uint64_t UnsignedType; };

//! \internal
template<typename T>
struct IntTraits {
  enum {
    kIsSigned = static_cast<T>(~static_cast<T>(0)) < static_cast<T>(0),
    kIsUnsigned = !kIsSigned,

    kIs8Bit = sizeof(T) == 1,
    kIs16Bit = sizeof(T) == 2,
    kIs32Bit = sizeof(T) == 4,
    kIs64Bit = sizeof(T) == 8,

    kIsIntPtr = sizeof(T) == sizeof(intptr_t)
  };

  typedef typename IntTraitsPrivate<sizeof(T), kIsSigned>::IntType IntType;
  typedef typename IntTraitsPrivate<sizeof(T), kIsSigned>::SignedType SignedType;
  typedef typename IntTraitsPrivate<sizeof(T), kIsSigned>::UnsignedType UnsignedType;

  //! Get a minimum value of `T`.
  static ASMJIT_INLINE T minValue() noexcept {
    if (kIsSigned)
      return static_cast<T>((~static_cast<UnsignedType>(0) >> 1) + static_cast<UnsignedType>(1));
    else
      return static_cast<T>(0);
  }

  //! Get a maximum value of `T`.
  static ASMJIT_INLINE T maxValue() noexcept {
    if (kIsSigned)
      return static_cast<T>(~static_cast<UnsignedType>(0) >> 1);
    else
      return ~static_cast<T>(0);
  }
};

//! \}

// ============================================================================
// [asmjit::Utils]
// ============================================================================

//! AsmJit utilities - integer, string, etc...
struct Utils {
  // --------------------------------------------------------------------------
  // [Float <-> Int]
  // --------------------------------------------------------------------------

  //! \internal
  union FloatBits {
    int32_t i;
    float f;
  };

  //! \internal
  union DoubleBits {
    int64_t i;
    double d;
  };

  //! Bit-cast `float` to a 32-bit integer.
  static ASMJIT_INLINE int32_t floatAsInt(float f) noexcept { FloatBits m; m.f = f; return m.i; }
  //! Bit-cast 32-bit integer to `float`.
  static ASMJIT_INLINE float intAsFloat(int32_t i) noexcept { FloatBits m; m.i = i; return m.f; }

  //! Bit-cast `double` to a 64-bit integer.
  static ASMJIT_INLINE int64_t doubleAsInt(double d) noexcept { DoubleBits m; m.d = d; return m.i; }
  //! Bit-cast 64-bit integer to `double`.
  static ASMJIT_INLINE double intAsDouble(int64_t i) noexcept { DoubleBits m; m.i = i; return m.d; }

  // --------------------------------------------------------------------------
  // [Pack / Unpack]
  // --------------------------------------------------------------------------

  //! Pack two 8-bit integer and one 16-bit integer into a 32-bit integer as it
  //! is an array of `{b0,b1,w2}`.
  static ASMJIT_INLINE uint32_t pack32_2x8_1x16(uint32_t b0, uint32_t b1, uint32_t w2) noexcept {
    return ASMJIT_ARCH_LE ? b0 + (b1 << 8) + (w2 << 16)
                          : (b0 << 24) + (b1 << 16) + w2;
  }

  //! Pack four 8-bit integer into a 32-bit integer as it is an array of `{b0,b1,b2,b3}`.
  static ASMJIT_INLINE uint32_t pack32_4x8(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3) noexcept {
    return ASMJIT_ARCH_LE ? b0 + (b1 << 8) + (b2 << 16) + (b3 << 24)
                          : (b0 << 24) + (b1 << 16) + (b2 << 8) + b3;
  }

  //! Pack two 32-bit integer into a 64-bit integer as it is an array of `{u0,u1}`.
  static ASMJIT_INLINE uint64_t pack64_2x32(uint32_t u0, uint32_t u1) noexcept {
    return ASMJIT_ARCH_LE ? (static_cast<uint64_t>(u1) << 32) + u0
                          : (static_cast<uint64_t>(u0) << 32) + u1;
  }

  // --------------------------------------------------------------------------
  // [Position of byte (in bit-shift)]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t byteShiftOfDWordStruct(uint32_t index) noexcept {
    if (ASMJIT_ARCH_LE)
      return index * 8;
    else
      return (sizeof(uint32_t) - 1 - index) * 8;
  }

  // --------------------------------------------------------------------------
  // [Min/Max]
  // --------------------------------------------------------------------------

  // Some environments declare `min()` and `max()` as preprocessor macros so it
  // was decided to use different names to prevent such collision.

  //! Get minimum value of `a` and `b`.
  template<typename T>
  static ASMJIT_INLINE T iMin(const T& a, const T& b) noexcept { return a < b ? a : b; }

  //! Get maximum value of `a` and `b`.
  template<typename T>
  static ASMJIT_INLINE T iMax(const T& a, const T& b) noexcept { return a > b ? a : b; }

  // --------------------------------------------------------------------------
  // [InInterval]
  // --------------------------------------------------------------------------

  //! Get whether `x` is greater than or equal to `a` and lesses than or equal to `b`.
  template<typename T>
  static ASMJIT_INLINE bool inInterval(T x, T a, T b) noexcept {
    return x >= a && x <= b;
  }

  // --------------------------------------------------------------------------
  // [AsInt]
  // --------------------------------------------------------------------------

  //! Map an integer `x` of type `T` to an `int` or `int64_t`, depending on the
  //! type. Used internally by AsmJit to dispatch an argument that can be an
  //! arbitrary integer type into a function that accepts either `int` or
  //! `int64_t`.
  template<typename T>
  static ASMJIT_INLINE typename IntTraits<T>::IntType asInt(T x) noexcept {
    return static_cast<typename IntTraits<T>::IntType>(x);
  }

  // --------------------------------------------------------------------------
  // [IsInt / IsUInt]
  // --------------------------------------------------------------------------

  //! Get whether the given integer `x` can be casted to an 8-bit signed integer.
  template<typename T>
  static ASMJIT_INLINE bool isInt8(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return sizeof(T) <= 1 || inInterval<SignedType>(SignedType(x), -128, 127);
    else
      return UnsignedType(x) <= UnsignedType(127U);
  }

  //! Get whether the given integer `x` can be casted to a 16-bit signed integer.
  template<typename T>
  static ASMJIT_INLINE bool isInt16(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return sizeof(T) <= 2 || inInterval<SignedType>(SignedType(x), -32768, 32767);
    else
      return sizeof(T) <= 1 || UnsignedType(x) <= UnsignedType(32767U);
  }

  //! Get whether the given integer `x` can be casted to a 32-bit signed integer.
  template<typename T>
  static ASMJIT_INLINE bool isInt32(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return sizeof(T) <= 4 || inInterval<SignedType>(SignedType(x), -2147483647 - 1, 2147483647);
    else
      return sizeof(T) <= 2 || UnsignedType(x) <= UnsignedType(2147483647U);
  }

  //! Get whether the given integer `x` can be casted to an 8-bit unsigned integer.
  template<typename T>
  static ASMJIT_INLINE bool isUInt8(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 1 ? true : x <= T(255));
    else
      return sizeof(T) <= 1 || UnsignedType(x) <= UnsignedType(255U);
  }

  //! Get whether the given integer `x` can be casted to a 12-bit unsigned integer (ARM specific).
  template<typename T>
  static ASMJIT_INLINE bool isUInt12(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 1 ? true : x <= T(4095));
    else
      return sizeof(T) <= 1 || UnsignedType(x) <= UnsignedType(4095U);
  }

  //! Get whether the given integer `x` can be casted to a 16-bit unsigned integer.
  template<typename T>
  static ASMJIT_INLINE bool isUInt16(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 2 ? true : x <= T(65535));
    else
      return sizeof(T) <= 2 || UnsignedType(x) <= UnsignedType(65535U);
  }

  //! Get whether the given integer `x` can be casted to a 32-bit unsigned integer.
  template<typename T>
  static ASMJIT_INLINE bool isUInt32(T x) noexcept {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 4 ? true : x <= T(4294967295U));
    else
      return sizeof(T) <= 4 || UnsignedType(x) <= UnsignedType(4294967295U);
  }

  // --------------------------------------------------------------------------
  // [IsPowerOf2]
  // --------------------------------------------------------------------------

  //! Get whether the `n` value is a power of two (only one bit is set).
  template<typename T>
  static ASMJIT_INLINE bool isPowerOf2(T n) noexcept {
    return n != 0 && (n & (n - 1)) == 0;
  }

  // --------------------------------------------------------------------------
  // [Mask]
  // --------------------------------------------------------------------------

  //! Generate a bit-mask that has `x` bit set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x) noexcept {
    ASMJIT_ASSERT(x < 32);
    return static_cast<uint32_t>(1) << x;
  }

  //! Generate a bit-mask that has `x0` and `x1` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1) noexcept {
    return mask(x0) | mask(x1);
  }

  //! Generate a bit-mask that has `x0`, `x1` and `x2` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2) noexcept {
    return mask(x0, x1) | mask(x2);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2` and `x3` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3) noexcept {
    return mask(x0, x1) | mask(x2, x3);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3` and `x4` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4) noexcept {
    return mask(x0, x1) | mask(x2, x3) | mask(x4);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4` and `x5` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5) noexcept {
    return mask(x0, x1) | mask(x2, x3) | mask(x4, x5);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5` and `x6` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6) noexcept {
    return mask(x0, x1) | mask(x2, x3) | mask(x4, x5) | mask(x6);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5`, `x6` and `x7` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7) noexcept {
    return mask(x0, x1) | mask(x2, x3) | mask(x4, x5) | mask(x6, x7);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5`, `x6`, `x7` and `x8` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7, uint32_t x8) noexcept {
    return mask(x0, x1) | mask(x2, x3) | mask(x4, x5) | mask(x6, x7) | mask(x8);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5`, `x6`, `x7`, `x8` and `x9` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7, uint32_t x8, uint32_t x9) noexcept {
    return mask(x0, x1) | mask(x2, x3) | mask(x4, x5) | mask(x6, x7) | mask(x8, x9);
  }

  // --------------------------------------------------------------------------
  // [Bits]
  // --------------------------------------------------------------------------

  //! Generate a bit-mask that has `x` most significant bits set.
  static ASMJIT_INLINE uint32_t bits(uint32_t x) noexcept {
    // Shifting more bits than the type has results in undefined behavior. In
    // such case asmjit trashes the result by ORing with `overflow` mask, which
    // discards the undefined value returned by the shift.
    uint32_t overflow = static_cast<uint32_t>(
      -static_cast<int32_t>(x >= sizeof(uint32_t) * 8));

    return ((static_cast<uint32_t>(1) << x) - 1U) | overflow;
  }

  // --------------------------------------------------------------------------
  // [HasBit]
  // --------------------------------------------------------------------------

  //! Get whether `x` has bit `n` set.
  template<typename T, typename Index>
  static ASMJIT_INLINE bool hasBit(T x, Index n) noexcept {
    return (x & (static_cast<T>(1) << n)) != 0;
  }

  // --------------------------------------------------------------------------
  // [BitCount]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t bitCountSlow(uint32_t x) noexcept {
    // From: http://graphics.stanford.edu/~seander/bithacks.html
    x = x - ((x >> 1) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
    return (((x + (x >> 4)) & 0x0F0F0F0FU) * 0x01010101U) >> 24;
  }

  //! Get count of bits in `x`.
  static ASMJIT_INLINE uint32_t bitCount(uint32_t x) noexcept {
#if ASMJIT_CC_GCC || ASMJIT_CC_CLANG
    return __builtin_popcount(x);
#else
    return bitCountSlow(x);
#endif
  }

  // --------------------------------------------------------------------------
  // [FindFirstBit]
  // --------------------------------------------------------------------------

  //! \internal
  static ASMJIT_INLINE uint32_t findFirstBitSlow(uint32_t mask) noexcept {
    // This is a reference (slow) implementation of `findFirstBit()`, used when
    // we don't have a C++ compiler support. The implementation speed has been
    // improved to check for 2 bits per iteration.
    uint32_t i = 1;

    while (mask != 0) {
      uint32_t two = mask & 0x3;
      if (two != 0x0)
        return i - (two & 0x1);

      i += 2;
      mask >>= 2;
    }

    return 0xFFFFFFFFU;
  }

  //! Find a first bit in `mask`.
  static ASMJIT_INLINE uint32_t findFirstBit(uint32_t mask) noexcept {
#if ASMJIT_CC_MSC_GE(14, 0, 0) && (ASMJIT_ARCH_X86 || ASMJIT_ARCH_ARM32 || \
                                   ASMJIT_ARCH_X64 || ASMJIT_ARCH_ARM64)
    DWORD i;
    if (_BitScanForward(&i, mask))
      return static_cast<uint32_t>(i);
    else
      return 0xFFFFFFFFU;
#elif ASMJIT_CC_GCC_GE(3, 4, 6) || ASMJIT_CC_CLANG
    if (mask)
      return __builtin_ctz(mask);
    else
      return 0xFFFFFFFFU;
#else
    return findFirstBitSlow(mask);
#endif
  }

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t keepNOnesFromRight(uint32_t mask, uint32_t nBits) noexcept {
    uint32_t m = 0x1;

    do {
      nBits -= (mask & m) != 0;
      m <<= 1;
      if (nBits == 0) {
        m -= 1;
        mask &= m;
        break;
      }
    } while (m);

    return mask;
  }

  static ASMJIT_INLINE uint32_t indexNOnesFromRight(uint8_t* dst, uint32_t mask, uint32_t nBits) noexcept {
    uint32_t totalBits = nBits;
    uint8_t i = 0;
    uint32_t m = 0x1;

    do {
      if (mask & m) {
        *dst++ = i;
        if (--nBits == 0)
          break;
      }

      m <<= 1;
      i++;
    } while (m);

    return totalBits - nBits;
  }

  // --------------------------------------------------------------------------
  // [Alignment]
  // --------------------------------------------------------------------------

  template<typename T>
  static ASMJIT_INLINE bool isAligned(T base, T alignment) noexcept {
    return (base % alignment) == 0;
  }

  //! Align `base` to `alignment`.
  template<typename T>
  static ASMJIT_INLINE T alignTo(T base, T alignment) noexcept {
    return (base + (alignment - 1)) & ~(alignment - 1);
  }

  template<typename T>
  static ASMJIT_INLINE T alignToPowerOf2(T base) noexcept {
    // Implementation is from "Hacker's Delight" by Henry S. Warren, Jr.
    base -= 1;

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable: 4293)
#endif // _MSC_VER

    base = base | (base >> 1);
    base = base | (base >> 2);
    base = base | (base >> 4);

    // 8/16/32 constants are multiplied by the condition to prevent a compiler
    // complaining about the 'shift count >= type width' (GCC).
    if (sizeof(T) >= 2) base = base | (base >> ( 8 * (sizeof(T) >= 2))); // Base >>  8.
    if (sizeof(T) >= 4) base = base | (base >> (16 * (sizeof(T) >= 4))); // Base >> 16.
    if (sizeof(T) >= 8) base = base | (base >> (32 * (sizeof(T) >= 8))); // Base >> 32.

#if defined(_MSC_VER)
# pragma warning(pop)
#endif // _MSC_VER

    return base + 1;
  }

  //! Get delta required to align `base` to `alignment`.
  template<typename T>
  static ASMJIT_INLINE T alignDiff(T base, T alignment) noexcept {
    return alignTo(base, alignment) - base;
  }

  // --------------------------------------------------------------------------
  // [String]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE size_t strLen(const char* s, size_t maxlen) noexcept {
    size_t i;
    for (i = 0; i < maxlen; i++)
      if (!s[i])
        break;
    return i;
  }

  // --------------------------------------------------------------------------
  // [BSwap]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t byteswap32(uint32_t x) noexcept {
#if ASMJIT_CC_MSC
    return static_cast<uint32_t>(_byteswap_ulong(x));
#elif ASMJIT_CC_GCC_GE(4, 3, 0) || ASMJIT_CC_CLANG_GE(2, 6, 0)
    return __builtin_bswap32(x);
#else
    uint32_t y = x & 0x00FFFF00U;
    x = (x << 24) + (x >> 24);
    y = (y <<  8) + (y >>  8);
    return x + (y & 0x00FFFF00U);
#endif
  }

  // --------------------------------------------------------------------------
  // [ReadMem]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t readU8(const void* p) noexcept {
    return static_cast<uint32_t>(static_cast<const uint8_t*>(p)[0]);
  }

  static ASMJIT_INLINE int32_t readI8(const void* p) noexcept {
    return static_cast<int32_t>(static_cast<const int8_t*>(p)[0]);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint32_t readU16xLE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_LE && (ASMJIT_ARCH_UNALIGNED_16 || Alignment >= 2)) {
      return static_cast<uint32_t>(static_cast<const uint16_t*>(p)[0]);
    }
    else {
      uint32_t x = static_cast<uint32_t>(static_cast<const uint8_t*>(p)[0]);
      uint32_t y = static_cast<uint32_t>(static_cast<const uint8_t*>(p)[1]);
      return x + (y << 8);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint32_t readU16xBE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_BE && (ASMJIT_ARCH_UNALIGNED_16 || Alignment >= 2)) {
      return static_cast<uint32_t>(static_cast<const uint16_t*>(p)[0]);
    }
    else {
      uint32_t x = static_cast<uint32_t>(static_cast<const uint8_t*>(p)[0]);
      uint32_t y = static_cast<uint32_t>(static_cast<const uint8_t*>(p)[1]);
      return (x << 8) + y;
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint32_t readU16x(const void* p) noexcept {
    return ASMJIT_ARCH_LE ? readU16xLE<Alignment>(p) : readU16xBE<Alignment>(p);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int32_t readI16xLE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_LE && (ASMJIT_ARCH_UNALIGNED_16 || Alignment >= 2)) {
      return static_cast<int32_t>(static_cast<const int16_t*>(p)[0]);
    }
    else {
      int32_t x = static_cast<int32_t>(static_cast<const uint8_t*>(p)[0]);
      int32_t y = static_cast<int32_t>(static_cast<const int8_t*>(p)[1]);
      return x + (y << 8);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int32_t readI16xBE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_BE && (ASMJIT_ARCH_UNALIGNED_16 || Alignment >= 2)) {
      return static_cast<int32_t>(static_cast<const int16_t*>(p)[0]);
    }
    else {
      int32_t x = static_cast<int32_t>(static_cast<const int8_t*>(p)[0]);
      int32_t y = static_cast<int32_t>(static_cast<const uint8_t*>(p)[1]);
      return (x << 8) + y;
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int32_t readI16x(const void* p) noexcept {
    return ASMJIT_ARCH_LE ? readI16xLE<Alignment>(p) : readI16xBE<Alignment>(p);
  }

  static ASMJIT_INLINE uint32_t readU16aLE(const void* p) noexcept { return readU16xLE<2>(p); }
  static ASMJIT_INLINE uint32_t readU16uLE(const void* p) noexcept { return readU16xLE<0>(p); }

  static ASMJIT_INLINE uint32_t readU16aBE(const void* p) noexcept { return readU16xBE<2>(p); }
  static ASMJIT_INLINE uint32_t readU16uBE(const void* p) noexcept { return readU16xBE<0>(p); }

  static ASMJIT_INLINE uint32_t readU16a(const void* p) noexcept { return readU16x<2>(p); }
  static ASMJIT_INLINE uint32_t readU16u(const void* p) noexcept { return readU16x<0>(p); }

  static ASMJIT_INLINE int32_t readI16aLE(const void* p) noexcept { return readI16xLE<2>(p); }
  static ASMJIT_INLINE int32_t readI16uLE(const void* p) noexcept { return readI16xLE<0>(p); }

  static ASMJIT_INLINE int32_t readI16aBE(const void* p) noexcept { return readI16xBE<2>(p); }
  static ASMJIT_INLINE int32_t readI16uBE(const void* p) noexcept { return readI16xBE<0>(p); }

  static ASMJIT_INLINE int32_t readI16a(const void* p) noexcept { return readI16x<2>(p); }
  static ASMJIT_INLINE int32_t readI16u(const void* p) noexcept { return readI16x<0>(p); }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint32_t readU32xLE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_UNALIGNED_32 || Alignment >= 4) {
      uint32_t x = static_cast<const uint32_t*>(p)[0];
      return ASMJIT_ARCH_LE ? x : byteswap32(x);
    }
    else {
      uint32_t x = readU16xLE<Alignment>(static_cast<const uint8_t*>(p) + 0);
      uint32_t y = readU16xLE<Alignment>(static_cast<const uint8_t*>(p) + 2);
      return x + (y << 16);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint32_t readU32xBE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_UNALIGNED_32 || Alignment >= 4) {
      uint32_t x = static_cast<const uint32_t*>(p)[0];
      return ASMJIT_ARCH_BE ? x : byteswap32(x);
    }
    else {
      uint32_t x = readU16xBE<Alignment>(static_cast<const uint8_t*>(p) + 0);
      uint32_t y = readU16xBE<Alignment>(static_cast<const uint8_t*>(p) + 2);
      return (x << 16) + y;
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint32_t readU32x(const void* p) noexcept {
    return ASMJIT_ARCH_LE ? readU32xLE<Alignment>(p) : readU32xBE<Alignment>(p);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int32_t readI32xLE(const void* p) noexcept {
    return static_cast<int32_t>(readU32xLE<Alignment>(p));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int32_t readI32xBE(const void* p) noexcept {
    return static_cast<int32_t>(readU32xBE<Alignment>(p));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int32_t readI32x(const void* p) noexcept {
    return ASMJIT_ARCH_LE ? readI32xLE<Alignment>(p) : readI32xBE<Alignment>(p);
  }

  static ASMJIT_INLINE uint32_t readU32a(const void* p) noexcept { return readU32x<4>(p); }
  static ASMJIT_INLINE uint32_t readU32u(const void* p) noexcept { return readU32x<0>(p); }

  static ASMJIT_INLINE uint32_t readU32aLE(const void* p) noexcept { return readU32xLE<4>(p); }
  static ASMJIT_INLINE uint32_t readU32uLE(const void* p) noexcept { return readU32xLE<0>(p); }

  static ASMJIT_INLINE uint32_t readU32aBE(const void* p) noexcept { return readU32xBE<4>(p); }
  static ASMJIT_INLINE uint32_t readU32uBE(const void* p) noexcept { return readU32xBE<0>(p); }

  static ASMJIT_INLINE int32_t readI32a(const void* p) noexcept { return readI32x<4>(p); }
  static ASMJIT_INLINE int32_t readI32u(const void* p) noexcept { return readI32x<0>(p); }

  static ASMJIT_INLINE int32_t readI32aLE(const void* p) noexcept { return readI32xLE<4>(p); }
  static ASMJIT_INLINE int32_t readI32uLE(const void* p) noexcept { return readI32xLE<0>(p); }

  static ASMJIT_INLINE int32_t readI32aBE(const void* p) noexcept { return readI32xBE<4>(p); }
  static ASMJIT_INLINE int32_t readI32uBE(const void* p) noexcept { return readI32xBE<0>(p); }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint64_t readU64xLE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_LE && (ASMJIT_ARCH_UNALIGNED_64 || Alignment >= 8)) {
      return static_cast<const uint64_t*>(p)[0];
    }
    else {
      uint32_t x = readU32xLE<Alignment / 2U>(static_cast<const uint8_t*>(p) + 0);
      uint32_t y = readU32xLE<Alignment / 2U>(static_cast<const uint8_t*>(p) + 4);
      return static_cast<uint64_t>(x) + (static_cast<uint64_t>(y) << 32);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint64_t readU64xBE(const void* p) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_BE && (ASMJIT_ARCH_UNALIGNED_64 || Alignment >= 8)) {
      return static_cast<const uint64_t*>(p)[0];
    }
    else {
      uint32_t x = readU32xLE<Alignment / 2U>(static_cast<const uint8_t*>(p) + 0);
      uint32_t y = readU32xLE<Alignment / 2U>(static_cast<const uint8_t*>(p) + 4);
      return (static_cast<uint64_t>(x) << 32) + static_cast<uint64_t>(y);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE uint64_t readU64x(const void* p) noexcept {
    return ASMJIT_ARCH_LE ? readU64xLE<Alignment>(p) : readU64xBE<Alignment>(p);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int64_t readI64xLE(const void* p) noexcept {
    return static_cast<int64_t>(readU64xLE<Alignment>(p));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int64_t readI64xBE(const void* p) noexcept {
    return static_cast<int64_t>(readU64xBE<Alignment>(p));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE int64_t readI64x(const void* p) noexcept {
    return ASMJIT_ARCH_LE ? readI64xLE<Alignment>(p) : readI64xBE<Alignment>(p);
  }

  static ASMJIT_INLINE uint64_t readU64a(const void* p) noexcept { return readU64x<8>(p); }
  static ASMJIT_INLINE uint64_t readU64u(const void* p) noexcept { return readU64x<0>(p); }

  static ASMJIT_INLINE uint64_t readU64aLE(const void* p) noexcept { return readU64xLE<8>(p); }
  static ASMJIT_INLINE uint64_t readU64uLE(const void* p) noexcept { return readU64xLE<0>(p); }

  static ASMJIT_INLINE uint64_t readU64aBE(const void* p) noexcept { return readU64xBE<8>(p); }
  static ASMJIT_INLINE uint64_t readU64uBE(const void* p) noexcept { return readU64xBE<0>(p); }

  static ASMJIT_INLINE int64_t readI64a(const void* p) noexcept { return readI64x<8>(p); }
  static ASMJIT_INLINE int64_t readI64u(const void* p) noexcept { return readI64x<0>(p); }

  static ASMJIT_INLINE int64_t readI64aLE(const void* p) noexcept { return readI64xLE<8>(p); }
  static ASMJIT_INLINE int64_t readI64uLE(const void* p) noexcept { return readI64xLE<0>(p); }

  static ASMJIT_INLINE int64_t readI64aBE(const void* p) noexcept { return readI64xBE<8>(p); }
  static ASMJIT_INLINE int64_t readI64uBE(const void* p) noexcept { return readI64xBE<0>(p); }

  // --------------------------------------------------------------------------
  // [WriteMem]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE void writeU8(void* p, uint32_t x) noexcept {
    static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>(x & 0xFFU);
  }

  static ASMJIT_INLINE void writeI8(void* p, int32_t x) noexcept {
    static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>(x & 0xFF);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU16xLE(void* p, uint32_t x) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_LE && (ASMJIT_ARCH_UNALIGNED_16 || Alignment >= 2)) {
      static_cast<uint16_t*>(p)[0] = static_cast<uint16_t>(x & 0xFFFFU);
    }
    else {
      static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>((x     ) & 0xFFU);
      static_cast<uint8_t*>(p)[1] = static_cast<uint8_t>((x >> 8) & 0xFFU);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU16xBE(void* p, uint32_t x) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_BE && (ASMJIT_ARCH_UNALIGNED_16 || Alignment >= 2)) {
      static_cast<uint16_t*>(p)[0] = static_cast<uint16_t>(x & 0xFFFFU);
    }
    else {
      static_cast<uint8_t*>(p)[0] = static_cast<uint8_t>((x >> 8) & 0xFFU);
      static_cast<uint8_t*>(p)[1] = static_cast<uint8_t>((x     ) & 0xFFU);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU16x(void* p, uint32_t x) noexcept {
    if (ASMJIT_ARCH_LE)
      writeU16xLE<Alignment>(p, x);
    else
      writeU16xBE<Alignment>(p, x);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI16xLE(void* p, int32_t x) noexcept {
    writeU16xLE<Alignment>(p, static_cast<uint32_t>(x));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI16xBE(void* p, int32_t x) noexcept {
    writeU16xBE<Alignment>(p, static_cast<uint32_t>(x));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI16x(void* p, int32_t x) noexcept {
    writeU16x<Alignment>(p, static_cast<uint32_t>(x));
  }

  static ASMJIT_INLINE void writeU16aLE(void* p, uint32_t x) noexcept { writeU16xLE<2>(p, x); }
  static ASMJIT_INLINE void writeU16uLE(void* p, uint32_t x) noexcept { writeU16xLE<0>(p, x); }

  static ASMJIT_INLINE void writeU16aBE(void* p, uint32_t x) noexcept { writeU16xBE<2>(p, x); }
  static ASMJIT_INLINE void writeU16uBE(void* p, uint32_t x) noexcept { writeU16xBE<0>(p, x); }

  static ASMJIT_INLINE void writeU16a(void* p, uint32_t x) noexcept { writeU16x<2>(p, x); }
  static ASMJIT_INLINE void writeU16u(void* p, uint32_t x) noexcept { writeU16x<0>(p, x); }

  static ASMJIT_INLINE void writeI16aLE(void* p, int32_t x) noexcept { writeI16xLE<2>(p, x); }
  static ASMJIT_INLINE void writeI16uLE(void* p, int32_t x) noexcept { writeI16xLE<0>(p, x); }

  static ASMJIT_INLINE void writeI16aBE(void* p, int32_t x) noexcept { writeI16xBE<2>(p, x); }
  static ASMJIT_INLINE void writeI16uBE(void* p, int32_t x) noexcept { writeI16xBE<0>(p, x); }

  static ASMJIT_INLINE void writeI16a(void* p, int32_t x) noexcept { writeI16x<2>(p, x); }
  static ASMJIT_INLINE void writeI16u(void* p, int32_t x) noexcept { writeI16x<0>(p, x); }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU32xLE(void* p, uint32_t x) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_UNALIGNED_32 || Alignment >= 4) {
      static_cast<uint32_t*>(p)[0] = ASMJIT_ARCH_LE ? x : byteswap32(x);
    }
    else {
      writeU16xLE<Alignment>(static_cast<uint8_t*>(p) + 0, x >> 16);
      writeU16xLE<Alignment>(static_cast<uint8_t*>(p) + 2, x);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU32xBE(void* p, uint32_t x) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_UNALIGNED_32 || Alignment >= 4) {
      static_cast<uint32_t*>(p)[0] = ASMJIT_ARCH_BE ? x : byteswap32(x);
    }
    else {
      writeU16xBE<Alignment>(static_cast<uint8_t*>(p) + 0, x);
      writeU16xBE<Alignment>(static_cast<uint8_t*>(p) + 2, x >> 16);
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU32x(void* p, uint32_t x) noexcept {
    if (ASMJIT_ARCH_LE)
      writeU32xLE<Alignment>(p, x);
    else
      writeU32xBE<Alignment>(p, x);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI32xLE(void* p, int32_t x) noexcept {
    writeU32xLE<Alignment>(p, static_cast<uint32_t>(x));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI32xBE(void* p, int32_t x) noexcept {
    writeU32xBE<Alignment>(p, static_cast<uint32_t>(x));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI32x(void* p, int32_t x) noexcept {
    writeU32x<Alignment>(p, static_cast<uint32_t>(x));
  }

  static ASMJIT_INLINE void writeU32aLE(void* p, uint32_t x) noexcept { writeU32xLE<4>(p, x); }
  static ASMJIT_INLINE void writeU32uLE(void* p, uint32_t x) noexcept { writeU32xLE<0>(p, x); }

  static ASMJIT_INLINE void writeU32aBE(void* p, uint32_t x) noexcept { writeU32xBE<4>(p, x); }
  static ASMJIT_INLINE void writeU32uBE(void* p, uint32_t x) noexcept { writeU32xBE<0>(p, x); }

  static ASMJIT_INLINE void writeU32a(void* p, uint32_t x) noexcept { writeU32x<4>(p, x); }
  static ASMJIT_INLINE void writeU32u(void* p, uint32_t x) noexcept { writeU32x<0>(p, x); }

  static ASMJIT_INLINE void writeI32aLE(void* p, int32_t x) noexcept { writeI32xLE<4>(p, x); }
  static ASMJIT_INLINE void writeI32uLE(void* p, int32_t x) noexcept { writeI32xLE<0>(p, x); }

  static ASMJIT_INLINE void writeI32aBE(void* p, int32_t x) noexcept { writeI32xBE<4>(p, x); }
  static ASMJIT_INLINE void writeI32uBE(void* p, int32_t x) noexcept { writeI32xBE<0>(p, x); }

  static ASMJIT_INLINE void writeI32a(void* p, int32_t x) noexcept { writeI32x<4>(p, x); }
  static ASMJIT_INLINE void writeI32u(void* p, int32_t x) noexcept { writeI32x<0>(p, x); }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU64xLE(void* p, uint64_t x) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_LE && (ASMJIT_ARCH_UNALIGNED_64 || Alignment >= 8)) {
      static_cast<uint64_t*>(p)[0] = x;
    }
    else {
      writeU32xLE<Alignment / 2U>(static_cast<uint8_t*>(p) + 0, static_cast<uint32_t>(x >> 32));
      writeU32xLE<Alignment / 2U>(static_cast<uint8_t*>(p) + 4, static_cast<uint32_t>(x & 0xFFFFFFFFU));
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU64xBE(void* p, uint64_t x) noexcept {
    ASMJIT_ASSUME_ALIGNED(p, Alignment > 1 ? Alignment : 1U);
    if (ASMJIT_ARCH_BE && (ASMJIT_ARCH_UNALIGNED_64 || Alignment >= 8)) {
      static_cast<uint64_t*>(p)[0] = x;
    }
    else {
      writeU32xBE<Alignment / 2U>(static_cast<uint8_t*>(p) + 0, static_cast<uint32_t>(x & 0xFFFFFFFFU));
      writeU32xBE<Alignment / 2U>(static_cast<uint8_t*>(p) + 4, static_cast<uint32_t>(x >> 32));
    }
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeU64x(void* p, uint64_t x) noexcept {
    if (ASMJIT_ARCH_LE)
      writeU64xLE<Alignment>(p, x);
    else
      writeU64xBE<Alignment>(p, x);
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI64xLE(void* p, int64_t x) noexcept {
    writeU64xLE<Alignment>(p, static_cast<uint64_t>(x));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI64xBE(void* p, int64_t x) noexcept {
    writeU64xBE<Alignment>(p, static_cast<uint64_t>(x));
  }

  template<unsigned int Alignment>
  static ASMJIT_INLINE void writeI64x(void* p, int64_t x) noexcept {
    writeU64x<Alignment>(p, static_cast<uint64_t>(x));
  }

  static ASMJIT_INLINE void writeU64aLE(void* p, uint64_t x) noexcept { writeU64xLE<8>(p, x); }
  static ASMJIT_INLINE void writeU64uLE(void* p, uint64_t x) noexcept { writeU64xLE<0>(p, x); }

  static ASMJIT_INLINE void writeU64aBE(void* p, uint64_t x) noexcept { writeU64xBE<8>(p, x); }
  static ASMJIT_INLINE void writeU64uBE(void* p, uint64_t x) noexcept { writeU64xBE<0>(p, x); }

  static ASMJIT_INLINE void writeU64a(void* p, uint64_t x) noexcept { writeU64x<8>(p, x); }
  static ASMJIT_INLINE void writeU64u(void* p, uint64_t x) noexcept { writeU64x<0>(p, x); }

  static ASMJIT_INLINE void writeI64aLE(void* p, int64_t x) noexcept { writeI64xLE<8>(p, x); }
  static ASMJIT_INLINE void writeI64uLE(void* p, int64_t x) noexcept { writeI64xLE<0>(p, x); }

  static ASMJIT_INLINE void writeI64aBE(void* p, int64_t x) noexcept { writeI64xBE<8>(p, x); }
  static ASMJIT_INLINE void writeI64uBE(void* p, int64_t x) noexcept { writeI64xBE<0>(p, x); }

  static ASMJIT_INLINE void writeI64a(void* p, int64_t x) noexcept { writeI64x<8>(p, x); }
  static ASMJIT_INLINE void writeI64u(void* p, int64_t x) noexcept { writeI64x<0>(p, x); }

  // --------------------------------------------------------------------------
  // [GetTickCount]
  // --------------------------------------------------------------------------

  //! Get the current CPU tick count, used for benchmarking (1ms resolution).
  static ASMJIT_API uint32_t getTickCount() noexcept;
};

// ============================================================================
// [asmjit::UInt64]
// ============================================================================

union UInt64 {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64 fromUInt64(uint64_t val) noexcept {
    UInt64 data;
    data.setUInt64(val);
    return data;
  }

  ASMJIT_INLINE UInt64 fromUInt64(const UInt64& val) noexcept {
    UInt64 data;
    data.setUInt64(val);
    return data;
  }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 = 0;
    }
    else {
      u32[0] = 0;
      u32[1] = 0;
    }
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE uint64_t getUInt64() const noexcept {
    return u64;
  }

  ASMJIT_INLINE UInt64& setUInt64(uint64_t val) noexcept {
    u64 = val;
    return *this;
  }

  ASMJIT_INLINE UInt64& setUInt64(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 = val.u64;
    }
    else {
      u32[0] = val.u32[0];
      u32[1] = val.u32[1];
    }
    return *this;
  }

  ASMJIT_INLINE UInt64& setPacked_2x32(uint32_t u0, uint32_t u1) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 = Utils::pack64_2x32(u0, u1);
    }
    else {
      u32[0] = u0;
      u32[1] = u1;
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Add]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& add(uint64_t val) noexcept {
    u64 += val;
    return *this;
  }

  ASMJIT_INLINE UInt64& add(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 += val.u64;
    }
    else {
      u32[0] += val.u32[0];
      u32[1] += val.u32[1];
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Sub]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& sub(uint64_t val) noexcept {
    u64 -= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& sub(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 -= val.u64;
    }
    else {
      u32[0] -= val.u32[0];
      u32[1] -= val.u32[1];
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [And]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& and_(uint64_t val) noexcept {
    u64 &= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& and_(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 &= val.u64;
    }
    else {
      u32[0] &= val.u32[0];
      u32[1] &= val.u32[1];
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [AndNot]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& andNot(uint64_t val) noexcept {
    u64 &= ~val;
    return *this;
  }

  ASMJIT_INLINE UInt64& andNot(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 &= ~val.u64;
    }
    else {
      u32[0] &= ~val.u32[0];
      u32[1] &= ~val.u32[1];
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Or]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& or_(uint64_t val) noexcept {
    u64 |= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& or_(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 |= val.u64;
    }
    else {
      u32[0] |= val.u32[0];
      u32[1] |= val.u32[1];
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Xor]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& xor_(uint64_t val) noexcept {
    u64 ^= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& xor_(const UInt64& val) noexcept {
    if (ASMJIT_ARCH_64BIT) {
      u64 ^= val.u64;
    }
    else {
      u32[0] ^= val.u32[0];
      u32[1] ^= val.u32[1];
    }
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Eq]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isZero() const noexcept {
    if (ASMJIT_ARCH_64BIT)
      return u64 == 0;
    else
      return (u32[0] | u32[1]) == 0;
  }

  ASMJIT_INLINE bool isNonZero() const noexcept {
    if (ASMJIT_ARCH_64BIT)
      return u64 != 0;
    else
      return (u32[0] | u32[1]) != 0;
  }

  ASMJIT_INLINE bool eq(uint64_t val) const noexcept {
    return u64 == val;
  }

  ASMJIT_INLINE bool eq(const UInt64& val) const noexcept {
    if (ASMJIT_ARCH_64BIT)
      return u64 == val.u64;
    else
      return u32[0] == val.u32[0] && u32[1] == val.u32[1];
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& operator+=(uint64_t val) noexcept { return add(val); }
  ASMJIT_INLINE UInt64& operator+=(const UInt64& val) noexcept { return add(val); }

  ASMJIT_INLINE UInt64& operator-=(uint64_t val) noexcept { return sub(val); }
  ASMJIT_INLINE UInt64& operator-=(const UInt64& val) noexcept { return sub(val); }

  ASMJIT_INLINE UInt64& operator&=(uint64_t val) noexcept { return and_(val); }
  ASMJIT_INLINE UInt64& operator&=(const UInt64& val) noexcept { return and_(val); }

  ASMJIT_INLINE UInt64& operator|=(uint64_t val) noexcept { return or_(val); }
  ASMJIT_INLINE UInt64& operator|=(const UInt64& val) noexcept { return or_(val); }

  ASMJIT_INLINE UInt64& operator^=(uint64_t val) noexcept { return xor_(val); }
  ASMJIT_INLINE UInt64& operator^=(const UInt64& val) noexcept { return xor_(val); }

  ASMJIT_INLINE bool operator==(uint64_t val) const noexcept { return eq(val); }
  ASMJIT_INLINE bool operator==(const UInt64& val) const noexcept { return eq(val); }

  ASMJIT_INLINE bool operator!=(uint64_t val) const noexcept { return !eq(val); }
  ASMJIT_INLINE bool operator!=(const UInt64& val) const noexcept { return !eq(val); }

  ASMJIT_INLINE bool operator<(uint64_t val) const noexcept { return u64 < val; }
  ASMJIT_INLINE bool operator<(const UInt64& val) const noexcept { return u64 < val.u64; }

  ASMJIT_INLINE bool operator<=(uint64_t val) const noexcept { return u64 <= val; }
  ASMJIT_INLINE bool operator<=(const UInt64& val) const noexcept { return u64 <= val.u64; }

  ASMJIT_INLINE bool operator>(uint64_t val) const noexcept { return u64 > val; }
  ASMJIT_INLINE bool operator>(const UInt64& val) const noexcept { return u64 > val.u64; }

  ASMJIT_INLINE bool operator>=(uint64_t val) const noexcept { return u64 >= val; }
  ASMJIT_INLINE bool operator>=(const UInt64& val) const noexcept { return u64 >= val.u64; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! 64-bit unsigned value.
  uint64_t u64;

  uint32_t u32[2];
  uint16_t u16[4];
  uint8_t u8[8];

  struct {
#if ASMJIT_ARCH_LE
    uint32_t lo, hi;
#else
    uint32_t hi, lo;
#endif // ASMJIT_ARCH_LE
  };
};

// ============================================================================
// [asmjit::Lock]
// ============================================================================

//! \internal
//!
//! Lock.
struct Lock {
  ASMJIT_NO_COPY(Lock)

  // --------------------------------------------------------------------------
  // [Windows]
  // --------------------------------------------------------------------------

#if ASMJIT_OS_WINDOWS
  typedef CRITICAL_SECTION Handle;

  //! Create a new `Lock` instance.
  ASMJIT_INLINE Lock() noexcept { InitializeCriticalSection(&_handle); }
  //! Destroy the `Lock` instance.
  ASMJIT_INLINE ~Lock() noexcept { DeleteCriticalSection(&_handle); }

  //! Lock.
  ASMJIT_INLINE void lock() noexcept { EnterCriticalSection(&_handle); }
  //! Unlock.
  ASMJIT_INLINE void unlock() noexcept { LeaveCriticalSection(&_handle); }
#endif // ASMJIT_OS_WINDOWS

  // --------------------------------------------------------------------------
  // [Posix]
  // --------------------------------------------------------------------------

#if ASMJIT_OS_POSIX
  typedef pthread_mutex_t Handle;

  //! Create a new `Lock` instance.
  ASMJIT_INLINE Lock() noexcept { pthread_mutex_init(&_handle, nullptr); }
  //! Destroy the `Lock` instance.
  ASMJIT_INLINE ~Lock() noexcept { pthread_mutex_destroy(&_handle); }

  //! Lock.
  ASMJIT_INLINE void lock() noexcept { pthread_mutex_lock(&_handle); }
  //! Unlock.
  ASMJIT_INLINE void unlock() noexcept { pthread_mutex_unlock(&_handle); }
#endif // ASMJIT_OS_POSIX

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Native handle.
  Handle _handle;
};

// ============================================================================
// [asmjit::AutoLock]
// ============================================================================

//! \internal
//!
//! Scoped lock.
struct AutoLock {
  ASMJIT_NO_COPY(AutoLock)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE AutoLock(Lock& target) noexcept : _target(target) {
    _target.lock();
  }

  ASMJIT_INLINE ~AutoLock() noexcept {
    _target.unlock();
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Reference to the `Lock`.
  Lock& _target;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_UTILS_H
