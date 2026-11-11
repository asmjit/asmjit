// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api_build_test_p.h>
#if defined(ASMJIT_TEST)

#include <asmjit/axl/inplace_string.h>

ASMJIT_BEGIN_NAMESPACE

TEST_CASE(axl_inplace_string) {
  axl::InplaceString<3> s;
  const char* strings[] = {"", "a", "ab", "abc"};

  for (const char* str : strings) {
    EXPECT_OK(s.assign(str));
    EXPECT_EQ(s.size(), strlen(str));
    EXPECT_TRUE(s.equals(str));

    axl::InplaceString<3> copy(Span<const char>(str, strlen(str)));
    EXPECT_EQ(copy.size(), strlen(str));
    EXPECT_TRUE(copy.equals(str));
  }

  EXPECT_OK(s.clear());
  EXPECT_TRUE(s.is_empty());
  EXPECT_TRUE(s.equals(""));
  EXPECT_OK(s.assign("ab"));
  EXPECT_OK(s.append("c"));
  EXPECT_EQ(s.size(), 3u);
  EXPECT_TRUE(s.equals("abc"));
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST
