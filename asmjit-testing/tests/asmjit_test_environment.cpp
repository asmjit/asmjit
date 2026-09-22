// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86) && ASMJIT_TARGET_ARCH_X86 != 0
  #include <asmjit/x86.h>
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_TARGET_ARCH_ARM == 64
  #include <asmjit/a64.h>
#endif

#include <asmjit-testing/commons/asmjit_utils.h>

using namespace asmjit;

static void print_app_info() {
  printf("AsmJit Environment Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  printf("This application can be used to verify AsmJit build options and to verify the\n");
  printf("environment where it runs. For example to check CPU extensions available, system\n");
  printf("hardening (RWX restrictions), large page support, and virtual memory allocations.\n");
  printf("\n");
}

[[maybe_unused]]
static const char* stringify_bool(bool b) noexcept { return b ? "true" : "false"; };

[[maybe_unused]]
static const char* stringify_result(Error err) noexcept { return err == Error::kOk ? "success" : stringify_error(err); };

using VoidFunc = void (ASMJIT_CDECL*)(void);

#if !defined(ASMJIT_NO_JIT)

#if !defined(ASMJIT_NO_X86) && ASMJIT_TARGET_ARCH_X86 != 0
#define TEST_ENVIRONMENT_HAS_JIT

static void emit_void_function(CodeHolder& code) noexcept {
  x86::Assembler a(&code);
  a.ret();
}
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_TARGET_ARCH_ARM == 64
#define TEST_ENVIRONMENT_HAS_JIT

static void emit_void_function(CodeHolder& code) noexcept {
  a64::Assembler a(&code);
  a.ret(a64::x30);
}
#endif

#if defined(TEST_ENVIRONMENT_HAS_JIT)
static void* offset_pointer(void* ptr, size_t offset) noexcept {
  return static_cast<void*>(static_cast<uint8_t*>(ptr) + offset);
}

static size_t write_empty_function_at(void* ptr, size_t size) noexcept {
  printf("  Write JIT code at addr  : %p\n", ptr);

  CodeHolder code;
  Error err = code.init(Environment::host());
  if (err != Error::kOk) {
    printf(  "Failed to initialize CodeHolder (%s)\n", stringify_error(err));
    return 0;
  }

  emit_void_function(code);
  code.flatten();
  code.copy_flattened_data(ptr, size);

  return code.code_size();
}

static void flush_instruction_cache(void* ptr, size_t size) noexcept {
  printf("  Flush JIT code at addr  : %p [size=%zu]\n", ptr, size);
  VirtMem::flush_instruction_cache(ptr, size);
}

static void* make_authenticated(void* ptr) noexcept {
  if (PAuthUtils::is_pauth_enforced()) {
    void* signed_ptr = PAuthUtils::sign_c_func_ptr(ptr);
    printf("  Authenticate Pointer    : %p -> %p\n", ptr, signed_ptr);
    return signed_ptr;
  }
  else {
    return ptr;
  }
}

static void invoke_void_function(void* ptr) noexcept {
  printf("  Invoke JIT code at addr : %p\n", ptr);

  // In case it crashes, we want to have the output flushed.
  fflush(stdout);

  VoidFunc func = reinterpret_cast<VoidFunc>(ptr);
  func();
}
#endif

struct ByteUnit {
  size_t value;
  char unit[8];
};

static ByteUnit format_byte_quantity(size_t value) noexcept {
  static const ByteUnit table[] = {{10, "KiB"}, {20, "MiB"}, {30, "GiB"}};

  ByteUnit out{value, "B"};
  for (size_t i = 0; i < 3; i++) {
    size_t shift = table[i].value;
    size_t qty = value >> shift;

    if (value == (qty << shift)) {
      out.value = qty;
      memcpy(out.unit, table[i].unit, 8);
    }
  }

  return out;
}

static void print_virt_mem_info_and_test_execution() noexcept {
  using Limits = VirtMem::Limits;
  using MemoryFlags = VirtMem::MemoryFlags;
  using HardenedRuntimeInfo = VirtMem::HardenedRuntimeInfo;
  using HardenedRuntimeFlags = VirtMem::HardenedRuntimeFlags;

  // Size of a virtual memory allocation.
  constexpr size_t kVMemAllocSize = 65536;

  // Offset to the first function to execute (must be greater than 8 for UBSAN to work).
  [[maybe_unused]]
  constexpr size_t kVirtFuncOffset = 4096;

  size_t large_page_size = VirtMem::large_page_size();
  Limits limits = VirtMem::limits();
  HardenedRuntimeInfo rti = VirtMem::hardened_runtime_info();

  ByteUnit fmt_page_size = format_byte_quantity(VirtMem::info().page_size);
  ByteUnit fmt_large_page_size = format_byte_quantity(large_page_size);

  ByteUnit fmt_physical_memory_size = format_byte_quantity(limits.physical_memory_size);
  ByteUnit fmt_address_space_limit = format_byte_quantity(limits.address_space_limit);
  ByteUnit fmt_file_size_limit = format_byte_quantity(limits.file_size_limit);
  ByteUnit fmt_dual_mapping_size_limit = format_byte_quantity(VirtMem::dual_mapping_size_limit());

  if (large_page_size == 0u) {
    memcpy(fmt_large_page_size.unit, "(N/A)", 6);
  }

  if (fmt_physical_memory_size.value == SIZE_MAX) {
    memcpy(fmt_physical_memory_size.unit, "(MAX)", 6);
  }

  if (fmt_address_space_limit.value == SIZE_MAX) {
    memcpy(fmt_address_space_limit.unit, "(MAX)", 6);
  }

  if (fmt_file_size_limit.value == SIZE_MAX) {
    memcpy(fmt_file_size_limit.unit, "(MAX)", 6);
  }

  if (fmt_dual_mapping_size_limit.value == SIZE_MAX) {
    memcpy(fmt_dual_mapping_size_limit.unit, "(MAX)", 6);
  }

  printf("Page/LargePage Info:\n");
  printf("  Page size               : %zu %s\n", fmt_page_size.value, fmt_page_size.unit);
  printf("  Large page size         : %zu %s\n", fmt_large_page_size.value, fmt_large_page_size.unit);
  printf("\n");

  printf("Virtual Memory Limits:\n");
  printf("  Physical memory size    : %zu %s\n", fmt_physical_memory_size.value, fmt_physical_memory_size.unit);
  printf("  Address space limit     : %zu %s\n", fmt_address_space_limit.value, fmt_address_space_limit.unit);
  printf("  File size limit         : %zu %s\n", fmt_file_size_limit.value, fmt_file_size_limit.unit);
  printf("  Dual-mapping size limit : %zu %s\n", fmt_dual_mapping_size_limit.value, fmt_dual_mapping_size_limit.unit);
  printf("\n");

  printf("Hardened Environment Info:\n");
  printf("  Hardening was detected  : %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kEnabled    )));
  printf("  MAP_JIT is available    : %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kMapJit     )));
  printf("  DualMapping is available: %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kDualMapping)));
  printf("  Pointer authentication  : %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kPtrAuth    )));
  printf("\n");

  auto test_rwx_alloc = [&](size_t alloc_size, MemoryFlags flags, const char* flags_string) noexcept {
    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, alloc_size, flags | MemoryFlags::kAccessRWX);
    printf("  Alloc virt memory (RWX) : %s (alloc_size=%zu, flags=%s)\n", stringify_result(result), alloc_size, flags_string);

    if (result == Error::kOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* func_ptr = offset_pointer(ptr, kVirtFuncOffset);
      size_t func_size = write_empty_function_at(func_ptr, alloc_size);

      if (func_size) {
        flush_instruction_cache(func_ptr, func_size);
        invoke_void_function(make_authenticated(func_ptr));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release(ptr, alloc_size);
      printf("  Release virt memory     : %s\n", stringify_result(result));
    }
  };

  auto test_rw_rx_flip_alloc = [&](size_t alloc_size, MemoryFlags flags, const char* flags_string) noexcept {
    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, alloc_size, flags | MemoryFlags::kAccessRW | MemoryFlags::kMMapMaxAccessRWX);
    printf("  Alloc virt memory (RW)  : %s (alloc_size=%zu bytes, flags=%s)\n", stringify_result(result), alloc_size, flags_string);

    if (result == Error::kOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* func_ptr = offset_pointer(ptr, kVirtFuncOffset);
      size_t func_size = write_empty_function_at(func_ptr, alloc_size);
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, alloc_size, MemoryFlags::kAccessRX);
      printf("  Protect virt memory (RX): %s\n", stringify_result(result));

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (func_size) {
        flush_instruction_cache(func_ptr, func_size);
        invoke_void_function(make_authenticated(func_ptr));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, alloc_size, MemoryFlags::kAccessRW);
      printf("  Protect virt memory (RW): %s\n", stringify_result(result));

      result = VirtMem::release(ptr, alloc_size);
      printf("  Release virt memory (RW): %s\n", stringify_result(result));
    }
  };

  if (!rti.has_flag(HardenedRuntimeFlags::kEnabled)) {
    printf("Virtual Memory Allocation (RWX):\n");
    test_rwx_alloc(kVMemAllocSize, MemoryFlags::kNone, "kAccessRWX");
    printf("\n");
  }

  if (!rti.has_flag(HardenedRuntimeFlags::kEnabled) && large_page_size != 0u) {
    printf("Virtual Memory Allocation (RWX | LargePages):\n");
    test_rwx_alloc(large_page_size, MemoryFlags::kMMapLargePages, "kAccessRWX|kMMapLargePages");
    printf("\n");
  }

  {
    printf("Virtual Memory Allocation (Flipping Permissions RW<->RX):\n");
    test_rw_rx_flip_alloc(kVMemAllocSize, MemoryFlags::kNone, "kAccessRW|kMMapMaxAccessRWX");
    printf("\n");
  }

  if (large_page_size != 0u) {
    printf("Virtual Memory Allocation (Flipping Permissions RW<->RX | LargePages):\n");
    test_rw_rx_flip_alloc(large_page_size, MemoryFlags::kMMapLargePages, "kAccessRW|kMMapMaxAccessRWX|kMMapLargePages");
    printf("\n");
  }

  if (rti.has_flag(HardenedRuntimeFlags::kMapJit)) {
    printf("Virtual Memory Allocation (MAP_JIT):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX | MemoryFlags::kMMapEnableMapJit);
    printf("  Alloc virt mem (RWX)    : %s (allocation uses kMMapEnableMapJit)\n", stringify_result(result));

    if (result == Error::kOk) {
      printf("  Protect JIT Memory (RW) : (per-thread protection)\n");
      VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadWrite);

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* func_ptr = offset_pointer(ptr, kVirtFuncOffset);
      size_t func_size = write_empty_function_at(func_ptr, kVMemAllocSize);
#endif // TEST_ENVIRONMENT_HAS_JIT

      printf("  Protect JIT Memory (RX) : (per-thread protection)\n");
      VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadExecute);

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (func_size) {
        flush_instruction_cache(func_ptr, func_size);
        invoke_void_function(make_authenticated(func_ptr));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory     : %s\n", stringify_result(result));
    }

    printf("\n");
  }

  auto test_dual_mapping = [&](size_t alloc_size, MemoryFlags flags, const char* flags_string) noexcept {
    VirtMem::DualMapping dm {};
    Error result = VirtMem::alloc_dual_mapping(Out(dm), alloc_size, flags | MemoryFlags::kAccessRWX);
    printf("  Alloc dual mem (RW+RX)  : %s (alloc_size=%zu flags=%s)\n", stringify_result(result), alloc_size, flags_string);

    if (result == Error::kOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      size_t func_size = write_empty_function_at(offset_pointer(dm.rw, kVirtFuncOffset), alloc_size);
      if (func_size) {
        flush_instruction_cache(offset_pointer(dm.rx, kVirtFuncOffset), func_size);
        invoke_void_function(make_authenticated(offset_pointer(dm.rx, kVirtFuncOffset)));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release_dual_mapping(dm, alloc_size);
      printf("  Release dual mem (RW+RX): %s\n", stringify_result(result));
    }
  };

  if (rti.has_flag(HardenedRuntimeFlags::kDualMapping)) {
    printf("Virtual Memory Allocation (DualMapping):\n");
    test_dual_mapping(kVMemAllocSize, MemoryFlags::kNone, "kAccessRWX");
    printf("\n");
  }

  if (rti.has_flag(HardenedRuntimeFlags::kDualMapping) && large_page_size != 0u) {
    printf("Virtual Memory Allocation (DualMapping | LargePages):\n");
    test_dual_mapping(large_page_size, MemoryFlags::kMMapLargePages, "kAccessRWX|kMMapLargePages");
    printf("\n");
  }
}

#if defined(TEST_ENVIRONMENT_HAS_JIT)
static void print_jit_runtime_info_and_test_execution_with_params(const JitAllocator::CreateParams* params, const char* params_name) noexcept {
  printf("JitRuntime (%s):\n", params_name);

  JitRuntime rt(params);
  CodeHolder code;

  Error result = code.init(rt.environment());
  printf("  CodeHolder init result  : %s\n", stringify_result(result));

  if (result != Error::kOk) {
    return;
  }

  emit_void_function(code);
  VoidFunc fn;

  result = rt.add(&fn, &code);
  printf("  Runtime.add() result    : %s\n", stringify_result(result));

  if (result == Error::kOk) {
    // NOTE: JitRuntime provides an already authenticated function pointer, so don't sign it again.
    invoke_void_function((void*)fn);

    result = rt.release(fn);
    printf("  Runtime.release() result: %s\n", stringify_result(result));
  }

  printf("\n");
}

static void print_jit_runtime_info_and_test_execution() noexcept {
  print_jit_runtime_info_and_test_execution_with_params(nullptr, "Default");

  if (VirtMem::large_page_size()) {
    JitAllocator::CreateParams p{};
    p.options = JitAllocatorOptions::kUseLargePages;

    print_jit_runtime_info_and_test_execution_with_params(&p, "LargePages");
  }
}
#endif // TEST_ENVIRONMENT_HAS_JIT

#endif // !ASMJIT_NO_JIT

int main() {
  print_app_info();
  print_build_info();
  print_cpu_info();

#if !defined(ASMJIT_NO_JIT)
  print_virt_mem_info_and_test_execution();
#endif // !ASMJIT_NO_JIT

#if !defined(ASMJIT_NO_JIT) && defined(TEST_ENVIRONMENT_HAS_JIT)
  print_jit_runtime_info_and_test_execution();
#endif // !ASMJIT_NO_JIT && TEST_ENVIRONMENT_HAS_JIT

  return 0;
}
