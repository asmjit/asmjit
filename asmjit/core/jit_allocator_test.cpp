// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core/build_export_p.h>
#if defined(ASMJIT_TEST) && !defined(ASMJIT_NO_JIT)

#include <asmjit/axl/arena.h>
#include <asmjit/axl/arena_list.h>
#include <asmjit/axl/arena_pool.h>
#include <asmjit/axl/arena_tree.h>
#include <asmjit/axl/bit_set_utils.h>
#include <asmjit/axl/commons.h>
#include <asmjit/axl/support_p.h>
#include <asmjit/core/arch_traits.h>
#include <asmjit/core/globals.h>
#include <asmjit/core/jit_allocator.h>
#include <asmjit/core/os_utils_p.h>
#include <asmjit/core/virt_mem.h>

#include <asmjit-testing/commons/random.h>

ASMJIT_BEGIN_NAMESPACE

namespace JitAllocatorUtils {
  static void fill_pattern_64(void* p_, uint64_t pattern, size_t size_in_bytes) noexcept {
    uint64_t* p = static_cast<uint64_t*>(p_);
    size_t n = size_in_bytes / 8u;

    for (size_t i = 0; i < n; i++) {
      p[i] = pattern;
    }
  }

  static bool verify_pattern_64(const void* p_, uint64_t pattern, size_t size_in_bytes) noexcept {
    const uint64_t* p = static_cast<const uint64_t*>(p_);
    size_t n = size_in_bytes / 8u;

    for (size_t i = 0; i < n; i++) {
      if (p[i] != pattern) {
        TEST_LOG("Pattern verification failed at 0x%p [%zu * 8]: value(0x%016llX) != expected(0x%016llX)",
          p,
          i,
          (unsigned long long)p[i],
          (unsigned long long)pattern);
        return false;
      }
    }

    return true;
  }
}

// Helper class to verify that JitAllocator doesn't return addresses that overlap.
class JitAllocatorWrapper {
public:
  // Address to a memory region of a given size.
  class Range {
  public:
    inline Range(uint8_t* addr, size_t size) noexcept
      : addr(addr),
        size(size) {}
    uint8_t* addr;
    size_t size;
  };

  // Based on JitAllocator::Block, serves our purpose well...
  class Record : public axl::ArenaTreeNodeT<Record>, public Range {
  public:
    //! Read/write address, in case this is a dual mapping.
    void* _rw;
    //! Describes a pattern used to fill the allocated memory.
    uint64_t pattern;

    inline Record(void* rx, void* rw, size_t size, uint64_t pattern)
      : axl::ArenaTreeNodeT<Record>(),
        Range(static_cast<uint8_t*>(rx), size),
        _rw(rw),
        pattern(pattern) {}

    inline void* rx() const noexcept { return addr; }
    inline void* rw() const noexcept { return _rw; }
  };

  struct RecordLessThan {
    inline bool operator()(const Record& a, const Record& b) const noexcept { return a.addr < b.addr; }
    inline bool operator()(const Record& a, const uint8_t* b) const noexcept { return a.addr + a.size <= b; }
    inline bool operator()(const uint8_t* a, const Record& b) const noexcept { return a < b.addr; }
  };

  axl::Arena _arena;
  axl::ArenaPool<Record> _record_pool;
  axl::ArenaTree<Record> _records;
  JitAllocator _allocator;
  TestUtils::Random _rng;

  explicit JitAllocatorWrapper(const JitAllocator::CreateParams* params) noexcept
    : _arena(1024u * 1024u),
      _allocator(params),
      _rng(0x123456789u) {}

  void _insert(void* rx_ptr, void* rw_ptr, size_t size) noexcept {
    uint8_t* ptr = static_cast<uint8_t*>(rx_ptr);
    uint8_t* end_ptr = ptr + size - 1u;

    Record* record;

    record = _records.get(ptr, RecordLessThan{});
    EXPECT_NULL(record)
      .message("Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, ptr, ptr + size);

    record = _records.get(end_ptr, RecordLessThan{});
    EXPECT_NULL(record)
      .message("Address [%p:%p] collides with a newly allocated [%p:%p]\n", record->addr, record->addr + record->size, ptr, ptr + size);

    uint64_t pattern = _rng.next_uint64();
    record = new(axl::PlacementNew{_record_pool.alloc(_arena)}) Record(rx_ptr, rw_ptr, size, pattern);
    EXPECT_NOT_NULL(record);

    {
      VirtMem::ProtectJitReadWriteScope scope(rw_ptr, size);
      JitAllocatorUtils::fill_pattern_64(rw_ptr, pattern, size);
    }

    VirtMem::flush_instruction_cache(rx_ptr, size);
    EXPECT_TRUE(JitAllocatorUtils::verify_pattern_64(rx_ptr, pattern, size));

    _records.insert(record, RecordLessThan{});
  }

  void _remove(void* p) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p), RecordLessThan{});
    EXPECT_NOT_NULL(record);

    EXPECT_TRUE(JitAllocatorUtils::verify_pattern_64(record->rx(), record->pattern, record->size));
    EXPECT_TRUE(JitAllocatorUtils::verify_pattern_64(record->rw(), record->pattern, record->size));

    _records.remove(record, RecordLessThan{});
    _record_pool.release(record);
  }

  void* alloc(size_t size) noexcept {
    JitAllocator::Span span;
    Error err = _allocator.alloc(Out(span), size);
    EXPECT_EQ(err, Error::kOk)
      .message("JitAllocator failed to allocate %zu bytes\n", size);

    _insert(span.rx(), span.rw(), size);
    return span.rx();
  }

  void release(void* p) noexcept {
    _remove(p);
    EXPECT_EQ(_allocator.release(p), Error::kOk)
      .message("JitAllocator failed to release '%p'\n", p);
  }

  void shrink(void* p, size_t new_size) noexcept {
    Record* record = _records.get(static_cast<uint8_t*>(p), RecordLessThan{});
    EXPECT_NOT_NULL(record);

    if (!new_size) {
      return release(p);
    }

    JitAllocator::Span span;
    EXPECT_EQ(_allocator.query(Out(span), p), Error::kOk);
    Error err = _allocator.shrink(span, new_size);
    EXPECT_EQ(err, Error::kOk)
      .message("JitAllocator failed to shrink %p to %zu bytes\n", p, new_size);

    record->size = new_size;
  }
};

