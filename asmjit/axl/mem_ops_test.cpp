// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>
#if defined(ASMJIT_TEST)

#include <asmjit/axl/mem_ops.h>

ASMJIT_BEGIN_NAMESPACE

TEST_CASE(axl_memops_load_store) {
  uint8_t arr[32] = { 0 };

  axl::storeu_u16_be(arr + 1, 0x0102u);
  axl::storeu_u16_be(arr + 3, 0x0304u);
  EXPECT_EQ(axl::loadu_u32_be(arr + 1), 0x01020304u);
  EXPECT_EQ(axl::loadu_u32_le(arr + 1), 0x04030201u);
  EXPECT_EQ(axl::loadu_u32_be(arr + 2), 0x02030400u);
  EXPECT_EQ(axl::loadu_u32_le(arr + 2), 0x00040302u);

  axl::storeu_u32_le(arr + 5, 0x05060708u);
  EXPECT_EQ(axl::loadu_u64_be(arr + 1), 0x0102030408070605u);
  EXPECT_EQ(axl::loadu_u64_le(arr + 1), 0x0506070804030201u);

  axl::storeu_u64_le(arr + 7, 0x1122334455667788u);
  EXPECT_EQ(axl::loadu_u32_be(arr + 8), 0x77665544u);

  double d = 134.44;
  axl::storeu(arr + 3, d);
  EXPECT_EQ(axl::loadu<double>(arr + 3), d);
}

TEST_CASE(axl_memops_fill) {
  uint64_t arr[4] {};

  axl::fill_bytes(arr, 31, uint8_t(0xFFu));
  EXPECT_EQ(axl::load_u8(reinterpret_cast<uint8_t*>(arr) + 0), 0xFFu);
  EXPECT_EQ(axl::load_u8(reinterpret_cast<uint8_t*>(arr) + 30), 0xFFu);
  EXPECT_EQ(axl::load_u8(reinterpret_cast<uint8_t*>(arr) + 31), 0x00u);

  axl::fill_items(reinterpret_cast<uint16_t*>(arr), 4, uint16_t(0x0102u));
  EXPECT_EQ(axl::loadu_u16(reinterpret_cast<uint8_t*>(arr) + 0), uint16_t(0x0102u));
  EXPECT_EQ(axl::loadu_u16(reinterpret_cast<uint8_t*>(arr) + 2), uint16_t(0x0102u));
  EXPECT_EQ(axl::loadu_u16(reinterpret_cast<uint8_t*>(arr) + 4), uint16_t(0x0102u));
  EXPECT_EQ(axl::loadu_u16(reinterpret_cast<uint8_t*>(arr) + 6), uint16_t(0x0102u));
  EXPECT_EQ(axl::loadu_u16(reinterpret_cast<uint8_t*>(arr) + 8), uint16_t(0xFFFFu));

  axl::fill_items(reinterpret_cast<uint32_t*>(arr), 2, uint32_t(0x01020304u));
  EXPECT_EQ(axl::loadu_u32(reinterpret_cast<uint8_t*>(arr) + 0), uint32_t(0x01020304u));
  EXPECT_EQ(axl::loadu_u32(reinterpret_cast<uint8_t*>(arr) + 4), uint32_t(0x01020304u));
  EXPECT_EQ(axl::loadu_u32(reinterpret_cast<uint8_t*>(arr) + 8), uint32_t(0xFFFFFFFFu));

  axl::fill_items(reinterpret_cast<uint64_t*>(arr), 2, uint64_t(0x0102030405060708u));
  EXPECT_EQ(axl::loadu_u64(reinterpret_cast<uint8_t*>(arr) + 0), uint64_t(0x0102030405060708u));
  EXPECT_EQ(axl::loadu_u64(reinterpret_cast<uint8_t*>(arr) + 8), uint64_t(0x0102030405060708u));
  EXPECT_EQ(axl::loadu_u64(reinterpret_cast<uint8_t*>(arr) + 16), uint64_t(0xFFFFFFFFFFFFFFFFu));
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST
