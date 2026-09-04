// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
#endif

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/a64.h>
#endif

#include <asmjit-testing/commons/asmjit_utils.h>
#include <asmjit-testing/tests/zero_test.h>

#if !defined(ASMJIT_NO_COMPILER)
#include <asmjit/core/ra_cfg_block_p.h>
#include <asmjit/core/ra_inst_p.h>
#include <asmjit/core/ra_pass_p.h>
#endif

using namespace asmjit;

int main(int argc, const char* argv[]) {
  printf("AsmJit Test Runner v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  ZeroTest::TestPrecedence precedence_table[] = {
    {"axl*"},
    {"core*"},
    {"a64*"},
    {"x86*"},
    {"uni_compiler*"}
  };

  ZeroTest::TestRunner test_runner(argc, argv);
  test_runner.apply_precedence(precedence_table);

  auto message = [&](ZeroTest::MessageType msg_type, const char* msg, size_t msg_size) {
    (void)msg_type;
    fwrite(msg, 1, msg_size, stdout);
    fflush(stdout);
  };

  auto options_list = [&]() {
    auto MT = ZeroTest::MessageType::kRunnerMessage;
    test_runner.message(MT, "  --build-info  - print build information\n");
  };

  test_runner.on_message(message);
  test_runner.on_options_list(options_list);

  if (test_runner.has_arg("--build-info")) {
    print_build_info();
    return 0;
  }

  return test_runner.run_tests();
}
