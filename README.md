AsmJit
------

Complete x86/x64 JIT and Remote Assembler for C++.

  * [Official Repository (asmjit/asmjit)](https://github.com/asmjit/asmjit)
  * [Official Blog (asmbits)](https://asmbits.blogspot.com/ncr)
  * [Official Chat (gitter)](https://gitter.im/asmjit/asmjit)
  * [Permissive ZLIB license](./LICENSE.md)

'NEXT' TODO:
  * This README contains outdated code and is not complete.
  * Only the first section works atm.
  * AVX512 {sae} and {er} not supported yet.

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
  Error err = rt.add(&fn, &code);         // Add the code generated to the runtime.
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

  * Breaking changes are described in [BREAKING.md](./BREAKING.md)

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
  * **ASMJIT_RELEASE** - Define to always turn debugging off (regardless of compiler-time options detected).
  * **ASMJIT_TRACE** - Define to enable AsmJit tracing. Tracing is used to catch bugs in AsmJit and it has to be enabled explicitly. When AsmJit is compiled with `ASMJIT_TRACE` it uses `stdout` to log information related to AsmJit execution. This log can be helpful AsmJit developers and users in case something goes wrong..

By default none of these is defined, AsmJit detects build-type based on compile-time macros and supports most IDE and compiler settings out of box.

### Build Mode:

  * **ASMJIT_STATIC** - Define to build AsmJit as a static library. No symbols are exported in such case.
  * **ASMJIT_EMBED** - Define to embed AsmJit in another project. Embedding means that neither shared nor static library is created and AsmJit's source files and source files of the product that embeds AsmJit are part of the same target. This way of building AsmJit has certain advantages that are beyond this manual. **ASMJIT_EMBED** behaves identically to **ASMJIT_STATIC** (no API exports).

By default AsmJit build is configured to be built as a shared library, thus note of **ASMJIT_EMBED** and **ASMJIT_STATIC** is defined.

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
  * **ASMJIT_DISABLE_VALIDATION** - Disables instruction validation. Saves around 5kB of space when used.

NOTE: Please don't disable any features if you plan to build AsmJit as a shared library that will be used by multiple projects that you don't control (for example asmjit in a Linux distribution). The possibility to disable certain features exists mainly for static builds of AsmJit.

Using AsmJit
------------

AsmJit library uses one global namespace called `asmjit` that provides the whole functionality. Architecture specific code is prefixed by the architecture name and architecture specific registers and operand builders have their own namespace. For example API targeting both X86 and X64 architectures is prefixed with `X86` and registers & operand builders are accessible through `x86` namespace. This design is very different from the initial version of AsmJit and it seems now as the most convenient one.

### CodeHolder & CodeEmitter

AsmJit provides two classes that are used together for code generation:

  * **CodeHolder** - Provides functionality to hold generated code and stores all necessary information about code sections, labels, symbols, and possible relocations.
  * **CodeEmitter** - Provides functionality to emit code into `CodeHolder`. `CodeEmitter` is abstract and provides just basic building blocks that are then implemented by `Assembler`, `CodeBuilder`, and `CodeCompiler`.

Code emitters:

  * **Assembler** - Emitter designed to emit machine code directly.
  * **CodeBuilder** - Emitter designed to emit code into a representation that can be processed. It stores the whole code in a double linked list consisting of nodes (`CBNode` aka code-builder node). There are nodes that represent instructions (`CBInst`), labels (`CBLabel`), and other building blocks (`CBAlign`, `CBData`, ...). Some nodes are used as markers (`CBSentinel`) and comments (`CBComment`).
  * **CodeCompiler** - High-level code emitter that uses virtual registers and contains high-level function building features. `CodeCompiler` is based on `CodeBuilder`, but extends its functionality and introduces new node types starting with CC (`CCFunc`, `CCFuncRet`, `CCFuncCall`). `CodeCompiler` is the simplest way to start with AsmJit as it abstracts many details required to generate a function in asm language.

### Runtime

AsmJit's `Runtime` is designed for execution and/or linking. The `Runtime` itself is abstract and defines only how to `add` and `release` code held by `CodeHolder`. `CodeHolder` holds machine code and relocation entries, but should be seen as a temporary object only - after the code in `CodeHolder` is ready, it should be passed to `Runtime` or relocated manually. Users interested in inspecting the generated machine-code (instead of executing or linking) can keep it in `CodeHodler` and process it manually of course.

The only `Runtime` implementation provided directly by AsmJit is called `JitRuntime`, which is suitable for storing and executing dynamically generated code. JitRuntime is used in most AsmJit examples as it makes the code management easy. It allows to add and release dynamically generated functions, so it's suitable for JIT code generators that want to keep many functions alive, and release functions which are no longer needed.

### Instructions & Operands

Instructions specify operations performed by the CPU, and operands specify the operation's input(s) and output(s). Each AsmJit's instruction has it's own unique id (`X86Inst::Id` for example) and platform specific code emitters always provide a type safe intrinsic (or multiple overloads) to emit such instruction. There are two ways of emitting an instruction:

  * Using emitter.**instName**(operands...) - A type-safe way provided by platform specific emitters - for example `X86Assembler` provides `mov(X86Gp, X86Gp)`.
  * Using emitter.emit(**instId**, operands...) - Allows to emit an instruction in a dynamic way - you just need to know instruction's id and provide its operands.

AsmJit's operands all inherit from a base class called `Operand` and then specialize its type to:

  * **None** (not used or uninitialized operand).
  * **Register** (**Reg**) - Describes either physical or virtual register. Physical registers have id that matches the target's machine id directly, whereas virtual registers must be allocated into physical registers by a register allocator pass. Each `Reg` provides:
    * *Register Type* - Unique id that describes each possible register provided by the target architecture - for example X86 backend provides `X86Reg::RegType`, which defines all variations of general purpose registers (GPB-LO, GPB-HI, GPW, GPD, and GPQ) and all types of other registers like K, MM, BND, XMM, YMM, and ZMM.
    * *Register Kind* - Groups multiple register types under a single kind - for example all general-purpose registers (of all sizes) on X86 are `X86Reg::kKindGp`, all SIMD registers (XMM, YMM, ZMM) are `X86Reg::kKindVec`, etc.
    * *Register Size* - Contains the size of the register in bytes. If the size depends on the mode (32-bit vs 64-bit) then generally the higher size is used (for example RIP register has size 8 by default).
    * *Register ID* - Contains physical or virtual id of the register.
  * **Memory Address** (**Mem**) - Used to reference a memory location. Each `Mem` provides:
    * *Base Register* - A base register id (physical or virtual).
    * *Index Register* - An index register id (physical or virtual).
    * *Offset* - Displacement or absolute address to be referenced (32-bit if base register is used and 64-bit if base register is not used).
    * *Flags* that can describe various architecture dependent information (like scale and segment-override on X86).
  * **Immediate Value** (**Imm**) - Immediate values are usually part of instructions (encoded within the instruction itself) or data.
  * **Label** - used to reference a location in code or data. Labels must be created by the `CodeEmitter` or by `CodeHolder`. Each label has its unique id per `CodeHolder` instance.

AsmJit allows to construct operands dynamically, to store them, and to query a complete information about them at run-time. Operands are small (always 16 bytes per **Operand**) and should be always copied if you intend to store them (don't create operands by using `new` keyword, it's not recommended). Operands are safe to be `memcpy()`ed and `memset()`ed if you work with arrays of operands.

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
    a.mov(arr, ptr(x86::esp, 4));         // Fetch first argument from [ESP + 4].
    a.mov(cnt, ptr(x86::esp, 8));         // Fetch second argument from [ESP + 8].
  }

  Label Loop = a.newLabel();              // To construct the loop, we need some labels.
  Label Exit = a.newLabel();

  a.xor_(sum, sum);                       // Clear 'sum' register (shorter than 'mov').
  a.test(cnt, cnt);                       // Border case:
  a.jz(Exit);                             //   If 'cnt' is zero jump to 'Exit' now.

  a.bind(Loop);                           // Start of a loop iteration.
  a.add(sum, dword_ptr(arr));             // Add int at [arr] to 'sum'.
  a.add(arr, 4);                          // Increment 'arr' pointer.
  a.dec(cnt);                             // Decrease 'cnt'.
  a.jnz(Loop);                            // If not zero jump to 'Loop'.

  a.bind(Exit);                           // Exit to handle the border case.
  a.ret();                                // Return from function ('sum' == 'eax').
  // ----> X86Assembler is no longer needed from here and can be destroyed <----

  SumFunc fn;
  Error err = rt.add(&fn, &code);         // Add the code generated to the runtime.

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
X86Mem a = dword_ptr(rax, rbx);           // dword ptr [rax + rbx].
X86Mem b = qword_ptr(rdx, rsi, 0, 1);     // qword ptr [rdx + rsi << 0 + 1].
```

Memory operands provide API that can be used to work with them:

```c++
X86Mem mem = dword_ptr(rax, 12);          // dword ptr [rax + 12].

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
using namespace asmjit::x86;

X86Assembler a(...);                      // Your initialized X86Assembler.
X86Mem m = ptr(eax);                      // Construct [eax] memory operand.

// One way of emitting bunch of loads is to use `mem.adjusted()`. It returns
// a new memory operand and keeps the source operand unchanged.
a.movaps(xmm0, m);                        // No adjustment needed to load [eax].
a.movaps(xmm1, m.adjusted(16));           // Loads [eax + 16].
a.movaps(xmm2, m.adjusted(32));           // Loads [eax + 32].
a.movaps(xmm3, m.adjusted(48));           // Loads [eax + 48].

// ... do something with xmm0-3 ...

// Another way of adjusting memory is to change the operand in-place. If you
// want to keep the original operand you can simply clone it.
X86Mem mx = m.clone();
a.movaps(mx, xmm0); mx.addOffset(16);     // Stores [eax]      (and adds 16 to mx).
a.movaps(mx, xmm1); mx.addOffset(16);     // Stores [eax + 16] (and adds 16 to mx).
a.movaps(mx, xmm2); mx.addOffset(16);     // Stores [eax + 32] (and adds 16 to mx).
a.movaps(mx, xmm3);                       // Stores [eax + 48].
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
  using namespace asmjit::x86;            // Easier access to x86/x64 registers.

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
    dst   = eax;
    src_a = ecx;
    src_b = edx;
    a.mov(dst  , dword_ptr(esp, 4));      // Load the destination pointer.
    a.mov(src_a, dword_ptr(esp, 8));      // Load the first source pointer.
    a.mov(src_b, dword_ptr(esp, 12));     // Load the second source pointer.
  }
  else {
    dst   = isWin ? rcx : rdi;            // First argument  (destination pointer).
    src_a = isWin ? rdx : rsi;            // Second argument (source 'a' pointer).
    src_b = isWin ? r8  : rdx;            // Third argument  (source 'b' pointer).
  }

  a.movdqu(xmm0, ptr(src_a));             // Load 4 ints from [src_a] to XMM0.
  a.movdqu(xmm1, ptr(src_b));             // Load 4 ints from [src_b] to XMM1.
  a.paddd(xmm0, xmm1);                    // Add 4 ints in XMM1 to XMM0.
  a.movdqu(ptr(dst), xmm0);               // Store the result to [dst].
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

  // This code uses AsmJit's ptr_cast<> to cast between void* and SumIntsFunc.
  ptr_cast<SumIntsFunc>(p)(result, arr_a, arr_b);

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
  Error err = rt.add(&fn, &code);         // Add the code generated to the runtime.
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
  * AsmJit allows to override the default decision by using `short_()` and `long()_` instruction options to force short or long form, respectively. The most useful is `long_()` as it basically forces AsmJit to always emit the long form. The `short_()` is not that useful as it's automatic (except jumps to non-bound labels). Note the underscore after each function name as it avoids collision with built-in C++ types.

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
  Error err = rt.add(&fn, &code);         // Add the code generated to the runtime.
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

### Using Func-API

So far all examples shown above handled creating function prologs and epilogs manually. While it's possible to do it that way it's much better to automate such process as function calling conventions vary across architectures and also across operating systems.

AsmJit contains a functionality that can be used to define function signatures and to calculate automatically optimal frame layout that can be used directly by a prolog and epilog inserter. This feature was exclusive to AsmJit's CodeCompiler for a very long time, but was abstracted out and is now available for all users regardless of CodeEmitter they use. The design of handling functions prologs and epilogs allows generally two use cases:

  * Calculate function layout before the function is generated - this is the only way if you use pure `Assembler` emitter and shown in the next example.
  * Calculate function layout after the function is generated - this way is generally used by `CodeBuilder` and `CodeCompiler` (will be described together with `X86Compiler`).

The following concepts are used to describe and create functions in AsmJit:

  * **CallConv** - Describes a calling convention - this class contains instructions to assign registers and stack addresses to function arguments and return value(s), but doesn't specify any function signature. Calling conventions are architecture and OS dependent.

  * **TypeId** - TypeId is an 8-bit value that describes a platform independent type. It provides abstractions for most common types like `int8_t`, `uint32_t`, `uintptr_t`, `float`, `double`, and all possible vector types to match ISAs up to AVX512. TypeId was introduced originally for CodeCompiler, but is also used by FuncSignature.

  * **FuncSignature** - Describes a function signature, for example `int func(int, int)`. FuncSignature contains a function calling convention id, return value type, and function arguments. The signature itself is platform independent and uses TypeId to describe types of function arguments and its return value(s).

  * **FuncDetail** - Architecture and ABI dependent information that describes CallConv and expanded FuncSignature. Each function argument and return value is represented as **FuncDetail::Value** that contains the original TypeId enriched by additional information that specifies if the value is passed/returned by register (and which register) or by stack. Each value also contains some other metadata that provide additional information required to handle it properly (for example if a vector value is passed indirectly by a pointer as required by WIN64 calling convention, etc...).

  * **FuncArgsMapper** - A helper that can be used to define where each function argument is expected to be. It's architecture and ABI dependent mapping from function arguments described by CallConv and FuncDetail into registers specified by the user.

  * **FuncFrameInfo** - Contains information about a function-frame. Holds callout-stack size and alignment (i.e. stack used to call functions), stack-frame size and alignment (the stack required by the function itself), and various attributes that describe how prolog and epilog should be constructed. FuncFrameInfo doesn't know anything about function arguments or returns, it should be seen as a class that describes minimum requirements of the function frame and its attributes before the final `FuncFrameLayout` is calculated.

  * **FuncFrameLayout** - Contains the final function layout that can be passed to `FuncUtils::emitProlog()` and `FuncUtils::emitEpilog()`. The content of this class should always be calculated by AsmJit by calling `FuncFrameLayout::init(const FuncDetail& detail, const FuncFrameInfo& ffi)`.

It's a lot of concepts where each represents one step in the function layout calculation. In addition, the whole machinery can also be used to create function calls, instead of function prologs and epilogs. The next example shows how AsmJit can be used to create functions for both 32-bit and 64-bit targets and various calling conventions:

```c++
using namespace asmjit;

typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

int main(int argc, char* argv[]) {
  JitRuntime rt;                          // Create JIT Runtime

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
  Error err = rt.add(&fn, &code);         // Add the code generated to the runtime.
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

CodeBuilder & CodeCompiler
--------------------------

Both **CodeBuilder** and **CodeCompiler** are emitters that emit everything to a representation that allows further processing. The code stored in such representation is completely safe to be patched, simplified, reordered, obfuscated, removed, injected, analyzed, and 'think-of-something-else'. Each instruction (or label, directive, ...) is stored as **CBNode** (Code-Builder Node) and contains all the necessary information to emit machine code out of it later.

There is a difference between **CodeBuilder** and **CodeCompiler**:

  * **CodeBuilder** (low-level):
    * Maximum compatibility with *Assembler**, easy to switch from **Assembler** to **CodeBuilder** and vice versa.
    * Doesn't generate machine code directly, allows to serialize to **Assembler** when the whole code is ready to be encoded.

  * **CodeCompiler** (high-level):
    * Virtual registers - allows to use unlimited number of virtual registers which are allocated into physical registers by a built-in register allocator.
    * Function nodes - allows to create functions by specifying their signatures and assigning virtual registers to function arguments and return value(s).
    * Function calls - allows to call other functions withing the generated code by using the same interface for defining function signatures.

There are multiple node types used by both:

  * Basic Nodes:
    * **CBNode** - Base class for all nodes.
    * **CBInst** - Instruction node.
    * **CBAlign** - Alignment directive (.align).
    * **CBLabel** - Bound label.

  * Data Nodes:
    * **CBData** - Data embedded into the code.
    * **CBConstPool** - Constant pool data.
    * **CBLabelData** - Label address embedded as data.

  * Informative Nodes:
    * **CBComment** - Contains a comment string, doesn't affect code generation.
    * **CBSentinel** - A marker that can be used to remember certain position, doesn't affect code generation.

  * **CodeCompiler** Nodes:
    * **CCFunc** - Start of a function.
    * **CCFuncRet** - Return from a function.
    * **CCFuncCall* - Function call.

### Using CodeBuilder








### TODO

Documentation not updated from here...













### Function Signature

AsmJit needs to know the prototype of the function it will generate or call. AsmJit contains a mapping between a type and the register that will be used to represent it. To make life easier there is a function builder that does the mapping on the fly. Function builder is a template class that helps with creating a function prototype by using native C/C++ types that describe function arguments and return value. It translates C/C++ native types into AsmJit specific IDs and makes these IDs accessible to Compiler.

### Putting It All Together

Let's put all together and generate a first function that sums its two arguments and returns the result. At the end the generated function is called from a C++ code.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  // Create JitRuntime.
  JitRuntime rt;

  // Create X86Assembler and X86Compiler.
  X86Assembler a(&rt);
  X86Compiler c(&a);

  // Build function having two arguments and a return value of type 'int'.
  // First type in function builder describes the return value. kCallConvHost
  // tells the compiler to use the host calling convention.
  c.addFunc(FuncBuilder2<int, int, int>(kCallConvHost));

  // Create 32-bit virtual registers and assign some names to them. Using names
  // is not necessary, however, it can make debugging easier as these show in
  // annotations, if turned-on.
  X86Gp x = c.newI32("x");
  X86Gp y = c.newI32("y");

  // Tell asmjit to use these variables as function arguments.
  c.setArg(0, x);
  c.setArg(1, y);

  // x = x + y;
  c.add(x, y);

  // Tell asmjit to return `x`.
  c.ret(x);

  // Finalize the current function.
  c.endFunc();

  // Now the Compiler contains the whole function, but the code is not yet
  // generated. To tell the compiler to serialize the code to `Assembler`
  // `c.finalize()` has to be called. After finalization the `Compiler`
  // won't contain the code anymore and will be detached from the `Assembler`.
  c.finalize();

  // After finalization the code has been send to `Assembler`. It contains
  // a handy method `make()`, which returns a pointer that points to the
  // first byte of the generated code, which is the function entry in our
  // case.
  void* funcPtr = a.make();

  // In order to run 'funcPtr' it has to be casted to the desired type.
  // Typedef is a recommended and safe way to create a function-type.
  typedef int (*FuncType)(int, int);

  // Using asmjit_cast is purely optional, it's basically a C-style cast
  // that tries to make it visible that a function-type is returned.
  FuncType func = asmjit_cast<FuncType>(funcPtr);

  // Finally, run it and do something with the result...
  int z = func(1, 2);
  printf("z=%d\n", z); // Outputs "z=3".

  // The function will remain in memory after Compiler and Assembler are
  // destroyed. This is why the `JitRuntime` is used - it keeps track of
  // the code generated. When `Runtime` is destroyed it also invalidates
  // all code relocated by it (which is in our case also our `func`). So
  // it's safe to just do nothing in our case, because destroying `Runtime`
  // will free `func` as well, however, it's always better to release the
  // generated code that is not needed anymore manually.
  rt.release((void*)func);

  return 0;
}
```

The code should be self explanatory, however there are some details to be clarified.

The code above generates and calls a function of `kCallConvHost` calling convention. 32-bit architecture contains a wide range of function calling conventions that can be all used by a single program, so it's important to know which calling convention is used by your C/C++ compiler so you can call the function. However, most compilers should generate CDecl by default. In 64-bit mode there are only two calling conventions, one is specific for Windows (Win64 calling convention) and the other for Unix (AMD64 calling convention). The `kCallConvHost` is defined to be one of CDecl, Win64 or AMD64 depending on your architecture and operating system.

Default integer size is platform specific, virtual types `kVarTypeIntPtr` and `kVarTypeUIntPtr` can be used to make the code more portable and they should be always used when a pointer type is needed. When no type is specified AsmJit always defaults to `kVarTypeIntPtr`. The code above works with integers where the default behavior has been overidden to 32-bits. Note it's always a good practice to specify the type of the variable used. Alternative form of creating a variable is `c.newGpVar(...)`, `c.newMmVar(...)`, `c.newXmmVar` and so on...

The function starts with `c.addFunc()` and ends with `c.endFunc()`. It's not allowed to put code outside of the function; however, embedding data outside of the function body is allowed.

### Using Labels

Labels are essential for making jumps, function calls or to refer to a data that is embedded in the code section. Label has to be explicitly created by using `newLabel()` method of your code generator in order to be used. The following example executes a code that depends on the condition by using a `Label` and conditional jump instruction. If the first parameter is zero it returns `a + b`, otherwise `a - b`.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime rt;

  X86Assembler a(&rt);
  X86Compiler c(&a);

  // This function uses 3 arguments - `int func(int, int, int)`.
  c.addFunc(FuncBuilder3<int, int, int, int>(kCallConvHost));

  X86Gp op = c.newI32("op");
  X86Gp x = c.newI32("x");
  X86Gp y = c.newI32("y");

  c.setArg(0, op);
  c.setArg(1, x);
  c.setArg(2, y);

  // Create labels.
  Label L_Sub = c.newLabel();
  Label L_Skip = c.newLabel();

  // If (op != 0)
  //   goto L_Sub;
  c.test(op, op);
  c.jne(L_Sub);

  // x = x + y;
  // goto L_Skip;
  c.add(x, y);
  c.jmp(L_Skip);

  // L_Sub:
  // x = x - y;
  c.bind(L_Sub);
  c.sub(x, y);

  // L_Skip:
  c.bind(L_Skip);

  c.ret(x);
  c.endFunc();
  c.finalize();

  // The prototype of the generated function.
  typedef int (*FuncType)(int, int, int);
  FuncType func = asmjit_cast<FuncType>(a.make());

  int res0 = func(0, 1, 2);
  int res1 = func(1, 1, 2);

  printf("res0=%d\n", res0); // Outputs "res0=3".
  printf("res1=%d\n", res1); // Outputs "res1=-1".

  rt.release((void*)func);
  return 0;
}
```

In this example conditional and unconditional jumps were used with labels together. Labels have to be created explicitely by `Compiler` by using a `Label L = c.newLabel()` form. Each label as an unique ID that identifies it, however it's not a string and there is no way to query for a `Label` instance that already exists at the moment. Label is like any other operand moved by value, so the copy of the label will still reference the same label and changing a copied label will not change the original label.

Each label has to be bound to the location in the code by using `bind()`; however, it can be bound only once! Trying to bind the same label multiple times has undefined behavior - assertion failure is the best case.

### Memory Addressing

X86/X64 architectures have several memory addressing modes which can be used to combine base register, index register and a displacement. In addition, index register can be shifted by a constant from 1 to 3 that can help with addressing elements up to 8-byte long in an array. AsmJit supports all forms of memory addressing. Memory operand can be created by using `asmjit::X86Mem` or by using related non-member functions like `asmjit::x86::ptr`. Use `ptr` to create a memory operand having a base register with optional index register and a displacement; use and `ptr_abs` to create a memory operand referring to an absolute address in memory (32-bit) and optionally having an index register.

In the following example various memory addressing modes are used to demonstrate how to construct and use them. It creates a function that accepts an array and two indexes which specify which elements to sum and return.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime rt;

  X86Assembler a(&rt);
  X86Compiler c(&a);

  // Function returning 'int' accepting pointer and two indexes.
  c.addFunc(FuncBuilder3<int, const int*, intptr_t, intptr_t>(kCallConvHost));

  X86Gp p = c.newIntPtr("p");
  X86Gp xIndex = c.newIntPtr("xIndex");
  X86Gp yIndex = c.newIntPtr("yIndex");

  c.setArg(0, p);
  c.setArg(1, xIndex);
  c.setArg(2, yIndex);

  X86Gp x = c.newI32("x");
  X86Gp y = c.newI32("y");

  // Read `x` by using a memory operand having base register, index register
  // and scale. Translates to `mov x, dword ptr [p + xIndex << 2]`.
  c.mov(x, x86::ptr(p, xIndex, 2));

  // Read `y` by using a memory operand having base register only. Registers
  // `p` and `yIndex` are both modified.

  // Shift bIndex by 2 (exactly the same as multiplying by 4).
  // And add scaled 'bIndex' to 'p' resulting in 'p = p + bIndex * 4'.
  c.shl(yIndex, 2);
  c.add(p, yIndex);

  // Read `y`.
  c.mov(y, x86::ptr(p));

  // x = x + y;
  c.add(x, y);

  c.ret(x);
  c.endFunc();
  c.finalize();

  // The prototype of the generated function.
  typedef int (*FuncType)(const int*, intptr_t, intptr_t);
  FuncType func = asmjit_cast<FuncType>(a.make());

  // Array passed to `func`.
  static const int array[] = { 1, 2, 3, 5, 8, 13 };

  int xVal = func(array, 1, 2);
  int yVal = func(array, 3, 5);

  printf("xVal=%d\n", xVal); // Outputs "xVal=5".
  printf("yVal=%d\n", yVal); // Outputs "yVal=18".

  rt.release((void*)func);
  return 0;
}
```

