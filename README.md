AsmJit
======

Complete x86/x64 JIT and Remote Assembler for C++.

Official Repository
-------------------

https://github.com/kobalicek/asmjit

Support the Project
-------------------

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](
  https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=QDRM6SRNG7378&lc=EN;&item_name=asmjit&currency_code=EUR)

Introduction
------------

AsmJit is a complete JIT and remote assembler for C++ language. It can generate native code for x86 and x64 architectures and supports the whole x86/x64 instruction set (from legacy MMX to the newest AVX2). It has a type-safe API that allows C++ compiler to do a semantic checks at compile-time even before the assembled code is generated or run.

AsmJit is not a virtual machine nor tries to be. It's a general purpose tool that can be used to encode assembly instructions into their machine code representations and tries to make such process easy and fun. AsmJit has been used so far in software encryption, image/sound processing, emulators and as a JIT backend in virtual machines.

Features
--------

  * Complete x86/x64 instruction set - MMX, SSE, AVX, BMI, XOP, FMA...,
  * Low-level and high-level code generation,
  * Built-in CPU detection,
  * Virtual Memory management,
  * Pretty logging and error handling,
  * Small and embeddable, around 150-200kB compiled,
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
  * MSVC (VS2005+)
  * Other compilers require testing

### Backends

  * X86
  * X64

Project Organization
--------------------

  - project root /
    - src         - Source code
      - asmjit    - Public header files (always include from here)
        - base    - Base files, used by the AsmJit and all backends
        - contrib - Contributions that extends base functionality
        - x86     - X86/X64 specific files, used only by X86/X64 backend
    - tools       - Tools used for configuring, documenting and generating files

Code Generation Concepts
------------------------

AsmJit has two completely different code generation concepts. The difference is in how the code is generated. The first concept, also referred as the low level concept, is called 'Assembler' and it's the same as writing RAW assembly by using physical registers directly. In this case AsmJit does only instruction encoding, verification and relocation.

The second concept, also referred as the high level concept, is called 'Compiler'. Compiler lets you use virtually unlimited number of registers (called variables) significantly simplifying the code generation process. Compiler allocates these virtual registers to physical registers after the code generation is done. This requires some extra effort - Compiler has to generate information for each node (instruction, function declaration, function call) in the code, perform a variable liveness analysis and translate the code having variables into code having only registers.

In addition, Compiler understands functions and function calling conventions. It has been designed in a way that the code generated is always a function having prototype like in a programming language. By having a function prototype the Compiler is able to insert prolog and epilog to a function being generated and it is able to call a function inside a generated one.

There is no conclusion on which concept is better. Assembler brings full control on how the code is generated, while Compiler makes the generation easier and more portable.

Configuring & Building
----------------------

AsmJit is designed to be easy embeddable in any project. However, it has some compile-time flags that can be used to build a specific version of AsmJit including or omitting certain features:

### Library Type

  * *ASMJIT_EMBED* - Parameter that can be set to cmake to turn off building library, useful if you want to include asmjit in your project without building the library.
  * *ASMJIT_STATIC* - Define when building AsmJit as a static library. No symbols will be exported by AsmJit by default.
  * *ASMJIT_API* - This is AsmJit API decorator that is used in all functions that has to be exported. It can be redefined, however it's not a recommended way.
  * By default AsmJit build is configured as a shared library and *ASMJIT_API* contains compiler specific attributes to import/export AsmJit symbols.

### Backends

  * *ASMJIT_BUILD_X86* - Always build x86 backend regardless of host architecture.
  * *ASMJIT_BUILD_X64* - Always build x64 backend regardless of host architecture.
  * *ASMJIT_BUILD_HOST* - Always build host backand, if only *ASMJIT_BUILD_HOST* is used only the host architecture detected at compile-time will be included.
  * By default only *ASMJIT_BUILD_HOST* is defined.

### Build Mode

  * *ASMJIT_DEBUG* - Define to always turn debugging on (regardless of build-mode).
  * *ASMJIT_RELEASE* - Define to always turn debugging off (regardless of build-mode).
  * By default none of these is defined, AsmJit detects mode based on compile-time macros (useful when using IDE that has switches for Debug/Release/etc...).


