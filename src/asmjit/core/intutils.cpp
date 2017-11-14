// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/intutils.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::IntUtils - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(core_intutils) {
  uint32_t i;

  INFO("IntUtils::shl() / shr()");
  EXPECT(IntUtils::shl<int32_t>(0x00001111, 16) == int32_t(0x11110000U));
  EXPECT(IntUtils::shl<uint32_t>(0x00001111, 16) == uint32_t(0x11110000U));
  EXPECT(IntUtils::shr<int32_t>(0x11110000U, 16) == int32_t(0x00001111U));
  EXPECT(IntUtils::shr<uint32_t>(0x11110000U, 16) == uint32_t(0x00001111U));
  EXPECT(IntUtils::sar<int32_t>(0xFFFF0000U, 16) == int32_t(0xFFFFFFFFU));
  EXPECT(IntUtils::sar<uint32_t>(0xFFFF0000U, 16) == uint32_t(0xFFFFFFFFU));

  INFO("IntUtils::blsi()");
  for (i = 0; i < 32; i++) EXPECT(IntUtils::blsi(uint32_t(1) << i) == uint32_t(1) << i);
  for (i = 0; i < 31; i++) EXPECT(IntUtils::blsi(uint32_t(3) << i) == uint32_t(1) << i);
  for (i = 0; i < 64; i++) EXPECT(IntUtils::blsi(uint64_t(1) << i) == uint64_t(1) << i);
  for (i = 0; i < 63; i++) EXPECT(IntUtils::blsi(uint64_t(3) << i) == uint64_t(1) << i);

  INFO("IntUtils::ctz()");
  for (i = 0; i < 32; i++) EXPECT(IntUtils::ctz(uint32_t(1) << i) == i);
  for (i = 0; i < 64; i++) EXPECT(IntUtils::ctz(uint64_t(1) << i) == i);
  for (i = 0; i < 32; i++) EXPECT(IntUtils::Internal::ctzGeneric(uint32_t(1) << i) == i);
  for (i = 0; i < 64; i++) EXPECT(IntUtils::Internal::ctzGeneric(uint64_t(1) << i) == i);

  INFO("IntUtils::mask()");
  EXPECT(IntUtils::mask(0, 1, 7) == 0x83U);
  for (i = 0; i < 32; i++)
    EXPECT(IntUtils::mask(i) == (1U << i));

  INFO("IntUtils::bitTest()");
  for (i = 0; i < 32; i++) {
    EXPECT(IntUtils::bitTest((1 << i), i) == true, "IntUtils::bitTest(%X, %u) should return true", (1 << i), i);
  }

  INFO("IntUtils::lsbMask()");
  for (i = 0; i < 32; i++) {
    uint32_t expectedBits = 0;
    for (uint32_t b = 0; b < i; b++)
      expectedBits |= uint32_t(1) << b;
    EXPECT(IntUtils::lsbMask<uint32_t>(i) == expectedBits);
  }

  INFO("IntUtils::popcnt()");
  for (i = 0; i < 32; i++) EXPECT(IntUtils::popcnt((uint32_t(1) << i)) == 1);
  for (i = 0; i < 64; i++) EXPECT(IntUtils::popcnt((uint64_t(1) << i)) == 1);
  EXPECT(IntUtils::popcnt(0x000000F0) ==  4);
  EXPECT(IntUtils::popcnt(0x10101010) ==  4);
  EXPECT(IntUtils::popcnt(0xFF000000) ==  8);
  EXPECT(IntUtils::popcnt(0xFFFFFFF7) == 31);
  EXPECT(IntUtils::popcnt(0x7FFFFFFF) == 31);

  INFO("IntUtils::isPowerOf2()");
  for (i = 0; i < 64; i++) {
    EXPECT(IntUtils::isPowerOf2(uint64_t(1) << i) == true);
    EXPECT(IntUtils::isPowerOf2((uint64_t(1) << i) ^ 0x001101) == false);
  }

  INFO("IntUtils::isAligned()");
  EXPECT(IntUtils::isAligned<size_t>(0xFFFF,  4) == false);
  EXPECT(IntUtils::isAligned<size_t>(0xFFF4,  4) == true);
  EXPECT(IntUtils::isAligned<size_t>(0xFFF8,  8) == true);
  EXPECT(IntUtils::isAligned<size_t>(0xFFF0, 16) == true);

  INFO("IntUtils::alignUp()");
  EXPECT(IntUtils::alignUp<size_t>(0xFFFF,  4) == 0x10000);
  EXPECT(IntUtils::alignUp<size_t>(0xFFF4,  4) == 0x0FFF4);
  EXPECT(IntUtils::alignUp<size_t>(0xFFF8,  8) == 0x0FFF8);
  EXPECT(IntUtils::alignUp<size_t>(0xFFF0, 16) == 0x0FFF0);
  EXPECT(IntUtils::alignUp<size_t>(0xFFF0, 32) == 0x10000);

  INFO("IntUtils::alignUpDiff()");
  EXPECT(IntUtils::alignUpDiff<size_t>(0xFFFF,  4) == 1);
  EXPECT(IntUtils::alignUpDiff<size_t>(0xFFF4,  4) == 0);
  EXPECT(IntUtils::alignUpDiff<size_t>(0xFFF8,  8) == 0);
  EXPECT(IntUtils::alignUpDiff<size_t>(0xFFF0, 16) == 0);
  EXPECT(IntUtils::alignUpDiff<size_t>(0xFFF0, 32) == 16);

  INFO("IntUtils::alignUpPowerOf2()");
  EXPECT(IntUtils::alignUpPowerOf2<size_t>(0x0000) == 0x00000);
  EXPECT(IntUtils::alignUpPowerOf2<size_t>(0xFFFF) == 0x10000);
  EXPECT(IntUtils::alignUpPowerOf2<size_t>(0xF123) == 0x10000);
  EXPECT(IntUtils::alignUpPowerOf2<size_t>(0x0F00) == 0x01000);
  EXPECT(IntUtils::alignUpPowerOf2<size_t>(0x0100) == 0x00100);
  EXPECT(IntUtils::alignUpPowerOf2<size_t>(0x1001) == 0x02000);

  INFO("IntUtils::byteswap()");
  EXPECT(IntUtils::byteswap32(int32_t(0x01020304)) == int32_t(0x04030201));
  EXPECT(IntUtils::byteswap32(uint32_t(0x01020304)) == uint32_t(0x04030201));

  INFO("IntUtils::bytepack()");
  union BytePackData {
    uint8_t bytes[4];
    uint32_t u32;
  } bpdata;

  bpdata.u32 = IntUtils::bytepack32_4x8(0x00, 0x11, 0x22, 0x33);
  EXPECT(bpdata.bytes[0] == 0x00);
  EXPECT(bpdata.bytes[1] == 0x11);
  EXPECT(bpdata.bytes[2] == 0x22);
  EXPECT(bpdata.bytes[3] == 0x33);

  INFO("IntUtils::isBetween()");
  EXPECT(IntUtils::isBetween<int>(11 , 10, 20) == true);
  EXPECT(IntUtils::isBetween<int>(101, 10, 20) == false);

  INFO("IntUtils::isInt8()");
  EXPECT(IntUtils::isInt8(-128) == true);
  EXPECT(IntUtils::isInt8( 127) == true);
  EXPECT(IntUtils::isInt8(-129) == false);
  EXPECT(IntUtils::isInt8( 128) == false);

  INFO("IntUtils::isInt16()");
  EXPECT(IntUtils::isInt16(-32768) == true);
  EXPECT(IntUtils::isInt16( 32767) == true);
  EXPECT(IntUtils::isInt16(-32769) == false);
  EXPECT(IntUtils::isInt16( 32768) == false);

  INFO("IntUtils::isInt32()");
  EXPECT(IntUtils::isInt32( 2147483647    ) == true);
  EXPECT(IntUtils::isInt32(-2147483647 - 1) == true);
  EXPECT(IntUtils::isInt32(uint64_t(2147483648U)) == false);
  EXPECT(IntUtils::isInt32(uint64_t(0xFFFFFFFFU)) == false);
  EXPECT(IntUtils::isInt32(uint64_t(0xFFFFFFFFU) + 1) == false);

  INFO("IntUtils::isUInt8()");
  EXPECT(IntUtils::isUInt8(0)   == true);
  EXPECT(IntUtils::isUInt8(255) == true);
  EXPECT(IntUtils::isUInt8(256) == false);
  EXPECT(IntUtils::isUInt8(-1)  == false);

  INFO("IntUtils::isUInt12()");
  EXPECT(IntUtils::isUInt12(0)    == true);
  EXPECT(IntUtils::isUInt12(4095) == true);
  EXPECT(IntUtils::isUInt12(4096) == false);
  EXPECT(IntUtils::isUInt12(-1)   == false);

  INFO("IntUtils::isUInt16()");
  EXPECT(IntUtils::isUInt16(0)     == true);
  EXPECT(IntUtils::isUInt16(65535) == true);
  EXPECT(IntUtils::isUInt16(65536) == false);
  EXPECT(IntUtils::isUInt16(-1)    == false);

  INFO("IntUtils::isUInt32()");
  EXPECT(IntUtils::isUInt32(uint64_t(0xFFFFFFFF)) == true);
  EXPECT(IntUtils::isUInt32(uint64_t(0xFFFFFFFF) + 1) == false);
  EXPECT(IntUtils::isUInt32(-1) == false);

  INFO("IntUtils::bitVectorOp");
  {
    uint32_t vec[3] = { 0 };
    IntUtils::bitVectorFill(vec, 1, 64);
    EXPECT(vec[0] == 0xFFFFFFFEU);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0x00000001U);

    IntUtils::bitVectorClear(vec, 1, 1);
    EXPECT(vec[0] == 0xFFFFFFFCU);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0x00000001U);

    IntUtils::bitVectorFill(vec, 0, 32);
    EXPECT(vec[0] == 0xFFFFFFFFU);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0x00000001U);

    IntUtils::bitVectorClear(vec, 0, 32);
    EXPECT(vec[0] == 0x00000000U);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0x00000001U);

    IntUtils::bitVectorFill(vec, 1, 30);
    EXPECT(vec[0] == 0x7FFFFFFEU);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0x00000001U);

    IntUtils::bitVectorClear(vec, 1, 95);
    EXPECT(vec[0] == 0x00000000U);
    EXPECT(vec[1] == 0x00000000U);
    EXPECT(vec[2] == 0x00000000U);

    IntUtils::bitVectorFill(vec, 32, 64);
    EXPECT(vec[0] == 0x00000000U);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0xFFFFFFFFU);

    IntUtils::bitVectorSetBit(vec, 1, true);
    EXPECT(vec[0] == 0x00000002U);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0xFFFFFFFFU);

    IntUtils::bitVectorSetBit(vec, 95, false);
    EXPECT(vec[0] == 0x00000002U);
    EXPECT(vec[1] == 0xFFFFFFFFU);
    EXPECT(vec[2] == 0x7FFFFFFFU);

    IntUtils::bitVectorClear(vec, 33, 32);
    EXPECT(vec[0] == 0x00000002U);
    EXPECT(vec[1] == 0x00000001U);
    EXPECT(vec[2] == 0x7FFFFFFEU);
  }

  INFO("IntUtils::bitVectorIndexOf");
  {
    uint32_t vec1[1] = { 0x80000000 };
    EXPECT(IntUtils::bitVectorIndexOf(vec1, 0, true) == 31);
    EXPECT(IntUtils::bitVectorIndexOf(vec1, 1, true) == 31);
    EXPECT(IntUtils::bitVectorIndexOf(vec1, 31, true) == 31);

    uint32_t vec2[2] = { 0x00000000, 0x80000000 };
    EXPECT(IntUtils::bitVectorIndexOf(vec2, 0, true) == 63);
    EXPECT(IntUtils::bitVectorIndexOf(vec2, 1, true) == 63);
    EXPECT(IntUtils::bitVectorIndexOf(vec2, 31, true) == 63);
    EXPECT(IntUtils::bitVectorIndexOf(vec2, 32, true) == 63);
    EXPECT(IntUtils::bitVectorIndexOf(vec2, 33, true) == 63);
    EXPECT(IntUtils::bitVectorIndexOf(vec2, 63, true) == 63);

    uint32_t vec3[3] = { 0x00000001, 0x00000000, 0x80000000 };
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 0, true) == 0);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 1, true) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 2, true) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 31, true) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 32, true) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 63, true) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 64, true) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec3, 95, true) == 95);

    uint32_t vec4[3] = { ~vec3[0], ~vec3[1], ~vec3[2] };
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 0, false) == 0);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 1, false) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 2, false) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 31, false) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 32, false) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 63, false) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 64, false) == 95);
    EXPECT(IntUtils::bitVectorIndexOf(vec4, 95, false) == 95);
  }

  INFO("IntUtils::BitWordIterator<uint32_t>");
  {
    IntUtils::BitWordIterator<uint32_t> it(0x80000F01U);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 0);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 8);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 9);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 10);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 11);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 31);
    EXPECT(!it.hasNext());

    // No bits set.
    it.init(0x00000000U);
    ASMJIT_ASSERT(!it.hasNext());

    // Only first bit set.
    it.init(0x00000001U);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 0);
    ASMJIT_ASSERT(!it.hasNext());

    // Only last bit set (special case).
    it.init(0x80000000U);
    ASMJIT_ASSERT(it.hasNext());
    ASMJIT_ASSERT(it.next() == 31);
    ASMJIT_ASSERT(!it.hasNext());
  }

  INFO("IntUtils::BitWordIterator<uint64_t>");
  {
    IntUtils::BitWordIterator<uint64_t> it(uint64_t(1) << 63);
    ASMJIT_ASSERT(it.hasNext());
    ASMJIT_ASSERT(it.next() == 63);
    ASMJIT_ASSERT(!it.hasNext());
  }

  INFO("IntUtils::BitVectorIterator<uint32_t>");
  {
    // Border cases.
    static const uint32_t bitsNone[] = { 0xFFFFFFFFU };
    IntUtils::BitVectorIterator<uint32_t> it(bitsNone, 0);

    EXPECT(!it.hasNext());
    it.init(bitsNone, 0, 1);
    EXPECT(!it.hasNext());
    it.init(bitsNone, 0, 128);
    EXPECT(!it.hasNext());

    static const uint32_t bits1[] = { 0x80000008U, 0x80000001U, 0x00000000U, 0x80000000U, 0x00000000U, 0x00000000U, 0x00003000U };
    it.init(bits1, ASMJIT_ARRAY_SIZE(bits1));

    EXPECT(it.hasNext());
    EXPECT(it.next() == 3);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 31);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 32);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 63);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 127);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 204);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 205);
    EXPECT(!it.hasNext());

    it.init(bits1, ASMJIT_ARRAY_SIZE(bits1), 4);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 31);

    it.init(bits1, ASMJIT_ARRAY_SIZE(bits1), 64);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 127);

    it.init(bits1, ASMJIT_ARRAY_SIZE(bits1), 127);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 127);

    static const uint32_t bits2[] = { 0x80000000U, 0x80000000U, 0x00000000U, 0x80000000U };
    it.init(bits2, ASMJIT_ARRAY_SIZE(bits2));

    EXPECT(it.hasNext());
    EXPECT(it.next() == 31);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 63);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 127);
    EXPECT(!it.hasNext());

    static const uint32_t bits3[] = { 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U };
    it.init(bits3, ASMJIT_ARRAY_SIZE(bits3));
    EXPECT(!it.hasNext());

    static const uint32_t bits4[] = { 0x00000000U, 0x00000000U, 0x00000000U, 0x80000000U };
    it.init(bits4, ASMJIT_ARRAY_SIZE(bits4));
    EXPECT(it.hasNext());
    EXPECT(it.next() == 127);
    EXPECT(!it.hasNext());
  }

  INFO("IntUtils::BitVectorIterator<uint64_t>");
  {
    static const uint64_t bits1[] = { 0x80000000U, 0x80000000U, 0x00000000U, 0x80000000U };
    IntUtils::BitVectorIterator<uint64_t> it(bits1, ASMJIT_ARRAY_SIZE(bits1));

    EXPECT(it.hasNext());
    EXPECT(it.next() == 31);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 95);
    EXPECT(it.hasNext());
    EXPECT(it.next() == 223);
    EXPECT(!it.hasNext());

    static const uint64_t bits2[] = { 0x8000000000000000U, 0, 0, 0 };
    it.init(bits2, ASMJIT_ARRAY_SIZE(bits2));

    EXPECT(it.hasNext());
    EXPECT(it.next() == 63);
    EXPECT(!it.hasNext());
  }

  INFO("IntUtils::BitVectorFlipIterator<uint32_t>");
  {
    static const uint32_t bits[] = { 0x80000000U, 0x80000000U, 0x00000000U, 0x80000000U };
    IntUtils::BitVectorFlipIterator<uint32_t> it(bits, ASMJIT_ARRAY_SIZE(bits));

    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 31);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 32);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 63);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 64);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 127);
    EXPECT(!it.hasNext());
  }

  INFO("IntUtils::BitVectorFlipIterator<uint64_t>");
  {
    static const uint64_t bits[] = { 0xFFFFFFFFFFFFFFFFU, 0xFFFFFFFFFFFFFFFF, 0, 0 };
    IntUtils::BitVectorFlipIterator<uint64_t> it(bits, ASMJIT_ARRAY_SIZE(bits));

    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 0);
    EXPECT(it.hasNext());
    EXPECT(it.nextAndFlip() == 128);
    EXPECT(!it.hasNext());
  }
}
#endif

ASMJIT_END_NAMESPACE
