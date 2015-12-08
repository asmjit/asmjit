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

#if defined(_MSC_VER) && _MSC_VER >= 1400
# include <intrin.h>
# pragma intrinsic(_BitScanForward)
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
  static ASMJIT_INLINE T minValue() {
    if (kIsSigned)
      return static_cast<T>((~static_cast<UnsignedType>(0) >> 1) + static_cast<UnsignedType>(1));
    else
      return static_cast<T>(0);
  }

  //! Get a maximum value of `T`.
  static ASMJIT_INLINE T maxValue() {
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

  //! Bit-cast `float` to 32-bit integer.
  static ASMJIT_INLINE int32_t floatAsInt(float f) { FloatBits m; m.f = f; return m.i; }
  //! Bit-cast 32-bit integer to `float`.
  static ASMJIT_INLINE float intAsFloat(int32_t i) { FloatBits m; m.i = i; return m.f; }

  //! Bit-cast `double` to 64-bit integer.
  static ASMJIT_INLINE int64_t doubleAsInt(double d) { DoubleBits m; m.d = d; return m.i; }
  //! Bit-cast 64-bit integer to `double`.
  static ASMJIT_INLINE double intAsDouble(int64_t i) { DoubleBits m; m.i = i; return m.d; }

  // --------------------------------------------------------------------------
  // [Pack / Unpack]
  // --------------------------------------------------------------------------

  //! Pack two 8-bit integer and one 16-bit integer into a 32-bit integer as it
  //! is an array of `{u0,u1,w2}`.
  static ASMJIT_INLINE uint32_t pack32_2x8_1x16(uint32_t u0, uint32_t u1, uint32_t w2) {
    return ASMJIT_ARCH_LE ? u0 + (u1 << 8) + (w2 << 16)
                          : (u0 << 24) + (u1 << 16) + w2;
  }

  //! Pack four 8-bit integer into a 32-bit integer as it is an array of `{u0,u1,u2,u3}`.
  static ASMJIT_INLINE uint32_t pack32_4x8(uint32_t u0, uint32_t u1, uint32_t u2, uint32_t u3) {
    return ASMJIT_ARCH_LE ? u0 + (u1 << 8) + (u2 << 16) + (u3 << 24)
                          : (u0 << 24) + (u1 << 16) + (u2 << 8) + u3;
  }

  //! Pack two 32-bit integer into a 64-bit integer as it is an array of `{u0,u1}`.
  static ASMJIT_INLINE uint64_t pack64_2x32(uint32_t u0, uint32_t u1) {
    return ASMJIT_ARCH_LE ? (static_cast<uint64_t>(u1) << 32) + u0
                          : (static_cast<uint64_t>(u0) << 32) + u1;
  }

  // --------------------------------------------------------------------------
  // [Min/Max]
  // --------------------------------------------------------------------------

  // NOTE: Because some environments declare min() and max() as macros, it has
  // been decided to use different name so we never collide with them.

  //! Get minimum value of `a` and `b`.
  template<typename T>
  static ASMJIT_INLINE T iMin(const T& a, const T& b) { return a < b ? a : b; }

  //! Get maximum value of `a` and `b`.
  template<typename T>
  static ASMJIT_INLINE T iMax(const T& a, const T& b) { return a > b ? a : b; }

  // --------------------------------------------------------------------------
  // [InInterval]
  // --------------------------------------------------------------------------

  //! Get whether `x` is greater than or equal to `a` and lesses than or equal to `b`.
  template<typename T>
  static ASMJIT_INLINE bool inInterval(T x, T a, T b) {
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
  static ASMJIT_INLINE typename IntTraits<T>::IntType asInt(T x) {
    return static_cast<typename IntTraits<T>::IntType>(x);
  }

  // --------------------------------------------------------------------------
  // [IsInt / IsUInt]
  // --------------------------------------------------------------------------

  //! Get whether the given integer `x` can be casted to an 8-bit signed integer.
  template<typename T>
  static ASMJIT_INLINE bool isInt8(T x) {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return sizeof(T) <= 1 || inInterval<SignedType>(SignedType(x), -128, 127);
    else
      return UnsignedType(x) <= UnsignedType(127U);
  }

  //! Get whether the given integer `x` can be casted to a 16-bit signed integer.
  template<typename T>
  static ASMJIT_INLINE bool isInt16(T x) {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return sizeof(T) <= 2 || inInterval<SignedType>(SignedType(x), -32768, 32767);
    else
      return sizeof(T) <= 1 || UnsignedType(x) <= UnsignedType(32767U);
  }

  //! Get whether the given integer `x` can be casted to a 32-bit signed integer.
  template<typename T>
  static ASMJIT_INLINE bool isInt32(T x) {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return sizeof(T) <= 4 || inInterval<SignedType>(SignedType(x), -2147483647 - 1, 2147483647);
    else
      return sizeof(T) <= 2 || UnsignedType(x) <= UnsignedType(2147483647U);
  }

  //! Get whether the given integer `x` can be casted to an 8-bit unsigned integer.
  template<typename T>
  static ASMJIT_INLINE bool isUInt8(T x) {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 1 ? true : x <= T(255));
    else
      return sizeof(T) <= 1 || UnsignedType(x) <= UnsignedType(255U);
  }

  //! Get whether the given integer `x` can be casted to a 12-bit unsigned integer (ARM specific).
  template<typename T>
  static ASMJIT_INLINE bool isUInt12(T x) {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 1 ? true : x <= T(4095));
    else
      return sizeof(T) <= 1 || UnsignedType(x) <= UnsignedType(4095U);
  }

  //! Get whether the given integer `x` can be casted to a 16-bit unsigned integer.
  template<typename T>
  static ASMJIT_INLINE bool isUInt16(T x) {
    typedef typename IntTraits<T>::SignedType SignedType;
    typedef typename IntTraits<T>::UnsignedType UnsignedType;

    if (IntTraits<T>::kIsSigned)
      return x >= T(0) && (sizeof(T) <= 2 ? true : x <= T(65535));
    else
      return sizeof(T) <= 2 || UnsignedType(x) <= UnsignedType(65535U);
  }

  //! Get whether the given integer `x` can be casted to a 32-bit unsigned integer.
  template<typename T>
  static ASMJIT_INLINE bool isUInt32(T x) {
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
  static ASMJIT_INLINE bool isPowerOf2(T n) {
    return n != 0 && (n & (n - 1)) == 0;
  }

  // --------------------------------------------------------------------------
  // [Mask]
  // --------------------------------------------------------------------------

  //! Generate a bit-mask that has `x` bit set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x) {
    ASMJIT_ASSERT(x < 32);
    return static_cast<uint32_t>(1) << x;
  }

  //! Generate a bit-mask that has `x0` and `x1` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1) {
    return mask(x0) | mask(x1);
  }

  //! Generate a bit-mask that has `x0`, `x1` and `x2` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2) {
    return mask(x0) | mask(x1) | mask(x2);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2` and `x3` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3);
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3` and `x4` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3) | mask(x4) ;
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4` and `x5` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3) | mask(x4) | mask(x5) ;
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5` and `x6` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3) | mask(x4) | mask(x5) | mask(x6) ;
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5`, `x6` and `x7` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3) | mask(x4) | mask(x5) | mask(x6) | mask(x7) ;
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5`, `x6`, `x7` and `x8` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7, uint32_t x8) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3) | mask(x4) | mask(x5) | mask(x6) | mask(x7) | mask(x8) ;
  }

  //! Generate a bit-mask that has `x0`, `x1`, `x2`, `x3`, `x4`, `x5`, `x6`, `x7`, `x8` and `x9` bits set.
  static ASMJIT_INLINE uint32_t mask(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5, uint32_t x6, uint32_t x7, uint32_t x8, uint32_t x9) {
    return mask(x0) | mask(x1) | mask(x2) | mask(x3) | mask(x4) | mask(x5) | mask(x6) | mask(x7) | mask(x8) | mask(x9) ;
  }

  // --------------------------------------------------------------------------
  // [Bits]
  // --------------------------------------------------------------------------

  //! Generate a bit-mask that has `x` most significant bits set.
  static ASMJIT_INLINE uint32_t bits(uint32_t x) {
    // Shifting more bits that the type has has undefined behavior. Everything
    // we need is that application shouldn't crash because of that, but the
    // content of register after shift is not defined. So in case that the
    // requested shift is too large for the type we correct this undefined
    // behavior by setting all bits to ones (this is why we generate an overflow
    // mask).
    uint32_t overflow = static_cast<uint32_t>(
      -static_cast<int32_t>(x >= sizeof(uint32_t) * 8));

    return ((static_cast<uint32_t>(1) << x) - 1U) | overflow;
  }

  // --------------------------------------------------------------------------
  // [HasBit]
  // --------------------------------------------------------------------------

  //! Get whether `x` has bit `n` set.
  static ASMJIT_INLINE bool hasBit(uint32_t x, uint32_t n) {
    return (x & (static_cast<uint32_t>(1) << n)) != 0;
  }

  // --------------------------------------------------------------------------
  // [BitCount]
  // --------------------------------------------------------------------------

  //! Get count of bits in `x`.
  //!
  //! Taken from http://graphics.stanford.edu/~seander/bithacks.html .
  static ASMJIT_INLINE uint32_t bitCount(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
    return (((x + (x >> 4)) & 0x0F0F0F0FU) * 0x01010101U) >> 24;
  }

  // --------------------------------------------------------------------------
  // [FindFirstBit]
  // --------------------------------------------------------------------------

  //! \internal
  static ASMJIT_INLINE uint32_t findFirstBitSlow(uint32_t mask) {
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
  static ASMJIT_INLINE uint32_t findFirstBit(uint32_t mask) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
    DWORD i;
    if (_BitScanForward(&i, mask)) {
      ASMJIT_ASSERT(findFirstBitSlow(mask) == i);
      return static_cast<uint32_t>(i);
    }
    return 0xFFFFFFFFU;
#else
    return findFirstBitSlow(mask);
#endif
  }

  // --------------------------------------------------------------------------
  // [Misc]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE uint32_t keepNOnesFromRight(uint32_t mask, uint32_t nBits) {
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

  static ASMJIT_INLINE uint32_t indexNOnesFromRight(uint8_t* dst, uint32_t mask, uint32_t nBits) {
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
  static ASMJIT_INLINE bool isAligned(T base, T alignment) {
    return (base % alignment) == 0;
  }

  //! Align `base` to `alignment`.
  template<typename T>
  static ASMJIT_INLINE T alignTo(T base, T alignment) {
    return (base + (alignment - 1)) & ~(alignment - 1);
  }

  template<typename T>
  static ASMJIT_INLINE T alignToPowerOf2(T base) {
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
  static ASMJIT_INLINE T alignDiff(T base, T alignment) {
    return alignTo(base, alignment) - base;
  }

  // --------------------------------------------------------------------------
  // [String]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE size_t strLen(const char* s, size_t maxlen) {
    size_t i;
    for (i = 0; i < maxlen; i++)
      if (!s[i])
        break;
    return i;
  }

  // --------------------------------------------------------------------------
  // [CpuTicks]
  // --------------------------------------------------------------------------

  //! Get the current CPU tick count, used for benchmarking (1ms resolution).
  static ASMJIT_API uint32_t getTickCount();
};