### Using Stack

AsmJit uses stack automatically to spill virtual registers if there is not enough physical registers to keep them all allocated. The stack frame is managed by `Compiler` that provides also an interface to allocate chunks of memory of user specified size and alignment.

In the following example a stack of 256 bytes size is allocated, filled by bytes starting from 0 to 255 and then iterated again to sum all the values.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime rt;

  X86Assembler a(&rt);
  X86Compiler c(&a);

  // Function returning 'int' without any arguments.
  c.addFunc(FuncBuilder0<int>(kCallConvHost));

  // Allocate 256 bytes on the stack aligned to 4 bytes.
  X86Mem stack = c.newStack(256, 4);

  X86Gp p = c.newIntPtr("p");
  X86Gp i = c.newIntPtr("i");

  // Load a stack address to `p`. This step is purely optional and shows
  // that `lea` is useful to load a memory operands address (even absolute)
  // to a general purpose register.
  c.lea(p, stack);

  // Clear `i`. Notice that `xor_()` is used instead of `xor()` as it's keyword.
  c.xor_(i, i);

  Label L1 = c.newLabel();
  Label L2 = c.newLabel();

  // First loop, fill the stack allocated by a sequence of bytes from 0 to 255.
  c.bind(L1);

  // Mov byte ptr[p + i], i.
  //
  // Any operand can be cloned and modified. By cloning `stack` and calling
  // `setIndex()` we created a new memory operand based on stack having an
  // index register assigned to it.
  c.mov(stack.clone().setIndex(i), i.r8());

  // if (++i < 256)
  //   goto L1;
  c.inc(i);
  c.cmp(i, 256);
  c.jb(L1);

  // Second loop, sum all bytes stored in `stack`.
  X86Gp sum = c.newI32("sum");
  X86Gp val = c.newI32("val");

  c.xor_(i, i);
  c.xor_(sum, sum);

  c.bind(L2);

  // Movzx val, byte ptr [stack + i]
  c.movzx(val, stack.clone().setIndex(i).setSize(1));
  // sum += val;
  c.add(sum, val);

  // if (++i < 256)
  //   goto L2;
  c.inc(i);
  c.cmp(i, 256);
  c.jb(L2);

  c.ret(sum);
  c.endFunc();
  c.finalize();

  typedef int (*FuncType)(void);
  FuncType func = asmjit_cast<FuncType>(a.make());

  printf("sum=%d\n", func()); // Outputs "sum=32640".

  rt.release((void*)func);
  return 0;
}
```

### Built-In Logging

Failures are common when working at machine-code level. AsmJit does already a good job with function overloading to prevent from emitting semantically incorrect instructions; however, AsmJit can't prevent from emitting code that is semantically correct, but contains bug(s). Logging has always been an important part of AsmJit's infrastructure and the output can be very valuable after something went wrong.

AsmJit contains extensible logging interface defined by `Logger` and specialized by `FileLogger` and `StringLogger` classes (you can create your own as well). `FileLogger` can log into a standard C-based `FILE*` stream while `StringLogger` logs into an internal buffer that can be used after the code generation is done.

After a logger is attached to `Assembler` its methods will be called every time `Assembler` emits something. A single `Logger` instance can be used multiple times, however, loggers that contain state(s) like `StringLogger` must not be used by two or more threads at the same time.

The following snippet describes how to log into `FILE*`:

```c++
// Create logger logging to `stdout`. Logger life-time should always be
// greater than a life-time of the code generator. Alternatively the
// logger can be reset before it's destroyed.
FileLogger logger(stdout);

