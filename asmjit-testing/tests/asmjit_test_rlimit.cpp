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

#if !defined(_WIN32)
  #include <sys/resource.h>
#endif

using namespace asmjit;

static void print_app_info() {
  printf("AsmJit RLimit Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  printf("This application can be used to verify AsmJit functionality when it comes to\n"
         "resource limits. It's only useful on platforms that provide rlimit syscall\n"
         "and support dual-mapping that can only be backed by a file.\n"
         "\n");
}

#if !defined(_WIN32) && defined(RLIMIT_FSIZE) && !defined(ASMJIT_NO_JIT)

#if !defined(ASMJIT_NO_X86) && ASMJIT_TARGET_ARCH_X86 != 0
#define TEST_ENVIRONMENT_HAS_RLIMIT_AND_JIT
static void emit_void_function_with_data(CodeHolder& code, size_t extra_data) noexcept {
  x86::Assembler a(&code);
  a.ret();

  for (size_t i = 0; i < extra_data / 8; i++) {
    a.embed_uint64(0u);
  }
}
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_TARGET_ARCH_ARM == 64
#define TEST_ENVIRONMENT_HAS_RLIMIT_AND_JIT
static void emit_void_function_with_data(CodeHolder& code, size_t extra_data) noexcept {
  a64::Assembler a(&code);
  a.ret(a64::x30);

  for (size_t i = 0; i < extra_data / 8; i++) {
    a.embed_uint64(0u);
  }
}
#endif
#endif

#if defined(TEST_ENVIRONMENT_HAS_RLIMIT_AND_JIT)

using VoidFunc = void (ASMJIT_CDECL*)(void);

static const char* stringify_result(Error err) noexcept {
  return err == Error::kOk ? "success" : stringify_error(err);
};

static void invoke_void_function(void* ptr) noexcept {
  printf("  Invoke JIT code at addr : %p\n", ptr);

  // In case it crashes, we want to have the output flushed.
  fflush(stdout);

  VoidFunc func = reinterpret_cast<VoidFunc>(ptr);
  func();
}

static Error test_jit_runtime_function(size_t block_size, size_t extra_size) noexcept {
  JitAllocator::CreateParams allocator_params{
    .options = JitAllocatorOptions::kUseDualMapping,
    .block_size = static_cast<uint32_t>(block_size)
  };

  JitRuntime rt(allocator_params);
  CodeHolder code;

  Error result = code.init(rt.environment());
  printf("  CodeHolder init result  : %s\n", stringify_result(result));

  if (result != Error::kOk) {
    return result;
  }

  emit_void_function_with_data(code, extra_size);
  VoidFunc fn;

  result = rt.add(&fn, &code);
  printf("  Runtime.add() result    : %s\n", stringify_result(result));

  if (result != Error::kOk) {
    printf("\n");
    return result;
  }
  else {
    invoke_void_function((void*)fn);

    result = rt.release(fn);
    printf("  Runtime.release() result: %s\n\n", stringify_result(result));
    return result;
  }
}

int main() {
  print_app_info();

  constexpr size_t MiB = 1024 * 1024;

  struct rlimit rlim;
  printf("Trying to read resource limits\n");
  if (getrlimit(RLIMIT_FSIZE, &rlim) != 0) {
    printf("Failed to read limit 'RLIMIT_FSIZE', terminating...\n");
    return 1;
  }

  printf("The current 'RLIMIT_FSIZE' limit is: [cur=%zu max=%zu]\n",
    static_cast<size_t>(rlim.rlim_cur),
    static_cast<size_t>(rlim.rlim_max));

  if (static_cast<uint64_t>(rlim.rlim_cur) < 2u * MiB) {
    printf("The current limit is less than 2MiB, terminating...\n");
    return 1;
  }

  if (!VirtMem::hardened_runtime_info().has_flag(VirtMem::HardenedRuntimeFlags::kDualMapping)) {
    printf("\n");
    printf("** WARNING: Cannot continue as dual-mapping is not supported on this platform **\n");
    return 0;
  }

  // Assume that without any restrictions we can allocate at least 2MiB of contiguous memory.
  printf("\n");
  printf("Testing JitRuntime with 2 MiB executable blocks and 1 MiB function core/data\n");
  if (test_jit_runtime_function(2 * MiB, 1 * MiB) != Error::kOk) {
    printf("Failed...\n");
    return 1;
  }

  if (rlim.rlim_cur < static_cast<rlim_t>(2 * MiB)) {
    printf("Resource limit is already too low, the previous code should have failed...\n");
    return 1;
  }

  printf("Trying to decrease 'RLIMIT_FSIZE' limit to: 1 MiB\n");
  rlim.rlim_cur = static_cast<rlim_t>(1 * MiB);
  rlim.rlim_max = static_cast<rlim_t>(1 * MiB);
  if (setrlimit(RLIMIT_FSIZE, &rlim) != 0) {
    printf("Failed to decrease the limit\n");
    return 1;
  }

  printf("Rereading resource limits to verify they were applied properly\n");
  if (getrlimit(RLIMIT_FSIZE, &rlim) != 0) {
    printf("Failed to read limit 'RLIMIT_FSIZE', terminating...\n");
    return 1;
  }

  printf("The current 'RLIMIT_FSIZE' limit is: [cur=%zu max=%zu]\n",
    static_cast<size_t>(rlim.rlim_cur),
    static_cast<size_t>(rlim.rlim_max));

  // NOTE: Not all platforms that offer RLIMIT_FSIZE would actually restrict dual-mapped memory. This
  // applies to dual mappings that are backed by a file (could be memfd in Linux case), however, if the
  // platform supports dual mapping without the need of a file, then the whole RLIMIT_FSIZE would not
  // work. So, verify whether AsmJit actually sees the restricted values, and if not, change what the
  // test expects.
  size_t max_dual_mapping_size = VirtMem::dual_mapping_size_limit();

  printf("\n");
  printf("Testing JitRuntime with 2 MiB executable blocks and 1 MiB function core/data\n");

  Error result = test_jit_runtime_function(2 * MiB, 1 * MiB);
  if (max_dual_mapping_size <= 1 * MiB) {
    if (result == Error::kOutOfMemory) {
      printf("** SUCCESS: 'OutOfMemory' error was expected, because of applied resource limits **\n");
      return 0;
    }
    else {
      printf("** FAILURE: RLIMIT_FSIZE didn't work, although AsmJit thinks it should have worked **\n");
      return 1;
    }
  }
  else {
    if (result == Error::kOk) {
      printf("** SUCCESS: RLIMIT_FSIZE doesn't influence dual-mapping on this platform **\n");
      return 0;
    }
    else {
      printf("** FAILURE: RLIMIT_FSIZE doesn't influence dual-mapping, but it did? **\n");
      return 1;
    }
  }
}

#else

int main() {
  print_app_info();

  printf("This application does nothing because the platform doesn't support\n"
         "rlimit or AsmJit cannot generate code for the target architecture.\n");
  return 0;
}

#endif
