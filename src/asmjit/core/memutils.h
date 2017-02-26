// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_MEMUTILS_H
#define _ASMJIT_CORE_MEMUTILS_H

// [Dependencies]
#include "../core/intutils.h"

ASMJIT_BEGIN_NAMESPACE

namespace MemUtils {

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::MemUtils - Config]
// ============================================================================

static constexpr bool kUnalignedAccess16 = ASMJIT_ARCH_X86 != 0;
static constexpr bool kUnalignedAccess32 = ASMJIT_ARCH_X86 != 0;
static constexpr bool kUnalignedAccess64 = ASMJIT_ARCH_X86 != 0;

// ============================================================================
// [asmjit::MemUtils - Alloc / Release]
// ============================================================================

// ============================================================================
// [asmjit::MemUtils - Read]
// ============================================================================

template<typename T, size_t ALIGNMENT>
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

static inline uint32_t readU8(const void* p) noexcept { return uint32_t(static_cast<const uint8_t*>(p)[0]); }
static inline int32_t readI8(const void* p) noexcept { return int32_t(static_cast<const int8_t*>(p)[0]); }

template<size_t ALIGNMENT>
static inline uint32_t readU16xLE(const void* p) noexcept {
  if (ASMJIT_ARCH_LE && (kUnalignedAccess16 || ALIGNMENT >= 2)) {
    typedef typename AlignedInt<uint16_t, ALIGNMENT>::T U16AlignedToN;
    return uint32_t(static_cast<const U16AlignedToN*>(p)[0]);
  }
  else {
    uint32_t x = uint32_t(static_cast<const uint8_t*>(p)[0]);
    uint32_t y = uint32_t(static_cast<const uint8_t*>(p)[1]);
    return x | (y << 8);
  }
}

template<size_t ALIGNMENT>
static inline uint32_t readU16xBE(const void* p) noexcept {
  if (ASMJIT_ARCH_BE && (kUnalignedAccess16 || ALIGNMENT >= 2)) {
    typedef typename AlignedInt<uint16_t, ALIGNMENT>::T U16AlignedToN;
    return uint32_t(static_cast<const U16AlignedToN*>(p)[0]);
  }
  else {
    uint32_t x = uint32_t(static_cast<const uint8_t*>(p)[0]);
    uint32_t y = uint32_t(static_cast<const uint8_t*>(p)[1]);
    return (x << 8) | y;
  }
}

template<size_t ALIGNMENT>
static inline int32_t readI16xLE(const void* p) noexcept {
  if (ASMJIT_ARCH_LE && (kUnalignedAccess16 || ALIGNMENT >= 2)) {
    typedef typename AlignedInt<uint16_t, ALIGNMENT>::T U16AlignedToN;
    return int32_t(int16_t(static_cast<const U16AlignedToN*>(p)[0]));
  }
  else {
    int32_t x = int32_t(static_cast<const uint8_t*>(p)[0]);
    int32_t y = int32_t(static_cast<const int8_t*>(p)[1]);
    return x | (y << 8);
  }
}

template<size_t ALIGNMENT>
static inline int32_t readI16xBE(const void* p) noexcept {
  if (ASMJIT_ARCH_BE && (kUnalignedAccess16 || ALIGNMENT >= 2)) {
    typedef typename AlignedInt<uint16_t, ALIGNMENT>::T U16AlignedToN;
    return int32_t(int16_t(static_cast<const U16AlignedToN*>(p)[0]));
  }
  else {
    int32_t x = int32_t(static_cast<const int8_t*>(p)[0]);
    int32_t y = int32_t(static_cast<const uint8_t*>(p)[1]);
    return (x << 8) | y;
  }
}

template<size_t ALIGNMENT>
static inline uint32_t readU16x(const void* p) noexcept { return ASMJIT_ARCH_LE ? readU16xLE<ALIGNMENT>(p) : readU16xBE<ALIGNMENT>(p); }

template<size_t ALIGNMENT>
static inline int32_t readI16x(const void* p) noexcept { return ASMJIT_ARCH_LE ? readI16xLE<ALIGNMENT>(p) : readI16xBE<ALIGNMENT>(p); }

static inline uint32_t readU16aLE(const void* p) noexcept { return readU16xLE<2>(p); }
static inline uint32_t readU16uLE(const void* p) noexcept { return readU16xLE<1>(p); }

static inline uint32_t readU16aBE(const void* p) noexcept { return readU16xBE<2>(p); }
static inline uint32_t readU16uBE(const void* p) noexcept { return readU16xBE<1>(p); }

