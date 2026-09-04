// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>
#if defined(ASMJIT_TEST)

#include <asmjit/axl/bit_set_utils.h>

ASMJIT_BEGIN_NAMESPACE

TEST_CASE(axl_bit_set_utils) {
  TEST_LOG("axl::bit_vector_op");
  {
    uint32_t vec[3] = { 0 };
    axl::bit_vector_fill(vec, 1, 64);
    EXPECT_EQ(vec[0], 0xFFFFFFFEu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    axl::bit_vector_clear(vec, 1, 1);
    EXPECT_EQ(vec[0], 0xFFFFFFFCu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    axl::bit_vector_fill(vec, 0, 32);
    EXPECT_EQ(vec[0], 0xFFFFFFFFu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    axl::bit_vector_clear(vec, 0, 32);
    EXPECT_EQ(vec[0], 0x00000000u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    axl::bit_vector_fill(vec, 1, 30);
    EXPECT_EQ(vec[0], 0x7FFFFFFEu);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x00000001u);

    axl::bit_vector_clear(vec, 1, 95);
    EXPECT_EQ(vec[0], 0x00000000u);
    EXPECT_EQ(vec[1], 0x00000000u);
    EXPECT_EQ(vec[2], 0x00000000u);

    axl::bit_vector_fill(vec, 32, 64);
    EXPECT_EQ(vec[0], 0x00000000u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0xFFFFFFFFu);

    axl::bit_vector_set_bit(vec, 1, true);
    EXPECT_EQ(vec[0], 0x00000002u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0xFFFFFFFFu);

    axl::bit_vector_set_bit(vec, 95, false);
    EXPECT_EQ(vec[0], 0x00000002u);
    EXPECT_EQ(vec[1], 0xFFFFFFFFu);
    EXPECT_EQ(vec[2], 0x7FFFFFFFu);

    axl::bit_vector_clear(vec, 33, 32);
    EXPECT_EQ(vec[0], 0x00000002u);
    EXPECT_EQ(vec[1], 0x00000001u);
    EXPECT_EQ(vec[2], 0x7FFFFFFEu);
  }

  TEST_LOG("axl::bit_vector_index_of");
  {
    uint32_t vec1[1] = { 0x80000000 };
    EXPECT_EQ(axl::bit_vector_index_of(vec1, 0, true), 31u);
    EXPECT_EQ(axl::bit_vector_index_of(vec1, 1, true), 31u);
    EXPECT_EQ(axl::bit_vector_index_of(vec1, 31, true), 31u);

    uint32_t vec2[2] = { 0x00000000, 0x80000000 };
    EXPECT_EQ(axl::bit_vector_index_of(vec2, 0, true), 63u);
    EXPECT_EQ(axl::bit_vector_index_of(vec2, 1, true), 63u);
    EXPECT_EQ(axl::bit_vector_index_of(vec2, 31, true), 63u);
    EXPECT_EQ(axl::bit_vector_index_of(vec2, 32, true), 63u);
    EXPECT_EQ(axl::bit_vector_index_of(vec2, 33, true), 63u);
    EXPECT_EQ(axl::bit_vector_index_of(vec2, 63, true), 63u);

    uint32_t vec3[3] = { 0x00000001, 0x00000000, 0x80000000 };
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 0, true), 0u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 1, true), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 2, true), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 31, true), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 32, true), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 63, true), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 64, true), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec3, 95, true), 95u);

    uint32_t vec4[3] = { ~vec3[0], ~vec3[1], ~vec3[2] };
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 0, false), 0u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 1, false), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 2, false), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 31, false), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 32, false), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 63, false), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 64, false), 95u);
    EXPECT_EQ(axl::bit_vector_index_of(vec4, 95, false), 95u);
  }

  TEST_LOG("axl::BitWordIterator<uint32_t>");
  {
    axl::BitWordIterator<uint32_t> it(0x80000F01u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 0u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 8u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 9u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 10u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 11u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_FALSE(it.has_next());

    // No bits set.
    it.init(0x00000000u);
    EXPECT_FALSE(it.has_next());

    // Only first bit set.
    it.init(0x00000001u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 0u);
    EXPECT_FALSE(it.has_next());

    // Only last bit set (special case).
    it.init(0x80000000u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_FALSE(it.has_next());
  }

  TEST_LOG("axl::BitWordIterator<uint64_t>");
  {
    axl::BitWordIterator<uint64_t> it(uint64_t(1) << 63);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_FALSE(it.has_next());
  }

  TEST_LOG("axl::BitVectorIterator<uint32_t>");
  {
    // Border cases.
    static const uint32_t bits_none[] = { 0xFFFFFFFFu };
    axl::BitVectorIterator<uint32_t> it(Span<const uint32_t>(bits_none, 0));

    EXPECT_FALSE(it.has_next());
    it.init(Span<const uint32_t>(bits_none, 0), 1);
    EXPECT_FALSE(it.has_next());
    it.init(Span<const uint32_t>(bits_none, 0), 128);
    EXPECT_FALSE(it.has_next());

    static const uint32_t bits1[] = { 0x80000008u, 0x80000001u, 0x00000000u, 0x80000000u, 0x00000000u, 0x00000000u, 0x00003000u };
    it.init(Span<const uint32_t>::from_array(bits1));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 3u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 32u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 204u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 205u);
    EXPECT_FALSE(it.has_next());

    it.init(Span<const uint32_t>::from_array(bits1), 4);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);

    it.init(Span<const uint32_t>::from_array(bits1), 64);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);

    it.init(Span<const uint32_t>::from_array(bits1), 127);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);

    static const uint32_t bits2[] = { 0x80000000u, 0x80000000u, 0x00000000u, 0x80000000u };
    it.init(Span<const uint32_t>::from_array(bits2));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);
    EXPECT_FALSE(it.has_next());

    static const uint32_t bits3[] = { 0x00000000u, 0x00000000u, 0x00000000u, 0x00000000u };
    it.init(Span<const uint32_t>::from_array(bits3));
    EXPECT_FALSE(it.has_next());

    static const uint32_t bits4[] = { 0x00000000u, 0x00000000u, 0x00000000u, 0x80000000u };
    it.init(Span<const uint32_t>::from_array(bits4));
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 127u);
    EXPECT_FALSE(it.has_next());
  }

  TEST_LOG("axl::BitVectorIterator<uint64_t>");
  {
    static const uint64_t bits1[] = { 0x80000000u, 0x80000000u, 0x00000000u, 0x80000000u };
    axl::BitVectorIterator<uint64_t> it(Span<const uint64_t>::from_array(bits1));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 31u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 95u);
    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 223u);
    EXPECT_FALSE(it.has_next());

    static const uint64_t bits2[] = { 0x8000000000000000u, 0, 0, 0 };
    it.init(Span<const uint64_t>::from_array(bits2));

    EXPECT_TRUE(it.has_next());
    EXPECT_EQ(it.next(), 63u);
    EXPECT_FALSE(it.has_next());
  }
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST
