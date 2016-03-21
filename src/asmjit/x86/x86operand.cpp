// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)

// [Dependencies - AsmJit]
#include "../x86/x86operand.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {
namespace x86 {

// ============================================================================
// [asmjit::X86Mem - abs[]]
// ============================================================================

X86Mem ptr_abs(Ptr p, int32_t disp, uint32_t size) noexcept {
  X86Mem m(NoInit);

  m._init_packed_op_sz_b0_b1_id(Operand::kTypeMem, size, kMemTypeAbsolute, 0, kInvalidValue);
  m._vmem.index = kInvalidValue;
  m._vmem.displacement = static_cast<int32_t>((intptr_t)(p + disp));

  return m;
}

X86Mem ptr_abs(Ptr p, const X86Reg& index, uint32_t shift, int32_t disp, uint32_t size) noexcept {
  X86Mem m(NoInit);
  uint32_t flags = shift << kX86MemShiftIndex;

  if (index.isGp())
    flags |= X86Mem::_getGpdFlags(index);
  else if (index.isXmm())
    flags |= kX86MemVSibXmm << kX86MemVSibIndex;
  else if (index.isYmm())
    flags |= kX86MemVSibYmm << kX86MemVSibIndex;

  m._init_packed_op_sz_b0_b1_id(Operand::kTypeMem, size, kMemTypeAbsolute, flags, kInvalidValue);
  m._vmem.index = index.getRegIndex();
  m._vmem.displacement = static_cast<int32_t>((intptr_t)(p + disp));

  return m;
}

#if !defined(ASMJIT_DISABLE_COMPILER)
X86Mem ptr_abs(Ptr p, const X86Var& index, uint32_t shift, int32_t disp, uint32_t size) noexcept {
  X86Mem m(NoInit);
  uint32_t flags = shift << kX86MemShiftIndex;

  const Var& index_ = reinterpret_cast<const Var&>(index);
  uint32_t indexRegType = index_.getRegType();

  if (indexRegType <= kX86RegTypeGpq)
    flags |= X86Mem::_getGpdFlags(reinterpret_cast<const Var&>(index));
  else if (indexRegType == kX86RegTypeXmm)
    flags |= kX86MemVSibXmm << kX86MemVSibIndex;
  else if (indexRegType == kX86RegTypeYmm)
    flags |= kX86MemVSibYmm << kX86MemVSibIndex;

  m._init_packed_op_sz_b0_b1_id(Operand::kTypeMem, size, kMemTypeAbsolute, flags, kInvalidValue);
  m._vmem.index = index_.getId();
  m._vmem.displacement = static_cast<int32_t>((intptr_t)(p + disp));

  return m;
}
#endif // !ASMJIT_DISABLE_COMPILER

} // x86 namespace
} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64
