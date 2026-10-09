// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>
#if defined(ASMJIT_TEST)

#include <asmjit/core/code_holder.h>

ASMJIT_BEGIN_NAMESPACE

TEST_CASE(core_code_holder) {
  CodeHolder code;

  TEST_LOG("Verifying CodeHolder::init()");
  Environment env;
  env.init(Arch::kX86);

  code.init(env);
  EXPECT_EQ(code.arch(), Arch::kX86);

  TEST_LOG("Verifying named labels");
  uint32_t dummy_id;
  uint32_t label_id1;
  uint32_t label_id2;

  // Anonymous labels can have no-name (this is basically like calling `code.new_label_id()`).
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "", SIZE_MAX, LabelType::kAnonymous), Error::kOk);

  // Global labels must have a name - not providing one is an error.
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "", SIZE_MAX, LabelType::kGlobal), Error::kInvalidLabelName);

  // A name of a global label cannot repeat.
  EXPECT_EQ(code.new_named_label_id(Out(label_id1), "NamedLabel1", SIZE_MAX, LabelType::kGlobal), Error::kOk);
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "NamedLabel1", SIZE_MAX, LabelType::kGlobal), Error::kLabelAlreadyDefined);
  EXPECT_TRUE(code.is_label_valid(label_id1));
  EXPECT_EQ(code.label_entry_of(label_id1).name_size(), 11u);
  EXPECT_EQ(strcmp(code.label_entry_of(label_id1).name(), "NamedLabel1"), 0);
  EXPECT_EQ(code.label_id_by_name("NamedLabel1"), label_id1);

  EXPECT_EQ(code.new_named_label_id(Out(label_id2), "NamedLabel2", SIZE_MAX, LabelType::kGlobal), Error::kOk);
  EXPECT_EQ(code.new_named_label_id(Out(dummy_id), "NamedLabel2", SIZE_MAX, LabelType::kGlobal), Error::kLabelAlreadyDefined);
  EXPECT_TRUE(code.is_label_valid(label_id2));
  EXPECT_EQ(code.label_entry_of(label_id2).name_size(), 11u);
  EXPECT_EQ(strcmp(code.label_entry_of(label_id2).name(), "NamedLabel2"), 0);
  EXPECT_EQ(code.label_id_by_name("NamedLabel2"), label_id2);

  TEST_LOG("Verifying .text properties");
  Section* section_text = code.text_section();
  EXPECT_TRUE(section_text->has_offset());

  TEST_LOG("Verifying section ordering");
  Section* section_a;
  EXPECT_EQ(code.new_section(Out(section_a), "high-priority", SIZE_MAX, SectionFlags::kNone, 1, -1), Error::kOk);
  EXPECT_EQ(code.sections()[0], section_text);
  EXPECT_EQ(code.sections()[1], section_a);
  EXPECT_EQ(code.sections_by_order()[0], section_text);
  EXPECT_EQ(code.sections_by_order()[1], section_a);
  EXPECT_FALSE(section_a->has_offset());

  Section* section_b;
  EXPECT_EQ(code.new_section(Out(section_b), "higher-priority", SIZE_MAX, SectionFlags::kNone, 1, -2), Error::kOk);
  EXPECT_EQ(code.sections()[0], section_text);
  EXPECT_EQ(code.sections()[1], section_a);
  EXPECT_EQ(code.sections()[2], section_b);
  EXPECT_EQ(code.sections_by_order()[0], section_text);
  EXPECT_EQ(code.sections_by_order()[1], section_b);
  EXPECT_EQ(code.sections_by_order()[2], section_a);
  EXPECT_FALSE(section_b->has_offset());

  Section* section_c;
  EXPECT_EQ(code.new_section(Out(section_c), "low-priority", SIZE_MAX, SectionFlags::kNone, 1, 2), Error::kOk);
  EXPECT_EQ(code.sections()[0], section_text);
  EXPECT_EQ(code.sections()[1], section_a);
  EXPECT_EQ(code.sections()[2], section_b);
  EXPECT_EQ(code.sections()[3], section_c);
  EXPECT_EQ(code.sections_by_order()[0], section_text);
  EXPECT_EQ(code.sections_by_order()[1], section_b);
  EXPECT_EQ(code.sections_by_order()[2], section_a);
  EXPECT_EQ(code.sections_by_order()[3], section_c);
  EXPECT_FALSE(section_c->has_offset());
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST
