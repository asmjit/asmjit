2016-03-21

CpuInfo has been completely redesigned. It now supports multiple CPUs without having to inherit it to support a specific architecture. Also all CpuInfo-related constants have been moved to CpuInfo.

Change:

```
const X86CpuInfo* cpu = X86CpuInfo::getHost();
cpu->hasFeature(kX86CpuFeatureSSE4_1);
```

to

```
const CpuInfo& cpu = CpuInfo::getHost();
cpu.hasFeature(CpuInfo::kX86FeatureSSE4_1);
```

The whole code-base now uses `noexcept` keyword to inform API users that these functions won't throw an exception. Moreover, the possibility to throw exception through `ErrorHandler` has been removed as it seems that nobody has ever used it. `Assembler::emit()` and friends are still not marked as `noexcept` in case this decision is taken back. If there is no complaint even `emit()` functions will be marked `noexcept` in the near future.

2015-12-07
----------

Compiler now attaches to Assembler. This change was required to create resource sharing where Assembler is the central part and Compiler is a "high-level" part that serializes to it. It's an incremental work to implement sections and to allow code generators to create executables and libraries.

Also, Compiler has no longer Logger interface, it uses Assembler's one after it's attached to it.

```
JitRuntime runtime;
X86Compiler c(&runtime);

// ... code generation ...

void* p = c.make();
```

to

```
JitRuntime runtime;
X86Assembler a(&runtime);
X86Compiler c(&a);

// ... code generation ...

c.finalize();
void* p = a.make();
```

All nodes were prefixed with HL, except for platform-specific nodes, change:

```
Node        -> HLNode
FuncNode    -> HLFunc
X86FuncNode -> X86Func
X86CallNode -> X86Call
```

`FuncConv` renamed to `CallConv` and is now part of a function prototype, change:

```
compiler.addFunc(kFuncConvHost, FuncBuilder0<Void>());
```

to

```
compiler.addFunc(FuncBuilder0<Void>(kCallConvHost));
```

Operand constructors that accept Assembler or Compiler are deprecated. Variables can now be created by using handy shortcuts like newInt32(), newIntPtr(), newXmmPd(), etc... Change:

```
X86Compiler c(...);
Label L(c);
X86GpVar x(c, kVarTypeIntPtr, "x");
```

to

```
X86Compiler c(...);
Label L = c.newLabel();
X86GpVar x = c.newIntPtr("x");
```

