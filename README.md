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
  JitRuntime rt;                          // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.

  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.
  a.mov(x86::eax, 1);                     // Move one to 'eax' register.
  a.ret();                                // Return from function.
  // ----> X86Assembler is no longer needed from here and can be destroyed <----

  Func fn;
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "1".

  // All classes use RAII, all resources will be released before `main()` returns,
  // the generated function can be, however, released explicitly if you intend to
  // reuse or keep the runtime alive, which you should in a production-ready code.
  rt.release(fn);

  return 0;
}
```

AsmJit Summary
--------------

  * Complete x86/x64 instruction set - MMX, SSEx, BMIx, ADX, TBM, XOP, AVXx, FMAx, and AVX512.
  * Assembler, CodeBuilder, and CodeCompiler emitters - each suitable for different tasks.
  * Built-in CPU vendor and features detection.
  * Advanced logging/formatting and robust error handling.
  * Virtual memory management similar to malloc/free for JIT code-generation and execution.
  * Lightweight and embeddable - 200-250kB compiled with all built-in features.
  * Modularity - unneeded features can be disabled at compile-time to make the library smaller.
  * Zero dependencies - no external libraries, no STL/RTTI - easy to embed and/or link statically.
  * Doesn't use exceptions internally, but allows to attach a "throwable" error handler (your choice).

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
  * Breaking changes are described in [BREAKING.md](./BREAKING.md) document.
  * Visit our [Official Chat](https://gitter.im/asmjit/asmjit) if you need a quick help.
  * See asmjit tests, they always compile and provide an implementation of a lot of use-cases:
    * [asmjit_test_x86_asm.cpp](./test/asmjit_test_x86_cc.cpp) - Tests that use **X86Assembler** and **X86Builder**.
    * [asmjit_test_x86_cc.cpp](./test/asmjit_test_x86_cc.cpp) - Tests that use **X86Compiler**.

TODOs:
  * [ ] AsmJit added support for code sections, but only the first section (executable code) works atm.
  * [ ] AsmJit supports AVX512, but {sae} and {er} are not handled properly yet.
  * [ ] AsmJit next-wip branch implements a brand-new register allocator (and contains reworked CodeBuilder and CodeCompiler), but it's not complete yet.

Supported Environments
----------------------

### C++ Compilers:

  * Tested
    * **Clang** - tested by Travis-CI.
    * **GCC** - tested by Travis-CI.
    * **MinGW** - tested by AppVeyor.
    * **MSVC** - tested by AppVeyor.
  * Maybe
    * **CodeGear** - no maintainers.
    * **Intel** - no maintainers.
  * Other c++ compilers would require some testing and support in [asmjit_build.h](./src/asmjit/asmjit_build.h).

### Operating Systems:

  * Tested
    * **Linux** - tested by Travis-CI.
    * **Mac** - tested by Travis-CI.
    * **Windows** - tested by AppVeyor.
  * Maybe
    * **BSDs** - no maintainers.
  * Other operating systems would require some testing and support in [asmjit_build.h](./src/asmjit/asmjit_build.h) and [osutils.cpp](./src/asmjit/base/osutils.cpp).

### Backends:

  * **X86** - tested by both Travis-CI and AppVeyor.
  * **X64** - tested by both Travis-CI and AppVeyor.
  * **ARM** - work-in-progress (not public at the moment).

Project Organization
--------------------

  * **`/`**        - Project root
    * **src**      - Source code
      * **asmjit** - Source code and headers (always point include path in here)
        * **base** - Backend independent API
        * **arm**  - ARM specific API, used only by ARM32 and ARM64 backends
        * **x86**  - X86 specific API, used only by X86 and X64 backends
    * **test**     - Unit and integration tests (don't embed in your project)
    * **tools**    - Tools used for configuring, documenting and generating files

Configuring & Building
----------------------

AsmJit is designed to be easy embeddable in any project. However, it depends on some compile-time macros that can be used to build a specific version of AsmJit that includes or excludes certain features. A typical way of building AsmJit is to use [cmake](https://www.cmake.org), but it's also possible to just include AsmJit source code in your project and just build it. The easiest way to include AsmJit in your project is to just include **src** directory in your project and to define **ASMJIT_STATIC** or **ASMJIT_EMBED**. AsmJit can be just updated from time to time without any changes to this integration process. Do not embed AsmJit's [/test](./test) files in such case as these are used for testing.

### Build Type:

  * **ASMJIT_DEBUG** - Define to always turn debugging on (regardless of compile-time options detected).
  * **ASMJIT_RELEASE** - Define to always turn debugging off (regardless of compile-time options detected).

By default none of these is defined, AsmJit detects build-type based on compile-time macros and supports most IDE and compiler settings out of box.

### Build Mode:

  * **ASMJIT_EMBED** - Define to embed AsmJit in another project. Embedding means that neither shared nor static library is created and AsmJit's source files and source files of the product that embeds AsmJit are part of the same target. This way of building AsmJit has certain advantages that are beyond this manual. **ASMJIT_EMBED** behaves similarly to **ASMJIT_STATIC** (no API exports).
  * **ASMJIT_STATIC** - Define to build AsmJit as a static library. No symbols are exported in such case.

By default AsmJit build is configured to be built as a shared library, thus none of **ASMJIT_EMBED** and **ASMJIT_STATIC** is defined.

### Build Backends:

  * **ASMJIT_BUILD_ARM** - Build ARM32 and ARM64 backends (work-in-progress).
  * **ASMJIT_BUILD_X86** - Build X86 and X64 backends.
  * **ASMJIT_BUILD_HOST** - Build only the host backend (default).

If none of **ASMJIT_BUILD_...** is defined AsmJit bails to **ASMJIT_BUILD_HOST**, which will detect the target architecture at compile-time. Each backend automatically supports 32-bit and 64-bit targets, so for example AsmJit with X86 support can generate both 32-bit and 64-bit code.

### Disabling Features:

  * **ASMJIT_DISABLE_BUILDER** - Disables both **CodeBuilder** and **CodeCompiler** emitters (only **Assembler** will be available). Ideal for users that don't use **CodeBuilder** concept and want to create smaller AsmJit.
  * **ASMJIT_DISABLE_COMPILER** - Disables **CodeCompiler** emitter. For users that use **CodeBuilder**, but not **CodeCompiler**
  * **ASMJIT_DISABLE_LOGGING** - Disables logging (**Logger** and all classes that inherit it) and formatting features.
  * **ASMJIT_DISABLE_TEXT** - Disables everything that uses text-representation and that causes certain strings to be stored in the resulting binary. For example when this flag is enabled all instruction and error names (and related APIs) will not be available. This flag has to be disabled together with **ASMJIT_DISABLE_LOGGING**. This option is suitable for deployment builds or builds that don't want to reveal the use of AsmJit.
  * **ASMJIT_DISABLE_VALIDATION** - Disables instruction validation feature. Saves around 5kB of space when used.

NOTE: Please don't disable any features if you plan to build AsmJit as a shared library that will be used by multiple projects that you don't control (for example asmjit in a Linux distribution). The possibility to disable certain features exists mainly for static builds of AsmJit.

Using AsmJit
------------

AsmJit library uses one global namespace called `asmjit` that provides the whole functionality. Architecture specific code is prefixed by the architecture name and architecture specific registers and operand builders have their own namespace. For example API targeting both X86 and X64 architectures is prefixed with `X86` and registers & operand builders are accessible through `x86` namespace. This design is very different from the initial version of AsmJit and it seems now as the most convenient one.

### CodeHolder & CodeEmitter

AsmJit provides two classes that are used together for code generation:

  * **CodeHolder** - Provides functionality to hold generated code and stores all necessary information about code sections, labels, symbols, and possible relocations.
  * **CodeEmitter** - Provides functionality to emit code into **CodeHolder**. **CodeEmitter** is abstract and provides just basic building blocks that are then implemented by **Assembler**, **CodeBuilder**, and **CodeCompiler**.

Code emitters:

  * **Assembler** - Emitter designed to emit machine code directly.
  * **CodeBuilder** - Emitter designed to emit code into a representation that can be processed. It stores the whole code in a double linked list consisting of nodes (**CBNode** aka code-builder node). There are nodes that represent instructions (**CBInst**), labels (**CBLabel**), and other building blocks (**CBAlign**, **CBData**, ...). Some nodes are used as markers (**CBSentinel**) and comments (**CBComment**).
  * **CodeCompiler** - High-level code emitter that uses virtual registers and contains high-level function building features. **CodeCompiler** is based on **CodeBuilder**, but extends its functionality and introduces new node types starting with CC (**CCFunc**, **CCFuncExit**, **CCFuncCall**). CodeCompiler is the simplest way to start with AsmJit as it abstracts many details required to generate a function in asm language.

### Runtime

AsmJit's **Runtime** is designed for execution and/or linking. The **Runtime** itself is abstract and defines only how to **add()** and **release()** code held by **CodeHolder**. **CodeHolder** holds machine code and relocation entries, but should be seen as a temporary object only - after the code in **CodeHolder** is ready, it should be passed to **Runtime** or relocated manually. Users interested in inspecting the generated machine-code (instead of executing or linking) can keep it in **CodeHodler** and process it manually of course.

The only **Runtime** implementation provided directly by AsmJit is called **JitRuntime**, which is suitable for storing and executing dynamically generated code. **JitRuntime** is used in most AsmJit examples as it makes the code management easy. It allows to add and release dynamically generated functions, so it's suitable for JIT code generators that want to keep many functions alive, and release functions which are no longer needed.

### Instructions & Operands

Instructions specify operations performed by the CPU, and operands specify the operation's input(s) and output(s). Each AsmJit's instruction has it's own unique id (**X86Inst::Id** for example) and platform specific code emitters always provide a type safe intrinsic (or multiple overloads) to emit such instruction. There are two ways of emitting an instruction:

  * Using emitter.**instName**(operands...) - A type-safe way provided by platform specific emitters - for example **X86Assembler** provides `mov(X86Gp, X86Gp)`.
  * Using emitter.emit(**instId**, operands...) - Allows to emit an instruction in a dynamic way - you just need to know instruction's id and provide its operands.

AsmJit's operands all inherit from a base class called **Operand** and then specialize its type to:

  * **None** (not used or uninitialized operand).
  * **Register** (**Reg**) - Describes either physical or virtual register. Physical registers have id that matches the target's machine id directly, whereas virtual registers must be allocated into physical registers by a register allocator pass. Each **Reg** provides:
    * **Register Type** - Unique id that describes each possible register provided by the target architecture - for example X86 backend provides **X86Reg::RegType**, which defines all variations of general purpose registers (GPB-LO, GPB-HI, GPW, GPD, and GPQ) and all types of other registers like K, MM, BND, XMM, YMM, and ZMM.
    * **Register Kind** - Groups multiple register types under a single kind - for example all general-purpose registers (of all sizes) on X86 are **X86Reg::kKindGp**, all SIMD registers (XMM, YMM, ZMM) are **X86Reg::kKindVec**, etc.
    * **Register Size** - Contains the size of the register in bytes. If the size depends on the mode (32-bit vs 64-bit) then generally the higher size is used (for example RIP register has size 8 by default).
    * **Register ID** - Contains physical or virtual id of the register.
  * **Memory Address** (**Mem**) - Used to reference a memory location. Each **Mem** provides:
    * **Base Register** - A base register id (physical or virtual).
    * **Index Register** - An index register id (physical or virtual).
    * **Offset** - Displacement or absolute address to be referenced (32-bit if base register is used and 64-bit if base register is not used).
    * **Flags** that can describe various architecture dependent information (like scale and segment-override on X86).
  * **Immediate Value** (**Imm**) - Immediate values are usually part of instructions (encoded within the instruction itself) or data.
  * **Label** - used to reference a location in code or data. Labels must be created by the **CodeEmitter** or by **CodeHolder**. Each label has its unique id per **CodeHolder** instance.

AsmJit allows to construct operands dynamically, to store them, and to query a complete information about them at run-time. Operands are small (always 16 bytes per **Operand**) and should be always copied if you intend to store them (don't create operands by using **new** keyword, it's not recommended). Operands are safe to be **memcpy()ed** and **memset()ed** if you need to work with arrays of operands.

Small example of manipulating and using operands:

```c++
using namespace asmjit;

