AsmJit - Complete x86/x64 JIT and Remote Assembler for C++
==========================================================

Official Repository: https://github.com/kobalicekp/asmjit

Support the Project: [![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](
  https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=QDRM6SRNG7378&lc=EN;&item_name=asmjit&currency_code=EUR)

Introduction
------------

AsmJit is a complete JIT and remote assembler for C++ language. It can generate native code for x86 and x64 architectures having support for a full instruction set, from legacy MMX to the newest AVX2. It has a type-safe API that allows C++ compiler to do a semantic checks at compile-time even before the assembled code is generated or run.

AsmJit is not a virtual machine (VM). It doesn't have functionality to implement VM out of the box; however, it can be be used as a JIT backend for your own VM. The usage of AsmJit is not limited at all; it's suitable for multimedia, VM backends or remote code generation.

Features
--------

  * Complete x86/x64 instruction set - MMX, SSE, AVX, BMI, XOP, FMA...,
  * Low-level and high-level code generation,
  * Built-in CPU detection,
  * Virtual Memory management,
  * Pretty logging and error handling,
  * Small and embeddable, around 150kB compiled,
  * Zero dependencies, not even STL or RTTI.

Supported Environments
----------------------

### Operating Systems

  * BSDs
  * Linux
  * Mac
  * Windows

### C++ Compilers

  * BorlandC++
  * GNU (3.4.X+, 4.0+, MinGW)
  * MSVC (VS2005, VS2008, VS2010)
  * Other compilers require testing

### Backends

  * X86
  * X64

Project Organization
--------------------

  - project root /
    - extras     - Documentation and addons
      - contrib  - Contributions (not official, but included)
      - doc      - Documentation generator files
      - msvs     - MS Visual Studio additions
    - scripts    - Scripts to generate project files and regenerate defs
    - src        - Source code
      - asmjit   - Public header files (always include from here)
        - base   - Base files, used by the AsmJit and all backends
        - x86    - X86/X64 specific files, used only by X86/X64 backend

Code Generation Concepts
------------------------

AsmJit has two completely different code generation concepts. The difference is in how the code is generated. The first concept, also referred as the low level concept, is called 'Assembler' and it's the same as writing RAW assembly by using physical registers directly. In this case AsmJit does only instruction encoding, verification and relocation.

The second concept, also referred as the high level concept, is called 'Compiler'. Compiler lets you use virtually unlimited number of registers (called variables) significantly simplifying the code generation process. Compiler allocates these virtual registers to physical registers after the code generation is done. This requires some extra effort - Compiler has to generate information for each node (instruction, function declaration, function call) in the code, perform a variable liveness analysis and translate the code having variables into code having only registers.

In addition, Compiler understands functions and function calling conventions. It has been designed in a way that the code generated is always a function having prototype like in a programming language. By having a function prototype the Compiler is able to insert prolog and epilog to a function being generated and it is able to call a function inside a generated one.

There is no conclusion on which concept is better. Assembler brings full control on how the code is generated, while Compiler makes the generation more portable.

Configuring & Building
----------------------

AsmJit is designed to be easy embeddable in any project. However, it has some compile-time flags that can be used to build a specific version of AsmJit including or omitting certain features:

### Debugging

  - *ASMJIT_DEBUG* - Define to always turn debugging on (regardless of build-mode).
  - *ASMJIT_RELEASE* - Define to always turn debugging off (regardless of build-mode).

  - By default none of these is defined, AsmJit detects mode based on compile-time macros (useful when using IDE that has switches for Debug/Release/etc...).

### Library

  - *ASMJIT_STATIC* - Define when building AsmJit as a static library. No symbols will be exported by AsmJit by default.
  - *ASMJIT_API* - This is AsmJit API decorator that is used in all functions that has to be exported. It can be redefined, however it's not a recommended way.

  - By default AsmJit build is configured as a shared library and *ASMJIT_API* contains compiler specific attributes to import/export AsmJit symbols.

### Backends

  - *ASMJIT_BUILD_X86* - Always build x86 backend regardless of host architecture.
  - *ASMJIT_BUILD_X64* - Always build x64 backend regardless of host architecture.
  - *ASMJIT_BUILD_HOST* - Always build host backand, if only *ASMJIT_BUILD_HOST* is used only the host architecture detected at compile-time will be included.

  - By default only *ASMJIT_BUILD_HOST* is defined.

To build AsmJit please use cmake <http://www.cmake.org> that will generate  project files for your favorite IDE and platform. If you don't use cmake and you still want to include AsmJit in your project it's perfectly fine by just including it there, probably defining *ASMJIT_STATIC* to prevent AsmJit trying to export the API.

Examples
--------

Comprehensive test suite can be found at src/app/test directory in AsmJit package. It can be used as a starting point before using AsmJit.

License
-------

AsmJit can be distributed under zlib license:

  * <http://www.opensource.org/licenses/zlib-license.php>

Google Groups & Mailing Lists
-----------------------------

AsmJit google group:

  * http://groups.google.com/group/asmjit-dev

AsmJit mailing list:

  * asmjit-dev@googlegroups.com

Contact Authors & Maintainers
-----------------------------

  * Petr Kobalicek <kobalicek.petr@gmail.com>