static void JitAllocatorTest_shuffle(void** ptr_array, size_t count, TestUtils::Random& prng) noexcept {
  for (size_t i = 0; i < count; ++i) {
    axl::swap(ptr_array[i], ptr_array[size_t(prng.next_uint32() % count)]);
  }
}

static void JitAllocatorTest_usage(JitAllocator& allocator) noexcept {
  JitAllocator::Statistics stats = allocator.statistics();
  TEST_LOG("    Block Count       : %9llu [Blocks]"        , (unsigned long long)(stats.block_count()));
  TEST_LOG("    Reserved (VirtMem): %9llu [Bytes]"         , (unsigned long long)(stats.reserved_size()));
  TEST_LOG("    Used     (VirtMem): %9llu [Bytes] (%.1f%%)", (unsigned long long)(stats.used_size()), stats.used_ratio() * 100.0);
  TEST_LOG("    Overhead (HeapMem): %9llu [Bytes] (%.1f%%)", (unsigned long long)(stats.overhead_size()), stats.overhead_ratio() * 100.0);
}

template<typename T, size_t kPatternSize, bool Bit>
static void BitVectorRangeIterator_test_random(TestUtils::Random& rnd, size_t count) noexcept {
  for (size_t i = 0; i < count; i++) {
    T in[kPatternSize];
    T out[kPatternSize];

    for (size_t j = 0; j < kPatternSize; j++) {
      in[j] = T(uint64_t(rnd.next_uint32() & 0xFFu) * 0x0101010101010101);
      out[j] = Bit == 0 ? axl::bit_ones<T> : T(0);
    }

    {
      axl::BitVectorRangeIterator<T, Bit> it(in, kPatternSize);
      size_t range_start, range_end;
      while (it.next_range(Out(range_start), Out(range_end))) {
        if (Bit) {
          axl::bit_vector_fill(out, range_start, range_end - range_start);
        }
        else {
          axl::bit_vector_clear(out, range_start, range_end - range_start);
        }
      }
    }

    for (size_t j = 0; j < kPatternSize; j++) {
      EXPECT_EQ(in[j], out[j])
        .message("Invalid pattern detected at [%zu] (%llX != %llX)", j, (unsigned long long)in[j], (unsigned long long)out[j]);
    }
  }
}

static void test_jit_allocator_reset_empty() noexcept {
  JitAllocator allocator;
  allocator.reset(ResetPolicy::kSoft);
}

