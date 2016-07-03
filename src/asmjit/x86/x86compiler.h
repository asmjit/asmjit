// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86COMPILER_H
#define _ASMJIT_X86_X86COMPILER_H

#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies]
#include "../base/compiler.h"
#include "../base/vectypes.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compilerfunc.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Forward Declarations]
// ============================================================================

class X86CallNode;
class X86FuncNode;

//! \addtogroup asmjit_x86
//! \{

//! \internal
ASMJIT_VARAPI const VarInfo _x86VarInfo[];

#if defined(ASMJIT_BUILD_X86)
//! \internal
//!
//! Mapping of x86 variable types, including all abstract types, into their real types.
//!
//! This mapping translates the following:
//! - `kVarTypeInt64` to `kInvalidVar`.
//! - `kVarTypeUInt64` to `kInvalidVar`.
//! - `kVarTypeIntPtr` to `kVarTypeInt32`.
//! - `kVarTypeUIntPtr` to `kVarTypeUInt32`.
ASMJIT_VARAPI const uint8_t _x86VarMapping[kX86VarTypeCount];
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
//! \internal
//!
//! Mapping of x64 variable types, including all abstract types, into their real types.
//!
//! This mapping translates the following:
//! - `kVarTypeIntPtr` to `kVarTypeInt64`.
//! - `kVarTypeUIntPtr` to `kVarTypeUInt64`.
ASMJIT_VARAPI const uint8_t _x64VarMapping[kX86VarTypeCount];
#endif // ASMJIT_BUILD_X64

// ============================================================================
// [asmjit::X86FuncNode]
// ============================================================================

//! X86/X64 function node.
class X86FuncNode : public HLFunc {
 public:
  ASMJIT_NO_COPY(X86FuncNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `X86FuncNode` instance.
  ASMJIT_INLINE X86FuncNode(Compiler* compiler) noexcept : HLFunc(compiler) {
    _decl = &_x86Decl;
    _saveRestoreRegs.reset();

    _alignStackSize = 0;
    _alignedMemStackSize = 0;
    _pushPopStackSize = 0;
    _moveStackSize = 0;
    _extraStackSize = 0;

    _stackFrameRegIndex = kInvalidReg;
    _isStackFrameRegPreserved = false;

    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(_stackFrameCopyGpIndex); i++)
      _stackFrameCopyGpIndex[i] = static_cast<uint8_t>(kInvalidReg);
  }

  //! Destroy the `X86FuncNode` instance.
  ASMJIT_INLINE ~X86FuncNode() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get function declaration as `X86FuncDecl`.
  ASMJIT_INLINE X86FuncDecl* getDecl() const noexcept {
    return const_cast<X86FuncDecl*>(&_x86Decl);
  }

  //! Get argument.
  ASMJIT_INLINE VarData* getArg(uint32_t i) const noexcept {
    ASMJIT_ASSERT(i < _x86Decl.getNumArgs());
    return static_cast<VarData*>(_args[i]);
  }

  //! Get registers which have to be saved in prolog/epilog.
  ASMJIT_INLINE uint32_t getSaveRestoreRegs(uint32_t rc) noexcept { return _saveRestoreRegs.get(rc); }

  //! Get stack size needed to align stack back to the nature alignment.
  ASMJIT_INLINE uint32_t getAlignStackSize() const noexcept { return _alignStackSize; }
  //! Set stack size needed to align stack back to the nature alignment.
  ASMJIT_INLINE void setAlignStackSize(uint32_t s) noexcept { _alignStackSize = s; }

  //! Get aligned stack size used by variables and memory allocated on the stack.
  ASMJIT_INLINE uint32_t getAlignedMemStackSize() const noexcept { return _alignedMemStackSize; }

  //! Get stack size used by push/pop sequences in prolog/epilog.
  ASMJIT_INLINE uint32_t getPushPopStackSize() const noexcept { return _pushPopStackSize; }
  //! Set stack size used by push/pop sequences in prolog/epilog.
  ASMJIT_INLINE void setPushPopStackSize(uint32_t s) noexcept { _pushPopStackSize = s; }

  //! Get stack size used by mov sequences in prolog/epilog.
  ASMJIT_INLINE uint32_t getMoveStackSize() const noexcept { return _moveStackSize; }
  //! Set stack size used by mov sequences in prolog/epilog.
  ASMJIT_INLINE void setMoveStackSize(uint32_t s) noexcept { _moveStackSize = s; }

  //! Get extra stack size.
  ASMJIT_INLINE uint32_t getExtraStackSize() const noexcept { return _extraStackSize; }
  //! Set extra stack size.
  ASMJIT_INLINE void setExtraStackSize(uint32_t s) noexcept { _extraStackSize  = s; }

  //! Get whether the function has stack frame register (only when the stack is misaligned).
  //!
  //! NOTE: Stack frame register can be used for both - aligning purposes or
  //! generating standard prolog/epilog sequence.
  ASMJIT_INLINE bool hasStackFrameReg() const noexcept {
    return _stackFrameRegIndex != kInvalidReg;
  }

  //! Get stack frame register index.
  //!
  //! NOTE: Used only when stack is misaligned.
  ASMJIT_INLINE uint32_t getStackFrameRegIndex() const noexcept {
    return _stackFrameRegIndex;
  }

  //! Get whether the stack frame register is preserved.
  //!
  //! NOTE: Used only when stack is misaligned.
  ASMJIT_INLINE bool isStackFrameRegPreserved() const noexcept {
    return static_cast<bool>(_isStackFrameRegPreserved);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! X86 function decl.
  X86FuncDecl _x86Decl;
  //! Registers which must be saved/restored in prolog/epilog.
  X86RegMask _saveRestoreRegs;

  //! Stack size needed to align function back to the nature alignment.
  uint32_t _alignStackSize;
  //! Like `_memStackSize`, but aligned.
  uint32_t _alignedMemStackSize;

  //! Stack required for push/pop in prolog/epilog (X86/X64 specific).
  uint32_t _pushPopStackSize;
  //! Stack required for movs in prolog/epilog (X86/X64 specific).
  uint32_t _moveStackSize;

  //! Stack required to put extra data (for example function arguments
  //! when manually aligning to requested alignment).
  uint32_t _extraStackSize;

  //! Stack frame register.
  uint8_t _stackFrameRegIndex;
  //! Whether the stack frame register is preserved.
  uint8_t _isStackFrameRegPreserved;
  //! Gp registers indexes that can be used to copy function arguments
  //! to a new location in case we are doing manual stack alignment.
  uint8_t _stackFrameCopyGpIndex[6];
};

// ============================================================================
// [asmjit::X86CallNode]
// ============================================================================

//! X86/X64 function-call node.
class X86CallNode : public HLCall {
 public:
  ASMJIT_NO_COPY(X86CallNode)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `X86CallNode` instance.
  ASMJIT_INLINE X86CallNode(Compiler* compiler, const Operand& target) noexcept : HLCall(compiler, target) {
    _decl = &_x86Decl;
    _usedArgs.reset();
  }

  //! Destroy the `X86CallNode` instance.
  ASMJIT_INLINE ~X86CallNode() noexcept {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the function prototype.
  ASMJIT_INLINE X86FuncDecl* getDecl() const noexcept {
    return const_cast<X86FuncDecl*>(&_x86Decl);
  }

  // --------------------------------------------------------------------------
  // [Prototype]
  // --------------------------------------------------------------------------

  //! Set function prototype.
  ASMJIT_INLINE Error setPrototype(const FuncPrototype& p) noexcept {
    return _x86Decl.setPrototype(p);
  }

  // --------------------------------------------------------------------------
  // [Arg / Ret]
  // --------------------------------------------------------------------------

  //! Set argument at `i` to `op`.
  ASMJIT_API bool _setArg(uint32_t i, const Operand& op) noexcept;
  //! Set return at `i` to `op`.
  ASMJIT_API bool _setRet(uint32_t i, const Operand& op) noexcept;

  //! Set argument at `i` to `var`.
  ASMJIT_INLINE bool setArg(uint32_t i, const Var& var) noexcept { return _setArg(i, var); }
  //! Set argument at `i` to `reg` (FP registers only).
  ASMJIT_INLINE bool setArg(uint32_t i, const X86FpReg& reg) noexcept { return _setArg(i, reg); }
  //! Set argument at `i` to `imm`.
  ASMJIT_INLINE bool setArg(uint32_t i, const Imm& imm) noexcept { return _setArg(i, imm); }

  //! Set return at `i` to `var`.
  ASMJIT_INLINE bool setRet(uint32_t i, const Var& var) noexcept { return _setRet(i, var); }
  //! Set return at `i` to `reg` (FP registers only).
  ASMJIT_INLINE bool setRet(uint32_t i, const X86FpReg& reg) noexcept { return _setRet(i, reg); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! X86 declaration.
  X86FuncDecl _x86Decl;
  //! Mask of all registers actually used to pass function arguments.
  //!
  //! NOTE: This bit-mask is not the same as `X86Func::_passed`. It contains
  //! only registers actually used to do the call while `X86Func::_passed`
  //! mask contains all registers for all function prototype combinations.
  X86RegMask _usedArgs;
};

// ============================================================================
// [asmjit::X86Compiler]
// ============================================================================

//! X86/X64 compiler.
//!
//! This class is used to store instruction stream and allows to modify it on
//! the fly. It uses a different concept than `Assembler` class and in fact
//! `Assembler` is only used as a backend. Compiler never emits machine code
//! directly, it stores instructions in a code-stream instead. This allows to
//! modify the code-stream later or and to apply various transformations to it.
//!
//! `X86Compiler` moves the code generation to a higher level. Higher level
//! constructs allow to write more abstract and extensible code that is not
//! possible with pure `X86Assembler` solution.
//!
//! The Story
//! ---------
//!
//! The compiler was created as a solution to bring higher level concepts into
//! a very low-level code generation. It started as an experiment to unify
//! the code generator for X86 and X86 architectures. These architectures are
//! built on the same ground, but use some concepts that are radically different
//! between X86 and X64. Basically the X64 architecture is a good evolution of
//! X86, because it offers much more registers and it added support for relative
//! addressing. Both architectures also use different ABI, which means that
//! function calling conventions are incompatible between each other (not just
//! between architectures, but also between OSes).
//!
//! This is a pain when it comes to low-level code generation. When AsmJit was
//! first published the main author's plan was to use it for a 2D pipeline
//! generation. In this task the main use of AsmJit was to combine several code
//! sections altogether without worrying about "which register should contain
//! what". This meant that a pure `X86Assembler` probably won't do the job
//! itself. Instead of hacking the `X86Assembler` to do more the `X86Compiler`
//! concept started to provide a layer that will consume instructions the same
//! way as `X86Assembler`, transform them, and serialize them to `X86Assembler`.
//!
//! The compiler concept evolved rapidly after the initial version and was
//! rewritten several times before it stabilized into the current form. It is
//! at the moment still evolving and it used to be the biggest source of bugs
//! in AsmJit in the past (doing non-trivial transformations has it's down-sides).
//!
//! The compiler at the moment uses linear-scan register allocation and can
//! look ahead to see which registers it should use. There are many limitations
//! at the moment, but if the resulting code doesn't use so much registers at
//! a same time it's pretty decent. However, please don't expect miracles, it
//! cannot compete with register allocators used in todays C++ compilers.
//!
//! Code Generation
//! ---------------
//!
//! The `X86Compiler` uses `X86Assembler` as a backend. It integrates with it,
//! which means that labels created by Assembler can be used by Compiler and
//! vice-versa. The following code shows the preferred and simplest way of
//! creating a compiler:
//!
//! ~~~
//! JitRuntime runtime;
//! X86Assembler a(&runtime);
//! X86Compiler c(&a);
//!
//! // ... use the compiler `c` ...
//!
//! c.finalize();
//! ~~~
//!
//! After the `finalize()` is called the compiler is detached from the assembler
//! and reset (cannot be used after finalization). It can be reattached again by
//! using `c.attach(&a)`, but the Compiler won't remember anything from the
//! previous code generation execution - it will be like creating a new instance
//! of `X86Compiler`.
//!
//! Functions
//! ---------
//!
//! See \ref asmjit::Compiler::addFunc().
//!
//! Variables
//! ---------
//!
//! Compiler has a built-in support for variables and assigning function
//! arguments. Variables are created by using `newXXX()` methods. If the
//! methods ends with `Var`, like `newXmmVar()` it accepts a variable type
//! as a first parameter. Variable type defines the layout and size of the
//! variable. It's the most important for general purpose registers, where
//! the variable type affects which instructions are generated when used as
//! operands. For example "mov eax, edx" is different than "mov rax, rdx",
//! but it's still the same "mov" instruction. Since the variable types are
//! verbose an alternative form to create variables easier was introduced.
//!
//! Instead of using `newGpVar(kVarTypeIntX, ...)` alternative forms like
//! `newIntX(...)` or `newUIntX(...)` can be used instead. Variables can
//! have a name so the code that creates a variable usually looks like
//! `newInt32("a")` or `newIntPtr("pInputBuffer")`, etc...
//!
//! Other register types like MMX or XMM have also alternative forms, so
//! for example `newMm("mmx")`, `newXmm("xmm")`, `newXmmPd("doubles")`, and
//! other forms can be used to create SIMD variables.
//!
//! Function arguments are associated with variables by using `setArg()`, where
//! the first parameter is argument index and the second parameter is the
//! variable instance. Function arguments can be a little bit tricky, because
//! asmjit allows to also define 64-bit arguments on a 32-bit architecture,
//! where the argument itself is split into two - lower-32 bit and higher 32-bit.
//! This applies also to a return value.
//!
//! The following snippet shows how to create a function and associate function
//! arguments with variables:
//!
//! ~~~
//! JitRuntime runtime;
//! X86Assembler a(&runtime);
//! X86Compiler c(&a);
//!
//! // Function prototype is "int function(int*, int*)" by using the host
//! // calling convention, which should be __cdecl in our case (if not
//! // configured to something else).
//! c.addFunc(FuncBuilder2<int, int*, int*>(kCallConvHost));
//!
//! // Associate function arguments.
//! X86GpVar pX = c.newIntPtr("pX");
//! X86GpVar pY = c.newIntPtr("pY");
//!
//! c.setArg(0, pX);
//! c.setArg(1, pY);
//!
//! // Do something useful :)
//! X86GpVar x = c.newInt32("x");
//! X86GpVar y = c.newInt32("y");
//!
//! c.mov(x, dword_ptr(pX));
//! c.add(y, dword_ptr(pY));
//!
//! // Return `x`.
//! c.ret(x);
//!
//! // End of the function body.
//! c.endFunc();
//!
//! // Finalize the compiler.
//! c.finalize();
//!
//! // Use the `X86Assembler` to assemble and relocate the function. It returns
//! // a pointer to the first byte of the code generated, which is the function
//! // entry point in our case.
//! typedef void (*MyFunc)(int*);
//! MyFunc func = asmjit_cast<MyFunc>(a.make());
//! ~~~
//!
//! The snippet uses methods to create variables, to associate them with
//! function arguments, and to use them to return from the generated function.
//!
//! When a variable is created, the initial state is `kVarStateNone`, when
//! it's allocated to the register or spilled to a memory it changes its
//! state to `kVarStateReg` or `kVarStateMem`, respectively. It's usual
//! during the variable that its state is changed multiple times. To generate
//! a better code, you can control explicitely the allocation and spilling:
//!
//!   - `alloc()` - Explicit method to alloc variable into register. It can be
//!     used to force allocation a variable before a loop for example.
//!
//!   - `spill()` - Explicit method to spill variable. If variable is in
//!     register and you call this method, it's moved to its home memory
//!     location. If the variable is not in register no operation is performed.
//!
//!   - `unuse()` - Unuse variable (you can use this to end the variable scope
//!     or sub-scope).
//!
//! List of X86/X64 variable types:
//!   - `kVarTypeInt8`     - Signed 8-bit integer, mapped to GPD register (eax, ebx, ...).
//!   - `kVarTypeUInt8`    - Unsigned 8-bit integer, mapped to GPD register (eax, ebx, ...).
//!   - `kVarTypeInt16`    - Signed 16-bit integer, mapped to GPD register (eax, ebx, ...).
//!   - `kVarTypeUInt16`   - Unsigned 16-bit integer, mapped to GPD register (eax, ebx, ...).
//!   - `kVarTypeInt32`    - Signed 32-bit integer, mapped to GPD register (eax, ebx, ...).
//!   - `kVarTypeUInt32`   - Unsigned 32-bit integer, mapped to GPD register (eax, ebx, ...).
//!   - `kVarTypeInt64`    - Signed 64-bit integer, mapped to GPQ register (rax, rbx, ...).
//!   - `kVarTypeUInt64`   - Unsigned 64-bit integer, mapped to GPQ register (rax, rbx, ...).
//!   - `kVarTypeIntPtr`   - intptr_t, mapped to GPD/GPQ register; depends on target, not host!
//!   - `kVarTypeUIntPtr`  - uintptr_t, mapped to GPD/GPQ register; depends on target, not host!
//!   - `kX86VarTypeMm`    - 64-bit MMX register (MM0, MM1, ...).
//!   - `kX86VarTypeXmm`   - 128-bit XMM register.
//!   - `kX86VarTypeXmmSs` - 128-bit XMM register that contains a scalar float.
//!   - `kX86VarTypeXmmSd` - 128-bit XMM register that contains a scalar double.
//!   - `kX86VarTypeXmmPs` - 128-bit XMM register that contains 4 packed floats.
//!   - `kX86VarTypeXmmPd` - 128-bit XMM register that contains 2 packed doubles.
//!   - `kX86VarTypeYmm`   - 256-bit YMM register.
//!   - `kX86VarTypeYmmPs` - 256-bit YMM register that contains 8 packed floats.
//!   - `kX86VarTypeYmmPd` - 256-bit YMM register that contains 4 packed doubles.
//!   - `kX86VarTypeZmm`   - 512-bit ZMM register.
//!   - `kX86VarTypeZmmPs` - 512-bit ZMM register that contains 16 packed floats.
//!   - `kX86VarTypeZmmPd` - 512-bit ZMM register that contains 8 packed doubles.
//!
//! List of X86/X64 variable states:
//!   - `kVarStateNone - State that is assigned to newly created variables or to
//!      not used variables (dereferenced to zero).
//!   - `kVarStateReg - State that means that variable is currently allocated in
//!      register.
//!   - `kVarStateMem - State that means that variable is currently only in
//!      memory location.
//!
//! Memory Management
//! -----------------
//!
//! Compiler Memory management follows these rules:
//!
//!   - Everything created by `X86Compiler` is always freed by `X86Compiler`.
//!   - To get a decent performance, compiler always uses large memory buffers
//!     to allocate objects. When the compiler is destroyed, it invalidates all
//!     objects that it created.
//!   - This type of memory management is called 'zone memory management'.
//!
//! In other words, anything that returns a pointer to something cannot be
//! used after the compiler was destroyed. However, since compiler integrates
//! with assembler, labels created by Compiler can be used by Assembler or
//! another Compiler attached to it.
//!
//! Control-Flow and State Management
//! ---------------------------------
//!
//! The `X86Compiler` automatically manages state of all variables when using
//! control flow instructions like jumps, conditional jumps and function calls.
//!
//! In general the internal state can be changed only when using jump or
//! conditional jump. When using non-conditional jump the state change is
//! embedded before the jump itself, so there is basically zero overhead.
//! However, conditional jumps are more complicated and the compiler can
//! generate in some cases a block at the end of the function that changes
//! the state of one branch. Usually the "taken" branch is embedded directly
//! before the jump, and "not-taken" branch has the separate code block.
//!
//! The next example shows to the extra code block generated for a state change:
//!
//! ~~~
//! JitRuntime runtime;
//! X86Assembler a(&runtime);
//! X86Compiler c(&a);
//!
//! c.addFunc(FuncBuilder0<Void>(kCallConvHost));
//!
//! Label L0 = c.newLabel();
//! X86GpVar x = c.newInt32("x");
//! X86GpVar y = c.newInt32("y");
//!
//! // After these two lines, `x` and `y` will be always stored in registers:
//! //   x - register.
//! //   y - register.
//! c.xor_(x, x);
//! c.xor_(y, y);
//! c.cmp(x, y);
//!
//! // Manually spill `x` and `y`:
//! //   x - memory.
//! //   y - memory.
//! c.spill(x);
//! c.spill(y);
//!
//! // Conditional jump to L0. It will be always taken, but the compiler thinks
//! // that it is unlikely to be taken so it will embed the state-change code
//! // somewhere else.
//! c.je(L0);
//!
//! // Do something. The variables `x` and `y` will be allocated again.
//! //   `x` - register.
//! //   `y` - register.
//! c.add(x, 1);
//! c.add(y, 2);
//!
//! // Bind a label here, the state is not changed.
//! //   `x` - register.
//! //   `y` - register.
//! c.bind(L0);
//!
//! // Use `x` and `y`, because the compiler knows the life-time and can
//! // eliminate the state change of dead variables.
//! //   `x` - register.
//! //   `y` - register.
//! c.sub(x, y);
//!
//! c.endFunc();
//! ~~~
//!
//! The output:
//!
//! ~~~
//! xor eax, eax                    ; xor x, x
//! xor ecx, ecx                    ; xor y, y
//! cmp eax, ecx                    ; cmp x, y
//! mov [esp - 24], eax             ; spill x
//! mov [esp - 28], ecx             ; spill x
//! je L0_Switch
//! mov eax, [esp - 24]             ; alloc x
//! add eax, 1                      ; add x, 1
//! mov ecx, [esp - 28]             ; alloc y
//! add ecx, 2                      ; add y, 2
//! L0:
//! sub eax, ecx                    ; sub x, y
//! ret
//!
//! ; state-switch begin
//! L0_Switch0:
//! mov eax, [esp - 24]             ; alloc x
//! mov ecx, [esp - 28]             ; alloc y
//! jmp short L0
//! ; state-switch end
//! ~~~
//!
//! As can be seen, the state-switch section was generated (L0_Switch0). The
//! compiler was unable to restore the state immediately when emitting the
//! forward jump (the code is generated from the first to last instruction
//! and the target state is simply not known at this time).
//!
//! To tell the compiler to embed the state-switch code before the jump it's
//! needed to create a backward jump (where also processor expects that it
//! will be taken). A slightly modified code is used to demonstrate the
//! possibility to embed the state-switch before the jump:
//!
//! ~~~
//! JitRuntime runtime;
//! X86Assembler a(&runtime);
//! Compiler c(&a);
//!
//! c.addFunc(FuncBuilder0<Void>(kCallConvHost));
//!
//! Label L0 = c.newLabel();
//! X86GpVar x = c.newInt32("x");
//! X86GpVar y = c.newInt32("y");
//!
//! // After these two lines, `x` and `y` will be always stored in registers.
//! //   `x` - register.
//! //   `y` - register.
//! c.xor_(x, x);
//! c.xor_(y, y);
//!
//! // Manually spill `x` and `y`.
//! //   `x` - memory.
//! //   `y` - memory.
//! c.spill(x);
//! c.spill(y);
//!
//! // Bind a label here, the state is not changed.
//! //   `x` - memory.
//! //   `y` - memory.
//! c.bind(L0);
//!
//! // Do something, the variables will be allocated again.
//! c.add(x, 1);
//! c.add(y, 2);
//! // State:
//! //   `x` - register.
//! //   `y` - register.
//!
//! // Backward conditional jump to L0. The default behavior is that it
//! // will be taken so the state-change code will be embedded here.
//! c.je(L0);
//!
//! c.endFunc();
//! ~~~
//!
//! The output:
//!
//! ~~~
//! xor ecx, ecx                    ; xor x, x
//! xor edx, edx                    ; xor y, y
//! mov [esp - 24], ecx             ; spill x
//! mov [esp - 28], edx             ; spill y
//! L2:
//! mov ecx, [esp - 24]             ; alloc x
//! add ecx, 1                      ; add x, 1
//! mov edx, [esp - 28]             ; alloc y
//! add edx, 2                      ; add y, 2
//!
//! ; state-switch begin
//! mov [esp - 24], ecx             ; spill x
//! mov [esp - 28], edx             ; spill y
//! ; state-switch end
//!
//! je short L2
//! ret
//! ~~~
//!
//! Please note where the state-switch sections are located in both examples.
//! To inform the compiler which branch is likely to be taken use the following
//! options:
//!   - `kInstOptionTaken` - The conditional jump is likely to be taken.
//!   - `kInstOptionNotTaken` - The conditional jump is unlikely to be taken.
//!
//! Both options can be used by simply using `taken()` and/or `notTaken()`. The
//! example above could be changed to `c.taken().je(L0)`, which would generate
//! the following output:
//!
//! ~~~
//! xor ecx, ecx                    ; xor x, x
//! xor edx, edx                    ; xor y, y
//! mov [esp - 24], ecx             ; spill x
//! mov [esp - 28], edx             ; spill y
//! L0:
//! mov ecx, [esp - 24]             ; alloc x
//! add ecx, 1                      ; add x, 1
//! mov edx, [esp - 28]             ; alloc y
//! add edx, 2                      ; add y, 2
//! je L0_Switch, 2
//! ret
//!
//! ; state-switch begin
//! L0_Switch:
//! mov [esp - 24], ecx             ; spill x
//! mov [esp - 28], edx             ; spill y
//! jmp short L0
//! ; state-switch end
//! ~~~
//!
//! This section provided information of how the state-change works. The
//! behavior is deterministic and can be overridden manually if needed.
//!
//! Advanced Code Generation
//! ------------------------
//!
//! This section describes an advanced method of code generation available in
//! assembler and compiler. Every instruction supported by AsmJit has its ID,
//! which can be used with method `emit()` instead of using compiler's intrinsics.
//! For example `mov(x, y)` is an equivalent to `emit(kX86InstIdMov, x, y)`.
//! The later is, however, not type-safe and C++ compiler won't help you to
//! detect some bugs at compile time. On the other hand the later allows to
//! generate some code programatically without using if/else constructs.
//!
//! There are many use-cases where the unsafe API can be used, for example:
//!
//! ~~~
//! uint32_t translateOp(const char* op) {
//!   if (strcmp(op, "add")) return kX86InstIdAddsd;
//!   if (strcmp(op, "sub")) return kX86InstIdSubsd;
//!   if (strcmp(op, "mul")) return kX86InstIdMulsd;
//!   if (strcmp(op, "div")) return kX86InstIdDivsd;
//!
//!   return kInstIdNone;
//! }
//!
//! void emitArith(X86Compiler& c, const char* op, const X86XmmVar& a, const X86XmmVar& b) {
//!   uint32_t instId = translateOp(op);
//!   if (instId != kInstIdNone)
//!     c.emit(instId, a, b);
//! }
//! ~~~
//!
//! Other use cases are waiting for you! Be sure that instruction that are
//! being emitted are correct and encodable, otherwise the Assembler will
//! fail and set the status code to `kErrorUnknownInst`.
class ASMJIT_VIRTAPI X86Compiler : public Compiler {
 public:
  ASMJIT_NO_COPY(X86Compiler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `X86Compiler` instance.
  ASMJIT_API X86Compiler(X86Assembler* assembler = nullptr) noexcept;
  //! Destroy the `X86Compiler` instance.
  ASMJIT_API ~X86Compiler() noexcept;

  // --------------------------------------------------------------------------
  // [Attach / Reset]
  // --------------------------------------------------------------------------

  //! \override
  ASMJIT_API virtual Error attach(Assembler* assembler) noexcept;
  //! \override
  ASMJIT_API virtual void reset(bool releaseMemory) noexcept;

  // -------------------------------------------------------------------------
  // [Finalize]
  // -------------------------------------------------------------------------

  ASMJIT_API virtual Error finalize() noexcept;

  // --------------------------------------------------------------------------
  // [Assembler]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE X86Assembler* getAssembler() const noexcept {
    return static_cast<X86Assembler*>(_assembler);
  }

  // --------------------------------------------------------------------------
  // [Arch]
  // --------------------------------------------------------------------------

  //! Get count of registers of the current architecture and mode.
  ASMJIT_INLINE const X86RegCount& getRegCount() const noexcept { return _regCount; }

  //! Get GPD or GPQ register depending on the current architecture.
  ASMJIT_INLINE X86GpReg gpz(uint32_t index) const noexcept { return X86GpReg(zax, index); }

  //! Create an architecture dependent intptr_t memory operand.
  ASMJIT_INLINE X86Mem intptr_ptr(const X86GpReg& base, int32_t disp = 0) const noexcept {
    return x86::ptr(base, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const X86GpReg& base, const X86GpReg& index, uint32_t shift = 0, int32_t disp = 0) const noexcept {
    return x86::ptr(base, index, shift, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const Label& label, int32_t disp = 0) const noexcept {
    return x86::ptr(label, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const Label& label, const X86GpReg& index, uint32_t shift, int32_t disp = 0) const noexcept {
    return x86::ptr(label, index, shift, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const X86RipReg& rip, int32_t disp = 0) const noexcept {
    return x86::ptr(rip, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr_abs(Ptr pAbs, int32_t disp = 0) const noexcept {
    return x86::ptr_abs(pAbs, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr_abs(Ptr pAbs, const X86GpReg& index, uint32_t shift, int32_t disp = 0) const noexcept {
    return x86::ptr_abs(pAbs, index, shift, disp, zax.getSize());
  }

  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const X86GpVar& base, int32_t disp = 0) noexcept {
    return x86::ptr(base, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const X86GpVar& base, const X86GpVar& index, uint32_t shift = 0, int32_t disp = 0) noexcept {
    return x86::ptr(base, index, shift, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr(const Label& label, const X86GpVar& index, uint32_t shift, int32_t disp = 0) noexcept {
    return x86::ptr(label, index, shift, disp, zax.getSize());
  }
  //! \overload
  ASMJIT_INLINE X86Mem intptr_ptr_abs(Ptr pAbs, const X86GpVar& index, uint32_t shift, int32_t disp = 0) noexcept {
    return x86::ptr_abs(pAbs, index, shift, disp, zax.getSize());
  }

  // --------------------------------------------------------------------------
  // [Inst / Emit]
  // --------------------------------------------------------------------------

  //! Create a new `HLInst`.
  ASMJIT_API HLInst* newInst(uint32_t code) noexcept;
  //! \overload
  ASMJIT_API HLInst* newInst(uint32_t code, const Operand& o0) noexcept;
  //! \overload
  ASMJIT_API HLInst* newInst(uint32_t code, const Operand& o0, const Operand& o1) noexcept;
  //! \overload
  ASMJIT_API HLInst* newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2) noexcept;
  //! \overload
  ASMJIT_API HLInst* newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) noexcept;
  //! \overload
  ASMJIT_API HLInst* newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3, const Operand& o4) noexcept;

  //! Add a new `HLInst`.
  ASMJIT_API HLInst* emit(uint32_t code) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3, const Operand& o4) noexcept;

  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, int o0) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, uint64_t o0) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, int o1) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, uint64_t o1) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, int o2) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, uint64_t o2) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int o3) noexcept;
  //! \overload
  ASMJIT_API HLInst* emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, uint64_t o3) noexcept;

  // --------------------------------------------------------------------------
  // [Func]
  // --------------------------------------------------------------------------

  //! Create a new `X86FuncNode`.
  ASMJIT_API X86FuncNode* newFunc(const FuncPrototype& p) noexcept;

  using Compiler::addFunc;

  //! Add a new function.
  //!
  //! \param p Function prototype.
  //!
  //! This method is usually used as a first step used to generate a dynamic
  //! function. The prototype `p` contains a function calling convention,
  //! return value, and parameters. There are some helper classes that simplify
  //! function prototype building, see `FuncBuilder0<...>`, `FuncBuilder1<...>`,
  //! `FuncBuilder2<...>`, etc...
  //!
  //! Templates with `FuncBuilder` prefix are used to generate a function
  //! prototype based on real C++ types. See the next example that shows how
  //! to generate a function with two 32-bit integer arguments.
  //!
  //! ~~~
  //! JitRuntime runtime;
  //! X86Assembler a(&runtime);
  //! X86Compiler c(&a);
  //!
  //! // Add a function - <return value, arguments>.
  //! c.addFunc(FuncBuilder2<Void, int, int>(kCallConvHost));
  //!
  //! // ... body ...
  //!
  //! // End of the function.
  //! c.endFunc();
  //! ~~~
  //!
  //! Building functions is really easy! The code snippet above can be used
  //! to generate a function with two `int32_t` arguments. To assign a variable
  //! to a function argument use `c.setArg(index, variable)`.
  //!
  //! ~~~
  //! JitRuntime runtime;
  //! X86Assembler a(&runtime);
  //! X86Compiler c(&a);
  //!
  //! X86GpVar arg0 = c.newInt32("arg0");
  //! X86GpVar arg1 = c.newInt32("arg1");
  //!
  //! // Add a function - <return value, arguments>.
  //! c.addFunc(FuncBuilder2<Void, int, int>(kCallConvHost));
  //!
  //! c.setArg(0, arg0);
  //! c.setArg(1, arg1);
  //!
  //! // ... do something ...
  //! c.add(arg0, arg1);
  //!
  //! // End of the function.
  //! c.endFunc();
  //! ~~~
  //!
  //! Arguments are like variables. How to manipulate with variables is
  //! documented in a variables section of `X86Compiler` documentation.
  //!
  //! NOTE: To get the current function use `getFunc()` method.
  //!
  //! \sa \ref FuncBuilder0, \ref FuncBuilder1, \ref FuncBuilder2.
  ASMJIT_API X86FuncNode* addFunc(const FuncPrototype& p) noexcept;

  //! Emit a sentinel that marks the end of the current function.
  ASMJIT_API HLSentinel* endFunc() noexcept;

  //! Get the current function node casted to `X86FuncNode`.
  //!
  //! This method can be called within `addFunc()` and `endFunc()` block to get
  //! current function you are working with. It's recommended to store `HLFunc`
  //! pointer returned by `addFunc<>` method, because this allows you in future
  //! implement function sections outside of the function itself.
  ASMJIT_INLINE X86FuncNode* getFunc() const noexcept {
    return static_cast<X86FuncNode*>(_func);
  }

  // --------------------------------------------------------------------------
  // [Ret]
  // --------------------------------------------------------------------------

  //! Create a new `HLRet`.
  ASMJIT_API HLRet* newRet(const Operand& o0, const Operand& o1) noexcept;
  //! Add a new `HLRet`.
  ASMJIT_API HLRet* addRet(const Operand& o0, const Operand& o1) noexcept;

  // --------------------------------------------------------------------------
  // [Call]
  // --------------------------------------------------------------------------

  //! Create a new `X86CallNode`.
  ASMJIT_API X86CallNode* newCall(const Operand& o0, const FuncPrototype& p) noexcept;
  //! Add a new `X86CallNode`.
  ASMJIT_API X86CallNode* addCall(const Operand& o0, const FuncPrototype& p) noexcept;

  // --------------------------------------------------------------------------
  // [Args]
  // --------------------------------------------------------------------------

  //! Set function argument to `var`.
  ASMJIT_API Error setArg(uint32_t argIndex, const Var& var) noexcept;

  // --------------------------------------------------------------------------
  // [Vars]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _newVar(Var* var, uint32_t vType, const char* name) noexcept;
  ASMJIT_API Error _newVar(Var* var, uint32_t vType, const char* fmt, va_list ap) noexcept;

#if !defined(ASMJIT_DISABLE_LOGGER)
#define ASMJIT_NEW_VAR_TYPE_EX(func, type, typeFirst, typeLast) \
  ASMJIT_NOINLINE type new##func(uint32_t vType, const char* name, ...) { \
    ASMJIT_ASSERT(vType < kX86VarTypeCount); \
    ASMJIT_ASSERT(Utils::inInterval<uint32_t>(vType, typeFirst, typeLast)); \
    \
    type var(NoInit); \
    va_list ap; \
    va_start(ap, name); \
    \
    _newVar(&var, vType, name, ap); \
    \
    va_end(ap); \
    return var; \
  }
#define ASMJIT_NEW_VAR_AUTO_EX(func, type, typeId) \
  ASMJIT_NOINLINE type new##func(const char* name, ...) { \
    type var(NoInit); \
    va_list ap; \
    va_start(ap, name); \
    \
    _newVar(&var, typeId, name, ap); \
    \
    va_end(ap); \
    return var; \
  }
#else
#define ASMJIT_NEW_VAR_TYPE_EX(func, type, typeFirst, typeLast) \
  ASMJIT_NOINLINE type new##func(uint32_t vType, const char* name, ...) { \
    ASMJIT_ASSERT(vType < kX86VarTypeCount); \
    ASMJIT_ASSERT(Utils::inInterval<uint32_t>(vType, typeFirst, typeLast)); \
    \
    type var(NoInit); \
    _newVar(&var, vType, nullptr); \
    return var; \
  }
#define ASMJIT_NEW_VAR_AUTO_EX(func, type, typeId) \
  ASMJIT_NOINLINE type new##func(const char* name, ...) { \
    type var(NoInit); \
    _newVar(&var, typeId, nullptr); \
    return var; \
  }
#endif

#define ASMJIT_REGISTER_VAR_TYPE(func, type, typeFirst, typeLast) \
  ASMJIT_INLINE type get##func##ById(uint32_t vType, uint32_t id) { \
    ASMJIT_ASSERT(vType < kX86VarTypeCount); \
    ASMJIT_ASSERT(Utils::inInterval<uint32_t>(vType, typeFirst, typeLast)); \
    \
    type var(NoInit); \
    \
    vType = _targetVarMapping[vType]; \
    const VarInfo& vInfo = _x86VarInfo[vType]; \
    \
    var._init_packed_op_sz_w0_id(Operand::kTypeVar, vInfo.getSize(), vInfo.getRegType() << 8, id); \
    var._vreg.vType = vType; \
    \
    return var; \
  } \
  \
  ASMJIT_INLINE type new##func(uint32_t vType) { \
    ASMJIT_ASSERT(vType < kX86VarTypeCount); \
    ASMJIT_ASSERT(Utils::inInterval<uint32_t>(vType, typeFirst, typeLast)); \
    \
    type var(NoInit); \
    _newVar(&var, vType, nullptr); \
    return var; \
  } \
  \
  ASMJIT_NEW_VAR_TYPE_EX(func, type, typeFirst, typeLast)