// Create runtime and assembler and attach the logger to the assembler.
JitRuntime rt;
X86Assembler a(&rt);
a.setLogger(&logger);

// ... Generate the code ...
```

The following snippet describes how to log into a string:

```c++
StringLogger logger;

JitRuntime rt;
X86Assembler a(&rt);
a.setLogger(&logger);

// ... Generate the code ...

printf("Logger Content:\n%s", logger.getString());

// You can use `logger.clearString()` if the intend is to reuse the logger.
```

Logger can be configured to show more information by using `logger.addOptions()` method. The following options are available:

  * `Logger::kOptionBinaryForm` - Output instructions also in binary form.
  * `Logger::kOptionHexImmediate` - Output constants in hexadecimal form.
  * `Logger::kOptionHexDisplacement` - Output displacements in hexadecimal form.

### Error Handling

AsmJit uses error codes to represent and return errors; and every function where error can occur will also return it. It's recommended to only check errors of the most important functions and to write a custom `ErrorHandler` to handle the rest. An error can happen in many places, but error handler is mostly used by `Assembler` to report a fatal problem. There are multiple ways of using `ErrorHandler`:

  * 1. Returning `true` or `false` from `handleError()`. If `true` is returned it means that error was handled and AsmJit can continue execution. The error code still be propagated to the caller, but won't put the origin into an error state (it won't set last-error). However, `false` reports to AsmJit that the error cannot be handled - in such case it stores the error, which can retrieved later by `getLastError()`. Returning `false` is the default behavior when no error handler is provided. To put the assembler into a non-error state again the `resetLastError()` must be called.
  * 2. Throwing an exception. AsmJit doesn't use exceptions and is completely exception-safe, but you can throw exception from the error handler if this way is easier / preferred by you. Throwing an exception acts virtually as returning `true` - AsmJit won't store the error.
  * 3. Using plain old C's `setjmp()` and `longjmp()`. Asmjit always puts `Assembler` and `Compiler` to a consistent state before calling the `handleError()` so `longjmp()` can be used without issues to cancel the code-generation if an error occurred.

Here is an example of using `ErrorHandler` to just print the error but do nothing else:

```c++
// Error handling #1:
#include <asmjit/asmjit.h>