To build AsmJit please use cmake <http://www.cmake.org> that will generate  project files for your favorite IDE and platform. If you don't use cmake and you still want to include AsmJit in your project it's perfectly fine by just including it there, probably defining *ASMJIT_STATIC* to prevent AsmJit trying to export the API.

Using AsmJit
------------

AsmJit test suite contains up-to-date tests that can be used as a starting point. Base concepts are discussed below. Most of the constructs will work in pure Assembler if variables are replaced by registers and functions prologs/epilogs hand coded. The Compiler is used just to make things simple and most of users prefer it anyway. To use AsmJit basic skills to setup your environment are required and not discussed here.

AsmJit library uses one global namespace called `asmjit` which contains the basics. Architecture specific code is nested, for example x86 support is in `asmjit::x86`, x64 support is in `asmjit::x64` and shared x86/x64 in `asmjit::x86x64`. To make things simple AsmJit provides `asmjit::host` namespace which imports namespace of the detected host architecture automatically. Nested namespaces were introduced to enable support of multiple architectures in the future and to make JIT code generation a special case, not a mandatory requirement. To use AsmJit include only the main `asmjit.h` header usually in form `<asmjit/asmjit.h>`, don't include headers found in subdirectories.

### Runtime & Code-Generators

AsmJit contains two classes that are required to generate machine code. Runtime specifies where the code is generated and acts as a storage, while Code-Generator specifies how the code is generated and acts as a machine code stream. All the examples here use `asmjit::host::Compiler` class to generate the code and 'asmjit::JitRuntime` to store and run it.

### Instruction Operands

Operand is a part of CPU instruction which specifices the data the instruction will operate on. There are five types of operands in AsmJit:

  * *Register* - Physical register (used only by Assember)
  * *Variable* - Virtual register (used only by Compiler)
  * *Memory* - Location in memory
  * *Label* - Location in code
  * *Immediate* - Constant that is encoded with the instruction itself

Base class for all operands is `asmjit::Operand`, but it contains only interface that can be used by all of them. Operand is a statically allocated structure that acts lika a value, not a pointer - just copy if you need multiple instances of the same operand. Since the most of the operands are architecture dependent,AsmJit always contain a base-operand structure - for example `asmjit::BaseReg` or `asmjit::BaseMem` and their architecture specific counterparts `asmjit::x86x64::GpReg` or `asmjit::x86x64::Mem`. 

When using a code-generator some operands have to be created explicitly, for example use `newLabel()` to create a label and `newGpVar()` to create a virtual general purpose register.

### Function Prototypes

AsmJit needs to know the prototype of the function it will generate or call. AsmJit contains a mapping between a type and the register that will be used to represent it. To make life easier there is a function builder that does the mapping on the fly. Function builder is a template class that helps with creating a function prototype by using native C/C++ types that describe function arguments and return value. It translates C/C++ native types into AsmJit specific IDs and makes these IDs accessible to Compiler.

### Putting It All Together

Let's put all together and generate a first function that sums its two arguments and returns the result. At the end the generated function is called from a C++ code.

