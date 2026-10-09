// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>
#if defined(ASMJIT_TEST)

#include <asmjit/axl/commons.h>
#include <asmjit/core/string.h>

ASMJIT_BEGIN_NAMESPACE

static void test_string_grow() noexcept {
  String s;
  size_t c = s.capacity();

  TEST_LOG("Testing string grow strategy (SSO capacity: %zu)", c);
  for (size_t i = 0; i < 1000000; i++) {
    s.append('x');
    if (s.capacity() != c) {
      c = s.capacity();
      TEST_LOG("  String reallocated to new capacity: %zu", c);
    }
  }

  // We don't expect a 1 million character string to occupy 4MiB, for example. So verify that!
  EXPECT_LT(c, size_t(4 * 1024 * 1024));
}

TEST_CASE(axl_string) {
  String s;

  TEST_LOG("Testing string functionality");

  EXPECT_FALSE(s.is_large_or_external());
  EXPECT_FALSE(s.is_external());

  EXPECT_EQ(s.assign('a'), Error::kOk);
  EXPECT_EQ(s.size(), 1u);
  EXPECT_EQ(s.capacity(), String::kSSOCapacity);
  EXPECT_EQ(s.data()[0], 'a');
  EXPECT_EQ(s.data()[1], '\0');
  EXPECT_TRUE(s.equals("a"));
  EXPECT_TRUE(s.equals("a", 1));

  EXPECT_EQ(s.assign_chars('b', 4), Error::kOk);
  EXPECT_EQ(s.size(), 4u);
  EXPECT_EQ(s.capacity(), String::kSSOCapacity);
  EXPECT_EQ(s.data()[0], 'b');
  EXPECT_EQ(s.data()[1], 'b');
  EXPECT_EQ(s.data()[2], 'b');
  EXPECT_EQ(s.data()[3], 'b');
  EXPECT_EQ(s.data()[4], '\0');
  EXPECT_TRUE(s.equals("bbbb"));
  EXPECT_TRUE(s.equals("bbbb", 4));

  EXPECT_EQ(s.assign("abc"), Error::kOk);
  EXPECT_EQ(s.size(), 3u);
  EXPECT_EQ(s.capacity(), String::kSSOCapacity);
  EXPECT_EQ(s.data()[0], 'a');
  EXPECT_EQ(s.data()[1], 'b');
  EXPECT_EQ(s.data()[2], 'c');
  EXPECT_EQ(s.data()[3], '\0');
  EXPECT_TRUE(s.equals("abc"));
  EXPECT_TRUE(s.equals("abc", 3));

  const char* large = "Large string that will not fit into SSO buffer";
  EXPECT_EQ(s.assign(large), Error::kOk);
  EXPECT_TRUE(s.is_large_or_external());
  EXPECT_EQ(s.size(), strlen(large));
  EXPECT_GT(s.capacity(), String::kSSOCapacity);
  EXPECT_TRUE(s.equals(large));
  EXPECT_TRUE(s.equals(large, strlen(large)));

  const char* additional = " (additional content)";
  EXPECT_TRUE(s.is_large_or_external());
  EXPECT_EQ(s.append(additional), Error::kOk);
  EXPECT_EQ(s.size(), strlen(large) + strlen(additional));

  EXPECT_EQ(s.clear(), Error::kOk);
  EXPECT_EQ(s.size(), 0u);
  EXPECT_TRUE(s.is_empty());
  EXPECT_EQ(s.data()[0], '\0');
  EXPECT_TRUE(s.is_large_or_external()); // Clear should never release the memory.

  EXPECT_EQ(s.append_uint(1234), Error::kOk);
  EXPECT_TRUE(s.equals("1234"));

  EXPECT_EQ(s.assign_uint(0xFFFF, 16, 0, StringFormatFlags::kAlternate), Error::kOk);
  EXPECT_TRUE(s.equals("0xFFFF"));

  StringTmp<64> s_tmp;
  EXPECT_TRUE(s_tmp.is_large_or_external());
  EXPECT_TRUE(s_tmp.is_external());
  EXPECT_EQ(s_tmp.append_chars(' ', 1000), Error::kOk);
  EXPECT_FALSE(s_tmp.is_external());

  test_string_grow();
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST
