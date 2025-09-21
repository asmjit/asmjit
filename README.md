AsmJit
------

AsmJit is a lightweight library for machine code generation written in C++ language.

  * [Official Home Page (asmjit.com)](https://asmjit.com)
  * [Official Repository (asmjit/asmjit)](https://github.com/asmjit/asmjit)
  * [Public Chat Channel](https://app.gitter.im/#/room/#asmjit:gitter.im)
  * [Zlib License](./LICENSE.md)

See [asmjit.com](https://asmjit.com) page for more details, examples, and documentation.

Project Organization
--------------------

  * **`/`**        - Project root
    * **src**      - Source code
      * **asmjit** - Source code and headers (always point include path in here)
        * **core** - Core API, backend independent except relocations
        * **arm**  - ARM specific API, designed to be common for both AArch32 and AArch64
        * **a64**  - AArch64 specific API, used only by AArch64 backends
        * **x86**  - X86 specific API, used only by X86 and X64 backends
        * **ujit** - Universal JIT API
    * **test**     - Unit and integration tests (don't embed in your project)
    * **tools**    - Tools used for configuring, documenting, and generating files

Roadmap
-------

  * See [Roadmap](https://asmjit.com/roadmap.html) page for more details

Contributing
------------

  * See [CONTRIBUTING](./CONTRIBUTING.md) page for more details

Documentation
-------------

  * [Documentation Index](https://asmjit.com/doc/index.html)
  * [Build Instructions](https://asmjit.com/doc/group__asmjit__build.html) (includes [CMake Integration](https://asmjit.com/doc/group__asmjit__build.html#cmake_integration))

Breaking Changes
----------------

Breaking the API is sometimes inevitable, what to do?

  * See [Breaking Changes Guide](https://asmjit.com/doc/group__asmjit__breaking__changes.html), which is now part of AsmJit documentation
  * See asmjit tests, they always compile and provide implementation of many use-cases:
    * [asmjit_test_emitters.cpp](./test/asmjit_test_emitters.cpp) - Tests that demonstrate the purpose of emitters
    * [asmjit_test_assembler_x86.cpp](./test/asmjit_test_assembler_x86.cpp) - Tests targeting AsmJit's Assembler (x86/x64)
    * [asmjit_test_compiler_x86.cpp](./test/asmjit_test_compiler_x86.cpp) - Tests targeting AsmJit's Compiler (x86/x64)
    * [asmjit_test_instinfo.cpp](./test/asmjit_test_instinfo.cpp) - Tests that query instruction information
    * [asmjit_test_x86_sections.cpp](./test/asmjit_test_x86_sections.cpp) - Multiple sections test
  * Visit our [Gitter Chat](https://app.gitter.im/#/room/#asmjit:gitter.im) if you need a quick help

Support
-------

  * AsmJit project has both community and commercial support, see [AsmJit's Support Page](https://asmjit.com/support.html)
  * Organizations that rely on AsmJit should support the development!

Notable Donors List:

  * [ZehMatt](https://github.com/ZehMatt)

Authors & Maintainers
---------------------

  * Petr Kobalicek <kobalicek.petr@gmail.com> ([website](https://kobalicek.com))
