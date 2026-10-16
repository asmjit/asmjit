// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api_build_test_p.h>
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

TEST_CASE(core_virt_mem_address_range) {
  using VirtMem::MemoryFlags;

  constexpr size_t kRangeSize = 64u * 1024u * 1024u;
  constexpr size_t kSize = 1024u * 1024u;

  // Find free address space by mapping and unmapping it (the test runner is single-threaded).
  void* hole;
  EXPECT_EQ(VirtMem::alloc(&hole, kRangeSize, MemoryFlags::kAccessRW), Error::kOk);
  EXPECT_EQ(VirtMem::release(hole, kRangeSize), Error::kOk);

  uintptr_t start = uintptr_t(hole);
  uintptr_t end = start + kRangeSize;

  void* p;
  Error err = VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, start, end);
  if (err == Error::kFeatureNotEnabled) {
    TEST_LOG("VirtMem address range is not supported on this platform");
    return;
  }

  TEST_LOG("Verifying VirtMem::alloc() within a range");
  EXPECT_EQ(err, Error::kOk);
  EXPECT_GE(uintptr_t(p), start);
  EXPECT_LE(uintptr_t(p) + kSize, end);
  memset(p, 0xCC, kSize);
  EXPECT_EQ(VirtMem::release(p, kSize), Error::kOk);

  if (VirtMem::hardened_runtime_info().has_flag(VirtMem::HardenedRuntimeFlags::kDualMapping)) {
    TEST_LOG("Verifying VirtMem::alloc_dual_mapping() within a range");
    VirtMem::DualMapping dm;
    EXPECT_EQ(VirtMem::alloc_dual_mapping(Out(dm), kSize, MemoryFlags::kAccessRWX, start, end), Error::kOk);
    EXPECT_GE(uintptr_t(dm.rx), start);
    EXPECT_LE(uintptr_t(dm.rx) + kSize, end);

    static_cast<uint8_t*>(dm.rw)[kSize - 1] = 0xCC;
    EXPECT_EQ(static_cast<const uint8_t*>(dm.rx)[kSize - 1], 0xCC);
    EXPECT_EQ(VirtMem::release_dual_mapping(dm, kSize), Error::kOk);
  }

  TEST_LOG("Verifying VirtMem::alloc() within a range that is taken");
  void* taken;
  EXPECT_EQ(VirtMem::alloc(&taken, kSize, MemoryFlags::kAccessRW), Error::kOk);
  uintptr_t taken_start = uintptr_t(taken);
  EXPECT_EQ(VirtMem::alloc(&p, kSize / 4u, MemoryFlags::kAccessRW, taken_start, taken_start + kSize), Error::kOutOfMemory);
  EXPECT_NULL(p);
  EXPECT_EQ(VirtMem::release(taken, kSize), Error::kOk);

  TEST_LOG("Verifying VirtMem::alloc() with invalid ranges");
  EXPECT_EQ(VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, end, start), Error::kInvalidArgument);
  EXPECT_EQ(VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, start, start + kSize / 2u), Error::kOutOfMemory);
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST && !ASMJIT_NO_JIT