// ============================================================================
// [asmjit::UInt64]
// ============================================================================

union UInt64 {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64 fromUInt64(uint64_t val) {
    UInt64 data;
    data.setUInt64(val);
    return data;
  }

  ASMJIT_INLINE UInt64 fromUInt64(const UInt64& val) {
    UInt64 data;
    data.setUInt64(val);
    return data;
  }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE void reset() {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE uint64_t getUInt64() const {
    return u64;
  }

  ASMJIT_INLINE UInt64& setUInt64(uint64_t val) {
    u64 = val;
    return *this;
  }

  ASMJIT_INLINE UInt64& setUInt64(const UInt64& val) {
    if (kArchHost64Bit) {
      u64 = val.u64;
    }
    else {
      u32[0] = val.u32[0];
      u32[1] = val.u32[1];
    }
    return *this;
  }

  ASMJIT_INLINE UInt64& setPacked_2x32(uint32_t u0, uint32_t u1) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE UInt64& add(uint64_t val) {
    u64 += val;
    return *this;
  }

  ASMJIT_INLINE UInt64& add(const UInt64& val) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE UInt64& sub(uint64_t val) {
    u64 -= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& sub(const UInt64& val) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE UInt64& and_(uint64_t val) {
    u64 &= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& and_(const UInt64& val) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE UInt64& andNot(uint64_t val) {
    u64 &= ~val;
    return *this;
  }

  ASMJIT_INLINE UInt64& andNot(const UInt64& val) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE UInt64& or_(uint64_t val) {
    u64 |= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& or_(const UInt64& val) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE UInt64& xor_(uint64_t val) {
    u64 ^= val;
    return *this;
  }

  ASMJIT_INLINE UInt64& xor_(const UInt64& val) {
    if (kArchHost64Bit) {
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

  ASMJIT_INLINE bool isZero() const {
    return kArchHost64Bit ? u64 == 0 : (u32[0] | u32[1]) == 0;
  }

  ASMJIT_INLINE bool isNonZero() const {
    return kArchHost64Bit ? u64 != 0 : (u32[0] | u32[1]) != 0;
  }

  ASMJIT_INLINE bool eq(uint64_t val) const {
    return u64 == val;
  }

  ASMJIT_INLINE bool eq(const UInt64& val) const {
    return kArchHost64Bit ? u64 == val.u64 : (u32[0] == val.u32[0]) & (u32[1] == val.u32[1]);
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE UInt64& operator+=(uint64_t val) { return add(val); }
  ASMJIT_INLINE UInt64& operator+=(const UInt64& val) { return add(val); }

  ASMJIT_INLINE UInt64& operator-=(uint64_t val) { return sub(val); }
  ASMJIT_INLINE UInt64& operator-=(const UInt64& val) { return sub(val); }

  ASMJIT_INLINE UInt64& operator&=(uint64_t val) { return and_(val); }
  ASMJIT_INLINE UInt64& operator&=(const UInt64& val) { return and_(val); }

  ASMJIT_INLINE UInt64& operator|=(uint64_t val) { return or_(val); }
  ASMJIT_INLINE UInt64& operator|=(const UInt64& val) { return or_(val); }

  ASMJIT_INLINE UInt64& operator^=(uint64_t val) { return xor_(val); }
  ASMJIT_INLINE UInt64& operator^=(const UInt64& val) { return xor_(val); }

  ASMJIT_INLINE bool operator==(uint64_t val) const { return eq(val); }
  ASMJIT_INLINE bool operator==(const UInt64& val) const { return eq(val); }

  ASMJIT_INLINE bool operator!=(uint64_t val) const { return !eq(val); }
  ASMJIT_INLINE bool operator!=(const UInt64& val) const { return !eq(val); }

  ASMJIT_INLINE bool operator<(uint64_t val) const { return u64 < val; }
  ASMJIT_INLINE bool operator<(const UInt64& val) const { return u64 < val.u64; }

  ASMJIT_INLINE bool operator<=(uint64_t val) const { return u64 <= val; }
  ASMJIT_INLINE bool operator<=(const UInt64& val) const { return u64 <= val.u64; }

  ASMJIT_INLINE bool operator>(uint64_t val) const { return u64 > val; }
  ASMJIT_INLINE bool operator>(const UInt64& val) const { return u64 > val.u64; }

  ASMJIT_INLINE bool operator>=(uint64_t val) const { return u64 >= val; }
  ASMJIT_INLINE bool operator>=(const UInt64& val) const { return u64 >= val.u64; }

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
  ASMJIT_INLINE Lock() { InitializeCriticalSection(&_handle); }
  //! Destroy the `Lock` instance.
  ASMJIT_INLINE ~Lock() { DeleteCriticalSection(&_handle); }

  //! Lock.
  ASMJIT_INLINE void lock() { EnterCriticalSection(&_handle); }
  //! Unlock.
  ASMJIT_INLINE void unlock() { LeaveCriticalSection(&_handle); }
#endif // ASMJIT_OS_WINDOWS

  // --------------------------------------------------------------------------
  // [Posix]
  // --------------------------------------------------------------------------

#if ASMJIT_OS_POSIX
  typedef pthread_mutex_t Handle;

  //! Create a new `Lock` instance.
  ASMJIT_INLINE Lock() { pthread_mutex_init(&_handle, NULL); }
  //! Destroy the `Lock` instance.
  ASMJIT_INLINE ~Lock() { pthread_mutex_destroy(&_handle); }

  //! Lock.
  ASMJIT_INLINE void lock() { pthread_mutex_lock(&_handle); }
  //! Unlock.
  ASMJIT_INLINE void unlock() { pthread_mutex_unlock(&_handle); }
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

  //! Lock `target`, scoped.
  ASMJIT_INLINE AutoLock(Lock& target) : _target(target) { _target.lock(); }
  //! Unlock `target`.
  ASMJIT_INLINE ~AutoLock() { _target.unlock(); }

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
