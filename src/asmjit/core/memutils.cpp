// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/memutils.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::MemUtils - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
UNIT(core_memutils) {
  uint8_t arr[32] = { 0 };

  INFO("Testing unaligned read/write and basic endianness conversion");

  MemUtils::writeU16uBE(arr + 1, 0x0102U);
  MemUtils::writeU16uBE(arr + 3, 0x0304U);
  EXPECT(MemUtils::readU32uBE(arr + 1) == 0x01020304U);
  EXPECT(MemUtils::readU32uLE(arr + 1) == 0x04030201U);
  EXPECT(MemUtils::readU32uBE(arr + 2) == 0x02030400U);
  EXPECT(MemUtils::readU32uLE(arr + 2) == 0x00040302U);

  MemUtils::writeU32uLE(arr + 5, 0x05060708U);
  EXPECT(MemUtils::readU64uBE(arr + 1) == 0x0102030408070605U);
  EXPECT(MemUtils::readU64uLE(arr + 1) == 0x0506070804030201U);

  MemUtils::writeU64uLE(arr + 7, 0x1122334455667788U);
  EXPECT(MemUtils::readU32uBE(arr + 8) == 0x77665544U);
}
#endif

ASMJIT_END_NAMESPACE
