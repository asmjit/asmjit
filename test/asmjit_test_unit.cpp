// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
#include <asmjit/x86.h>
#endif

#if !defined(ASMJIT_NO_AARCH64)
#include <asmjit/a64.h>
#endif

#include "asmjitutils.h"
#include "broken.h"

using namespace asmjit;

#define DUMP_TYPE(...) \
  printf("  %-26s: %u\n", #__VA_ARGS__, uint32_t(sizeof(__VA_ARGS__)))

static void printTypeSizes(void) noexcept {
  printf("Size of C++ types:\n");
    DUMP_TYPE(int8_t);
    DUMP_TYPE(int16_t);
    DUMP_TYPE(int32_t);
    DUMP_TYPE(int64_t);
    DUMP_TYPE(int);
    DUMP_TYPE(long);
    DUMP_TYPE(size_t);
    DUMP_TYPE(intptr_t);
    DUMP_TYPE(float);
    DUMP_TYPE(double);
    DUMP_TYPE(void*);
  printf("\n");

  printf("Size of base classes:\n");
    DUMP_TYPE(BaseAssembler);
    DUMP_TYPE(BaseEmitter);
    DUMP_TYPE(CodeBuffer);
    DUMP_TYPE(CodeHolder);
    DUMP_TYPE(ConstPool);
    DUMP_TYPE(Fixup);
    DUMP_TYPE(LabelEntry);
    DUMP_TYPE(LabelEntry::ExtraData);
    DUMP_TYPE(RelocEntry);
    DUMP_TYPE(Section);
    DUMP_TYPE(String);
    DUMP_TYPE(Target);
  printf("\n");

  printf("Size of zone classes:\n");
    DUMP_TYPE(Zone);
    DUMP_TYPE(ZoneAllocator);
    DUMP_TYPE(ZoneBitVector);
    DUMP_TYPE(ZoneHashNode);
    DUMP_TYPE(ZoneHash<ZoneHashNode>);
    DUMP_TYPE(ZoneList<int>);
    DUMP_TYPE(ZoneVector<int>);
  printf("\n");

  printf("Size of operand classes:\n");
    DUMP_TYPE(Operand);
    DUMP_TYPE(Reg);
    DUMP_TYPE(BaseMem);
    DUMP_TYPE(Imm);
    DUMP_TYPE(Label);
  printf("\n");

  printf("Size of function classes:\n");
    DUMP_TYPE(CallConv);
    DUMP_TYPE(FuncFrame);
    DUMP_TYPE(FuncValue);
    DUMP_TYPE(FuncDetail);
    DUMP_TYPE(FuncSignature);
    DUMP_TYPE(FuncArgsAssignment);
  printf("\n");

#if !defined(ASMJIT_NO_BUILDER)
  printf("Size of builder classes:\n");
    DUMP_TYPE(BaseBuilder);
    DUMP_TYPE(BaseNode);
    DUMP_TYPE(InstNode);
    DUMP_TYPE(InstNodeWithOperands<InstNode::kBaseOpCapacity>);
    DUMP_TYPE(InstNodeWithOperands<InstNode::kFullOpCapacity>);
    DUMP_TYPE(AlignNode);
    DUMP_TYPE(LabelNode);
    DUMP_TYPE(EmbedDataNode);
    DUMP_TYPE(EmbedLabelNode);
    DUMP_TYPE(ConstPoolNode);
    DUMP_TYPE(CommentNode);
    DUMP_TYPE(SentinelNode);
  printf("\n");
#endif

#if !defined(ASMJIT_NO_COMPILER)
  printf("Size of compiler classes:\n");
    DUMP_TYPE(BaseCompiler);
    DUMP_TYPE(FuncNode);
    DUMP_TYPE(FuncRetNode);
    DUMP_TYPE(InvokeNode);
  printf("\n");
#endif

#if !defined(ASMJIT_NO_X86)
  printf("Size of x86-specific classes:\n");
    DUMP_TYPE(x86::Assembler);
    #if !defined(ASMJIT_NO_BUILDER)
    DUMP_TYPE(x86::Builder);
    #endif
    #if !defined(ASMJIT_NO_COMPILER)
    DUMP_TYPE(x86::Compiler);
    #endif
    DUMP_TYPE(x86::InstDB::InstInfo);
    DUMP_TYPE(x86::InstDB::CommonInfo);
    DUMP_TYPE(x86::InstDB::OpSignature);
    DUMP_TYPE(x86::InstDB::InstSignature);
  printf("\n");
#endif

#if !defined(ASMJIT_NO_AARCH64)
  printf("Size of aarch64-specific classes:\n");
    DUMP_TYPE(a64::Assembler);
    #if !defined(ASMJIT_NO_BUILDER)
    DUMP_TYPE(a64::Builder);
    #endif
    #if !defined(ASMJIT_NO_COMPILER)
    DUMP_TYPE(a64::Compiler);
    #endif
  printf("\n");
#endif
}

#undef DUMP_TYPE

static void onBeforeRun(void) noexcept {
  printBuildOptions();
  printCpuInfo();
  printTypeSizes();
}

int main(int argc, const char* argv[]) {
  printf("AsmJit Unit-Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost),
    asmjitBuildType()
  );

  return BrokenAPI::run(argc, argv, onBeforeRun);
}
