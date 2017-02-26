// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86BUILDER_H
#define _ASMJIT_X86_X86BUILDER_H

#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/codebuilder.h"
#include "../core/simdtypes.h"
#include "../x86/x86emitter.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::CodeBuilder]
// ============================================================================

//! Architecture-dependent \ref CodeBuilder targeting X86 and X64.
class ASMJIT_VIRTAPI X86Builder
  : public CodeBuilder,
    public X86EmitterImplicitT<X86Builder> {
public:
  ASMJIT_NONCOPYABLE(X86Builder)
  typedef CodeBuilder Base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `X86Builder` instance.
  ASMJIT_API explicit X86Builder(CodeHolder* code = nullptr) noexcept;
  //! Destroy the `X86Builder` instance.
  ASMJIT_API virtual ~X86Builder() noexcept;

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

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
#endif // _ASMJIT_X86_X86BUILDER_H