```C++
#include <asmjit/asmjit.h>

using namespace asmjit;
using namespace asmjit::host;

int main(int argc, char* argv[]) {
  // Create JitRuntime and host specific Compiler.
  JitRuntime runtime;
  Compiler c(&runtime);

  // Build function having two arguments and a return value of type 'int'.
  // First type in function builder describes the return value. kFuncConvHost
  // tells compiler to use a host calling convention.
  c.addFunc(kFuncConvHost, FuncBuilder2<int, int, int>());

  // Create 32-bit variables (virtual registers) and assign some names to
  // them. Using names is purely optional and only greatly helps while
  // debugging.
  GpVar a(c, kVarTypeInt32, "a");
  GpVar b(c, kVarTypeInt32, "b");

  // Tell asmjit to use these variables as function arguments.
  c.setArg(0, a);
  c.setArg(1, b);

  // a = a + b;
  c.add(a, b);

  // Tell asmjit to return 'a'.
  c.ret(a);

  // Finalize the current function.
  c.endFunc();

  // Now the Compiler contains the whole function, but the code is not yet
  // generated. To tell compiler to generate the function make() has to be
  // called.

  // Make uses the JitRuntime passed to Compiler constructor to allocate a
  // buffer for the function and make it executable.
  void* funcPtr = c.make();

  // In order to run 'funcPtr' it has to be casted to the desired type.
  // Typedef is a recommended and safe way to create a function-type.
  typedef int (*FuncType)(int, int);

  // Using asmjit_cast is purely optional, it's basically a C-style cast
  // that tries to make it visible that a function-type is returned.
  FuncType func = asmjit_cast<FuncType>(funcPtr);

  // Finally, run it and do something with the result...
  int x = func(1, 2);
  printf("x=%d\n", x); // Outputs "x=3".

  // The function will remain in memory after Compiler is destroyed, but
  // will be destroyed together with Runtime. This is just simple example
  // where we can just destroy both at the end of the scope and that's it.
  // However, it's a good practice to clean-up resources after they are
  // not needed and using runtime.release() is the preferred way to free
  // a function added to JitRuntime.
  runtime.release((void*)func);

  // Runtime and Compiler will be destroyed at the end of the scope.
  return 0;
}
```

The code should be self explanatory, however there are some details to be clarified.

The code above generates and calls a function of `kFuncConvHost` calling convention. 32-bit architecture contains a wide range of function calling conventions that can be all used by a single program, so it's important to know which calling convention is used by your C/C++ compiler so you can call the function. However, most compilers should generate CDecl by default. In 64-bit mode there are only two calling conventions, one is specific for Windows (Win64 calling convention) and the other for Unix (AMD64 calling convention). The `kFuncConvHost` is defined to be one of CDecl, Win64 or AMD64 depending on your architecture and operating system.

Default integer size is platform specific, virtual types `kVarTypeIntPtr` and `kVarTypeUIntPtr` can be used to make the code more portable and they should be always used when a pointer type is needed. When no type is specified AsmJit always defaults to `kVarTypeIntPtr`. The code above works with integers where the default behavior has been overidden to 32-bits. Note it's always a good practice to specify the type of the variable used. Alternative form of creating a variable is `c.newGpVar(...)`, `c.newMmVar(...)`, `c.newXmmVar` and so on...

The function starts with `c.addFunc()` and ends with `c.endFunc()`. It's not allowed to put code outside of the function; however, embedding data outside of the function body is allowed.

### Using Labels

Labels are essential for making jumps, function calls or to refer to a data that is embedded in the code section. Label has to be explicitly created by using `newLabel()` member function of your code generator in order to be used. The following example executes a code that depends on the condition by using a `Label` and conditional jump instruction. If the first parameter is zero it returns `a + b`, otherwise `a - b`.

```C++
#include <asmjit/asmjit.h>

using namespace asmjit;
using namespace asmjit::host;

int main(int argc, char* argv[]) {
  JitRuntime runtime;
  Compiler c(&runtime);

  // This function uses 3 arguments.
  c.addFunc(kFuncConvHost, FuncBuilder3<int, int, int, int>());

  // New variable 'op' added.
  GpVar op(c, kVarTypeInt32, "op");
  GpVar a(c, kVarTypeInt32, "a");
  GpVar b(c, kVarTypeInt32, "b");

  c.setArg(0, op);
  c.setArg(1, a);
  c.setArg(2, b);

  // Create labels.
  Label L_Subtract(c);
  Label L_Skip(c);

  // If (op != 0)
  //   goto L_Subtract;
  c.test(op, op);
  c.jne(L_Subtract);

  // a = a + b;
  // goto L_Skip;
  c.add(a, b);
  c.jmp(L_Skip);

  // L_Subtract:
  // a = a - b;
  c.bind(L_Subtract);
  c.sub(a, b);

  // L_Skip:
  c.bind(L_Skip);

  c.ret(a);
  c.endFunc();

  // The prototype of the generated function changed also here.
  typedef int (*FuncType)(int, int, int);
  FuncType func = asmjit_cast<FuncType>(c.make());

  int x = func(0, 1, 2);
  int y = func(1, 1, 2);

  printf("x=%d\n", x); // Outputs "x=3".
  printf("y=%d\n", y); // Outputs "y=-1".

  runtime.release((void*)func);
  return 0;
}
```

