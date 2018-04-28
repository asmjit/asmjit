// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86BUILDER_H
#define _ASMJIT_X86_X86BUILDER_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/builder.h"
#include "../core/datatypes.h"
#include "../x86/x86emitter.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86_api
//! \{

// ============================================================================
// [asmjit::x86::Builder]
// ============================================================================

//! Architecture-dependent asm-builder (X86).
class ASMJIT_VIRTAPI Builder
  : public BaseBuilder,
    public EmitterImplicitT<Builder> {
public:
  ASMJIT_NONCOPYABLE(Builder)
  typedef BaseBuilder Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API explicit Builder(CodeHolder* code = nullptr) noexcept;
  ASMJIT_API virtual ~Builder() noexcept;

  // --------------------------------------------------------------------------
  // [Finalize]
  // --------------------------------------------------------------------------

  ASMJIT_API Error finalize() override;

  // --------------------------------------------------------------------------
  // [Events]
  // --------------------------------------------------------------------------

  ASMJIT_API Error onAttach(CodeHolder* code) noexcept override;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
#endif // _ASMJIT_X86_X86BUILDER_H