X86Gp getDstRegByValue() { return x86::ecx; }

void usingOperandsExample(X86Assembler& a) {
  // Create some operands.
  X86Gp dst = getDstRegByValue();         // Get `ecx` register returned by a function.
  X86Gp src = x86::rax;                   // Get `rax` register directly from the provided `x86` namespace.
  X86Gp idx = x86::gpq(10);               // Construct `r10` dynamically.
  X86Mem m = x86::ptr(src, idx);          // Construct [src + idx] memory address - referencing [rax + r10].

  // Examine `m`:
  m.getIndexType();                       // Returns `X86Reg::kRegGpq`.
  m.getIndexId();                         // Returns 10 (`r10`).

  // Reconstruct `idx` stored in mem:
  X86Gp idx_2 = X86Gp::fromTypeAndId(m.getIndexType(), m.getIndexId());
  idx == idx_2;                           // True, `idx` and idx_2` are identical.

  Operand op = m;                         // Possible.
  op.isMem();                             // True (can be casted to Mem and X86Mem).

  m == op;                                // True, `op` is just a copy of `m`.
  static_cast<Mem&>(op).addOffset(1);     // Static cast is fine and valid here.
  m == op;                                // False, `op` now points to [rax + r10 + 1], which is not [rax + r10].

  // Emitting 'mov'
  a.mov(dst, m);                          // Type-safe way.
  a.mov(dst, op);                         // Not possible, `mov` doesn't provide `X86Reg, Operand` overload.

  a.emit(X86Inst::kIdMov, dst, m);        // Type-unsafe, but possible.
  a.emit(X86Inst::kIdMov, dst, op);       // Also possible, `emit()` is typeless and can be used dynamically.
}
```

Some operands have to be created explicitly by `CodeEmitter`. For example labels must be created by `newLabel()` before they are used.

### Assembler Example

X86Assembler is a code emitter that emits machine code into a CodeBuffer directly. It's capable of targeting both 32-bit and 64-bit instruction sets and it's possible to target both instruction sets within the same code-base. The following example shows how to generate a function that works in both 32-bit and 64-bit modes, and how to use JitRuntime, CodeHolder, and X86Assembler together.

The example handles 3 calling conventions manually just to show how it could be done, however, AsmJit contains utilities that can be used to create function prologs and epilogs automatically, but these concepts will be explained later.

```c++
using namespace asmjit;

// Signature of the generated function.
typedef int (*SumFunc)(const int* arr, size_t count);