In this example conditional and uncondition jumps were used with labels together. Labels are created explicitly by the Compiler by passing a Compiler instance to a `Label` constructor or by using a `Label l = c.newLabel()` form. Each label as an unique ID that identifies it, however it's not a string and there is no way to query for a Label instance that already exists. Label is like any other operand moved by value so the copy of the label will still reference the same label and changing a copied will not change the original label.

Each label has to be bound to the location in the code by using `c.bind()`; however, it can be only bound once! Trying to bind the same label multiple times has undefined behavior - it will trigger an assertion failure in the best case.

### Memory Addressing

X86/X64 architectures have several memory addressing modes which can be used to combine base register, index register and a displacement. In addition, index register can be shifted by a constant from 1 to 3 that can help with addressing elements up to 8-byte long in an array. AsmJit supports all forms of memory addressing. Memory operand can be created by using `asmjit::host::Mem` or by using related non-member functions like `asmjit::host::ptr` or `asmjit::host::ptr_abs`. Use `ptr` to create a memory operand having a base register with optional index register and a displacement and `ptr_abs` to create a memory operand refering to an absolute address in memory and optionally having an index register.

In the following example various memory addressing modes are used to demonstrate how to construct and use memory operands. It creates a function that accepts an array and two indexes which specify which elements to sum and return.

```C++
#include <asmjit/asmjit.h>

using namespace asmjit;
using namespace asmjit::host;

int main(int argc, char* argv[]) {
  JitRuntime runtime;
  Compiler c(&runtime);

  // Function returning 'int' accepting pointer and two indexes.
  c.addFunc(kFuncConvHost, FuncBuilder3<int, const int*, intptr_t, intptr_t>());

  GpVar p(c, kVarTypeIntPtr, "p");
  GpVar aIndex(c, kVarTypeIntPtr, "aIndex");
  GpVar bIndex(c, kVarTypeIntPtr, "bIndex");

  c.setArg(0, p);
  c.setArg(1, aIndex);
  c.setArg(2, bIndex);

  GpVar a(c, kVarTypeInt32, "a");
  GpVar b(c, kVarTypeInt32, "b");

  // Read 'a' by using a memory operand having base register, index register
  // and scale. Translates to 'mov a, dword ptr [p + aIndex << 2]'.
  c.mov(a, ptr(p, aIndex, 2));

  // Read 'b' by using a memory operand having base register only. Variables
  // 'p' and 'bIndex' are both modified.

  // Shift bIndex by 2 (exactly the same as multiplying by 4).
  // And add scaled 'bIndex' to 'p' resulting in 'p = p + bIndex * 4'.
  c.shl(bIndex, 2);
  c.add(p, bIndex);

  // Read 'b'.
  c.mov(b, ptr(p));  
  
  // a = a + b;
  c.add(a, b);
  
  c.ret(a);
  c.endFunc();

  // The prototype of the generated function changed also here.
  typedef int (*FuncType)(const int*, intptr_t, intptr_t);
  FuncType func = asmjit_cast<FuncType>(c.make());

  // Array passed to 'func'
  const int array[] = { 1, 2, 3, 5, 8, 13 };

  int x = func(array, 1, 2);
  int y = func(array, 3, 5);

  printf("x=%d\n", x); // Outputs "x=5".
  printf("y=%d\n", y); // Outputs "y=18".

  runtime.release((void*)func);
  return 0;
}
```

### Using Stack

AsmJit uses stack automatically to spill variables if there is not enough registers to keep them all allocated. Stack is allocated automatically by Compiler and it's not allowed to manipulate it directly. However, Compiler provides an interface to allocate chunks of memory of user specified size and alignment on the stack.

