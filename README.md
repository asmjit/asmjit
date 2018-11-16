AsmJit
------

Complete x86/x64 JIT and Remote Assembler for C++.

  * [Official Repository (asmjit/asmjit)](https://github.com/asmjit/asmjit)
  * [Official Blog (asmbits)](https://asmbits.blogspot.com/ncr)
  * [Official Chat (gitter)](https://gitter.im/asmjit/asmjit)
  * [Permissive ZLIB license](./LICENSE.md)

Introduction
------------

AsmJit is a complete JIT and remote assembler for C++ language. It can generate native code for x86 and x64 architectures and supports the whole x86/x64 instruction set - from legacy MMX to the newest AVX512. It has a type-safe API that allows C++ compiler to do semantic checks at compile-time even before the assembled code is generated and/or executed.

AsmJit, as the name implies, started as a project that provided JIT code-generation and execution. However, AsmJit evolved and it now contains features that are far beyond the scope of a simple JIT compilation. To keep the library small and lightweight the functionality not strictly related to JIT is provided by a sister project called [asmtk](https://github.com/asmjit/asmtk).

Minimal Example
---------------

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(jit.codeInfo());              // Initialize to the same arch as JIT runtime.

  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.
  a.mov(x86::eax, 1);                     // Move one to 'eax' register.
  a.ret();                                // Return from function.
  // ----> x86::Assembler is no longer needed from here and can be destroyed <----

  Func fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "1".

  // All classes use RAII, all resources will be released before `main()` returns,
  // the generated function can be, however, released explicitly if you intend to
  // reuse or keep the runtime alive, which you should in a production-ready code.
  jit.release(fn);

  return 0;
}
```

AsmJit Summary
--------------

  * Complete x86/x64 instruction set - MMX, SSEx, BMIx, ADX, TBM, XOP, AVXx, FMAx, and AVX512.
  * Different emitters providing various abstraction levels (Assembler, BaseBuilder, BaseCompiler).
  * Built-in CPU vendor and features detection.
  * Advanced logging/formatting and robust error handling.
  * JIT memory allocator - interface similar to malloc/free for JIT code-generation and execution.
  * Lightweight and easily embeddable - 200-250kB compiled with all built-in features.
  * Modular - unneeded features can be disabled at compile-time to make the library even smaller.
  * Zero dependencies - no external libraries, no STL/RTTI - easy to embed and/or link statically.
  * Doesn't use exceptions internally, but allows to attach a "throwable" error handler of your choice.

Advanced Features
-----------------

  * AsmJit contains a highly compressed instruction database:
    * Instruction names - allows to convert instruction id to its name and vice versa.
    * Instruction metadata - access (read|write|rw) of all operand combinations of all instructions.
    * Instruction signatures - allows to strictly validate if an instruction (with all its operands) is valid.
  * AsmJit allows to precisely control how instructions are encoded if there are multiple variations.
  * AsmJit is highly dynamic, constructing operands at runtime is a common practice.
  * Multiple emitters with the same interface - emit machine code directly or to a representation that can be processed afterwards.

Important
---------

Breaking the official API is sometimes inevitable, what to do?
  * See asmjit tests, they always compile and provide an implementation of a lot of use-cases:
    * [asmjit_test_x86_asm.cpp](./test/asmjit_test_x86_asm.cpp) - Tests that use all emitters.
    * [asmjit_test_x86_cc.cpp](./test/asmjit_test_x86_cc.cpp) - Tests that use **x86::Compiler**.
  * Visit our [Official Chat](https://gitter.im/asmjit/asmjit) if you need a quick help.

TODOs:
  * [ ] Add support for user external buffers.
  * [ ] AsmJit added support for code sections, but only the first section (executable code) works atm.

Supported Environments
----------------------

### C++ Compilers:

  * Features:
    * AsmJit won't build without C++11 enabled. If you use older GCC and Clang you would have to enable at least c++11 through compiler flags.
  * Tested:
    * **Clang** - tested by Travis-CI - Clang 3.9+ (with C++11 enabled) is officially supported (older Clang versions having C++11 support are probably fine, but are not regularly tested).
    * **GNU** - tested by Travis-CI - GCC 4.8+ (with C++11 enabled) is officially supported.
    * **MINGW** - tested by AppVeyor - Use the latest version if possible.
    * **MSVC** - tested by AppVeyor - **MSVC2017+ only!** - there is a severe bug in MSVC2015's `constexpr` implementation that makes that compiler unusable.
  * Untested:
    * **Intel** - no maintainers, no CI environment to regularly test this compiler.
    * Other c++ compilers would require basic support in [core/build.h](./src/asmjit/core/build.h).

### Operating Systems:

  * Tested:
    * **Linux** - tested by Travis-CI - any distribution is generally supported.
    * **Mac** - tested by Travis-CI - any version is supported.
    * **Windows** - tested by AppVeyor - Windows 7+ is officially supported although Windows XP should work as well.
  * Untested:
    * **BSDs** - no maintainers, no CI environment to regularly test this OS.
    * Other operating systems would require some testing and support in [core/build.h](./src/asmjit/core/build.h) and [core/osutils.cpp](./src/asmjit/core/osutils.cpp).

### Backends:

  * **X86** - tested by both Travis-CI and AppVeyor - both 32-bit and 64-bit backends are fully functional.
  * **ARM** - work-in-progress (not public at the moment).

Project Organization
--------------------

  * **`/`**        - Project root.
    * **src**      - Source code.
      * **asmjit** - Source code and headers (always point include path in here).
        * **core** - Core API, backend independent.
        * **arm**  - ARM specific API, used only by ARM32 and ARM64 backends.
        * **x86**  - X86 specific API, used only by X86 and X64 backends.
    * **test**     - Unit and integration tests (don't embed in your project).
    * **tools**    - Tools used for configuring, documenting and generating data files.

Configuring & Feature Selection
-------------------------------

AsmJit is designed to be easy embeddable in any project. However, it depends on some compile-time macros that can be used to build a specific version of AsmJit that includes or excludes certain features. A typical way of building AsmJit is to use [cmake](https://www.cmake.org), but it's also possible to just include AsmJit source code in your project and just build it. The easiest way to include AsmJit in your project is to just include **src** directory in your project and to define `ASMJIT_BUILD_STATIC` or `ASMJIT_BUILD_EMBED`. AsmJit can be just updated from time to time without any changes to this integration process. Do not embed AsmJit's [/test](./test) files in such case as these are used for testing.

### Build Type:

  * `ASMJIT_BUILD_DEBUG` - Define to always turn debugging on (regardless of compile-time options detected).
  * `ASMJIT_BUILD_RELEASE` - Define to always turn debugging off (regardless of compile-time options detected).

By default none of these is defined, AsmJit detects build-type based on compile-time macros and supports most IDE and compiler settings out of box. By default AsmJit switches to release mode when `NDEBUG` is defined.

### Build Mode:

  * `ASMJIT_BUILD_EMBED` - Define to embed AsmJit in another project. Embedding means that neither shared nor static library is created and AsmJit's source files and source files of the product that embeds AsmJit are part of the same target. This way of building AsmJit has certain advantages that are beyond this manual. `ASMJIT_BUILD_EMBED` behaves similarly to `ASMJIT_BUILD_STATIC` (no API exports).
  * `ASMJIT_BUILD_STATIC` - Define to build AsmJit as a static library. No symbols are exported in such case.

By default AsmJit build is configured to be built as a shared library, thus none of `ASMJIT_BUILD_EMBED` and `ASMJIT_BUILD_STATIC` is defined.

### Build Backends:

  * `ASMJIT_BUILD_ARM` - Build ARM backends (not ready, work-in-progress).
  * `ASMJIT_BUILD_X86` - Build X86 backends (X86 and X86_64).
  * `ASMJIT_BUILD_HOST` - Build only the host backend (default).

If none of `ASMJIT_BUILD_...` is defined AsmJit bails to `ASMJIT_BUILD_HOST`, which will detect the target architecture at compile-time. Each backend automatically supports 32-bit and 64-bit targets, so for example AsmJit with X86 support can generate both 32-bit and 64-bit code.

### Disabling Features:

  * `ASMJIT_DISABLE_BUILDER` - Disables both `BaseBuilder` and `BaseCompiler` emitters (only `Assembler` will be available). Ideal for users that don't use `BaseBuilder` concept and want to have AsmJit a bit smaller.
  * `ASMJIT_DISABLE_COMPILER` - Disables `BaseCompiler` emitter. For users that use `BaseBuilder`, but not `BaseCompiler`.
  * `ASMJIT_DISABLE_JIT` - Disables JIT execution engine, which includes `JitUtils`, `JitAllocator`, and `JitRuntime`.
  * `ASMJIT_DISABLE_LOGGING` - Disables logging (`Logger` and all classes that inherit it) and instruction formatting.
  * `ASMJIT_DISABLE_TEXT` - Disables everything that uses text-representation and that causes certain strings to be stored in the resulting binary. For example when this flag is enabled all instruction and error names (and related APIs) will not be available. This flag has to be disabled together with `ASMJIT_DISABLE_LOGGING`. This option is suitable for deployment builds or builds that don't want to reveal the use of AsmJit.
  * `ASMJIT_DISABLE_INST_API` - Disables strict validation, read/write information, and all additional data and APIs that can output information about instructions.

NOTE: Please don't disable any features if you plan to build AsmJit as a shared library that will be used by multiple projects that you don't control (for example asmjit in a Linux distribution). The possibility to disable certain features exists mainly for customized builds of AsmJit.

Using AsmJit
------------

AsmJit library uses one global namespace called `asmjit` that provides the whole functionality. Architecture specific code is prefixed by the architecture name and architecture specific registers and operand builders have their own namespace. For example API targeting both X86 and X64 architectures is prefixed with `X86` and registers & operand builders are accessible through `x86` namespace. This design is very different from the initial version of AsmJit and it seems now as the most convenient one.

### CodeHolder & Emitters

AsmJit provides two classes that are used together for code generation:

  * `CodeHolder` - Provides functionality to hold generated code and stores all necessary information about code sections, labels, symbols, and possible relocations.
  * `[Base]Emitter` - Provides functionality to emit code into `CodeHolder`. `BaseEmitter` is abstract and provides just basic building blocks that are then implemented by `BaseAssembler`, `BaseBuilder`, `BaseCompiler`, and their architecture-specific implementations like `x86::Assembler`, `x86::Builder`, and `x86::Compiler`.

Code emitters:

  * `[Base]Assembler` - Emitter designed to emit machine code directly into a `CodeBuffer` held by `CodeHolder`.
  * `[Base]Builder` - Emitter designed to emit code into a representation that can be processed afterwards. It stores the whole code in a double linked list consisting of nodes (`BaseNode` and all derived classes). There are nodes that represent instructions (`InstNode`), labels (`LabelNode`), and other building blocks (`AlignNode`, `DataNode`, ...). Some nodes are used as markers (`SentinelNode` and comments (`CommentNode`).
  * `[Base]Compiler` - High-level code emitter that uses virtual registers and contains high-level function building features. Compiler extends `[Base]Builder` functionality and introduces new nodes like `FuncNode`, `FuncRetNode`, and `FuncCallNode`. Compiler is the simplest way to start with AsmJit as it abstracts lots of details required to generate a function that can be called from a C/C++ language.

### Runtime

AsmJit's `Runtime` is designed for execution and/or linking. The `Runtime` itself is abstract and defines only how to `add()` and `release()` code held by `CodeHolder`. `CodeHolder` holds machine code and relocation entries, but should be seen as a temporary object only - after the code in `CodeHolder` is ready, it should be passed to `Runtime` or relocated manually. Users interested in inspecting the generated machine-code (instead of executing or linking) can keep it in `CodeHodler` and process it manually of course.

The only `Runtime` implementation provided directly by AsmJit is called `JitRuntime`, which is suitable for storing and executing dynamically generated code. `JitRuntime` is used in most AsmJit examples as it makes the code management easy. It allows to add and release dynamically generated functions easily, so it's suitable for JIT code generators that want to keep many functions alive, and possibly release functions which are no longer needed.

### Instructions & Operands

Instructions specify operations performed by the CPU, and operands specify the operation's input(s) and output(s). Each AsmJit's instruction has it's own unique id (`Inst::Id` for example) and platform specific code emitters always provide a type safe intrinsic (or multiple overloads) to emit such instruction. There are two ways of emitting an instruction:

  * Using `BaseEmitter::inst(operands...)` - A type-safe way provided by platform specific emitters - for example `x86::Assembler` provides `x86::Assembler::mov(x86::Gp, x86::Gp)`.
  * Using `BaseEmitter::emit(instId, operands...)` - Allows to emit an instruction in a dynamic way - you just need to know instruction's id and provide its operands.

AsmJit's operands all inherit from a base class called `Operand` and then specialize its type to:

  * **None** (not used or uninitialized operand).
  * **Register** (`BaseReg`) - Describes either physical or virtual register. Physical registers have id that matches the target's machine id directly whereas virtual registers must be allocated into physical registers by a register allocator pass. Register operand provides:
    * **Register Type** - Unique id that describes each possible register provided by the target architecture - for example X86 backend provides `x86::Reg::RegType`, which defines all variations of general purpose registers (GPB-LO, GPB-HI, GPW, GPD, and GPQ) and all types of other registers like K, MM, BND, XMM, YMM, and ZMM.
    * **Register Group** - Groups multiple register types under a single group - for example all general-purpose registers (of all sizes) on X86 are `x86::Reg::kGroupGp`, all SIMD registers (XMM, YMM, ZMM) are `x86::Reg::kGroupVec`, etc.
    * **Register Size** - Contains the size of the register in bytes. If the size depends on the mode (32-bit vs 64-bit) then generally the higher size is used (for example RIP register has size 8 by default).
    * **Register ID** - Contains physical or virtual id of the register.
    * Each architecture provides its own register that adds a architecture-specific API to `BaseReg`.
  * **Memory Address** (`BaseMem`) - Used to reference a memory location. Memory operand provides:
    * **Base Register** - A base register type and id (physical or virtual).
    * **Index Register** - An index register type and id (physical or virtual).
    * **Offset** - Displacement or absolute address to be referenced (32-bit if base register is used and 64-bit if base register is not used).
    * **Flags** that can describe various architecture dependent information (like scale and segment-override on X86).
    * Each architecture provides its own register that adds a architecture-specific API to `BaseMem`.
  * **Immediate Value** (`Imm`) - Immediate values are usually part of instructions (encoded within the instruction itself) or data.
  * **Label** - used to reference a location in code or data. Labels must be created by the `BaseEmitter` or by `CodeHolder`. Each label has its unique id per `CodeHolder` instance.

AsmJit allows to construct operands dynamically, to store them, and to query a complete information about them at run-time. Operands are small (always 16 bytes per `Operand`) and should be always copied (by value) if you intend to store them (don't create operands by using `new` keyword, it's not recommended). Operands are safe to be `memcpy()`ed and `memset()`ed if you need to work with arrays of operands.

Small example of manipulating and using operands:

```c++
using namespace asmjit;

x86::Gp dstRegByValue() { return x86::ecx; }

void usingOperandsExample(x86::Assembler& a) {
  // Create some operands.
  x86::Gp dst = dstRegByValue();          // Get `ecx` register returned by a function.
  x86::Gp src = x86::rax;                 // Get `rax` register directly from the provided `x86` namespace.
  x86::Gp idx = x86::gpq(10);             // Construct `r10` dynamically.
  x86::Mem m = x86::ptr(src, idx);        // Construct [src + idx] memory address - referencing [rax + r10].

  // Examine `m`:
  m.indexType();                          // Returns `x86::Reg::kTypeGpq`.
  m.indexId();                            // Returns 10 (`r10`).

  // Reconstruct `idx` stored in mem:
  x86::Gp idx_2 = x86::Gp::fromTypeAndId(m.indexType(), m.indexId());
  idx == idx_2;                           // True, `idx` and idx_2` are identical.

  Operand op = m;                         // Possible.
  op.isMem();                             // True (can be casted to BaseMem or architecture-specific Mem).

  m == op;                                // True, `op` is just a copy of `m`.
  static_cast<BaseMem&>(op).addOffset(1); // Static cast is fine and valid here.
  op.as<BaseMem>().addOffset(1);          // However, using `as<T>()` to cast to a derived type is preferred.
  m == op;                                // False, `op` now points to [rax + r10 + 1], which is not [rax + r10].

  // Emitting 'mov'
  a.mov(dst, m);                          // Type-safe way.
  a.mov(dst, op);                         // Not possible, `mov` doesn't provide `mov(x86::Gp, Operand)` overload.

  a.emit(x86::Inst::kIdMov, dst, m);      // Type-unsafe, but possible.
  a.emit(x86::Inst::kIdMov, dst, op);     // Also possible, `emit()` is typeless and can be used with raw `Operand`s.
}
```

Some operands have to be created explicitly by `BaseEmitter`. For example labels must be created by `newLabel()` before they are used.

### Assembler Example

`x86::Assembler` is a code emitter that emits machine code into a CodeBuffer directly. It's capable of targeting both 32-bit and 64-bit instruction sets and it's possible to target both instruction sets within the same code-base. The following example shows how to generate a function that works in both 32-bit and 64-bit modes, and how to use JitRuntime, `CodeHolder`, and `x86::Assembler` together.

The example handles 3 calling conventions manually just to show how it could be done, however, AsmJit contains utilities that can be used to create function prologs and epilogs automatically, but these concepts will be explained later.

```c++
using namespace asmjit;

// Signature of the generated function.
typedef int (*SumFunc)(const int* arr, size_t count);

int main(int argc, char* argv[]) {
  assert(sizeof(void*) == 8 &&
    "This example requires 64-bit environment.");

  JitRuntime jit;                         // Create a runtime specialized for JIT.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(jit.codeInfo());              // Initialize it to be compatible with `jit`.

  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.

  // Decide between 32-bit CDECL, WIN64, and SysV64 calling conventions:
  //   32-BIT - passed all arguments by stack.
  //   WIN64  - passes first 4 arguments by RCX, RDX, R8, and R9.
  //   UNIX64 - passes first 6 arguments by RDI, RSI, RCX, RDX, R8, and R9.
  x86::Gp arr, cnt;
  x86::Gp sum = x86::eax;                 // Use EAX as 'sum' as it's a return register.

  if (ASMJIT_ARCH_BITS == 64) {
  #if defined(_WIN32)
    arr = x86::rcx;                       // First argument (array ptr).
    cnt = x86::rdx;                       // Second argument (number of elements)
  #else
    arr = x86::rdi;                       // First argument (array ptr).
    cnt = x86::rsi;                       // Second argument (number of elements)
  #endif
  }
  else {
    arr = x86::edx;                       // Use EDX to hold the array pointer.
    cnt = x86::ecx;                       // Use ECX to hold the counter.
    a.mov(arr, x86::ptr(x86::esp, 4));    // Fetch first argument from [ESP + 4].
    a.mov(cnt, x86::ptr(x86::esp, 8));    // Fetch second argument from [ESP + 8].
  }

  Label Loop = a.newLabel();              // To construct the loop, we need some labels.
  Label Exit = a.newLabel();

  a.xor_(sum, sum);                       // Clear 'sum' register (shorter than 'mov').
  a.test(cnt, cnt);                       // Border case:
  a.jz(Exit);                             //   If 'cnt' is zero jump to 'Exit' now.

  a.bind(Loop);                           // Start of a loop iteration.
  a.add(sum, x86::dword_ptr(arr));        // Add int at [arr] to 'sum'.
  a.add(arr, 4);                          // Increment 'arr' pointer.
  a.dec(cnt);                             // Decrease 'cnt'.
  a.jnz(Loop);                            // If not zero jump to 'Loop'.

  a.bind(Exit);                           // Exit to handle the border case.
  a.ret();                                // Return from function ('sum' == 'eax').
  // ----> x86::Assembler is no longer needed from here and can be destroyed <----

  SumFunc fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.

  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  static const int array[6] = { 4, 8, 15, 16, 23, 42 };

  int result = fn(array, 6);              // Execute the generated code.
  printf("%d\n", result);                 // Print sum of array (108).

  jit.release(fn);                        // Remove the function from the runtime.
  return 0;
}
```

The example should be self-explanatory. It shows how to work with labels, how to use operands, and how to emit instructions that can use different registers based on runtime selection. It implements 32-bit CDECL, WIN64, and SysV64 caling conventions and will work on most X86 environments.

### More About Memory Addresses

X86 provides a complex memory addressing model that allows to encode addresses having a BASE register, INDEX register with a possible scale (left shift), and displacement (called offset in AsmJit). Memory address can also specify memory segment (segment-override in X86 terminology) and some instructions (gather / scatter) require INDEX to be a VECTOR register instead of a general-purpose register. AsmJit allows to encode and work with all forms of addresses mentioned and implemented by X86. It also allows to construct a 64-bit memory address, which is only allowed in one form of 'mov' instruction.

```c++
// Memory operand construction is provided by x86 namespace.
using namespace asmjit;
using namespace asmjit::x86;              // Easier to access x86 regs.

// BASE + OFFSET.
x86::Mem a = ptr(rax);                    // a = [rax]
x86::Mem b = ptr(rax, 15)                 // b = [rax + 15]

// BASE + INDEX << SHIFT - Shift is in BITS as used by X86!
x86::Mem c = ptr(rax, rbx)                // c = [rax + rbx]
x86::Mem d = ptr(rax, rbx, 2)             // d = [rax + rbx << 2]
x86::Mem e = ptr(rax, rbx, 2, 15)         // e = [rax + rbx << 2 + 15]

// BASE + VM (Vector Index) (encoded as MOD+VSIB).
x86::Mem f = ptr(rax, xmm1)               // f = [rax + xmm1]
x86::Mem g = ptr(rax, xmm1, 2)            // g = [rax + xmm1 << 2]
x86::Mem h = ptr(rax, xmm1, 2, 15)        // h = [rax + xmm1 << 2 + 15]

// WITHOUT BASE:
uint64_t ADDR = (uint64_t)0x1234;
x86::Mem i = ptr(ADDR);                   // i = [0x1234]
x86::Mem j = ptr(ADDR, rbx);              // j = [0x1234 + rbx]
x86::Mem k = ptr(ADDR, rbx, 2);           // k = [0x1234 + rbx << 2]

// LABEL - Will be encoded as RIP (64-bit) or absolute address (32-bit).
Label L = ...;
x86::Mem m = ptr(L);                      // m = [L]
x86::Mem n = ptr(L, rbx);                 // n = [L + rbx]
x86::Mem o = ptr(L, rbx, 2);              // o = [L + rbx << 2]
x86::Mem p = ptr(L, rbx, 2, 15);          // p = [L + rbx << 2 + 15]

// RIP - 64-bit only (RIP can't use INDEX).
x86::Mem q = ptr(rip, 24);                // q = [rip + 24]
```

Memory operands can optionally contain memory size. This is required by instructions where the memory size cannot be deduced from other operands, like `inc` and `dec`:

```c++
x86::Mem a = x86::dword_ptr(rax, rbx);       // dword ptr [rax + rbx].
x86::Mem b = x86::qword_ptr(rdx, rsi, 0, 1); // qword ptr [rdx + rsi << 0 + 1].
```

Memory operands provide API that can be used to work with them:

```c++
x86::Mem mem = x86::dword_ptr(rax, 12);   // dword ptr [rax + 12].

mem.hasBase();                            // true.
mem.hasIndex();                           // false.
mem.size();                               // 4.
mem.offset();                             // 12.

mem.setSize(0);                           // Sets the size to 0 (makes it sizeless).
mem.addOffset(-1);                        // Adds -1 to the offset and makes it 11.
mem.setOffset(0);                         // Sets the offset to 0.
mem.setBase(rcx);                         // Changes BASE to RCX.
mem.setIndex(rax);                        // Changes INDEX to RAX.
mem.hasIndex();                           // true.

// ...
```

Making changes to memory operand is very comfortable when emitting loads and stores:

```c++
using namespace asmjit;

x86::Assembler a(...);                    // Your initialized x86::Assembler.
x86::Mem m = x86::ptr(eax);               // Construct [eax] memory operand.

// One way of emitting bunch of loads is to use `mem.adjusted()`. It returns
// a new memory operand and keeps the source operand unchanged.
a.movaps(x86::xmm0, m);                   // No adjustment needed to load [eax].
a.movaps(x86::xmm1, m.adjusted(16));      // Loads from [eax + 16].
a.movaps(x86::xmm2, m.adjusted(32));      // Loads from [eax + 32].
a.movaps(x86::xmm3, m.adjusted(48));      // Loads from [eax + 48].

// ... do something with xmm0-3 ...

// Another way of adjusting memory is to change the operand in-place. If you
// want to keep the original operand you can simply clone it.
x86::Mem mx = m.clone();
a.movaps(mx, x86::xmm0); mx.addOffset(16);// Stores to [eax]      (and adds 16 to mx).
a.movaps(mx, x86::xmm1); mx.addOffset(16);// Stores to [eax + 16] (and adds 16 to mx).
a.movaps(mx, x86::xmm2); mx.addOffset(16);// Stores to [eax + 32] (and adds 16 to mx).
a.movaps(mx, x86::xmm3);                  // Stores to [eax + 48].
```

You can explore the possibilities by taking a look at:

  * [core/operand.h](./src/asmjit/core/operand.h)
  * [x86/x86operand.h](./src/asmjit/x86/x86operand.h).

### More About CodeInfo

In the first complete example the `CodeInfo` is retrieved from `JitRuntime`. It's logical as `JitRuntime` will always return a `CodeInfo` that is compatible with the runtime environment. For example if your application runs in 64-bit mode the `CodeInfo` will use `ArchInfo::kIdX64` architecture in contrast to `ArchInfo::kIdX86`, which will be used in 32-bit mode. AsmJit also allows to setup `CodeInfo` manually, and to select a different architecture when needed. So let's do something else this time, let's always generate a 32-bit code and print it's binary representation. To do that, we create our own `CodeInfo` and initialize it to `ArchInfo::kIdX86` architecture. CodeInfo will populate all basic fields just based on the architecture we provide, so it's super-easy:

```c++
using namespace asmjit;

int main(int argc, char* argv[]) {
  using namespace asmjit::x86;            // Easier access to x86/x64 registers.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(CodeInfo(ArchInfo::kIdX86));// Initialize it for a 32-bit X86 target.

  // Generate a 32-bit function that sums 4 floats and looks like:
  //   void func(float* dst, const float* a, const float* b)
  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.

  a.mov(eax, dword_ptr(esp, 4));          // Load the destination pointer.
  a.mov(ecx, dword_ptr(esp, 8));          // Load the first source pointer.
  a.mov(edx, dword_ptr(esp, 12));         // Load the second source pointer.

  a.movups(xmm0, ptr(ecx));               // Load 4 floats from [ecx] to XMM0.
  a.movups(xmm1, ptr(edx));               // Load 4 floats from [edx] to XMM1.
  a.addps(xmm0, xmm1);                    // Add 4 floats in XMM1 to XMM0.
  a.movups(ptr(eax), xmm0);               // Store the result to [eax].
  a.ret();                                // Return from function.

  // We have no Runtime this time, it's on us what we do with the code.
  // CodeHolder stores code in SectionEntry, which embeds CodeSection
  // and CodeBuffer structures. We are interested in section's CodeBuffer only.
  //
  // NOTE: The first section is always '.text', so it's safe to just use 0 index.
  CodeBuffer& buffer = code.sectionEntry(0)->buffer();

  // Print the machine-code generated or do something more interesting with it?
  //   8B4424048B4C24048B5424040F28010F58010F2900C3
  for (size_t i = 0; i < buffer.length; i++)
    printf("%02X", buffer.data[i]);

  return 0;
}
```

### Explicit Code Relocation

CodeInfo contains much more information than just the target architecture. It can be configured to specify a base-address (or a virtual base-address in a linker terminology), which could be static (useful when you know the location of the target's machine code) or dynamic. AsmJit assumes dynamic base-address by default and relocates the code held by `CodeHolder` to a user-provided address on-demand. To be able to relocate to a user-provided address it needs to store some information about relocations, which is represented by `CodeHolder::RelocEntry`. Relocation entries are only required if you call external functions from the generated code that cannot be encoded by using a 32-bit displacement (X64 architecture doesn't provide 64-bit encodable displacement) and when a label referenced in one section is bound in another, but this is not really a JIT case and it's more related to AOT (ahead-of-time) compilation.

Next example shows how to use a built-in virtual memory allocator `JitAllocator` instead of using `JitRuntime` (just in case you want to use your own memory management) and how to relocate the generated code into your own memory block - you can use your own virtual memory allocator if you prefer that, but that's OS specific and it's already provided by AsmJit, so we will use what AsmJit offers instead of going deep into OS APIs.

The following code is similar to the previous one, but implements a function working in both 32-bit and 64-bit environments:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

int main(int argc, char* argv[]) {
  CodeHolder code;                        // Create a CodeHolder.
  code.init(CodeInfo(ArchInfo::kIdHost));// Initialize it for the host architecture.

  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.

  // Generate a function runnable in both 32-bit and 64-bit architectures:
  bool isX86 = ASMJIT_ARCH_X86 == 32;

  // Signature: 'void func(int* dst, const int* a, const int* b)'.
  x86::Gp dst;
  x86::Gp src_a;
  x86::Gp src_b;

  // Handle the difference between 32-bit and 64-bit calling convention.
  // (arguments passed through stack vs. arguments passed by registers).
  if (isX86) {
    dst   = x86::eax;
    src_a = x86::ecx;
    src_b = x86::edx;
    a.mov(dst  , dword_ptr(x86::esp, 4)); // Load the destination pointer.
    a.mov(src_a, dword_ptr(x86::esp, 8)); // Load the first source pointer.
    a.mov(src_b, dword_ptr(x86::esp, 12));// Load the second source pointer.
  }
  else {
  #if defined(_WIN32)
    dst   = x86::rcx;                     // First argument  (destination pointer).
    src_a = x86::rdx;                     // Second argument (source 'a' pointer).
    src_b = x86::r8;                      // Third argument  (source 'b' pointer).
  #else
    dst   = x86::rdi;                     // First argument  (destination pointer).
    src_a = x86::rsi;                     // Second argument (source 'a' pointer).
    src_b = x86::rdx;                     // Third argument  (source 'b' pointer).
  #endif
  }

  a.movdqu(x86::xmm0, x86::ptr(src_a));   // Load 4 ints from [src_a] to XMM0.
  a.movdqu(x86::xmm1, x86::ptr(src_b));   // Load 4 ints from [src_b] to XMM1.
  a.paddd(x86::xmm0, x86::xmm1);          // Add 4 ints in XMM1 to XMM0.
  a.movdqu(x86::ptr(dst), x86::xmm0);     // Store the result to [dst].
  a.ret();                                // Return from function.

  // After the code was generated it can be relocated manually to any memory
  // location, however, we need to know it's size before we perform memory
  // allocation. CodeHolder's `codeSize()` returns the worst estimated
  // code-size (the biggest possible) in case that relocations are not
  // possible without trampolines (in that case some extra code at the end
  // of the current code buffer is generated during relocation).
  size_t size = code.codeSize();

  // Instead of rolling our own virtual memory allocator we can use the one
  // AsmJit uses. It's decoupled so you don't need to use Runtime for that.
  JitAllocator allocator;

  void* p = allocator.alloc(size);        // Allocate a virtual memory (executable).
  if (!p) return 0;                       // Handle a possible out-of-memory case.

  size_t realSize = code.relocate(p);     // Relocate & store the output in 'p'.

  // Execute the generated function.
  int inA[4] = { 4, 3, 2, 1 };
  int inB[4] = { 1, 5, 2, 8 };
  int out[4];

  // This code uses AsmJit's ptr_as_func<> to cast between void* and SumIntsFunc.
  ptr_as_func<SumIntsFunc>(p)(result, arr_a, arr_b);

  // Prints {5 8 4 9}
  printf("{%d %d %d %d}\n", out[0], out[1], out[2], out[3]);

  // Release 'p' is it's no longer needed. It will be destroyed with 'vm'
  // instance anyway, but it's a good practice to release it explicitly
  // when you know that the function will not be needed anymore.
  allocator.release(p);

  return 0;
}
```

Configure the CodeInfo by calling `CodeInfo::setBaseAddress()` to initialize it to a user-provided base-address before passing it to `CodeHolder`:

```c++
// Configure CodeInfo.
CodeInfo ci(...);
ci.setBaseAddress(uint64_t(0x1234));

// Then initialize CodeHolder with it.
CodeHolder code;
code.init(ci);

// ... after you emit the machine code it will be relocated to the base address
//     provided and stored in the pointer passed to `CodeHolder::relocate()`.
```

TODO: Maybe `CodeHolder::relocate()` is not the best name?

### Using Native Registers - zax, zbx, zcx, ...

AsmJit's X86 code emitters always provide functions to construct machine-size registers depending on the target. This feature is for people that want to write code targeting both 32-bit and 64-bit at the same time. In AsmJit terminology these registers are named **zax**, **zcx**, **zdx**, **zbx**, **zsp**, **zbp**, **zsi**, and **zdi** (they are defined in this exact order by X86). They are accessible through `x86::Assembler`, `x86::Builder`, and `x86::Compiler`. The following example illustrates how to use this feature:

```c++
using namespace asmjit;

typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Create a runtime specialized for JIT.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(jit.codeInfo());              // Initialize it to be compatible with `jit`.

  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.

  // Let's get these registers from x86::Assembler.
  x86::Gp zbp = a.zbp();
  x86::Gp zsp = a.zsp();

  int stackSize = 32;

  // Function prolog.
  a.push(zbp);
  a.mov(zbp, zsp);
  a.sub(zsp, stackSize);

  // ... emit some code (this just sets return value to zero) ...
  a.xor_(x86::eax, x86::eax);

  // Function epilog and return.
  a.mov(zsp, zbp);
  a.pop(zbp);
  a.ret();

  // To make the example complete let's call it.
  Func fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "0".

  jit.release(fn);                        // Remove the function from the runtime.
  return 0;
}
```

The example just returns `0`, but the function generated contains a standard prolog and epilog sequence and the function itself reserves 32 bytes of local stack. The advantage is clear - a single code-base can handle multiple targets easily. If you want to create a register of native size dynamically by specifying its id it's also possible:

```c++
void example(x86::Assembler& a) {
  x86::Gp zax = a.gpz(x86::Gp::kIdAx);
  x86::Gp zbx = a.gpz(x86::Gp::kIdBx);
  x86::Gp zcx = a.gpz(x86::Gp::kIdCx);
  x86::Gp zdx = a.gpz(x86::Gp::kIdDx);

  // You can also change register's id easily.
  x86::Gp zsp = zax;
  zsp.setId(4); // or x86::Gp::kIdSp.
}
```

Cloning existing registers and chaning their IDs is fine in AsmJit; and this technique is used internally in many places.

### Using Assembler as Code-Patcher

This is an advanced topic that is sometimes unavoidable. AsmJit by default appends machine-code it generates into a `CodeBuffer`, however, it also allows to set the offset in `CodeBuffer` explicitly and to overwrite its content. This technique is extremely dangerous for asm beginners as X86 instructions have variable length (see below), so you should in general only patch code to change instruction's offset or some basic other details you didn't know about the first time you emitted it. A typical scenario that requires code-patching is when you start emitting function and you don't know how much stack you want to reserve for it.

Before we go further it's important to introduce instruction options, because they can help with code-patching (and not only patching, but that will be explained in AVX-512 section):

  * Many general-purpose instructions (especially arithmetic ones) on X86 have multiple encodings - in AsmJit this is usually called 'short form' and 'long form'.
  * AsmJit always tries to use 'short form' as it makes the resulting machine-code smaller, which is always good - this decision is used by majority of assemblers out there.
  * AsmJit allows to override the default decision by using `short_()` and `long_()` instruction options to force short or long form, respectively. The most useful is `long_()` as it basically forces AsmJit to always emit the long form. The `short_()` is not that useful as it's automatic (except jumps to non-bound labels). Note the underscore after each function name as it avoids collision with built-in C++ types.

To illustrate what short form and long form means in binary let's assume we want to emit `add esp, 16` instruction, which has two possible binary encodings:

  * `83C410` - This is a short form aka `short add esp, 16` - You can see opcode byte (0x8C), MOD/RM byte (0xC4) and an 8-bit immediate value representing `16`.
  * `81C410000000` - This is a long form aka `long add esp, 16` - You can see a different opcode byte (0x81), the same Mod/RM byte (0xC4) and a 32-bit immediate in little-endian representing `16`.

If you generate an instruction in a short form and then patch it in a long form or vice-versa then something really bad will happen when you try to execute such code. The following example illustrates how to patch the code properly (it just extends the previous example):

```c++
using namespace asmjit;

typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Create a runtime specialized for JIT.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(jit.codeInfo());              // Initialize it to be compatible with `jit`.

  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.

  // Let's get these registers from x86::Assembler.
  x86::Gp zbp = a.zbp();
  x86::Gp zsp = a.zsp();

  // Function prolog.
  a.push(zbp);
  a.mov(zbp, zsp);

  // This is where we are gonna patch the code later, so let's get the offset
  // (the current location) from the beginning of the code-buffer.
  size_t patchOffset = a.offset();
  // Let's just emit 'sub zsp, 0' for now, but don't forget to use LONG form.
  a.long_().sub(zsp, 0);

  // ... emit some code (this just sets return value to zero) ...
  a.xor_(x86::eax, x86::eax);

  // Function epilog and return.
  a.mov(zsp, zbp);
  a.pop(zbp);
  a.ret();

  // Now we know how much stack size we want to reserve. I have chosen 128
  // bytes on purpose as it's encodable only in long form that we have used.

  int stackSize = 128;                    // Number of bytes to reserve on the stack.
  a.setOffset(patchOffset);               // Move the current cursor to `patchOffset`.
  a.long_().sub(zsp, stackSize);          // Patch the code; don't forget to use LONG form.

  // Now the code is ready to be called
  Func fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "0".

  jit.release(fn);                        // Remove the function from the runtime.
  return 0;
}
```

If you run the example it would just work. As an experiment you can try removing `long_()` form to see what happens when wrong code is generated.

### Code Patching and REX Prefix

In 64-bit mode there is one more thing to worry about when patching code - REX prefix. It's a single byte prefix designed to address registers with ids from 9 to 15 and to override the default width of operation from 32 to 64 bits. AsmJit, like other assemblers, only emits REX prefix when it's necessary. If the patched code only changes the immediate value as shown in the previous example then there is nothing to worry about as it doesn't change the logic behind emitting REX prefix, however, if the patched code changes register id or overrides the operation width then it's important to take care of REX prefix as well.

AsmJit contains another instruction option that controls (forces) REX prefix - `rex()`. If you use it the instruction emitted will always use REX prefix even when it's encodable without it. The following list contains some instructions and their binary representations to illustrate when it's emitted:

  * `__83C410` - `add esp, 16`     - 32-bit operation in 64-bit mode doesn't require REX prefix.
  * `4083C410` - `rex add esp, 16` - 32-bit operation in 64-bit mode with forced REX prefix (0x40).
  * `4883C410` - `add rsp, 16`     - 64-bit operation in 64-bit mode requires REX prefix (0x48).
  * `4183C410` - `add r12d, 16`    - 32-bit operation in 64-bit mode using R12D requires REX prefix (0x41).
  * `4983C410` - `add r12, 16`     - 64-bit operation in 64-bit mode using R12  requires REX prefix (0x49).

### Generic Function API

So far all examples shown above handled creating function prologs and epilogs manually. While it's possible to do it that way it's much better to automate such process as function calling conventions vary across architectures and also across operating systems.

AsmJit contains a functionality that can be used to define function signatures and to calculate automatically optimal function frame that can be used directly by a prolog and epilog inserter. This feature was exclusive to AsmJit's BaseCompiler for a very long time, but was abstracted out and is now available for all users regardless of BaseEmitter they use. The design of handling functions prologs and epilogs allows generally two use cases:

  * Calculate function frame before the function is generated - this is the only way if you use pure `Assembler` emitter and shown in the next example.
  * Calculate function frame after the function is generated - this way is generally used by `BaseBuilder` and `BaseCompiler` (will be described together with `x86::Compiler`).

The following concepts are used to describe and create functions in AsmJit:

  * `Type` - Type is an 8-bit value that describes a platform independent type as we know it from C/C++. It provides abstractions for most common types like `int8_t`, `uint32_t`, `uintptr_t`, `float`, `double`, and all possible vector types to match ISAs up to AVX512. `Type::Id` was introduced originally to be used with `BaseCompiler`, but is now used by `FuncSignature` as well.

  * `CallConv` - Describes a calling convention - this class contains instructions to assign registers and stack addresses to function arguments and return value(s), but doesn't specify any function signature. Calling conventions are architecture and OS dependent.

  * `FuncSignature` - Describes a function signature, for example `int func(int, int)`. `FuncSignature` contains a function calling convention id, return value type, and function arguments. The signature itself is platform independent and uses `Type::Id` to describe types of function arguments and its return value(s).

  * `FuncDetail` - Architecture and ABI dependent information that describes `CallConv` and expanded `FuncSignature`. Each function argument and return value is represented as `FuncValue` that contains the original `Type::Id` enriched by additional information that specifies if the value is passed/returned by register (and which register) or by stack. Each value also contains some other metadata that provide additional information required to handle it properly (for example if a vector value is passed indirectly by a pointer as required by WIN64 calling convention, etc...).

  * `FuncFrame` - Contains information about the function frame that can be used by prolog/epilog inserter (PEI). Holds call stack size size and alignment, local stack size and alignment, and various attributes that describe how prolog and epilog should be constructed. `FuncFrame` doesn't know anything about function's arguments or return values, it hold only information necessary to create a valid and ABI conforming function prologs and epilogs.

  * `FuncArgsAssignment` - A helper class that can be used to reassign function arguments into user specified registers. It's architecture and ABI dependent mapping from function arguments described by CallConv and FuncDetail into registers specified by the user.

It's a lot of concepts where each represents one step in the function frame calculation. In addition, the whole machinery can also be used to create function calls, instead of function prologs and epilogs. The next example shows how AsmJit can be used to create functions for both 32-bit and 64-bit targets and various calling conventions:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Create JIT Runtime.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(jit.codeInfo());              // Initialize it to match `jit`.
  x86::Assembler a(&code);                // Create and attach x86::Assembler to `code`.

  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  x86::Gp dst   = a.zax();
  x86::Gp src_a = a.zcx();
  x86::Gp src_b = a.zdx();

  X86::Xmm vec0 = x86::xmm0;
  X86::Xmm vec1 = x86::xmm1;

  // Create and initialize `FuncDetail` and `FuncFrame`.
  FuncDetail func;
  func.init(FuncSignature3<void, int*, const int*, const int*>(CallConv::kIdHost));

  FuncFrame frame;
  frame.init(func);

  // Make XMM0 and XMM1 dirty; `kGroupVec` describes XMM|YMM|ZMM registers.
  frame.setDirtyRegs(x86::Reg::kGroupVec, IntUtils::mask(0, 1));

  FuncArgsAssignment args(&func);         // Create arguments assignment context.
  args.assignAll(dst, src_a, src_b);      // Assign our registers to arguments.
  args.updateFrameInfo(frame);            // Reflect our args in FuncFrame.
  frame.finalize();                       // Finalize the FuncFrame (updates it).

  a.emitProlog(frame);                    // Emit function prolog.
  a.emitArgsAssignment(frame, args);      // Assign arguments to registers.
  a.movdqu(vec0, x86::ptr(src_a));        // Load 4 ints from [src_a] to XMM0.
  a.movdqu(vec1, x86::ptr(src_b));        // Load 4 ints from [src_b] to XMM1.
  a.paddd(vec0, vec1);                    // Add 4 ints in XMM1 to XMM0.
  a.movdqu(x86::ptr(dst), vec0);          // Store the result to [dst].
  a.emitEpilog(frame);                    // Emit function epilog and return.

  SumIntsFunc fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error case.

  // Execute the generated function.
  int inA[4] = { 4, 3, 2, 1 };
  int inB[4] = { 1, 5, 2, 8 };
  int out[4];
  fn(out, inA, inB);

  // Prints {5 8 4 9}
  printf("{%d %d %d %d}\n", out[0], out[1], out[2], out[3]);

  jit.release(fn);                        // Remove the function from the runtime.
  return 0;
}
```

BaseBuilder
-----------

Both `[Base]Builder` and `[Base]Compiler` are emitters that emit everything to a representation that allows further processing. The code stored in such representation is completely safe to be patched, simplified, reordered, obfuscated, removed, injected, analyzed, and 'think-of-anything-else'. Each instruction, label, directive, etc... is stored in `BaseNode` (or derived class like `InstNode` or `LabelNode`) and contains all the information relevant to it.

There is a difference between `[Base]Builder` and `[Base]Compiler`:

  * `BaseBuilder` (low-level):
    * Maximum compatibility with `Assembler`, easy to switch from `Assembler` to `BaseBuilder` and vice versa.
    * Doesn't generate machine code directly, allows to serialize to `Assembler` when the whole code is ready to be encoded.

  * `BaseCompiler` (high-level):
    * Virtual registers - allows to use unlimited number of virtual registers which are allocated into physical registers by a built-in register allocator.
    * Function nodes - allows to create functions by specifying their signatures and assigning virtual registers to function arguments and return value(s).
    * Function calls - allows to call other functions within the generated code by using the same interface that is used to create functions.

There are multiple node types used by both `BaseBuilder` and `BaseCompiler`:

  * Basic nodes:
    * `BaseNode` - Base class for all nodes.
    * `InstNode` - Instruction node.
    * `AlignNode` - Alignment directive (.align).
    * `LabelNode` - Label (location where to bound it).

  * Data nodes:
    * `DataNode` - Data embedded into the code.
    * `ConstPoolNode` - Constant pool data.
    * `LabelDataNode` - Label address embedded as data.

  * Informative nodes:
    * `CommentNode` - Contains a comment string, doesn't affect code generation.
    * `SentinelNode` - A marker that can be used to remember certain position, doesn't affect code generation.

  * `BaseCompiler` nodes:
    * `FuncNode` - Start of a function.
    * `FuncRetNode` - Return from a function.
    * `FuncCallNode` - Function call.

### Using BaseBuilder

`BaseBuilder` was designed to be used as an `Assembler` replacement in case that post-processing of the generated code is required. The code can be modified during or after code generation. The post processing can be done manually or through `Pass` (Code-Builder Pass) object. `BaseBuilder` stores the emitted code as a double-linked list, which allows O(1) insertion and removal.

The code representation used by `BaseBuilder` is compatible with everything AsmJit provides. Each instruction is stored as `InstNode`, which contains instruction id, options, and operands. Each instruction emitted will create a new `InstNode` instance and add it to the current cursor in the double-linked list of nodes. Since the instruction stream used by `BaseBuilder` can be manipulated, we can rewrite the **SumInts** example into the following:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

// Small helper function to print the current content of `cb`.
static void dumpCode(BaseBuilder& cb, const char* phase) {
  StringBuilder sb;
  cb.dump(sb);
  printf("%s:\n%s\n", phase, sb.data());
}

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Create JIT Runtime.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(jit.codeInfo());              // Initialize it to match `jit`.
  x86::Builder cb(&code);                 // Create and attach x86::Builder to `code`.

  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  x86::Gp dst   = cb.zax();
  x86::Gp src_a = cb.zcx();
  x86::Gp src_b = cb.zdx();

  X86::Xmm vec0 = x86::xmm0;
  X86::Xmm vec1 = x86::xmm1;

  // Create and initialize `FuncDetail`.
  FuncDetail func;
  func.init(FuncSignature3<void, int*, const int*, const int*>(CallConv::kIdHost));

  // Remember prolog insertion point.
  BaseNode* prologInsertionPoint = cb.cursor();

  // Emit function body:
  cb.movdqu(vec0, x86::ptr(src_a));       // Load 4 ints from [src_a] to XMM0.
  cb.movdqu(vec1, x86::ptr(src_b));       // Load 4 ints from [src_b] to XMM1.
  cb.paddd(vec0, vec1);                   // Add 4 ints in XMM1 to XMM0.
  cb.movdqu(x86::ptr(dst), vec0);         // Store the result to [dst].

  // Remember epilog insertion point.
  BaseNode* epilogInsertionPoint = cb.cursor();

  // Let's see what we have now.
  dumpCode(cb, "Raw Function");

  // Now, after we emitted the function body, we can insert the prolog, arguments
  // allocation, and epilog. This is not possible with using pure x86::Assembler.
  FuncFrame frame;
  frame.init(func);

  // Make XMM0 and XMM1 dirty; `kGroupVec` describes XMM|YMM|ZMM registers.
  frame.setDirtyRegs(x86::Reg::kGroupVec, IntUtils::mask(0, 1));

  FuncArgsAssignment args(&func);         // Create arguments assignment context.
  args.assignAll(dst, src_a, src_b);      // Assign our registers to arguments.
  args.updateFrame(frame);                // Reflect our args in FuncFrame.
  frame.finalize();                       // Finalize the FuncFrame (updates it).

  // Insert function prolog and allocate arguments to registers.
  cb.setCursor(prologInsertionPoint);
  cb.emitProlog(frame);
  cb.emitArgsAssignment(frame, args);

  // Insert function epilog.
  cb.setCursor(epilogInsertionPoint);
  cb.emitEpilog(frame);

  // Let's see how the function's prolog and epilog looks.
  dumpCode(cb, "Prolog & Epilog");

  // IMPORTANT: BaseBuilder requires `finalize()` to be called to serialize
  // the code to the Assembler (it automatically creates one if not attached).
  cb.finalize();

  SumIntsFunc fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error case.

  // Execute the generated function.
  int inA[4] = { 4, 3, 2, 1 };
  int inB[4] = { 1, 5, 2, 8 };
  int out[4];
  fn(out, inA, inB);

  // Prints {5 8 4 9}
  printf("{%d %d %d %d}\n", out[0], out[1], out[2], out[3]);

  jit.release(fn);                        // Remove the function from the runtime.
  return 0;
}
```

When the example is executed it should output the following (this one using AMD64-SystemV ABI):

```
Raw Function:
movdqu xmm0, [rcx]
movdqu xmm1, [rdx]
paddd xmm0, xmm1
movdqu [rax], xmm0

Prolog & Epilog:
mov rax, rdi
mov rcx, rsi
movdqu xmm0, [rcx]
movdqu xmm1, [rdx]
paddd xmm0, xmm1
movdqu [rax], xmm0
ret

{5 8 4 9}
```

The number of use-cases of **x86::Builder** is not limited and highly depends on your creativity and experience. The previous example can be easily improved to collect all dirty registers inside the function programmatically and to pass them to `frame.setDirtyRegs()`:

```c++
using namespace asmjit;

// NOTE: This function doesn't cover all possible constructs. It ignores
// instructions that write to implicit registers that are not part of the
// operand list. It also counts read-only registers. Real implementation
// would be a bit more complicated, but still relatively easy to implement.
static void collectDirtyRegs(const BaseNode* first, const BaseNode* last, uint32_t regMask[BaseReg::kGroupVirt]) {
  const BaseNode* node = first;
  while (node) {
    if (node->actsAsInst()) {
      const InstNode* inst = node->as<InstNode>();
      const Operand* opArray = inst->operands();

      for (uint32_t i = 0, opCount = inst->opCount(); i < opCount; i++) {
        const Operand& op = opArray[i];
        if (op.isReg()) {
          const x86::Reg& reg = op.as<x86::Reg>();
          if (reg.group() < BaseReg::kGroupVirt)
            regMask[reg.group()] |= 1u << reg.id();
        }
      }
    }

    if (node == last) break;
    node = node->next();
  n}

static void setDirtyRegsOfFuncFrame(const x86::Builder& cb, FuncFrame& frame) {
  uint32_t regMask[BaseReg::kGroupVirt] = { 0 };
  collectDirtyRegs(cb.firstNode(), cb.lastNode(), regMask);

  // X86/X64 ABIs only require to save GP/XMM registers:
  frame.setDirtyRegs(x86::Reg::kGroupGp , regMask[x86::Reg::kGroupGp ]);
  frame.setDirtyRegs(x86::Reg::kGroupVec, regMask[x86::Reg::kGroupVec]);
}
```

### Using x86::Assembler or x86::Builder through X86::Emitter

Even when **Assembler** and **BaseBuilder** implement the same interface defined by **BaseEmitter** their platform dependent variants (**x86::Assembler** and **x86::Builder**, respective) cannot be interchanged or casted to each other by using C++'s `static_cast<>`. The main reason is the inheritance graph of these classes is different and cast-incompatible, as illustrated in the following graph:

```
                                            +--------------+      +=========================+
                   +----------------------->| x86::Emitter |<--+--# x86::EmitterImplicitT<> #<--+
                   |                        +--------------+   |  +=========================+   |
                   |                           (abstract)      |           (mixin)              |
                   |   +--------------+     +~~~~~~~~~~~~~~+   |                                |
                   +-->| BaseAssembler|---->|x86::Assembler|<--+                                |
                   |   +--------------+     +~~~~~~~~~~~~~~+   |                                |
                   |      (abstract)            (final)        |                                |
+===============+  |   +--------------+     +~~~~~~~~~~~~~~+   |                                |
#  BaseEmitter  #--+-->|  BaseBuilder |--+->| x86::Builder |<--+                                |
+===============+      +--------------+  |  +~~~~~~~~~~~~~~+                                    |
   (abstract)             (abstract)     |      (final)                                         |
                   +---------------------+                                                      |
                   |                                                                            |
                   |   +--------------+     +~~~~~~~~~~~~~~+      +=========================+   |
                   +-->| BaseCompiler |---->| x86::Compiler|<-----# x86::EmitterExplicitT<> #---+
                       +--------------+     +~~~~~~~~~~~~~~+      +=========================+
                          (abstract)            (final)                   (mixin)
```

The graph basically shows that it's not possible to cast `x86::Assembler` to `x86::Builder` and vice versa. However, since both `x86::Assembler` and `x86::Builder` share the same interface defined by both `BaseEmitter` and `x86::EmmiterImplicitT` a class called `x86::Emitter` was introduced to make it possible to write a function that can emit to both `x86::Assembler` and `x86::Builder`. Note that `x86::Emitter` cannot be created, it's abstract and has private constructors and destructors; it was only designed to be casted to and used as an interface.

Each X86 emitter implements a member function called `as<x86::Emitter>()`, which casts the instance to the `x86::Emitter`, as illustrated on the next example:

```c++
using namespace asmjit;

static void emitSomething(x86::Emitter* e) {
  e->mov(x86::eax, x86::ebx);
}

static void assemble(CodeHolder& code, bool useAsm) {
  if (useAsm) {
    x86::Assembler a(&code);
    emitSomething(a.as<x86::Emitter>());
  }
  else {
    x86::Builder cb(&code);
    emitSomething(cb.as<x86::Emitter>());

    // IMPORTANT: BaseBuilder requires `finalize()` to be called to serialize
    // the code to the Assembler (it automatically creates one if not attached).
    cb.finalize();
  }
}
```

The example above shows how to create a function that can emit code to either **x86::Assembler** or **x86::Builder** through **x86::Emitter**, which provides emitter-neutral functionality. **x86::Emitter**, however, doesn't provide any emitter **x86::Assembler** or **x86::Builder** specific functionality like **setCursor()**.

BaseCompiler
------------

**BaseCompiler** is a high-level code emitter that provides virtual registers and automatically handles function calling conventions. It's still architecture dependent, but makes the code generation much easier by offering a built-in register allocator and function builder. Functions are essential; the first-step to generate some code is to define the signature of the function you want to generate (before generating the function body). Function arguments and return value(s) are handled by assigning virtual registers to them. Similarly, function calls are handled the same way.

**BaseCompiler** also makes the use of passes (introduced by **BaseBuilder**) and automatically adds an architecture-dependent register allocator pass to the list of passes when attached to **CodeHolder**.

### Compiler Basics

The first **BaseCompiler** example shows how to generate a function that simply returns an integer value. It's an analogy to the very first example:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(jit.codeInfo());              // Initialize to the same arch as JIT runtime.

  x86::Compiler cc(&code);                // Create and attach x86::Compiler to `code`.
  cc.addFunc(FuncSignature0<int>());      // Begin a function of `int fn(void)` signature.

  x86::Gp vReg = cc.newGpd();             // Create a 32-bit general purpose register.
  cc.mov(vReg, 1);                        // Move one to our virtual register `vReg`.
  cc.ret(vReg);                           // Return `vReg` from the function.

  cc.endFunc();                           // End of the function body.
  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> x86::Compiler is no longer needed from here and can be destroyed <----

  Func fn;
  Error err = jit.add(&fn, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "1".

  jit.release(fn);                        // RAII, but let's make it explicit.
  return 0;
}
```

The **addFunc()** and **endFunc()** methods define the body of the function. Both functions must be called per function, but the body doesn't have to be generated in sequence. An example of generating two functions will be shown later. The next example shows more complicated code that contain a loop and generates a **memcpy32()** function:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef void (*MemCpy32)(uint32_t* dst, const uint32_t* src, size_t count);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(jit.codeInfo());              // Initialize to the same arch as JIT runtime.

  x86::Compiler cc(&code);                // Create and attach x86::Compiler to `code`.
  cc.addFunc(                             // Begin the function of the following signature:
    FuncSignature3<void,                  //   Return value - void      (no return value).
      uint32_t*,                          //   1st argument - uint32_t* (machine reg-size).
      const uint32_t*,                    //   2nd argument - uint32_t* (machine reg-size).
      size_t>());                         //   3rd argument - size_t    (machine reg-size).

  Label L_Loop = cc.newLabel();           // Start of the loop.
  Label L_Exit = cc.newLabel();           // Used to exit early.

  x86::Gp dst = cc.newIntPtr("dst");      // Create `dst` register (destination pointer).
  x86::Gp src = cc.newIntPtr("src");      // Create `src` register (source pointer).
  x86::Gp cnt = cc.newUIntPtr("cnt");     // Create `cnt` register (loop counter).

  cc.setArg(0, dst);                      // Assign `dst` argument.
  cc.setArg(1, src);                      // Assign `src` argument.
  cc.setArg(2, cnt);                      // Assign `cnt` argument.

  cc.test(cnt, cnt);                      // Early exit if length is zero.
  cc.jz(L_Exit);

  cc.bind(L_Loop);                        // Bind the beginning of the loop here.

  x86::Gp tmp = cc.newInt32("tmp");       // Copy a single dword (4 bytes).
  cc.mov(tmp, x86::dword_ptr(src));       // Load DWORD from [src] address.
  cc.mov(x86::dword_ptr(dst), tmp);       // Store DWORD to [dst] address.

  cc.add(src, 4);                         // Increment `src`.
  cc.add(dst, 4);                         // Increment `dst`.

  cc.dec(cnt);                            // Loop until `cnt` is non-zero.
  cc.jnz(L_Loop);

  cc.bind(L_Exit);                        // Label used by early exit.
  cc.endFunc();                           // End of the function body.

  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> x86::Compiler is no longer needed from here and can be destroyed <----

  MemCpy32 memcpy32;
  Error err = jit.add(&memcpy32, &code);  // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  // Test the generated code.
  uint32_t src[6] = { 1, 2, 3, 5, 8, 13 };
  uint32_t dst[6];
  memcpy32(dst, src, 6);

  for (uint32_t i = 0; i < 6; i++)
    printf("%d\n", dst[i]);

  jit.release(memcpy32);                  // RAII, but let's make it explicit.
  return 0;
}
```

### Recursive Functions

It's possible to create more functions by using the same `x86::Compiler` instance and make links between them. In such case it's important to keep the pointer to the `FuncNode` node. The first example creates a simple Fibonacci function that calls itself recursively:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef uint32_t (*Fibonacci)(uint32_t x);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(jit.codeInfo());              // Initialize to the same arch as JIT runtime.

  x86::Compiler cc(&code);                // Create and attach x86::Compiler to `code`.
  FuncNode* func = cc.addFunc(            // Begin of the Fibonacci function, `addFunc()`
    FuncSignature1<int, int>());          // Returns a pointer to the `FuncNode` node.

  Label L_Exit = cc.newLabel()            // Exit label.
  x86::Gp x = cc.newU32();                // Function `x` argument.
  x86::Gp y = cc.newU32();                // Temporary.

  cc.setArg(0, x);

  cc.cmp(x, 3);                           // Return `x` if less than 3.
  cc.jb(L_Exit);

  cc.mov(y, x);                           // Make copy of the original `x`.
  cc.dec(x);                              // Decrease `x`.

  FuncCallNode* call = cc.call(           // Function call:
    func->label(),                        //   Function address or Label.
    FuncSignature1<int, int>());          //   Function signature.

  call->setArg(0, x);                     // Assign `x` as the first argument and
  call->setRet(0, x);                     // assign `x` as a return value as well.

  cc.add(x, y);                           // Combine the return value with `y`.

  cc.bind(L_Exit);
  cc.ret(x);                              // Return `x`.
  cc.endFunc();                           // End of the function body.

  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> x86::Compiler is no longer needed from here and can be destroyed <----

  Fibonacci fib;
  Error err = jit.add(&fib, &code);       // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  printf("Fib(%u) -> %u\n, 8, fib(8));    // Test the generated code.

  jit.release(fib);                       // RAII, but let's make it explicit.
  return 0;
}
```

### Stack Management

**BaseCompiler** manages function's stack-frame, which is used by the register allocator to spill virtual registers. It also provides an interface to allocate user-defined block of the stack, which can be used as a temporary storage by the generated function. In the following example a stack of 256 bytes size is allocated, filled by bytes starting from 0 to 255 and then iterated again to sum all the values.

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(jit.codeInfo());              // Initialize to the same arch as JIT runtime.

  x86::Compiler cc(&code);                // Create and attach x86::Compiler to `code`.
  cc.addFunc(FuncSignature0<int>());      // Create a function that returns 'int'.

  x86::Gp p = cc.newIntPtr("p");
  x86::Gp i = cc.newIntPtr("i");

  x86::Mem stack = cc.newStack(256, 4);   // Allocate 256 bytes on the stack aligned to 4 bytes.
  x86::Mem stackIdx(stack);               // Copy of `stack` with `i` added.
  stackIdx.setIndex(i);                   // stackIdx <- stack[i].
  stackIdx.setSize(1);                    // stackIdx <- byte ptr stack[i].

  // Load a stack address to `p`. This step is purely optional and shows
  // that `lea` is useful to load a memory operands address (even absolute)
  // to a general purpose register.
  cc.lea(p, stack);

  // Clear `i` (`xor` as it's C++ keyword, hence `xor_` is used instead).
  cc.xor_(i, i);

  Label L1 = cc.newLabel();
  Label L2 = cc.newLabel();

  cc.bind(L1);                            // First loop, fill the stack.
  cc.mov(stackIdx, i.r8());               // stack[i] = uint8_t(i).

  cc.inc(i);                              // i++;
  cc.cmp(i, 256);                         // if (i < 256)
  cc.jb(L1);                              //   goto L1;

  // Second loop, sum all bytes stored in `stack`.
  x86::Gp sum = cc.newI32("sum");
  x86::Gp val = cc.newI32("val");

  cc.xor_(i, i);
  cc.xor_(sum, sum);

  cc.bind(L2);

  cc.movzx(val, stackIdx);                // val = uint32_t(stack[i]);
  cc.add(sum, val);                       // sum += val;

  cc.inc(i);                              // i++;
  cc.cmp(i, 256);                         // if (i < 256)
  cc.jb(L2);                              //   goto L2;

  cc.ret(sum);                            // Return the `sum` of all values.
  cc.endFunc();                           // End of the function body.

  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> x86::Compiler is no longer needed from here and can be destroyed <----

  Func func;
  Error err = jit.add(&func, &code);      // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  printf("Func() -> %d\n", func());       // Test the generated code.

  jit.release(func);                      // RAII, but let's make it explicit.
  return 0;
}
```

### Constant Pool

**BaseCompiler** provides two constant pools for a general purpose code generation - local and global. Local constant pool is related to a single **FuncNode** node and is generally flushed after the function body, and global constant pool is flushed at the end of the generated code by **BaseCompiler::finalize()**.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

static void exampleUseOfConstPool(x86::Compiler& cc) {
  cc.addFunc(FuncSignature0<int>());

  x86::Gp v0 = cc.newGpd("v0");
  x86::Gp v1 = cc.newGpd("v1");

  x86::Mem c0 = cc.newInt32Const(ConstPool::kScopeLocal, 200);
  x86::Mem c1 = cc.newInt32Const(ConstPool::kScopeLocal, 33);

  cc.mov(v0, c0);
  cc.mov(v1, c1);
  cc.add(v0, v1);

  cc.ret(v0);
  cc.endFunc();
}
```

Advanced Features
-----------------

### Logging

The initial phase of any project that generates machine code is not always smooth. Failure cases are common especially at the beginning of the project and AsmJit provides a logging functionality to address this issue. AsmJit does already a good job with function overloading to prevent from emitting semantically incorrect instructions, but it can't prevent from emitting machine code that is semantically correct, but doesn't work when it's executed. Logging has always been an important part of AsmJit's infrastructure and looking at logs can sometimes reveal code generation issues quickly.

AsmJit provides API for logging and formatting:
  * `Logger` - A logger that you can pass to `CodeHolder` and all emitters that inherit `BaseEmitter`.
  * `FormatOptions` - Formatting options that can change how instructions and operands are formatted.

AsmJit's `Logger` serves the following purposes:
  * Provides a basic foundation for logging.
  * Abstract class leaving the implementation (destination) on users. Two backends are built-in for simplicity:
    * `FileLogger` implements logging into a standard `std::FILE` stream.
    * `StringLogger` stores the logged text in `StringBuilder` instance.

AsmJit's `FormatOptions` provides the following to customize the formatting of instructions and operands:
  * Flags:
    * `FormatOptions::kFlagMachineCode`    - Show a machine code of each encoded instruction.
    * `FormatOptions::kFlagExplainConsts`  - Show a text explanation of some immediate values that are used as predicates.
    * `FormatOptions::kFlagHexImms`        - Use hexadecimal notation to output immediates.
    * `FormatOptions::kFlagHexOffsets`     - Use hexadecimal notation to output offsets.
    * `FormatOptions::kFlagRegCasts`       - Show casts between various register types (compiler).
    * `FormatOptions::kFlagPositions`      - Show positions associated with nodes (compiler).
  * Indentation:
    * `FormatOptions::kIndentationCode`    - Indentation of instructions and directives.
    * `FormatOptions::kIndentationLabel`   - Indentation of labels.
    * `FormatOptions::kIndentationComment` - Indentation of whole-line comments.

**Logger** is typically attached to **CodeHolder** and all attached code emitters automatically use it:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime jit;                         // Runtime specialized for JIT code execution.
  FileLogger logger(stdout);              // Logger should always survive the CodeHolder.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(jit.codeInfo());              // Initialize to the same arch as JIT runtime.
  code.setLogger(&logger);                // Attach the `logger` to `code` holder.

  // ... code as usual, everything you emit will be logged to `stdout` ...

  return 0;
}
```

### Error Handling

AsmJit uses error codes to represent and return errors. Every function where error can occur returns **Error**. Exceptions are never thrown by AsmJit even in extreme conditions like out-of-memory. Errors should never be ignored, however, checking errors after each asmjit API call would simply overcomplicate the whole code generation experience. To make life simpler AsmJit provides **ErrorHandler**, which provides **handleError()** function:

    `virtual bool handleError(Error err, const char* message, BaseEmitter* origin) = 0;`

That can be overridden by AsmJit users and do the following:

  * 1. Record the error and continue (the way how the error is user-implemented).
  * 2. Throw an exception. AsmJit doesn't use exceptions and is completely exception-safe, but it's perfectly legal to throw an exception from the error handler.
  * 3. Use plain old C's `setjmp()` and `longjmp()`. Asmjit always puts `Assembler` and `Compiler` to a consistent state before calling the `handleError()` so `longjmp()` can be used without issues to cancel the code-generation if an error occurred. This method can be used if exception handling in your project is turned off and you still want some comfort. In most cases it should be safe as AsmJit uses Zone memory and the ownership of memory it allocates always ends with the instance that allocated it. If using this approach please never jump outside the life-time of **CodeHolder** and **BaseEmitter**.

**ErrorHandler** can be attached to **CodeHolder** and/or **BaseEmitter** (which has a priority). The first example uses error handler that just prints the error, but lets AsmJit continue:

```c++
// Error handling #1:
#include <asmjit/asmjit.h>

#include <stdio.h>

// Error handler that just prints the error and lets AsmJit ignore it.
class SimpleErrorHandler : public asmjit::ErrorHandler {
public:
  inline SimpleErrorHandler() : lastError(kErrorOk) {}

  void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
    this->err = err;
    fprintf(stderr, "ERROR: %s\n", message);
  }

  Error err;
};

int main(int argc, char* argv[]) {
  using namespace asmjit;

  JitRuntime jit;
  SimpleErrorHandler eh;

  CodeHolder code;
  code.init(jit.codeInfo());
  code.setErrorHandler(&eh);

  // Try to emit instruction that doesn't exist.
  x86::Assembler a(&code);
  a.emit(x86::Inst::kIdMov, x86::xmm0, x86::xmm1);

  if (eh.err) {
    // Assembler failed!
  }

  return 0;
}
```

If error happens during instruction emitting / encoding the assembler behaves transactionally - the output buffer won't advance if encoding failed, thus either a fully encoded instruction or nothing is emitted. The error handling shown above is useful, but it's still not the best way of dealing with errors in AsmJit. The following example shows how to use exception handling to handle errors in a more C++ way:

```c++
// Error handling #2:
#include <asmjit/asmjit.h>

#include <exception>
#include <string>
#include <stdio.h>

// Error handler that throws a user-defined `AsmJitException`.
class AsmJitException : public std::exception {
public:
  AsmJitException(asmjit::Error err, const char* message) noexcept
    : err(err),
      message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

  asmjit::Error err;
  std::string message;
};

class ThrowableErrorHandler : public asmjit::ErrorHandler {
public:
  // Throw is possible, functions that use ErrorHandler are never 'noexcept'.
  void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
    throw AsmJitException(err, message);
  }
};

int main(int argc, char* argv[]) {
  using namespace asmjit;

  JitRuntime jit;
  ThrowableErrorHandler eh;

  CodeHolder code;
  code.init(jit.codeInfo());
  code.setErrorHandler(&eh);

  x86::Assembler a(&code);

  // Try to emit instruction that doesn't exist.
  try {
    a.emit(x86::Inst::kIdMov, x86::xmm0, x86::xmm1);
  }
  catch (const AsmJitException& ex) {
    printf("EXCEPTION THROWN: %s\n", ex.what());
  }

  return 0;
}
```

If C++ exceptions are not what you like or your project turns off them completely there is still a way of reducing the error handling to a minimum by using a standard `setjmp/longjmp` approach. AsmJit is exception-safe and cleans up everything before calling the **ErrorHandler**, so any approach is safe. You can simply jump from the error handler without causing any side-effects or memory leaks. The following example demonstrates how it could be done:

```c++
// Error handling #3:
#include <asmjit/asmjit.h>

#include <setjmp.h>
#include <stdio.h>

class LongJmpErrorHandler : public asmjit::ErrorHandler {
public:
  inline LongJmpErrorHandler() : err(asmjit::kErrorOk) {}

  void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
    this->err = err;
    longjmp(state, 1);
  }

  jmp_buf state;
  asmjit::Error err;
};

int main(int argc, char* argv[]) {
  using namespace asmjit;

  JitRuntime jit;
  LongJmpErrorHandler eh;

  CodeHolder code;
  code.init(jit.codeInfo());
  code.setErrorHandler(&eh);

  x86::Assembler a(&code);

  // Try to emit instruction that doesn't exist.
  if (!setjmp(eh.state)) {
    a.emit(x86::Inst::kIdMov, x86::xmm0, x86::xmm1);
  }
  else {
    Error err = eh.err;
    printf("ASMJIT ERROR: 0x%08X [%s]\n", err, DebugUtils::errorAsString(err));
  }

  return 0;
}
```

### Code Injection

`BaseBuilder` and `BaseCompiler` emitters store their nodes in a double-linked list, which makes it easy to manipulate during the code generation or after it. Each node is always emitted next to the current `cursor` and the cursor is changed to that newly emitted node. Cursor can be explicitly retrieved and assigned by `cursor()` and `setCursor()`, respectively.

The following example shows how to inject code at the beginning of the function by implementing an `XmmConstInjector` helper class.

```c++
```

### TODO

...More documentation...

Support
-------

Please consider a donation if you use the project and would like to keep it active in the future.

  * [Donate by PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=QDRM6SRNG7378&lc=EN;&item_name=asmjit&currency_code=EUR)

Donors:

  * [ZehMatt](https://github.com/ZehMatt)

Authors & Maintainers
---------------------

  * Petr Kobalicek <kobalicek.petr@gmail.com>
