// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/api_build_test_p.h>
#if defined(ASMJIT_TEST) && !defined(ASMJIT_NO_JIT)

#include <asmjit/axl/support_p.h>
#include <asmjit/core/virt_mem.h>

#if !defined(_WIN32)
  #include <sys/mman.h>
#endif

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
  EXPECT_OK(VirtMem::alloc(&hole, kRangeSize, MemoryFlags::kAccessRW));
  EXPECT_OK(VirtMem::release(hole, kRangeSize));

  VirtMem::AddressRange range { uintptr_t(hole), uintptr_t(hole) + kRangeSize };

  TEST_LOG("Verifying VirtMem::alloc() within a range");
  void* p;
  EXPECT_OK(VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, range));
  EXPECT_GE(uintptr_t(p), range.start);
  EXPECT_LE(uintptr_t(p) + kSize, range.end);
  memset(p, 0xCC, kSize);
  EXPECT_OK(VirtMem::release(p, kSize));

  if (VirtMem::hardened_runtime_info().has_flag(VirtMem::HardenedRuntimeFlags::kDualMapping)) {
    TEST_LOG("Verifying VirtMem::alloc_dual_mapping() within a range");
    VirtMem::DualMapping dm;
    EXPECT_OK(VirtMem::alloc_dual_mapping(Out(dm), kSize, MemoryFlags::kAccessRWX, range));
    EXPECT_GE(uintptr_t(dm.rx), range.start);
    EXPECT_LE(uintptr_t(dm.rx) + kSize, range.end);

    static_cast<uint8_t*>(dm.rw)[kSize - 1] = 0xCC;
    EXPECT_EQ(static_cast<const uint8_t*>(dm.rx)[kSize - 1], 0xCC);
    EXPECT_OK(VirtMem::release_dual_mapping(dm, kSize));
  }

  TEST_LOG("Verifying VirtMem::alloc() within a range that is taken");
  void* taken;
  EXPECT_OK(VirtMem::alloc(&taken, kSize, MemoryFlags::kAccessRW));
  VirtMem::AddressRange taken_range { uintptr_t(taken), uintptr_t(taken) + kSize };
  EXPECT_EQ(VirtMem::alloc(&p, kSize / 4u, MemoryFlags::kAccessRW, taken_range), Error::kOutOfMemory);
  EXPECT_NULL(p);
  EXPECT_OK(VirtMem::release(taken, kSize));

  TEST_LOG("Verifying VirtMem::alloc() with invalid ranges");
  VirtMem::AddressRange reversed { range.end, range.start };
  VirtMem::AddressRange empty { range.start, range.start };
  VirtMem::AddressRange too_small { range.start, range.start + kSize / 2u };
  EXPECT_EQ(VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, reversed), Error::kInvalidArgument);
  EXPECT_EQ(VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, empty), Error::kInvalidArgument);
  EXPECT_EQ(VirtMem::alloc(&p, kSize, MemoryFlags::kAccessRW, too_small), Error::kOutOfMemory);

  VirtMem::DualMapping dm { &p, &p };
  EXPECT_EQ(VirtMem::alloc_dual_mapping(Out(dm), kSize, MemoryFlags::kAccessRWX, reversed), Error::kInvalidArgument);
  EXPECT_NULL(dm.rx);
  EXPECT_NULL(dm.rw);
}

#if !defined(_WIN32)
TEST_CASE(core_virt_mem_aligned) {
  const size_t page_size = VirtMem::info().page_size;
#if defined(MADV_HUGEPAGE)
  void* probe;
  EXPECT_OK(VirtMem::alloc(&probe, page_size, VirtMem::MemoryFlags::kAccessRW));
  // Kernels without THP can reject advice even when the build headers support it.
  bool advice_supported = madvise(probe, page_size, MADV_HUGEPAGE) == 0;
  EXPECT_OK(VirtMem::release(probe, page_size));
#else
  bool advice_supported = false;
#endif

  for (size_t alignment : {page_size, page_size * 4, size_t(2u * 1024u * 1024u)}) {
    for (VirtMem::MemoryFlags advice : {VirtMem::MemoryFlags::kNone, VirtMem::MemoryFlags::kMAdviseLargePages}) {
      for (size_t size : {alignment, alignment * 2}) {
        void* p = nullptr;
        Error err = VirtMem::alloc_aligned(&p, size, alignment, VirtMem::MemoryFlags::kAccessRW | advice);
        if (advice != VirtMem::MemoryFlags::kNone && !advice_supported) {
          EXPECT_NE(err, Error::kOk);
          EXPECT_NULL(p);
          continue;
        }
        EXPECT_OK(err);
        EXPECT_TRUE(axl::is_aligned(uintptr_t(p), alignment));
        memset(p, 0xA5, size);
        EXPECT_EQ(static_cast<uint8_t*>(p)[size - 1], uint8_t(0xA5));
        EXPECT_OK(VirtMem::release(p, size));
      }
    }
  }

  VirtMem::HardenedRuntimeInfo runtime_info = VirtMem::hardened_runtime_info();
  if (!runtime_info.has_flag(VirtMem::HardenedRuntimeFlags::kEnabled) ||
      runtime_info.has_flag(VirtMem::HardenedRuntimeFlags::kMapJit)) {
    const size_t alignment = 2u * 1024u * 1024u;
    void* executable;
    EXPECT_OK(VirtMem::alloc_aligned(&executable, alignment, alignment, VirtMem::MemoryFlags::kAccessRWX));
    {
      VirtMem::ProtectJitReadWriteScope scope(executable, alignment);
      memset(executable, 0xA5, alignment);
    }
    EXPECT_OK(VirtMem::release(executable, alignment));
  }

  const struct {
    size_t size;
    size_t alignment;
  } invalid_allocations[] = {
    {0, page_size},
    {page_size - 1, page_size},
    {page_size, page_size * 3},
    {page_size, 1},
    {page_size, page_size * 2}
  };
  for (const auto& allocation : invalid_allocations) {
    void* p = nullptr;
    EXPECT_EQ(VirtMem::alloc_aligned(&p, allocation.size, allocation.alignment, VirtMem::MemoryFlags::kAccessRW),
              Error::kInvalidArgument);
    EXPECT_NULL(p);
  }
  for (VirtMem::MemoryFlags flags : {VirtMem::MemoryFlags::kMMapLargePages, VirtMem::MemoryFlags::kMapShared}) {
    void* p = nullptr;
    EXPECT_EQ(VirtMem::alloc_aligned(&p, page_size, page_size, VirtMem::MemoryFlags::kAccessRW | flags),
              Error::kInvalidArgument);
    EXPECT_NULL(p);
  }
}
#endif

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST && !ASMJIT_NO_JIT