static inline uint32_t readU16a(const void* p) noexcept { return readU16x<2>(p); }
static inline uint32_t readU16u(const void* p) noexcept { return readU16x<1>(p); }

static inline int32_t readI16aLE(const void* p) noexcept { return readI16xLE<2>(p); }
static inline int32_t readI16uLE(const void* p) noexcept { return readI16xLE<1>(p); }

static inline int32_t readI16aBE(const void* p) noexcept { return readI16xBE<2>(p); }
static inline int32_t readI16uBE(const void* p) noexcept { return readI16xBE<1>(p); }

static inline int32_t readI16a(const void* p) noexcept { return readI16x<2>(p); }
static inline int32_t readI16u(const void* p) noexcept { return readI16x<1>(p); }

template<size_t ALIGNMENT>
static inline uint32_t readU32xLE(const void* p) noexcept {
  if (kUnalignedAccess32 || ALIGNMENT >= 4) {
    typedef typename AlignedInt<uint32_t, ALIGNMENT>::T U32AlignedToN;
    uint32_t x = static_cast<const U32AlignedToN*>(p)[0];
    return ASMJIT_ARCH_LE ? x : IntUtils::byteswap32(x);
  }
  else {
    uint32_t x = readU16xLE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<const uint8_t*>(p) + 0);
    uint32_t y = readU16xLE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<const uint8_t*>(p) + 2);
    return x + (y << 16);
  }
}

template<size_t ALIGNMENT>
static inline uint32_t readU32xBE(const void* p) noexcept {
  if (kUnalignedAccess32 || ALIGNMENT >= 4) {
    typedef typename AlignedInt<uint32_t, ALIGNMENT>::T U32AlignedToN;
    uint32_t x = static_cast<const U32AlignedToN*>(p)[0];
    return ASMJIT_ARCH_BE ? x : IntUtils::byteswap32(x);
  }
  else {
    uint32_t x = readU16xBE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<const uint8_t*>(p) + 0);
    uint32_t y = readU16xBE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<const uint8_t*>(p) + 2);
    return (x << 16) + y;
  }
}

template<size_t ALIGNMENT>
static inline int32_t readI32xLE(const void* p) noexcept { return int32_t(readU32xLE<ALIGNMENT>(p)); }

template<size_t ALIGNMENT>
static inline int32_t readI32xBE(const void* p) noexcept { return int32_t(readU32xBE<ALIGNMENT>(p)); }

template<size_t ALIGNMENT>
static inline uint32_t readU32x(const void* p) noexcept { return ASMJIT_ARCH_LE ? readU32xLE<ALIGNMENT>(p) : readU32xBE<ALIGNMENT>(p); }

template<size_t ALIGNMENT>
static inline int32_t readI32x(const void* p) noexcept { return ASMJIT_ARCH_LE ? readI32xLE<ALIGNMENT>(p) : readI32xBE<ALIGNMENT>(p); }

static inline uint32_t readU32a(const void* p) noexcept { return readU32x<4>(p); }
static inline uint32_t readU32u(const void* p) noexcept { return readU32x<1>(p); }

static inline uint32_t readU32aLE(const void* p) noexcept { return readU32xLE<4>(p); }
static inline uint32_t readU32uLE(const void* p) noexcept { return readU32xLE<1>(p); }

static inline uint32_t readU32aBE(const void* p) noexcept { return readU32xBE<4>(p); }
static inline uint32_t readU32uBE(const void* p) noexcept { return readU32xBE<1>(p); }

static inline int32_t readI32a(const void* p) noexcept { return readI32x<4>(p); }
static inline int32_t readI32u(const void* p) noexcept { return readI32x<1>(p); }

static inline int32_t readI32aLE(const void* p) noexcept { return readI32xLE<4>(p); }
static inline int32_t readI32uLE(const void* p) noexcept { return readI32xLE<1>(p); }

static inline int32_t readI32aBE(const void* p) noexcept { return readI32xBE<4>(p); }
static inline int32_t readI32uBE(const void* p) noexcept { return readI32xBE<1>(p); }

template<size_t ALIGNMENT>
static inline uint64_t readU64xLE(const void* p) noexcept {
  if (ASMJIT_ARCH_LE && (kUnalignedAccess64 || ALIGNMENT >= 8)) {
    typedef typename AlignedInt<uint64_t, ALIGNMENT>::T U64AlignedToN;
    return static_cast<const U64AlignedToN*>(p)[0];
  }
  else {
    uint32_t x = readU32xLE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<const uint8_t*>(p) + 0);
    uint32_t y = readU32xLE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<const uint8_t*>(p) + 4);
    return uint64_t(x) | (uint64_t(y) << 32);
  }
}