int main(int argc, char* argv[]) {
  assert(sizeof(void*) == 8 &&
    "This example requires 64-bit environment.");

  JitRuntime rt;                          // Create a runtime specialized for JIT.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(rt.getCodeInfo());            // Initialize it to be compatible with `rt`.

  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.

  // Decide between 32-bit CDECL, WIN64, and SysV64 calling conventions:
  //   32-BIT - passed all arguments by stack.
  //   WIN64  - passes first 4 arguments by RCX, RDX, R8, and R9.
  //   UNIX64 - passes first 6 arguments by RDI, RSI, RCX, RDX, R8, and R9.
  X86Gp arr, cnt;
  X86Gp sum = x86::eax;                   // Use EAX as 'sum' as it's a return register.

  if (ASMJIT_ARCH_64BIT) {
    bool isWinOS = static_cast<bool>(ASMJIT_OS_WINDOWS);
    arr = isWinOS ? x86::rcx : x86::rdi;  // First argument (array ptr).
    cnt = isWinOS ? x86::rdx : x86::rsi;  // Second argument (number of elements)
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
  // ----> X86Assembler is no longer needed from here and can be destroyed <----

  SumFunc fn;
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.

  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  static const int array[6] = { 4, 8, 15, 16, 23, 42 };

  int result = fn(array, 6);              // Execute the generated code.
  printf("%d\n", result);                 // Print sum of array (108).

  rt.release(fn);                         // Remove the function from the runtime.
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
X86Mem a = ptr(rax);                      // a = [rax]
X86Mem b = ptr(rax, 15)                   // b = [rax + 15]

// BASE + INDEX + SCALE - Scale is in BITS as used by X86!
X86Mem c = ptr(rax, rbx)                  // c = [rax + rbx]
X86Mem d = ptr(rax, rbx, 2)               // d = [rax + rbx << 2]
X86Mem e = ptr(rax, rbx, 2, 15)           // e = [rax + rbx << 2 + 15]

// BASE + VM (Vector Index) (encoded as MOD+VSIB).
X86Mem f = ptr(rax, xmm1)                 // f = [rax + xmm1]
X86Mem g = ptr(rax, xmm1, 2)              // g = [rax + xmm1 << 2]
X86Mem h = ptr(rax, xmm1, 2, 15)          // h = [rax + xmm1 << 2 + 15]

// WITHOUT BASE:
uint64_t ADDR = (uint64_t)0x1234;
X86Mem i = ptr(ADDR);                     // i = [0x1234]
X86Mem j = ptr(ADDR, rbx);                // j = [0x1234 + rbx]
X86Mem k = ptr(ADDR, rbx, 2);             // k = [0x1234 + rbx << 2]

// LABEL - Will be encoded as RIP (64-bit) or absolute address (32-bit).
Label L = ...;
X86Mem m = ptr(L);                        // m = [L]
X86Mem n = ptr(L, rbx);                   // n = [L + rbx]
X86Mem o = ptr(L, rbx, 2);                // o = [L + rbx << 2]
X86Mem p = ptr(L, rbx, 2, 15);            // p = [L + rbx << 2 + 15]

// RIP - 64-bit only (RIP can't use INDEX).
X86Mem q = ptr(rip, 24);                  // q = [rip + 24]
```

Memory operands can optionally contain memory size. This is required by instructions where the memory size cannot be deduced from other operands, like `inc` and `dec`:

```c++
X86Mem a = x86::dword_ptr(rax, rbx);      // dword ptr [rax + rbx].
X86Mem b = x86::qword_ptr(rdx, rsi, 0, 1);// qword ptr [rdx + rsi << 0 + 1].
```

Memory operands provide API that can be used to work with them:

```c++
X86Mem mem = x86::dword_ptr(rax, 12);     // dword ptr [rax + 12].

mem.hasBase();                            // true.
mem.hasIndex();                           // false.
mem.getSize();                            // 4.
mem.getOffset();                          // 12.

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

X86Assembler a(...);                      // Your initialized X86Assembler.
X86Mem m = x86::ptr(eax);                 // Construct [eax] memory operand.

// One way of emitting bunch of loads is to use `mem.adjusted()`. It returns
// a new memory operand and keeps the source operand unchanged.
a.movaps(x86::xmm0, m);                   // No adjustment needed to load [eax].
a.movaps(x86::xmm1, m.adjusted(16));      // Loads [eax + 16].
a.movaps(x86::xmm2, m.adjusted(32));      // Loads [eax + 32].
a.movaps(x86::xmm3, m.adjusted(48));      // Loads [eax + 48].

// ... do something with xmm0-3 ...

// Another way of adjusting memory is to change the operand in-place. If you
// want to keep the original operand you can simply clone it.
X86Mem mx = m.clone();
a.movaps(mx, x86::xmm0); mx.addOffset(16);// Stores [eax]      (and adds 16 to mx).
a.movaps(mx, x86::xmm1); mx.addOffset(16);// Stores [eax + 16] (and adds 16 to mx).
a.movaps(mx, x86::xmm2); mx.addOffset(16);// Stores [eax + 32] (and adds 16 to mx).
a.movaps(mx, x86::xmm3);                  // Stores [eax + 48].
```

You can explore the possibilities by taking a look at [base/operand.h](./src/asmjit/base/operand.h) and [x86/x86operand.h](./src/asmjit/x86/x86operand.h). Always use `X86Mem` when targeting X86 as it extends the base `Mem` operand with features provided only by X86.

### More About CodeInfo

In the first complete example the `CodeInfo` is retrieved from `JitRuntime`. It's logical as `JitRuntime` will always return a `CodeInfo` that is compatible with the runtime environment. For example if your application runs in 64-bit mode the `CodeInfo` will use `ArchInfo::kTypeX64` architecture in contrast to `ArchInfo::kTypeX86`, which will be used in 32-bit mode. AsmJit also allows to setup `CodeInfo` manually, and to select a different architecture when needed. So let's do something else this time, let's always generate a 32-bit code and print it's binary representation. To do that, we create our own `CodeInfo` and initialize it to `ArchInfo::kTypeX86` architecture. CodeInfo will populate all basic fields just based on the architecture we provide, so it's super-easy:

```c++
using namespace asmjit;

int main(int argc, char* argv[]) {
  using namespace asmjit::x86;            // Easier access to x86/x64 registers.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(CodeInfo(ArchInfo::kTypeX86));// Initialize it for a 32-bit X86 target.

  // Generate a 32-bit function that sums 4 floats and looks like:
  //   void func(float* dst, const float* a, const float* b)
  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.

  a.mov(eax, dword_ptr(esp, 4));          // Load the destination pointer.
  a.mov(ecx, dword_ptr(esp, 8));          // Load the first source pointer.
  a.mov(edx, dword_ptr(esp, 12));         // Load the second source pointer.

  a.movups(xmm0, ptr(ecx));               // Load 4 floats from [ecx] to XMM0.
  a.movups(xmm1, ptr(edx));               // Load 4 floats from [edx] to XMM1.
  a.addps(xmm0, xmm1);                    // Add 4 floats in XMM1 to XMM0.
  a.movups(ptr(eax), xmm0);               // Store the result to [eax].
  a.ret();                                // Return from function.

  // Now we have two options if we want to do something with the code hold
  // by CodeHolder. In order to use it we must first sync X86Assembler with
  // the CodeHolder as it doesn't do it for every instruction it generates for
  // performance reasons. The options are:
  //
  //   1. Detach X86Assembler from CodeHolder (will automatically sync).
  //   2. Sync explicitly, allows to use X86Assembler again if needed.
  //
  // NOTE: AsmJit always syncs internally when CodeHolder needs to access these
  // buffers and knows that there is an Assembler attached, so you have to sync
  // explicitly only if you bypass CodeHolder and intend to do something on your
  // own.
  code.sync();                            // So let's sync, it's easy.

  // We have no Runtime this time, it's on us what we do with the code.
  // CodeHolder stores code in SectionEntry, which embeds CodeSection
  // and CodeBuffer structures. We are interested in section's CodeBuffer only.
  //
  // NOTE: The first section is always '.text', so it's safe to just use 0 index.
  CodeBuffer& buf = code.getSectionEntry(0)->buffer;

  // Print the machine-code generated or do something more interesting with it?
  //   8B4424048B4C24048B5424040F28010F58010F2900C3
  for (size_t i = 0; i < buf.length; i++)
    printf("%02X", buf.data[i]);

  return 0;
}
```

### Explicit Code Relocation

CodeInfo contains much more information than just the target architecture. It can be configured to specify a base-address (or a virtual base-address in a linker terminology), which could be static (useful when you know the location of the target's machine code) or dynamic. AsmJit assumes dynamic base-address by default and relocates the code held by `CodeHolder` to a user-provided address on-demand. To be able to relocate to a user-provided address it needs to store some information about relocations, which is represented by `CodeHolder::RelocEntry`. Relocation entries are only required if you call external functions from the generated code that cannot be encoded by using a 32-bit displacement (X64 architecture doesn't provide 64-bit encodable displacement) and when a label referenced in one section is bound in another, but this is not really a JIT case and it's more related to AOT (ahead-of-time) compilation.

Next example shows how to use a built-in virtual memory manager `VMemMgr` instead of using `JitRuntime` (just in case you want to use your own memory management) and how to relocate the generated code into your own memory block - you can use your own virtual memory allocator if you need that, but that's OS specific and it's already provided by AsmJit, so we will use what AsmJit offers instead of rolling our own here.

The following code is similar to the previous one, but implements a function working in both 32-bit and 64-bit environments:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

int main(int argc, char* argv[]) {
  CodeHolder code;                        // Create a CodeHolder.
  code.init(CodeInfo(ArchInfo::kTypeHost));// Initialize it for the host architecture.

  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.

  // Generate a function runnable in both 32-bit and 64-bit architectures:
  bool isX86 = static_cast<bool>(ASMJIT_ARCH_X86);
  bool isWin = static_cast<bool>(ASMJIT_OS_WINDOWS);

  // Signature: 'void func(int* dst, const int* a, const int* b)'.
  X86Gp dst;
  X86Gp src_a;
  X86Gp src_b;

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
    dst   = isWin ? x86::rcx : x86::rdi;  // First argument  (destination pointer).
    src_a = isWin ? x86::rdx : x86::rsi;  // Second argument (source 'a' pointer).
    src_b = isWin ? x86::r8  : x86::rdx;  // Third argument  (source 'b' pointer).
  }

  a.movdqu(x86::xmm0, x86::ptr(src_a));   // Load 4 ints from [src_a] to XMM0.
  a.movdqu(x86::xmm1, x86::ptr(src_b));   // Load 4 ints from [src_b] to XMM1.
  a.paddd(x86::xmm0, x86::xmm1);          // Add 4 ints in XMM1 to XMM0.
  a.movdqu(x86::ptr(dst), x86::xmm0);     // Store the result to [dst].
  a.ret();                                // Return from function.

  // After the code was generated it can be relocated manually to any memory
  // location, however, we need to know it's size before we perform memory
  // allocation. CodeHolder's `getCodeSize()` returns the worst estimated
  // code-size (the biggest possible) in case that relocations are not
  // possible without trampolines (in that case some extra code at the end
  // of the current code buffer is generated during relocation).
  size_t size = code.getCodeSize();

  // Instead of rolling our own virtual memory allocator we can use the one
  // AsmJit uses. It's decoupled so you don't need to use Runtime for that.
  VMemMgr vm;

  void* p = vm.alloc(size);               // Allocate a virtual memory (executable).
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
  vm.release(p);

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

AsmJit's X86 code emitters always provide functions to construct machine-size registers depending on the target. This feature is for people that want to write code targeting both 32-bit and 64-bit at the same time. In AsmJit terminology these registers are named **zax**, **zcx**, **zdx**, **zbx**, **zsp**, **zbp**, **zsi**, and **zdi** (they are defined in this exact order by X86). They are accessible through `X86Assembler`, `X86Builder`, and `X86Compiler`. The following example illustrates how to use this feature:

```c++
using namespace asmjit;

typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Create a runtime specialized for JIT.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(rt.getCodeInfo());            // Initialize it to be compatible with `rt`.

  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.

  // Let's get these registers from X86Assembler.
  X86Gp zbp = a.zbp();
  X86Gp zsp = a.zsp();

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
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "0".

  rt.release(fn);                         // Remove the function from the runtime.
  return 0;
}
```

The example just returns `0`, but the function generated contains a standard prolog and epilog sequence and the function itself reserves 32 bytes of local stack. The advantage is clear - a single code-base can handle multiple targets easily. If you want to create a register of native size dynamically by specifying its id it's also possible:

```c++
void example(X86Assembler& a) {
  X86Gp zax = a.gpz(X86Gp::kIdAx);
  X86Gp zbx = a.gpz(X86Gp::kIdBx);
  X86Gp zcx = a.gpz(X86Gp::kIdCx);
  X86Gp zdx = a.gpz(X86Gp::kIdDx);

  // You can also change register's id easily.
  X86Gp zsp = zax;
  zsp.setId(4); // or X86Gp::kIdSp.
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
  JitRuntime rt;                          // Create a runtime specialized for JIT.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(rt.getCodeInfo());            // Initialize it to be compatible with `rt`.

  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.

  // Let's get these registers from X86Assembler.
  X86Gp zbp = a.zbp();
  X86Gp zsp = a.zsp();

  // Function prolog.
  a.push(zbp);
  a.mov(zbp, zsp);

  // This is where we are gonna patch the code later, so let's get the offset
  // (the current location) from the beginning of the code-buffer.
  size_t patchOffset = a.getOffset();
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
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "0".

  rt.release(fn);                         // Remove the function from the runtime.
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

AsmJit contains a functionality that can be used to define function signatures and to calculate automatically optimal frame layout that can be used directly by a prolog and epilog inserter. This feature was exclusive to AsmJit's CodeCompiler for a very long time, but was abstracted out and is now available for all users regardless of CodeEmitter they use. The design of handling functions prologs and epilogs allows generally two use cases:

  * Calculate function layout before the function is generated - this is the only way if you use pure `Assembler` emitter and shown in the next example.
  * Calculate function layout after the function is generated - this way is generally used by `CodeBuilder` and `CodeCompiler` (will be described together with `X86Compiler`).

The following concepts are used to describe and create functions in AsmJit:

  * **CallConv** - Describes a calling convention - this class contains instructions to assign registers and stack addresses to function arguments and return value(s), but doesn't specify any function signature. Calling conventions are architecture and OS dependent.

  * **TypeId** - TypeId is an 8-bit value that describes a platform independent type. It provides abstractions for most common types like `int8_t`, `uint32_t`, `uintptr_t`, `float`, `double`, and all possible vector types to match ISAs up to AVX512. **TypeId** was introduced originally to be used with **CodeCompiler**, but is now used by **FuncSignature** as well.

  * **FuncSignature** - Describes a function signature, for example `int func(int, int)`. **FuncSignature** contains a function calling convention id, return value type, and function arguments. The signature itself is platform independent and uses **TypeId** to describe types of function arguments and its return value(s).

  * **FuncDetail** - Architecture and ABI dependent information that describes **CallConv** and expanded **FuncSignature**. Each function argument and return value is represented as **FuncDetail::Value** that contains the original **TypeId** enriched by additional information that specifies if the value is passed/returned by register (and which register) or by stack. Each value also contains some other metadata that provide additional information required to handle it properly (for example if a vector value is passed indirectly by a pointer as required by WIN64 calling convention, etc...).

  * **FuncArgsMapper** - A helper that can be used to define where each function argument is expected to be. It's architecture and ABI dependent mapping from function arguments described by CallConv and FuncDetail into registers specified by the user.

  * **FuncFrameInfo** - Contains information about a function-frame. Holds callout-stack size and alignment (i.e. stack used to call functions), stack-frame size and alignment (the stack required by the function itself), and various attributes that describe how prolog and epilog should be constructed. FuncFrameInfo doesn't know anything about function arguments or returns, it should be seen as a class that describes minimum requirements of the function frame and its attributes before the final `FuncFrameLayout` is calculated.

  * **FuncFrameLayout** - Contains the final function layout that can be passed to `FuncUtils::emitProlog()` and `FuncUtils::emitEpilog()`. The content of this class should always be calculated by AsmJit by calling `FuncFrameLayout::init(const FuncDetail& detail, const FuncFrameInfo& ffi)`.

It's a lot of concepts where each represents one step in the function layout calculation. In addition, the whole machinery can also be used to create function calls, instead of function prologs and epilogs. The next example shows how AsmJit can be used to create functions for both 32-bit and 64-bit targets and various calling conventions:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Create JIT Runtime.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(rt.getCodeInfo());            // Initialize it to match `rt`.
  X86Assembler a(&code);                  // Create and attach X86Assembler to `code`.

  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  X86Gp dst   = a.zax();
  X86Gp src_a = a.zcx();
  X86Gp src_b = a.zdx();

  X86Xmm vec0 = x86::xmm0;
  X86Xmm vec1 = x86::xmm1;

  // Create and initialize `FuncDetail` and `FuncFrameInfo`. Both are
  // needed to create a function and they hold different kind of data.
  FuncDetail func;
  func.init(FuncSignature3<void, int*, const int*, const int*>(CallConv::kIdHost));

  FuncFrameInfo ffi;
  ffi.setDirtyRegs(X86Reg::kKindVec,      // Make XMM0 and XMM1 dirty. VEC kind
                   Utils::mask(0, 1));    // describes XMM|YMM|ZMM registers.

  FuncArgsMapper args(&func);             // Create function arguments mapper.
  args.assignAll(dst, src_a, src_b);      // Assign our registers to arguments.
  args.updateFrameInfo(ffi);              // Reflect our args in FuncFrameInfo.

  FuncFrameLayout layout;                 // Create the FuncFrameLayout, which
  layout.init(func, ffi);                 // contains metadata of prolog/epilog.

  FuncUtils::emitProlog(&a, layout);      // Emit function prolog.
  FuncUtils::allocArgs(&a, layout, args); // Allocate arguments to registers.
  a.movdqu(vec0, x86::ptr(src_a));        // Load 4 ints from [src_a] to XMM0.
  a.movdqu(vec1, x86::ptr(src_b));        // Load 4 ints from [src_b] to XMM1.
  a.paddd(vec0, vec1);                    // Add 4 ints in XMM1 to XMM0.
  a.movdqu(x86::ptr(dst), vec0);          // Store the result to [dst].
  FuncUtils::emitEpilog(&a, layout);      // Emit function epilog and return.

  SumIntsFunc fn;
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error case.

  // Execute the generated function.
  int inA[4] = { 4, 3, 2, 1 };
  int inB[4] = { 1, 5, 2, 8 };
  int out[4];
  fn(out, inA, inB);

  // Prints {5 8 4 9}
  printf("{%d %d %d %d}\n", out[0], out[1], out[2], out[3]);

  rt.release(fn);                         // Remove the function from the runtime.
  return 0;
}
```

CodeBuilder
-----------

Both **CodeBuilder** and **CodeCompiler** are emitters that emit everything to a representation that allows further processing. The code stored in such representation is completely safe to be patched, simplified, reordered, obfuscated, removed, injected, analyzed, and 'think-of-anything-else'. Each instruction (or label, directive, ...) is stored as **CBNode** (Code-Builder Node) and contains all the necessary information to emit machine code out of it later.

There is a difference between **CodeBuilder** and **CodeCompiler**:

  * **CodeBuilder** (low-level):
    * Maximum compatibility with **Assembler**, easy to switch from **Assembler** to **CodeBuilder** and vice versa.
    * Doesn't generate machine code directly, allows to serialize to **Assembler** when the whole code is ready to be encoded.

  * **CodeCompiler** (high-level):
    * Virtual registers - allows to use unlimited number of virtual registers which are allocated into physical registers by a built-in register allocator.
    * Function nodes - allows to create functions by specifying their signatures and assigning virtual registers to function arguments and return value(s).
    * Function calls - allows to call other functions within the generated code by using the same interface that is used to create functions.

There are multiple node types used by both **CodeBuilder** and **CodeCompiler**:

  * Basic nodes:
    * **CBNode** - Base class for all nodes.
    * **CBInst** - Instruction node.
    * **CBAlign** - Alignment directive (.align).
    * **CBLabel** - Label (location where to bound it).

  * Data nodes:
    * **CBData** - Data embedded into the code.
    * **CBConstPool** - Constant pool data.
    * **CBLabelData** - Label address embedded as data.

  * Informative nodes:
    * **CBComment** - Contains a comment string, doesn't affect code generation.
    * **CBSentinel** - A marker that can be used to remember certain position, doesn't affect code generation.

  * **CodeCompiler** nodes:
    * **CCFunc** - Start of a function.
    * **CCFuncExit** - Return from a function.
    * **CCFuncCall** - Function call.

NOTE: All nodes that have **CB** prefix are used by both **CodeBuilder** and **CodeCompiler**. Nodes that have **CC** prefix are exclusive to **CodeCompiler** and are usually lowered to **CBNodes** by a **CodeBuilder** specific pass or treated as one of **CB** nodes; for example **CCFunc** inherits **CBLabel** so it's treated as **CBLabel** by **CodeBuilder** and as **CCFunc** by **CodeCompiler**.

### Using CodeBuilder

**CodeBuilder** was designed to be used as an **Assembler** replacement in case that post-processing of the generated code is required. The code can be modified during or after code generation. The post processing can be done manually or through **Pass** (Code-Builder Pass) object. **CodeBuilder** stores the emitted code as a double-linked list, which allows O(1) insertion and removal.

The code representation used by **CodeBuilder** is compatible with everything AsmJit provides. Each instruction is stored as **CBInst**, which contains instruction id, options, and operands. Each instruction emitted will create a new **CBInst** instance and add it to the current cursor in the double-linked list of nodes. Since the instruction stream used by **CodeBuilder** can be manipulated, we can rewrite the **SumInts** example into the following:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

// Small helper function to print the current content of `cb`.
static void dumpCode(CodeBuilder& cb, const char* phase) {
  StringBuilder sb;
  cb.dump(sb);
  printf("%s:\n%s\n", phase, sb.getData());
}

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Create JIT Runtime.

  CodeHolder code;                        // Create a CodeHolder.
  code.init(rt.getCodeInfo());            // Initialize it to match `rt`.
  X86Builder cb(&code);                   // Create and attach X86Builder to `code`.

  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  X86Gp dst   = cb.zax();
  X86Gp src_a = cb.zcx();
  X86Gp src_b = cb.zdx();

  X86Xmm vec0 = x86::xmm0;
  X86Xmm vec1 = x86::xmm1;

  // Create and initialize `FuncDetail`.
  FuncDetail func;
  func.init(FuncSignature3<void, int*, const int*, const int*>(CallConv::kIdHost));

  // Remember prolog insertion point.
  CBNode* prologInsertionPoint = cb.getCursor();

  // Emit function body:
  cb.movdqu(vec0, x86::ptr(src_a));       // Load 4 ints from [src_a] to XMM0.
  cb.movdqu(vec1, x86::ptr(src_b));       // Load 4 ints from [src_b] to XMM1.
  cb.paddd(vec0, vec1);                   // Add 4 ints in XMM1 to XMM0.
  cb.movdqu(x86::ptr(dst), vec0);         // Store the result to [dst].

  // Remember epilog insertion point.
  CBNode* epilogInsertionPoint = cb.getCursor();

  // Let's see what we have now.
  dumpCode(cb, "Raw Function");

  // Now, after we emitted the function body, we can insert the prolog, arguments
  // allocation, and epilog. This is not possible with using pure X86Assembler.
  FuncFrameInfo ffi;
  ffi.setDirtyRegs(X86Reg::kKindVec,      // Make XMM0 and XMM1 dirty. VEC kind
                   Utils::mask(0, 1));    // describes XMM|YMM|ZMM registers.

  FuncArgsMapper args(&func);             // Create function arguments mapper.
  args.assignAll(dst, src_a, src_b);      // Assign our registers to arguments.
  args.updateFrameInfo(ffi);              // Reflect our args in FuncFrameInfo.

  FuncFrameLayout layout;                 // Create the FuncFrameLayout, which
  layout.init(func, ffi);                 // contains metadata of prolog/epilog.

  // Insert function prolog and allocate arguments to registers.
  cb.setCursor(prologInsertionPoint);
  FuncUtils::emitProlog(&cb, layout);
  FuncUtils::allocArgs(&cb, layout, args);

  // Insert function epilog.
  cb.setCursor(epilogInsertionPoint);
  FuncUtils::emitEpilog(&cb, layout);

  // Let's see how the function prolog and epilog looks.
  dumpCode(cb, "Prolog & Epilog");

  // IMPORTANT: CodeBuilder requires `finalize()` to be called to serialize
  // the code to the Assembler (it automatically creates one if not attached).
  cb.finalize();

  SumIntsFunc fn;
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error case.

  // Execute the generated function.
  int inA[4] = { 4, 3, 2, 1 };
  int inB[4] = { 1, 5, 2, 8 };
  int out[4];
  fn(out, inA, inB);

  // Prints {5 8 4 9}
  printf("{%d %d %d %d}\n", out[0], out[1], out[2], out[3]);

  rt.release(fn);                         // Remove the function from the runtime.
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

The number of use-cases of **X86Builder** is not limited and highly depends on your creativity and experience. The previous example can be easily improved to collect all dirty registers inside the function programmatically and to pass them to `ffi.setDirtyRegs()`:

```c++
using namespace asmjit;

// NOTE: This function doesn't cover all possible instructions. It ignores
// instructions that write to implicit registers that are not part of the
// operand list. It also counts read-only registers. Real implementation
// would be a bit more complicated, but still relatively easy.
static void collectDirtyRegs(const CBNode* first, const CBNode* last, uint32_t regMask[X86Reg::kKindCount]) {
  const CBNode* node = first;
  while (node) {
    if (node->actsAsInst()) {
      const CBInst* inst = node->as<CBInst>();
      const Operand* opArray = inst->getOpArray();

      for (uint32_t i = 0, opCount = inst->getOpCount(); i < opCount; i++) {
        const Operand& op = opArray[i];
        if (op.isReg()) {
          const X86Reg& reg = op.as<X86Reg>();
          regMask[reg.getKind()] |= 1U << reg.getId();
        }
      }
    }

    if (node == last) break;
    node = node->getNext();
  }
}

static void setDirtyRegsOfFFI(const X86Builder& cb, FuncFrameInfo& ffi) {
  uint32_t regMask[X86Reg::kKindCount] = { 0 };
  collectDirtyRegs(cb.getFirstNode(), cb.getLastNode(), regMask);

  // X86/X64 ABIs only require to save GP/XMM registers:
  ffi.setDirtyRegs(X86Reg::kKindGp, regMask[X86Reg::kKindGp]);
  ffi.setDirtyRegs(X86Reg::kKindVec, regMask[X86Reg::kKindVec]);
}
```

### Using X86Assembler or X86Builder through X86Emitter

Even when **Assembler** and **CodeBuilder** implement the same interface defined by **CodeEmitter** their platform dependent variants (**X86Assembler** and **X86Builder**, respective) cannot be interchanged or casted to each other by using C++'s `static_cast<>`. The main reason is the inheritance graph of these classes is different and cast-incompatible, as illustrated in the following graph:

```
                                            +--------------+      +=======================+
                   +----------------------->|  X86Emitter  |<--+--# X86EmitterImplicitT<> #<--+
                   |                        +--------------+   |  +=======================+   |
                   |                           (abstract)      |          (mixin)             |
                   |   +--------------+     +~~~~~~~~~~~~~~+   |                              |
                   +-->|  Assembler   |---->| X86Assembler |<--+                              |
                   |   +--------------+     +~~~~~~~~~~~~~~+   |                              |
                   |      (abstract)            (final)        |                              |
+===============+  |   +--------------+     +~~~~~~~~~~~~~~+   |                              |
#  CodeEmitter  #--+-->| CodeBuilder  |--+->|  X86Builder  |<--+                              |
+===============+      +--------------+  |  +~~~~~~~~~~~~~~+                                  |
   (abstract)             (abstract)     |      (final)                                       |
                   +---------------------+                                                    |
                   |                                                                          |
                   |   +--------------+     +~~~~~~~~~~~~~~+      +=======================+   |
                   +-->| CodeCompiler |---->| X86Compiler  |<-----# X86EmitterExplicitT<> #---+
                       +--------------+     +~~~~~~~~~~~~~~+      +=======================+
                          (abstract)            (final)                   (mixin)
```

The graph basically shows that it's not possible to cast **X86Assembler** to **X86Builder** and vice versa. However, since both **X86Assembler** and **X86Builder** share the same interface defined by both **CodeEmitter** and **X86EmmiterImplicitT** a class called **X86Emitter** was introduced to make it possible to write a function that can emit to both **X86Assembler** and **X86Builder**. Note that **X86Emitter** cannot be created, it's abstract and has private constructors and destructors; it was only designed to be casted to and used as an interface.

Each X86 emitter implements a member function called **asEmitter()**, which casts the instance to the **X86Emitter**, as illustrated on the next example:

```c++
using namespace asmjit;

static void emitSomething(X86Emitter* e) {
  e->mov(x86::eax, x86::ebx);
}

static void assemble(CodeHolder& code, bool useAsm) {
  if (useAsm) {
    X86Assembler a(&code);
    emitSomething(a.asEmitter());
  }
  else {
    X86Builder cb(&code);
    emitSomething(cb.asEmitter());

    // IMPORTANT: CodeBuilder requires `finalize()` to be called to serialize
    // the code to the Assembler (it automatically creates one if not attached).
    cb.finalize();
  }
}
```

The example above shows how to create a function that can emit code to either **X86Assembler** or **X86Builder** through **X86Emitter**, which provides emitter-neutral functionality. **X86Emitter**, however, doesn't provide any emitter **X86Assembler** or **X86Builder** specific functionality like **setCursor()**.

CodeCompiler
------------

**CodeCompiler** is a high-level code emitter that provides virtual registers and automatically handles function calling conventions. It's still architecture dependent, but makes the code generation much easier by offering a built-in register allocator and function builder. Functions are essential; the first-step to generate some code is to define the signature of the function you want to generate (before generating the function body). Function arguments and return value(s) are handled by assigning virtual registers to them. Similarly, function calls are handled the same way.

**CodeCompiler** also makes the use of passes (introduced by **CodeBuilder**) and automatically adds an architecture-dependent register allocator pass to the list of passes when attached to **CodeHolder**.

### Compiler Basics

The first **CodeCompiler** example shows how to generate a function that simply returns an integer value. It's an analogy to the very first example:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.

  X86Compiler cc(&code);                  // Create and attach X86Compiler to `code`.
  cc.addFunc(FuncSignature0<int>());      // Begin a function of `int fn(void)` signature.

  X86Gp vReg = cc.newGpd();               // Create a 32-bit general purpose register.
  cc.mov(vReg, 1);                        // Move one to our virtual register `vReg`.
  cc.ret(vReg);                           // Return `vReg` from the function.

  cc.endFunc();                           // End of the function body.
  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> X86Compiler is no longer needed from here and can be destroyed <----

  Func fn;
  Error err = rt.add(&fn, &code);         // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  int result = fn();                      // Execute the generated code.
  printf("%d\n", result);                 // Print the resulting "1".

  rt.release(fn);                         // RAII, but let's make it explicit.
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
  JitRuntime rt;                          // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.

  X86Compiler cc(&code);                  // Create and attach X86Compiler to `code`.
  cc.addFunc(                             // Begin the function of the following signature:
    FuncSignature3<void,                  //   Return value - void      (no return value).
      uint32_t*,                          //   1st argument - uint32_t* (machine reg-size).
      const uint32_t*,                    //   2nd argument - uint32_t* (machine reg-size).
      size_t>());                         //   3rd argument - size_t    (machine reg-size).

  Label L_Loop = cc.newLabel();           // Start of the loop.
  Label L_Exit = cc.newLabel();           // Used to exit early.

  X86Gp dst = cc.newIntPtr("dst");        // Create `dst` register (destination pointer).
  X86Gp src = cc.newIntPtr("src");        // Create `src` register (source pointer).
  X86Gp cnt = cc.newUIntPtr("cnt");       // Create `cnt` register (loop counter).

  cc.setArg(0, dst);                      // Assign `dst` argument.
  cc.setArg(1, src);                      // Assign `src` argument.
  cc.setArg(2, cnt);                      // Assign `cnt` argument.

  cc.test(cnt, cnt);                      // Early exit if length is zero.
  cc.jz(L_Exit);

  cc.bind(L_Loop);                        // Bind the beginning of the loop here.

  X86Gp tmp = cc.newInt32("tmp");         // Copy a single dword (4 bytes).
  cc.mov(tmp, x86::dword_ptr(src));       // Load DWORD from [src] address.
  cc.mov(x86::dword_ptr(dst), tmp);       // Store DWORD to [dst] address.

  cc.add(src, 4);                         // Increment `src`.
  cc.add(dst, 4);                         // Increment `dst`.

  cc.dec(cnt);                            // Loop until `cnt` is non-zero.
  cc.jnz(L_Loop);

  cc.bind(L_Exit);                        // Label used by early exit.
  cc.endFunc();                           // End of the function body.

  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> X86Compiler is no longer needed from here and can be destroyed <----

  MemCpy32 memcpy32;
  Error err = rt.add(&memcpy32, &code);   // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  // Test the generated code.
  uint32_t src[6] = { 1, 2, 3, 5, 8, 13 };
  uint32_t dst[6];
  memcpy32(dst, src, 6);

  for (uint32_t i = 0; i < 6; i++)
    printf("%d\n", dst[i]);

  rt.release(memcpy32);                   // RAII, but let's make it explicit.
  return 0;
}
```

### Recursive Functions

It's possible to create more functions by using the same `X86Compiler` instance and make links between them. In such case it's important to keep the pointer to the `CCFunc` node. The first example creates a simple Fibonacci function that calls itself recursively:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef uint32_t (*Fibonacci)(uint32_t x);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.

  X86Compiler cc(&code);                  // Create and attach X86Compiler to `code`.
  CCFunc* func = cc.addFunc(              // Begin of the Fibonacci function, `addFunc()`
    FuncSignature1<int, int>());          // Returns a pointer to the `CCFunc` node.

  Label L_Exit = cc.newLabel()            // Exit label.
  X86Gp x = cc.newU32();                  // Function `x` argument.
  X86Gp y = cc.newU32();                  // Temporary.

  cc.setArg(0, x);

  cc.cmp(x, 3);                           // Return `x` if less than 3.
  cc.jb(L_Exit);

  cc.mov(y, x);                           // Make copy of the original `x`.
  cc.dec(x);                              // Decrease `x`.

  CCFuncCall* call = cc.call(             // Function call:
    func->getLabel(),                     //   Function address or Label.
    FuncSignature1<int, int>());          //   Function signature.

  call->setArg(0, x);                     // Assign `x` as the first argument and
  call->setRet(0, x);                     // assign `x` as a return value as well.

  cc.add(x, y);                           // Combine the return value with `y`.

  cc.bind(L_Exit);
  cc.ret(x);                              // Return `x`.
  cc.endFunc();                           // End of the function body.

  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> X86Compiler is no longer needed from here and can be destroyed <----

  Fibonacci fib;
  Error err = rt.add(&fib, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  printf("Fib(%u) -> %u\n, 8, fib(8));    // Test the generated code.

  rt.release(fib);                        // RAII, but let's make it explicit.
  return 0;
}
```

### Stack Management

**CodeCompiler** manages function's stack-frame, which is used by the register allocator to spill virtual registers. It also provides an interface to allocate user-defined block of the stack, which can be used as a temporary storage by the generated function. In the following example a stack of 256 bytes size is allocated, filled by bytes starting from 0 to 255 and then iterated again to sum all the values.

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

// Signature of the generated function.
typedef int (*Func)(void);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Runtime specialized for JIT code execution.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.

  X86Compiler cc(&code);                  // Create and attach X86Compiler to `code`.
  cc.addFunc(FuncSignature0<int>());      // Create a function that returns 'int'.

  X86Gp p = cc.newIntPtr("p");
  X86Gp i = cc.newIntPtr("i");

  X86Mem stack = c.newStack(256, 4);      // Allocate 256 bytes on the stack aligned to 4 bytes.
  X86Mem stackIdx(stack);                 // Copy of `stack` with `i` added.
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
  X86Gp sum = cc.newI32("sum");
  X86Gp val = cc.newI32("val");

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
  // ----> X86Compiler is no longer needed from here and can be destroyed <----

  Func fib;
  Error err = rt.add(&fib, &code);        // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  printf("Func() -> %d\n, func());        // Test the generated code.

  rt.release(fib);                        // RAII, but let's make it explicit.
  return 0;
}
```

### Constant Pool

**CodeCompiler** provides two constant pools for a general purpose code generation - local and global. Local constant pool is related to a single **CCFunc** node and is generally flushed after the function body, and global constant pool is flushed at the end of the generated code by **CodeCompiler::finalize()**.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

static void exampleUseOfConstPool(X86Compiler& cc) {
  cc.addFunc(FuncSignature0<int>());

  X86Gp v0 = cc.newGpd("v0");
  X86Gp v1 = cc.newGpd("v1");

  X86Mem c0 = cc.newInt32Const(kConstScopeLocal, 200);
  X86Mem c1 = cc.newInt32Const(kConstScopeLocal, 33);

  cc.mov(v0, c0);
  cc.mov(v1, c1);
  cc.add(v0, v1);

  cc.ret(v0);
  cc.endFunc();
}
```

### Code Injection

Both **CodeBuilder** and **CodeCompiler** emitters store their nodes in a double-linked list, which makes it easy to manipulate during the code generation or after it. Each node is always emitted next to the current **cursor** and the cursor is changed to that newly emitted node. Cursor can be explicitly retrieved and assigned by **getCursor()** and **setCursor()**, respectively.

The following example shows how to inject code at the beginning of the function by providing an **XmmConstInjector** helper class.

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>
#include <vector>

using namespace asmjit;

// Simple injector that injects `movaps()` to the beginning of the function.
class XmmConstInjector {
public:
  struct Slot {
    X86Xmm reg;
    Data128 value;
  };

  XmmConstInjector(X86Compiler* cc)
    : _cc(cc),
      _injectTarget(cc->getCursor()) {}

  X86Xmm xmmOf(const Data128& value) {
    // First reuse the register if it already holds the given `value`.
    for (std::vector<Slot>::const_iterator it(_slots.begin()); it != _slots.end(); ++it) {
      const Slot& slot = *it;
      if (::memcmp(&slot.value, &value, sizeof(Data128)) == 0)
        return slot.reg;
    }

    // Create a new register / value pair and store in `_slots`.
    X86Xmm reg = _cc->newXmm("const%u", static_cast<unsigned int>(_slots.size()));

    Slot newSlot;
    newSlot.value = value;
    newSlot.reg = reg;
    _slots.push_back(newSlot);

    // Create the constant and inject it after the injectTarget.
    X86Mem mem = _cc->newConst(kConstScopeGlobal, &value, 16);
    CBNode* saved = _cc->setCursor(_injectTarget);

    _cc->movaps(reg, mem);
    // Make sure we inject next load after the load we just emitted.
    _injectTarget = _cc->getCursor();

    // Restore the original cursor, so the code emitting can continue from where it was.
    _cc->setCursor(saved);
    return reg;
  }

  X86Compiler* _cc;
  CBNode* _injectTarget;
  std::vector<Slot> _slots;
};

// Signature of the generated function.
typedef void (*Func)(uint16_t*);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Runtime specialized for JIT code execution.

  FileLogger logger(stdout);

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.
  code.setLogger(&logger);

  X86Compiler cc(&code);                  // Create and attach X86Compiler to `code`.
  cc.addFunc(
    FuncSignature1<void, uint16_t*>());   // Create a function that accepts `uint16_t[]'.

  X86Gp p = cc.newIntPtr("p");            // Create and Assign the function argument `p`.
  cc.setArg(0, p);

  XmmConstInjector injector(&cc);         // The injector will inject the code |here|.

  X86Xmm x = cc.newXmm("x");
  cc.movups(x, x86::ptr(p));              // Load 16 bytes from `[p]` to `x`.

  // Now use injector to add constants to the constant pool and to inject their loads.
  Data128 data0 = Data128::fromU16(0x80);
  Data128 data1 = Data128::fromU16(0x13);

  cc.paddw(x, injector.xmmOf(data0));     // x.u16 = x.u16 + 0x80.
  cc.pmullw(x, injector.xmmOf(data1));    // x.u16 = x.u16 * 0x13.
  cc.movups(x86::ptr(p), x);              // Write back to `[p]`.

  cc.endFunc();                           // End of the function body.
  cc.finalize();                          // Translate and assemble the whole `cc` content.
  // ----> X86Compiler is no longer needed from here and can be destroyed <----

  Func func;
  Error err = rt.add(&func, &code);       // Add the generated code to the runtime.
  if (err) return 1;                      // Handle a possible error returned by AsmJit.
  // ----> CodeHolder is no longer needed from here and can be destroyed <----

  // Test the generated function
  uint16_t vec[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  func(vec);

  for (uint32_t i = 0; i < 8; i++)
    printf("%u ", vec[i]);
  printf("\n");

  rt.release(func);                       // RAII, but let's make it explicit.
  return 0;
}
```

The code generated would look similar to:

```x86asm
L0:
movaps xmm0, oword [L2]                 ; movaps const0, oword [L2]
movaps xmm1, oword [L2+16]              ; movaps const1, oword [L2+16]
movups xmm2, [rdi]                      ; movups x, [p]
paddw xmm2, xmm0                        ; paddw x, const0
pmullw xmm2, xmm1                       ; pmullw x, const1
movups [rdi], xmm2                      ; movups [p], x
L1:
ret
.align 16
L2:
.data 80008000800080008000800080008000
.data 13001300130013001300130013001300
```

There are many other applications of code injection, usually it's used to lazy-add some initialization code and such, but the application is practically unlimited.

Advanced Features
-----------------

### Logging

Failures are common, especially when working at machine-code level. AsmJit does already a good job with function overloading to prevent from emitting semantically incorrect instructions, but it can't prevent from emitting code that is semantically correct, but contains bugs. Logging has always been an important part of AsmJit's infrastructure and looking at logs could become handy when your code doesn't work as expected.

AsmJit's **Logger** provides the following:
  * Defines basic logging interface used by AsmJit,
  * Allows to reimplement its `Error _log(const char* str, size_t len)` function.
  * **FileLogger** implements logging into a C `FILE*` stream.
  * **StringLogger** implements logging into AsmJit's `StringBuilder`.

**Logger** also contains useful options that control the output and what should be logged:

  * **Logger::kOptionBinaryForm** - Output also binary representation of each instruction.
  * **Logger::kOptionImmExtended** - Output meaning of some immediate values.
  * **Logger::kOptionHexImmediate** - Display all immediates in hexadecimal.
  * **Logger::kOptionHexDisplacement** - Display all offsets in hexadecimal.

**Logger** is typically attached to **CodeHolder** and all attached code emitters automatically use it:

```c++
#include <asmjit/asmjit.h>
#include <stdio.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Runtime specialized for JIT code execution.
  FileLogger logger(stdout);              // Logger should always survive the CodeHolder.

  CodeHolder code;                        // Holds code and relocation information.
  code.init(rt.getCodeInfo());            // Initialize to the same arch as JIT runtime.
  code.setLogger(&logger);                // Attach the `logger` to `code` holder.

  // ... code as usual, everything you emit will be logged to `stdout` ...

  return 0;
}
```

### Error Handling

AsmJit uses error codes to represent and return errors. Every function where error can occur returns **Error**. Exceptions are never thrown by AsmJit even in extreme conditions like out-of-memory. Errors should never be ignored, however, checking errors after each asmjit API call would simply overcomplicate the whole code generation. To handle these errors AsmJit provides **ErrorHandler**, which contains **handleError()**:

    `virtual bool handleError(Error err, const char* message, CodeEmitter* origin) = 0;`

That can be overridden by AsmJit users and do the following:

  * 1. Return `true` or `false` from `handleError()`. If `true` is returned it means that error was handled and AsmJit can continue execution. The error code still be propagated to the caller, but the error origin (CodeEmitter) won't be put into an error state (last-error won't be set and `isInErrorState()` would return `true`). However, `false` reports to AsmJit that the error cannot be handled - in such case it stores the error, which can be retrieved later by `getLastError()`. Returning `false` is the default behavior when no error handler is provided. To put the assembler into a non-error state again `resetLastError()` must be called.
  * 2. Throw an exception. AsmJit doesn't use exceptions and is completely exception-safe, but you can throw exception from the error handler if this way is easier / preferred by you. Throwing an exception acts virtually as returning `true` - AsmJit won't store the error.
  * 3. Use plain old C's `setjmp()` and `longjmp()`. Asmjit always puts `Assembler` and `Compiler` to a consistent state before calling the `handleError()` so `longjmp()` can be used without issues to cancel the code-generation if an error occurred. This method can be used if exception handling in your project is turned off and you still want some comfort. In most cases it should be safe as AsmJit is based on Zone memory, so no memory leaks will occur if you jump back to a location where `CodeHolder` still exist.

**ErrorHandler** is simply attached to **CodeHolder** and will be used by every emitter attached to it. The first example uses error handler that just prints the error, but lets AsmJit continue:

```c++
// Error handling #1:
#include <asmjit/asmjit.h>

#include <stdio.h>

// Error handler that just prints the error and lets AsmJit ignore it.
class PrintErrorHandler : public asmjit::ErrorHandler {
public:
  // Return `true` to set last error to `err`, return `false` to do nothing.
  bool handleError(asmjit::Error err, const char* message, asmjit::CodeEmitter* origin) override {
    fprintf(stderr, "ERROR: %s\n", message);
    return false;
  }
};

int main(int argc, char* argv[]) {
  using namespace asmjit;

  JitRuntime rt;
  PrintErrorHandler eh;

  CodeHolder code;
  code.init(rt.getCodeInfo());
  code.setErrorHandler(&eh);

  // Try to emit instruction that doesn't exist.
  X86Assembler a(&code);
  a.emit(X86Inst::kIdMov, x86::xmm0, x86::xmm1);

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
    : error(err),
      message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

  asmjit::Error error;
  std::string message;
};

class ThrowErrorHandler : public asmjit::ErrorHandler {
public:
  // Throw is possible, functions that use ErrorHandler are never 'noexcept'.
  bool handleError(asmjit::Error err, const char* message, asmjit::CodeEmitter* origin) override {
    throw AsmJitException(err, message);
  }
};

int main(int argc, char* argv[]) {
  using namespace asmjit;

  JitRuntime rt;
  ThrowErrorHandler eh;

  CodeHolder code;
  code.init(rt.getCodeInfo());
  code.setErrorHandler(&eh);

  // Try to emit instruction that doesn't exist.
  try {
    X86Assembler a(&code);
    a.emit(X86Inst::kIdMov, x86::xmm0, x86::xmm1);
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

  virtual bool handleError(asmjit::Error err, const char* message, asmjit::CodeEmitter* origin) {
    this->err = err;
    longjmp(state, 1);
  }

  jmp_buf state;
  asmjit::Error err;
};

int main(int argc, char* argv[]) {
  using namespace asmjit;

  JitRuntime rt;
  LongJmpErrorHandler eh;

  CodeHolder code;
  code.init(rt.getCodeInfo());
  code.setErrorHandler(&eh);

  // Try to emit instruction that doesn't exist.
  X86Assembler a(&code);
  if (!setjmp(eh.state)) {
    a.emit(X86Inst::kIdMov, x86::xmm0, x86::xmm1);
  }
  else {
    Error err = eh.err;
    printf("ASMJIT ERROR: 0x%08X [%s]\n", err, DebugUtils::errorAsString(err));
  }

  return 0;
}
```

### TODO

...More documentation...

Support
-------

Please consider a donation if you use the project and would like to keep it active in the future.

  * [Donate by PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=QDRM6SRNG7378&lc=EN;&item_name=asmjit&currency_code=EUR)

Received From:

  * [PELock - Software copy protection and license key system](https://www.pelock.com)

Authors & Maintainers
---------------------

  * Petr Kobalicek <kobalicek.petr@gmail.com>
