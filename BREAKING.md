2016-07-20
----------

  * Global `asmjit_cast<>` removed and introduced a more type-safe `asmjit::ptr_cast<>`, which can cast a function to `void*` (and vice-versa), but will refuse to cast a function to `void**`, for example. Just change `asmjit_cast` to `asmjit::ptr_cast` and everything should work as usual. As a consequence, the Runtime now contains a typesafe (templated) `add()` and `remove()` methods that accept a function type directly, no need to cast manually to `void*` and `void**`. If you use your own runtime rename your virtual methods from `add` to `_add` and from `release` to `_release` and enjoy the type-safe wrappers.
  * Removed `Logger::Style` and `uint32_t style` parameter in Logging API. It was never used for anything so it was removed.
  * There is a new `CodeEmitter` base class that defines assembler building blocks that are implemented by `Assembler` and `CodeBuilder`. `CodeCompiler` is now based on `CodeBuilder` and shares its instruction storage functionality. Most API haven't changed, just base classes and new functionality has been added. It's now possible to serialize code for further processing by using `CodeBuilder`.
  * Renamed compile-time macro `ASMJIT_DISABLE_LOGGER` to `ASMJIT_DISABLE_LOGGING`. There is a new `Formatter` class which is also disabled with this option.

  * Operand API is mostly intact, omitting Var/Reg should fix most compile-time errors. There is now no difference between a register index and register id internally. If you ever used `reg.getRegIndex()` then use `reg.getId()` instead. Also renamed `isInitialized()` to `isValid()`.
    * There are much more changes, but they are mostly internal and keeping most operand methods compatible.
    * Added new functionality into `asmjit::x86` namespace related to operands.
    * X86Xmm/X86Ymm/X86Zmm register operands now inherit from X86Vec.
    * Register kind (was register class) is now part of `Reg` operand, you can get it by using `reg.getRegKind()`.
    * Register class enum moved to `X86Reg`, `kX86RegClassGp` is now `X86Reg::kKindGp`.
    * Register type enum moved to `X86Reg`, `kX86RegTypeXmm` is now `X86Reg::kRegXmm`.
    * Register index enum moved to `X86Gp`, `kX86RegIndexAx` is now `X86Gp::kIdAx`.
    * Segment index enum moved to `X86Seg`, `kX86SegFs` is now `X86Seg::kIdFs`.
    * If you used `asmjit::noOperand` for any reason, change it to `Operand()`.

  * CodeBuilder and CodeCompiler now contain different prefix of their nodes to distinguish between them:

    * Rename `HLNode` to `CBNode` (CodeBuilder node).
    * Rename all other `HL` to `CB`.
    * Rename `X86FuncNode` to `CCFunc` (CodeCompiler function), no more arch specific prefixes here.
    * Rename `X86CallNode` to `CCFuncCall` (CodeCompiler function-call), also, no more X86 prefix.

  * AsmJit now uses CodeHolder to hold code. You don't need `Runtime` anymore if you don't plan to execute the code or if you plan to relocate it yourself:

```c++
CodeHolder code;                       // Create CodeHolder (holds the code).
code.init(CodeInfo(ArchInfo::kIdX64)); // Initialize CodeHolder to hold X64 code.

// Everything else as usual:
X86Assembler a(&code);                 // Create the emitter (Assembler, CodeBuilder, CodeCompiler).
```

  * Initializing with JitRuntime involves using CodeHolder:

```c++
JitRuntime rt;                         // Create JitRuntime.

CodeHolder code;                       // Create CodeHolder.
code.init(rt.getCodeInfo());           // Initialize CodeHolder to match the JitRuntime.

X86Assembler a(&code);                 // Create the emitter (Assembler, CodeBuilder, CodeCompiler).
...                                    // Generate some code.

typedef void (*SomeFunc)(void);        // Prototype of the function you generated.

SomeFunc func;                         // Function pointer.
Error err = rt.add(&func, &code);      // Add the generated function to the runtime.

rt.remove(func);                       // Remove the generated function from the runtime.
```

  * Merged virtual registers (known as variables or Vars) into registers themselves, making the interface simpler:

```c++
X86GpReg/X86GpVar merged to X86Gp
X86MmReg/X86MmVar merged to X86Mm
X86XmmReg/X86XmmVar merged to X86Xmm
X86YmmReg/X86YmmVar merged to X86Ymm
```

  * Refactored instruction database, moved many enums related to instructions into `X86Inst`. Also some instructions were wrong (having wrong signature in Assembler and Compiler) and were fixed.

```c++
X86InstInfo             renamed to X86Inst
kX86InstIdSomething     renamed to X86Inst::kIdSomething
kX86InstOptionSomething renamed to X86Inst::kOptionSomething
kX86CondSomething       renamed to X86Inst::kCondSomething
kX86CmpSomething        renamed to X86Inst::kCmpSomething
kX86VCmpSomething       renamed to X86Inst::kVCmpSomething
kX86PrefetchSomething   renamed to X86Inst::kPrefetchSomething
```