template<size_t ALIGNMENT>
static inline uint64_t readU64xBE(const void* p) noexcept {
  if (ASMJIT_ARCH_BE && (kUnalignedAccess64 || ALIGNMENT >= 8)) {
    typedef typename AlignedInt<uint64_t, ALIGNMENT>::T U64AlignedToN;
    return static_cast<const U64AlignedToN*>(p)[0];
  }
  else {
    uint32_t x = readU32xBE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<const uint8_t*>(p) + 0);
    uint32_t y = readU32xBE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<const uint8_t*>(p) + 4);
    return (uint64_t(x) << 32) | uint64_t(y);
  }
}

template<size_t ALIGNMENT>
static inline int64_t readI64xLE(const void* p) noexcept { return int64_t(readU64xLE<ALIGNMENT>(p)); }

template<size_t ALIGNMENT>
static inline int64_t readI64xBE(const void* p) noexcept { return int64_t(readU64xBE<ALIGNMENT>(p)); }

template<size_t ALIGNMENT>
static inline uint64_t readU64x(const void* p) noexcept { return ASMJIT_ARCH_LE ? readU64xLE<ALIGNMENT>(p) : readU64xBE<ALIGNMENT>(p); }

template<size_t ALIGNMENT>
static inline int64_t readI64x(const void* p) noexcept { return ASMJIT_ARCH_LE ? readI64xLE<ALIGNMENT>(p) : readI64xBE<ALIGNMENT>(p); }

static inline uint64_t readU64a(const void* p) noexcept { return readU64x<8>(p); }
static inline uint64_t readU64u(const void* p) noexcept { return readU64x<1>(p); }

static inline uint64_t readU64aLE(const void* p) noexcept { return readU64xLE<8>(p); }
static inline uint64_t readU64uLE(const void* p) noexcept { return readU64xLE<1>(p); }

static inline uint64_t readU64aBE(const void* p) noexcept { return readU64xBE<8>(p); }
static inline uint64_t readU64uBE(const void* p) noexcept { return readU64xBE<1>(p); }

static inline int64_t readI64a(const void* p) noexcept { return readI64x<8>(p); }
static inline int64_t readI64u(const void* p) noexcept { return readI64x<1>(p); }

static inline int64_t readI64aLE(const void* p) noexcept { return readI64xLE<8>(p); }
static inline int64_t readI64uLE(const void* p) noexcept { return readI64xLE<1>(p); }

static inline int64_t readI64aBE(const void* p) noexcept { return readI64xBE<8>(p); }
static inline int64_t readI64uBE(const void* p) noexcept { return readI64xBE<1>(p); }

// ============================================================================
// [asmjit::MemUtils - Write]
// ============================================================================

static inline void writeU8(void* p, uint32_t x) noexcept { static_cast<uint8_t*>(p)[0] = uint8_t(x & 0xFFU); }
static inline void writeI8(void* p, int32_t x) noexcept { static_cast<uint8_t*>(p)[0] = uint8_t(x & 0xFF); }

template<size_t ALIGNMENT>
static inline void writeU16xLE(void* p, uint32_t x) noexcept {
  if (ASMJIT_ARCH_LE && (kUnalignedAccess16 || ALIGNMENT >= 2)) {
    typedef typename AlignedInt<uint16_t, ALIGNMENT>::T U16AlignedToN;
    static_cast<U16AlignedToN*>(p)[0] = uint16_t(x & 0xFFFFU);
  }
  else {
    static_cast<uint8_t*>(p)[0] = uint8_t((x     ) & 0xFFU);
    static_cast<uint8_t*>(p)[1] = uint8_t((x >> 8) & 0xFFU);
  }
}

template<size_t ALIGNMENT>
static inline void writeU16xBE(void* p, uint32_t x) noexcept {
  if (ASMJIT_ARCH_BE && (kUnalignedAccess16 || ALIGNMENT >= 2)) {
    typedef typename AlignedInt<uint16_t, ALIGNMENT>::T U16AlignedToN;
    static_cast<U16AlignedToN*>(p)[0] = uint16_t(x & 0xFFFFU);
  }
  else {
    static_cast<uint8_t*>(p)[0] = uint8_t((x >> 8) & 0xFFU);
    static_cast<uint8_t*>(p)[1] = uint8_t((x     ) & 0xFFU);
  }
}