static void test_jit_allocator_alloc_release() noexcept {
  size_t kCount = ZeroTest::test_runner()->has_arg("--quick") ? 20000 : 100000;

  struct TestInfo {
    const char* name;
    JitAllocatorOptions options;
    uint32_t block_size;
    uint32_t granularity;
  };

  using Opt = JitAllocatorOptions;

  VirtMem::HardenedRuntimeInfo hri = VirtMem::hardened_runtime_info();

  TestInfo test_info_table[] = {
    { "Default"                                    , Opt::kNone, 0, 0 },
    { "16MB blocks"                                , Opt::kNone, 16 * 1024 * 1024, 0 },
    { "256B granularity"                           , Opt::kNone, 0, 256 },
    { "kUseMultiplePools"                          , Opt::kUseMultiplePools, 0, 0 },
    { "kFillUnusedMemory"                          , Opt::kFillUnusedMemory, 0, 0 },
    { "kImmediateRelease"                          , Opt::kImmediateRelease, 0, 0 },
    { "kDisableInitialPadding"                     , Opt::kDisableInitialPadding, 0, 0 },
    { "kUseLargePages"                             , Opt::kUseLargePages, 0, 0 },
    { "kUseLargePages | kFillUnusedMemory"         , Opt::kUseLargePages | Opt::kFillUnusedMemory, 0, 0 },
    { "kUseLargePages | kAlignBlockSizeToLargePage", Opt::kUseLargePages | Opt::kAlignBlockSizeToLargePage, 0, 0 },
    { "kUseDualMapping"                            , Opt::kUseDualMapping , 0, 0 },
    { "kUseDualMapping | kFillUnusedMemory"        , Opt::kUseDualMapping | Opt::kFillUnusedMemory, 0, 0 }
  };

  TEST_LOG("BitVectorRangeIterator<uint32_t>");
  {
    TestUtils::Random rnd;
    BitVectorRangeIterator_test_random<uint32_t, 64, 0>(rnd, kCount);
  }

  TEST_LOG("BitVectorRangeIterator<uint64_t>");
  {
    TestUtils::Random rnd;
    BitVectorRangeIterator_test_random<uint64_t, 64, 0>(rnd, kCount);
  }

  for (const TestInfo& test_info : test_info_table) {
    // Don't try to allocate dual-mapping if dual mapping is not possible - it would fail the test.
    if (axl::test(test_info.options, JitAllocatorOptions::kUseDualMapping) &&
        !axl::test(hri.flags, VirtMem::HardenedRuntimeFlags::kDualMapping)) {
      continue;
    }

    TEST_LOG("JitAllocator(%s)", test_info.name);

    JitAllocator::CreateParams params {};
    params.options = test_info.options;
    params.block_size = test_info.block_size;
    params.granularity = test_info.granularity;

    size_t fixed_block_size = 256;

    JitAllocatorWrapper wrapper(&params);
    TestUtils::Random prng(100);

    size_t i;

    TEST_LOG("  Memory alloc/release test - %d allocations", kCount);

    void** ptr_array = (void**)::malloc(sizeof(void*) * size_t(kCount));
    EXPECT_NOT_NULL(ptr_array);

    // Random blocks tests...
    TEST_LOG("  Allocating random blocks...");
    for (i = 0; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc((prng.next_uint32() % 1024) + 8);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Releasing all allocated blocks from the beginning...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Allocating random blocks again...", kCount);
    for (i = 0; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc((prng.next_uint32() % 1024) + 8);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Shuffling allocated blocks...");
    JitAllocatorTest_shuffle(ptr_array, unsigned(kCount), prng);

    TEST_LOG("  Releasing 50%% of allocated blocks...");
    for (i = 0; i < kCount / 2; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Allocating 50%% more blocks again...");
    for (i = 0; i < kCount / 2; i++) {
      ptr_array[i] = wrapper.alloc((prng.next_uint32() % 1024) + 8);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Releasing all allocated blocks from the end...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[kCount - i - 1]);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    // Fixed blocks tests...
    TEST_LOG("  Allocating %zuB blocks...", fixed_block_size);
    for (i = 0; i < kCount / 2; i++) {
      ptr_array[i] = wrapper.alloc(fixed_block_size);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Shrinking each %zuB block to 1 byte", fixed_block_size);
    for (i = 0; i < kCount / 2; i++) {
      wrapper.shrink(ptr_array[i], 1);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Allocating more 64B blocks...", 64);
    for (i = kCount / 2; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc(64);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Releasing all blocks from the beginning...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Allocating %zuB blocks...", fixed_block_size);
    for (i = 0; i < kCount; i++) {
      ptr_array[i] = wrapper.alloc(fixed_block_size);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Shuffling allocated blocks...");
    JitAllocatorTest_shuffle(ptr_array, unsigned(kCount), prng);

    TEST_LOG("  Releasing 50%% of allocated blocks...");
    for (i = 0; i < kCount / 2; i++) {
      wrapper.release(ptr_array[i]);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Allocating 50%% more %zuB blocks again...", fixed_block_size);
    for (i = 0; i < kCount / 2; i++) {
      ptr_array[i] = wrapper.alloc(fixed_block_size);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    TEST_LOG("  Releasing all allocated blocks from the end...");
    for (i = 0; i < kCount; i++) {
      wrapper.release(ptr_array[kCount - i - 1]);
    }
    JitAllocatorTest_usage(wrapper._allocator);

    ::free(ptr_array);
  }
}

static void test_jit_allocator_query() noexcept {
  JitAllocator allocator;
  size_t allocated_size = 100;

  JitAllocator::Span allocated_span;
  EXPECT_EQ(allocator.alloc(Out(allocated_span), allocated_size), Error::kOk);
  EXPECT_NOT_NULL(allocated_span.rx());
  EXPECT_GE(allocated_span.size(), allocated_size);

  JitAllocator::Span queried_span;
  EXPECT_EQ(allocator.query(Out(queried_span), allocated_span.rx()), Error::kOk);
  EXPECT_EQ(allocated_span.rx(), queried_span.rx());
  EXPECT_EQ(allocated_span.rw(), queried_span.rw());
  EXPECT_EQ(allocated_span.size(), queried_span.size());
}

TEST_CASE(core_jit_allocator) {
  test_jit_allocator_reset_empty();
  test_jit_allocator_alloc_release();
  test_jit_allocator_query();
}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_TEST && !ASMJIT_NO_JIT
