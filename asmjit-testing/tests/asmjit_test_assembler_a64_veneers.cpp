// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// ----------------------------------------------------------------------------
// This test demonstrates AArch64 address entry functionality, which allows
// branch instructions (b and bl) to use an address table when the target
// is out of range for a direct branch (±128MB).
// ----------------------------------------------------------------------------

#include <asmjit/core.h>
#if ASMJIT_ARCH_ARM >= ASMJIT_ARCH_ARM64 && !defined(ASMJIT_NO_AARCH64) && !defined(ASMJIT_NO_JIT)

#include <asmjit/a64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

static void fail(const char* message, Error err) {
  printf("** FAILURE: %s (%s) **\n", message, DebugUtils::error_as_string(err));
  exit(1);
}

int main() {
  printf("AsmJit AArch64 Address Entry Test\n\n");

  Environment env = Environment::host();
  JitAllocator allocator;

#ifndef ASMJIT_NO_LOGGING
  FileLogger logger(stdout);
  logger.set_indentation(FormatIndentationGroup::kCode, 2);
#endif

  CodeHolder code;
  code.init(env);

#ifndef ASMJIT_NO_LOGGING
  code.set_logger(&logger);
#endif

  printf("Generating code:\n");
  a64::Assembler a(&code);

  // Create a simple function that branches to an absolute address
  // This simulates a far branch that would be out of range
  FuncDetail func;
  func.init(FuncSignature::build<int>(), code.environment());

  FuncFrame frame;
  frame.init(func);

  a.emit_prolog(frame);

  // Use addresses far enough apart to force veneer creation
  // Direct branch range is ±128MB (0x8000000 bytes)
  // We'll use an address 256MB away to guarantee veneer creation
  uint64_t far_address = 0x0000100000000000ULL;  // 256MB away

  // Create a label for the return path
  Label ret_label = a.new_label();

  // Test 1: Branch with link to far address (should use veneer+address table)
  a.bl(Imm(far_address));

  // Bind return label
  a.bind(ret_label);

  // Return 42 as test value
  a.mov(a64::w0, 42);

  a.emit_epilog(frame);

  // Flatten the code to assign section offsets
  printf("\nFlattening code:\n");
  Error err = code.flatten();
  if (err != Error::kOk)
    fail("Failed to flatten code", err);

  // Print section information BEFORE relocation
  printf("Sections (before relocation):\n");
  for (Section* section : code.sections_by_order()) {
    printf("  [0x%08X %s] {Id=%u Size=%u}\n",
           uint32_t(section->offset()),
           section->name(),
           section->section_id(),
           uint32_t(section->real_size()));
  }

  size_t code_size = code.code_size();
  printf("  Final code size: %zu\n", code_size);

  // Resolve cross-section fixups if any
  if (code.has_unresolved_fixups()) {
    printf("\nResolving cross-section fixups:\n");
    printf("  Before: %zu\n", code.unresolved_fixup_count());

    err = code.resolve_cross_section_fixups();
    if (err != Error::kOk)
      fail("Failed to resolve cross-section fixups", err);

    printf("  After: %zu\n", code.unresolved_fixup_count());
  }

  // Check if address table section was created
  if (code.has_address_table_section()) {
    Section* addrtab = code.address_table_section();
    printf("\nAddress table section created:\n");
    printf("  Offset: 0x%08X\n", uint32_t(addrtab->offset()));
    printf("  Size: %u\n", uint32_t(addrtab->real_size()));
  }

  // Allocate memory for the function and relocate it there
  JitAllocator::Span span;
  err = allocator.alloc(Out(span), code_size);
  if (err != Error::kOk)
    fail("Failed to allocate executable memory", err);

  // Use a base address that's far from our target to force veneer creation
  // We'll use a base address near 0 so that our far_address (256MB away) requires a veneer
  uint64_t base_address = 0x0000000000100000ULL;  // Low address

  printf("\nRelocating to base address: 0x%llX\n", (unsigned long long)base_address);
  printf("  Target branch address: 0x%llX\n", (unsigned long long)far_address);
  printf("  Distance: 0x%llX bytes (%.1f MB)\n",
         (unsigned long long)(far_address - base_address),
         (far_address - base_address) / (1024.0 * 1024.0));

  // Relocate to a base address that will trigger veneer creation
  CodeHolder::RelocationSummary summary;
  err = code.relocate_to_base(base_address, &summary);
  if (err != Error::kOk)
    fail("Failed to relocate code", err);

  printf("  Code size reduction: %zu bytes\n", summary.code_size_reduction);

  // Print section information AFTER relocation to see if veneer was created
  printf("\nSections (after relocation):\n");
  for (Section* section : code.sections_by_order()) {
    printf("  [0x%08X %s] {Id=%u Size=%u}\n",
           uint32_t(section->offset()),
           section->name(),
           section->section_id(),
           uint32_t(section->real_size()));
  }

  // Verify veneer section was created
  bool veneer_section_found = false;
  for (Section* section : code.sections_by_order()) {
    if (strcmp(section->name(), ".veneer") == 0) {
      veneer_section_found = true;
      printf("\n** Veneer section successfully created! **\n");
      printf("  Offset: 0x%08X\n", uint32_t(section->offset()));
      printf("  Size: %u bytes\n", uint32_t(section->real_size()));
      break;
    }
  }

  if (!veneer_section_found) {
    fail("Veneer section was NOT created (test did not exercise veneer path)", Error::kOk);
  }

  // Copy the flattened code to executable memory (using actual allocated span)
  allocator.write(span, [&](JitAllocator::Span& span) noexcept -> Error {
    return code.copy_flattened_data(span.rw(), code_size, CopySectionFlags::kPadTargetBuffer);
  });

  // Note: We can't actually execute this function as it would branch to an invalid address
  // This test is mainly to verify that:
  // 1. The address table section is created
  // 2. The veneer section is created when needed
  // 3. The code is properly relocated
  // 4. No errors occur during the process

  printf("\n** SUCCESS **\n");
  printf("The veneer mechanism was properly set up.\n");
  printf("(Note: Function not executed as it would branch to an invalid address)\n");

  allocator.release(span.rx());
  return 0;
}

#else
int main() {
  printf("!! This test is disabled: ASMJIT_NO_JIT or unsuitable target architecture !!\n\n");
  return 0;
}
#endif // ASMJIT_ARCH_ARM >= ASMJIT_ARCH_ARM64 && !ASMJIT_NO_AARCH64 && !ASMJIT_NO_JIT
