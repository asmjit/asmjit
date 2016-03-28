AsmJit
------

Complete x86/x64 JIT and Remote Assembler for C++.

  * [Official Repository (kobalicek/asmjit)](https://github.com/kobalicek/asmjit)
  * [Official Blog (asmbits)] (https://asmbits.blogspot.com/ncr)
  * [Official Chat (gitter)](https://gitter.im/kobalicek/asmjit)
  * [Permissive ZLIB license](./LICENSE.md)

Introduction
------------

AsmJit is a complete JIT and remote assembler for C++ language. It can generate native code for x86 and x64 architectures and supports the whole x86/x64 instruction set - from legacy MMX to the newest AVX2. It has a type-safe API that allows C++ compiler to do semantic checks at compile-time even before the assembled code is generated and executed.

AsmJit is not a virtual machine (VM). It doesn't have functionality to implement VM out of the box; however, it can be be used as a JIT backend of your own VM. The usage of AsmJit is not limited at all; it's suitable for multimedia, VM backends, remote code generation, and many other tasks.

Features
--------

  * Complete x86/x64 instruction set - MMX, SSEx, AVX1/2, BMI, XOP, FMA3, and FMA4 (AVX-512 in progress)
  * Low-level and high-level code generation concepts
  * Built-in CPU features detection
  * Virtual Memory management similar to malloc/free
  * Powerful logging and error handling
  * Small and embeddable, around 150-200kB compiled
  * Zero dependencies, not even STL or RTTI

Supported Environments
----------------------

### Operating Systems

  * BSDs (not tested regularly) 
  * Linux (tested by Travis-CI)
  * Mac (tested by Travis-CI)
  * Windows (tested manually)

### C++ Compilers

  * Clang (tested by Travis-CI)
  * CodeGear (including BorlandC++, not tested regularly)
  * GCC (tested by Travis-CI)
  * MinGW (tested manually)
  * MSVC (tested manually, at least VS2003 is required)
  * Other compilers require some testing and support in `asmjit/build.h`

### Backends

  * ARM (work-in-progress)
  * X86 (tested by Travis-CI)
  * X64 (tested by Travis-CI)

Project Organization
--------------------

  * `/`             - Project root
    * `src`         - Source code
      * `asmjit`    - Source code and headers (always point include path in here)
        * `base`    - Generic API and interfaces, used by all backends
        * `arm`     - ARM/ARM64 specific API, used only by ARM and ARM64 backends
        * `x86`     - X86/X64 specific API, used only by X86 and X64 backends
      * `test`      - Unit and integration tests (don't embed in your project)
    * `tools`       - Tools used for configuring, documenting and generating files

Code Generation Concepts
------------------------

AsmJit has two completely different code generation concepts. The difference is in how the code is generated. The first concept, also referred as a low level concept, is called `Assembler` and it's the same as writing RAW assembly by inserting instructions that use physical registers directly. In this case AsmJit does only instruction encoding, verification and final code relocation.

The second concept, also referred as a high level concept, is called `Compiler`. Compiler lets you use virtually unlimited number of registers (it calls them variables), which significantly simplifies the code generation process. Compiler allocates these virtual registers to physical registers after the code generation is done. This requires some extra effort - Compiler has to generate information for each node (instruction, function declaration, function call, etc...) in the code, perform a variable liveness analysis and translate the code using variables to a code that uses only physical registers.

In addition, Compiler understands functions and their calling conventions. It has been designed in a way that the code generated is always a function having a prototype like a real programming language. By having a function prototype the Compiler is able to insert prolog and epilog sequence to the function being generated and it's able to also generate a necessary code to call other function from your own code.

There is no conclusion on which concept is better. `Assembler` brings full control and the best performance, while `Compiler` makes the code-generation more fun and more portable.

Configuring & Building
----------------------

AsmJit is designed to be easy embeddable in any kind project. However, it has some compile-time flags that can be used to build a specific version of AsmJit including or omitting certain features. A typical way to build AsmJit is to use [cmake](http://www.cmake.org), but it's also possible to just include AsmJit source code in our project and build it with it optionally editing its `asmjit/config.h` file to turn on/off some specific features. The most easies way to include AsmJit in your project is to just copy AsmJit source somewhere into it and to define globally `ASMJIT_STATIC` macro. This way AsmJit can be just updated from time to time without any changes to it. Please do not include / compile AsmJit test files (`asmjit/test` directory) when embedding.

### Build Type

  * `ASMJIT_EMBED` - Parameter that can be set to cmake to turn off building library, useful if you want to include asmjit in your project without building the library. `ASMJIT_EMBED` behaves identically as `ASMJIT_STATIC`.
  * `ASMJIT_STATIC` - Define when building AsmJit as a static library. No symbols will be exported by AsmJit by default.

  * By default AsmJit build is configured as a shared library so none of `ASMJIT_EMBED` and `ASMJIT_STATIC` have to be defined explicitly.

### Build Mode

  * `ASMJIT_DEBUG` - Define to always turn debugging on (regardless of build-mode).
  * `ASMJIT_RELEASE` - Define to always turn debugging off (regardless of build-mode).
  * `ASMJIT_TRACE` - Define to enable AsmJit tracing. Tracing is used to catch bugs in AsmJit and it has to be enabled explicitly. When AsmJit is compiled with `ASMJIT_TRACE` it uses `stdout` to log information related to AsmJit execution. This log can be helpful when examining liveness analysis, register allocation or any other part of AsmJit.

  * By default none of these is defined, AsmJit detects mode based on compile-time macros (useful when using IDE that has switches for Debug/Release/etc...).

### Architectures

  * `ASMJIT_BUILD_ARM` - Build ARM backend.
  * `ASMJIT_BUILD_ARM64` - Build ARM64 backend.
  * `ASMJIT_BUILD_X86` - Build x86 backend.
  * `ASMJIT_BUILD_X64` - Build x64 backend.
  * `ASMJIT_BUILD_HOST` - Build host backend, if only `ASMJIT_BUILD_HOST` is used only the host architecture detected at compile-time will be included.

  * By default only `ASMJIT_BUILD_HOST` is defined.

### Features

  * `ASMJIT_DISABLE_COMPILER` - Disable `Compiler` completely. Use this flag if you don't use Compiler and want a slimmer binary.
  * `ASMJIT_DISABLE_LOGGER` - Disable `Logger` completely. Use this flag if you don't need `Logger` functionality and want slimmer binary. AsmJit compiled with or without `Logger` support is binary compatible (all classes that use Logger pointer will simply use `void*`), but the Logger interface and in general instruction dumps won't be available anymore.
  * `ASMJIT_DISABLE_NAMES` - Disable everything that uses strings and that causes certain strings to be stored in the resulting binary. For example when this flag is enabled all instruction and error names (and related APIs) will not be available. This flag has to be disabled together with `ASMJIT_DISABLE_LOGGER`.

Using AsmJit
------------

AsmJit library uses one global namespace called `asmjit`, which contains the basics. Architecture specific code is prefixed by the architecture and architecture specific registers and operand builders are in its own namespace. For example classes for both x86 and x64 code generation are prefixed by `X86`, enums by `kX86`, registers and operand builders are accessible through `x86` namespace. This design is very different from the initial version of AsmJit and it seems now as the most convenient one.

### Runtime & Code-Generators

AsmJit contains two classes that are required to generate a machine code. `Runtime` specifies where the code is generated and acts as a storage, while `CodeGen` specifies how the code is generated and acts as a machine code stream. All the examples here use `Compiler` code-generator to generate the code and `JitRuntime` to store and run it.

### Instruction Operands

Operand is a part of an instruction, which specifies the data the instruction will operate on. There are five types of operands in AsmJit:

  * `Reg` - Physical register, used only by `Assembler`
  * `Var` - Virtual register, used only by `Compiler`
  * `Mem` - Used to reference memory location
  * `Label` - Used to reference a location in code
  * `Imm` - Immediate value that is encoded with the instruction itself

Base class for all operands is `Operand`. It contains interface that can be used by all types of operands only and it is typically passed by value, not as a pointer. The classes `Reg`, `Var`, `BaseMem`, `Label` and `Imm` all inherit `Operand` and provide an operand specific functionality. Architecture specific operands are prefixed by the architecture like `X86Reg` or `X86Mem`. Most of the architectures provide several types of registers, for example x86/x64 architecture has `X86GpReg`, `X86MmReg`, `X86FpReg`, `X86XmmReg` and `X86YmmReg` registers plus some extras including segment registers and `rip` (relative instruction pointer).

When using a code-generator some operands have to be created explicitly by using its interface. For example labels are created by using `newLabel()` method of the code-generator and variables are created by using architecture specific methods like `newGpVar()`, `newMmVar()` or `newXmmVar()`.

### Function Prototypes

AsmJit needs to know the prototype of the function it will generate or call. AsmJit contains a mapping between a type and the register that will be used to represent it. To make life easier there is a function builder that does the mapping on the fly. Function builder is a template class that helps with creating a function prototype by using native C/C++ types that describe function arguments and return value. It translates C/C++ native types into AsmJit specific IDs and makes these IDs accessible to Compiler.

### Putting It All Together

Let's put all together and generate a first function that sums its two arguments and returns the result. At the end the generated function is called from a C++ code.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  // Create JitRuntime and X86 Assembler/Compiler.
  JitRuntime runtime;
  X86Assembler a(&runtime);
  X86Compiler c(&a);

  // Build function having two arguments and a return value of type 'int'.
  // First type in function builder describes the return value. kCallConvHost
  // tells the compiler to use the host calling convention.
  c.addFunc(FuncBuilder2<int, int, int>(kCallConvHost));

  // Create 32-bit variables (virtual registers) and assign some names to
  // them. Using variable names is not necessary, however, it can make
  // debugging easier.
  X86GpVar x = c.newInt32("x");
  X86GpVar y = c.newInt32("y");

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
  runtime.release((void*)func);

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
  JitRuntime runtime;
  X86Assembler a(&runtime);
  X86Compiler c(&a);

  // This function uses 3 arguments.
  c.addFunc(FuncBuilder3<int, int, int, int>(kCallConvHost));

  // New variable 'op' added.
  X86GpVar op = c.newInt32("op");
  X86GpVar x = c.newInt32("x");
  X86GpVar y = c.newInt32("y");

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

  runtime.release((void*)func);
  return 0;
}
```

In this example conditional and unconditional jumps were used with labels together. Labels have to be created explicitely by `Compiler` by using a `Label L = c.newLabel()` form. Each label as an unique ID that identifies it, however it's not a string and there is no way to query for a `Label` instance that already exists at the moment. Label is like any other operand moved by value, so the copy of the label will still reference the same label and changing a copied label will not change the original label.

Each label has to be bound to the location in the code by using `bind()`; however, it can be bound only once! Trying to bind the same label multiple times has undefined behavior - assertion failure is the best case.

### Memory Addressing

X86/X64 architectures have several memory addressing modes which can be used to combine base register, index register and a displacement. In addition, index register can be shifted by a constant from 1 to 3 that can help with addressing elements up to 8-byte long in an array. AsmJit supports all forms of memory addressing. Memory operand can be created by using `asmjit::X86Mem` or by using related non-member functions like `asmjit::x86::ptr` or `asmjit::x86::ptr_abs`. Use `ptr` to create a memory operand having a base register with optional index register and a displacement; use and `ptr_abs` to create a memory operand referring to an absolute address in memory (32-bit) and optionally having an index register.

In the following example various memory addressing modes are used to demonstrate how to construct and use them. It creates a function that accepts an array and two indexes which specify which elements to sum and return.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime runtime;
  X86Assembler a(&runtime);
  X86Compiler c(&a);

  // Function returning 'int' accepting pointer and two indexes.
  c.addFunc(FuncBuilder3<int, const int*, intptr_t, intptr_t>(kCallConvHost));

  X86GpVar p = c.newIntPtr("p");
  X86GpVar xIndex = c.newIntPtr("xIndex");
  X86GpVar yIndex = c.newIntPtr("yIndex");

  c.setArg(0, p);
  c.setArg(1, xIndex);
  c.setArg(2, yIndex);

  X86GpVar x = c.newInt32("x");
  X86GpVar y = c.newInt32("y");

  // Read `x` by using a memory operand having base register, index register
  // and scale. Translates to `mov x, dword ptr [p + xIndex << 2]`.
  c.mov(x, x86::ptr(p, xIndex, 2));

  // Read `y` by using a memory operand having base register only. Variables
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

  runtime.release((void*)func);
  return 0;
}
```

### Using Stack

AsmJit uses stack automatically to spill variables if there is not enough registers to keep them all allocated. The stack frame is managed by `Compiler` that provides also an interface to allocate chunks of memory of user specified size and alignment.

In the following example a stack of 256 bytes size is allocated, filled by bytes starting from 0 to 255 and then iterated again to sum all the values.

```c++
#include <asmjit/asmjit.h>

using namespace asmjit;

int main(int argc, char* argv[]) {
  JitRuntime runtime;
  X86Assembler a(&runtime);
  X86Compiler c(&a);

  // Function returning 'int' without any arguments.
  c.addFunc(FuncBuilder0<int>(kCallConvHost));

  // Allocate 256 bytes on the stack aligned to 4 bytes.
  X86Mem stack = c.newStack(256, 4);

  X86GpVar p = c.newIntPtr("p");
  X86GpVar i = c.newIntPtr("i");

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
  X86GpVar sum = c.newInt32("sum");
  X86GpVar val = c.newInt32("val");

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

  runtime.release((void*)func);
  return 0;
}
```

### Using Constant Pool

To be documented.

Advanced Features
-----------------

AsmJit offers much more, but not everything can fit into the introduction. The following sections don't have complete examples, but contain hints that can be useful and can change a bit the way AsmJit is used.

### Logging and Error Handling

Failures are common when working at machine level. AsmJit does already a good job with function overloading to prevent from emitting semantically incorrect instructions; however, AsmJit can't prevent from emitting code that is semantically correct, but contains bug(s). Logging has always been an important part of AsmJit's infrastructure and the output can be very valuable after something went wrong.

AsmJit contains extensible logging interface defined by `Logger` class and implemented by `FileLogger` and `StringLogger`. `FileLogger` can log into a standard C-based `FILE*` stream while `StringLogger` logs to an internal buffer that can be used after the code generation is done.

Loggers can be assigned to any code generator and there is no restriction of assigning a single logger to multiple code generators, but this is not practical when running these in multiple threads. `FileLogger` is thread-safe since it uses plain C `FILE*` stream, but `StringLogger` is not!

The following snippet describes how to log into `FILE*`:

```c++
// Create logger logging to `stdout`. Logger life-time should always be
// greater than a life-time of the code generator. Alternatively the
// logger can be reset before it's destroyed.
FileLogger logger(stdout);

// Create runtime and assembler and attach a logger to the assembler.
JitRuntime runtime;
X86Assembler a(&runtime);
a.setLogger(&logger);

// ... Generate the code ...
```

The following snippet describes how to log into a string:

```c++
StringLogger logger;

JitRuntime runtime;
X86Assembler a(&runtime);
a.setLogger(&logger);

// ... Generate the code ...

printf("Logger Content:\n%s", logger.getString());

// You can use `logger.clearString()` if the intend is to reuse the logger.
```

Logger can be configured to show more information by using `logger.setOption()` method. The following options are available:

  * `kLoggerOptionBinaryForm` - Log also binary sequence for each instruction generated.
  * `kLoggerOptionHexImmediate` - Format immediate values to base16 (hex) form.
  * `kLoggerOptionHexDisplacement` - Format memory displacements to base16 (hex) form.

TODO: Liveness analysis and instruction scheduling options.

### Code Injection

Code injection was one of key concepts of Compiler from the beginning. Compiler records all emitted instructions in a double-linked list which can be manipulated before `make()` is called. Any call to Compiler that adds instruction, function or anything else in fact manipulates this list by inserting nodes into it.

To manipulate the current cursor use Compiler's `getCursor()` and `setCursor()` methods. The following snippet demonstrates the proper way of code injection.

```c++
X86Compiler c(...);

X86GpVar x = c.newInt32("x");
X86GpVar y = c.newInt32("y");

ASNode* here = c.getCursor();
c.mov(y, 2);

// Now, `here` can be used to inject something before `mov y, 2`. To inject
// something it's always good to remember the current cursor so it can be set
// back after the injecting is done. When `setCursor()` is called it returns
// the old cursor to be remembered.
ASNode* prev = c.setCursor(here);
c.mov(x, 1);
c.setCursor(prev);
```

The resulting code would look like:

```
c.mov(x, 1);
c.mov(y, 2);
```

Support
-------

Please consider a donation if you use the project and would like to keep it active in the future.

  * [Donate by PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=QDRM6SRNG7378&lc=EN;&item_name=asmjit&currency_code=EUR)

Received From:

  * [PELock - Software copy protection and license key system](https://www.pelock.com)

Authors & Maintainers
---------------------

  * Petr Kobalicek <kobalicek.petr@gmail.com>
