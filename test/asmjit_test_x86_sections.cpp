// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// ----------------------------------------------------------------------------
// This is a working example that demonstrates how multiple sections can be
// used in a JIT-based code generator. It shows also the necessary tooling
// that is expected to be done by the user when the feature is used. It's
// important to handle the following cases:
//
//   - Assign offsets to sections when the code generation is finished.
//   - Tell the CodeHolder to resolve unresolved links and check whether
//     all links were resolved.
//   - Relocate the code
//   - Copy the code to the destination address.
// ----------------------------------------------------------------------------

#include <asmjit/core.h>
#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86

#include <asmjit/x86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

// The generated function is very simple, it only accesses the built-in data
// (from .data section) at the index as provided by its first argument. This
// data is inlined into the resulting function so we can use it this array
// for verification that the function returns correct values.
static const uint8_t dataArray[] = { 2, 9, 4, 7, 1, 3, 8, 5, 6, 0 };

static void fail(const char* message, Error err) {
  printf("** FAILURE: %s (%s) **\n", message, DebugUtils::errorAsString(err));
  exit(1);
}

int main() {
  printf("AsmJit X86 Sections Test\n\n");

  Environment env = Environment::host();
  JitAllocator allocator;

#ifndef ASMJIT_NO_LOGGING
  FileLogger logger(stdout);
  logger.setIndentation(FormatIndentationGroup::kCode, 2);
#endif

  CodeHolder code;
  code.init(env);

#ifndef ASMJIT_NO_LOGGING
  code.setLogger(&logger);
#endif

  Section* dataSection;
  Error err = code.newSection(&dataSection, ".data", SIZE_MAX, SectionFlags::kNone, 8);

  if (err) {
    fail("Failed to create a .data section", err);
  }
  else {
    printf("Generating code:\n");
    x86::Assembler a(&code);
    x86::Gp idx = a.zax();
    x86::Gp addr = a.zcx();

    Label data = a.newLabel();

    FuncDetail func;
    func.init(FuncSignatureT<size_t, size_t>(CallConvId::kHost), code.environment());

    FuncFrame frame;
    frame.init(func);
    frame.addDirtyRegs(idx, addr);

    FuncArgsAssignment args(&func);
    args.assignAll(idx);
    args.updateFuncFrame(frame);
    frame.finalize();

    a.emitProlog(frame);
    a.emitArgsAssignment(frame, args);

    a.lea(addr, x86::ptr(data));
    a.movzx(idx, x86::byte_ptr(addr, idx));

    a.emitEpilog(frame);

    a.section(dataSection);
    a.bind(data);

    a.embed(dataArray, sizeof(dataArray));
  }

  // Manually change he offsets of each section, start at 0. This code is very
  // similar to what `CodeHolder::flatten()` does, however, it's shown here
  // how to do it explicitly.
  printf("\nCalculating section offsets:\n");
  uint64_t offset = 0;
  for (Section* section : code.sectionsByOrder()) {
    offset = Support::alignUp(offset, section->alignment());
    section->setOffset(offset);
    offset += section->realSize();

    printf("  [0x%08X %s] {Id=%u Size=%u}\n",
           uint32_t(section->offset()),
           section->name(),
           section->id(),
           uint32_t(section->realSize()));
  }
  size_t codeSize = size_t(offset);
  printf("  Final code size: %zu\n", codeSize);

  // Resolve cross-section links (if any). On 32-bit X86 this is not necessary
  // as this is handled through relocations as the addressing is different.
  if (code.hasUnresolvedLinks()) {
    printf("\nResolving cross-section links:\n");
    printf("  Before 'resolveUnresolvedLinks()': %zu\n", code.unresolvedLinkCount());

    err = code.resolveUnresolvedLinks();
    if (err)
      fail("Failed to resolve cross-section links", err);
    printf("  After 'resolveUnresolvedLinks()': %zu\n", code.unresolvedLinkCount());
  }

  // Allocate memory for the function and relocate it there.
  void* rxPtr;
  void* rwPtr;
  err = allocator.alloc(&rxPtr, &rwPtr, codeSize);
  if (err)
    fail("Failed to allocate executable memory", err);

  // Relocate to the base-address of the allocated memory.
  code.relocateToBase(uint64_t(uintptr_t(rxPtr)));

  VirtMem::protectJitMemory(VirtMem::ProtectJitAccess::kReadWrite);

  // Copy the flattened code into `mem.rw`. There are two ways. You can either copy
  // everything manually by iterating over all sections or use `copyFlattenedData`.
  // This code is similar to what `copyFlattenedData(p, codeSize, 0)` would do:
  for (Section* section : code.sectionsByOrder())
    memcpy(static_cast<uint8_t*>(rwPtr) + size_t(section->offset()), section->data(), section->bufferSize());

  VirtMem::protectJitMemory(VirtMem::ProtectJitAccess::kReadExecute);
  VirtMem::flushInstructionCache(rwPtr, code.codeSize());

  // Execute the function and test whether it works.
  typedef size_t (*Func)(size_t idx);
  Func fn = (Func)rxPtr;

  printf("\n");
  if (fn(0) != dataArray[0] ||
      fn(3) != dataArray[3] ||
      fn(6) != dataArray[6] ||
      fn(9) != dataArray[9] ) {
    printf("** FAILURE: The generated function returned incorrect result(s) **\n");
    return 1;
  }

  printf("** SUCCESS **\n");
  return 0;
}

#else
int main() {
  printf("AsmJit X86 Sections Test is disabled on non-x86 host\n\n");
  return 0;
}
#endif // !ASMJIT_NO_X86 && ASMJIT_ARCH_X86