template<size_t ALIGNMENT>
static inline void writeU16x(void* p, uint32_t x) noexcept {
  if (ASMJIT_ARCH_LE)
    writeU16xLE<ALIGNMENT>(p, x);
  else
    writeU16xBE<ALIGNMENT>(p, x);
}

template<size_t ALIGNMENT>
static inline void writeI16xLE(void* p, int32_t x) noexcept { writeU16xLE<ALIGNMENT>(p, uint32_t(x)); }

template<size_t ALIGNMENT>
static inline void writeI16xBE(void* p, int32_t x) noexcept { writeU16xBE<ALIGNMENT>(p, uint32_t(x)); }

template<size_t ALIGNMENT>
static inline void writeI16x(void* p, int32_t x) noexcept { writeU16x<ALIGNMENT>(p, uint32_t(x)); }

static inline void writeU16aLE(void* p, uint32_t x) noexcept { writeU16xLE<2>(p, x); }
static inline void writeU16uLE(void* p, uint32_t x) noexcept { writeU16xLE<1>(p, x); }

static inline void writeU16aBE(void* p, uint32_t x) noexcept { writeU16xBE<2>(p, x); }
static inline void writeU16uBE(void* p, uint32_t x) noexcept { writeU16xBE<1>(p, x); }

static inline void writeU16a(void* p, uint32_t x) noexcept { writeU16x<2>(p, x); }
static inline void writeU16u(void* p, uint32_t x) noexcept { writeU16x<1>(p, x); }

static inline void writeI16aLE(void* p, int32_t x) noexcept { writeI16xLE<2>(p, x); }
static inline void writeI16uLE(void* p, int32_t x) noexcept { writeI16xLE<1>(p, x); }

static inline void writeI16aBE(void* p, int32_t x) noexcept { writeI16xBE<2>(p, x); }
static inline void writeI16uBE(void* p, int32_t x) noexcept { writeI16xBE<1>(p, x); }

static inline void writeI16a(void* p, int32_t x) noexcept { writeI16x<2>(p, x); }
static inline void writeI16u(void* p, int32_t x) noexcept { writeI16x<1>(p, x); }

template<size_t ALIGNMENT>
static inline void writeU32xLE(void* p, uint32_t x) noexcept {
  if (kUnalignedAccess32 || ALIGNMENT >= 4) {
    typedef typename AlignedInt<uint32_t, ALIGNMENT>::T U32AlignedToN;
    static_cast<U32AlignedToN*>(p)[0] = ASMJIT_ARCH_LE ? x : IntUtils::byteswap32(x);
  }
  else {
    writeU16xLE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<uint8_t*>(p) + 0, x);
    writeU16xLE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<uint8_t*>(p) + 2, x >> 16);
  }
}

template<size_t ALIGNMENT>
static inline void writeU32xBE(void* p, uint32_t x) noexcept {
  if (kUnalignedAccess32 || ALIGNMENT >= 4) {
    typedef typename AlignedInt<uint32_t, ALIGNMENT>::T U32AlignedToN;
    static_cast<U32AlignedToN*>(p)[0] = ASMJIT_ARCH_BE ? x : IntUtils::byteswap32(x);
  }
  else {
    writeU16xBE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<uint8_t*>(p) + 0, x >> 16);
    writeU16xBE<ALIGNMENT < 4 ? ALIGNMENT : size_t(2)>(static_cast<uint8_t*>(p) + 2, x);
  }
}

template<size_t ALIGNMENT>
static inline void writeU32x(void* p, uint32_t x) noexcept {
  if (ASMJIT_ARCH_LE)
    writeU32xLE<ALIGNMENT>(p, x);
  else
    writeU32xBE<ALIGNMENT>(p, x);
}

template<size_t ALIGNMENT>
static inline void writeI32xLE(void* p, int32_t x) noexcept { writeU32xLE<ALIGNMENT>(p, uint32_t(x)); }

template<size_t ALIGNMENT>
static inline void writeI32xBE(void* p, int32_t x) noexcept { writeU32xBE<ALIGNMENT>(p, uint32_t(x)); }

template<size_t ALIGNMENT>
static inline void writeI32x(void* p, int32_t x) noexcept { writeU32x<ALIGNMENT>(p, uint32_t(x)); }

static inline void writeU32aLE(void* p, uint32_t x) noexcept { writeU32xLE<4>(p, x); }
static inline void writeU32uLE(void* p, uint32_t x) noexcept { writeU32xLE<1>(p, x); }

