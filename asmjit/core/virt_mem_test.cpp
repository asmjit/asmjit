// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>
#if defined(ASMJIT_TEST) && !defined(ASMJIT_NO_JIT)

#include <asmjit/core/virt_mem.h>

ASMJIT_BEGIN_NAMESPACE

TEST_CASE(core_virt_mem) {
  VirtMem::Info vm_info = VirtMem::info();

  TEST_LOG("VirtMem::info():");
  TEST_LOG("  page_size: %zu", size_t(vm_info.page_size));
  TEST_LOG("  page_granularity: %zu", size_t(vm_info.page_granularity));

  TEST_LOG("VirtMem::large_page_size():");
  TEST_LOG("  large_page_size: %zu", size_t(VirtMem::large_page_size()));

  VirtMem::HardenedRuntimeInfo hardened_rt_info = VirtMem::hardened_runtime_info();
  VirtMem::HardenedRuntimeFlags hardened_rt_flags = hardened_rt_info.flags;

  TEST_LOG("VirtMem::hardened_runtime_info():");
  TEST_LOG("  flags:");
  TEST_LOG("    kEnabled: %s"    , axl::test(hardened_rt_flags, VirtMem::HardenedRuntimeFlags::kEnabled    ) ? "true" : "false");
  TEST_LOG("    kMapJit: %s"     , axl::test(hardened_rt_flags, VirtMem::HardenedRuntimeFlags::kMapJit     ) ? "true" : "false");
  TEST_LOG("    kDualMapping: %s", axl::test(hardened_rt_flags, VirtMem::HardenedRuntimeFlags::kDualMapping) ? "true" : "false");
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST && !ASMJIT_NO_JIT