using namespace asmjit;

class MyErrorHandler : public ErrorHandler {
public:
  // Return `true` to set last error to `err`, return `false` to do nothing.
  // The `origin` points to the `X86Assembler` instance (&a in our case).
  virtual bool handleError(Error err, const char* message, void* origin) {
    printf("ASMJIT ERROR: 0x%08X [%s]\n", err, message);
    return false;
  }
};

int main(int argc, char* argv[]) {
  JitRuntime rt;
  MyErrorHandler eh;

  X86Assembler a(&rt);
  a.setErrorHandler(&eh);

  // Use RAW emit to emit an illegal instruction.
  Error err = a.emit(X86Inst::kIdMov, x86::eax, x86::xmm4, x86::xmm1);

  // Message printed, the error contains the same error as passed to the
  // error handler. Since we returned `false` the assembler is in an error
  // state. Use `resetLastError` to reset it back to normal.
  assert(a.getLastError() == err);
  a.resetLastError();

  // After the error is reset it should return `kErrorOk` like nothing happened.
  assert(a.getLastError() == kErrorOk);

  return 0;
}
```

NOTE: If error happens during instruction emitting / encoding the assembler behaves transactionally - the output buffer won't advance if failed, thus a fully encoded instruction is either emitted or not. AsmJit is very safe and strict in this regard. The error handling shown above is useful, but it's still not the best way of dealing with errors in AsmJit. The following example shows how to use exception handling to handle errors in a safe way:

```c++
// Error handling #2:
#include <asmjit/asmjit.h>
#include <exception>