static inline void writeU32aBE(void* p, uint32_t x) noexcept { writeU32xBE<4>(p, x); }
static inline void writeU32uBE(void* p, uint32_t x) noexcept { writeU32xBE<1>(p, x); }

static inline void writeU32a(void* p, uint32_t x) noexcept { writeU32x<4>(p, x); }
static inline void writeU32u(void* p, uint32_t x) noexcept { writeU32x<1>(p, x); }

static inline void writeI32aLE(void* p, int32_t x) noexcept { writeI32xLE<4>(p, x); }
static inline void writeI32uLE(void* p, int32_t x) noexcept { writeI32xLE<1>(p, x); }

static inline void writeI32aBE(void* p, int32_t x) noexcept { writeI32xBE<4>(p, x); }
static inline void writeI32uBE(void* p, int32_t x) noexcept { writeI32xBE<1>(p, x); }

static inline void writeI32a(void* p, int32_t x) noexcept { writeI32x<4>(p, x); }
static inline void writeI32u(void* p, int32_t x) noexcept { writeI32x<1>(p, x); }

template<size_t ALIGNMENT>
static inline void writeU64xLE(void* p, uint64_t x) noexcept {
  if (ASMJIT_ARCH_LE && (kUnalignedAccess64 || ALIGNMENT >= 8)) {
    typedef typename AlignedInt<uint64_t, ALIGNMENT>::T U64AlignedToN;
    static_cast<U64AlignedToN*>(p)[0] = x;
  }
  else {
    writeU32xLE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<uint8_t*>(p) + 0, uint32_t(x & 0xFFFFFFFFU));
    writeU32xLE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<uint8_t*>(p) + 4, uint32_t(x >> 32));
  }
}

template<size_t ALIGNMENT>
static inline void writeU64xBE(void* p, uint64_t x) noexcept {
  if (ASMJIT_ARCH_BE && (kUnalignedAccess64 || ALIGNMENT >= 8)) {
    typedef typename AlignedInt<uint64_t, ALIGNMENT>::T U64AlignedToN;
    static_cast<U64AlignedToN*>(p)[0] = x;
  }
  else {
    writeU32xBE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<uint8_t*>(p) + 0, uint32_t(x >> 32));
    writeU32xBE<ALIGNMENT < 8 ? ALIGNMENT : size_t(4)>(static_cast<uint8_t*>(p) + 4, uint32_t(x & 0xFFFFFFFFU));
  }
}

template<size_t ALIGNMENT>
static inline void writeU64x(void* p, uint64_t x) noexcept {
  if (ASMJIT_ARCH_LE)
    writeU64xLE<ALIGNMENT>(p, x);
  else
    writeU64xBE<ALIGNMENT>(p, x);
}

template<size_t ALIGNMENT>
static inline void writeI64xLE(void* p, int64_t x) noexcept { writeU64xLE<ALIGNMENT>(p, uint64_t(x)); }

template<size_t ALIGNMENT>
static inline void writeI64xBE(void* p, int64_t x) noexcept { writeU64xBE<ALIGNMENT>(p, uint64_t(x)); }

template<size_t ALIGNMENT>
static inline void writeI64x(void* p, int64_t x) noexcept { writeU64x<ALIGNMENT>(p, uint64_t(x)); }

static inline void writeU64aLE(void* p, uint64_t x) noexcept { writeU64xLE<8>(p, x); }
static inline void writeU64uLE(void* p, uint64_t x) noexcept { writeU64xLE<1>(p, x); }

static inline void writeU64aBE(void* p, uint64_t x) noexcept { writeU64xBE<8>(p, x); }
static inline void writeU64uBE(void* p, uint64_t x) noexcept { writeU64xBE<1>(p, x); }

static inline void writeU64a(void* p, uint64_t x) noexcept { writeU64x<8>(p, x); }
static inline void writeU64u(void* p, uint64_t x) noexcept { writeU64x<1>(p, x); }

static inline void writeI64aLE(void* p, int64_t x) noexcept { writeI64xLE<8>(p, x); }
static inline void writeI64uLE(void* p, int64_t x) noexcept { writeI64xLE<1>(p, x); }

static inline void writeI64aBE(void* p, int64_t x) noexcept { writeI64xBE<8>(p, x); }
static inline void writeI64uBE(void* p, int64_t x) noexcept { writeI64xBE<1>(p, x); }

static inline void writeI64a(void* p, int64_t x) noexcept { writeI64x<8>(p, x); }
static inline void writeI64u(void* p, int64_t x) noexcept { writeI64x<1>(p, x); }

//! \}

} // MemUtils namespace

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_MEMUTILS_H