In the following example a stack of 256 bytes size is allocated, filled by bytes starting from 0 to 255 and then iterated again to sum all the values.

```C++
#include <asmjit/asmjit.h>

using namespace asmjit;
using namespace asmjit::host;

int main(int argc, char* argv[]) {
  JitRuntime runtime;
  Compiler c(&runtime);

  // Function returning 'int' without any arguments.
  c.addFunc(kFuncConvHost, FuncBuilder0<int>());

  // Allocate a function stack of size 256 aligned to 4 bytes.
  Mem stack = c.newStack(256, 4);

  GpVar p(c, kVarTypeIntPtr, "p");
  GpVar i(c, kVarTypeIntPtr, "i");

  // Load a stack address to 'p'. This step is purely optional and shows
  // that 'lea' is useful to load a memory operands address (even absolute)
  // to a general purpose register.
  c.lea(p, stack);

  // Clear 'i'. Notice that xor_() is used instead of xor(), because xor is
  // unfortunately a keyword in C++.
  c.xor_(i, i);

  // First loop, fill the stack allocated by a sequence of bytes from 0 to 255.
  Label L1(c);
  c.bind(L1);

  // Mov [p + i], i.
  //
  // Any operand can be cloned and modified. By cloning 'stack' and calling
  // 'setIndex' we created a new memory operand based on stack having an
  // index register set.
  c.mov(stack.clone().setIndex(i), i.r8());

  // if (++i < 256)
  //   goto L1;
  c.inc(i);
  c.cmp(i, 256);
  c.jb(L1);

  // Second loop, sum all bytes stored in 'stack'.
  GpVar a(c, kVarTypeInt32, "a");
  GpVar t(c, kVarTypeInt32, "t");

  c.xor_(i, i);
  c.xor_(a, a);

  Label L2(c);
  c.bind(L2);

  // Movzx t, byte ptr [stack + i]
  c.movzx(t, stack.clone().setIndex(i).setSize(1));
  // a += t;
  c.add(a, t);

  // if (++i < 256)
  //   goto L2;
  c.inc(i);
  c.cmp(i, 256);
  c.jb(L2);

  c.ret(a);
  c.endFunc();

  typedef int (*FuncType)(void);

  FuncType func = asmjit_cast<FuncType>(c.make());
  printf("a=%d\n", func()); // Outputs "a=32640".

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

Failures are common when working at machine level. AsmJit does already a good job by using overloaded functions per instruction to prevent from emitting semantically incorrect code. However, AsmJit can't prevent you from emitting code that is semantically correct, but doesn't do what it it supposed to do. Logging has always been an important part of AsmJit's infrastructure and the output can be very valuable after something went wrong.

To be documented.

### Code Injection

Code injection was one of key concepts of Compiler from the beginning. Compiler records all emitted instructions in a double-linked list which can be manipulated before `make()` is called. Any call to Compiler that adds instruction, function or anything else in fact manipulates this list by inserting nodes into it.

To manipulate the current cursor use Compiler's `getCursor()` and `setCursor()` member functions. The following snippet demonstrates the proper way of code injection.

```C++
Compiler c(...);

GpVar a(c, kVarTypeInt32, "a");
GpVar b(c, kVarTypeInt32, "b");

BaseNode* here = c.getCursor();
c.mov(b, 2);

// Now, 'here' can be used to inject something before 'mov b, 2'. To inject
// anything it's good to remember the current cursor so it can be set back
// after the injecting is done. When setCursor() is called it returns the old
// cursor.
BaseNode* oldCursor = c.setCursor(here);
c.mov(a, 1);
c.setCursor(oldCursor);
```

The resulting code would look like:

```
c.mov(a, 1);
c.mov(b, 2);
```

License
-------

AsmJit can be distributed under zlib license:

  * <http://www.opensource.org/licenses/zlib-license.php>

Contact Authors & Maintainers
-----------------------------

  * Petr Kobalicek <kobalicek.petr@gmail.com>
