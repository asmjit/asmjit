// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include <asmjit/asmjit.h>

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

// ============================================================================
// [EXPECT]
// ============================================================================

static void expectFailed(const char* msg) {
  printf("Failure: %s\n", msg);
  abort();
}

#define EXPECT(_Exp_, _Msg_) \
  do { \
    if (!(_Exp_)) { \
      expectFailed(_Msg_); \
    } \
  } while(0)

// ============================================================================
// [Main]
// ============================================================================

int main(int argc, char* argv[]) {
  Zone zone(16192);
  ConstPool pool(&zone);

  uint32_t i;
  uint32_t kCount = 1000000;

  printf("Adding %u constants to the pool.\n", kCount);
  {
    size_t prevOffset;
    size_t curOffset;
    uint64_t c = ASMJIT_UINT64_C(0x0101010101010101);

    EXPECT(pool.add(&c, 8, prevOffset) == kErrorOk,
      "pool.add() - Returned error.");
    EXPECT(prevOffset == 0,
      "pool.add() - First constant should have zero offset.");

    for (i = 1; i < kCount; i++) {
      c++;
      EXPECT(pool.add(&c, 8, curOffset) == kErrorOk,
        "pool.add() - Returned error.");
      EXPECT(prevOffset + 8 == curOffset,
        "pool.add() - Returned incorrect curOffset.");
      EXPECT(pool.getSize() == (i + 1) * 8,
        "pool.getSize() - Reports incorrect size.");
      prevOffset = curOffset;
    }

    EXPECT(pool.getAlignment() == 8,
      "pool.getAlignment() - Expected 8-byte alignment.");
  }
  printf("Done.\n");

  printf("Retrieving %u constants from the pool.\n", kCount);
  {
    uint64_t c = ASMJIT_UINT64_C(0x0101010101010101);

    for (i = 0; i < kCount; i++) {
      size_t offset;
      EXPECT(pool.add(&c, 8, offset) == kErrorOk,
        "pool.add() - Returned error.");
      EXPECT(offset == i * 8,
        "pool.add() - Should have reused constant.");
      c++;
    }
  }
  printf("Done.\n");

  printf("Checking if the constants were split into 4-byte patterns.\n");
  {
    uint32_t c = 0x01010101;
    for (i = 0; i < kCount; i++) {
      size_t offset;
      EXPECT(pool.add(&c, 4, offset) == kErrorOk,
        "pool.add() - Returned error.");
      EXPECT(offset == i * 8,
        "pool.add() - Should reuse existing constant.");
      c++;
    }
  }
  printf("Done.\n");

  printf("Adding 2 byte constant to misalign the current offset.\n");
  {
    uint16_t c = 0xFFFF;
    size_t offset;

    EXPECT(pool.add(&c, 2, offset) == kErrorOk,
      "pool.add() - Returned error.");
    EXPECT(offset == kCount * 8,
      "pool.add() - Didn't return expected position.");
    EXPECT(pool.getAlignment() == 8,
      "pool.getAlignment() - Expected 8-byte alignment.");
  }
  printf("Done.\n");

  printf("Adding 8 byte constant to check if pool gets aligned again.\n");
  {
    uint64_t c = ASMJIT_UINT64_C(0xFFFFFFFFFFFFFFFF);
    size_t offset;

    EXPECT(pool.add(&c, 8, offset) == kErrorOk,
      "pool.add() - Returned error.");
    EXPECT(offset == kCount * 8 + 8,
      "pool.add() - Didn't return aligned offset.");
  }
  printf("Done.\n");

  printf("Adding 2 byte constant verify the gap is filled.\n");
  {
    uint16_t c = 0xFFFE;
    size_t offset;

    EXPECT(pool.add(&c, 2, offset) == kErrorOk,
      "pool.add() - Returned error.");
    EXPECT(offset == kCount * 8 + 2,
      "pool.add() - Didn't fill the gap.");
    EXPECT(pool.getAlignment() == 8,
      "pool.getAlignment() - Expected 8-byte alignment.");
  }
  printf("Done.\n");

  printf("Checking reset functionality.\n");
  {
    pool.reset();

    EXPECT(pool.getSize() == 0,
      "pool.getSize() - Expected pool size to be zero.");
    EXPECT(pool.getAlignment() == 0,
      "pool.getSize() - Expected pool alignment to be zero.");
  }
  printf("Done.\n");

  printf("Checking pool alignment when combined constants are added.\n");
  {
    uint8_t bytes[32] = { 0 };
    uint64_t c = 0;
    size_t offset;

    pool.add(bytes, 1, offset);

    EXPECT(pool.getSize() == 1,
      "pool.getSize() - Expected pool size to be 1 byte.");
    EXPECT(pool.getAlignment() == 1,
      "pool.getSize() - Expected pool alignment to be 1 byte.");
    EXPECT(offset == 0,
      "pool.getSize() - Expected offset returned to be zero.");

    pool.add(bytes, 2, offset);

    EXPECT(pool.getSize() == 4,
      "pool.getSize() - Expected pool size to be 4 bytes.");
    EXPECT(pool.getAlignment() == 2,
      "pool.getSize() - Expected pool alignment to be 2 bytes.");
    EXPECT(offset == 2,
      "pool.getSize() - Expected offset returned to be 2.");

    pool.add(bytes, 4, offset);

    EXPECT(pool.getSize() == 8,
      "pool.getSize() - Expected pool size to be 8 bytes.");
    EXPECT(pool.getAlignment() == 4,
      "pool.getSize() - Expected pool alignment to be 4 bytes.");
    EXPECT(offset == 4,
      "pool.getSize() - Expected offset returned to be 4.");

    pool.add(bytes, 4, offset);

    EXPECT(pool.getSize() == 8,
      "pool.getSize() - Expected pool size to be 8 bytes.");
    EXPECT(pool.getAlignment() == 4,
      "pool.getSize() - Expected pool alignment to be 4 bytes.");
    EXPECT(offset == 4,
      "pool.getSize() - Expected offset returned to be 8.");

    pool.add(bytes, 32, offset);
    EXPECT(pool.getSize() == 64,
      "pool.getSize() - Expected pool size to be 64 bytes.");
    EXPECT(pool.getAlignment() == 32,
      "pool.getSize() - Expected pool alignment to be 32 bytes.");
    EXPECT(offset == 32,
      "pool.getSize() - Expected offset returned to be 32.");
  }
  printf("Done.\n");

  return 0;
}