#define ASMJIT_REGISTER_VAR_AUTO(func, type, typeId) \
  ASMJIT_INLINE type get##func##ById(uint32_t id) { \
    type var(NoInit); \
    \
    uint32_t vType = _targetVarMapping[typeId]; \
    const VarInfo& vInfo = _x86VarInfo[vType]; \
    \
    var._init_packed_op_sz_w0_id(Operand::kTypeVar, vInfo.getSize(), vInfo.getRegType() << 8, id); \
    var._vreg.vType = vType; \
    \
    return var; \
  } \
  \
  ASMJIT_INLINE type new##func() { \
    type var(NoInit); \
    _newVar(&var, typeId, nullptr); \
    return var; \
  } \
  \
  ASMJIT_NEW_VAR_AUTO_EX(func, type, typeId)

  ASMJIT_REGISTER_VAR_TYPE(GpVar  , X86GpVar , _kVarTypeIntStart   , _kVarTypeIntEnd   )
  ASMJIT_REGISTER_VAR_TYPE(MmVar  , X86MmVar , _kX86VarTypeMmStart , _kX86VarTypeMmEnd )
  ASMJIT_REGISTER_VAR_TYPE(XmmVar , X86XmmVar, _kX86VarTypeXmmStart, _kX86VarTypeXmmEnd)
  ASMJIT_REGISTER_VAR_TYPE(YmmVar , X86YmmVar, _kX86VarTypeYmmStart, _kX86VarTypeYmmEnd)

  ASMJIT_REGISTER_VAR_AUTO(Int8   , X86GpVar , kVarTypeInt8    )
  ASMJIT_REGISTER_VAR_AUTO(Int16  , X86GpVar , kVarTypeInt16   )
  ASMJIT_REGISTER_VAR_AUTO(Int32  , X86GpVar , kVarTypeInt32   )
  ASMJIT_REGISTER_VAR_AUTO(Int64  , X86GpVar , kVarTypeInt64   )
  ASMJIT_REGISTER_VAR_AUTO(IntPtr , X86GpVar , kVarTypeIntPtr  )
  ASMJIT_REGISTER_VAR_AUTO(UInt8  , X86GpVar , kVarTypeUInt8   )
  ASMJIT_REGISTER_VAR_AUTO(UInt16 , X86GpVar , kVarTypeUInt16  )
  ASMJIT_REGISTER_VAR_AUTO(UInt32 , X86GpVar , kVarTypeUInt32  )
  ASMJIT_REGISTER_VAR_AUTO(UInt64 , X86GpVar , kVarTypeUInt64  )
  ASMJIT_REGISTER_VAR_AUTO(UIntPtr, X86GpVar , kVarTypeUIntPtr )
  ASMJIT_REGISTER_VAR_AUTO(Mm     , X86MmVar , kX86VarTypeMm   )
  ASMJIT_REGISTER_VAR_AUTO(Xmm    , X86XmmVar, kX86VarTypeXmm  )
  ASMJIT_REGISTER_VAR_AUTO(XmmSs  , X86XmmVar, kX86VarTypeXmmSs)
  ASMJIT_REGISTER_VAR_AUTO(XmmSd  , X86XmmVar, kX86VarTypeXmmSd)
  ASMJIT_REGISTER_VAR_AUTO(XmmPs  , X86XmmVar, kX86VarTypeXmmPs)
  ASMJIT_REGISTER_VAR_AUTO(XmmPd  , X86XmmVar, kX86VarTypeXmmPd)
  ASMJIT_REGISTER_VAR_AUTO(Ymm    , X86YmmVar, kX86VarTypeYmm  )
  ASMJIT_REGISTER_VAR_AUTO(YmmPs  , X86YmmVar, kX86VarTypeYmmPs)
  ASMJIT_REGISTER_VAR_AUTO(YmmPd  , X86YmmVar, kX86VarTypeYmmPd)

#undef ASMJIT_NEW_VAR_AUTO
#undef ASMJIT_NEW_VAR_TYPE

  // --------------------------------------------------------------------------
  // [Stack]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error _newStack(BaseMem* mem, uint32_t size, uint32_t alignment, const char* name) noexcept;

  //! Create a new memory chunk allocated on the current function's stack.
  ASMJIT_INLINE X86Mem newStack(uint32_t size, uint32_t alignment, const char* name = nullptr) noexcept {
    X86Mem m(NoInit);
    _newStack(&m, size, alignment, name);
    return m;
  }

  // --------------------------------------------------------------------------
  // [Const]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error _newConst(BaseMem* mem, uint32_t scope, const void* data, size_t size) noexcept;

  //! Put data to a constant-pool and get a memory reference to it.
  ASMJIT_INLINE X86Mem newConst(uint32_t scope, const void* data, size_t size) noexcept {
    X86Mem m(NoInit);
    _newConst(&m, scope, data, size);
    return m;
  }

  //! Put a BYTE `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newByteConst(uint32_t scope, uint8_t val) noexcept { return newConst(scope, &val, 1); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newWordConst(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newDWordConst(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newQWordConst(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newInt16Const(uint32_t scope, int16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a WORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newUInt16Const(uint32_t scope, uint16_t val) noexcept { return newConst(scope, &val, 2); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newInt32Const(uint32_t scope, int32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DWORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newUInt32Const(uint32_t scope, uint32_t val) noexcept { return newConst(scope, &val, 4); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newInt64Const(uint32_t scope, int64_t val) noexcept { return newConst(scope, &val, 8); }
  //! Put a QWORD `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newUInt64Const(uint32_t scope, uint64_t val) noexcept { return newConst(scope, &val, 8); }

  //! Put a SP-FP `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newFloatConst(uint32_t scope, float val) noexcept { return newConst(scope, &val, 4); }
  //! Put a DP-FP `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newDoubleConst(uint32_t scope, double val) noexcept { return newConst(scope, &val, 8); }

  //! Put a MMX `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newMmConst(uint32_t scope, const Vec64& val) noexcept { return newConst(scope, &val, 8); }
  //! Put a XMM `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newXmmConst(uint32_t scope, const Vec128& val) noexcept { return newConst(scope, &val, 16); }
  //! Put a YMM `val` to a constant-pool.
  ASMJIT_INLINE X86Mem newYmmConst(uint32_t scope, const Vec256& val) noexcept { return newConst(scope, &val, 32); }

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! Add 8-bit integer data to the instruction stream.
  ASMJIT_INLINE Error db(uint8_t x) noexcept { return embed(&x, 1); }
  //! Add 16-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dw(uint16_t x) noexcept { return embed(&x, 2); }
  //! Add 32-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dd(uint32_t x) noexcept { return embed(&x, 4); }
  //! Add 64-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dq(uint64_t x) noexcept { return embed(&x, 8); }

  //! Add 8-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dint8(int8_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(int8_t))); }
  //! Add 8-bit integer data to the instruction stream.
  ASMJIT_INLINE Error duint8(uint8_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(uint8_t))); }

  //! Add 16-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dint16(int16_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(int16_t))); }
  //! Add 16-bit integer data to the instruction stream.
  ASMJIT_INLINE Error duint16(uint16_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(uint16_t))); }

  //! Add 32-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dint32(int32_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(int32_t))); }
  //! Add 32-bit integer data to the instruction stream.
  ASMJIT_INLINE Error duint32(uint32_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(uint32_t))); }

  //! Add 64-bit integer data to the instruction stream.
  ASMJIT_INLINE Error dint64(int64_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(int64_t))); }
  //! Add 64-bit integer data to the instruction stream.
  ASMJIT_INLINE Error duint64(uint64_t x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(uint64_t))); }

  //! Add float data to the instruction stream.
  ASMJIT_INLINE Error dfloat(float x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(float))); }
  //! Add double data to the instruction stream.
  ASMJIT_INLINE Error ddouble(double x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(double))); }

  //! Add MMX data to the instruction stream.
  ASMJIT_INLINE Error dmm(const Vec64& x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(Vec64))); }
  //! Add XMM data to the instruction stream.
  ASMJIT_INLINE Error dxmm(const Vec128& x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(Vec128))); }
  //! Add YMM data to the instruction stream.
  ASMJIT_INLINE Error dymm(const Vec256& x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(Vec256))); }

  //! Add data in a given structure instance to the instruction stream.
  template<typename T>
  ASMJIT_INLINE Error dstruct(const T& x) noexcept { return embed(&x, static_cast<uint32_t>(sizeof(T))); }

  // -------------------------------------------------------------------------
  // [Instruction Options]
  // -------------------------------------------------------------------------

  ASMJIT_X86_EMIT_OPTIONS(X86Compiler)

  //! Force the compiler to not follow the conditional or unconditional jump.
  ASMJIT_INLINE X86Compiler& unfollow() noexcept {
    _instOptions |= kInstOptionUnfollow;
    return *this;
  }

  //! Tell the compiler that the destination variable will be overwritten.
  ASMJIT_INLINE X86Compiler& overwrite() noexcept {
    _instOptions |= kInstOptionOverwrite;
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Count of registers depending on the current architecture.
  X86RegCount _regCount;

  //! EAX or RAX register depending on the current architecture.
  X86GpReg zax;
  //! ECX or RCX register depending on the current architecture.
  X86GpReg zcx;
  //! EDX or RDX register depending on the current architecture.
  X86GpReg zdx;
  //! EBX or RBX register depending on the current architecture.
  X86GpReg zbx;
  //! ESP or RSP register depending on the current architecture.
  X86GpReg zsp;
  //! EBP or RBP register depending on the current architecture.
  X86GpReg zbp;
  //! ESI or RSI register depending on the current architecture.
  X86GpReg zsi;
  //! EDI or RDI register depending on the current architecture.
  X86GpReg zdi;

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

#define INST_0x(_Inst_, _Code_) \
  ASMJIT_INLINE HLInst* _Inst_() noexcept { \
    return emit(_Code_); \
  }

#define INST_1x(_Inst_, _Code_, _Op0_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0) noexcept { \
    return emit(_Code_, o0); \
  }