using namespace asmjit;

class AsmJitException : public std::exception {
public:
  AsmJitException(Error err, const char* message) noexcept
    : error(err),
    : message(message) {}

  Error error;
  std::string message;
}

class MyErrorHandler : public ErrorHandler {
public:
  // `origin` points to the `X86Assembler` instance (&a) in our case.
  virtual bool handleError(Error err, const char* message, void* origin) {
    throw AsmJitException(err, message);
  }
};

int main(int argc, char* argv[]) {
  JitRuntime rt;
  MyErrorHandler eh;

  X86Assembler a(&rt);
  a.setErrorHandler(&eh);

  try {
    // This will call `eh.handleError()`, which will throw.
    a.emit(X86Inst::kIdMov, x86::eax, x86::xmm4, x86::xmm1);
  }
  catch (const AsmJitException& ex) {
    printf("ASMJIT ERROR: 0x%08X [%s]\n", ex.error, ex.message.c_str());
  }
}
```

If C++ exceptions are not what you like or your project turns off them completely there is still a way of reducing the error handling to a minimum by using a standard `setjmp/longjmp` pair. AsmJit is exception-safe and doesn't use RAII for resource management internally, so you can just jump from the error handler without causing any side-effects or memory leaks. The following example demonstrates how to do that:

```c++
// Error handling #2:
#include <asmjit/asmjit.h>
#include <setjmp.h>

