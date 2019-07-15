// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_X86_X86INSTAPI_P_H
#define _ASMJIT_X86_X86INSTAPI_P_H

#include "../core/inst.h"
#include "../core/operand.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \cond INTERNAL
//! \addtogroup asmjit_x86
//! \{

namespace InstInternal {

#ifndef ASMJIT_NO_TEXT
Error instIdToString(uint32_t archId, uint32_t instId, String& output) noexcept;
uint32_t stringToInstId(uint32_t archId, const char* s, size_t len) noexcept;
#endif // !ASMJIT_NO_TEXT

#ifndef ASMJIT_NO_VALIDATION
Error validate(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept;
#endif // !ASMJIT_NO_VALIDATION

#ifndef ASMJIT_NO_INTROSPECTION
Error queryRWInfo(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, InstRWInfo& out) noexcept;
Error queryFeatures(uint32_t archId, const BaseInst& inst, const Operand_* operands, uint32_t opCount, BaseFeatures& out) noexcept;
#endif // !ASMJIT_NO_INTROSPECTION

} // {InstInternal}

//! \}
//! \endcond

ASMJIT_END_SUB_NAMESPACE

#endif // _ASMJIT_X86_X86INSTAPI_P_H