#define INST_1i(_Inst_, _Code_, _Op0_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0) noexcept { \
    return emit(_Code_, o0); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(int o0) noexcept { \
    return emit(_Code_, o0); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(unsigned int o0) noexcept { \
    return emit(_Code_, static_cast<uint64_t>(o0)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(int64_t o0) noexcept { \
    return emit(_Code_, static_cast<uint64_t>(o0)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(uint64_t o0) noexcept { \
    return emit(_Code_, o0); \
  }

#define INST_1cc(_Inst_, _Code_, _Translate_, _Op0_) \
  ASMJIT_INLINE HLInst* _Inst_(uint32_t cc, const _Op0_& o0) noexcept { \
    return emit(_Translate_(cc), o0); \
  } \
  \
  ASMJIT_INLINE HLInst* _Inst_##a(const _Op0_& o0) noexcept { return emit(_Code_##a, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##ae(const _Op0_& o0) noexcept { return emit(_Code_##ae, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##b(const _Op0_& o0) noexcept { return emit(_Code_##b, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##be(const _Op0_& o0) noexcept { return emit(_Code_##be, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##c(const _Op0_& o0) noexcept { return emit(_Code_##c, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##e(const _Op0_& o0) noexcept { return emit(_Code_##e, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##g(const _Op0_& o0) noexcept { return emit(_Code_##g, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##ge(const _Op0_& o0) noexcept { return emit(_Code_##ge, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##l(const _Op0_& o0) noexcept { return emit(_Code_##l, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##le(const _Op0_& o0) noexcept { return emit(_Code_##le, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##na(const _Op0_& o0) noexcept { return emit(_Code_##na, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nae(const _Op0_& o0) noexcept { return emit(_Code_##nae, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nb(const _Op0_& o0) noexcept { return emit(_Code_##nb, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nbe(const _Op0_& o0) noexcept { return emit(_Code_##nbe, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nc(const _Op0_& o0) noexcept { return emit(_Code_##nc, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##ne(const _Op0_& o0) noexcept { return emit(_Code_##ne, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##ng(const _Op0_& o0) noexcept { return emit(_Code_##ng, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nge(const _Op0_& o0) noexcept { return emit(_Code_##nge, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nl(const _Op0_& o0) noexcept { return emit(_Code_##nl, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nle(const _Op0_& o0) noexcept { return emit(_Code_##nle, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##no(const _Op0_& o0) noexcept { return emit(_Code_##no, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##np(const _Op0_& o0) noexcept { return emit(_Code_##np, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##ns(const _Op0_& o0) noexcept { return emit(_Code_##ns, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##nz(const _Op0_& o0) noexcept { return emit(_Code_##nz, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##o(const _Op0_& o0) noexcept { return emit(_Code_##o, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##p(const _Op0_& o0) noexcept { return emit(_Code_##p, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##pe(const _Op0_& o0) noexcept { return emit(_Code_##pe, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##po(const _Op0_& o0) noexcept { return emit(_Code_##po, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##s(const _Op0_& o0) noexcept { return emit(_Code_##s, o0); } \
  ASMJIT_INLINE HLInst* _Inst_##z(const _Op0_& o0) noexcept { return emit(_Code_##z, o0); }

#define INST_2x(_Inst_, _Code_, _Op0_, _Op1_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1) noexcept { \
    return emit(_Code_, o0, o1); \
  }

#define INST_2i(_Inst_, _Code_, _Op0_, _Op1_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1) noexcept { \
    return emit(_Code_, o0, o1); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, int o1) noexcept { \
    return emit(_Code_, o0, o1); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, unsigned int o1) noexcept { \
    return emit(_Code_, o0, static_cast<uint64_t>(o1)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, int64_t o1) noexcept { \
    return emit(_Code_, o0, static_cast<uint64_t>(o1)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, uint64_t o1) noexcept { \
    return emit(_Code_, o0, o1); \
  }

#define INST_2cc(_Inst_, _Code_, _Translate_, _Op0_, _Op1_) \
  ASMJIT_INLINE HLInst* _Inst_(uint32_t cc, const _Op0_& o0, const _Op1_& o1) noexcept { \
    return emit(_Translate_(cc), o0, o1); \
  } \
  \
  ASMJIT_INLINE HLInst* _Inst_##a(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##a, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##ae(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##ae, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##b(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##b, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##be(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##be, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##c(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##c, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##e(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##e, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##g(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##g, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##ge(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##ge, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##l(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##l, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##le(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##le, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##na(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##na, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nae(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nae, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nb(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nb, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nbe(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nbe, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nc(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nc, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##ne(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##ne, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##ng(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##ng, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nge(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nge, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nl(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nl, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nle(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nle, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##no(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##no, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##np(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##np, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##ns(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##ns, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##nz(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##nz, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##o(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##o, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##p(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##p, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##pe(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##pe, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##po(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##po, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##s(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##s, o0, o1); } \
  ASMJIT_INLINE HLInst* _Inst_##z(const _Op0_& o0, const _Op1_& o1) noexcept { return emit(_Code_##z, o0, o1); }

#define INST_3x(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) noexcept { \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3i(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) noexcept { \
    return emit(_Code_, o0, o1, o2); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, int o2) noexcept { \
    return emit(_Code_, o0, o1, o2); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, unsigned int o2) noexcept { \
    return emit(_Code_, o0, o1, static_cast<uint64_t>(o2)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, int64_t o2) noexcept { \
    return emit(_Code_, o0, o1, static_cast<uint64_t>(o2)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, uint64_t o2) noexcept { \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3ii(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) noexcept { \
    return emit(_Code_, o0, o1, o2); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, int o1, int o2) noexcept { \
    Imm o1Imm(o1); \
    return emit(_Code_, o0, o1Imm, o2); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, unsigned int o1, unsigned int o2) noexcept { \
    Imm o1Imm(o1); \
    return emit(_Code_, o0, o1Imm, static_cast<uint64_t>(o2)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, int64_t o1, int64_t o2) noexcept { \
    Imm o1Imm(o1); \
    return emit(_Code_, o0, o1Imm, static_cast<uint64_t>(o2)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, uint64_t o1, uint64_t o2) noexcept { \
    Imm o1Imm(o1); \
    return emit(_Code_, o0, o1Imm, o2); \
  }

#define INST_4x(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) noexcept { \
    return emit(_Code_, o0, o1, o2, o3); \
  }

#define INST_4i(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) noexcept { \
    return emit(_Code_, o0, o1, o2, o3); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, int o3) noexcept { \
    return emit(_Code_, o0, o1, o2, o3); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, unsigned int o3) noexcept { \
    return emit(_Code_, o0, o1, o2, static_cast<uint64_t>(o3)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, int64_t o3) noexcept { \
    return emit(_Code_, o0, o1, o2, static_cast<uint64_t>(o3)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, uint64_t o3) noexcept { \
    return emit(_Code_, o0, o1, o2, o3); \
  }

#define INST_4ii(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_) \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) noexcept { \
    return emit(_Code_, o0, o1, o2, o3); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, int o2, int o3) noexcept { \
    Imm o2Imm(o2); \
    return emit(_Code_, o0, o1, o2Imm, o3); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, unsigned int o2, unsigned int o3) noexcept { \
    Imm o2Imm(o2); \
    return emit(_Code_, o0, o1, o2Imm, static_cast<uint64_t>(o3)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, int64_t o2, int64_t o3) noexcept { \
    Imm o2Imm(o2); \
    return emit(_Code_, o0, o1, o2Imm, static_cast<uint64_t>(o3)); \
  } \
  /*! \overload */ \
  ASMJIT_INLINE HLInst* _Inst_(const _Op0_& o0, const _Op1_& o1, uint64_t o2, uint64_t o3) noexcept { \
    Imm o2Imm(o2); \
    return emit(_Code_, o0, o1, o2Imm, o3); \
  }

  // --------------------------------------------------------------------------
  // [X86/X64]
  // --------------------------------------------------------------------------

  //! Add with carry.
  INST_2x(adc, kX86InstIdAdc, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(adc, kX86InstIdAdc, X86GpVar, X86Mem)
  //! \overload
  INST_2i(adc, kX86InstIdAdc, X86GpVar, Imm)
  //! \overload
  INST_2x(adc, kX86InstIdAdc, X86Mem, X86GpVar)
  //! \overload
  INST_2i(adc, kX86InstIdAdc, X86Mem, Imm)

  //! Add.
  INST_2x(add, kX86InstIdAdd, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(add, kX86InstIdAdd, X86GpVar, X86Mem)
  //! \overload
  INST_2i(add, kX86InstIdAdd, X86GpVar, Imm)
  //! \overload
  INST_2x(add, kX86InstIdAdd, X86Mem, X86GpVar)
  //! \overload
  INST_2i(add, kX86InstIdAdd, X86Mem, Imm)

  //! And.
  INST_2x(and_, kX86InstIdAnd, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(and_, kX86InstIdAnd, X86GpVar, X86Mem)
  //! \overload
  INST_2i(and_, kX86InstIdAnd, X86GpVar, Imm)
  //! \overload
  INST_2x(and_, kX86InstIdAnd, X86Mem, X86GpVar)
  //! \overload
  INST_2i(and_, kX86InstIdAnd, X86Mem, Imm)

  //! Bit scan forward.
  INST_2x(bsf, kX86InstIdBsf, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(bsf, kX86InstIdBsf, X86GpVar, X86Mem)

  //! Bit scan reverse.
  INST_2x(bsr, kX86InstIdBsr, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(bsr, kX86InstIdBsr, X86GpVar, X86Mem)

  //! Byte swap (32-bit or 64-bit registers only) (i486).
  INST_1x(bswap, kX86InstIdBswap, X86GpVar)

  //! Bit test.
  INST_2x(bt, kX86InstIdBt, X86GpVar, X86GpVar)
  //! \overload
  INST_2i(bt, kX86InstIdBt, X86GpVar, Imm)
  //! \overload
  INST_2x(bt, kX86InstIdBt, X86Mem, X86GpVar)
  //! \overload
  INST_2i(bt, kX86InstIdBt, X86Mem, Imm)

  //! Bit test and complement.
  INST_2x(btc, kX86InstIdBtc, X86GpVar, X86GpVar)
  //! \overload
  INST_2i(btc, kX86InstIdBtc, X86GpVar, Imm)
  //! \overload
  INST_2x(btc, kX86InstIdBtc, X86Mem, X86GpVar)
  //! \overload
  INST_2i(btc, kX86InstIdBtc, X86Mem, Imm)

  //! Bit test and reset.
  INST_2x(btr, kX86InstIdBtr, X86GpVar, X86GpVar)
  //! \overload
  INST_2i(btr, kX86InstIdBtr, X86GpVar, Imm)
  //! \overload
  INST_2x(btr, kX86InstIdBtr, X86Mem, X86GpVar)
  //! \overload
  INST_2i(btr, kX86InstIdBtr, X86Mem, Imm)

  //! Bit test and set.
  INST_2x(bts, kX86InstIdBts, X86GpVar, X86GpVar)
  //! \overload
  INST_2i(bts, kX86InstIdBts, X86GpVar, Imm)
  //! \overload
  INST_2x(bts, kX86InstIdBts, X86Mem, X86GpVar)
  //! \overload
  INST_2i(bts, kX86InstIdBts, X86Mem, Imm)

  //! Call a function.
  ASMJIT_INLINE X86CallNode* call(const X86GpVar& dst, const FuncPrototype& p) {
    return addCall(dst, p);
  }
  //! \overload
  ASMJIT_INLINE X86CallNode* call(const X86Mem& dst, const FuncPrototype& p) {
    return addCall(dst, p);
  }
  //! \overload
  ASMJIT_INLINE X86CallNode* call(const Label& label, const FuncPrototype& p) {
    return addCall(label, p);
  }
  //! \overload
  ASMJIT_INLINE X86CallNode* call(const Imm& dst, const FuncPrototype& p) {
    return addCall(dst, p);
  }
  //! \overload
  ASMJIT_INLINE X86CallNode* call(Ptr dst, const FuncPrototype& p) {
    return addCall(Imm(dst), p);
  }

  //! Clear carry flag
  INST_0x(clc, kX86InstIdClc)
  //! Clear direction flag
  INST_0x(cld, kX86InstIdCld)
  //! Complement carry Flag.
  INST_0x(cmc, kX86InstIdCmc)

  //! Convert BYTE to WORD (AX <- Sign Extend AL).
  INST_1x(cbw, kX86InstIdCbw, X86GpVar  /* AL */)
  //! Convert DWORD to QWORD (EDX:EAX <- Sign Extend EAX).
  INST_2x(cdq, kX86InstIdCdq, X86GpVar /* EDX */, X86GpVar /* EAX */)
  //! Convert DWORD to QWORD (RAX <- Sign Extend EAX) (X64 Only).
  INST_1x(cdqe, kX86InstIdCdqe, X86GpVar /* EAX */)
  //! Convert QWORD to DQWORD (RDX:RAX <- Sign Extend RAX) (X64 Only).
  INST_2x(cqo, kX86InstIdCdq, X86GpVar /* RDX */, X86GpVar /* RAX */)
  //! Convert WORD to DWORD (DX:AX <- Sign Extend AX).
  INST_2x(cwd, kX86InstIdCwd, X86GpVar  /* DX */, X86GpVar /* AX */)
  //! Convert WORD to DWORD (EAX <- Sign Extend AX).
  INST_1x(cwde, kX86InstIdCwde, X86GpVar /* EAX */)

  //! Conditional move.
  INST_2cc(cmov, kX86InstIdCmov, X86Util::condToCmovcc, X86GpVar, X86GpVar)
  //! Conditional move.
  INST_2cc(cmov, kX86InstIdCmov, X86Util::condToCmovcc, X86GpVar, X86Mem)

  //! Compare two operands.
  INST_2x(cmp, kX86InstIdCmp, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(cmp, kX86InstIdCmp, X86GpVar, X86Mem)
  //! \overload
  INST_2i(cmp, kX86InstIdCmp, X86GpVar, Imm)
  //! \overload
  INST_2x(cmp, kX86InstIdCmp, X86Mem, X86GpVar)
  //! \overload
  INST_2i(cmp, kX86InstIdCmp, X86Mem, Imm)

  //! Compare BYTE in ES:`o0` and DS:`o1`.
  INST_2x(cmpsb, kX86InstIdCmpsB, X86GpVar, X86GpVar)
  //! Compare DWORD in ES:`o0` and DS:`o1`.
  INST_2x(cmpsd, kX86InstIdCmpsD, X86GpVar, X86GpVar)
  //! Compare QWORD in ES:`o0` and DS:`o1` (X64 Only).
  INST_2x(cmpsq, kX86InstIdCmpsQ, X86GpVar, X86GpVar)
  //! Compare WORD in ES:`o0` and DS:`o1`.
  INST_2x(cmpsw, kX86InstIdCmpsW, X86GpVar, X86GpVar)

  //! Compare and exchange (i486).
  INST_3x(cmpxchg, kX86InstIdCmpxchg, X86GpVar /* EAX */, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(cmpxchg, kX86InstIdCmpxchg, X86GpVar /* EAX */, X86Mem, X86GpVar)

  //! Compare and exchange 128-bit value in RDX:RAX with `x_mem` (X64 Only).
  ASMJIT_INLINE HLInst* cmpxchg16b(
    const X86GpVar& r_edx, const X86GpVar& r_eax,
    const X86GpVar& r_ecx, const X86GpVar& r_ebx,
    const X86Mem& x_mem) {

    return emit(kX86InstIdCmpxchg16b, r_edx, r_eax, r_ecx, r_ebx, x_mem);
  }

  //! Compare and exchange 64-bit value in EDX:EAX with `x_mem` (Pentium).
  ASMJIT_INLINE HLInst* cmpxchg8b(
    const X86GpVar& r_edx, const X86GpVar& r_eax,
    const X86GpVar& r_ecx, const X86GpVar& r_ebx,
    const X86Mem& x_mem) {

    return emit(kX86InstIdCmpxchg8b, r_edx, r_eax, r_ecx, r_ebx, x_mem);
  }

  //! CPU identification (i486).
  ASMJIT_INLINE HLInst* cpuid(const X86GpVar& x_eax, const X86GpVar& w_ebx, const X86GpVar& x_ecx, const X86GpVar& w_edx) {
    return emit(kX86InstIdCpuid, x_eax, w_ebx, x_ecx, w_edx);
  }

  //! Decimal adjust AL after addition (X86 Only).
  INST_1x(daa, kX86InstIdDaa, X86GpVar)
  //! Decimal adjust AL after subtraction (X86 Only).
  INST_1x(das, kX86InstIdDas, X86GpVar)

  //! Decrement by 1.
  INST_1x(dec, kX86InstIdDec, X86GpVar)
  //! \overload
  INST_1x(dec, kX86InstIdDec, X86Mem)

  //! Unsigned divide (o0:o1 <- o0:o1 / o2).
  //!
  //! Remainder is stored in `o0`, quotient is stored in `o1`.
  INST_3x(div, kX86InstIdDiv, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(div, kX86InstIdDiv, X86GpVar, X86GpVar, X86Mem)

  //! Signed divide (o0:o1 <- o0:o1 / o2).
  //!
  //! Remainder is stored in `o0`, quotient is stored in `o1`.
  INST_3x(idiv, kX86InstIdIdiv, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(idiv, kX86InstIdIdiv, X86GpVar, X86GpVar, X86Mem)

  //! Signed multiply (o0:o1 <- o1 * o2).
  //!
  //! Hi value is stored in `o0`, lo value is stored in `o1`.
  INST_3x(imul, kX86InstIdImul, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(imul, kX86InstIdImul, X86GpVar, X86GpVar, X86Mem)

  //! Signed multiply.
  INST_2x(imul, kX86InstIdImul, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(imul, kX86InstIdImul, X86GpVar, X86Mem)
  //! \overload
  INST_2i(imul, kX86InstIdImul, X86GpVar, Imm)

  //! Signed multiply.
  INST_3i(imul, kX86InstIdImul, X86GpVar, X86GpVar, Imm)
  //! \overload
  INST_3i(imul, kX86InstIdImul, X86GpVar, X86Mem, Imm)

  //! Increment by 1.
  INST_1x(inc, kX86InstIdInc, X86GpVar)
  //! \overload
  INST_1x(inc, kX86InstIdInc, X86Mem)

  //! Interrupt.
  INST_1i(int_, kX86InstIdInt, Imm)
  //! Interrupt 3 - trap to debugger.
  ASMJIT_INLINE HLInst* int3() { return int_(3); }

  //! Jump to label `label` if condition `cc` is met.
  INST_1cc(j, kX86InstIdJ, X86Util::condToJcc, Label)

  //! Short jump if CX/ECX/RCX is zero.
  INST_2x(jecxz, kX86InstIdJecxz, X86GpVar, Label)

  //! Jump.
  INST_1x(jmp, kX86InstIdJmp, X86GpVar)
  //! \overload
  INST_1x(jmp, kX86InstIdJmp, X86Mem)
  //! \overload
  INST_1x(jmp, kX86InstIdJmp, Label)
  //! \overload
  INST_1x(jmp, kX86InstIdJmp, Imm)
  //! \overload
  ASMJIT_INLINE HLInst* jmp(Ptr dst) { return jmp(Imm(dst)); }

  //! Load AH from flags.
  INST_1x(lahf, kX86InstIdLahf, X86GpVar)

  //! Load effective address
  INST_2x(lea, kX86InstIdLea, X86GpVar, X86Mem)

  //! Load BYTE from DS:`o1` to `o0`.
  INST_2x(lodsb, kX86InstIdLodsB, X86GpVar, X86GpVar)
  //! Load DWORD from DS:`o1` to `o0`.
  INST_2x(lodsd, kX86InstIdLodsD, X86GpVar, X86GpVar)
  //! Load QWORD from DS:`o1` to `o0` (X64 Only).
  INST_2x(lodsq, kX86InstIdLodsQ, X86GpVar, X86GpVar)
  //! Load WORD from DS:`o1` to `o0`.
  INST_2x(lodsw, kX86InstIdLodsW, X86GpVar, X86GpVar)

  //! Move.
  INST_2x(mov, kX86InstIdMov, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(mov, kX86InstIdMov, X86GpVar, X86Mem)
  //! \overload
  INST_2i(mov, kX86InstIdMov, X86GpVar, Imm)
  //! \overload
  INST_2x(mov, kX86InstIdMov, X86Mem, X86GpVar)
  //! \overload
  INST_2i(mov, kX86InstIdMov, X86Mem, Imm)

  //! Move from segment register.
  INST_2x(mov, kX86InstIdMov, X86GpVar, X86SegReg)
  //! \overload
  INST_2x(mov, kX86InstIdMov, X86Mem, X86SegReg)
  //! Move to segment register.
  INST_2x(mov, kX86InstIdMov, X86SegReg, X86GpVar)
  //! \overload
  INST_2x(mov, kX86InstIdMov, X86SegReg, X86Mem)

  //! Move (AL|AX|EAX|RAX <- absolute address in immediate).
  INST_2x(mov_ptr, kX86InstIdMovPtr, X86GpReg, Imm);
  //! \overload
  ASMJIT_INLINE HLInst* mov_ptr(const X86GpReg& o0, Ptr o1) {
    ASMJIT_ASSERT(o0.getRegIndex() == 0);
    return emit(kX86InstIdMovPtr, o0, Imm(o1));
  }

  //! Move (absolute address in immediate <- AL|AX|EAX|RAX).
  INST_2x(mov_ptr, kX86InstIdMovPtr, Imm, X86GpReg);
  //! \overload
  ASMJIT_INLINE HLInst* mov_ptr(Ptr o0, const X86GpReg& o1) {
    ASMJIT_ASSERT(o1.getRegIndex() == 0);
    return emit(kX86InstIdMovPtr, Imm(o0), o1);
  }

  //! Move data after swapping bytes (SSE3 - Atom).
  INST_2x(movbe, kX86InstIdMovbe, X86GpVar, X86Mem);
  //! \overload
  INST_2x(movbe, kX86InstIdMovbe, X86Mem, X86GpVar);

  //! Load BYTE from DS:`o1` to ES:`o0`.
  INST_2x(movsb, kX86InstIdMovsB, X86GpVar, X86GpVar)
  //! Load DWORD from DS:`o1` to ES:`o0`.
  INST_2x(movsd, kX86InstIdMovsD, X86GpVar, X86GpVar)
  //! Load QWORD from DS:`o1` to ES:`o0` (X64 Only).
  INST_2x(movsq, kX86InstIdMovsQ, X86GpVar, X86GpVar)
  //! Load WORD from DS:`o1` to ES:`o0`.
  INST_2x(movsw, kX86InstIdMovsW, X86GpVar, X86GpVar)

  //! Move with sign-extension.
  INST_2x(movsx, kX86InstIdMovsx, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(movsx, kX86InstIdMovsx, X86GpVar, X86Mem)

  //! Move DWORD to QWORD with sign-extension (X64 Only).
  INST_2x(movsxd, kX86InstIdMovsxd, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(movsxd, kX86InstIdMovsxd, X86GpVar, X86Mem)

  //! Move with zero-extension.
  INST_2x(movzx, kX86InstIdMovzx, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(movzx, kX86InstIdMovzx, X86GpVar, X86Mem)

  //! Unsigned multiply (o0:o1 <- o1 * o2).
  INST_3x(mul, kX86InstIdMul, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(mul, kX86InstIdMul, X86GpVar, X86GpVar, X86Mem)

  //! Two's complement negation.
  INST_1x(neg, kX86InstIdNeg, X86GpVar)
  //! \overload
  INST_1x(neg, kX86InstIdNeg, X86Mem)

  //! No operation.
  INST_0x(nop, kX86InstIdNop)

  //! One's complement negation.
  INST_1x(not_, kX86InstIdNot, X86GpVar)
  //! \overload
  INST_1x(not_, kX86InstIdNot, X86Mem)

  //! Or.
  INST_2x(or_, kX86InstIdOr, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(or_, kX86InstIdOr, X86GpVar, X86Mem)
  //! \overload
  INST_2i(or_, kX86InstIdOr, X86GpVar, Imm)
  //! \overload
  INST_2x(or_, kX86InstIdOr, X86Mem, X86GpVar)
  //! \overload
  INST_2i(or_, kX86InstIdOr, X86Mem, Imm)

  //! Pop a value from the stack.
  INST_1x(pop, kX86InstIdPop, X86GpVar)
  //! \overload
  INST_1x(pop, kX86InstIdPop, X86Mem)

  //! Pop stack into EFLAGS Register (32-bit or 64-bit).
  INST_0x(popf, kX86InstIdPopf)

  //! Push WORD or DWORD/QWORD on the stack.
  INST_1x(push, kX86InstIdPush, X86GpVar)
  //! Push WORD or DWORD/QWORD on the stack.
  INST_1x(push, kX86InstIdPush, X86Mem)
  //! Push segment register on the stack.
  INST_1x(push, kX86InstIdPush, X86SegReg)
  //! Push WORD or DWORD/QWORD on the stack.
  INST_1i(push, kX86InstIdPush, Imm)

  //! Push EFLAGS register (32-bit or 64-bit) on the stack.
  INST_0x(pushf, kX86InstIdPushf)

  //! Rotate bits left.
  INST_2x(rcl, kX86InstIdRcl, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(rcl, kX86InstIdRcl, X86Mem, X86GpVar)
  //! Rotate bits left.
  INST_2i(rcl, kX86InstIdRcl, X86GpVar, Imm)
  //! \overload
  INST_2i(rcl, kX86InstIdRcl, X86Mem, Imm)

  //! Rotate bits right.
  INST_2x(rcr, kX86InstIdRcr, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(rcr, kX86InstIdRcr, X86Mem, X86GpVar)
  //! Rotate bits right.
  INST_2i(rcr, kX86InstIdRcr, X86GpVar, Imm)
  //! \overload
  INST_2i(rcr, kX86InstIdRcr, X86Mem, Imm)

  //! Read time-stamp counter (Pentium).
  INST_2x(rdtsc, kX86InstIdRdtsc, X86GpVar, X86GpVar)
  //! Read time-stamp counter and processor id (Pentium).
  INST_3x(rdtscp, kX86InstIdRdtscp, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated load ECX/RCX BYTEs from DS:[ESI/RSI] to AL.
  INST_3x(rep_lodsb, kX86InstIdRepLodsB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated load ECX/RCX DWORDs from DS:[ESI/RSI] to AL.
  INST_3x(rep_lodsd, kX86InstIdRepLodsD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated load ECX/RCX QWORDs from DS:[RSI] to RAX (X64 Only).
  INST_3x(rep_lodsq, kX86InstIdRepLodsQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated load ECX/RCX WORDs from DS:[ESI/RSI] to AX.
  INST_3x(rep_lodsw, kX86InstIdRepLodsW, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated move ECX/RCX BYTEs from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x(rep_movsb, kX86InstIdRepMovsB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated move ECX/RCX DWORDs from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x(rep_movsd, kX86InstIdRepMovsD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated move ECX/RCX QWORDs from DS:[RSI] to ES:[RDI] (X64 Only).
  INST_3x(rep_movsq, kX86InstIdRepMovsQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated move ECX/RCX DWORDs from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_3x(rep_movsw, kX86InstIdRepMovsW, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated fill ECX/RCX BYTEs at ES:[EDI/RDI] with AL.
  INST_3x(rep_stosb, kX86InstIdRepStosB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated fill ECX/RCX DWORDs at ES:[EDI/RDI] with EAX.
  INST_3x(rep_stosd, kX86InstIdRepStosD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated fill ECX/RCX QWORDs at ES:[RDI] with RAX (X64 Only).
  INST_3x(rep_stosq, kX86InstIdRepStosQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated fill ECX/RCX WORDs at ES:[EDI/RDI] with AX.
  INST_3x(rep_stosw, kX86InstIdRepStosW, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated find non-AL BYTEs in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x(repe_cmpsb, kX86InstIdRepeCmpsB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find non-EAX DWORDs in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x(repe_cmpsd, kX86InstIdRepeCmpsD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find non-RAX QWORDs in ES:[RDI] and DS:[RDI] (X64 Only).
  INST_3x(repe_cmpsq, kX86InstIdRepeCmpsQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find non-AX WORDs in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_3x(repe_cmpsw, kX86InstIdRepeCmpsW, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated find non-AL BYTE starting at ES:[EDI/RDI].
  INST_3x(repe_scasb, kX86InstIdRepeScasB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find non-EAX DWORD starting at ES:[EDI/RDI].
  INST_3x(repe_scasd, kX86InstIdRepeScasD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find non-RAX QWORD starting at ES:[RDI] (X64 Only).
  INST_3x(repe_scasq, kX86InstIdRepeScasQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find non-AX WORD starting at ES:[EDI/RDI].
  INST_3x(repe_scasw, kX86InstIdRepeScasW, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated find AL BYTEs in [RDI] and [RSI].
  INST_3x(repne_cmpsb, kX86InstIdRepneCmpsB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find EAX DWORDs in [RDI] and [RSI].
  INST_3x(repne_cmpsd, kX86InstIdRepneCmpsD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find RAX QWORDs in [RDI] and [RSI] (X64 Only).
  INST_3x(repne_cmpsq, kX86InstIdRepneCmpsQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find AX WORDs in [RDI] and [RSI].
  INST_3x(repne_cmpsw, kX86InstIdRepneCmpsW, X86GpVar, X86GpVar, X86GpVar)

  //! Repeated Find AL BYTEs, starting at ES:[EDI/RDI].
  INST_3x(repne_scasb, kX86InstIdRepneScasB, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find EAX DWORDs, starting at ES:[EDI/RDI].
  INST_3x(repne_scasd, kX86InstIdRepneScasD, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find RAX QWORDs, starting at ES:[RDI] (X64 Only).
  INST_3x(repne_scasq, kX86InstIdRepneScasQ, X86GpVar, X86GpVar, X86GpVar)
  //! Repeated find AX WORDs, starting at ES:[EDI/RDI].
  INST_3x(repne_scasw, kX86InstIdRepneScasW, X86GpVar, X86GpVar, X86GpVar)

  //! Return.
  ASMJIT_INLINE HLRet* ret() { return addRet(noOperand, noOperand); }
  //! \overload
  ASMJIT_INLINE HLRet* ret(const X86GpVar& o0) { return addRet(o0, noOperand); }
  //! \overload
  ASMJIT_INLINE HLRet* ret(const X86GpVar& o0, const X86GpVar& o1) { return addRet(o0, o1); }
  //! \overload
  ASMJIT_INLINE HLRet* ret(const X86XmmVar& o0) { return addRet(o0, noOperand); }
  //! \overload
  ASMJIT_INLINE HLRet* ret(const X86XmmVar& o0, const X86XmmVar& o1) { return addRet(o0, o1); }

  //! Rotate bits left.
  INST_2x(rol, kX86InstIdRol, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(rol, kX86InstIdRol, X86Mem, X86GpVar)
  //! Rotate bits left.
  INST_2i(rol, kX86InstIdRol, X86GpVar, Imm)
  //! \overload
  INST_2i(rol, kX86InstIdRol, X86Mem, Imm)

  //! Rotate bits right.
  INST_2x(ror, kX86InstIdRor, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(ror, kX86InstIdRor, X86Mem, X86GpVar)
  //! Rotate bits right.
  INST_2i(ror, kX86InstIdRor, X86GpVar, Imm)
  //! \overload
  INST_2i(ror, kX86InstIdRor, X86Mem, Imm)

  //! Store `a` (allocated in AH/AX/EAX/RAX) into Flags.
  INST_1x(sahf, kX86InstIdSahf, X86GpVar)

  //! Integer subtraction with borrow.
  INST_2x(sbb, kX86InstIdSbb, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(sbb, kX86InstIdSbb, X86GpVar, X86Mem)
  //! \overload
  INST_2i(sbb, kX86InstIdSbb, X86GpVar, Imm)
  //! \overload
  INST_2x(sbb, kX86InstIdSbb, X86Mem, X86GpVar)
  //! \overload
  INST_2i(sbb, kX86InstIdSbb, X86Mem, Imm)

  //! Shift bits left.
  INST_2x(sal, kX86InstIdSal, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(sal, kX86InstIdSal, X86Mem, X86GpVar)
  //! Shift bits left.
  INST_2i(sal, kX86InstIdSal, X86GpVar, Imm)
  //! \overload
  INST_2i(sal, kX86InstIdSal, X86Mem, Imm)

  //! Shift bits right.
  INST_2x(sar, kX86InstIdSar, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(sar, kX86InstIdSar, X86Mem, X86GpVar)
  //! Shift bits right.
  INST_2i(sar, kX86InstIdSar, X86GpVar, Imm)
  //! \overload
  INST_2i(sar, kX86InstIdSar, X86Mem, Imm)

  //! Find non `o1` BYTE starting at ES:`o0`.
  INST_2x(scasb, kX86InstIdScasB, X86GpVar, X86GpVar)
  //! Find non `o1` DWORD starting at ES:`o0`.
  INST_2x(scasd, kX86InstIdScasD, X86GpVar, X86GpVar)
  //! Find non `o1` QWORD starting at ES:`o0` (X64 Only).
  INST_2x(scasq, kX86InstIdScasQ, X86GpVar, X86GpVar)
  //! Find non `o1` WORD starting at ES:`o0`.
  INST_2x(scasw, kX86InstIdScasW, X86GpVar, X86GpVar)

  //! Set byte on condition.
  INST_1cc(set, kX86InstIdSet, X86Util::condToSetcc, X86GpVar)
  //! Set byte on condition.
  INST_1cc(set, kX86InstIdSet, X86Util::condToSetcc, X86Mem)

  //! Shift bits left.
  INST_2x(shl, kX86InstIdShl, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(shl, kX86InstIdShl, X86Mem, X86GpVar)
  //! Shift bits left.
  INST_2i(shl, kX86InstIdShl, X86GpVar, Imm)
  //! \overload
  INST_2i(shl, kX86InstIdShl, X86Mem, Imm)

  //! Shift bits right.
  INST_2x(shr, kX86InstIdShr, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(shr, kX86InstIdShr, X86Mem, X86GpVar)
  //! Shift bits right.
  INST_2i(shr, kX86InstIdShr, X86GpVar, Imm)
  //! \overload
  INST_2i(shr, kX86InstIdShr, X86Mem, Imm)

  //! Double precision shift left.
  INST_3x(shld, kX86InstIdShld, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(shld, kX86InstIdShld, X86Mem, X86GpVar, X86GpVar)
  //! Double precision shift left.
  INST_3i(shld, kX86InstIdShld, X86GpVar, X86GpVar, Imm)
  //! \overload
  INST_3i(shld, kX86InstIdShld, X86Mem, X86GpVar, Imm)

  //! Double precision shift right.
  INST_3x(shrd, kX86InstIdShrd, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(shrd, kX86InstIdShrd, X86Mem, X86GpVar, X86GpVar)
  //! Double precision shift right.
  INST_3i(shrd, kX86InstIdShrd, X86GpVar, X86GpVar, Imm)
  //! \overload
  INST_3i(shrd, kX86InstIdShrd, X86Mem, X86GpVar, Imm)

  //! Set carry flag to 1.
  INST_0x(stc, kX86InstIdStc)
  //! Set direction flag to 1.
  INST_0x(std, kX86InstIdStd)

  //! Fill BYTE at ES:`o0` with `o1`.
  INST_2x(stosb, kX86InstIdStosB, X86GpVar, X86GpVar)
  //! Fill DWORD at ES:`o0` with `o1`.
  INST_2x(stosd, kX86InstIdStosD, X86GpVar, X86GpVar)
  //! Fill QWORD at ES:`o0` with `o1` (X64 Only).
  INST_2x(stosq, kX86InstIdStosQ, X86GpVar, X86GpVar)
  //! Fill WORD at ES:`o0` with `o1`.
  INST_2x(stosw, kX86InstIdStosW, X86GpVar, X86GpVar)

  //! Subtract.
  INST_2x(sub, kX86InstIdSub, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(sub, kX86InstIdSub, X86GpVar, X86Mem)
  //! \overload
  INST_2i(sub, kX86InstIdSub, X86GpVar, Imm)
  //! \overload
  INST_2x(sub, kX86InstIdSub, X86Mem, X86GpVar)
  //! \overload
  INST_2i(sub, kX86InstIdSub, X86Mem, Imm)

  //! Logical compare.
  INST_2x(test, kX86InstIdTest, X86GpVar, X86GpVar)
  //! \overload
  INST_2i(test, kX86InstIdTest, X86GpVar, Imm)
  //! \overload
  INST_2x(test, kX86InstIdTest, X86Mem, X86GpVar)
  //! \overload
  INST_2i(test, kX86InstIdTest, X86Mem, Imm)

  //! Undefined instruction - Raise #UD exception.
  INST_0x(ud2, kX86InstIdUd2)

  //! Exchange and add.
  INST_2x(xadd, kX86InstIdXadd, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(xadd, kX86InstIdXadd, X86Mem, X86GpVar)

  //! Exchange register/memory with register.
  INST_2x(xchg, kX86InstIdXchg, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(xchg, kX86InstIdXchg, X86Mem, X86GpVar)
  //! \overload
  INST_2x(xchg, kX86InstIdXchg, X86GpVar, X86Mem)

  //! Xor.
  INST_2x(xor_, kX86InstIdXor, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(xor_, kX86InstIdXor, X86GpVar, X86Mem)
  //! \overload
  INST_2i(xor_, kX86InstIdXor, X86GpVar, Imm)
  //! \overload
  INST_2x(xor_, kX86InstIdXor, X86Mem, X86GpVar)
  //! \overload
  INST_2i(xor_, kX86InstIdXor, X86Mem, Imm)

  // --------------------------------------------------------------------------
  // [FPU]
  // --------------------------------------------------------------------------

  //! Compute `2^x - 1` - `fp0 = POW(2, fp0) - 1` (FPU).
  INST_0x(f2xm1, kX86InstIdF2xm1)
  //! Abs `fp0 = ABS(fp0)` (FPU).
  INST_0x(fabs, kX86InstIdFabs)

  //! Add `o0 = o0 + o1` (one operand has to be `fp0`) (FPU).
  INST_2x(fadd, kX86InstIdFadd, X86FpReg, X86FpReg)
  //! Add `fp0 = fp0 + float_or_double[o0]` (FPU).
  INST_1x(fadd, kX86InstIdFadd, X86Mem)
  //! Add `o0 = o0 + fp0` and POP (FPU).
  INST_1x(faddp, kX86InstIdFaddp, X86FpReg)
  //! Add `fp1 = fp1 + fp0` and POP (FPU).
  INST_0x(faddp, kX86InstIdFaddp)

  //! Load BCD from `[o0]` and PUSH (FPU).
  INST_1x(fbld, kX86InstIdFbld, X86Mem)
  //! Store BCD-Integer to `[o0]` and POP (FPU).
  INST_1x(fbstp, kX86InstIdFbstp, X86Mem)

  //! Complement Sign `fp0 = -fp0` (FPU).
  INST_0x(fchs, kX86InstIdFchs)

  //! Clear exceptions (FPU).
  INST_0x(fclex, kX86InstIdFclex)

  //! Conditional move `if (CF=1) fp0 = o0` (FPU).
  INST_1x(fcmovb, kX86InstIdFcmovb, X86FpReg)
  //! Conditional move `if (CF|ZF=1) fp0 = o0` (FPU).
  INST_1x(fcmovbe, kX86InstIdFcmovbe, X86FpReg)
  //! Conditional move `if (ZF=1) fp0 = o0` (FPU).
  INST_1x(fcmove, kX86InstIdFcmove, X86FpReg)
  //! Conditional move `if (CF=0) fp0 = o0` (FPU).
  INST_1x(fcmovnb, kX86InstIdFcmovnb, X86FpReg)
  //! Conditional move `if (CF|ZF=0) fp0 = o0` (FPU).
  INST_1x(fcmovnbe, kX86InstIdFcmovnbe, X86FpReg)
  //! Conditional move `if (ZF=0) fp0 = o0` (FPU).
  INST_1x(fcmovne, kX86InstIdFcmovne, X86FpReg)
  //! Conditional move `if (PF=0) fp0 = o0` (FPU).
  INST_1x(fcmovnu, kX86InstIdFcmovnu, X86FpReg)
  //! Conditional move `if (PF=1) fp0 = o0` (FPU).
  INST_1x(fcmovu, kX86InstIdFcmovu, X86FpReg)

  //! Compare `fp0` with `o0` (FPU).
  INST_1x(fcom, kX86InstIdFcom, X86FpReg)
  //! Compare `fp0` with `fp1` (FPU).
  INST_0x(fcom, kX86InstIdFcom)
  //! Compare `fp0` with `float_or_double[o0]` (FPU).
  INST_1x(fcom, kX86InstIdFcom, X86Mem)
  //! Compare `fp0` with `o0` and POP (FPU).
  INST_1x(fcomp, kX86InstIdFcomp, X86FpReg)
  //! Compare `fp0` with `fp1` and POP (FPU).
  INST_0x(fcomp, kX86InstIdFcomp)
  //! Compare `fp0` with `float_or_double[o0]` and POP (FPU).
  INST_1x(fcomp, kX86InstIdFcomp, X86Mem)
  //! Compare `fp0` with `fp1` and POP twice (FPU).
  INST_0x(fcompp, kX86InstIdFcompp)
  //! Compare `fp0` with `o0` and set EFLAGS (FPU).
  INST_1x(fcomi, kX86InstIdFcomi, X86FpReg)
  //! Compare `fp0` with `o0` and set EFLAGS and POP (FPU).
  INST_1x(fcomip, kX86InstIdFcomip, X86FpReg)

  //! Cos `fp0 = cos(fp0)` (FPU).
  INST_0x(fcos, kX86InstIdFcos)

  //! Decrement FPU stack pointer (FPU).
  INST_0x(fdecstp, kX86InstIdFdecstp)

  //! Divide `o0 = o0 / o1` (one has to be `fp0`) (FPU).
  INST_2x(fdiv, kX86InstIdFdiv, X86FpReg, X86FpReg)
  //! Divide `fp0 = fp0 / float_or_double[o0]` (FPU).
  INST_1x(fdiv, kX86InstIdFdiv, X86Mem)
  //! Divide `o0 = o0 / fp0` and POP (FPU).
  INST_1x(fdivp, kX86InstIdFdivp, X86FpReg)
  //! Divide `fp1 = fp1 / fp0` and POP (FPU).
  INST_0x(fdivp, kX86InstIdFdivp)

  //! Reverse divide `o0 = o1 / o0` (one has to be `fp0`) (FPU).
  INST_2x(fdivr, kX86InstIdFdivr, X86FpReg, X86FpReg)
  //! Reverse divide `fp0 = float_or_double[o0] / fp0` (FPU).
  INST_1x(fdivr, kX86InstIdFdivr, X86Mem)
  //! Reverse divide `o0 = fp0 / o0` and POP (FPU).
  INST_1x(fdivrp, kX86InstIdFdivrp, X86FpReg)
  //! Reverse divide `fp1 = fp0 / fp1` and POP (FPU).
  INST_0x(fdivrp, kX86InstIdFdivrp)

  //! Free FP register (FPU).
  INST_1x(ffree, kX86InstIdFfree, X86FpReg)

  //! Add `fp0 = fp0 + short_or_int[o0]` (FPU).
  INST_1x(fiadd, kX86InstIdFiadd, X86Mem)
  //! Compare `fp0` with `short_or_int[o0]` (FPU).
  INST_1x(ficom, kX86InstIdFicom, X86Mem)
  //! Compare `fp0` with `short_or_int[o0]` and POP (FPU).
  INST_1x(ficomp, kX86InstIdFicomp, X86Mem)
  //! Divide `fp0 = fp0 / short_or_int[o0]` (FPU).
  INST_1x(fidiv, kX86InstIdFidiv, X86Mem)
  //! Reverse divide `fp0 = short_or_int[o0] / fp0` (FPU).
  INST_1x(fidivr, kX86InstIdFidivr, X86Mem)

  //! Load `short_or_int_or_long[o0]` and PUSH (FPU).
  INST_1x(fild, kX86InstIdFild, X86Mem)
  //! Multiply `fp0 *= short_or_int[o0]` (FPU).
  INST_1x(fimul, kX86InstIdFimul, X86Mem)

  //! Increment FPU stack pointer (FPU).
  INST_0x(fincstp, kX86InstIdFincstp)
  //! Initialize FPU (FPU).
  INST_0x(finit, kX86InstIdFinit)

  //! Subtract `fp0 = fp0 - short_or_int[o0]` (FPU).
  INST_1x(fisub, kX86InstIdFisub, X86Mem)
  //! Reverse subtract `fp0 = short_or_int[o0] - fp0` (FPU).
  INST_1x(fisubr, kX86InstIdFisubr, X86Mem)

  //! Initialize FPU without checking for pending unmasked exceptions (FPU).
  INST_0x(fninit, kX86InstIdFninit)

  //! Store `fp0` as `short_or_int[o0]` (FPU).
  INST_1x(fist, kX86InstIdFist, X86Mem)
  //! Store `fp0` as `short_or_int_or_long[o0]` and POP (FPU).
  INST_1x(fistp, kX86InstIdFistp, X86Mem)

  //! Load `float_or_double_or_extended[o0]` and PUSH (FPU).
  INST_1x(fld, kX86InstIdFld, X86Mem)
  //! PUSH `o0` (FPU).
  INST_1x(fld, kX86InstIdFld, X86FpReg)

  //! PUSH `1.0` (FPU).
  INST_0x(fld1, kX86InstIdFld1)
  //! PUSH `log2(10)` (FPU).
  INST_0x(fldl2t, kX86InstIdFldl2t)
  //! PUSH `log2(e)` (FPU).
  INST_0x(fldl2e, kX86InstIdFldl2e)
  //! PUSH `pi` (FPU).
  INST_0x(fldpi, kX86InstIdFldpi)
  //! PUSH `log10(2)` (FPU).
  INST_0x(fldlg2, kX86InstIdFldlg2)
  //! PUSH `ln(2)` (FPU).
  INST_0x(fldln2, kX86InstIdFldln2)
  //! PUSH `+0.0` (FPU).
  INST_0x(fldz, kX86InstIdFldz)

  //! Load x87 FPU control word from `word_ptr[o0]` (FPU).
  INST_1x(fldcw, kX86InstIdFldcw, X86Mem)
  //! Load x87 FPU environment (14 or 28 bytes) from `[o0]` (FPU).
  INST_1x(fldenv, kX86InstIdFldenv, X86Mem)

  //! Multiply `o0 = o0  * o1` (one has to be `fp0`) (FPU).
  INST_2x(fmul, kX86InstIdFmul, X86FpReg, X86FpReg)
  //! Multiply `fp0 = fp0 * float_or_double[o0]` (FPU).
  INST_1x(fmul, kX86InstIdFmul, X86Mem)
  //! Multiply `o0 = o0 * fp0` and POP (FPU).
  INST_1x(fmulp, kX86InstIdFmulp, X86FpReg)
  //! Multiply `fp1 = fp1 * fp0` and POP (FPU).
  INST_0x(fmulp, kX86InstIdFmulp)

  //! Clear exceptions (FPU).
  INST_0x(fnclex, kX86InstIdFnclex)
  //! No operation (FPU).
  INST_0x(fnop, kX86InstIdFnop)
  //! Save FPU state to `[o0]` (FPU).
  INST_1x(fnsave, kX86InstIdFnsave, X86Mem)
  //! Store x87 FPU environment to `[o0]` (FPU).
  INST_1x(fnstenv, kX86InstIdFnstenv, X86Mem)
  //! Store x87 FPU control word to `[o0]` (FPU).
  INST_1x(fnstcw, kX86InstIdFnstcw, X86Mem)

  //! Store x87 FPU status word to `o0` (AX) (FPU).
  INST_1x(fnstsw, kX86InstIdFnstsw, X86GpVar)
  //! Store x87 FPU status word to `word_ptr[o0]` (FPU).
  INST_1x(fnstsw, kX86InstIdFnstsw, X86Mem)

  //! Partial Arctan `fp1 = atan2(fp1, fp0)` and POP (FPU).
  INST_0x(fpatan, kX86InstIdFpatan)
  //! Partial Remainder[Trunc] `fp1 = fp0 % fp1` and POP (FPU).
  INST_0x(fprem, kX86InstIdFprem)
  //! Partial Remainder[Round] `fp1 = fp0 % fp1` and POP (FPU).
  INST_0x(fprem1, kX86InstIdFprem1)
  //! Partial Tan `fp0 = tan(fp0)` and PUSH `1.0` (FPU).
  INST_0x(fptan, kX86InstIdFptan)
  //! Round `fp0 = round(fp0)` (FPU).
  INST_0x(frndint, kX86InstIdFrndint)

  //! Restore FPU state from `[o0]` (94 or 108 bytes) (FPU).
  INST_1x(frstor, kX86InstIdFrstor, X86Mem)
  //! Save FPU state to `[o0]` (94 or 108 bytes) (FPU).
  INST_1x(fsave, kX86InstIdFsave, X86Mem)

  //! Scale `fp0 = fp0 * pow(2, RoundTowardsZero(fp1))` (FPU).
  INST_0x(fscale, kX86InstIdFscale)
  //! Sin `fp0 = sin(fp0)` (FPU).
  INST_0x(fsin, kX86InstIdFsin)
  //! Sincos `fp0 = sin(fp0)` and PUSH `cos(fp0)` (FPU).
  INST_0x(fsincos, kX86InstIdFsincos)
  //! Square root `fp0 = sqrt(fp0)` (FPU).
  INST_0x(fsqrt, kX86InstIdFsqrt)

  //! Store floating point value to `float_or_double[o0]` (FPU).
  INST_1x(fst, kX86InstIdFst, X86Mem)
  //! Copy `o0 = fp0` (FPU).
  INST_1x(fst, kX86InstIdFst, X86FpReg)
  //! Store floating point value to `float_or_double_or_extended[o0]` and POP (FPU).
  INST_1x(fstp, kX86InstIdFstp, X86Mem)
  //! Copy `o0 = fp0` and POP (FPU).
  INST_1x(fstp, kX86InstIdFstp, X86FpReg)

  //! Store x87 FPU control word to `word_ptr[o0]` (FPU).
  INST_1x(fstcw, kX86InstIdFstcw, X86Mem)
  //! Store x87 FPU environment to `[o0]` (14 or 28 bytes) (FPU).
  INST_1x(fstenv, kX86InstIdFstenv, X86Mem)
  //! Store x87 FPU status word to `o0` (AX) (FPU).
  INST_1x(fstsw, kX86InstIdFstsw, X86GpVar)
  //! Store x87 FPU status word to `word_ptr[o0]` (FPU).
  INST_1x(fstsw, kX86InstIdFstsw, X86Mem)

  //! Subtract `o0 = o0 - o1` (one has to be `fp0`) (FPU).
  INST_2x(fsub, kX86InstIdFsub, X86FpReg, X86FpReg)
  //! Subtract `fp0 = fp0 - float_or_double[o0]` (FPU).
  INST_1x(fsub, kX86InstIdFsub, X86Mem)
  //! Subtract `o0 = o0 - fp0` and POP (FPU).
  INST_1x(fsubp, kX86InstIdFsubp, X86FpReg)
  //! Subtract `fp1 = fp1 - fp0` and POP (FPU).
  INST_0x(fsubp, kX86InstIdFsubp)

  //! Reverse subtract `o0 = o1 - o0` (one has to be `fp0`) (FPU).
  INST_2x(fsubr, kX86InstIdFsubr, X86FpReg, X86FpReg)
  //! Reverse subtract `fp0 = fp0 - float_or_double[o0]` (FPU).
  INST_1x(fsubr, kX86InstIdFsubr, X86Mem)
  //! Reverse subtract `o0 = o0 - fp0` and POP (FPU).
  INST_1x(fsubrp, kX86InstIdFsubrp, X86FpReg)
  //! Reverse subtract `fp1 = fp1 - fp0` and POP (FPU).
  INST_0x(fsubrp, kX86InstIdFsubrp)

  //! Compare `fp0` with `0.0` (FPU).
  INST_0x(ftst, kX86InstIdFtst)

  //! Unordered compare `fp0` with `o0` (FPU).
  INST_1x(fucom, kX86InstIdFucom, X86FpReg)
  //! Unordered compare `fp0` with `fp1` (FPU).
  INST_0x(fucom, kX86InstIdFucom)
  //! Unordered compare `fp0` with `o0`, check for ordered values and set EFLAGS (FPU).
  INST_1x(fucomi, kX86InstIdFucomi, X86FpReg)
  //! Unordered compare `fp0` with `o0`, check for ordered values and set EFLAGS and POP (FPU).
  INST_1x(fucomip, kX86InstIdFucomip, X86FpReg)
  //! Unordered compare `fp0` with `o0` and POP (FPU).
  INST_1x(fucomp, kX86InstIdFucomp, X86FpReg)
  //! Unordered compare `fp0` with `fp1` and POP (FPU).
  INST_0x(fucomp, kX86InstIdFucomp)
  //! Unordered compare `fp0` with `fp1` and POP twice (FPU).
  INST_0x(fucompp, kX86InstIdFucompp)

  INST_0x(fwait, kX86InstIdFwait)

  //! Examine fp0 (FPU).
  INST_0x(fxam, kX86InstIdFxam)
  //! Exchange `fp0` with `o0` (FPU).
  INST_1x(fxch, kX86InstIdFxch, X86FpReg)

  //! Extract `fp0 = exponent(fp0)` and PUSH `significant(fp0)` (FPU).
  INST_0x(fxtract, kX86InstIdFxtract)

  //! Compute `fp1 = fp1 * log2(fp0)` and POP (FPU).
  INST_0x(fyl2x, kX86InstIdFyl2x)
  //! Compute `fp1 = fp1 * log2(fp0 + 1)` and POP (FPU).
  INST_0x(fyl2xp1, kX86InstIdFyl2xp1)

  // --------------------------------------------------------------------------
  // [FXSR]
  // --------------------------------------------------------------------------

  //! Restore FP/MMX/SIMD extension states to `o0` (512 bytes) (FXSR).
  INST_1x(fxrstor, kX86InstIdFxrstor, X86Mem)
  //! Restore FP/MMX/SIMD extension states to `o0` (512 bytes) (FXSR & X64).
  INST_1x(fxrstor64, kX86InstIdFxrstor64, X86Mem)
  //! Store FP/MMX/SIMD extension states to `o0` (512 bytes) (FXSR).
  INST_1x(fxsave, kX86InstIdFxsave, X86Mem)
  //! Store FP/MMX/SIMD extension states to `o0` (512 bytes) (FXSR & X64).
  INST_1x(fxsave64, kX86InstIdFxsave64, X86Mem)

  // --------------------------------------------------------------------------
  // [XSAVE]
  // --------------------------------------------------------------------------

  //! Restore Processor Extended States specified by `o1:o2` (XSAVE).
  INST_3x(xrstor, kX86InstIdXrstor, X86Mem, X86GpVar, X86GpVar)
  //! Restore Processor Extended States specified by `o1:o2` (XSAVE & X64).
  INST_3x(xrstor64, kX86InstIdXrstor64, X86Mem, X86GpVar, X86GpVar)

  //! Save Processor Extended States specified by `o1:o2` (XSAVE).
  INST_3x(xsave, kX86InstIdXsave, X86Mem, X86GpVar, X86GpVar)
  //! Save Processor Extended States specified by `o1:o2` (XSAVE & X64).
  INST_3x(xsave64, kX86InstIdXsave64, X86Mem, X86GpVar, X86GpVar)

  //! Save Processor Extended States specified by `o1:o2` (Optimized) (XSAVEOPT).
  INST_3x(xsaveopt, kX86InstIdXsaveopt, X86Mem, X86GpVar, X86GpVar)
  //! Save Processor Extended States specified by `o1:o2` (Optimized) (XSAVEOPT & X64).
  INST_3x(xsaveopt64, kX86InstIdXsaveopt64, X86Mem, X86GpVar, X86GpVar)

  //! Get XCR - `o1:o2 <- XCR[o0]` (`EDX:EAX <- XCR[ECX]`) (XSAVE).
  INST_3x(xgetbv, kX86InstIdXgetbv, X86GpVar, X86GpVar, X86GpVar)
  //! Set XCR - `XCR[o0] <- o1:o2` (`XCR[ECX] <- EDX:EAX`) (XSAVE).
  INST_3x(xsetbv, kX86InstIdXsetbv, X86GpVar, X86GpVar, X86GpVar)

  // --------------------------------------------------------------------------
  // [POPCNT]
  // --------------------------------------------------------------------------

  //! Return the count of number of bits set to 1 (POPCNT).
  INST_2x(popcnt, kX86InstIdPopcnt, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(popcnt, kX86InstIdPopcnt, X86GpVar, X86Mem)

  // --------------------------------------------------------------------------
  // [LZCNT]
  // --------------------------------------------------------------------------

  //! Count the number of leading zero bits (LZCNT).
  INST_2x(lzcnt, kX86InstIdLzcnt, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(lzcnt, kX86InstIdLzcnt, X86GpVar, X86Mem)

  // --------------------------------------------------------------------------
  // [BMI]
  // --------------------------------------------------------------------------

  //! Bitwise and-not (BMI).
  INST_3x(andn, kX86InstIdAndn, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(andn, kX86InstIdAndn, X86GpVar, X86GpVar, X86Mem)

  //! Bit field extract (BMI).
  INST_3x(bextr, kX86InstIdBextr, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(bextr, kX86InstIdBextr, X86GpVar, X86Mem, X86GpVar)

  //! Extract lower set isolated bit (BMI).
  INST_2x(blsi, kX86InstIdBlsi, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blsi, kX86InstIdBlsi, X86GpVar, X86Mem)

  //! Get mask up to lowest set bit (BMI).
  INST_2x(blsmsk, kX86InstIdBlsmsk, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blsmsk, kX86InstIdBlsmsk, X86GpVar, X86Mem)

  //! Reset lowest set bit (BMI).
  INST_2x(blsr, kX86InstIdBlsr, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blsr, kX86InstIdBlsr, X86GpVar, X86Mem)

  //! Count the number of trailing zero bits (BMI).
  INST_2x(tzcnt, kX86InstIdTzcnt, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(tzcnt, kX86InstIdTzcnt, X86GpVar, X86Mem)

  // --------------------------------------------------------------------------
  // [BMI2]
  // --------------------------------------------------------------------------

  //! Zero high bits starting with specified bit position (BMI2).
  INST_3x(bzhi, kX86InstIdBzhi, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(bzhi, kX86InstIdBzhi, X86GpVar, X86Mem, X86GpVar)

  //! Unsigned multiply without affecting flags (BMI2).
  INST_3x(mulx, kX86InstIdMulx, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(mulx, kX86InstIdMulx, X86GpVar, X86GpVar, X86Mem)

  //! Parallel bits deposit (BMI2).
  INST_3x(pdep, kX86InstIdPdep, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(pdep, kX86InstIdPdep, X86GpVar, X86GpVar, X86Mem)

  //! Parallel bits extract (BMI2).
  INST_3x(pext, kX86InstIdPext, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(pext, kX86InstIdPext, X86GpVar, X86GpVar, X86Mem)

  //! Rotate right without affecting flags (BMI2).
  INST_3i(rorx, kX86InstIdRorx, X86GpVar, X86GpVar, Imm)
  //! \overload
  INST_3i(rorx, kX86InstIdRorx, X86GpVar, X86Mem, Imm)

  //! Shift arithmetic right without affecting flags (BMI2).
  INST_3x(sarx, kX86InstIdSarx, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(sarx, kX86InstIdSarx, X86GpVar, X86Mem, X86GpVar)

  //! Shift logical left without affecting flags (BMI2).
  INST_3x(shlx, kX86InstIdShlx, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(shlx, kX86InstIdShlx, X86GpVar, X86Mem, X86GpVar)

  //! Shift logical right without affecting flags (BMI2).
  INST_3x(shrx, kX86InstIdShrx, X86GpVar, X86GpVar, X86GpVar)
  //! \overload
  INST_3x(shrx, kX86InstIdShrx, X86GpVar, X86Mem, X86GpVar)

  // --------------------------------------------------------------------------
  // [ADX]
  // --------------------------------------------------------------------------

  //! Unsigned integer addition of two operands with carry flag (ADX).
  INST_2x(adcx, kX86InstIdAdcx, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(adcx, kX86InstIdAdcx, X86GpVar, X86Mem)

  //! Unsigned integer addition of two operands with overflow flag (ADX).
  INST_2x(adox, kX86InstIdAdox, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(adox, kX86InstIdAdox, X86GpVar, X86Mem)

  // --------------------------------------------------------------------------
  // [TBM]
  // --------------------------------------------------------------------------

  //! Fill from lowest clear bit (TBM).
  INST_2x(blcfill, kX86InstIdBlcfill, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blcfill, kX86InstIdBlcfill, X86GpVar, X86Mem)

  //! Isolate lowest clear bit (TBM).
  INST_2x(blci, kX86InstIdBlci, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blci, kX86InstIdBlci, X86GpVar, X86Mem)

  //! Isolate lowest clear bit and complement (TBM).
  INST_2x(blcic, kX86InstIdBlcic, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blcic, kX86InstIdBlcic, X86GpVar, X86Mem)

  //! Mask from lowest clear bit (TBM).
  INST_2x(blcmsk, kX86InstIdBlcmsk, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blcmsk, kX86InstIdBlcmsk, X86GpVar, X86Mem)

  //! Set lowest clear bit (TBM).
  INST_2x(blcs, kX86InstIdBlcs, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blcs, kX86InstIdBlcs, X86GpVar, X86Mem)

  //! Fill from lowest set bit (TBM).
  INST_2x(blsfill, kX86InstIdBlsfill, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blsfill, kX86InstIdBlsfill, X86GpVar, X86Mem)

  //! Isolate lowest set bit and complement (TBM).
  INST_2x(blsic, kX86InstIdBlsic, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(blsic, kX86InstIdBlsic, X86GpVar, X86Mem)

  //! Inverse mask from trailing ones (TBM)
  INST_2x(t1mskc, kX86InstIdT1mskc, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(t1mskc, kX86InstIdT1mskc, X86GpVar, X86Mem)

  //! Mask from trailing zeros (TBM)
  INST_2x(tzmsk, kX86InstIdTzmsk, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(tzmsk, kX86InstIdTzmsk, X86GpVar, X86Mem)

  // --------------------------------------------------------------------------
  // [CLFLUSH / CLFLUSH_OPT]
  // --------------------------------------------------------------------------

  //! Flush cache line (CLFLUSH).
  INST_1x(clflush, kX86InstIdClflush, X86Mem)

  //! Flush cache line (CLFLUSH_OPT).
  INST_1x(clflushopt, kX86InstIdClflushopt, X86Mem)

  // --------------------------------------------------------------------------
  // [PREFETCHW / PREFETCHW1]
  // --------------------------------------------------------------------------

  //! Prefetch data into caches in anticipation of a write (3DNOW / PREFETCHW).
  INST_1x(prefetchw, kX86InstIdPrefetchw, X86Mem)

  //! Prefetch vector data into caches with intent to write and T1 hint (PREFETCHWT1).
  INST_1x(prefetchwt1, kX86InstIdPrefetchwt1, X86Mem)

  // --------------------------------------------------------------------------
  // [RDRAND / RDSEED]
  // --------------------------------------------------------------------------

  //! Store a pseudo-random number in destination register (crypto-unsafe) (RDRAND).
  INST_1x(rdrand, kX86InstIdRdrand, X86GpVar)

  //! Store a random seed in destination register (crypto-unsafe) (RDSEED).
  INST_1x(rdseed, kX86InstIdRdseed, X86GpVar)

  // --------------------------------------------------------------------------
  // [FSGSBASE]
  // --------------------------------------------------------------------------

  INST_1x(rdfsbase, kX86InstIdRdfsbase, X86GpVar)
  INST_1x(rdgsbase, kX86InstIdRdgsbase, X86GpVar)
  INST_1x(wrfsbase, kX86InstIdWrfsbase, X86GpVar)
  INST_1x(wrgsbase, kX86InstIdWrgsbase, X86GpVar)

  // --------------------------------------------------------------------------
  // [MMX]
  // --------------------------------------------------------------------------

  //! Move DWORD (MMX).
  INST_2x(movd, kX86InstIdMovd, X86Mem, X86MmVar)
  //! \overload
  INST_2x(movd, kX86InstIdMovd, X86GpVar, X86MmVar)
  //! \overload
  INST_2x(movd, kX86InstIdMovd, X86MmVar, X86Mem)
  //! \overload
  INST_2x(movd, kX86InstIdMovd, X86MmVar, X86GpVar)

  //! Move QWORD (MMX).
  INST_2x(movq, kX86InstIdMovq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(movq, kX86InstIdMovq, X86Mem, X86MmVar)
  //! \overload
  INST_2x(movq, kX86InstIdMovq, X86MmVar, X86Mem)

  //! Move QWORD (X64 Only).
  INST_2x(movq, kX86InstIdMovq, X86GpVar, X86MmVar)
  //! \overload
  INST_2x(movq, kX86InstIdMovq, X86MmVar, X86GpVar)

  //! Pack DWORDs to WORDs with signed saturation (MMX).
  INST_2x(packssdw, kX86InstIdPackssdw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(packssdw, kX86InstIdPackssdw, X86MmVar, X86Mem)

  //! Pack WORDs to BYTEs with signed saturation (MMX).
  INST_2x(packsswb, kX86InstIdPacksswb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(packsswb, kX86InstIdPacksswb, X86MmVar, X86Mem)

  //! Pack WORDs to BYTEs with unsigned saturation (MMX).
  INST_2x(packuswb, kX86InstIdPackuswb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(packuswb, kX86InstIdPackuswb, X86MmVar, X86Mem)

  //! Packed BYTE add (MMX).
  INST_2x(paddb, kX86InstIdPaddb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddb, kX86InstIdPaddb, X86MmVar, X86Mem)

  //! Packed DWORD add (MMX).
  INST_2x(paddd, kX86InstIdPaddd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddd, kX86InstIdPaddd, X86MmVar, X86Mem)

  //! Packed BYTE add with saturation (MMX).
  INST_2x(paddsb, kX86InstIdPaddsb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddsb, kX86InstIdPaddsb, X86MmVar, X86Mem)

  //! Packed WORD add with saturation (MMX).
  INST_2x(paddsw, kX86InstIdPaddsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddsw, kX86InstIdPaddsw, X86MmVar, X86Mem)

  //! Packed BYTE add with unsigned saturation (MMX).
  INST_2x(paddusb, kX86InstIdPaddusb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddusb, kX86InstIdPaddusb, X86MmVar, X86Mem)

  //! Packed WORD add with unsigned saturation (MMX).
  INST_2x(paddusw, kX86InstIdPaddusw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddusw, kX86InstIdPaddusw, X86MmVar, X86Mem)

  //! Packed WORD add (MMX).
  INST_2x(paddw, kX86InstIdPaddw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddw, kX86InstIdPaddw, X86MmVar, X86Mem)

  //! Packed and (MMX).
  INST_2x(pand, kX86InstIdPand, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pand, kX86InstIdPand, X86MmVar, X86Mem)

  //! Packed and-not (MMX).
  INST_2x(pandn, kX86InstIdPandn, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pandn, kX86InstIdPandn, X86MmVar, X86Mem)

  //! Packed BYTEs compare for equality (MMX).
  INST_2x(pcmpeqb, kX86InstIdPcmpeqb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pcmpeqb, kX86InstIdPcmpeqb, X86MmVar, X86Mem)

  //! Packed DWORDs compare for equality (MMX).
  INST_2x(pcmpeqd, kX86InstIdPcmpeqd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pcmpeqd, kX86InstIdPcmpeqd, X86MmVar, X86Mem)

  //! Packed WORDs compare for equality (MMX).
  INST_2x(pcmpeqw, kX86InstIdPcmpeqw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pcmpeqw, kX86InstIdPcmpeqw, X86MmVar, X86Mem)

  //! Packed BYTEs compare if greater than (MMX).
  INST_2x(pcmpgtb, kX86InstIdPcmpgtb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pcmpgtb, kX86InstIdPcmpgtb, X86MmVar, X86Mem)

  //! Packed DWORDs compare if greater than (MMX).
  INST_2x(pcmpgtd, kX86InstIdPcmpgtd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pcmpgtd, kX86InstIdPcmpgtd, X86MmVar, X86Mem)

  //! Packed WORDs compare if greater than (MMX).
  INST_2x(pcmpgtw, kX86InstIdPcmpgtw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pcmpgtw, kX86InstIdPcmpgtw, X86MmVar, X86Mem)

  //! Packed WORD multiply high (MMX).
  INST_2x(pmulhw, kX86InstIdPmulhw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmulhw, kX86InstIdPmulhw, X86MmVar, X86Mem)

  //! Packed WORD multiply low (MMX).
  INST_2x(pmullw, kX86InstIdPmullw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmullw, kX86InstIdPmullw, X86MmVar, X86Mem)

  //! Packed bitwise or (MMX).
  INST_2x(por, kX86InstIdPor, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(por, kX86InstIdPor, X86MmVar, X86Mem)

  //! Packed WORD multiply and add to packed DWORD (MMX).
  INST_2x(pmaddwd, kX86InstIdPmaddwd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmaddwd, kX86InstIdPmaddwd, X86MmVar, X86Mem)

  //! Packed DWORD shift left logical (MMX).
  INST_2x(pslld, kX86InstIdPslld, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pslld, kX86InstIdPslld, X86MmVar, X86Mem)
  //! \overload
  INST_2i(pslld, kX86InstIdPslld, X86MmVar, Imm)

  //! Packed QWORD shift left logical (MMX).
  INST_2x(psllq, kX86InstIdPsllq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psllq, kX86InstIdPsllq, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psllq, kX86InstIdPsllq, X86MmVar, Imm)

  //! Packed WORD shift left logical (MMX).
  INST_2x(psllw, kX86InstIdPsllw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psllw, kX86InstIdPsllw, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psllw, kX86InstIdPsllw, X86MmVar, Imm)

  //! Packed DWORD shift right arithmetic (MMX).
  INST_2x(psrad, kX86InstIdPsrad, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psrad, kX86InstIdPsrad, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psrad, kX86InstIdPsrad, X86MmVar, Imm)

  //! Packed WORD shift right arithmetic (MMX).
  INST_2x(psraw, kX86InstIdPsraw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psraw, kX86InstIdPsraw, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psraw, kX86InstIdPsraw, X86MmVar, Imm)

  //! Packed DWORD shift right logical (MMX).
  INST_2x(psrld, kX86InstIdPsrld, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psrld, kX86InstIdPsrld, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psrld, kX86InstIdPsrld, X86MmVar, Imm)

  //! Packed QWORD shift right logical (MMX).
  INST_2x(psrlq, kX86InstIdPsrlq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psrlq, kX86InstIdPsrlq, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psrlq, kX86InstIdPsrlq, X86MmVar, Imm)

  //! Packed WORD shift right logical (MMX).
  INST_2x(psrlw, kX86InstIdPsrlw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psrlw, kX86InstIdPsrlw, X86MmVar, X86Mem)
  //! \overload
  INST_2i(psrlw, kX86InstIdPsrlw, X86MmVar, Imm)

  //! Packed BYTE subtract (MMX).
  INST_2x(psubb, kX86InstIdPsubb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubb, kX86InstIdPsubb, X86MmVar, X86Mem)

  //! Packed DWORD subtract (MMX).
  INST_2x(psubd, kX86InstIdPsubd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubd, kX86InstIdPsubd, X86MmVar, X86Mem)

  //! Packed BYTE subtract with saturation (MMX).
  INST_2x(psubsb, kX86InstIdPsubsb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubsb, kX86InstIdPsubsb, X86MmVar, X86Mem)

  //! Packed WORD subtract with saturation (MMX).
  INST_2x(psubsw, kX86InstIdPsubsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubsw, kX86InstIdPsubsw, X86MmVar, X86Mem)

  //! Packed BYTE subtract with unsigned saturation (MMX).
  INST_2x(psubusb, kX86InstIdPsubusb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubusb, kX86InstIdPsubusb, X86MmVar, X86Mem)

  //! Packed WORD subtract with unsigned saturation (MMX).
  INST_2x(psubusw, kX86InstIdPsubusw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubusw, kX86InstIdPsubusw, X86MmVar, X86Mem)

  //! Packed WORD subtract (MMX).
  INST_2x(psubw, kX86InstIdPsubw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubw, kX86InstIdPsubw, X86MmVar, X86Mem)

  //! Unpack high packed BYTEs to WORDs (MMX).
  INST_2x(punpckhbw, kX86InstIdPunpckhbw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(punpckhbw, kX86InstIdPunpckhbw, X86MmVar, X86Mem)

  //! Unpack high packed DWORDs to QWORDs (MMX).
  INST_2x(punpckhdq, kX86InstIdPunpckhdq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(punpckhdq, kX86InstIdPunpckhdq, X86MmVar, X86Mem)

  //! Unpack high packed WORDs to DWORDs (MMX).
  INST_2x(punpckhwd, kX86InstIdPunpckhwd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(punpckhwd, kX86InstIdPunpckhwd, X86MmVar, X86Mem)

  //! Unpack low packed BYTEs to WORDs (MMX).
  INST_2x(punpcklbw, kX86InstIdPunpcklbw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(punpcklbw, kX86InstIdPunpcklbw, X86MmVar, X86Mem)

  //! Unpack low packed DWORDs to QWORDs (MMX).
  INST_2x(punpckldq, kX86InstIdPunpckldq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(punpckldq, kX86InstIdPunpckldq, X86MmVar, X86Mem)

  //! Unpack low packed WORDs to DWORDs (MMX).
  INST_2x(punpcklwd, kX86InstIdPunpcklwd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(punpcklwd, kX86InstIdPunpcklwd, X86MmVar, X86Mem)

  //! Packed bitwise xor (MMX).
  INST_2x(pxor, kX86InstIdPxor, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pxor, kX86InstIdPxor, X86MmVar, X86Mem)

  //! Empty MMX state.
  INST_0x(emms, kX86InstIdEmms)

  // --------------------------------------------------------------------------
  // [3DNOW]
  // --------------------------------------------------------------------------

  //! Packed unsigned BYTE average (3DNOW).
  INST_2x(pavgusb, kX86InstIdPavgusb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pavgusb, kX86InstIdPavgusb, X86MmVar, X86Mem)

  //! Packed SP-FP to DWORD convert (3DNOW).
  INST_2x(pf2id, kX86InstIdPf2id, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pf2id, kX86InstIdPf2id, X86MmVar, X86Mem)

  //!  Packed SP-FP to WORD convert (3DNOW).
  INST_2x(pf2iw, kX86InstIdPf2iw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pf2iw, kX86InstIdPf2iw, X86MmVar, X86Mem)

  //! Packed SP-FP accumulate (3DNOW).
  INST_2x(pfacc, kX86InstIdPfacc, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfacc, kX86InstIdPfacc, X86MmVar, X86Mem)

  //! Packed SP-FP addition (3DNOW).
  INST_2x(pfadd, kX86InstIdPfadd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfadd, kX86InstIdPfadd, X86MmVar, X86Mem)

  //! Packed SP-FP compare - dst == src (3DNOW).
  INST_2x(pfcmpeq, kX86InstIdPfcmpeq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfcmpeq, kX86InstIdPfcmpeq, X86MmVar, X86Mem)

  //! Packed SP-FP compare - dst >= src (3DNOW).
  INST_2x(pfcmpge, kX86InstIdPfcmpge, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfcmpge, kX86InstIdPfcmpge, X86MmVar, X86Mem)

  //! Packed SP-FP compare - dst > src (3DNOW).
  INST_2x(pfcmpgt, kX86InstIdPfcmpgt, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfcmpgt, kX86InstIdPfcmpgt, X86MmVar, X86Mem)

  //! Packed SP-FP maximum (3DNOW).
  INST_2x(pfmax, kX86InstIdPfmax, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfmax, kX86InstIdPfmax, X86MmVar, X86Mem)

  //! Packed SP-FP minimum (3DNOW).
  INST_2x(pfmin, kX86InstIdPfmin, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfmin, kX86InstIdPfmin, X86MmVar, X86Mem)

  //! Packed SP-FP multiply (3DNOW).
  INST_2x(pfmul, kX86InstIdPfmul, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfmul, kX86InstIdPfmul, X86MmVar, X86Mem)

  //! Packed SP-FP negative accumulate (3DNOW).
  INST_2x(pfnacc, kX86InstIdPfnacc, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfnacc, kX86InstIdPfnacc, X86MmVar, X86Mem)

  //! Packed SP-FP mixed accumulate (3DNOW).
  INST_2x(pfpnacc, kX86InstIdPfpnacc, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfpnacc, kX86InstIdPfpnacc, X86MmVar, X86Mem)

  //! Packed SP-FP reciprocal approximation (3DNOW).
  INST_2x(pfrcp, kX86InstIdPfrcp, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfrcp, kX86InstIdPfrcp, X86MmVar, X86Mem)

  //! Packed SP-FP reciprocal, first iteration step (3DNOW).
  INST_2x(pfrcpit1, kX86InstIdPfrcpit1, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfrcpit1, kX86InstIdPfrcpit1, X86MmVar, X86Mem)

  //! Packed SP-FP reciprocal, second iteration step (3DNOW).
  INST_2x(pfrcpit2, kX86InstIdPfrcpit2, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfrcpit2, kX86InstIdPfrcpit2, X86MmVar, X86Mem)

  //! Packed SP-FP reciprocal square root, first iteration step (3DNOW).
  INST_2x(pfrsqit1, kX86InstIdPfrsqit1, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfrsqit1, kX86InstIdPfrsqit1, X86MmVar, X86Mem)

  //! Packed SP-FP reciprocal square root approximation (3DNOW).
  INST_2x(pfrsqrt, kX86InstIdPfrsqrt, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfrsqrt, kX86InstIdPfrsqrt, X86MmVar, X86Mem)

  //! Packed SP-FP subtract (3DNOW).
  INST_2x(pfsub, kX86InstIdPfsub, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfsub, kX86InstIdPfsub, X86MmVar, X86Mem)

  //! Packed SP-FP reverse subtract (3DNOW).
  INST_2x(pfsubr, kX86InstIdPfsubr, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pfsubr, kX86InstIdPfsubr, X86MmVar, X86Mem)

  //! Packed DWORDs to SP-FP (3DNOW).
  INST_2x(pi2fd, kX86InstIdPi2fd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pi2fd, kX86InstIdPi2fd, X86MmVar, X86Mem)

  //! Packed WORDs to SP-FP (3DNOW).
  INST_2x(pi2fw, kX86InstIdPi2fw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pi2fw, kX86InstIdPi2fw, X86MmVar, X86Mem)

  //! Packed multiply WORD with rounding (3DNOW).
  INST_2x(pmulhrw, kX86InstIdPmulhrw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmulhrw, kX86InstIdPmulhrw, X86MmVar, X86Mem)

  //! Packed swap DWORDs (3DNOW).
  INST_2x(pswapd, kX86InstIdPswapd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pswapd, kX86InstIdPswapd, X86MmVar, X86Mem)

  //! Prefetch (3DNOW).
  INST_1x(prefetch3dnow, kX86InstIdPrefetch3dNow, X86Mem)

  //! Faster EMMS (3DNOW).
  INST_0x(femms, kX86InstIdFemms)

  // --------------------------------------------------------------------------
  // [SSE]
  // --------------------------------------------------------------------------

  //! Packed SP-FP add (SSE).
  INST_2x(addps, kX86InstIdAddps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(addps, kX86InstIdAddps, X86XmmVar, X86Mem)

  //! Scalar SP-FP add (SSE).
  INST_2x(addss, kX86InstIdAddss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(addss, kX86InstIdAddss, X86XmmVar, X86Mem)

  //! Packed SP-FP bitwise and-not (SSE).
  INST_2x(andnps, kX86InstIdAndnps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(andnps, kX86InstIdAndnps, X86XmmVar, X86Mem)

  //! Packed SP-FP bitwise and (SSE).
  INST_2x(andps, kX86InstIdAndps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(andps, kX86InstIdAndps, X86XmmVar, X86Mem)

  //! Packed SP-FP compare (SSE).
  INST_3i(cmpps, kX86InstIdCmpps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(cmpps, kX86InstIdCmpps, X86XmmVar, X86Mem, Imm)

  //! Compare scalar SP-FP Values (SSE).
  INST_3i(cmpss, kX86InstIdCmpss, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(cmpss, kX86InstIdCmpss, X86XmmVar, X86Mem, Imm)

  //! Scalar ordered SP-FP compare and set EFLAGS (SSE).
  INST_2x(comiss, kX86InstIdComiss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(comiss, kX86InstIdComiss, X86XmmVar, X86Mem)

  //! Packed signed INT32 to packed SP-FP conversion (SSE).
  INST_2x(cvtpi2ps, kX86InstIdCvtpi2ps, X86XmmVar, X86MmVar)
  //! \overload
  INST_2x(cvtpi2ps, kX86InstIdCvtpi2ps, X86XmmVar, X86Mem)

  //! Packed SP-FP to packed INT32 conversion (SSE).
  INST_2x(cvtps2pi, kX86InstIdCvtps2pi, X86MmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtps2pi, kX86InstIdCvtps2pi, X86MmVar, X86Mem)

  //! Convert scalar INT32 to SP-FP (SSE).
  INST_2x(cvtsi2ss, kX86InstIdCvtsi2ss, X86XmmVar, X86GpVar)
  //! \overload
  INST_2x(cvtsi2ss, kX86InstIdCvtsi2ss, X86XmmVar, X86Mem)

  //! Convert scalar SP-FP to INT32 (SSE).
  INST_2x(cvtss2si, kX86InstIdCvtss2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(cvtss2si, kX86InstIdCvtss2si, X86GpVar, X86Mem)

  //! Convert with truncation packed SP-FP to packed INT32 (SSE).
  INST_2x(cvttps2pi, kX86InstIdCvttps2pi, X86MmVar, X86XmmVar)
  //! \overload
  INST_2x(cvttps2pi, kX86InstIdCvttps2pi, X86MmVar, X86Mem)

  //! Convert with truncation scalar SP-FP to INT32 (SSE).
  INST_2x(cvttss2si, kX86InstIdCvttss2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(cvttss2si, kX86InstIdCvttss2si, X86GpVar, X86Mem)

  //! Packed SP-FP divide (SSE).
  INST_2x(divps, kX86InstIdDivps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(divps, kX86InstIdDivps, X86XmmVar, X86Mem)

  //! Scalar SP-FP divide (SSE).
  INST_2x(divss, kX86InstIdDivss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(divss, kX86InstIdDivss, X86XmmVar, X86Mem)

  //! Load streaming SIMD extension control/status (SSE).
  INST_1x(ldmxcsr, kX86InstIdLdmxcsr, X86Mem)

  //! Byte mask write (SSE).
  INST_3x(maskmovq, kX86InstIdMaskmovq, X86GpVar /* ZDI */, X86MmVar, X86MmVar)

  //! Packed SP-FP maximum (SSE).
  INST_2x(maxps, kX86InstIdMaxps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(maxps, kX86InstIdMaxps, X86XmmVar, X86Mem)

  //! Scalar SP-FP maximum (SSE).
  INST_2x(maxss, kX86InstIdMaxss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(maxss, kX86InstIdMaxss, X86XmmVar, X86Mem)

  //! Packed SP-FP minimum (SSE).
  INST_2x(minps, kX86InstIdMinps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(minps, kX86InstIdMinps, X86XmmVar, X86Mem)

  //! Scalar SP-FP minimum (SSE).
  INST_2x(minss, kX86InstIdMinss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(minss, kX86InstIdMinss, X86XmmVar, X86Mem)

  //! Move aligned packed SP-FP (SSE).
  INST_2x(movaps, kX86InstIdMovaps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movaps, kX86InstIdMovaps, X86XmmVar, X86Mem)
  //! Move aligned packed SP-FP (SSE).
  INST_2x(movaps, kX86InstIdMovaps, X86Mem, X86XmmVar)

  //! Move DWORD.
  INST_2x(movd, kX86InstIdMovd, X86Mem, X86XmmVar)
  //! \overload
  INST_2x(movd, kX86InstIdMovd, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(movd, kX86InstIdMovd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movd, kX86InstIdMovd, X86XmmVar, X86GpVar)

  //! Move QWORD (SSE).
  INST_2x(movq, kX86InstIdMovq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movq, kX86InstIdMovq, X86Mem, X86XmmVar)
  //! \overload
  INST_2x(movq, kX86InstIdMovq, X86XmmVar, X86Mem)

  //! Move QWORD (X64 Only).
  INST_2x(movq, kX86InstIdMovq, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(movq, kX86InstIdMovq, X86XmmVar, X86GpVar)

  //! Move QWORD using NT hint (SSE).
  INST_2x(movntq, kX86InstIdMovntq, X86Mem, X86MmVar)

  //! Move high to low packed SP-FP (SSE).
  INST_2x(movhlps, kX86InstIdMovhlps, X86XmmVar, X86XmmVar)

  //! Move high packed SP-FP (SSE).
  INST_2x(movhps, kX86InstIdMovhps, X86XmmVar, X86Mem)
  //! Move high packed SP-FP (SSE).
  INST_2x(movhps, kX86InstIdMovhps, X86Mem, X86XmmVar)

  //! Move low to high packed SP-FP (SSE).
  INST_2x(movlhps, kX86InstIdMovlhps, X86XmmVar, X86XmmVar)

  //! Move low packed SP-FP (SSE).
  INST_2x(movlps, kX86InstIdMovlps, X86XmmVar, X86Mem)
  //! Move low packed SP-FP (SSE).
  INST_2x(movlps, kX86InstIdMovlps, X86Mem, X86XmmVar)

  //! Move aligned packed SP-FP using NT hint (SSE).
  INST_2x(movntps, kX86InstIdMovntps, X86Mem, X86XmmVar)

  //! Move scalar SP-FP (SSE).
  INST_2x(movss, kX86InstIdMovss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movss, kX86InstIdMovss, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movss, kX86InstIdMovss, X86Mem, X86XmmVar)

  //! Move unaligned packed SP-FP (SSE).
  INST_2x(movups, kX86InstIdMovups, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movups, kX86InstIdMovups, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movups, kX86InstIdMovups, X86Mem, X86XmmVar)

  //! Packed SP-FP multiply (SSE).
  INST_2x(mulps, kX86InstIdMulps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(mulps, kX86InstIdMulps, X86XmmVar, X86Mem)

  //! Scalar SP-FP multiply (SSE).
  INST_2x(mulss, kX86InstIdMulss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(mulss, kX86InstIdMulss, X86XmmVar, X86Mem)

  //! Packed SP-FP bitwise or (SSE).
  INST_2x(orps, kX86InstIdOrps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(orps, kX86InstIdOrps, X86XmmVar, X86Mem)

  //! Packed BYTE average (SSE).
  INST_2x(pavgb, kX86InstIdPavgb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pavgb, kX86InstIdPavgb, X86MmVar, X86Mem)

  //! Packed WORD average (SSE).
  INST_2x(pavgw, kX86InstIdPavgw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pavgw, kX86InstIdPavgw, X86MmVar, X86Mem)

  //! Extract WORD based on selector (SSE).
  INST_3i(pextrw, kX86InstIdPextrw, X86GpVar, X86MmVar, Imm)

  //! Insert WORD based on selector (SSE).
  INST_3i(pinsrw, kX86InstIdPinsrw, X86MmVar, X86GpVar, Imm)
  //! \overload
  INST_3i(pinsrw, kX86InstIdPinsrw, X86MmVar, X86Mem, Imm)

  //! Packed WORD maximum (SSE).
  INST_2x(pmaxsw, kX86InstIdPmaxsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmaxsw, kX86InstIdPmaxsw, X86MmVar, X86Mem)

  //! Packed BYTE unsigned maximum (SSE).
  INST_2x(pmaxub, kX86InstIdPmaxub, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmaxub, kX86InstIdPmaxub, X86MmVar, X86Mem)

  //! Packed WORD minimum (SSE).
  INST_2x(pminsw, kX86InstIdPminsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pminsw, kX86InstIdPminsw, X86MmVar, X86Mem)

  //! Packed BYTE unsigned minimum (SSE).
  INST_2x(pminub, kX86InstIdPminub, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pminub, kX86InstIdPminub, X86MmVar, X86Mem)

  //! Move byte mask to integer (SSE).
  INST_2x(pmovmskb, kX86InstIdPmovmskb, X86GpVar, X86MmVar)

  //! Packed WORD unsigned multiply high (SSE).
  INST_2x(pmulhuw, kX86InstIdPmulhuw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmulhuw, kX86InstIdPmulhuw, X86MmVar, X86Mem)

  //! Packed WORD sum of absolute differences (SSE).
  INST_2x(psadbw, kX86InstIdPsadbw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psadbw, kX86InstIdPsadbw, X86MmVar, X86Mem)

  //! Packed WORD shuffle (SSE).
  INST_3i(pshufw, kX86InstIdPshufw, X86MmVar, X86MmVar, Imm)
  //! \overload
  INST_3i(pshufw, kX86InstIdPshufw, X86MmVar, X86Mem, Imm)

  //! Packed SP-FP reciprocal (SSE).
  INST_2x(rcpps, kX86InstIdRcpps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(rcpps, kX86InstIdRcpps, X86XmmVar, X86Mem)

  //! Scalar SP-FP reciprocal (SSE).
  INST_2x(rcpss, kX86InstIdRcpss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(rcpss, kX86InstIdRcpss, X86XmmVar, X86Mem)

  //! Prefetch (SSE).
  INST_2i(prefetch, kX86InstIdPrefetch, X86Mem, Imm)

  //! Packed WORD sum of absolute differences (SSE).
  INST_2x(psadbw, kX86InstIdPsadbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psadbw, kX86InstIdPsadbw, X86XmmVar, X86Mem)

  //! Packed SP-FP Square root reciprocal (SSE).
  INST_2x(rsqrtps, kX86InstIdRsqrtps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(rsqrtps, kX86InstIdRsqrtps, X86XmmVar, X86Mem)

  //! Scalar SP-FP Square root reciprocal (SSE).
  INST_2x(rsqrtss, kX86InstIdRsqrtss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(rsqrtss, kX86InstIdRsqrtss, X86XmmVar, X86Mem)

  //! Store fence (SSE).
  INST_0x(sfence, kX86InstIdSfence)

  //! Shuffle SP-FP (SSE).
  INST_3i(shufps, kX86InstIdShufps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(shufps, kX86InstIdShufps, X86XmmVar, X86Mem, Imm)

  //! Packed SP-FP square root (SSE).
  INST_2x(sqrtps, kX86InstIdSqrtps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sqrtps, kX86InstIdSqrtps, X86XmmVar, X86Mem)

  //! Scalar SP-FP square root (SSE).
  INST_2x(sqrtss, kX86InstIdSqrtss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sqrtss, kX86InstIdSqrtss, X86XmmVar, X86Mem)

  //! Store streaming SIMD extension control/status (SSE).
  INST_1x(stmxcsr, kX86InstIdStmxcsr, X86Mem)

  //! Packed SP-FP subtract (SSE).
  INST_2x(subps, kX86InstIdSubps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(subps, kX86InstIdSubps, X86XmmVar, X86Mem)

  //! Scalar SP-FP subtract (SSE).
  INST_2x(subss, kX86InstIdSubss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(subss, kX86InstIdSubss, X86XmmVar, X86Mem)

  //! Unordered scalar SP-FP compare and set EFLAGS (SSE).
  INST_2x(ucomiss, kX86InstIdUcomiss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(ucomiss, kX86InstIdUcomiss, X86XmmVar, X86Mem)

  //! Unpack high packed SP-FP data (SSE).
  INST_2x(unpckhps, kX86InstIdUnpckhps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(unpckhps, kX86InstIdUnpckhps, X86XmmVar, X86Mem)

  //! Unpack low packed SP-FP data (SSE).
  INST_2x(unpcklps, kX86InstIdUnpcklps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(unpcklps, kX86InstIdUnpcklps, X86XmmVar, X86Mem)

  //! Packed SP-FP bitwise xor (SSE).
  INST_2x(xorps, kX86InstIdXorps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(xorps, kX86InstIdXorps, X86XmmVar, X86Mem)

  // --------------------------------------------------------------------------
  // [SSE2]
  // --------------------------------------------------------------------------

  //! Packed DP-FP add (SSE2).
  INST_2x(addpd, kX86InstIdAddpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(addpd, kX86InstIdAddpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP add (SSE2).
  INST_2x(addsd, kX86InstIdAddsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(addsd, kX86InstIdAddsd, X86XmmVar, X86Mem)

  //! Packed DP-FP bitwise and-not (SSE2).
  INST_2x(andnpd, kX86InstIdAndnpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(andnpd, kX86InstIdAndnpd, X86XmmVar, X86Mem)

  //! Packed DP-FP bitwise and (SSE2).
  INST_2x(andpd, kX86InstIdAndpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(andpd, kX86InstIdAndpd, X86XmmVar, X86Mem)

  //! Packed DP-FP compare (SSE2).
  INST_3i(cmppd, kX86InstIdCmppd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(cmppd, kX86InstIdCmppd, X86XmmVar, X86Mem, Imm)

  //! Scalar SP-FP compare (SSE2).
  INST_3i(cmpsd, kX86InstIdCmpsd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(cmpsd, kX86InstIdCmpsd, X86XmmVar, X86Mem, Imm)

  //! Scalar ordered DP-FP compare and set EFLAGS (SSE2).
  INST_2x(comisd, kX86InstIdComisd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(comisd, kX86InstIdComisd, X86XmmVar, X86Mem)

  //! Convert packed DWORD integers to packed DP-FP (SSE2).
  INST_2x(cvtdq2pd, kX86InstIdCvtdq2pd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtdq2pd, kX86InstIdCvtdq2pd, X86XmmVar, X86Mem)

  //! Convert packed DWORD integers to packed SP-FP (SSE2).
  INST_2x(cvtdq2ps, kX86InstIdCvtdq2ps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtdq2ps, kX86InstIdCvtdq2ps, X86XmmVar, X86Mem)

  //! Convert packed DP-FP to packed DWORDs (SSE2).
  INST_2x(cvtpd2dq, kX86InstIdCvtpd2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtpd2dq, kX86InstIdCvtpd2dq, X86XmmVar, X86Mem)

  //! Convert packed DP-FP to packed DWORDs (SSE2).
  INST_2x(cvtpd2pi, kX86InstIdCvtpd2pi, X86MmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtpd2pi, kX86InstIdCvtpd2pi, X86MmVar, X86Mem)

  //! Convert packed DP-FP to packed SP-FP (SSE2).
  INST_2x(cvtpd2ps, kX86InstIdCvtpd2ps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtpd2ps, kX86InstIdCvtpd2ps, X86XmmVar, X86Mem)

  //! Convert packed DWORDs to packed DP-FP (SSE2).
  INST_2x(cvtpi2pd, kX86InstIdCvtpi2pd, X86XmmVar, X86MmVar)
  //! \overload
  INST_2x(cvtpi2pd, kX86InstIdCvtpi2pd, X86XmmVar, X86Mem)

  //! Convert packed SP-FP to packed DWORDs (SSE2).
  INST_2x(cvtps2dq, kX86InstIdCvtps2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtps2dq, kX86InstIdCvtps2dq, X86XmmVar, X86Mem)

  //! Convert packed SP-FP to packed DP-FP (SSE2).
  INST_2x(cvtps2pd, kX86InstIdCvtps2pd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtps2pd, kX86InstIdCvtps2pd, X86XmmVar, X86Mem)

  //! Convert scalar DP-FP to DWORD (SSE2).
  INST_2x(cvtsd2si, kX86InstIdCvtsd2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(cvtsd2si, kX86InstIdCvtsd2si, X86GpVar, X86Mem)

  //! Convert scalar DP-FP to scalar SP-FP (SSE2).
  INST_2x(cvtsd2ss, kX86InstIdCvtsd2ss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtsd2ss, kX86InstIdCvtsd2ss, X86XmmVar, X86Mem)

  //! Convert DWORD to scalar DP-FP (SSE2).
  INST_2x(cvtsi2sd, kX86InstIdCvtsi2sd, X86XmmVar, X86GpVar)
  //! \overload
  INST_2x(cvtsi2sd, kX86InstIdCvtsi2sd, X86XmmVar, X86Mem)

  //! Convert scalar SP-FP to DP-FP (SSE2).
  INST_2x(cvtss2sd, kX86InstIdCvtss2sd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvtss2sd, kX86InstIdCvtss2sd, X86XmmVar, X86Mem)

  //! Convert with truncation packed DP-FP to packed DWORDs (SSE2).
  INST_2x(cvttpd2pi, kX86InstIdCvttpd2pi, X86MmVar, X86XmmVar)
  //! \overload
  INST_2x(cvttpd2pi, kX86InstIdCvttpd2pi, X86MmVar, X86Mem)

  //! Convert with truncation packed DP-FP to packed QWORDs (SSE2).
  INST_2x(cvttpd2dq, kX86InstIdCvttpd2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvttpd2dq, kX86InstIdCvttpd2dq, X86XmmVar, X86Mem)

  //! Convert with truncation packed SP-FP to packed QWORDs (SSE2).
  INST_2x(cvttps2dq, kX86InstIdCvttps2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(cvttps2dq, kX86InstIdCvttps2dq, X86XmmVar, X86Mem)

  //! Convert with truncation scalar DP-FP to DWORD (SSE2).
  INST_2x(cvttsd2si, kX86InstIdCvttsd2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(cvttsd2si, kX86InstIdCvttsd2si, X86GpVar, X86Mem)

  //! Packed DP-FP divide (SSE2).
  INST_2x(divpd, kX86InstIdDivpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(divpd, kX86InstIdDivpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP divide (SSE2).
  INST_2x(divsd, kX86InstIdDivsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(divsd, kX86InstIdDivsd, X86XmmVar, X86Mem)

  //! Load fence (SSE2).
  INST_0x(lfence, kX86InstIdLfence)

  //! Store selected bytes of DQWORD (SSE2).
  INST_3x(maskmovdqu, kX86InstIdMaskmovdqu, X86GpVar /* ZDI */, X86XmmVar, X86XmmVar)

  //! Packed DP-FP maximum (SSE2).
  INST_2x(maxpd, kX86InstIdMaxpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(maxpd, kX86InstIdMaxpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP maximum (SSE2).
  INST_2x(maxsd, kX86InstIdMaxsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(maxsd, kX86InstIdMaxsd, X86XmmVar, X86Mem)

  //! Memory fence (SSE2).
  INST_0x(mfence, kX86InstIdMfence)

  //! Packed DP-FP minimum (SSE2).
  INST_2x(minpd, kX86InstIdMinpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(minpd, kX86InstIdMinpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP minimum (SSE2).
  INST_2x(minsd, kX86InstIdMinsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(minsd, kX86InstIdMinsd, X86XmmVar, X86Mem)

  //! Move aligned DQWORD (SSE2).
  INST_2x(movdqa, kX86InstIdMovdqa, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movdqa, kX86InstIdMovdqa, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movdqa, kX86InstIdMovdqa, X86Mem, X86XmmVar)

  //! Move unaligned DQWORD (SSE2).
  INST_2x(movdqu, kX86InstIdMovdqu, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movdqu, kX86InstIdMovdqu, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movdqu, kX86InstIdMovdqu, X86Mem, X86XmmVar)

  //! Extract packed SP-FP sign mask (SSE2).
  INST_2x(movmskps, kX86InstIdMovmskps, X86GpVar, X86XmmVar)

  //! Extract packed DP-FP sign mask (SSE2).
  INST_2x(movmskpd, kX86InstIdMovmskpd, X86GpVar, X86XmmVar)

  //! Move scalar DP-FP (SSE2).
  INST_2x(movsd, kX86InstIdMovsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movsd, kX86InstIdMovsd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movsd, kX86InstIdMovsd, X86Mem, X86XmmVar)

  //! Move aligned packed DP-FP (SSE2).
  INST_2x(movapd, kX86InstIdMovapd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movapd, kX86InstIdMovapd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movapd, kX86InstIdMovapd, X86Mem, X86XmmVar)

  //! Move QWORD from XMM to MMX register (SSE2).
  INST_2x(movdq2q, kX86InstIdMovdq2q, X86MmVar, X86XmmVar)

  //! Move QWORD from MMX to XMM register (SSE2).
  INST_2x(movq2dq, kX86InstIdMovq2dq, X86XmmVar, X86MmVar)

  //! Move high packed DP-FP (SSE2).
  INST_2x(movhpd, kX86InstIdMovhpd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movhpd, kX86InstIdMovhpd, X86Mem, X86XmmVar)

  //! Move low packed DP-FP (SSE2).
  INST_2x(movlpd, kX86InstIdMovlpd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movlpd, kX86InstIdMovlpd, X86Mem, X86XmmVar)

  //! Store OWORD using NT hint (SSE2).
  INST_2x(movntdq, kX86InstIdMovntdq, X86Mem, X86XmmVar)

  //! Store DWORD using NT hint (SSE2).
  INST_2x(movnti, kX86InstIdMovnti, X86Mem, X86GpVar)

  //! Store packed DP-FP using NT hint (SSE2).
  INST_2x(movntpd, kX86InstIdMovntpd, X86Mem, X86XmmVar)

  //! Move unaligned packed DP-FP (SSE2).
  INST_2x(movupd, kX86InstIdMovupd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movupd, kX86InstIdMovupd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(movupd, kX86InstIdMovupd, X86Mem, X86XmmVar)

  //! Packed DP-FP multiply (SSE2).
  INST_2x(mulpd, kX86InstIdMulpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(mulpd, kX86InstIdMulpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP multiply (SSE2).
  INST_2x(mulsd, kX86InstIdMulsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(mulsd, kX86InstIdMulsd, X86XmmVar, X86Mem)

  //! Packed DP-FP bitwise or (SSE2).
  INST_2x(orpd, kX86InstIdOrpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(orpd, kX86InstIdOrpd, X86XmmVar, X86Mem)

  //! Pack WORDs to BYTEs with signed saturation (SSE2).
  INST_2x(packsswb, kX86InstIdPacksswb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(packsswb, kX86InstIdPacksswb, X86XmmVar, X86Mem)

  //! Pack DWORDs to WORDs with signed saturation (SSE2).
  INST_2x(packssdw, kX86InstIdPackssdw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(packssdw, kX86InstIdPackssdw, X86XmmVar, X86Mem)

  //! Pack WORDs to BYTEs with unsigned saturation (SSE2).
  INST_2x(packuswb, kX86InstIdPackuswb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(packuswb, kX86InstIdPackuswb, X86XmmVar, X86Mem)

  //! Packed BYTE add (SSE2).
  INST_2x(paddb, kX86InstIdPaddb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddb, kX86InstIdPaddb, X86XmmVar, X86Mem)

  //! Packed WORD add (SSE2).
  INST_2x(paddw, kX86InstIdPaddw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddw, kX86InstIdPaddw, X86XmmVar, X86Mem)

  //! Packed DWORD add (SSE2).
  INST_2x(paddd, kX86InstIdPaddd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddd, kX86InstIdPaddd, X86XmmVar, X86Mem)

  //! Packed QWORD add (SSE2).
  INST_2x(paddq, kX86InstIdPaddq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(paddq, kX86InstIdPaddq, X86MmVar, X86Mem)

  //! Packed QWORD add (SSE2).
  INST_2x(paddq, kX86InstIdPaddq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddq, kX86InstIdPaddq, X86XmmVar, X86Mem)

  //! Packed BYTE add with saturation (SSE2).
  INST_2x(paddsb, kX86InstIdPaddsb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddsb, kX86InstIdPaddsb, X86XmmVar, X86Mem)

  //! Packed WORD add with saturation (SSE2).
  INST_2x(paddsw, kX86InstIdPaddsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddsw, kX86InstIdPaddsw, X86XmmVar, X86Mem)

  //! Packed BYTE add with unsigned saturation (SSE2).
  INST_2x(paddusb, kX86InstIdPaddusb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddusb, kX86InstIdPaddusb, X86XmmVar, X86Mem)

  //! Packed WORD add with unsigned saturation (SSE2).
  INST_2x(paddusw, kX86InstIdPaddusw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(paddusw, kX86InstIdPaddusw, X86XmmVar, X86Mem)

  //! Packed bitwise and (SSE2).
  INST_2x(pand, kX86InstIdPand, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pand, kX86InstIdPand, X86XmmVar, X86Mem)

  //! Packed bitwise and-not (SSE2).
  INST_2x(pandn, kX86InstIdPandn, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pandn, kX86InstIdPandn, X86XmmVar, X86Mem)

  //! Spin loop hint (SSE2).
  INST_0x(pause, kX86InstIdPause)

  //! Packed BYTE average (SSE2).
  INST_2x(pavgb, kX86InstIdPavgb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pavgb, kX86InstIdPavgb, X86XmmVar, X86Mem)

  //! Packed WORD average (SSE2).
  INST_2x(pavgw, kX86InstIdPavgw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pavgw, kX86InstIdPavgw, X86XmmVar, X86Mem)

  //! Packed BYTE compare for equality (SSE2).
  INST_2x(pcmpeqb, kX86InstIdPcmpeqb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpeqb, kX86InstIdPcmpeqb, X86XmmVar, X86Mem)

  //! Packed WROD compare for equality (SSE2).
  INST_2x(pcmpeqw, kX86InstIdPcmpeqw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpeqw, kX86InstIdPcmpeqw, X86XmmVar, X86Mem)

  //! Packed DWORD compare for equality (SSE2).
  INST_2x(pcmpeqd, kX86InstIdPcmpeqd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpeqd, kX86InstIdPcmpeqd, X86XmmVar, X86Mem)

  //! Packed BYTE compare if greater than (SSE2).
  INST_2x(pcmpgtb, kX86InstIdPcmpgtb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpgtb, kX86InstIdPcmpgtb, X86XmmVar, X86Mem)

  //! Packed WORD compare if greater than (SSE2).
  INST_2x(pcmpgtw, kX86InstIdPcmpgtw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpgtw, kX86InstIdPcmpgtw, X86XmmVar, X86Mem)

  //! Packed DWORD compare if greater than (SSE2).
  INST_2x(pcmpgtd, kX86InstIdPcmpgtd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpgtd, kX86InstIdPcmpgtd, X86XmmVar, X86Mem)

  //! Extract WORD based on selector (SSE2).
  INST_3i(pextrw, kX86InstIdPextrw, X86GpVar, X86XmmVar, Imm)

  //! Insert WORD based on selector (SSE2).
  INST_3i(pinsrw, kX86InstIdPinsrw, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_3i(pinsrw, kX86InstIdPinsrw, X86XmmVar, X86Mem, Imm)

  //! Packed WORD maximum (SSE2).
  INST_2x(pmaxsw, kX86InstIdPmaxsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaxsw, kX86InstIdPmaxsw, X86XmmVar, X86Mem)

  //! Packed BYTE unsigned maximum (SSE2).
  INST_2x(pmaxub, kX86InstIdPmaxub, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaxub, kX86InstIdPmaxub, X86XmmVar, X86Mem)

  //! Packed WORD minimum (SSE2).
  INST_2x(pminsw, kX86InstIdPminsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pminsw, kX86InstIdPminsw, X86XmmVar, X86Mem)

  //! Packed BYTE unsigned minimum (SSE2).
  INST_2x(pminub, kX86InstIdPminub, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pminub, kX86InstIdPminub, X86XmmVar, X86Mem)

  //! Move BYTE mask (SSE2).
  INST_2x(pmovmskb, kX86InstIdPmovmskb, X86GpVar, X86XmmVar)

  //! Packed WORD multiply high (SSE2).
  INST_2x(pmulhw, kX86InstIdPmulhw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmulhw, kX86InstIdPmulhw, X86XmmVar, X86Mem)

  //! Packed WORD unsigned multiply high (SSE2).
  INST_2x(pmulhuw, kX86InstIdPmulhuw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmulhuw, kX86InstIdPmulhuw, X86XmmVar, X86Mem)

  //! Packed WORD multiply low (SSE2).
  INST_2x(pmullw, kX86InstIdPmullw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmullw, kX86InstIdPmullw, X86XmmVar, X86Mem)

  //! Packed DWORD multiply to QWORD (SSE2).
  INST_2x(pmuludq, kX86InstIdPmuludq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmuludq, kX86InstIdPmuludq, X86MmVar, X86Mem)

  //! Packed DWORD multiply to QWORD (SSE2).
  INST_2x(pmuludq, kX86InstIdPmuludq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmuludq, kX86InstIdPmuludq, X86XmmVar, X86Mem)

  //! Packed bitwise or (SSE2).
  INST_2x(por, kX86InstIdPor, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(por, kX86InstIdPor, X86XmmVar, X86Mem)

  //! Packed DWORD shift left logical (SSE2).
  INST_2x(pslld, kX86InstIdPslld, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pslld, kX86InstIdPslld, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(pslld, kX86InstIdPslld, X86XmmVar, Imm)

  //! Packed QWORD shift left logical (SSE2).
  INST_2x(psllq, kX86InstIdPsllq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psllq, kX86InstIdPsllq, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psllq, kX86InstIdPsllq, X86XmmVar, Imm)

  //! Packed WORD shift left logical (SSE2).
  INST_2x(psllw, kX86InstIdPsllw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psllw, kX86InstIdPsllw, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psllw, kX86InstIdPsllw, X86XmmVar, Imm)

  //! Packed DQWORD shift left logical (SSE2).
  INST_2i(pslldq, kX86InstIdPslldq, X86XmmVar, Imm)

  //! Packed DWORD shift right arithmetic (SSE2).
  INST_2x(psrad, kX86InstIdPsrad, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psrad, kX86InstIdPsrad, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psrad, kX86InstIdPsrad, X86XmmVar, Imm)

  //! Packed WORD shift right arithmetic (SSE2).
  INST_2x(psraw, kX86InstIdPsraw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psraw, kX86InstIdPsraw, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psraw, kX86InstIdPsraw, X86XmmVar, Imm)

  //! Packed BYTE subtract (SSE2).
  INST_2x(psubb, kX86InstIdPsubb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubb, kX86InstIdPsubb, X86XmmVar, X86Mem)

  //! Packed DWORD subtract (SSE2).
  INST_2x(psubd, kX86InstIdPsubd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubd, kX86InstIdPsubd, X86XmmVar, X86Mem)

  //! Packed QWORD subtract (SSE2).
  INST_2x(psubq, kX86InstIdPsubq, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psubq, kX86InstIdPsubq, X86MmVar, X86Mem)

  //! Packed QWORD subtract (SSE2).
  INST_2x(psubq, kX86InstIdPsubq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubq, kX86InstIdPsubq, X86XmmVar, X86Mem)

  //! Packed WORD subtract (SSE2).
  INST_2x(psubw, kX86InstIdPsubw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubw, kX86InstIdPsubw, X86XmmVar, X86Mem)

  //! Packed WORD to DWORD multiply and add (SSE2).
  INST_2x(pmaddwd, kX86InstIdPmaddwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaddwd, kX86InstIdPmaddwd, X86XmmVar, X86Mem)

  //! Packed DWORD shuffle (SSE2).
  INST_3i(pshufd, kX86InstIdPshufd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pshufd, kX86InstIdPshufd, X86XmmVar, X86Mem, Imm)

  //! Packed WORD shuffle high (SSE2).
  INST_3i(pshufhw, kX86InstIdPshufhw, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pshufhw, kX86InstIdPshufhw, X86XmmVar, X86Mem, Imm)

  //! Packed WORD shuffle low (SSE2).
  INST_3i(pshuflw, kX86InstIdPshuflw, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pshuflw, kX86InstIdPshuflw, X86XmmVar, X86Mem, Imm)

  //! Packed DWORD shift right logical (SSE2).
  INST_2x(psrld, kX86InstIdPsrld, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psrld, kX86InstIdPsrld, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psrld, kX86InstIdPsrld, X86XmmVar, Imm)

  //! Packed QWORD shift right logical (SSE2).
  INST_2x(psrlq, kX86InstIdPsrlq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psrlq, kX86InstIdPsrlq, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psrlq, kX86InstIdPsrlq, X86XmmVar, Imm)

  //! Scalar DQWORD shift right logical (SSE2).
  INST_2i(psrldq, kX86InstIdPsrldq, X86XmmVar, Imm)

  //! Packed WORD shift right logical (SSE2).
  INST_2x(psrlw, kX86InstIdPsrlw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psrlw, kX86InstIdPsrlw, X86XmmVar, X86Mem)
  //! \overload
  INST_2i(psrlw, kX86InstIdPsrlw, X86XmmVar, Imm)

  //! Packed BYTE subtract with saturation (SSE2).
  INST_2x(psubsb, kX86InstIdPsubsb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubsb, kX86InstIdPsubsb, X86XmmVar, X86Mem)

  //! Packed WORD subtract with saturation (SSE2).
  INST_2x(psubsw, kX86InstIdPsubsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubsw, kX86InstIdPsubsw, X86XmmVar, X86Mem)

  //! Packed BYTE subtract with unsigned saturation (SSE2).
  INST_2x(psubusb, kX86InstIdPsubusb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubusb, kX86InstIdPsubusb, X86XmmVar, X86Mem)

  //! Packed WORD subtract with unsigned saturation (SSE2).
  INST_2x(psubusw, kX86InstIdPsubusw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psubusw, kX86InstIdPsubusw, X86XmmVar, X86Mem)

  //! Unpack high packed BYTEs to WORDs (SSE2).
  INST_2x(punpckhbw, kX86InstIdPunpckhbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpckhbw, kX86InstIdPunpckhbw, X86XmmVar, X86Mem)

  //! Unpack high packed DWORDs to QWORDs (SSE2).
  INST_2x(punpckhdq, kX86InstIdPunpckhdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpckhdq, kX86InstIdPunpckhdq, X86XmmVar, X86Mem)

  //! Unpack high packed QWORDs to DQWORD (SSE2).
  INST_2x(punpckhqdq, kX86InstIdPunpckhqdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpckhqdq, kX86InstIdPunpckhqdq, X86XmmVar, X86Mem)

  //! Unpack high packed WORDs to DWORDs (SSE2).
  INST_2x(punpckhwd, kX86InstIdPunpckhwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpckhwd, kX86InstIdPunpckhwd, X86XmmVar, X86Mem)

  //! Unpack low packed BYTEs to WORDs (SSE2).
  INST_2x(punpcklbw, kX86InstIdPunpcklbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpcklbw, kX86InstIdPunpcklbw, X86XmmVar, X86Mem)

  //! Unpack low packed DWORDs to QWORDs (SSE2).
  INST_2x(punpckldq, kX86InstIdPunpckldq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpckldq, kX86InstIdPunpckldq, X86XmmVar, X86Mem)

  //! Unpack low packed QWORDs to DQWORD (SSE2).
  INST_2x(punpcklqdq, kX86InstIdPunpcklqdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpcklqdq, kX86InstIdPunpcklqdq, X86XmmVar, X86Mem)

  //! Unpack low packed WORDs to DWORDs (SSE2).
  INST_2x(punpcklwd, kX86InstIdPunpcklwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(punpcklwd, kX86InstIdPunpcklwd, X86XmmVar, X86Mem)

  //! Packed bitwise xor (SSE2).
  INST_2x(pxor, kX86InstIdPxor, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pxor, kX86InstIdPxor, X86XmmVar, X86Mem)

  //! Shuffle DP-FP (SSE2).
  INST_3i(shufpd, kX86InstIdShufpd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(shufpd, kX86InstIdShufpd, X86XmmVar, X86Mem, Imm)

  //! Packed DP-FP square root (SSE2).
  INST_2x(sqrtpd, kX86InstIdSqrtpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sqrtpd, kX86InstIdSqrtpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP square root (SSE2).
  INST_2x(sqrtsd, kX86InstIdSqrtsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sqrtsd, kX86InstIdSqrtsd, X86XmmVar, X86Mem)

  //! Packed DP-FP subtract (SSE2).
  INST_2x(subpd, kX86InstIdSubpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(subpd, kX86InstIdSubpd, X86XmmVar, X86Mem)

  //! Scalar DP-FP subtract (SSE2).
  INST_2x(subsd, kX86InstIdSubsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(subsd, kX86InstIdSubsd, X86XmmVar, X86Mem)

  //! Scalar DP-FP unordered compare and set EFLAGS (SSE2).
  INST_2x(ucomisd, kX86InstIdUcomisd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(ucomisd, kX86InstIdUcomisd, X86XmmVar, X86Mem)

  //! Unpack and interleave high packed DP-FP (SSE2).
  INST_2x(unpckhpd, kX86InstIdUnpckhpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(unpckhpd, kX86InstIdUnpckhpd, X86XmmVar, X86Mem)

  //! Unpack and interleave low packed DP-FP (SSE2).
  INST_2x(unpcklpd, kX86InstIdUnpcklpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(unpcklpd, kX86InstIdUnpcklpd, X86XmmVar, X86Mem)

  //! Packed DP-FP bitwise xor (SSE2).
  INST_2x(xorpd, kX86InstIdXorpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(xorpd, kX86InstIdXorpd, X86XmmVar, X86Mem)

  // --------------------------------------------------------------------------
  // [SSE3]
  // --------------------------------------------------------------------------

  //! Packed DP-FP add/subtract (SSE3).
  INST_2x(addsubpd, kX86InstIdAddsubpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(addsubpd, kX86InstIdAddsubpd, X86XmmVar, X86Mem)

  //! Packed SP-FP add/subtract (SSE3).
  INST_2x(addsubps, kX86InstIdAddsubps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(addsubps, kX86InstIdAddsubps, X86XmmVar, X86Mem)

  //! Store truncated `fp0` to `short_or_int_or_long[o0]` and POP (FPU & SSE3).
  INST_1x(fisttp, kX86InstIdFisttp, X86Mem)

  //! Packed DP-FP horizontal add (SSE3).
  INST_2x(haddpd, kX86InstIdHaddpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(haddpd, kX86InstIdHaddpd, X86XmmVar, X86Mem)

  //! Packed SP-FP horizontal add (SSE3).
  INST_2x(haddps, kX86InstIdHaddps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(haddps, kX86InstIdHaddps, X86XmmVar, X86Mem)

  //! Packed DP-FP horizontal subtract (SSE3).
  INST_2x(hsubpd, kX86InstIdHsubpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(hsubpd, kX86InstIdHsubpd, X86XmmVar, X86Mem)

  //! Packed SP-FP horizontal subtract (SSE3).
  INST_2x(hsubps, kX86InstIdHsubps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(hsubps, kX86InstIdHsubps, X86XmmVar, X86Mem)

  //! Load 128-bits unaligned (SSE3).
  INST_2x(lddqu, kX86InstIdLddqu, X86XmmVar, X86Mem)

  // //! Setup monitor address (SSE3).
  // INST_0x(monitor, kX86InstIdMonitor)

  //! Move one DP-FP and duplicate (SSE3).
  INST_2x(movddup, kX86InstIdMovddup, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movddup, kX86InstIdMovddup, X86XmmVar, X86Mem)

  //! Move packed SP-FP high and duplicate (SSE3).
  INST_2x(movshdup, kX86InstIdMovshdup, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movshdup, kX86InstIdMovshdup, X86XmmVar, X86Mem)

  //! Move packed SP-FP low and duplicate (SSE3).
  INST_2x(movsldup, kX86InstIdMovsldup, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(movsldup, kX86InstIdMovsldup, X86XmmVar, X86Mem)

  // //! Monitor wait (SSE3).
  // INST_0x(mwait, kX86InstIdMwait)

  // --------------------------------------------------------------------------
  // [SSSE3]
  // --------------------------------------------------------------------------

  //! Packed BYTE sign (SSSE3).
  INST_2x(psignb, kX86InstIdPsignb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psignb, kX86InstIdPsignb, X86MmVar, X86Mem)

  //! PackedBYTE  sign (SSSE3).
  INST_2x(psignb, kX86InstIdPsignb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psignb, kX86InstIdPsignb, X86XmmVar, X86Mem)

  //! Packed DWORD sign (SSSE3).
  INST_2x(psignd, kX86InstIdPsignd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psignd, kX86InstIdPsignd, X86MmVar, X86Mem)

  //! Packed DWORD sign (SSSE3).
  INST_2x(psignd, kX86InstIdPsignd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psignd, kX86InstIdPsignd, X86XmmVar, X86Mem)

  //! Packed WORD sign (SSSE3).
  INST_2x(psignw, kX86InstIdPsignw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(psignw, kX86InstIdPsignw, X86MmVar, X86Mem)

  //! Packed WORD sign (SSSE3).
  INST_2x(psignw, kX86InstIdPsignw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(psignw, kX86InstIdPsignw, X86XmmVar, X86Mem)

  //! Packed DWORD horizontal add (SSSE3).
  INST_2x(phaddd, kX86InstIdPhaddd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(phaddd, kX86InstIdPhaddd, X86MmVar, X86Mem)

  //! Packed DWORD horizontal add (SSSE3).
  INST_2x(phaddd, kX86InstIdPhaddd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phaddd, kX86InstIdPhaddd, X86XmmVar, X86Mem)

  //! Packed WORD horizontal add with saturation (SSSE3).
  INST_2x(phaddsw, kX86InstIdPhaddsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(phaddsw, kX86InstIdPhaddsw, X86MmVar, X86Mem)

  //! Packed WORD horizontal add with with saturation (SSSE3).
  INST_2x(phaddsw, kX86InstIdPhaddsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phaddsw, kX86InstIdPhaddsw, X86XmmVar, X86Mem)

  //! Packed WORD horizontal add (SSSE3).
  INST_2x(phaddw, kX86InstIdPhaddw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(phaddw, kX86InstIdPhaddw, X86MmVar, X86Mem)

  //! Packed WORD horizontal add (SSSE3).
  INST_2x(phaddw, kX86InstIdPhaddw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phaddw, kX86InstIdPhaddw, X86XmmVar, X86Mem)

  //! Packed DWORD horizontal subtract (SSSE3).
  INST_2x(phsubd, kX86InstIdPhsubd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(phsubd, kX86InstIdPhsubd, X86MmVar, X86Mem)

  //! Packed DWORD horizontal subtract (SSSE3).
  INST_2x(phsubd, kX86InstIdPhsubd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phsubd, kX86InstIdPhsubd, X86XmmVar, X86Mem)

  //! Packed WORD horizontal subtract with saturation (SSSE3).
  INST_2x(phsubsw, kX86InstIdPhsubsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(phsubsw, kX86InstIdPhsubsw, X86MmVar, X86Mem)

  //! Packed WORD horizontal subtract with saturation (SSSE3).
  INST_2x(phsubsw, kX86InstIdPhsubsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phsubsw, kX86InstIdPhsubsw, X86XmmVar, X86Mem)

  //! Packed WORD horizontal subtract (SSSE3).
  INST_2x(phsubw, kX86InstIdPhsubw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(phsubw, kX86InstIdPhsubw, X86MmVar, X86Mem)

  //! Packed WORD horizontal subtract (SSSE3).
  INST_2x(phsubw, kX86InstIdPhsubw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phsubw, kX86InstIdPhsubw, X86XmmVar, X86Mem)

  //! Packed multiply and add signed and unsigned bytes (SSSE3).
  INST_2x(pmaddubsw, kX86InstIdPmaddubsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmaddubsw, kX86InstIdPmaddubsw, X86MmVar, X86Mem)

  //! Packed multiply and add signed and unsigned bytes (SSSE3).
  INST_2x(pmaddubsw, kX86InstIdPmaddubsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaddubsw, kX86InstIdPmaddubsw, X86XmmVar, X86Mem)

  //! Packed BYTE absolute value (SSSE3).
  INST_2x(pabsb, kX86InstIdPabsb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pabsb, kX86InstIdPabsb, X86MmVar, X86Mem)

  //! Packed BYTE absolute value (SSSE3).
  INST_2x(pabsb, kX86InstIdPabsb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pabsb, kX86InstIdPabsb, X86XmmVar, X86Mem)

  //! Packed DWORD absolute value (SSSE3).
  INST_2x(pabsd, kX86InstIdPabsd, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pabsd, kX86InstIdPabsd, X86MmVar, X86Mem)

  //! Packed DWORD absolute value (SSSE3).
  INST_2x(pabsd, kX86InstIdPabsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pabsd, kX86InstIdPabsd, X86XmmVar, X86Mem)

  //! Packed WORD absolute value (SSSE3).
  INST_2x(pabsw, kX86InstIdPabsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pabsw, kX86InstIdPabsw, X86MmVar, X86Mem)

  //! Packed WORD absolute value (SSSE3).
  INST_2x(pabsw, kX86InstIdPabsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pabsw, kX86InstIdPabsw, X86XmmVar, X86Mem)

  //! Packed WORD multiply high, round and scale (SSSE3).
  INST_2x(pmulhrsw, kX86InstIdPmulhrsw, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pmulhrsw, kX86InstIdPmulhrsw, X86MmVar, X86Mem)

  //! Packed WORD multiply high, round and scale (SSSE3).
  INST_2x(pmulhrsw, kX86InstIdPmulhrsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmulhrsw, kX86InstIdPmulhrsw, X86XmmVar, X86Mem)

  //! Packed BYTE shuffle (SSSE3).
  INST_2x(pshufb, kX86InstIdPshufb, X86MmVar, X86MmVar)
  //! \overload
  INST_2x(pshufb, kX86InstIdPshufb, X86MmVar, X86Mem)

  //! Packed BYTE shuffle (SSSE3).
  INST_2x(pshufb, kX86InstIdPshufb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pshufb, kX86InstIdPshufb, X86XmmVar, X86Mem)

  //! Packed align right (SSSE3).
  INST_3i(palignr, kX86InstIdPalignr, X86MmVar, X86MmVar, Imm)
  //! \overload
  INST_3i(palignr, kX86InstIdPalignr, X86MmVar, X86Mem, Imm)

  //! Packed align right (SSSE3).
  INST_3i(palignr, kX86InstIdPalignr, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(palignr, kX86InstIdPalignr, X86XmmVar, X86Mem, Imm)

  // --------------------------------------------------------------------------
  // [SSE4.1]
  // --------------------------------------------------------------------------

  //! Packed DP-FP blend (SSE4.1).
  INST_3i(blendpd, kX86InstIdBlendpd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(blendpd, kX86InstIdBlendpd, X86XmmVar, X86Mem, Imm)

  //! Packed SP-FP blend (SSE4.1).
  INST_3i(blendps, kX86InstIdBlendps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(blendps, kX86InstIdBlendps, X86XmmVar, X86Mem, Imm)

  //! Packed DP-FP variable blend (SSE4.1).
  INST_3x(blendvpd, kX86InstIdBlendvpd, X86XmmVar, X86XmmVar, X86XmmVar  /* XMM0 */)
  //! \overload
  INST_3x(blendvpd, kX86InstIdBlendvpd, X86XmmVar, X86Mem, X86XmmVar /* XMM0 */)

  //! Packed SP-FP variable blend (SSE4.1).
  INST_3x(blendvps, kX86InstIdBlendvps, X86XmmVar, X86XmmVar, X86XmmVar /* XMM0 */)
  //! \overload
  INST_3x(blendvps, kX86InstIdBlendvps, X86XmmVar, X86Mem, X86XmmVar /* XMM0 */)

  //! Packed DP-FP dot product (SSE4.1).
  INST_3i(dppd, kX86InstIdDppd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(dppd, kX86InstIdDppd, X86XmmVar, X86Mem, Imm)

  //! Packed SP-FP dot product (SSE4.1).
  INST_3i(dpps, kX86InstIdDpps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(dpps, kX86InstIdDpps, X86XmmVar, X86Mem, Imm)

  //! Extract SP-FP based on selector (SSE4.1).
  INST_3i(extractps, kX86InstIdExtractps, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(extractps, kX86InstIdExtractps, X86Mem, X86XmmVar, Imm)

  //! Insert SP-FP based on selector (SSE4.1).
  INST_3i(insertps, kX86InstIdInsertps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(insertps, kX86InstIdInsertps, X86XmmVar, X86Mem, Imm)

  //! Load DQWORD aligned using NT hint (SSE4.1).
  INST_2x(movntdqa, kX86InstIdMovntdqa, X86XmmVar, X86Mem)

  //! Packed WORD sums of absolute difference (SSE4.1).
  INST_3i(mpsadbw, kX86InstIdMpsadbw, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(mpsadbw, kX86InstIdMpsadbw, X86XmmVar, X86Mem, Imm)

  //! Pack DWORDs to WORDs with unsigned saturation (SSE4.1).
  INST_2x(packusdw, kX86InstIdPackusdw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(packusdw, kX86InstIdPackusdw, X86XmmVar, X86Mem)

  //! Packed BYTE variable blend (SSE4.1).
  INST_3x(pblendvb, kX86InstIdPblendvb, X86XmmVar, X86XmmVar, X86XmmVar /* XMM0 */)
  //! \overload
  INST_3x(pblendvb, kX86InstIdPblendvb, X86XmmVar, X86Mem, X86XmmVar /* XMM0 */)

  //! Packed WORD blend (SSE4.1).
  INST_3i(pblendw, kX86InstIdPblendw, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pblendw, kX86InstIdPblendw, X86XmmVar, X86Mem, Imm)

  //! Packed QWORD compare for equality (SSE4.1).
  INST_2x(pcmpeqq, kX86InstIdPcmpeqq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpeqq, kX86InstIdPcmpeqq, X86XmmVar, X86Mem)

  //! Extract BYTE based on selector (SSE4.1).
  INST_3i(pextrb, kX86InstIdPextrb, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pextrb, kX86InstIdPextrb, X86Mem, X86XmmVar, Imm)

  //! Extract DWORD based on selector (SSE4.1).
  INST_3i(pextrd, kX86InstIdPextrd, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pextrd, kX86InstIdPextrd, X86Mem, X86XmmVar, Imm)

  //! Extract QWORD based on selector (SSE4.1).
  INST_3i(pextrq, kX86InstIdPextrq, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(pextrq, kX86InstIdPextrq, X86Mem, X86XmmVar, Imm)

  //! Extract WORD based on selector (SSE4.1).
  INST_3i(pextrw, kX86InstIdPextrw, X86Mem, X86XmmVar, Imm)

  //! Packed WORD horizontal minimum (SSE4.1).
  INST_2x(phminposuw, kX86InstIdPhminposuw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(phminposuw, kX86InstIdPhminposuw, X86XmmVar, X86Mem)

  //! Insert BYTE based on selector (SSE4.1).
  INST_3i(pinsrb, kX86InstIdPinsrb, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_3i(pinsrb, kX86InstIdPinsrb, X86XmmVar, X86Mem, Imm)

  //! Insert DWORD based on selector (SSE4.1).
  INST_3i(pinsrd, kX86InstIdPinsrd, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_3i(pinsrd, kX86InstIdPinsrd, X86XmmVar, X86Mem, Imm)

  //! Insert QWORD based on selector (SSE4.1).
  INST_3i(pinsrq, kX86InstIdPinsrq, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_3i(pinsrq, kX86InstIdPinsrq, X86XmmVar, X86Mem, Imm)

  //! Packed BYTE maximum (SSE4.1).
  INST_2x(pmaxsb, kX86InstIdPmaxsb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaxsb, kX86InstIdPmaxsb, X86XmmVar, X86Mem)

  //! Packed DWORD maximum (SSE4.1).
  INST_2x(pmaxsd, kX86InstIdPmaxsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaxsd, kX86InstIdPmaxsd, X86XmmVar, X86Mem)

  //! Packed DWORD unsigned maximum (SSE4.1).
  INST_2x(pmaxud, kX86InstIdPmaxud, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaxud,kX86InstIdPmaxud , X86XmmVar, X86Mem)

  //! Packed WORD unsigned maximum (SSE4.1).
  INST_2x(pmaxuw, kX86InstIdPmaxuw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmaxuw, kX86InstIdPmaxuw, X86XmmVar, X86Mem)

  //! Packed BYTE minimum (SSE4.1).
  INST_2x(pminsb, kX86InstIdPminsb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pminsb, kX86InstIdPminsb, X86XmmVar, X86Mem)

  //! Packed DWORD minimum (SSE4.1).
  INST_2x(pminsd, kX86InstIdPminsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pminsd, kX86InstIdPminsd, X86XmmVar, X86Mem)

  //! Packed WORD unsigned minimum (SSE4.1).
  INST_2x(pminuw, kX86InstIdPminuw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pminuw, kX86InstIdPminuw, X86XmmVar, X86Mem)

  //! Packed DWORD unsigned minimum (SSE4.1).
  INST_2x(pminud, kX86InstIdPminud, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pminud, kX86InstIdPminud, X86XmmVar, X86Mem)

  //! Packed BYTE to DWORD with sign extend (SSE4.1).
  INST_2x(pmovsxbd, kX86InstIdPmovsxbd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovsxbd, kX86InstIdPmovsxbd, X86XmmVar, X86Mem)

  //! Packed BYTE to QWORD with sign extend (SSE4.1).
  INST_2x(pmovsxbq, kX86InstIdPmovsxbq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovsxbq, kX86InstIdPmovsxbq, X86XmmVar, X86Mem)

  //! Packed BYTE to WORD with sign extend (SSE4.1).
  INST_2x(pmovsxbw, kX86InstIdPmovsxbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovsxbw, kX86InstIdPmovsxbw, X86XmmVar, X86Mem)

  //! Packed DWORD to QWORD with sign extend (SSE4.1).
  INST_2x(pmovsxdq, kX86InstIdPmovsxdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovsxdq, kX86InstIdPmovsxdq, X86XmmVar, X86Mem)

  //! Packed WORD to DWORD with sign extend (SSE4.1).
  INST_2x(pmovsxwd, kX86InstIdPmovsxwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovsxwd, kX86InstIdPmovsxwd, X86XmmVar, X86Mem)

  //! Packed WORD to QWORD with sign extend (SSE4.1).
  INST_2x(pmovsxwq, kX86InstIdPmovsxwq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovsxwq, kX86InstIdPmovsxwq, X86XmmVar, X86Mem)

  //! BYTE to DWORD with zero extend (SSE4.1).
  INST_2x(pmovzxbd, kX86InstIdPmovzxbd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovzxbd, kX86InstIdPmovzxbd, X86XmmVar, X86Mem)

  //! Packed BYTE to QWORD with zero extend (SSE4.1).
  INST_2x(pmovzxbq, kX86InstIdPmovzxbq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovzxbq, kX86InstIdPmovzxbq, X86XmmVar, X86Mem)

  //! BYTE to WORD with zero extend (SSE4.1).
  INST_2x(pmovzxbw, kX86InstIdPmovzxbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovzxbw, kX86InstIdPmovzxbw, X86XmmVar, X86Mem)

  //! Packed DWORD to QWORD with zero extend (SSE4.1).
  INST_2x(pmovzxdq, kX86InstIdPmovzxdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovzxdq, kX86InstIdPmovzxdq, X86XmmVar, X86Mem)

  //! Packed WORD to DWORD with zero extend (SSE4.1).
  INST_2x(pmovzxwd, kX86InstIdPmovzxwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovzxwd, kX86InstIdPmovzxwd, X86XmmVar, X86Mem)

  //! Packed WORD to QWORD with zero extend (SSE4.1).
  INST_2x(pmovzxwq, kX86InstIdPmovzxwq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmovzxwq, kX86InstIdPmovzxwq, X86XmmVar, X86Mem)

  //! Packed DWORD to QWORD multiply (SSE4.1).
  INST_2x(pmuldq, kX86InstIdPmuldq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmuldq, kX86InstIdPmuldq, X86XmmVar, X86Mem)

  //! Packed DWORD multiply low (SSE4.1).
  INST_2x(pmulld, kX86InstIdPmulld, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pmulld, kX86InstIdPmulld, X86XmmVar, X86Mem)

  //! Logical compare (SSE4.1).
  INST_2x(ptest, kX86InstIdPtest, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(ptest, kX86InstIdPtest, X86XmmVar, X86Mem)

  //! Packed DP-FP round (SSE4.1).
  INST_3i(roundpd, kX86InstIdRoundpd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(roundpd, kX86InstIdRoundpd, X86XmmVar, X86Mem, Imm)

  //! Packed SP-FP round (SSE4.1).
  INST_3i(roundps, kX86InstIdRoundps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(roundps, kX86InstIdRoundps, X86XmmVar, X86Mem, Imm)

  //! Scalar DP-FP round (SSE4.1).
  INST_3i(roundsd, kX86InstIdRoundsd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(roundsd, kX86InstIdRoundsd, X86XmmVar, X86Mem, Imm)

  //! Scalar SP-FP round (SSE4.1).
  INST_3i(roundss, kX86InstIdRoundss, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(roundss, kX86InstIdRoundss, X86XmmVar, X86Mem, Imm)

  // --------------------------------------------------------------------------
  // [SSE4.2]
  // --------------------------------------------------------------------------

  //! Accumulate CRC32 value (polynomial 0x11EDC6F41) (SSE4.2).
  INST_2x(crc32, kX86InstIdCrc32, X86GpVar, X86GpVar)
  //! \overload
  INST_2x(crc32, kX86InstIdCrc32, X86GpVar, X86Mem)

  //! Packed compare explicit length strings, return index in ECX (SSE4.2).
  INST_4i(pcmpestri, kX86InstIdPcmpestri, X86GpVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(pcmpestri, kX86InstIdPcmpestri, X86GpVar, X86XmmVar, X86Mem, Imm)

  //! Packed compare explicit length strings, return mask in XMM0 (SSE4.2).
  INST_4i(pcmpestrm, kX86InstIdPcmpestrm, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(pcmpestrm, kX86InstIdPcmpestrm, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed compare implicit length strings, return index in ECX (SSE4.2).
  INST_4i(pcmpistri, kX86InstIdPcmpistri, X86GpVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(pcmpistri, kX86InstIdPcmpistri, X86GpVar, X86XmmVar, X86Mem, Imm)

  //! Packed compare implicit length strings, return mask in XMM0 (SSE4.2).
  INST_4i(pcmpistrm, kX86InstIdPcmpistrm, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(pcmpistrm, kX86InstIdPcmpistrm, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed QWORD compare if greater than (SSE4.2).
  INST_2x(pcmpgtq, kX86InstIdPcmpgtq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(pcmpgtq, kX86InstIdPcmpgtq, X86XmmVar, X86Mem)

  // --------------------------------------------------------------------------
  // [SSE4a]
  // --------------------------------------------------------------------------

  //! Extract Field (SSE4a).
  INST_2x(extrq, kX86InstIdExtrq, X86XmmVar, X86XmmVar)
  //! Extract Field (SSE4a).
  INST_3ii(extrq, kX86InstIdExtrq, X86XmmVar, Imm, Imm)

  //! Insert Field (SSE4a).
  INST_2x(insertq, kX86InstIdInsertq, X86XmmVar, X86XmmVar)
  //! Insert Field (SSE4a).
  INST_4ii(insertq, kX86InstIdInsertq, X86XmmVar, X86XmmVar, Imm, Imm)

  //! Move Non-Temporal Scalar DP-FP (SSE4a).
  INST_2x(movntsd, kX86InstIdMovntsd, X86Mem, X86XmmVar)
  //! Move Non-Temporal Scalar SP-FP (SSE4a).
  INST_2x(movntss, kX86InstIdMovntss, X86Mem, X86XmmVar)

  // --------------------------------------------------------------------------
  // [AESNI]
  // --------------------------------------------------------------------------

  //! Perform a single round of the AES decryption flow.
  INST_2x(aesdec, kX86InstIdAesdec, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(aesdec, kX86InstIdAesdec, X86XmmVar, X86Mem)

  //! Perform the last round of the AES decryption flow.
  INST_2x(aesdeclast, kX86InstIdAesdeclast, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(aesdeclast, kX86InstIdAesdeclast, X86XmmVar, X86Mem)

  //! Perform a single round of the AES encryption flow.
  INST_2x(aesenc, kX86InstIdAesenc, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(aesenc, kX86InstIdAesenc, X86XmmVar, X86Mem)

  //! Perform the last round of the AES encryption flow.
  INST_2x(aesenclast, kX86InstIdAesenclast, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(aesenclast, kX86InstIdAesenclast, X86XmmVar, X86Mem)

  //! Perform the InvMixColumns transformation.
  INST_2x(aesimc, kX86InstIdAesimc, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(aesimc, kX86InstIdAesimc, X86XmmVar, X86Mem)

  //! Assist in expanding the AES cipher key.
  INST_3i(aeskeygenassist, kX86InstIdAeskeygenassist, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(aeskeygenassist, kX86InstIdAeskeygenassist, X86XmmVar, X86Mem, Imm)

  // --------------------------------------------------------------------------
  // [SHA]
  // --------------------------------------------------------------------------

  //! Perform an intermediate calculation for the next four SHA1 message DWORDs (SHA).
  INST_2x(sha1msg1, kX86InstIdSha1msg1, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sha1msg1, kX86InstIdSha1msg1, X86XmmVar, X86Mem)

  //! Perform a final calculation for the next four SHA1 message DWORDs (SHA).
  INST_2x(sha1msg2, kX86InstIdSha1msg2, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sha1msg2, kX86InstIdSha1msg2, X86XmmVar, X86Mem)

  //! Calculate SHA1 state variable E after four rounds (SHA).
  INST_2x(sha1nexte, kX86InstIdSha1nexte, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sha1nexte, kX86InstIdSha1nexte, X86XmmVar, X86Mem)

  //! Perform four rounds of SHA1 operation (SHA).
  INST_3i(sha1rnds4, kX86InstIdSha1rnds4, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(sha1rnds4, kX86InstIdSha1rnds4, X86XmmVar, X86Mem, Imm)

  //! Perform an intermediate calculation for the next four SHA256 message DWORDs (SHA).
  INST_2x(sha256msg1, kX86InstIdSha256msg1, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sha256msg1, kX86InstIdSha256msg1, X86XmmVar, X86Mem)

  //! Perform a final calculation for the next four SHA256 message DWORDs (SHA).
  INST_2x(sha256msg2, kX86InstIdSha256msg2, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(sha256msg2, kX86InstIdSha256msg2, X86XmmVar, X86Mem)

  //! Perform two rounds of SHA256 operation (SHA).
  INST_3x(sha256rnds2, kX86InstIdSha256rnds2, X86XmmVar, X86XmmVar, X86XmmVar /* XMM0 */)
  //! \overload
  INST_3x(sha256rnds2, kX86InstIdSha256rnds2, X86XmmVar, X86Mem, X86XmmVar /* XMM0 */)

  // --------------------------------------------------------------------------
  // [PCLMULQDQ]
  // --------------------------------------------------------------------------

  //! Packed QWORD to DQWORD carry-less multiply (PCLMULQDQ).
  INST_3i(pclmulqdq, kX86InstIdPclmulqdq, X86XmmVar, X86XmmVar, Imm);
  //! \overload
  INST_3i(pclmulqdq, kX86InstIdPclmulqdq, X86XmmVar, X86Mem, Imm);

  // --------------------------------------------------------------------------
  // [AVX]
  // --------------------------------------------------------------------------

  //! Packed DP-FP add (AVX).
  INST_3x(vaddpd, kX86InstIdVaddpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaddpd, kX86InstIdVaddpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vaddpd, kX86InstIdVaddpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vaddpd, kX86InstIdVaddpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP add (AVX).
  INST_3x(vaddps, kX86InstIdVaddps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaddps, kX86InstIdVaddps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vaddps, kX86InstIdVaddps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vaddps, kX86InstIdVaddps, X86YmmVar, X86YmmVar, X86Mem)

  //! Scalar DP-FP add (AVX)
  INST_3x(vaddsd, kX86InstIdVaddsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaddsd, kX86InstIdVaddsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP add (AVX)
  INST_3x(vaddss, kX86InstIdVaddss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaddss, kX86InstIdVaddss, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DP-FP add/subtract (AVX).
  INST_3x(vaddsubpd, kX86InstIdVaddsubpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaddsubpd, kX86InstIdVaddsubpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vaddsubpd, kX86InstIdVaddsubpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vaddsubpd, kX86InstIdVaddsubpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP add/subtract (AVX).
  INST_3x(vaddsubps, kX86InstIdVaddsubps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaddsubps, kX86InstIdVaddsubps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vaddsubps, kX86InstIdVaddsubps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vaddsubps, kX86InstIdVaddsubps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DP-FP bitwise and (AVX).
  INST_3x(vandpd, kX86InstIdVandpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vandpd, kX86InstIdVandpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vandpd, kX86InstIdVandpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vandpd, kX86InstIdVandpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP bitwise and (AVX).
  INST_3x(vandps, kX86InstIdVandps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vandps, kX86InstIdVandps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vandps, kX86InstIdVandps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vandps, kX86InstIdVandps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DP-FP bitwise and-not (AVX).
  INST_3x(vandnpd, kX86InstIdVandnpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vandnpd, kX86InstIdVandnpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vandnpd, kX86InstIdVandnpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vandnpd, kX86InstIdVandnpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP bitwise and-not (AVX).
  INST_3x(vandnps, kX86InstIdVandnps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vandnps, kX86InstIdVandnps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vandnps, kX86InstIdVandnps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vandnps, kX86InstIdVandnps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DP-FP blend (AVX).
  INST_4i(vblendpd, kX86InstIdVblendpd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vblendpd, kX86InstIdVblendpd, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vblendpd, kX86InstIdVblendpd, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vblendpd, kX86InstIdVblendpd, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed SP-FP blend (AVX).
  INST_4i(vblendps, kX86InstIdVblendps, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vblendps, kX86InstIdVblendps, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vblendps, kX86InstIdVblendps, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vblendps, kX86InstIdVblendps, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed DP-FP variable blend (AVX).
  INST_4x(vblendvpd, kX86InstIdVblendvpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_4x(vblendvpd, kX86InstIdVblendvpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  //! \overload
  INST_4x(vblendvpd, kX86InstIdVblendvpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_4x(vblendvpd, kX86InstIdVblendvpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)

  //! Packed SP-FP variable blend (AVX).
  INST_4x(vblendvps, kX86InstIdVblendvps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_4x(vblendvps, kX86InstIdVblendvps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  //! \overload
  INST_4x(vblendvps, kX86InstIdVblendvps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_4x(vblendvps, kX86InstIdVblendvps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)

  //! Broadcast 128-bits of FP data in `o1` to low and high 128-bits in `o0` (AVX).
  INST_2x(vbroadcastf128, kX86InstIdVbroadcastf128, X86YmmVar, X86Mem)
  //! Broadcast DP-FP element in `o1` to four locations in `o0` (AVX).
  INST_2x(vbroadcastsd, kX86InstIdVbroadcastsd, X86YmmVar, X86Mem)
  //! Broadcast SP-FP element in `o1` to four locations in `o0` (AVX).
  INST_2x(vbroadcastss, kX86InstIdVbroadcastss, X86XmmVar, X86Mem)
  //! Broadcast SP-FP element in `o1` to eight locations in `o0` (AVX).
  INST_2x(vbroadcastss, kX86InstIdVbroadcastss, X86YmmVar, X86Mem)

  //! Packed DP-FP compare (AVX).
  INST_4i(vcmppd, kX86InstIdVcmppd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vcmppd, kX86InstIdVcmppd, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vcmppd, kX86InstIdVcmppd, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vcmppd, kX86InstIdVcmppd, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed SP-FP compare (AVX).
  INST_4i(vcmpps, kX86InstIdVcmpps, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vcmpps, kX86InstIdVcmpps, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vcmpps, kX86InstIdVcmpps, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vcmpps, kX86InstIdVcmpps, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Scalar DP-FP compare (AVX).
  INST_4i(vcmpsd, kX86InstIdVcmpsd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vcmpsd, kX86InstIdVcmpsd, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Scalar SP-FP compare (AVX).
  INST_4i(vcmpss, kX86InstIdVcmpss, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vcmpss, kX86InstIdVcmpss, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Scalar DP-FP ordered compare and set EFLAGS (AVX).
  INST_2x(vcomisd, kX86InstIdVcomisd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcomisd, kX86InstIdVcomisd, X86XmmVar, X86Mem)

  //! Scalar SP-FP ordered compare and set EFLAGS (AVX).
  INST_2x(vcomiss, kX86InstIdVcomiss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcomiss, kX86InstIdVcomiss, X86XmmVar, X86Mem)

  //! Convert packed QWORDs to packed DP-FP (AVX).
  INST_2x(vcvtdq2pd, kX86InstIdVcvtdq2pd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtdq2pd, kX86InstIdVcvtdq2pd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vcvtdq2pd, kX86InstIdVcvtdq2pd, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtdq2pd, kX86InstIdVcvtdq2pd, X86YmmVar, X86Mem)

  //! Convert packed QWORDs to packed SP-FP (AVX).
  INST_2x(vcvtdq2ps, kX86InstIdVcvtdq2ps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtdq2ps, kX86InstIdVcvtdq2ps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vcvtdq2ps, kX86InstIdVcvtdq2ps, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vcvtdq2ps, kX86InstIdVcvtdq2ps, X86YmmVar, X86Mem)

  //! Convert packed DP-FP to packed DWORDs (AVX).
  INST_2x(vcvtpd2dq, kX86InstIdVcvtpd2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtpd2dq, kX86InstIdVcvtpd2dq, X86XmmVar, X86YmmVar)
  //! \overload
  INST_2x(vcvtpd2dq, kX86InstIdVcvtpd2dq, X86XmmVar, X86Mem)

  //! Convert packed DP-FP to packed SP-FP (AVX).
  INST_2x(vcvtpd2ps, kX86InstIdVcvtpd2ps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtpd2ps, kX86InstIdVcvtpd2ps, X86XmmVar, X86YmmVar)
  //! \overload
  INST_2x(vcvtpd2ps, kX86InstIdVcvtpd2ps, X86XmmVar, X86Mem)

  //! Convert packed SP-FP to packed DWORDs (AVX).
  INST_2x(vcvtps2dq, kX86InstIdVcvtps2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtps2dq, kX86InstIdVcvtps2dq, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vcvtps2dq, kX86InstIdVcvtps2dq, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vcvtps2dq, kX86InstIdVcvtps2dq, X86YmmVar, X86Mem)

  //! Convert packed SP-FP to packed DP-FP (AVX).
  INST_2x(vcvtps2pd, kX86InstIdVcvtps2pd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtps2pd, kX86InstIdVcvtps2pd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vcvtps2pd, kX86InstIdVcvtps2pd, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtps2pd, kX86InstIdVcvtps2pd, X86YmmVar, X86Mem)

  //! Convert scalar DP-FP to DWORD (AVX).
  INST_2x(vcvtsd2si, kX86InstIdVcvtsd2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtsd2si, kX86InstIdVcvtsd2si, X86GpVar, X86Mem)

  //! Convert scalar DP-FP to scalar SP-FP (AVX).
  INST_3x(vcvtsd2ss, kX86InstIdVcvtsd2ss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vcvtsd2ss, kX86InstIdVcvtsd2ss, X86XmmVar, X86XmmVar, X86Mem)

  //! Convert DWORD integer to scalar DP-FP (AVX).
  INST_3x(vcvtsi2sd, kX86InstIdVcvtsi2sd, X86XmmVar, X86XmmVar, X86GpVar)
  //! \overload
  INST_3x(vcvtsi2sd, kX86InstIdVcvtsi2sd, X86XmmVar, X86XmmVar, X86Mem)

  //! Convert scalar INT32 to SP-FP (AVX).
  INST_3x(vcvtsi2ss, kX86InstIdVcvtsi2ss, X86XmmVar, X86XmmVar, X86GpVar)
  //! \overload
  INST_3x(vcvtsi2ss, kX86InstIdVcvtsi2ss, X86XmmVar, X86XmmVar, X86Mem)

  //! Convert scalar SP-FP to DP-FP (AVX).
  INST_3x(vcvtss2sd, kX86InstIdVcvtss2sd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vcvtss2sd, kX86InstIdVcvtss2sd, X86XmmVar, X86XmmVar, X86Mem)

  //! Convert scalar SP-FP to INT32 (AVX).
  INST_2x(vcvtss2si, kX86InstIdVcvtss2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtss2si, kX86InstIdVcvtss2si, X86GpVar, X86Mem)

  //! Convert with truncation packed DP-FP to packed DWORDs (AVX).
  INST_2x(vcvttpd2dq, kX86InstIdVcvttpd2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvttpd2dq, kX86InstIdVcvttpd2dq, X86XmmVar, X86YmmVar)
  //! \overload
  INST_2x(vcvttpd2dq, kX86InstIdVcvttpd2dq, X86XmmVar, X86Mem)

  //! Convert with truncation packed SP-FP to packed DWORDs (AVX).
  INST_2x(vcvttps2dq, kX86InstIdVcvttps2dq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvttps2dq, kX86InstIdVcvttps2dq, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vcvttps2dq, kX86InstIdVcvttps2dq, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vcvttps2dq, kX86InstIdVcvttps2dq, X86YmmVar, X86Mem)

  //! Convert with truncation scalar DP-FP to INT32 (AVX).
  INST_2x(vcvttsd2si, kX86InstIdVcvttsd2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vcvttsd2si, kX86InstIdVcvttsd2si, X86GpVar, X86Mem)

  //! Convert with truncation scalar SP-FP to INT32 (AVX).
  INST_2x(vcvttss2si, kX86InstIdVcvttss2si, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vcvttss2si, kX86InstIdVcvttss2si, X86GpVar, X86Mem)

  //! Packed DP-FP divide (AVX).
  INST_3x(vdivpd, kX86InstIdVdivpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vdivpd, kX86InstIdVdivpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vdivpd, kX86InstIdVdivpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vdivpd, kX86InstIdVdivpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP divide (AVX).
  INST_3x(vdivps, kX86InstIdVdivps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vdivps, kX86InstIdVdivps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vdivps, kX86InstIdVdivps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vdivps, kX86InstIdVdivps, X86YmmVar, X86YmmVar, X86Mem)

  //! Scalar DP-FP divide (AVX).
  INST_3x(vdivsd, kX86InstIdVdivsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vdivsd, kX86InstIdVdivsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP divide (AVX).
  INST_3x(vdivss, kX86InstIdVdivss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vdivss, kX86InstIdVdivss, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DP-FP dot product (AVX).
  INST_4i(vdppd, kX86InstIdVdppd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vdppd, kX86InstIdVdppd, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed SP-FP dot product (AVX).
  INST_4i(vdpps, kX86InstIdVdpps, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vdpps, kX86InstIdVdpps, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vdpps, kX86InstIdVdpps, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vdpps, kX86InstIdVdpps, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Extract 128 bits of packed FP data from `o1` and store results in `o0` (AVX).
  INST_3i(vextractf128, kX86InstIdVextractf128, X86XmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vextractf128, kX86InstIdVextractf128, X86Mem, X86YmmVar, Imm)

  //! Extract SP-FP based on selector (AVX).
  INST_3i(vextractps, kX86InstIdVextractps, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vextractps, kX86InstIdVextractps, X86Mem, X86XmmVar, Imm)

  //! Packed DP-FP horizontal add (AVX).
  INST_3x(vhaddpd, kX86InstIdVhaddpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vhaddpd, kX86InstIdVhaddpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vhaddpd, kX86InstIdVhaddpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vhaddpd, kX86InstIdVhaddpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP horizontal add (AVX).
  INST_3x(vhaddps, kX86InstIdVhaddps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vhaddps, kX86InstIdVhaddps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vhaddps, kX86InstIdVhaddps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vhaddps, kX86InstIdVhaddps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DP-FP horizontal subtract (AVX).
  INST_3x(vhsubpd, kX86InstIdVhsubpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vhsubpd, kX86InstIdVhsubpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vhsubpd, kX86InstIdVhsubpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vhsubpd, kX86InstIdVhsubpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP horizontal subtract (AVX).
  INST_3x(vhsubps, kX86InstIdVhsubps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vhsubps, kX86InstIdVhsubps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vhsubps, kX86InstIdVhsubps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vhsubps, kX86InstIdVhsubps, X86YmmVar, X86YmmVar, X86Mem)

  //! Insert 128-bit of packed FP data based on selector (AVX).
  INST_4i(vinsertf128, kX86InstIdVinsertf128, X86YmmVar, X86YmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vinsertf128, kX86InstIdVinsertf128, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Insert SP-FP based on selector (AVX).
  INST_4i(vinsertps, kX86InstIdVinsertps, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vinsertps, kX86InstIdVinsertps, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Load 128-bits unaligned (AVX).
  INST_2x(vlddqu, kX86InstIdVlddqu, X86XmmVar, X86Mem)
  //! Load 256-bits unaligned (AVX).
  INST_2x(vlddqu, kX86InstIdVlddqu, X86YmmVar, X86Mem)

  //! Load streaming SIMD extension control/status (AVX).
  INST_1x(vldmxcsr, kX86InstIdVldmxcsr, X86Mem)

  //! Store selected bytes of DQWORD to DS:EDI/RDI (AVX).
  INST_3x(vmaskmovdqu, kX86InstIdMaskmovdqu, X86GpVar /* ZDI */, X86XmmVar, X86XmmVar)

  //! Conditionally load packed DP-FP from `o2` using mask in `o1 and store in `o0` (AVX).
  INST_3x(vmaskmovpd, kX86InstIdVmaskmovpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vmaskmovpd, kX86InstIdVmaskmovpd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vmaskmovpd, kX86InstIdVmaskmovpd, X86Mem, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmaskmovpd, kX86InstIdVmaskmovpd, X86Mem, X86YmmVar, X86YmmVar)

  //! Conditionally load packed SP-FP from `o2` using mask in `o1 and store in `o0` (AVX).
  INST_3x(vmaskmovps, kX86InstIdVmaskmovps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vmaskmovps, kX86InstIdVmaskmovps, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vmaskmovps, kX86InstIdVmaskmovps, X86Mem, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmaskmovps, kX86InstIdVmaskmovps, X86Mem, X86YmmVar, X86YmmVar)

  //! Packed DP-FP maximum (AVX).
  INST_3x(vmaxpd, kX86InstIdVmaxpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmaxpd, kX86InstIdVmaxpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vmaxpd, kX86InstIdVmaxpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vmaxpd, kX86InstIdVmaxpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP maximum (AVX).
  INST_3x(vmaxps, kX86InstIdVmaxps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmaxps, kX86InstIdVmaxps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vmaxps, kX86InstIdVmaxps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vmaxps, kX86InstIdVmaxps, X86YmmVar, X86YmmVar, X86Mem)

  //! Scalar DP-FP maximum (AVX).
  INST_3x(vmaxsd, kX86InstIdVmaxsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmaxsd, kX86InstIdVmaxsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP maximum (AVX).
  INST_3x(vmaxss, kX86InstIdVmaxss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmaxss, kX86InstIdVmaxss, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DP-FP minimum (AVX).
  INST_3x(vminpd, kX86InstIdVminpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vminpd, kX86InstIdVminpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vminpd, kX86InstIdVminpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vminpd, kX86InstIdVminpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP minimum (AVX).
  INST_3x(vminps, kX86InstIdVminps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vminps, kX86InstIdVminps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vminps, kX86InstIdVminps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vminps, kX86InstIdVminps, X86YmmVar, X86YmmVar, X86Mem)

  //! Scalar DP-FP minimum (AVX).
  INST_3x(vminsd, kX86InstIdVminsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vminsd, kX86InstIdVminsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP minimum (AVX).
  INST_3x(vminss, kX86InstIdVminss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vminss, kX86InstIdVminss, X86XmmVar, X86XmmVar, X86Mem)

  //! Move 128-bits of aligned packed DP-FP (AVX).
  INST_2x(vmovapd, kX86InstIdVmovapd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovapd, kX86InstIdVmovapd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovapd, kX86InstIdVmovapd, X86Mem, X86XmmVar)
  //! Move 256-bits of aligned packed DP-FP (AVX).
  INST_2x(vmovapd, kX86InstIdVmovapd, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovapd, kX86InstIdVmovapd, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vmovapd, kX86InstIdVmovapd, X86Mem, X86YmmVar)

  //! Move 128-bits of aligned packed SP-FP (AVX).
  INST_2x(vmovaps, kX86InstIdVmovaps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovaps, kX86InstIdVmovaps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovaps, kX86InstIdVmovaps, X86Mem, X86XmmVar)
  //! Move 256-bits of aligned packed SP-FP (AVX).
  INST_2x(vmovaps, kX86InstIdVmovaps, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovaps, kX86InstIdVmovaps, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vmovaps, kX86InstIdVmovaps, X86Mem, X86YmmVar)

  //! Move DWORD (AVX).
  INST_2x(vmovd, kX86InstIdVmovd, X86XmmVar, X86GpVar)
  //! \overload
  INST_2x(vmovd, kX86InstIdVmovd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovd, kX86InstIdVmovd, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vmovd, kX86InstIdVmovd, X86Mem, X86XmmVar)

  //! Move QWORD (AVX).
  INST_2x(vmovq, kX86InstIdVmovq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovq, kX86InstIdVmovq, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovq, kX86InstIdVmovq, X86Mem, X86XmmVar)

  //! Move QWORD (AVX and X64 Only).
  INST_2x(vmovq, kX86InstIdVmovq, X86XmmVar, X86GpVar)
  //! \overload
  INST_2x(vmovq, kX86InstIdVmovq, X86GpVar, X86XmmVar)

  //! Move one DP-FP and duplicate (AVX).
  INST_2x(vmovddup, kX86InstIdVmovddup, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovddup, kX86InstIdVmovddup, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovddup, kX86InstIdVmovddup, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovddup, kX86InstIdVmovddup, X86YmmVar, X86Mem)

  //! Move 128-bits aligned (AVX).
  INST_2x(vmovdqa, kX86InstIdVmovdqa, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovdqa, kX86InstIdVmovdqa, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovdqa, kX86InstIdVmovdqa, X86Mem, X86XmmVar)
  //! Move 256-bits aligned (AVX).
  INST_2x(vmovdqa, kX86InstIdVmovdqa, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovdqa, kX86InstIdVmovdqa, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vmovdqa, kX86InstIdVmovdqa, X86Mem, X86YmmVar)

  //! Move 128-bits unaligned (AVX).
  INST_2x(vmovdqu, kX86InstIdVmovdqu, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovdqu, kX86InstIdVmovdqu, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovdqu, kX86InstIdVmovdqu, X86Mem, X86XmmVar)
  //! Move 256-bits unaligned (AVX).
  INST_2x(vmovdqu, kX86InstIdVmovdqu, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovdqu, kX86InstIdVmovdqu, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vmovdqu, kX86InstIdVmovdqu, X86Mem, X86YmmVar)

  //! High to low packed SP-FP (AVX).
  INST_3x(vmovhlps, kX86InstIdVmovhlps, X86XmmVar, X86XmmVar, X86XmmVar)

  //! Move high packed DP-FP (AVX).
  INST_3x(vmovhpd, kX86InstIdVmovhpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovhpd, kX86InstIdVmovhpd, X86Mem, X86XmmVar)

  //! Move high packed SP-FP (AVX).
  INST_3x(vmovhps, kX86InstIdVmovhps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovhps, kX86InstIdVmovhps, X86Mem, X86XmmVar)

  //! Move low to high packed SP-FP (AVX).
  INST_3x(vmovlhps, kX86InstIdVmovlhps, X86XmmVar, X86XmmVar, X86XmmVar)

  //! Move low packed DP-FP (AVX).
  INST_3x(vmovlpd, kX86InstIdVmovlpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovlpd, kX86InstIdVmovlpd, X86Mem, X86XmmVar)

  //! Move low packed SP-FP (AVX).
  INST_3x(vmovlps, kX86InstIdVmovlps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovlps, kX86InstIdVmovlps, X86Mem, X86XmmVar)

  //! Extract packed DP-FP sign mask (AVX).
  INST_2x(vmovmskpd, kX86InstIdVmovmskpd, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vmovmskpd, kX86InstIdVmovmskpd, X86GpVar, X86YmmVar)

  //! Extract packed SP-FP sign mask (AVX).
  INST_2x(vmovmskps, kX86InstIdVmovmskps, X86GpVar, X86XmmVar)
  //! \overload
  INST_2x(vmovmskps, kX86InstIdVmovmskps, X86GpVar, X86YmmVar)

  //! Store 128-bits using NT hint (AVX).
  INST_2x(vmovntdq, kX86InstIdVmovntdq, X86Mem, X86XmmVar)
  //! Store 256-bits using NT hint (AVX).
  INST_2x(vmovntdq, kX86InstIdVmovntdq, X86Mem, X86YmmVar)

  //! Store 128-bits aligned using NT hint (AVX).
  INST_2x(vmovntdqa, kX86InstIdVmovntdqa, X86XmmVar, X86Mem)

  //! Store packed DP-FP (128-bits) using NT hint (AVX).
  INST_2x(vmovntpd, kX86InstIdVmovntpd, X86Mem, X86XmmVar)
  //! Store packed DP-FP (256-bits) using NT hint (AVX).
  INST_2x(vmovntpd, kX86InstIdVmovntpd, X86Mem, X86YmmVar)

  //! Store packed SP-FP (128-bits) using NT hint (AVX).
  INST_2x(vmovntps, kX86InstIdVmovntps, X86Mem, X86XmmVar)
  //! Store packed SP-FP (256-bits) using NT hint (AVX).
  INST_2x(vmovntps, kX86InstIdVmovntps, X86Mem, X86YmmVar)

  //! Move scalar DP-FP (AVX).
  INST_3x(vmovsd, kX86InstIdVmovsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovsd, kX86InstIdVmovsd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovsd, kX86InstIdVmovsd, X86Mem, X86XmmVar)

  //! Move packed SP-FP high and duplicate (AVX).
  INST_2x(vmovshdup, kX86InstIdVmovshdup, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovshdup, kX86InstIdVmovshdup, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovshdup, kX86InstIdVmovshdup, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovshdup, kX86InstIdVmovshdup, X86YmmVar, X86Mem)

  //! Move packed SP-FP low and duplicate (AVX).
  INST_2x(vmovsldup, kX86InstIdVmovsldup, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovsldup, kX86InstIdVmovsldup, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovsldup, kX86InstIdVmovsldup, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovsldup, kX86InstIdVmovsldup, X86YmmVar, X86Mem)

  //! Move scalar SP-FP (AVX).
  INST_3x(vmovss, kX86InstIdVmovss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovss, kX86InstIdVmovss, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovss, kX86InstIdVmovss, X86Mem, X86XmmVar)

  //! Move 128-bits of unaligned packed DP-FP (AVX).
  INST_2x(vmovupd, kX86InstIdVmovupd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovupd, kX86InstIdVmovupd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovupd, kX86InstIdVmovupd, X86Mem, X86XmmVar)
  //! Move 256-bits of unaligned packed DP-FP (AVX).
  INST_2x(vmovupd, kX86InstIdVmovupd, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovupd, kX86InstIdVmovupd, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vmovupd, kX86InstIdVmovupd, X86Mem, X86YmmVar)

  //! Move 128-bits of unaligned packed SP-FP (AVX).
  INST_2x(vmovups, kX86InstIdVmovups, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vmovups, kX86InstIdVmovups, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vmovups, kX86InstIdVmovups, X86Mem, X86XmmVar)
  //! Move 256-bits of unaligned packed SP-FP (AVX).
  INST_2x(vmovups, kX86InstIdVmovups, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vmovups, kX86InstIdVmovups, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vmovups, kX86InstIdVmovups, X86Mem, X86YmmVar)

  //! Packed WORD sums of absolute difference (AVX).
  INST_4i(vmpsadbw, kX86InstIdVmpsadbw, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vmpsadbw, kX86InstIdVmpsadbw, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed DP-FP multiply (AVX).
  INST_3x(vmulpd, kX86InstIdVmulpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmulpd, kX86InstIdVmulpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vmulpd, kX86InstIdVmulpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vmulpd, kX86InstIdVmulpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP multiply (AVX).
  INST_3x(vmulps, kX86InstIdVmulps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmulps, kX86InstIdVmulps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vmulps, kX86InstIdVmulps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vmulps, kX86InstIdVmulps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP multiply (AVX).
  INST_3x(vmulsd, kX86InstIdVmulsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmulsd, kX86InstIdVmulsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP multiply (AVX).
  INST_3x(vmulss, kX86InstIdVmulss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vmulss, kX86InstIdVmulss, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DP-FP bitwise or (AVX).
  INST_3x(vorpd, kX86InstIdVorpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vorpd, kX86InstIdVorpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vorpd, kX86InstIdVorpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vorpd, kX86InstIdVorpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP bitwise or (AVX).
  INST_3x(vorps, kX86InstIdVorps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vorps, kX86InstIdVorps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vorps, kX86InstIdVorps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vorps, kX86InstIdVorps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTE absolute value (AVX).
  INST_2x(vpabsb, kX86InstIdVpabsb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpabsb, kX86InstIdVpabsb, X86XmmVar, X86Mem)

  //! Packed DWORD absolute value (AVX).
  INST_2x(vpabsd, kX86InstIdVpabsd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpabsd, kX86InstIdVpabsd, X86XmmVar, X86Mem)

  //! Packed WORD absolute value (AVX).
  INST_2x(vpabsw, kX86InstIdVpabsw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpabsw, kX86InstIdVpabsw, X86XmmVar, X86Mem)

  //! Pack DWORDs to WORDs with signed saturation (AVX).
  INST_3x(vpackssdw, kX86InstIdVpackssdw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpackssdw, kX86InstIdVpackssdw, X86XmmVar, X86XmmVar, X86Mem)

  //! Pack WORDs to BYTEs with signed saturation (AVX).
  INST_3x(vpacksswb, kX86InstIdVpacksswb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpacksswb, kX86InstIdVpacksswb, X86XmmVar, X86XmmVar, X86Mem)

  //! Pack DWORDs to WORDs with unsigned saturation (AVX).
  INST_3x(vpackusdw, kX86InstIdVpackusdw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpackusdw, kX86InstIdVpackusdw, X86XmmVar, X86XmmVar, X86Mem)

  //! Pack WORDs to BYTEs with unsigned saturation (AVX).
  INST_3x(vpackuswb, kX86InstIdVpackuswb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpackuswb, kX86InstIdVpackuswb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE add (AVX).
  INST_3x(vpaddb, kX86InstIdVpaddb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddb, kX86InstIdVpaddb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD add (AVX).
  INST_3x(vpaddd, kX86InstIdVpaddd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddd, kX86InstIdVpaddd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed QWORD add (AVX).
  INST_3x(vpaddq, kX86InstIdVpaddq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddq, kX86InstIdVpaddq, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD add (AVX).
  INST_3x(vpaddw, kX86InstIdVpaddw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddw, kX86InstIdVpaddw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE add with saturation (AVX).
  INST_3x(vpaddsb, kX86InstIdVpaddsb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddsb, kX86InstIdVpaddsb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD add with saturation (AVX).
  INST_3x(vpaddsw, kX86InstIdVpaddsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddsw, kX86InstIdVpaddsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE add with unsigned saturation (AVX).
  INST_3x(vpaddusb, kX86InstIdVpaddusb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddusb, kX86InstIdVpaddusb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD add with unsigned saturation (AVX).
  INST_3x(vpaddusw, kX86InstIdVpaddusw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpaddusw, kX86InstIdVpaddusw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed align right (AVX).
  INST_4i(vpalignr, kX86InstIdVpalignr, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vpalignr, kX86InstIdVpalignr, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed bitwise and (AVX).
  INST_3x(vpand, kX86InstIdVpand, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpand, kX86InstIdVpand, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed bitwise and-not (AVX).
  INST_3x(vpandn, kX86InstIdVpandn, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpandn, kX86InstIdVpandn, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE average (AVX).
  INST_3x(vpavgb, kX86InstIdVpavgb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpavgb, kX86InstIdVpavgb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD average (AVX).
  INST_3x(vpavgw, kX86InstIdVpavgw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpavgw, kX86InstIdVpavgw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE variable blend (AVX).
  INST_4x(vpblendvb, kX86InstIdVpblendvb, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_4x(vpblendvb, kX86InstIdVpblendvb, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)

  //! Packed WORD blend (AVX).
  INST_4i(vpblendw, kX86InstIdVpblendw, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vpblendw, kX86InstIdVpblendw, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed BYTEs compare for equality (AVX).
  INST_3x(vpcmpeqb, kX86InstIdVpcmpeqb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpeqb, kX86InstIdVpcmpeqb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORDs compare for equality (AVX).
  INST_3x(vpcmpeqd, kX86InstIdVpcmpeqd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpeqd, kX86InstIdVpcmpeqd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed QWORDs compare for equality (AVX).
  INST_3x(vpcmpeqq, kX86InstIdVpcmpeqq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpeqq, kX86InstIdVpcmpeqq, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORDs compare for equality (AVX).
  INST_3x(vpcmpeqw, kX86InstIdVpcmpeqw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpeqw, kX86InstIdVpcmpeqw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTEs compare if greater than (AVX).
  INST_3x(vpcmpgtb, kX86InstIdVpcmpgtb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpgtb, kX86InstIdVpcmpgtb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORDs compare if greater than (AVX).
  INST_3x(vpcmpgtd, kX86InstIdVpcmpgtd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpgtd, kX86InstIdVpcmpgtd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed QWORDs compare if greater than (AVX).
  INST_3x(vpcmpgtq, kX86InstIdVpcmpgtq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpgtq, kX86InstIdVpcmpgtq, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORDs compare if greater than (AVX).
  INST_3x(vpcmpgtw, kX86InstIdVpcmpgtw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpcmpgtw, kX86InstIdVpcmpgtw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed compare explicit length strings, return index in ECX (AVX).
  INST_3i(vpcmpestri, kX86InstIdVpcmpestri, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpcmpestri, kX86InstIdVpcmpestri, X86XmmVar, X86Mem, Imm)

  //! Packed compare explicit length strings, return mask in XMM0 (AVX).
  INST_3i(vpcmpestrm, kX86InstIdVpcmpestrm, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpcmpestrm, kX86InstIdVpcmpestrm, X86XmmVar, X86Mem, Imm)

  //! Packed compare implicit length strings, return index in ECX (AVX).
  INST_4i(vpcmpistri, kX86InstIdVpcmpistri, X86GpVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vpcmpistri, kX86InstIdVpcmpistri, X86GpVar, X86XmmVar, X86Mem, Imm)

  //! Packed compare implicit length strings, return mask in XMM0 (AVX).
  INST_4i(vpcmpistrm, kX86InstIdVpcmpistrm, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vpcmpistrm, kX86InstIdVpcmpistrm, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed DP-FP permute (AVX).
  INST_3x(vpermilpd, kX86InstIdVpermilpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpermilpd, kX86InstIdVpermilpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vpermilpd, kX86InstIdVpermilpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpermilpd, kX86InstIdVpermilpd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3i(vpermilpd, kX86InstIdVpermilpd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpermilpd, kX86InstIdVpermilpd, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vpermilpd, kX86InstIdVpermilpd, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vpermilpd, kX86InstIdVpermilpd, X86YmmVar, X86Mem, Imm)

  //! Packed SP-FP permute (AVX).
  INST_3x(vpermilps, kX86InstIdVpermilps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpermilps, kX86InstIdVpermilps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vpermilps, kX86InstIdVpermilps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpermilps, kX86InstIdVpermilps, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3i(vpermilps, kX86InstIdVpermilps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpermilps, kX86InstIdVpermilps, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vpermilps, kX86InstIdVpermilps, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vpermilps, kX86InstIdVpermilps, X86YmmVar, X86Mem, Imm)

  //! Packed 128-bit FP permute (AVX).
  INST_4i(vperm2f128, kX86InstIdVperm2f128, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vperm2f128, kX86InstIdVperm2f128, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Extract BYTE (AVX).
  INST_3i(vpextrb, kX86InstIdVpextrb, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpextrb, kX86InstIdVpextrb, X86Mem, X86XmmVar, Imm)

  //! Extract DWORD (AVX).
  INST_3i(vpextrd, kX86InstIdVpextrd, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpextrd, kX86InstIdVpextrd, X86Mem, X86XmmVar, Imm)

  //! Extract QWORD (AVX and X64 Only).
  INST_3i(vpextrq, kX86InstIdVpextrq, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpextrq, kX86InstIdVpextrq, X86Mem, X86XmmVar, Imm)

  //! Extract WORD (AVX).
  INST_3i(vpextrw, kX86InstIdVpextrw, X86GpVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpextrw, kX86InstIdVpextrw, X86Mem, X86XmmVar, Imm)

  //! Packed DWORD horizontal add (AVX).
  INST_3x(vphaddd, kX86InstIdVphaddd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vphaddd, kX86InstIdVphaddd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD horizontal add with saturation (AVX).
  INST_3x(vphaddsw, kX86InstIdVphaddsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vphaddsw, kX86InstIdVphaddsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD horizontal add (AVX).
  INST_3x(vphaddw, kX86InstIdVphaddw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vphaddw, kX86InstIdVphaddw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD horizontal minimum (AVX).
  INST_2x(vphminposuw, kX86InstIdVphminposuw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vphminposuw, kX86InstIdVphminposuw, X86XmmVar, X86Mem)

  //! Packed DWORD horizontal subtract (AVX).
  INST_3x(vphsubd, kX86InstIdVphsubd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vphsubd, kX86InstIdVphsubd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD horizontal subtract with saturation (AVX).
  INST_3x(vphsubsw, kX86InstIdVphsubsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vphsubsw, kX86InstIdVphsubsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD horizontal subtract (AVX).
  INST_3x(vphsubw, kX86InstIdVphsubw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vphsubw, kX86InstIdVphsubw, X86XmmVar, X86XmmVar, X86Mem)

  //! Insert BYTE based on selector (AVX).
  INST_4i(vpinsrb, kX86InstIdVpinsrb, X86XmmVar, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_4i(vpinsrb, kX86InstIdVpinsrb, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Insert DWORD based on selector (AVX).
  INST_4i(vpinsrd, kX86InstIdVpinsrd, X86XmmVar, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_4i(vpinsrd, kX86InstIdVpinsrd, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Insert QWORD based on selector (AVX and X64 Only).
  INST_4i(vpinsrq, kX86InstIdVpinsrq, X86XmmVar, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_4i(vpinsrq, kX86InstIdVpinsrq, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Insert WORD based on selector (AVX).
  INST_4i(vpinsrw, kX86InstIdVpinsrw, X86XmmVar, X86XmmVar, X86GpVar, Imm)
  //! \overload
  INST_4i(vpinsrw, kX86InstIdVpinsrw, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Packed multiply and add signed and unsigned bytes (AVX).
  INST_3x(vpmaddubsw, kX86InstIdVpmaddubsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaddubsw, kX86InstIdVpmaddubsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD multiply and add to packed DWORD (AVX).
  INST_3x(vpmaddwd, kX86InstIdVpmaddwd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaddwd, kX86InstIdVpmaddwd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE maximum (AVX).
  INST_3x(vpmaxsb, kX86InstIdVpmaxsb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaxsb, kX86InstIdVpmaxsb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD maximum (AVX).
  INST_3x(vpmaxsd, kX86InstIdVpmaxsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaxsd, kX86InstIdVpmaxsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD maximum (AVX).
  INST_3x(vpmaxsw, kX86InstIdVpmaxsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaxsw, kX86InstIdVpmaxsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE unsigned maximum (AVX).
  INST_3x(vpmaxub, kX86InstIdVpmaxub, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaxub, kX86InstIdVpmaxub, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD unsigned maximum (AVX).
  INST_3x(vpmaxud, kX86InstIdVpmaxud, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaxud, kX86InstIdVpmaxud, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD unsigned maximum (AVX).
  INST_3x(vpmaxuw, kX86InstIdVpmaxuw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmaxuw, kX86InstIdVpmaxuw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE minimum (AVX).
  INST_3x(vpminsb, kX86InstIdVpminsb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpminsb, kX86InstIdVpminsb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD minimum (AVX).
  INST_3x(vpminsd, kX86InstIdVpminsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpminsd, kX86InstIdVpminsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD minimum (AVX).
  INST_3x(vpminsw, kX86InstIdVpminsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpminsw, kX86InstIdVpminsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE unsigned minimum (AVX).
  INST_3x(vpminub, kX86InstIdVpminub, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpminub, kX86InstIdVpminub, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD unsigned minimum (AVX).
  INST_3x(vpminud, kX86InstIdVpminud, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpminud, kX86InstIdVpminud, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD unsigned minimum (AVX).
  INST_3x(vpminuw, kX86InstIdVpminuw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpminuw, kX86InstIdVpminuw, X86XmmVar, X86XmmVar, X86Mem)

  //! Move Byte mask to integer (AVX).
  INST_2x(vpmovmskb, kX86InstIdVpmovmskb, X86GpVar, X86XmmVar)

  //! BYTE to DWORD with sign extend (AVX).
  INST_2x(vpmovsxbd, kX86InstIdVpmovsxbd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovsxbd, kX86InstIdVpmovsxbd, X86XmmVar, X86Mem)

  //! Packed BYTE to QWORD with sign extend (AVX).
  INST_2x(vpmovsxbq, kX86InstIdVpmovsxbq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovsxbq, kX86InstIdVpmovsxbq, X86XmmVar, X86Mem)

  //! Packed BYTE to WORD with sign extend (AVX).
  INST_2x(vpmovsxbw, kX86InstIdVpmovsxbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovsxbw, kX86InstIdVpmovsxbw, X86XmmVar, X86Mem)

  //! Packed DWORD to QWORD with sign extend (AVX).
  INST_2x(vpmovsxdq, kX86InstIdVpmovsxdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovsxdq, kX86InstIdVpmovsxdq, X86XmmVar, X86Mem)

  //! Packed WORD to DWORD with sign extend (AVX).
  INST_2x(vpmovsxwd, kX86InstIdVpmovsxwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovsxwd, kX86InstIdVpmovsxwd, X86XmmVar, X86Mem)

  //! Packed WORD to QWORD with sign extend (AVX).
  INST_2x(vpmovsxwq, kX86InstIdVpmovsxwq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovsxwq, kX86InstIdVpmovsxwq, X86XmmVar, X86Mem)

  //! BYTE to DWORD with zero extend (AVX).
  INST_2x(vpmovzxbd, kX86InstIdVpmovzxbd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovzxbd, kX86InstIdVpmovzxbd, X86XmmVar, X86Mem)

  //! Packed BYTE to QWORD with zero extend (AVX).
  INST_2x(vpmovzxbq, kX86InstIdVpmovzxbq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovzxbq, kX86InstIdVpmovzxbq, X86XmmVar, X86Mem)

  //! BYTE to WORD with zero extend (AVX).
  INST_2x(vpmovzxbw, kX86InstIdVpmovzxbw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovzxbw, kX86InstIdVpmovzxbw, X86XmmVar, X86Mem)

  //! Packed DWORD to QWORD with zero extend (AVX).
  INST_2x(vpmovzxdq, kX86InstIdVpmovzxdq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovzxdq, kX86InstIdVpmovzxdq, X86XmmVar, X86Mem)

  //! Packed WORD to DWORD with zero extend (AVX).
  INST_2x(vpmovzxwd, kX86InstIdVpmovzxwd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovzxwd, kX86InstIdVpmovzxwd, X86XmmVar, X86Mem)

  //! Packed WORD to QWORD with zero extend (AVX).
  INST_2x(vpmovzxwq, kX86InstIdVpmovzxwq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpmovzxwq, kX86InstIdVpmovzxwq, X86XmmVar, X86Mem)

  //! Packed DWORD to QWORD multiply (AVX).
  INST_3x(vpmuldq, kX86InstIdVpmuldq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmuldq, kX86InstIdVpmuldq, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD multiply high, round and scale (AVX).
  INST_3x(vpmulhrsw, kX86InstIdVpmulhrsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmulhrsw, kX86InstIdVpmulhrsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD unsigned multiply high (AVX).
  INST_3x(vpmulhuw, kX86InstIdVpmulhuw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmulhuw, kX86InstIdVpmulhuw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD multiply high (AVX).
  INST_3x(vpmulhw, kX86InstIdVpmulhw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmulhw, kX86InstIdVpmulhw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD multiply low (AVX).
  INST_3x(vpmulld, kX86InstIdVpmulld, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmulld, kX86InstIdVpmulld, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORDs multiply low (AVX).
  INST_3x(vpmullw, kX86InstIdVpmullw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmullw, kX86InstIdVpmullw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD multiply to QWORD (AVX).
  INST_3x(vpmuludq, kX86InstIdVpmuludq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpmuludq, kX86InstIdVpmuludq, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed bitwise or (AVX).
  INST_3x(vpor, kX86InstIdVpor, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpor, kX86InstIdVpor, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD sum of absolute differences (AVX).
  INST_3x(vpsadbw, kX86InstIdVpsadbw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsadbw, kX86InstIdVpsadbw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE shuffle (AVX).
  INST_3x(vpshufb, kX86InstIdVpshufb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpshufb, kX86InstIdVpshufb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD shuffle (AVX).
  INST_3i(vpshufd, kX86InstIdVpshufd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpshufd, kX86InstIdVpshufd, X86XmmVar, X86Mem, Imm)

  //! Packed WORD shuffle high (AVX).
  INST_3i(vpshufhw, kX86InstIdVpshufhw, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpshufhw, kX86InstIdVpshufhw, X86XmmVar, X86Mem, Imm)

  //! Packed WORD shuffle low (AVX).
  INST_3i(vpshuflw, kX86InstIdVpshuflw, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vpshuflw, kX86InstIdVpshuflw, X86XmmVar, X86Mem, Imm)

  //! Packed BYTE sign (AVX).
  INST_3x(vpsignb, kX86InstIdVpsignb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsignb, kX86InstIdVpsignb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD sign (AVX).
  INST_3x(vpsignd, kX86InstIdVpsignd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsignd, kX86InstIdVpsignd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD sign (AVX).
  INST_3x(vpsignw, kX86InstIdVpsignw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsignw, kX86InstIdVpsignw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD shift left logical (AVX).
  INST_3x(vpslld, kX86InstIdVpslld, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpslld, kX86InstIdVpslld, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpslld, kX86InstIdVpslld, X86XmmVar, X86XmmVar, Imm)

  //! Packed DQWORD shift left logical (AVX).
  INST_3i(vpslldq, kX86InstIdVpslldq, X86XmmVar, X86XmmVar, Imm)

  //! Packed QWORD shift left logical (AVX).
  INST_3x(vpsllq, kX86InstIdVpsllq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsllq, kX86InstIdVpsllq, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsllq, kX86InstIdVpsllq, X86XmmVar, X86XmmVar, Imm)

  //! Packed WORD shift left logical (AVX).
  INST_3x(vpsllw, kX86InstIdVpsllw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsllw, kX86InstIdVpsllw, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsllw, kX86InstIdVpsllw, X86XmmVar, X86XmmVar, Imm)

  //! Packed DWORD shift right arithmetic (AVX).
  INST_3x(vpsrad, kX86InstIdVpsrad, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsrad, kX86InstIdVpsrad, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsrad, kX86InstIdVpsrad, X86XmmVar, X86XmmVar, Imm)

  //! Packed WORD shift right arithmetic (AVX).
  INST_3x(vpsraw, kX86InstIdVpsraw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsraw, kX86InstIdVpsraw, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsraw, kX86InstIdVpsraw, X86XmmVar, X86XmmVar, Imm)

  //! Packed DWORD shift right logical (AVX).
  INST_3x(vpsrld, kX86InstIdVpsrld, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsrld, kX86InstIdVpsrld, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsrld, kX86InstIdVpsrld, X86XmmVar, X86XmmVar, Imm)

  //! Scalar DQWORD shift right logical (AVX).
  INST_3i(vpsrldq, kX86InstIdVpsrldq, X86XmmVar, X86XmmVar, Imm)

  //! Packed QWORD shift right logical (AVX).
  INST_3x(vpsrlq, kX86InstIdVpsrlq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsrlq, kX86InstIdVpsrlq, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsrlq, kX86InstIdVpsrlq, X86XmmVar, X86XmmVar, Imm)

  //! Packed WORD shift right logical (AVX).
  INST_3x(vpsrlw, kX86InstIdVpsrlw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsrlw, kX86InstIdVpsrlw, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3i(vpsrlw, kX86InstIdVpsrlw, X86XmmVar, X86XmmVar, Imm)

  //! Packed BYTE subtract (AVX).
  INST_3x(vpsubb, kX86InstIdVpsubb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubb, kX86InstIdVpsubb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DWORD subtract (AVX).
  INST_3x(vpsubd, kX86InstIdVpsubd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubd, kX86InstIdVpsubd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed QWORD subtract (AVX).
  INST_3x(vpsubq, kX86InstIdVpsubq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubq, kX86InstIdVpsubq, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD subtract (AVX).
  INST_3x(vpsubw, kX86InstIdVpsubw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubw, kX86InstIdVpsubw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE subtract with saturation (AVX).
  INST_3x(vpsubsb, kX86InstIdVpsubsb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubsb, kX86InstIdVpsubsb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD subtract with saturation (AVX).
  INST_3x(vpsubsw, kX86InstIdVpsubsw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubsw, kX86InstIdVpsubsw, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed BYTE subtract with unsigned saturation (AVX).
  INST_3x(vpsubusb, kX86InstIdVpsubusb, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubusb, kX86InstIdVpsubusb, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed WORD subtract with unsigned saturation (AVX).
  INST_3x(vpsubusw, kX86InstIdVpsubusw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpsubusw, kX86InstIdVpsubusw, X86XmmVar, X86XmmVar, X86Mem)

  //! Logical compare (AVX).
  INST_2x(vptest, kX86InstIdVptest, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vptest, kX86InstIdVptest, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vptest, kX86InstIdVptest, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vptest, kX86InstIdVptest, X86YmmVar, X86Mem)

  //! Unpack high packed BYTEs to WORDs (AVX).
  INST_3x(vpunpckhbw, kX86InstIdVpunpckhbw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpckhbw, kX86InstIdVpunpckhbw, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack high packed DWORDs to QWORDs (AVX).
  INST_3x(vpunpckhdq, kX86InstIdVpunpckhdq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpckhdq, kX86InstIdVpunpckhdq, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack high packed QWORDs to DQWORD (AVX).
  INST_3x(vpunpckhqdq, kX86InstIdVpunpckhqdq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpckhqdq, kX86InstIdVpunpckhqdq, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack high packed WORDs to DWORDs (AVX).
  INST_3x(vpunpckhwd, kX86InstIdVpunpckhwd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpckhwd, kX86InstIdVpunpckhwd, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack low packed BYTEs to WORDs (AVX).
  INST_3x(vpunpcklbw, kX86InstIdVpunpcklbw, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpcklbw, kX86InstIdVpunpcklbw, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack low packed DWORDs to QWORDs (AVX).
  INST_3x(vpunpckldq, kX86InstIdVpunpckldq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpckldq, kX86InstIdVpunpckldq, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack low packed QWORDs to DQWORD (AVX).
  INST_3x(vpunpcklqdq, kX86InstIdVpunpcklqdq, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpcklqdq, kX86InstIdVpunpcklqdq, X86XmmVar, X86XmmVar, X86Mem)

  //! Unpack low packed WORDs to DWORDs (AVX).
  INST_3x(vpunpcklwd, kX86InstIdVpunpcklwd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpunpcklwd, kX86InstIdVpunpcklwd, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed bitwise xor (AVX).
  INST_3x(vpxor, kX86InstIdVpxor, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vpxor, kX86InstIdVpxor, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed SP-FP reciprocal (AVX).
  INST_2x(vrcpps, kX86InstIdVrcpps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vrcpps, kX86InstIdVrcpps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vrcpps, kX86InstIdVrcpps, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vrcpps, kX86InstIdVrcpps, X86YmmVar, X86Mem)

  //! Scalar SP-FP reciprocal (AVX).
  INST_3x(vrcpss, kX86InstIdVrcpss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vrcpss, kX86InstIdVrcpss, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed SP-FP square root reciprocal (AVX).
  INST_2x(vrsqrtps, kX86InstIdVrsqrtps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vrsqrtps, kX86InstIdVrsqrtps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vrsqrtps, kX86InstIdVrsqrtps, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vrsqrtps, kX86InstIdVrsqrtps, X86YmmVar, X86Mem)

  //! Scalar SP-FP square root reciprocal (AVX).
  INST_3x(vrsqrtss, kX86InstIdVrsqrtss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vrsqrtss, kX86InstIdVrsqrtss, X86XmmVar, X86XmmVar, X86Mem)

  //! Packed DP-FP round (AVX).
  INST_3i(vroundpd, kX86InstIdVroundpd, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vroundpd, kX86InstIdVroundpd, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vroundpd, kX86InstIdVroundpd, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vroundpd, kX86InstIdVroundpd, X86YmmVar, X86Mem, Imm)

  //! Packed SP-FP round (AVX).
  INST_3i(vroundps, kX86InstIdVroundps, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vroundps, kX86InstIdVroundps, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vroundps, kX86InstIdVroundps, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vroundps, kX86InstIdVroundps, X86YmmVar, X86Mem, Imm)

  //! Scalar DP-FP round (AVX).
  INST_4i(vroundsd, kX86InstIdVroundsd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vroundsd, kX86InstIdVroundsd, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Scalar SP-FP round (AVX).
  INST_4i(vroundss, kX86InstIdVroundss, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vroundss, kX86InstIdVroundss, X86XmmVar, X86XmmVar, X86Mem, Imm)

  //! Shuffle DP-FP (AVX).
  INST_4i(vshufpd, kX86InstIdVshufpd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vshufpd, kX86InstIdVshufpd, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vshufpd, kX86InstIdVshufpd, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vshufpd, kX86InstIdVshufpd, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Shuffle SP-FP (AVX).
  INST_4i(vshufps, kX86InstIdVshufps, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vshufps, kX86InstIdVshufps, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vshufps, kX86InstIdVshufps, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vshufps, kX86InstIdVshufps, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed DP-FP square root (AVX).
  INST_2x(vsqrtpd, kX86InstIdVsqrtpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vsqrtpd, kX86InstIdVsqrtpd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vsqrtpd, kX86InstIdVsqrtpd, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vsqrtpd, kX86InstIdVsqrtpd, X86YmmVar, X86Mem)

  //! Packed SP-FP square root (AVX).
  INST_2x(vsqrtps, kX86InstIdVsqrtps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vsqrtps, kX86InstIdVsqrtps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vsqrtps, kX86InstIdVsqrtps, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vsqrtps, kX86InstIdVsqrtps, X86YmmVar, X86Mem)

  //! Scalar DP-FP square root (AVX).
  INST_3x(vsqrtsd, kX86InstIdVsqrtsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vsqrtsd, kX86InstIdVsqrtsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP square root (AVX).
  INST_3x(vsqrtss, kX86InstIdVsqrtss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vsqrtss, kX86InstIdVsqrtss, X86XmmVar, X86XmmVar, X86Mem)

  //! Store streaming SIMD extension control/status (AVX).
  INST_1x(vstmxcsr, kX86InstIdVstmxcsr, X86Mem)

  //! Packed DP-FP subtract (AVX).
  INST_3x(vsubpd, kX86InstIdVsubpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vsubpd, kX86InstIdVsubpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vsubpd, kX86InstIdVsubpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vsubpd, kX86InstIdVsubpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP subtract (AVX).
  INST_3x(vsubps, kX86InstIdVsubps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vsubps, kX86InstIdVsubps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vsubps, kX86InstIdVsubps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vsubps, kX86InstIdVsubps, X86YmmVar, X86YmmVar, X86Mem)

  //! Scalar DP-FP subtract (AVX).
  INST_3x(vsubsd, kX86InstIdVsubsd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vsubsd, kX86InstIdVsubsd, X86XmmVar, X86XmmVar, X86Mem)

  //! Scalar SP-FP subtract (AVX).
  INST_3x(vsubss, kX86InstIdVsubss, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vsubss, kX86InstIdVsubss, X86XmmVar, X86XmmVar, X86Mem)

  //! Logical compare DP-FP (AVX).
  INST_2x(vtestpd, kX86InstIdVtestpd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vtestpd, kX86InstIdVtestpd, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vtestpd, kX86InstIdVtestpd, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vtestpd, kX86InstIdVtestpd, X86YmmVar, X86Mem)

  //! Logical compare SP-FP (AVX).
  INST_2x(vtestps, kX86InstIdVtestps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vtestps, kX86InstIdVtestps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vtestps, kX86InstIdVtestps, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vtestps, kX86InstIdVtestps, X86YmmVar, X86Mem)

  //! Scalar DP-FP unordered compare and set EFLAGS (AVX).
  INST_2x(vucomisd, kX86InstIdVucomisd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vucomisd, kX86InstIdVucomisd, X86XmmVar, X86Mem)

  //! Unordered scalar SP-FP compare and set EFLAGS (AVX).
  INST_2x(vucomiss, kX86InstIdVucomiss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vucomiss, kX86InstIdVucomiss, X86XmmVar, X86Mem)

  //! Unpack and interleave high packed DP-FP (AVX).
  INST_3x(vunpckhpd, kX86InstIdVunpckhpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vunpckhpd, kX86InstIdVunpckhpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vunpckhpd, kX86InstIdVunpckhpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vunpckhpd, kX86InstIdVunpckhpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Unpack high packed SP-FP data (AVX).
  INST_3x(vunpckhps, kX86InstIdVunpckhps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vunpckhps, kX86InstIdVunpckhps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vunpckhps, kX86InstIdVunpckhps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vunpckhps, kX86InstIdVunpckhps, X86YmmVar, X86YmmVar, X86Mem)

  //! Unpack and interleave low packed DP-FP (AVX).
  INST_3x(vunpcklpd, kX86InstIdVunpcklpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vunpcklpd, kX86InstIdVunpcklpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vunpcklpd, kX86InstIdVunpcklpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vunpcklpd, kX86InstIdVunpcklpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Unpack low packed SP-FP data (AVX).
  INST_3x(vunpcklps, kX86InstIdVunpcklps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vunpcklps, kX86InstIdVunpcklps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vunpcklps, kX86InstIdVunpcklps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vunpcklps, kX86InstIdVunpcklps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DP-FP bitwise xor (AVX).
  INST_3x(vxorpd, kX86InstIdVxorpd, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vxorpd, kX86InstIdVxorpd, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vxorpd, kX86InstIdVxorpd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vxorpd, kX86InstIdVxorpd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed SP-FP bitwise xor (AVX).
  INST_3x(vxorps, kX86InstIdVxorps, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vxorps, kX86InstIdVxorps, X86XmmVar, X86XmmVar, X86Mem)
  //! \overload
  INST_3x(vxorps, kX86InstIdVxorps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vxorps, kX86InstIdVxorps, X86YmmVar, X86YmmVar, X86Mem)

  //! Zero all YMM registers.
  INST_0x(vzeroall, kX86InstIdVzeroall)
  //! Zero upper 128-bits of all YMM registers.
  INST_0x(vzeroupper, kX86InstIdVzeroupper)

  // --------------------------------------------------------------------------
  // [AVX+AESNI]
  // --------------------------------------------------------------------------

  //! Perform a single round of the AES decryption flow (AVX+AESNI).
  INST_3x(vaesdec, kX86InstIdVaesdec, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaesdec, kX86InstIdVaesdec, X86XmmVar, X86XmmVar, X86Mem)

  //! Perform the last round of the AES decryption flow (AVX+AESNI).
  INST_3x(vaesdeclast, kX86InstIdVaesdeclast, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaesdeclast, kX86InstIdVaesdeclast, X86XmmVar, X86XmmVar, X86Mem)

  //! Perform a single round of the AES encryption flow (AVX+AESNI).
  INST_3x(vaesenc, kX86InstIdVaesenc, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaesenc, kX86InstIdVaesenc, X86XmmVar, X86XmmVar, X86Mem)

  //! Perform the last round of the AES encryption flow (AVX+AESNI).
  INST_3x(vaesenclast, kX86InstIdVaesenclast, X86XmmVar, X86XmmVar, X86XmmVar)
  //! \overload
  INST_3x(vaesenclast, kX86InstIdVaesenclast, X86XmmVar, X86XmmVar, X86Mem)

  //! Perform the InvMixColumns transformation (AVX+AESNI).
  INST_2x(vaesimc, kX86InstIdVaesimc, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vaesimc, kX86InstIdVaesimc, X86XmmVar, X86Mem)

  //! Assist in expanding the AES cipher key (AVX+AESNI).
  INST_3i(vaeskeygenassist, kX86InstIdVaeskeygenassist, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vaeskeygenassist, kX86InstIdVaeskeygenassist, X86XmmVar, X86Mem, Imm)

  // --------------------------------------------------------------------------
  // [AVX+PCLMULQDQ]
  // --------------------------------------------------------------------------

  //! Carry-less multiplication QWORD (AVX+PCLMULQDQ).
  INST_4i(vpclmulqdq, kX86InstIdVpclmulqdq, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vpclmulqdq, kX86InstIdVpclmulqdq, X86XmmVar, X86XmmVar, X86Mem, Imm)

  // --------------------------------------------------------------------------
  // [AVX2]
  // --------------------------------------------------------------------------

  //! Broadcast low 128-bit element in `o1` to `o0` (AVX2).
  INST_2x(vbroadcasti128, kX86InstIdVbroadcasti128, X86YmmVar, X86Mem)
  //! Broadcast low DP-FP element in `o1` to `o0` (AVX2).
  INST_2x(vbroadcastsd, kX86InstIdVbroadcastsd, X86YmmVar, X86XmmVar)
  //! Broadcast low SP-FP element in `o1` to `o0` (AVX2).
  INST_2x(vbroadcastss, kX86InstIdVbroadcastss, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vbroadcastss, kX86InstIdVbroadcastss, X86YmmVar, X86XmmVar)

  //! Extract 128-bit element from `o1` to `o0` based on selector (AVX2).
  INST_3i(vextracti128, kX86InstIdVextracti128, X86XmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vextracti128, kX86InstIdVextracti128, X86Mem, X86YmmVar, Imm)

  //! Gather DP-FP from DWORD indexes specified in `o1`s VSIB (AVX2).
  INST_3x(vgatherdpd, kX86InstIdVgatherdpd, X86XmmVar, X86Mem, X86XmmVar)
  //! \overload
  INST_3x(vgatherdpd, kX86InstIdVgatherdpd, X86YmmVar, X86Mem, X86YmmVar)

  //! Gather SP-FP from DWORD indexes specified in `o1`s VSIB (AVX2).
  INST_3x(vgatherdps, kX86InstIdVgatherdps, X86XmmVar, X86Mem, X86XmmVar)
  //! \overload
  INST_3x(vgatherdps, kX86InstIdVgatherdps, X86YmmVar, X86Mem, X86YmmVar)

  //! Gather DP-FP from QWORD indexes specified in `o1`s VSIB (AVX2).
  INST_3x(vgatherqpd, kX86InstIdVgatherqpd, X86XmmVar, X86Mem, X86XmmVar)
  //! \overload
  INST_3x(vgatherqpd, kX86InstIdVgatherqpd, X86YmmVar, X86Mem, X86YmmVar)

  //! Gather SP-FP from QWORD indexes specified in `o1`s VSIB (AVX2).
  INST_3x(vgatherqps, kX86InstIdVgatherqps, X86XmmVar, X86Mem, X86XmmVar)

  //! Insert 128-bit of packed data based on selector (AVX2).
  INST_4i(vinserti128, kX86InstIdVinserti128, X86YmmVar, X86YmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vinserti128, kX86InstIdVinserti128, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Load 256-bits aligned using NT hint (AVX2).
  INST_2x(vmovntdqa, kX86InstIdVmovntdqa, X86YmmVar, X86Mem)

  //! Packed WORD sums of absolute difference (AVX2).
  INST_4i(vmpsadbw, kX86InstIdVmpsadbw, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vmpsadbw, kX86InstIdVmpsadbw, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed BYTE absolute value (AVX2).
  INST_2x(vpabsb, kX86InstIdVpabsb, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vpabsb, kX86InstIdVpabsb, X86YmmVar, X86Mem)

  //! Packed DWORD absolute value (AVX2).
  INST_2x(vpabsd, kX86InstIdVpabsd, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vpabsd, kX86InstIdVpabsd, X86YmmVar, X86Mem)

  //! Packed WORD absolute value (AVX2).
  INST_2x(vpabsw, kX86InstIdVpabsw, X86YmmVar, X86YmmVar)
  //! \overload
  INST_2x(vpabsw, kX86InstIdVpabsw, X86YmmVar, X86Mem)

  //! Pack DWORDs to WORDs with signed saturation (AVX2).
  INST_3x(vpackssdw, kX86InstIdVpackssdw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpackssdw, kX86InstIdVpackssdw, X86YmmVar, X86YmmVar, X86Mem)

  //! Pack WORDs to BYTEs with signed saturation (AVX2).
  INST_3x(vpacksswb, kX86InstIdVpacksswb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpacksswb, kX86InstIdVpacksswb, X86YmmVar, X86YmmVar, X86Mem)

  //! Pack DWORDs to WORDs with unsigned saturation (AVX2).
  INST_3x(vpackusdw, kX86InstIdVpackusdw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpackusdw, kX86InstIdVpackusdw, X86YmmVar, X86YmmVar, X86Mem)

  //! Pack WORDs to BYTEs with unsigned saturation (AVX2).
  INST_3x(vpackuswb, kX86InstIdVpackuswb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpackuswb, kX86InstIdVpackuswb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTE add (AVX2).
  INST_3x(vpaddb, kX86InstIdVpaddb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddb, kX86InstIdVpaddb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DWORD add (AVX2).
  INST_3x(vpaddd, kX86InstIdVpaddd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddd, kX86InstIdVpaddd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed QDWORD add (AVX2).
  INST_3x(vpaddq, kX86InstIdVpaddq, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddq, kX86InstIdVpaddq, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed WORD add (AVX2).
  INST_3x(vpaddw, kX86InstIdVpaddw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddw, kX86InstIdVpaddw, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTE add with saturation (AVX2).
  INST_3x(vpaddsb, kX86InstIdVpaddsb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddsb, kX86InstIdVpaddsb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed WORD add with saturation (AVX2).
  INST_3x(vpaddsw, kX86InstIdVpaddsw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddsw, kX86InstIdVpaddsw, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTE add with unsigned saturation (AVX2).
  INST_3x(vpaddusb, kX86InstIdVpaddusb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddusb, kX86InstIdVpaddusb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed WORD add with unsigned saturation (AVX2).
  INST_3x(vpaddusw, kX86InstIdVpaddusw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpaddusw, kX86InstIdVpaddusw, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed align right (AVX2).
  INST_4i(vpalignr, kX86InstIdVpalignr, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vpalignr, kX86InstIdVpalignr, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed bitwise and (AVX2).
  INST_3x(vpand, kX86InstIdVpand, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpand, kX86InstIdVpand, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed bitwise and-not (AVX2).
  INST_3x(vpandn, kX86InstIdVpandn, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpandn, kX86InstIdVpandn, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTE average (AVX2).
  INST_3x(vpavgb, kX86InstIdVpavgb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpavgb, kX86InstIdVpavgb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed WORD average (AVX2).
  INST_3x(vpavgw, kX86InstIdVpavgw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpavgw, kX86InstIdVpavgw, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DWORD blend (AVX2).
  INST_4i(vpblendd, kX86InstIdVpblendd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_4i(vpblendd, kX86InstIdVpblendd, X86XmmVar, X86XmmVar, X86Mem, Imm)
  //! \overload
  INST_4i(vpblendd, kX86InstIdVpblendd, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vpblendd, kX86InstIdVpblendd, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed DWORD variable blend (AVX2).
  INST_4x(vpblendvb, kX86InstIdVpblendvb, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_4x(vpblendvb, kX86InstIdVpblendvb, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)

  //! Packed WORD blend (AVX2).
  INST_4i(vpblendw, kX86InstIdVpblendw, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vpblendw, kX86InstIdVpblendw, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Broadcast BYTE from `o1` to 128-bits in `o0` (AVX2).
  INST_2x(vpbroadcastb, kX86InstIdVpbroadcastb, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastb, kX86InstIdVpbroadcastb, X86XmmVar, X86Mem)
  //! Broadcast BYTE from `o1` to 256-bits in `o0` (AVX2).
  INST_2x(vpbroadcastb, kX86InstIdVpbroadcastb, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastb, kX86InstIdVpbroadcastb, X86YmmVar, X86Mem)

  //! Broadcast DWORD from `o1` to 128-bits in `o0` (AVX2).
  INST_2x(vpbroadcastd, kX86InstIdVpbroadcastd, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastd, kX86InstIdVpbroadcastd, X86XmmVar, X86Mem)
  //! Broadcast DWORD from `o1` to 256-bits in `o0` (AVX2).
  INST_2x(vpbroadcastd, kX86InstIdVpbroadcastd, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastd, kX86InstIdVpbroadcastd, X86YmmVar, X86Mem)

  //! Broadcast QWORD from `o1` to 128-bits in `o0` (AVX2).
  INST_2x(vpbroadcastq, kX86InstIdVpbroadcastq, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastq, kX86InstIdVpbroadcastq, X86XmmVar, X86Mem)
  //! Broadcast QWORD from `o1` to 256-bits in `o0` (AVX2).
  INST_2x(vpbroadcastq, kX86InstIdVpbroadcastq, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastq, kX86InstIdVpbroadcastq, X86YmmVar, X86Mem)

  //! Broadcast WORD from `o1` to 128-bits in `o0` (AVX2).
  INST_2x(vpbroadcastw, kX86InstIdVpbroadcastw, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastw, kX86InstIdVpbroadcastw, X86XmmVar, X86Mem)
  //! Broadcast WORD from `o1` to 256-bits in `o0` (AVX2).
  INST_2x(vpbroadcastw, kX86InstIdVpbroadcastw, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vpbroadcastw, kX86InstIdVpbroadcastw, X86YmmVar, X86Mem)

  //! Packed BYTEs compare for equality (AVX2).
  INST_3x(vpcmpeqb, kX86InstIdVpcmpeqb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpeqb, kX86InstIdVpcmpeqb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DWORDs compare for equality (AVX2).
  INST_3x(vpcmpeqd, kX86InstIdVpcmpeqd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpeqd, kX86InstIdVpcmpeqd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed QWORDs compare for equality (AVX2).
  INST_3x(vpcmpeqq, kX86InstIdVpcmpeqq, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpeqq, kX86InstIdVpcmpeqq, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed WORDs compare for equality (AVX2).
  INST_3x(vpcmpeqw, kX86InstIdVpcmpeqw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpeqw, kX86InstIdVpcmpeqw, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTEs compare if greater than (AVX2).
  INST_3x(vpcmpgtb, kX86InstIdVpcmpgtb, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpgtb, kX86InstIdVpcmpgtb, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DWORDs compare if greater than (AVX2).
  INST_3x(vpcmpgtd, kX86InstIdVpcmpgtd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpgtd, kX86InstIdVpcmpgtd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed QWORDs compare if greater than (AVX2).
  INST_3x(vpcmpgtq, kX86InstIdVpcmpgtq, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpgtq, kX86InstIdVpcmpgtq, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed WORDs compare if greater than (AVX2).
  INST_3x(vpcmpgtw, kX86InstIdVpcmpgtw, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpcmpgtw, kX86InstIdVpcmpgtw, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DQWORD permute (AVX2).
  INST_4i(vperm2i128, kX86InstIdVperm2i128, X86YmmVar, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_4i(vperm2i128, kX86InstIdVperm2i128, X86YmmVar, X86YmmVar, X86Mem, Imm)

  //! Packed DWORD permute (AVX2).
  INST_3x(vpermd, kX86InstIdVpermd, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpermd, kX86InstIdVpermd, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed DP-FP permute (AVX2).
  INST_3i(vpermpd, kX86InstIdVpermpd, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vpermpd, kX86InstIdVpermpd, X86YmmVar, X86Mem, Imm)

  //! Packed SP-FP permute (AVX2).
  INST_3x(vpermps, kX86InstIdVpermps, X86YmmVar, X86YmmVar, X86YmmVar)
  //! \overload
  INST_3x(vpermps, kX86InstIdVpermps, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed QWORD permute (AVX2).
  INST_3i(vpermq, kX86InstIdVpermq, X86YmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vpermq, kX86InstIdVpermq, X86YmmVar, X86Mem, Imm)

  INST_3x(vpgatherdd, kX86InstIdVpgatherdd, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpgatherdd, kX86InstIdVpgatherdd, X86YmmVar, X86Mem, X86YmmVar)

  INST_3x(vpgatherdq, kX86InstIdVpgatherdq, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpgatherdq, kX86InstIdVpgatherdq, X86YmmVar, X86Mem, X86YmmVar)

  INST_3x(vpgatherqd, kX86InstIdVpgatherqd, X86XmmVar, X86Mem, X86XmmVar)

  INST_3x(vpgatherqq, kX86InstIdVpgatherqq, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpgatherqq, kX86InstIdVpgatherqq, X86YmmVar, X86Mem, X86YmmVar)

  //! Packed DWORD horizontal add (AVX2).
  INST_3x(vphaddd, kX86InstIdVphaddd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vphaddd, kX86InstIdVphaddd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD horizontal add with saturation (AVX2).
  INST_3x(vphaddsw, kX86InstIdVphaddsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vphaddsw, kX86InstIdVphaddsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD horizontal add (AVX2).
  INST_3x(vphaddw, kX86InstIdVphaddw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vphaddw, kX86InstIdVphaddw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD horizontal subtract (AVX2).
  INST_3x(vphsubd, kX86InstIdVphsubd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vphsubd, kX86InstIdVphsubd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD horizontal subtract with saturation (AVX2).
  INST_3x(vphsubsw, kX86InstIdVphsubsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vphsubsw, kX86InstIdVphsubsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD horizontal subtract (AVX2).
  INST_3x(vphsubw, kX86InstIdVphsubw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vphsubw, kX86InstIdVphsubw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Move Byte mask to integer (AVX2).
  INST_2x(vpmovmskb, kX86InstIdVpmovmskb, X86GpVar, X86YmmVar)

  //! BYTE to DWORD with sign extend (AVX).
  INST_2x(vpmovsxbd, kX86InstIdVpmovsxbd, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovsxbd, kX86InstIdVpmovsxbd, X86YmmVar, X86XmmVar)

  //! Packed BYTE to QWORD with sign extend (AVX2).
  INST_2x(vpmovsxbq, kX86InstIdVpmovsxbq, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovsxbq, kX86InstIdVpmovsxbq, X86YmmVar, X86XmmVar)

  //! Packed BYTE to WORD with sign extend (AVX2).
  INST_2x(vpmovsxbw, kX86InstIdVpmovsxbw, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovsxbw, kX86InstIdVpmovsxbw, X86YmmVar, X86XmmVar)

  //! Packed DWORD to QWORD with sign extend (AVX2).
  INST_2x(vpmovsxdq, kX86InstIdVpmovsxdq, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovsxdq, kX86InstIdVpmovsxdq, X86YmmVar, X86XmmVar)

  //! Packed WORD to DWORD with sign extend (AVX2).
  INST_2x(vpmovsxwd, kX86InstIdVpmovsxwd, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovsxwd, kX86InstIdVpmovsxwd, X86YmmVar, X86XmmVar)

  //! Packed WORD to QWORD with sign extend (AVX2).
  INST_2x(vpmovsxwq, kX86InstIdVpmovsxwq, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovsxwq, kX86InstIdVpmovsxwq, X86YmmVar, X86XmmVar)

  //! BYTE to DWORD with zero extend (AVX2).
  INST_2x(vpmovzxbd, kX86InstIdVpmovzxbd, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovzxbd, kX86InstIdVpmovzxbd, X86YmmVar, X86XmmVar)

  //! Packed BYTE to QWORD with zero extend (AVX2).
  INST_2x(vpmovzxbq, kX86InstIdVpmovzxbq, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovzxbq, kX86InstIdVpmovzxbq, X86YmmVar, X86XmmVar)

  //! BYTE to WORD with zero extend (AVX2).
  INST_2x(vpmovzxbw, kX86InstIdVpmovzxbw, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovzxbw, kX86InstIdVpmovzxbw, X86YmmVar, X86XmmVar)

  //! Packed DWORD to QWORD with zero extend (AVX2).
  INST_2x(vpmovzxdq, kX86InstIdVpmovzxdq, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovzxdq, kX86InstIdVpmovzxdq, X86YmmVar, X86XmmVar)

  //! Packed WORD to DWORD with zero extend (AVX2).
  INST_2x(vpmovzxwd, kX86InstIdVpmovzxwd, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovzxwd, kX86InstIdVpmovzxwd, X86YmmVar, X86XmmVar)

  //! Packed WORD to QWORD with zero extend (AVX2).
  INST_2x(vpmovzxwq, kX86InstIdVpmovzxwq, X86YmmVar, X86Mem)
  //! \overload
  INST_2x(vpmovzxwq, kX86InstIdVpmovzxwq, X86YmmVar, X86XmmVar)

  //! Packed multiply and add signed and unsigned bytes (AVX2).
  INST_3x(vpmaddubsw, kX86InstIdVpmaddubsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaddubsw, kX86InstIdVpmaddubsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD multiply and add to packed DWORD (AVX2).
  INST_3x(vpmaddwd, kX86InstIdVpmaddwd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaddwd, kX86InstIdVpmaddwd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vpmaskmovd, kX86InstIdVpmaskmovd, X86Mem, X86XmmVar, X86XmmVar)
  INST_3x(vpmaskmovd, kX86InstIdVpmaskmovd, X86Mem, X86YmmVar, X86YmmVar)
  INST_3x(vpmaskmovd, kX86InstIdVpmaskmovd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpmaskmovd, kX86InstIdVpmaskmovd, X86YmmVar, X86YmmVar, X86Mem)

  INST_3x(vpmaskmovq, kX86InstIdVpmaskmovq, X86Mem, X86XmmVar, X86XmmVar)
  INST_3x(vpmaskmovq, kX86InstIdVpmaskmovq, X86Mem, X86YmmVar, X86YmmVar)
  INST_3x(vpmaskmovq, kX86InstIdVpmaskmovq, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpmaskmovq, kX86InstIdVpmaskmovq, X86YmmVar, X86YmmVar, X86Mem)

  //! Packed BYTE maximum (AVX2).
  INST_3x(vpmaxsb, kX86InstIdVpmaxsb, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaxsb, kX86InstIdVpmaxsb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD maximum (AVX2).
  INST_3x(vpmaxsd, kX86InstIdVpmaxsd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaxsd, kX86InstIdVpmaxsd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD maximum (AVX2).
  INST_3x(vpmaxsw, kX86InstIdVpmaxsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaxsw, kX86InstIdVpmaxsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed BYTE unsigned maximum (AVX2).
  INST_3x(vpmaxub, kX86InstIdVpmaxub, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaxub, kX86InstIdVpmaxub, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD unsigned maximum (AVX2).
  INST_3x(vpmaxud, kX86InstIdVpmaxud, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaxud, kX86InstIdVpmaxud, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD unsigned maximum (AVX2).
  INST_3x(vpmaxuw, kX86InstIdVpmaxuw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmaxuw, kX86InstIdVpmaxuw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed BYTE minimum (AVX2).
  INST_3x(vpminsb, kX86InstIdVpminsb, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpminsb, kX86InstIdVpminsb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD minimum (AVX2).
  INST_3x(vpminsd, kX86InstIdVpminsd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpminsd, kX86InstIdVpminsd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD minimum (AVX2).
  INST_3x(vpminsw, kX86InstIdVpminsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpminsw, kX86InstIdVpminsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed BYTE unsigned minimum (AVX2).
  INST_3x(vpminub, kX86InstIdVpminub, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpminub, kX86InstIdVpminub, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD unsigned minimum (AVX2).
  INST_3x(vpminud, kX86InstIdVpminud, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpminud, kX86InstIdVpminud, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD unsigned minimum (AVX2).
  INST_3x(vpminuw, kX86InstIdVpminuw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpminuw, kX86InstIdVpminuw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD to QWORD multiply (AVX2).
  INST_3x(vpmuldq, kX86InstIdVpmuldq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmuldq, kX86InstIdVpmuldq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD multiply high, round and scale (AVX2).
  INST_3x(vpmulhrsw, kX86InstIdVpmulhrsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmulhrsw, kX86InstIdVpmulhrsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD unsigned multiply high (AVX2).
  INST_3x(vpmulhuw, kX86InstIdVpmulhuw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmulhuw, kX86InstIdVpmulhuw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD multiply high (AVX2).
  INST_3x(vpmulhw, kX86InstIdVpmulhw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmulhw, kX86InstIdVpmulhw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD multiply low (AVX2).
  INST_3x(vpmulld, kX86InstIdVpmulld, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmulld, kX86InstIdVpmulld, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORDs multiply low (AVX2).
  INST_3x(vpmullw, kX86InstIdVpmullw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmullw, kX86InstIdVpmullw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD multiply to QWORD (AVX2).
  INST_3x(vpmuludq, kX86InstIdVpmuludq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpmuludq, kX86InstIdVpmuludq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed bitwise or (AVX2).
  INST_3x(vpor, kX86InstIdVpor, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpor, kX86InstIdVpor, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD sum of absolute differences (AVX2).
  INST_3x(vpsadbw, kX86InstIdVpsadbw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsadbw, kX86InstIdVpsadbw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed BYTE shuffle (AVX2).
  INST_3x(vpshufb, kX86InstIdVpshufb, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpshufb, kX86InstIdVpshufb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD shuffle (AVX2).
  INST_3i(vpshufd, kX86InstIdVpshufd, X86YmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vpshufd, kX86InstIdVpshufd, X86YmmVar, X86YmmVar, Imm)

  //! Packed WORD shuffle high (AVX2).
  INST_3i(vpshufhw, kX86InstIdVpshufhw, X86YmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vpshufhw, kX86InstIdVpshufhw, X86YmmVar, X86YmmVar, Imm)

  //! Packed WORD shuffle low (AVX2).
  INST_3i(vpshuflw, kX86InstIdVpshuflw, X86YmmVar, X86Mem, Imm)
  //! \overload
  INST_3i(vpshuflw, kX86InstIdVpshuflw, X86YmmVar, X86YmmVar, Imm)

  //! Packed BYTE sign (AVX2).
  INST_3x(vpsignb, kX86InstIdVpsignb, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsignb, kX86InstIdVpsignb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD sign (AVX2).
  INST_3x(vpsignd, kX86InstIdVpsignd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsignd, kX86InstIdVpsignd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD sign (AVX2).
  INST_3x(vpsignw, kX86InstIdVpsignw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsignw, kX86InstIdVpsignw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD shift left logical (AVX2).
  INST_3x(vpslld, kX86InstIdVpslld, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpslld, kX86InstIdVpslld, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpslld, kX86InstIdVpslld, X86YmmVar, X86YmmVar, Imm)

  //! Packed DQWORD shift left logical (AVX2).
  INST_3i(vpslldq, kX86InstIdVpslldq, X86YmmVar, X86YmmVar, Imm)

  //! Packed QWORD shift left logical (AVX2).
  INST_3x(vpsllq, kX86InstIdVpsllq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsllq, kX86InstIdVpsllq, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpsllq, kX86InstIdVpsllq, X86YmmVar, X86YmmVar, Imm)

  INST_3x(vpsllvd, kX86InstIdVpsllvd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpsllvd, kX86InstIdVpsllvd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpsllvd, kX86InstIdVpsllvd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vpsllvd, kX86InstIdVpsllvd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vpsllvq, kX86InstIdVpsllvq, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpsllvq, kX86InstIdVpsllvq, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpsllvq, kX86InstIdVpsllvq, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vpsllvq, kX86InstIdVpsllvq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD shift left logical (AVX2).
  INST_3x(vpsllw, kX86InstIdVpsllw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsllw, kX86InstIdVpsllw, X86YmmVar, X86YmmVar, X86XmmVar)
  //! Packed WORD shift left logical (AVX2).
  INST_3i(vpsllw, kX86InstIdVpsllw, X86YmmVar, X86YmmVar, Imm)

  //! Packed DWORD shift right arithmetic (AVX2).
  INST_3x(vpsrad, kX86InstIdVpsrad, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsrad, kX86InstIdVpsrad, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpsrad, kX86InstIdVpsrad, X86YmmVar, X86YmmVar, Imm)

  INST_3x(vpsravd, kX86InstIdVpsravd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpsravd, kX86InstIdVpsravd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpsravd, kX86InstIdVpsravd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vpsravd, kX86InstIdVpsravd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD shift right arithmetic (AVX2).
  INST_3x(vpsraw, kX86InstIdVpsraw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsraw, kX86InstIdVpsraw, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpsraw, kX86InstIdVpsraw, X86YmmVar, X86YmmVar, Imm)

  //! Packed DWORD shift right logical (AVX2).
  INST_3x(vpsrld, kX86InstIdVpsrld, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsrld, kX86InstIdVpsrld, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpsrld, kX86InstIdVpsrld, X86YmmVar, X86YmmVar, Imm)

  //! Scalar DQWORD shift right logical (AVX2).
  INST_3i(vpsrldq, kX86InstIdVpsrldq, X86YmmVar, X86YmmVar, Imm)

  //! Packed QWORD shift right logical (AVX2).
  INST_3x(vpsrlq, kX86InstIdVpsrlq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsrlq, kX86InstIdVpsrlq, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpsrlq, kX86InstIdVpsrlq, X86YmmVar, X86YmmVar, Imm)

  INST_3x(vpsrlvd, kX86InstIdVpsrlvd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpsrlvd, kX86InstIdVpsrlvd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpsrlvd, kX86InstIdVpsrlvd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vpsrlvd, kX86InstIdVpsrlvd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vpsrlvq, kX86InstIdVpsrlvq, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vpsrlvq, kX86InstIdVpsrlvq, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpsrlvq, kX86InstIdVpsrlvq, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vpsrlvq, kX86InstIdVpsrlvq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD shift right logical (AVX2).
  INST_3x(vpsrlw, kX86InstIdVpsrlw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsrlw, kX86InstIdVpsrlw, X86YmmVar, X86YmmVar, X86XmmVar)
  //! \overload
  INST_3i(vpsrlw, kX86InstIdVpsrlw, X86YmmVar, X86YmmVar, Imm)

  //! Packed BYTE subtract (AVX2).
  INST_3x(vpsubb, kX86InstIdVpsubb, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vpsubb, kX86InstIdVpsubb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed DWORD subtract (AVX2).
  INST_3x(vpsubd, kX86InstIdVpsubd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubd, kX86InstIdVpsubd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed QWORD subtract (AVX2).
  INST_3x(vpsubq, kX86InstIdVpsubq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubq, kX86InstIdVpsubq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed BYTE subtract with saturation (AVX2).
  INST_3x(vpsubsb, kX86InstIdVpsubsb, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubsb, kX86InstIdVpsubsb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD subtract with saturation (AVX2).
  INST_3x(vpsubsw, kX86InstIdVpsubsw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubsw, kX86InstIdVpsubsw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed BYTE subtract with unsigned saturation (AVX2).
  INST_3x(vpsubusb, kX86InstIdVpsubusb, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubusb, kX86InstIdVpsubusb, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD subtract with unsigned saturation (AVX2).
  INST_3x(vpsubusw, kX86InstIdVpsubusw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubusw, kX86InstIdVpsubusw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed WORD subtract (AVX2).
  INST_3x(vpsubw, kX86InstIdVpsubw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpsubw, kX86InstIdVpsubw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack high packed BYTEs to WORDs (AVX2).
  INST_3x(vpunpckhbw, kX86InstIdVpunpckhbw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpckhbw, kX86InstIdVpunpckhbw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack high packed DWORDs to QWORDs (AVX2).
  INST_3x(vpunpckhdq, kX86InstIdVpunpckhdq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpckhdq, kX86InstIdVpunpckhdq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack high packed QWORDs to DQWORD (AVX2).
  INST_3x(vpunpckhqdq, kX86InstIdVpunpckhqdq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpckhqdq, kX86InstIdVpunpckhqdq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack high packed WORDs to DWORDs (AVX2).
  INST_3x(vpunpckhwd, kX86InstIdVpunpckhwd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpckhwd, kX86InstIdVpunpckhwd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack low packed BYTEs to WORDs (AVX2).
  INST_3x(vpunpcklbw, kX86InstIdVpunpcklbw, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpcklbw, kX86InstIdVpunpcklbw, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack low packed DWORDs to QWORDs (AVX2).
  INST_3x(vpunpckldq, kX86InstIdVpunpckldq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpckldq, kX86InstIdVpunpckldq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack low packed QWORDs to DQWORD (AVX2).
  INST_3x(vpunpcklqdq, kX86InstIdVpunpcklqdq, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpcklqdq, kX86InstIdVpunpcklqdq, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Unpack low packed WORDs to DWORDs (AVX2).
  INST_3x(vpunpcklwd, kX86InstIdVpunpcklwd, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpunpcklwd, kX86InstIdVpunpcklwd, X86YmmVar, X86YmmVar, X86YmmVar)

  //! Packed bitwise xor (AVX2).
  INST_3x(vpxor, kX86InstIdVpxor, X86YmmVar, X86YmmVar, X86Mem)
  //! \overload
  INST_3x(vpxor, kX86InstIdVpxor, X86YmmVar, X86YmmVar, X86YmmVar)

  // --------------------------------------------------------------------------
  // [FMA3]
  // --------------------------------------------------------------------------

  INST_3x(vfmadd132pd, kX86InstIdVfmadd132pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd132pd, kX86InstIdVfmadd132pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmadd132pd, kX86InstIdVfmadd132pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmadd132pd, kX86InstIdVfmadd132pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmadd132ps, kX86InstIdVfmadd132ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd132ps, kX86InstIdVfmadd132ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmadd132ps, kX86InstIdVfmadd132ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmadd132ps, kX86InstIdVfmadd132ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmadd132sd, kX86InstIdVfmadd132sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd132sd, kX86InstIdVfmadd132sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmadd132ss, kX86InstIdVfmadd132ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd132ss, kX86InstIdVfmadd132ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmadd213pd, kX86InstIdVfmadd213pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd213pd, kX86InstIdVfmadd213pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmadd213pd, kX86InstIdVfmadd213pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmadd213pd, kX86InstIdVfmadd213pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmadd213ps, kX86InstIdVfmadd213ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd213ps, kX86InstIdVfmadd213ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmadd213ps, kX86InstIdVfmadd213ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmadd213ps, kX86InstIdVfmadd213ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmadd213sd, kX86InstIdVfmadd213sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd213sd, kX86InstIdVfmadd213sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmadd213ss, kX86InstIdVfmadd213ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd213ss, kX86InstIdVfmadd213ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmadd231pd, kX86InstIdVfmadd231pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd231pd, kX86InstIdVfmadd231pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmadd231pd, kX86InstIdVfmadd231pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmadd231pd, kX86InstIdVfmadd231pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmadd231ps, kX86InstIdVfmadd231ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd231ps, kX86InstIdVfmadd231ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmadd231ps, kX86InstIdVfmadd231ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmadd231ps, kX86InstIdVfmadd231ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmadd231sd, kX86InstIdVfmadd231sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd231sd, kX86InstIdVfmadd231sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmadd231ss, kX86InstIdVfmadd231ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmadd231ss, kX86InstIdVfmadd231ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmaddsub132pd, kX86InstIdVfmaddsub132pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmaddsub132pd, kX86InstIdVfmaddsub132pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmaddsub132pd, kX86InstIdVfmaddsub132pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmaddsub132pd, kX86InstIdVfmaddsub132pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmaddsub132ps, kX86InstIdVfmaddsub132ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmaddsub132ps, kX86InstIdVfmaddsub132ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmaddsub132ps, kX86InstIdVfmaddsub132ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmaddsub132ps, kX86InstIdVfmaddsub132ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmaddsub213pd, kX86InstIdVfmaddsub213pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmaddsub213pd, kX86InstIdVfmaddsub213pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmaddsub213pd, kX86InstIdVfmaddsub213pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmaddsub213pd, kX86InstIdVfmaddsub213pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmaddsub213ps, kX86InstIdVfmaddsub213ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmaddsub213ps, kX86InstIdVfmaddsub213ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmaddsub213ps, kX86InstIdVfmaddsub213ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmaddsub213ps, kX86InstIdVfmaddsub213ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmaddsub231pd, kX86InstIdVfmaddsub231pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmaddsub231pd, kX86InstIdVfmaddsub231pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmaddsub231pd, kX86InstIdVfmaddsub231pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmaddsub231pd, kX86InstIdVfmaddsub231pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmaddsub231ps, kX86InstIdVfmaddsub231ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmaddsub231ps, kX86InstIdVfmaddsub231ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmaddsub231ps, kX86InstIdVfmaddsub231ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmaddsub231ps, kX86InstIdVfmaddsub231ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub132pd, kX86InstIdVfmsub132pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub132pd, kX86InstIdVfmsub132pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsub132pd, kX86InstIdVfmsub132pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsub132pd, kX86InstIdVfmsub132pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub132ps, kX86InstIdVfmsub132ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub132ps, kX86InstIdVfmsub132ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsub132ps, kX86InstIdVfmsub132ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsub132ps, kX86InstIdVfmsub132ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub132sd, kX86InstIdVfmsub132sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub132sd, kX86InstIdVfmsub132sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmsub132ss, kX86InstIdVfmsub132ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub132ss, kX86InstIdVfmsub132ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmsub213pd, kX86InstIdVfmsub213pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub213pd, kX86InstIdVfmsub213pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsub213pd, kX86InstIdVfmsub213pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsub213pd, kX86InstIdVfmsub213pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub213ps, kX86InstIdVfmsub213ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub213ps, kX86InstIdVfmsub213ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsub213ps, kX86InstIdVfmsub213ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsub213ps, kX86InstIdVfmsub213ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub213sd, kX86InstIdVfmsub213sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub213sd, kX86InstIdVfmsub213sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmsub213ss, kX86InstIdVfmsub213ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub213ss, kX86InstIdVfmsub213ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmsub231pd, kX86InstIdVfmsub231pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub231pd, kX86InstIdVfmsub231pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsub231pd, kX86InstIdVfmsub231pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsub231pd, kX86InstIdVfmsub231pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub231ps, kX86InstIdVfmsub231ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub231ps, kX86InstIdVfmsub231ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsub231ps, kX86InstIdVfmsub231ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsub231ps, kX86InstIdVfmsub231ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsub231sd, kX86InstIdVfmsub231sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub231sd, kX86InstIdVfmsub231sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmsub231ss, kX86InstIdVfmsub231ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsub231ss, kX86InstIdVfmsub231ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfmsubadd132pd, kX86InstIdVfmsubadd132pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsubadd132pd, kX86InstIdVfmsubadd132pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsubadd132pd, kX86InstIdVfmsubadd132pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsubadd132pd, kX86InstIdVfmsubadd132pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsubadd132ps, kX86InstIdVfmsubadd132ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsubadd132ps, kX86InstIdVfmsubadd132ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsubadd132ps, kX86InstIdVfmsubadd132ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsubadd132ps, kX86InstIdVfmsubadd132ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsubadd213pd, kX86InstIdVfmsubadd213pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsubadd213pd, kX86InstIdVfmsubadd213pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsubadd213pd, kX86InstIdVfmsubadd213pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsubadd213pd, kX86InstIdVfmsubadd213pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsubadd213ps, kX86InstIdVfmsubadd213ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsubadd213ps, kX86InstIdVfmsubadd213ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsubadd213ps, kX86InstIdVfmsubadd213ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsubadd213ps, kX86InstIdVfmsubadd213ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsubadd231pd, kX86InstIdVfmsubadd231pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsubadd231pd, kX86InstIdVfmsubadd231pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsubadd231pd, kX86InstIdVfmsubadd231pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsubadd231pd, kX86InstIdVfmsubadd231pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfmsubadd231ps, kX86InstIdVfmsubadd231ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfmsubadd231ps, kX86InstIdVfmsubadd231ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfmsubadd231ps, kX86InstIdVfmsubadd231ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfmsubadd231ps, kX86InstIdVfmsubadd231ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd132pd, kX86InstIdVfnmadd132pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd132pd, kX86InstIdVfnmadd132pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmadd132pd, kX86InstIdVfnmadd132pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmadd132pd, kX86InstIdVfnmadd132pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd132ps, kX86InstIdVfnmadd132ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd132ps, kX86InstIdVfnmadd132ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmadd132ps, kX86InstIdVfnmadd132ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmadd132ps, kX86InstIdVfnmadd132ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd132sd, kX86InstIdVfnmadd132sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd132sd, kX86InstIdVfnmadd132sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmadd132ss, kX86InstIdVfnmadd132ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd132ss, kX86InstIdVfnmadd132ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmadd213pd, kX86InstIdVfnmadd213pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd213pd, kX86InstIdVfnmadd213pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmadd213pd, kX86InstIdVfnmadd213pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmadd213pd, kX86InstIdVfnmadd213pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd213ps, kX86InstIdVfnmadd213ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd213ps, kX86InstIdVfnmadd213ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmadd213ps, kX86InstIdVfnmadd213ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmadd213ps, kX86InstIdVfnmadd213ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd213sd, kX86InstIdVfnmadd213sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd213sd, kX86InstIdVfnmadd213sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmadd213ss, kX86InstIdVfnmadd213ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd213ss, kX86InstIdVfnmadd213ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmadd231pd, kX86InstIdVfnmadd231pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd231pd, kX86InstIdVfnmadd231pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmadd231pd, kX86InstIdVfnmadd231pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmadd231pd, kX86InstIdVfnmadd231pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd231ps, kX86InstIdVfnmadd231ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd231ps, kX86InstIdVfnmadd231ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmadd231ps, kX86InstIdVfnmadd231ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmadd231ps, kX86InstIdVfnmadd231ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmadd231sd, kX86InstIdVfnmadd231sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd231sd, kX86InstIdVfnmadd231sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmadd231ss, kX86InstIdVfnmadd231ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmadd231ss, kX86InstIdVfnmadd231ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmsub132pd, kX86InstIdVfnmsub132pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub132pd, kX86InstIdVfnmsub132pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmsub132pd, kX86InstIdVfnmsub132pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmsub132pd, kX86InstIdVfnmsub132pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmsub132ps, kX86InstIdVfnmsub132ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub132ps, kX86InstIdVfnmsub132ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmsub132ps, kX86InstIdVfnmsub132ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmsub132ps, kX86InstIdVfnmsub132ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmsub132sd, kX86InstIdVfnmsub132sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub132sd, kX86InstIdVfnmsub132sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmsub132ss, kX86InstIdVfnmsub132ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub132ss, kX86InstIdVfnmsub132ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmsub213pd, kX86InstIdVfnmsub213pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub213pd, kX86InstIdVfnmsub213pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmsub213pd, kX86InstIdVfnmsub213pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmsub213pd, kX86InstIdVfnmsub213pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmsub213ps, kX86InstIdVfnmsub213ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub213ps, kX86InstIdVfnmsub213ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmsub213ps, kX86InstIdVfnmsub213ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmsub213ps, kX86InstIdVfnmsub213ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmsub213sd, kX86InstIdVfnmsub213sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub213sd, kX86InstIdVfnmsub213sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmsub213ss, kX86InstIdVfnmsub213ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub213ss, kX86InstIdVfnmsub213ss, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmsub231pd, kX86InstIdVfnmsub231pd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub231pd, kX86InstIdVfnmsub231pd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmsub231pd, kX86InstIdVfnmsub231pd, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmsub231pd, kX86InstIdVfnmsub231pd, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmsub231ps, kX86InstIdVfnmsub231ps, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub231ps, kX86InstIdVfnmsub231ps, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vfnmsub231ps, kX86InstIdVfnmsub231ps, X86YmmVar, X86YmmVar, X86Mem)
  INST_3x(vfnmsub231ps, kX86InstIdVfnmsub231ps, X86YmmVar, X86YmmVar, X86YmmVar)

  INST_3x(vfnmsub231sd, kX86InstIdVfnmsub231sd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub231sd, kX86InstIdVfnmsub231sd, X86XmmVar, X86XmmVar, X86XmmVar)

  INST_3x(vfnmsub231ss, kX86InstIdVfnmsub231ss, X86XmmVar, X86XmmVar, X86Mem)
  INST_3x(vfnmsub231ss, kX86InstIdVfnmsub231ss, X86XmmVar, X86XmmVar, X86XmmVar)

  // --------------------------------------------------------------------------
  // [FMA4]
  // --------------------------------------------------------------------------

  INST_4x(vfmaddpd, kX86InstIdVfmaddpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmaddpd, kX86InstIdVfmaddpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmaddpd, kX86InstIdVfmaddpd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmaddpd, kX86InstIdVfmaddpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmaddpd, kX86InstIdVfmaddpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmaddpd, kX86InstIdVfmaddpd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmaddps, kX86InstIdVfmaddps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmaddps, kX86InstIdVfmaddps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmaddps, kX86InstIdVfmaddps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmaddps, kX86InstIdVfmaddps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmaddps, kX86InstIdVfmaddps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmaddps, kX86InstIdVfmaddps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmaddsd, kX86InstIdVfmaddsd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmaddsd, kX86InstIdVfmaddsd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmaddsd, kX86InstIdVfmaddsd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfmaddss, kX86InstIdVfmaddss, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmaddss, kX86InstIdVfmaddss, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmaddss, kX86InstIdVfmaddss, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfmaddsubpd, kX86InstIdVfmaddsubpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmaddsubpd, kX86InstIdVfmaddsubpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmaddsubpd, kX86InstIdVfmaddsubpd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmaddsubpd, kX86InstIdVfmaddsubpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmaddsubpd, kX86InstIdVfmaddsubpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmaddsubpd, kX86InstIdVfmaddsubpd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmaddsubps, kX86InstIdVfmaddsubps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmaddsubps, kX86InstIdVfmaddsubps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmaddsubps, kX86InstIdVfmaddsubps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmaddsubps, kX86InstIdVfmaddsubps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmaddsubps, kX86InstIdVfmaddsubps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmaddsubps, kX86InstIdVfmaddsubps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmsubaddpd, kX86InstIdVfmsubaddpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmsubaddpd, kX86InstIdVfmsubaddpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmsubaddpd, kX86InstIdVfmsubaddpd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmsubaddpd, kX86InstIdVfmsubaddpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmsubaddpd, kX86InstIdVfmsubaddpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmsubaddpd, kX86InstIdVfmsubaddpd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmsubaddps, kX86InstIdVfmsubaddps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmsubaddps, kX86InstIdVfmsubaddps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmsubaddps, kX86InstIdVfmsubaddps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmsubaddps, kX86InstIdVfmsubaddps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmsubaddps, kX86InstIdVfmsubaddps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmsubaddps, kX86InstIdVfmsubaddps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmsubpd, kX86InstIdVfmsubpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmsubpd, kX86InstIdVfmsubpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmsubpd, kX86InstIdVfmsubpd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmsubpd, kX86InstIdVfmsubpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmsubpd, kX86InstIdVfmsubpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmsubpd, kX86InstIdVfmsubpd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmsubps, kX86InstIdVfmsubps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmsubps, kX86InstIdVfmsubps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmsubps, kX86InstIdVfmsubps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfmsubps, kX86InstIdVfmsubps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfmsubps, kX86InstIdVfmsubps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfmsubps, kX86InstIdVfmsubps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfmsubsd, kX86InstIdVfmsubsd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmsubsd, kX86InstIdVfmsubsd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmsubsd, kX86InstIdVfmsubsd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfmsubss, kX86InstIdVfmsubss, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfmsubss, kX86InstIdVfmsubss, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfmsubss, kX86InstIdVfmsubss, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfnmaddpd, kX86InstIdVfnmaddpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmaddpd, kX86InstIdVfnmaddpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmaddpd, kX86InstIdVfnmaddpd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfnmaddpd, kX86InstIdVfnmaddpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfnmaddpd, kX86InstIdVfnmaddpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfnmaddpd, kX86InstIdVfnmaddpd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfnmaddps, kX86InstIdVfnmaddps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmaddps, kX86InstIdVfnmaddps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmaddps, kX86InstIdVfnmaddps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfnmaddps, kX86InstIdVfnmaddps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfnmaddps, kX86InstIdVfnmaddps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfnmaddps, kX86InstIdVfnmaddps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfnmaddsd, kX86InstIdVfnmaddsd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmaddsd, kX86InstIdVfnmaddsd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmaddsd, kX86InstIdVfnmaddsd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfnmaddss, kX86InstIdVfnmaddss, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmaddss, kX86InstIdVfnmaddss, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmaddss, kX86InstIdVfnmaddss, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfnmsubpd, kX86InstIdVfnmsubpd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmsubpd, kX86InstIdVfnmsubpd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmsubpd, kX86InstIdVfnmsubpd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfnmsubpd, kX86InstIdVfnmsubpd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfnmsubpd, kX86InstIdVfnmsubpd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfnmsubpd, kX86InstIdVfnmsubpd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfnmsubps, kX86InstIdVfnmsubps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmsubps, kX86InstIdVfnmsubps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmsubps, kX86InstIdVfnmsubps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vfnmsubps, kX86InstIdVfnmsubps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vfnmsubps, kX86InstIdVfnmsubps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vfnmsubps, kX86InstIdVfnmsubps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vfnmsubsd, kX86InstIdVfnmsubsd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmsubsd, kX86InstIdVfnmsubsd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmsubsd, kX86InstIdVfnmsubsd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_4x(vfnmsubss, kX86InstIdVfnmsubss, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vfnmsubss, kX86InstIdVfnmsubss, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vfnmsubss, kX86InstIdVfnmsubss, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  // --------------------------------------------------------------------------
  // [XOP]
  // --------------------------------------------------------------------------

  INST_2x(vfrczpd, kX86InstIdVfrczpd, X86XmmVar, X86XmmVar)
  INST_2x(vfrczpd, kX86InstIdVfrczpd, X86XmmVar, X86Mem)
  INST_2x(vfrczpd, kX86InstIdVfrczpd, X86YmmVar, X86YmmVar)
  INST_2x(vfrczpd, kX86InstIdVfrczpd, X86YmmVar, X86Mem)

  INST_2x(vfrczps, kX86InstIdVfrczps, X86XmmVar, X86XmmVar)
  INST_2x(vfrczps, kX86InstIdVfrczps, X86XmmVar, X86Mem)
  INST_2x(vfrczps, kX86InstIdVfrczps, X86YmmVar, X86YmmVar)
  INST_2x(vfrczps, kX86InstIdVfrczps, X86YmmVar, X86Mem)

  INST_2x(vfrczsd, kX86InstIdVfrczsd, X86XmmVar, X86XmmVar)
  INST_2x(vfrczsd, kX86InstIdVfrczsd, X86XmmVar, X86Mem)

  INST_2x(vfrczss, kX86InstIdVfrczss, X86XmmVar, X86XmmVar)
  INST_2x(vfrczss, kX86InstIdVfrczss, X86XmmVar, X86Mem)

  INST_4x(vpcmov, kX86InstIdVpcmov, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpcmov, kX86InstIdVpcmov, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpcmov, kX86InstIdVpcmov, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vpcmov, kX86InstIdVpcmov, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vpcmov, kX86InstIdVpcmov, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vpcmov, kX86InstIdVpcmov, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4i(vpcomb, kX86InstIdVpcomb, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomb, kX86InstIdVpcomb, X86XmmVar, X86XmmVar, X86Mem, Imm)
  INST_4i(vpcomd, kX86InstIdVpcomd, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomd, kX86InstIdVpcomd, X86XmmVar, X86XmmVar, X86Mem, Imm)
  INST_4i(vpcomq, kX86InstIdVpcomq, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomq, kX86InstIdVpcomq, X86XmmVar, X86XmmVar, X86Mem, Imm)
  INST_4i(vpcomw, kX86InstIdVpcomw, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomw, kX86InstIdVpcomw, X86XmmVar, X86XmmVar, X86Mem, Imm)

  INST_4i(vpcomub, kX86InstIdVpcomub, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomub, kX86InstIdVpcomub, X86XmmVar, X86XmmVar, X86Mem, Imm)
  INST_4i(vpcomud, kX86InstIdVpcomud, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomud, kX86InstIdVpcomud, X86XmmVar, X86XmmVar, X86Mem, Imm)
  INST_4i(vpcomuq, kX86InstIdVpcomuq, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomuq, kX86InstIdVpcomuq, X86XmmVar, X86XmmVar, X86Mem, Imm)
  INST_4i(vpcomuw, kX86InstIdVpcomuw, X86XmmVar, X86XmmVar, X86XmmVar, Imm)
  INST_4i(vpcomuw, kX86InstIdVpcomuw, X86XmmVar, X86XmmVar, X86Mem, Imm)

  INST_4x(vpermil2pd, kX86InstIdVpermil2pd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpermil2pd, kX86InstIdVpermil2pd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpermil2pd, kX86InstIdVpermil2pd, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vpermil2pd, kX86InstIdVpermil2pd, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vpermil2pd, kX86InstIdVpermil2pd, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vpermil2pd, kX86InstIdVpermil2pd, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_4x(vpermil2ps, kX86InstIdVpermil2ps, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpermil2ps, kX86InstIdVpermil2ps, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpermil2ps, kX86InstIdVpermil2ps, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)
  INST_4x(vpermil2ps, kX86InstIdVpermil2ps, X86YmmVar, X86YmmVar, X86YmmVar, X86YmmVar)
  INST_4x(vpermil2ps, kX86InstIdVpermil2ps, X86YmmVar, X86YmmVar, X86Mem, X86YmmVar)
  INST_4x(vpermil2ps, kX86InstIdVpermil2ps, X86YmmVar, X86YmmVar, X86YmmVar, X86Mem)

  INST_2x(vphaddbd, kX86InstIdVphaddbd, X86XmmVar, X86XmmVar)
  INST_2x(vphaddbd, kX86InstIdVphaddbd, X86XmmVar, X86Mem)
  INST_2x(vphaddbq, kX86InstIdVphaddbq, X86XmmVar, X86XmmVar)
  INST_2x(vphaddbq, kX86InstIdVphaddbq, X86XmmVar, X86Mem)
  INST_2x(vphaddbw, kX86InstIdVphaddbw, X86XmmVar, X86XmmVar)
  INST_2x(vphaddbw, kX86InstIdVphaddbw, X86XmmVar, X86Mem)
  INST_2x(vphadddq, kX86InstIdVphadddq, X86XmmVar, X86XmmVar)
  INST_2x(vphadddq, kX86InstIdVphadddq, X86XmmVar, X86Mem)
  INST_2x(vphaddwd, kX86InstIdVphaddwd, X86XmmVar, X86XmmVar)
  INST_2x(vphaddwd, kX86InstIdVphaddwd, X86XmmVar, X86Mem)
  INST_2x(vphaddwq, kX86InstIdVphaddwq, X86XmmVar, X86XmmVar)
  INST_2x(vphaddwq, kX86InstIdVphaddwq, X86XmmVar, X86Mem)

  INST_2x(vphaddubd, kX86InstIdVphaddubd, X86XmmVar, X86XmmVar)
  INST_2x(vphaddubd, kX86InstIdVphaddubd, X86XmmVar, X86Mem)
  INST_2x(vphaddubq, kX86InstIdVphaddubq, X86XmmVar, X86XmmVar)
  INST_2x(vphaddubq, kX86InstIdVphaddubq, X86XmmVar, X86Mem)
  INST_2x(vphaddubw, kX86InstIdVphaddubw, X86XmmVar, X86XmmVar)
  INST_2x(vphaddubw, kX86InstIdVphaddubw, X86XmmVar, X86Mem)
  INST_2x(vphaddudq, kX86InstIdVphaddudq, X86XmmVar, X86XmmVar)
  INST_2x(vphaddudq, kX86InstIdVphaddudq, X86XmmVar, X86Mem)
  INST_2x(vphadduwd, kX86InstIdVphadduwd, X86XmmVar, X86XmmVar)
  INST_2x(vphadduwd, kX86InstIdVphadduwd, X86XmmVar, X86Mem)
  INST_2x(vphadduwq, kX86InstIdVphadduwq, X86XmmVar, X86XmmVar)
  INST_2x(vphadduwq, kX86InstIdVphadduwq, X86XmmVar, X86Mem)

  INST_2x(vphsubbw, kX86InstIdVphsubbw, X86XmmVar, X86XmmVar)
  INST_2x(vphsubbw, kX86InstIdVphsubbw, X86XmmVar, X86Mem)
  INST_2x(vphsubdq, kX86InstIdVphsubdq, X86XmmVar, X86XmmVar)
  INST_2x(vphsubdq, kX86InstIdVphsubdq, X86XmmVar, X86Mem)
  INST_2x(vphsubwd, kX86InstIdVphsubwd, X86XmmVar, X86XmmVar)
  INST_2x(vphsubwd, kX86InstIdVphsubwd, X86XmmVar, X86Mem)

  INST_4x(vpmacsdd, kX86InstIdVpmacsdd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacsdd, kX86InstIdVpmacsdd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacsdqh, kX86InstIdVpmacsdqh, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacsdqh, kX86InstIdVpmacsdqh, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacsdql, kX86InstIdVpmacsdql, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacsdql, kX86InstIdVpmacsdql, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacswd, kX86InstIdVpmacswd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacswd, kX86InstIdVpmacswd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacsww, kX86InstIdVpmacsww, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacsww, kX86InstIdVpmacsww, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)

  INST_4x(vpmacssdd, kX86InstIdVpmacssdd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacssdd, kX86InstIdVpmacssdd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacssdqh, kX86InstIdVpmacssdqh, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacssdqh, kX86InstIdVpmacssdqh, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacssdql, kX86InstIdVpmacssdql, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacssdql, kX86InstIdVpmacssdql, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacsswd, kX86InstIdVpmacsswd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacsswd, kX86InstIdVpmacsswd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpmacssww, kX86InstIdVpmacssww, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmacssww, kX86InstIdVpmacssww, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)

  INST_4x(vpmadcsswd, kX86InstIdVpmadcsswd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmadcsswd, kX86InstIdVpmadcsswd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)

  INST_4x(vpmadcswd, kX86InstIdVpmadcswd, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpmadcswd, kX86InstIdVpmadcswd, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)

  INST_4x(vpperm, kX86InstIdVpperm, X86XmmVar, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_4x(vpperm, kX86InstIdVpperm, X86XmmVar, X86XmmVar, X86Mem, X86XmmVar)
  INST_4x(vpperm, kX86InstIdVpperm, X86XmmVar, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vprotb, kX86InstIdVprotb, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vprotb, kX86InstIdVprotb, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vprotb, kX86InstIdVprotb, X86XmmVar, X86XmmVar, X86Mem)
  INST_3i(vprotb, kX86InstIdVprotb, X86XmmVar, X86XmmVar, Imm)
  INST_3i(vprotb, kX86InstIdVprotb, X86XmmVar, X86Mem, Imm)

  INST_3x(vprotd, kX86InstIdVprotd, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vprotd, kX86InstIdVprotd, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vprotd, kX86InstIdVprotd, X86XmmVar, X86XmmVar, X86Mem)
  INST_3i(vprotd, kX86InstIdVprotd, X86XmmVar, X86XmmVar, Imm)
  INST_3i(vprotd, kX86InstIdVprotd, X86XmmVar, X86Mem, Imm)

  INST_3x(vprotq, kX86InstIdVprotq, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vprotq, kX86InstIdVprotq, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vprotq, kX86InstIdVprotq, X86XmmVar, X86XmmVar, X86Mem)
  INST_3i(vprotq, kX86InstIdVprotq, X86XmmVar, X86XmmVar, Imm)
  INST_3i(vprotq, kX86InstIdVprotq, X86XmmVar, X86Mem, Imm)

  INST_3x(vprotw, kX86InstIdVprotw, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vprotw, kX86InstIdVprotw, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vprotw, kX86InstIdVprotw, X86XmmVar, X86XmmVar, X86Mem)
  INST_3i(vprotw, kX86InstIdVprotw, X86XmmVar, X86XmmVar, Imm)
  INST_3i(vprotw, kX86InstIdVprotw, X86XmmVar, X86Mem, Imm)

  INST_3x(vpshab, kX86InstIdVpshab, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshab, kX86InstIdVpshab, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshab, kX86InstIdVpshab, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshad, kX86InstIdVpshad, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshad, kX86InstIdVpshad, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshad, kX86InstIdVpshad, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshaq, kX86InstIdVpshaq, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshaq, kX86InstIdVpshaq, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshaq, kX86InstIdVpshaq, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshaw, kX86InstIdVpshaw, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshaw, kX86InstIdVpshaw, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshaw, kX86InstIdVpshaw, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshlb, kX86InstIdVpshlb, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshlb, kX86InstIdVpshlb, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshlb, kX86InstIdVpshlb, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshld, kX86InstIdVpshld, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshld, kX86InstIdVpshld, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshld, kX86InstIdVpshld, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshlq, kX86InstIdVpshlq, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshlq, kX86InstIdVpshlq, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshlq, kX86InstIdVpshlq, X86XmmVar, X86XmmVar, X86Mem)

  INST_3x(vpshlw, kX86InstIdVpshlw, X86XmmVar, X86XmmVar, X86XmmVar)
  INST_3x(vpshlw, kX86InstIdVpshlw, X86XmmVar, X86Mem, X86XmmVar)
  INST_3x(vpshlw, kX86InstIdVpshlw, X86XmmVar, X86XmmVar, X86Mem)

  // --------------------------------------------------------------------------
  // [F16C]
  // --------------------------------------------------------------------------

  //! Convert packed HP-FP to SP-FP.
  INST_2x(vcvtph2ps, kX86InstIdVcvtph2ps, X86XmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtph2ps, kX86InstIdVcvtph2ps, X86XmmVar, X86Mem)
  //! \overload
  INST_2x(vcvtph2ps, kX86InstIdVcvtph2ps, X86YmmVar, X86XmmVar)
  //! \overload
  INST_2x(vcvtph2ps, kX86InstIdVcvtph2ps, X86YmmVar, X86Mem)

  //! Convert packed SP-FP to HP-FP.
  INST_3i(vcvtps2ph, kX86InstIdVcvtps2ph, X86XmmVar, X86XmmVar, Imm)
  //! \overload
  INST_3i(vcvtps2ph, kX86InstIdVcvtps2ph, X86Mem, X86XmmVar, Imm)
  //! \overload
  INST_3i(vcvtps2ph, kX86InstIdVcvtps2ph, X86XmmVar, X86YmmVar, Imm)
  //! \overload
  INST_3i(vcvtps2ph, kX86InstIdVcvtps2ph, X86Mem, X86YmmVar, Imm)

  // --------------------------------------------------------------------------
  // [Cleanup]
  // --------------------------------------------------------------------------

#undef INST_0x

#undef INST_1x
#undef INST_1i
#undef INST_1cc

#undef INST_2x
#undef INST_2i
#undef INST_2cc

#undef INST_3x
#undef INST_3i
#undef INST_3ii

#undef INST_4x
#undef INST_4i
#undef INST_4ii
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
#endif // _ASMJIT_X86_X86COMPILER_H