using namespace asmjit;

class MyErrorHandler : public ErrorHandler {
public:
  inline bool init() noexcept {
    return setjmp(_state) == 0;
  }

  virtual bool handleError(Error err, const char* message, void* origin) {
    longjmp(_state, 1);
  }

  jmp_buf _state;
};

int main(int argc, char* argv[]) {
  JitRuntime rt;
  MyErrorHandler eh;

  X86Assembler a(&rt);
  a.setErrorHandler(&eh);

  if (eh.init()) {
    // This will call `eh.handleError()`, which will call `longjmp()`.
    a.emit(X86Inst::kIdMov, x86::eax, x86::xmm4, x86::xmm1);
  }
  else {
    Error err = a.getLastError();
    printf("ASMJIT ERROR: 0x%08X [%s]\n", err, DebugUtils::errorAsString(err));
  }
}
```

### Using Constant Pool

To be documented.

### Code Injection

Code injection was one of key concepts of Compiler from the beginning. Compiler records all emitted instructions in a double-linked list which can be manipulated before `make()` is called. Any call to Compiler that adds instruction, function or anything else in fact manipulates this list by inserting nodes into it.

To manipulate the current cursor use Compiler's `getCursor()` and `setCursor()` methods. The following snippet demonstrates the proper way of code injection.

```c++
X86Compiler c(...);

X86Gp x = c.newI32("x");
X86Gp y = c.newI32("y");

AsmNode* here = c.getCursor();
c.mov(y, 2);

// Now, `here` can be used to inject something before `mov y, 2`. To inject
// something it's always good to remember the current cursor so it can be set
// back after the injecting is done. When `setCursor()` is called it returns
// the old cursor to be remembered.
AsmNode* prev = c.setCursor(here);
c.mov(x, 1);
c.setCursor(prev);
```

The resulting code would look like:

```
c.mov(x, 1);
c.mov(y, 2);
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
