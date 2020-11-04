import argparse
import os
import platform
import subprocess


source_root = os.path.normpath(
  os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", ".."))

ubuntu_test_toolchain_ppa = "ppa:ubuntu-toolchain-r/test"

tests = [
  { "cmd": ["asmjit_test_unit", "--quick"] },
  { "cmd": ["asmjit_test_opcode", "--quiet"] },
  { "cmd": ["asmjit_test_x86_asm"] },
  { "cmd": ["asmjit_test_x86_sections"] },
  { "cmd": ["asmjit_test_x86_instinfo"] },
  { "cmd": ["asmjit_test_compiler"] }
]

sanitize_macro = "ASMJIT_SANITIZE"

valgrind_args = [
  "valgrind",
  "--leak-check=full",
  "--show-reachable=yes",
  "--track-origins=yes"
]


def log(message):
  print(message, flush=True)


def run(args, cwd=None, env=None, sudo=False, print_command=True):
  if sudo:
    args = ["sudo"] + args

  if print_command:
    log(" ".join(args))

  subprocess.run(args, cwd=cwd, env=env, check=True)


def main():
  parser = argparse.ArgumentParser(description="Step executor")

  parser.add_argument("--step", default=None, help="Step to execute (prepare|configure|build|test)")
  parser.add_argument("--compiler", default="", help="C++ compiler to use")
  parser.add_argument("--generator", default="", help="CMake generator to use")
  parser.add_argument("--architecture", default="default", help="Target architecture")
  parser.add_argument("--build-dir", default="build", help="Build directory")
  parser.add_argument("--build-type", default="", help="Build type (Debug / Release)")
  parser.add_argument("--build-defs", default="", help="Build definitions")
  parser.add_argument("--diagnose", default="", help="Diagnostics (valgrind|address|undefined)")

  args = parser.parse_args()
  step = args.step
  compiler = args.compiler
  generator = args.generator
  architecture = args.architecture.lower()
  diagnose = args.diagnose

  platform_name = platform.system()

  if not compiler:
    compiler = ""

  if not generator:
    if compiler == "vs2015":
      generator = "Visual Studio 14 2015"
    elif compiler == "vs2017":
      generator = "Visual Studio 15 2017"
    elif compiler == "vs2019":
      generator = "Visual Studio 16 2019"
    elif platform_name == "Darwin":
      generator = "Unix Makefiles"
    else:
      generator = "Ninja"


  # ---------------------------------------------------------------------------
  if step == "prepare":
    if platform_name == "Windows":
      pass

    elif platform_name == "Darwin":
      pass

    elif platform_name == "Linux":
      if compiler.startswith("gcc"):
        compiler_package = compiler.replace("gcc", "g++")
      elif compiler.startswith("clang"):
        compiler_package = compiler
      else:
        raise ValueError("Invalid compiler: {}".format(compiler))

      apt_packages = [compiler_package]

      if generator == "Ninja":
        apt_packages.append("ninja-build")

      if architecture == "x86":
        run(["dpkg", "--add-architecture", "i386"], sudo=True)
        apt_packages.append("linux-libc-dev:i386")
        if compiler.startswith("gcc"):
          apt_packages.append(compiler_package + "-multilib")
        else:
          apt_packages.append("g++-multilib") # Even clang requires this.

      if diagnose == "valgrind":
        apt_packages.append("valgrind")

      run(["apt-add-repository", "-y", ubuntu_test_toolchain_ppa], sudo=True)
      run(["apt-get", "update", "-qq"], sudo=True)
      run(["apt-get", "install", "-qq"] + apt_packages, sudo=True)

    else:
      raise ValueError("Unknown platform: {}".format(platform_name))

    exit(0)
  # ---------------------------------------------------------------------------


  # ---------------------------------------------------------------------------
  if step == "configure":
    os.makedirs(args.build_dir, exist_ok=True)

    cmd = ["cmake", source_root, "-G" + generator]
    env = os.environ.copy()

    if generator.startswith("Visual Studio"):
      if architecture == "x86":
        cmd.extend(["-A", "Win32"])
      else:
        cmd.extend(["-A", "x64"])
    else:
      if compiler.startswith("gcc"):
        cc_bin = compiler
        cxx_bin = compiler.replace("gcc", "g++")
      elif compiler.startswith("clang"):
        cc_bin = compiler
        cxx_bin = compiler.replace("clang", "clang++")
      else:
        raise ValueError("Invalid compiler: {}".format(compiler))

      env["CC"] = cc_bin
      env["CXX"] = cxx_bin

      if architecture == "x86":
        env["CFLAGS"] = "-m32"
        env["CXXFLAGS"] = "-m32"
        env["LDFLAGS"] = "-m32"

      if args.build_type:
        cmd.append("-DCMAKE_BUILD_TYPE=" + args.build_type)

    if args.build_defs:
      for build_def in args.build_defs.split(","):
        cmd.append("-D" + build_def)

    if diagnose and diagnose != "valgrind":
      cmd.append("-D" + sanitize_macro + "=" + diagnose)

    run(cmd, cwd=args.build_dir, env=env)
    exit(0)
  # ---------------------------------------------------------------------------


  # ---------------------------------------------------------------------------
  if step == "build":
    cmd = ["cmake", "--build", "."]

    if generator.startswith("Visual Studio"):
      cmd.extend(["--config", args.build_type, "--", "-nologo", "-v:minimal"])

    run(cmd, cwd=args.build_dir)
    exit(0)
  # ---------------------------------------------------------------------------


  # ---------------------------------------------------------------------------
  if step == "test":
    build_dir = args.build_dir
    build_type = args.build_type

    # Multi-Configuration build use a nested directory.
    if os.path.isdir(os.path.join(build_dir, build_type)):
      build_dir = os.path.join(build_dir, build_type)

    for test in tests:
      cmd = [] + test["cmd"]
      test_name = cmd[0]
      executable = os.path.abspath(os.path.join(build_dir, cmd[0]))

      if platform_name == "Windows":
        executable += ".exe"

      # Ignore tests, which were not built, because of disabled features.
      if os.path.isfile(executable):
        try:
          log("::group::" + test_name)
          log(" ".join(cmd))

          if diagnose == "valgrind":
            cmd = valgrind_args + cmd

          cmd[0] = executable
          run(cmd, cwd=build_dir, print_command=False)
        finally:
          log("::endgroup::")

    exit(0)
  # ---------------------------------------------------------------------------

  raise ValueError("Invalid step: {}".format(step))


if __name__ == "__main__":
  main()
