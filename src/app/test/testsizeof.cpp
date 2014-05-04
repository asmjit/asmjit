// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Dependencies - AsmJit]
#include <asmjit/base.h>

#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
#include <asmjit/x86.h>
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64

// [Dependencies - C]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  // --------------------------------------------------------------------------
  // [Runtime]
  // --------------------------------------------------------------------------

  printf("Sizeof[Runtime]:\n");
  printf("  int8_t                         : %u\n", static_cast<uint32_t>(sizeof(int8_t)));
  printf("  int16_t                        : %u\n", static_cast<uint32_t>(sizeof(int16_t)));
  printf("  int32_t                        : %u\n", static_cast<uint32_t>(sizeof(int32_t)));
  printf("  int64_t                        : %u\n", static_cast<uint32_t>(sizeof(int64_t)));
  printf("  long                           : %u\n", static_cast<uint32_t>(sizeof(long)));
  printf("  size_t                         : %u\n", static_cast<uint32_t>(sizeof(size_t)));
  printf("  intptr_t                       : %u\n", static_cast<uint32_t>(sizeof(intptr_t)));
  printf("  float                          : %u\n", static_cast<uint32_t>(sizeof(float)));
  printf("  double                         : %u\n", static_cast<uint32_t>(sizeof(double)));
  printf("  void*                          : %u\n", static_cast<uint32_t>(sizeof(void*)));
  printf("\n");

  // --------------------------------------------------------------------------
  // [Core]
  // --------------------------------------------------------------------------

  printf("Sizeof[Base]:\n");
  printf("  asmjit::CodeGen                : %u\n", static_cast<uint32_t>(sizeof(CodeGen)));
  printf("  asmjit::BaseAssembler          : %u\n", static_cast<uint32_t>(sizeof(BaseAssembler)));
  printf("  asmjit::BaseCompiler           : %u\n", static_cast<uint32_t>(sizeof(BaseCompiler)));
  printf("  asmjit::Runtime                : %u\n", static_cast<uint32_t>(sizeof(Runtime)));
  printf("\n");
  printf("  asmjit::Operand                : %u\n", static_cast<uint32_t>(sizeof(Operand)));
  printf("  asmjit::BaseReg                : %u\n", static_cast<uint32_t>(sizeof(BaseReg)));
  printf("  asmjit::BaseVar                : %u\n", static_cast<uint32_t>(sizeof(BaseVar)));
  printf("  asmjit::BaseMem                : %u\n", static_cast<uint32_t>(sizeof(BaseMem)));
  printf("  asmjit::Imm                    : %u\n", static_cast<uint32_t>(sizeof(Imm)));
  printf("  asmjit::Label                  : %u\n", static_cast<uint32_t>(sizeof(Label)));
  printf("\n");
  printf("  asmjit::Ptr                    : %u\n", static_cast<uint32_t>(sizeof(Ptr)));
  printf("  asmjit::SignedPtr              : %u\n", static_cast<uint32_t>(sizeof(SignedPtr)));
  printf("\n");
  printf("  asmjit::LabelData              : %u\n", static_cast<uint32_t>(sizeof(LabelData)));
  printf("  asmjit::RelocData              : %u\n", static_cast<uint32_t>(sizeof(RelocData)));
  printf("\n");
  printf("  asmjit::BaseNode               : %u\n", static_cast<uint32_t>(sizeof(BaseNode)));
  printf("  asmjit::AlignNode              : %u\n", static_cast<uint32_t>(sizeof(AlignNode)));
  printf("  asmjit::CallNode               : %u\n", static_cast<uint32_t>(sizeof(CallNode)));
  printf("  asmjit::CommentNode            : %u\n", static_cast<uint32_t>(sizeof(CommentNode)));
  printf("  asmjit::EmbedNode              : %u\n", static_cast<uint32_t>(sizeof(EmbedNode)));
  printf("  asmjit::FuncNode               : %u\n", static_cast<uint32_t>(sizeof(FuncNode)));
  printf("  asmjit::EndNode                : %u\n", static_cast<uint32_t>(sizeof(EndNode)));
  printf("  asmjit::InstNode               : %u\n", static_cast<uint32_t>(sizeof(InstNode)));
  printf("  asmjit::JumpNode               : %u\n", static_cast<uint32_t>(sizeof(JumpNode)));
  printf("  asmjit::TargetNode             : %u\n", static_cast<uint32_t>(sizeof(TargetNode)));
  printf("\n");
  printf("  asmjit::FuncDecl               : %u\n", static_cast<uint32_t>(sizeof(FuncDecl)));
  printf("  asmjit::FuncInOut              : %u\n", static_cast<uint32_t>(sizeof(FuncInOut)));
  printf("  asmjit::FuncPrototype          : %u\n", static_cast<uint32_t>(sizeof(FuncPrototype)));
  printf("\n");
  printf("  asmjit::VarAttr                : %u\n", static_cast<uint32_t>(sizeof(VarAttr)));
  printf("  asmjit::VarData                : %u\n", static_cast<uint32_t>(sizeof(VarData)));
  printf("  asmjit::BaseVarInst            : %u\n", static_cast<uint32_t>(sizeof(BaseVarInst)));
  printf("  asmjit::BaseVarState           : %u\n", static_cast<uint32_t>(sizeof(BaseVarState)));
  printf("\n");

  // --------------------------------------------------------------------------
  // [X86/X64]
  // --------------------------------------------------------------------------

#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
  printf("Sizeof[X86/X64]:\n");
  printf("  asmjit::x86x64::X86X64Assembler: %u\n", static_cast<uint32_t>(sizeof(x86x64::X86X64Assembler)));
  printf("  asmjit::x86x64::X86X64Compiler : %u\n", static_cast<uint32_t>(sizeof(x86x64::X86X64Compiler)));
  printf("\n");
  printf("  asmjit::x86x64::X86X64CallNode : %u\n", static_cast<uint32_t>(sizeof(x86x64::X86X64CallNode)));
  printf("  asmjit::x86x64::X86X64FuncNode : %u\n", static_cast<uint32_t>(sizeof(x86x64::X86X64FuncNode)));
  printf("\n");
  printf("  asmjit::x86x64::X86X64FuncDecl : %u\n", static_cast<uint32_t>(sizeof(x86x64::X86X64FuncDecl)));
  printf("\n");
  printf("  asmjit::x86x64::VarInst        : %u\n", static_cast<uint32_t>(sizeof(x86x64::VarInst)));
  printf("  asmjit::x86x64::VarState       : %u\n", static_cast<uint32_t>(sizeof(x86x64::VarState)));
  printf("\n");
  printf("  asmjit::x86x64::InstInfo       : %u\n", static_cast<uint32_t>(sizeof(x86x64::InstInfo)));
  printf("  asmjit::x86x64::VarInfo        : %u\n", static_cast<uint32_t>(sizeof(x86x64::VarInfo)));
  printf("\n");
#endif // ASMJIT_BUILD_X86

  return 0;
}
