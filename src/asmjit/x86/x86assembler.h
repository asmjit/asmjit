// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_X86_X86ASSEMBLER_H
#define _ASMJIT_X86_X86ASSEMBLER_H

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../x86/x86defs.h"

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {
namespace x86x64 {

//! @addtogroup asmjit_x86x64
//! @{

// ============================================================================
// [CodeGen-Begin]
// ============================================================================

#define INST_0x(_Inst_, _Code_) \
  ASMJIT_INLINE Error _Inst_() { \
    return emit(_Code_); \
  }

#define INST_1x(_Inst_, _Code_, _Op0_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0) { \
    return emit(_Code_, o0); \
  }

#define INST_1x_(_Inst_, _Code_, _Op0_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0); \
  }

#define INST_1i(_Inst_, _Code_, _Op0_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0) { \
    return emit(_Code_, o0); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(int o0) { \
    return emit(_Code_, o0); \
  }

#define INST_1i_(_Inst_, _Code_, _Op0_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(int o0) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0); \
  }

#define INST_1cc(_Inst_, _Code_, _Translate_, _Op0_) \
  ASMJIT_INLINE Error _Inst_(uint32_t cc, const _Op0_& o0) { \
    return emit(_Translate_(cc), o0); \
  } \
  \
  ASMJIT_INLINE Error _Inst_##a(const _Op0_& o0) { return emit(_Code_##a, o0); } \
  ASMJIT_INLINE Error _Inst_##ae(const _Op0_& o0) { return emit(_Code_##ae, o0); } \
  ASMJIT_INLINE Error _Inst_##b(const _Op0_& o0) { return emit(_Code_##b, o0); } \
  ASMJIT_INLINE Error _Inst_##be(const _Op0_& o0) { return emit(_Code_##be, o0); } \
  ASMJIT_INLINE Error _Inst_##c(const _Op0_& o0) { return emit(_Code_##c, o0); } \
  ASMJIT_INLINE Error _Inst_##e(const _Op0_& o0) { return emit(_Code_##e, o0); } \
  ASMJIT_INLINE Error _Inst_##g(const _Op0_& o0) { return emit(_Code_##g, o0); } \
  ASMJIT_INLINE Error _Inst_##ge(const _Op0_& o0) { return emit(_Code_##ge, o0); } \
  ASMJIT_INLINE Error _Inst_##l(const _Op0_& o0) { return emit(_Code_##l, o0); } \
  ASMJIT_INLINE Error _Inst_##le(const _Op0_& o0) { return emit(_Code_##le, o0); } \
  ASMJIT_INLINE Error _Inst_##na(const _Op0_& o0) { return emit(_Code_##na, o0); } \
  ASMJIT_INLINE Error _Inst_##nae(const _Op0_& o0) { return emit(_Code_##nae, o0); } \
  ASMJIT_INLINE Error _Inst_##nb(const _Op0_& o0) { return emit(_Code_##nb, o0); } \
  ASMJIT_INLINE Error _Inst_##nbe(const _Op0_& o0) { return emit(_Code_##nbe, o0); } \
  ASMJIT_INLINE Error _Inst_##nc(const _Op0_& o0) { return emit(_Code_##nc, o0); } \
  ASMJIT_INLINE Error _Inst_##ne(const _Op0_& o0) { return emit(_Code_##ne, o0); } \
  ASMJIT_INLINE Error _Inst_##ng(const _Op0_& o0) { return emit(_Code_##ng, o0); } \
  ASMJIT_INLINE Error _Inst_##nge(const _Op0_& o0) { return emit(_Code_##nge, o0); } \
  ASMJIT_INLINE Error _Inst_##nl(const _Op0_& o0) { return emit(_Code_##nl, o0); } \
  ASMJIT_INLINE Error _Inst_##nle(const _Op0_& o0) { return emit(_Code_##nle, o0); } \
  ASMJIT_INLINE Error _Inst_##no(const _Op0_& o0) { return emit(_Code_##no, o0); } \
  ASMJIT_INLINE Error _Inst_##np(const _Op0_& o0) { return emit(_Code_##np, o0); } \
  ASMJIT_INLINE Error _Inst_##ns(const _Op0_& o0) { return emit(_Code_##ns, o0); } \
  ASMJIT_INLINE Error _Inst_##nz(const _Op0_& o0) { return emit(_Code_##nz, o0); } \
  ASMJIT_INLINE Error _Inst_##o(const _Op0_& o0) { return emit(_Code_##o, o0); } \
  ASMJIT_INLINE Error _Inst_##p(const _Op0_& o0) { return emit(_Code_##p, o0); } \
  ASMJIT_INLINE Error _Inst_##pe(const _Op0_& o0) { return emit(_Code_##pe, o0); } \
  ASMJIT_INLINE Error _Inst_##po(const _Op0_& o0) { return emit(_Code_##po, o0); } \
  ASMJIT_INLINE Error _Inst_##s(const _Op0_& o0) { return emit(_Code_##s, o0); } \
  ASMJIT_INLINE Error _Inst_##z(const _Op0_& o0) { return emit(_Code_##z, o0); }

#define INST_2x(_Inst_, _Code_, _Op0_, _Op1_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    return emit(_Code_, o0, o1); \
  }

#define INST_2x_(_Inst_, _Code_, _Op0_, _Op1_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1); \
  }

#define INST_2i(_Inst_, _Code_, _Op0_, _Op1_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    return emit(_Code_, o0, o1); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, int o1) { \
    return emit(_Code_, o0, o1); \
  }

#define INST_2i_(_Inst_, _Code_, _Op0_, _Op1_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, int o1) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1); \
  }

#define INST_2cc(_Inst_, _Code_, _Translate_, _Op0_, _Op1_) \
  ASMJIT_INLINE Error _Inst_(uint32_t cc, const _Op0_& o0, const _Op1_& o1) { \
    return emit(_Translate_(cc), o0, o1); \
  } \
  \
  ASMJIT_INLINE Error _Inst_##a(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##a, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##ae(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ae, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##b(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##b, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##be(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##be, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##c(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##c, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##e(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##e, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##g(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##g, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##ge(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ge, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##l(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##l, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##le(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##le, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##na(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##na, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nae(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nae, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nb(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nb, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nbe(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nbe, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nc(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nc, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##ne(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ne, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##ng(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ng, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nge(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nge, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nl(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nl, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nle(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nle, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##no(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##no, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##np(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##np, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##ns(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##ns, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##nz(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##nz, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##o(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##o, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##p(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##p, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##pe(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##pe, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##po(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##po, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##s(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##s, o0, o1); } \
  ASMJIT_INLINE Error _Inst_##z(const _Op0_& o0, const _Op1_& o1) { return emit(_Code_##z, o0, o1); }

#define INST_3x(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3x_(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3i(_Inst_, _Code_, _Op0_, _Op1_, _Op2_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    return emit(_Code_, o0, o1, o2); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, int o2) { \
    return emit(_Code_, o0, o1, o2); \
  }

#define INST_3i_(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, int o2) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2); \
  }


#define INST_4x(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) { \
    return emit(_Code_, o0, o1, o2, o3); \
  }

#define INST_4x_(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2, o3); \
  }

#define INST_4i(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) { \
    return emit(_Code_, o0, o1, o2, o3); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, int o3) { \
    return emit(_Code_, o0, o1, o2, o3); \
  }

#define INST_4i_(_Inst_, _Code_, _Op0_, _Op1_, _Op2_, _Op3_, _Cond_) \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, const _Op3_& o3) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2, o3); \
  } \
  \
  /* @overload */ \
  ASMJIT_INLINE Error _Inst_(const _Op0_& o0, const _Op1_& o1, const _Op2_& o2, int o3) { \
    ASMJIT_ASSERT(_Cond_); \
    return emit(_Code_, o0, o1, o2, o3); \
  }

// ============================================================================
// [asmjit::x86x64::X86X64Assembler]
// ============================================================================

//! @brief X86/X64 assembler.
//!
//! @ref Assembler is the main class in AsmJit that can encode instructions
//! and their operands to a binary stream runnable by CPU. It creates internal
//! buffer where the encodes instructions are stored and it contains intrinsics
//! that can be used to emit the code in a convenent way. Code generation is in
//! general safe, because the intrinsics uses method overloading so even the
//! code is emitted it can be checked by a C++ compiler. It's nearly impossible
//! to create invalid instruction (for example <code>mov [eax], [eax]</code>,
//! because such overload doesn't exist.
//!
//! Each call to an assembler intrinsic function emits instruction directly
//! to the binary stream. There are also runtime checks that prevent invalid
//! code to be emitted. It will assert in debug mode and put the @ref Assembler
//! instance to an  error state in production mode.
//!
//! @section AsmJit_Assembler_CodeGeneration Code Generation
//!
//! To generate code is only needed to create instance of @c asmjit::Assembler
//! and to use intrinsics. See example how to do that:
//!
//! @code
//! // Use asmjit namespace.
//! using namespace asmjit;
//! using namespace asmjit::host;
//!
//! // Create Assembler instance.
//! Assembler a;
//!
//! // Prolog.
//! a.push(ebp);
//! a.mov(ebp, esp);
//!
//! // Mov 1024 to EAX, EAX is also return value.
//! a.mov(eax, 1024);
//!
//! // Epilog.
//! a.mov(esp, ebp);
//! a.pop(ebp);
//!
//! // Return.
//! a.ret();
//! @endcode
//!
//! You can see that syntax is very close to Intel one. Only difference is that
//! you are calling functions that emits the binary code for you. All registers
//! are in @c asmjit namespace, so it's very comfortable to use it (look at
//! first line). There is also used method @c asmjit::imm() to create an
//! immediate value. Use @c asmjit::imm_u() to create unsigned immediate value.
//!
//! There is also possibility to use memory addresses and immediates. To build
//! memory address use @c ptr(), @c byte_ptr(), @c word_ptr(), @c dword_ptr()
//! or other friend methods. In most cases you needs only @c ptr() method, but
//! there are instructions where you must specify address size,
//!
//! for example (a is @c asmjit::Assembler instance):
//!
//! @code
//! a.mov(ptr(eax), 0);             // mov ptr [eax], 0
//! a.mov(ptr(eax), edx);           // mov ptr [eax], edx
//! @endcode
//!
//! But it's also possible to create complex addresses:
//!
//! @code
//! // eax + ecx*x addresses
//! a.mov(ptr(eax, ecx, 0), 0);     // mov ptr [eax + ecx], 0
//! a.mov(ptr(eax, ecx, 1), 0);     // mov ptr [eax + ecx * 2], 0
//! a.mov(ptr(eax, ecx, 2), 0);     // mov ptr [eax + ecx * 4], 0
//! a.mov(ptr(eax, ecx, 3), 0);     // mov ptr [eax + ecx * 8], 0
//! // eax + ecx*x + disp addresses
//! a.mov(ptr(eax, ecx, 0,  4), 0); // mov ptr [eax + ecx     +  4], 0
//! a.mov(ptr(eax, ecx, 1,  8), 0); // mov ptr [eax + ecx * 2 +  8], 0
//! a.mov(ptr(eax, ecx, 2, 12), 0); // mov ptr [eax + ecx * 4 + 12], 0
//! a.mov(ptr(eax, ecx, 3, 16), 0); // mov ptr [eax + ecx * 8 + 16], 0
//! @endcode
//!
//! All addresses shown are using @c asmjit::ptr() to make memory operand.
//! Some assembler instructions (single operand ones) needs to have specified
//! memory operand size. For example <code>a.inc(ptr(eax))</code> can't be
//! called. @c asmjit::Assembler::inc(), @c asmjit::Assembler::dec() and similar
//! instructions can't be encoded without specifying the operand size. See
//! next code how the assembler works:
//!
//! @code
//! // [byte] address
//! a.inc(byte_ptr(eax));           // inc byte ptr [eax]
//! a.dec(byte_ptr(eax));           // dec byte ptr [eax]
//! // [word] address
//! a.inc(word_ptr(eax));           // inc word ptr [eax]
//! a.dec(word_ptr(eax));           // dec word ptr [eax]
//! // [dword] address
//! a.inc(dword_ptr(eax));          // inc dword ptr [eax]
//! a.dec(dword_ptr(eax));          // dec dword ptr [eax]
//! @endcode
//!
//! @section AsmJit_Assembler_CallingJitCode Calling JIT Code
//!
//! While you are over from emitting instructions, you can make your function
//! using @c asmjit::Assembler::make() method. This method will use memory
//! manager to allocate virtual memory and relocates generated code to it. For
//! memory allocation is used global memory manager by default and memory is
//! freeable, but of course this default behavior can be overridden specifying
//! your memory manager and allocation type. If you want to do with code
//! something else you can always override make() method and do what you want.
//!
//! You can get size of generated code by @c getCodeSize() or @c getOffset()
//! methods. These methods returns you code size (or more precisely current code
//! offset) in bytes. Use takeCode() to take internal buffer (all pointers in
//! @c asmjit::Assembler instance will be zeroed and current buffer returned)
//! to use it. If you don't take it,  @c asmjit::Assembler destructor will
//! free it automatically. To alloc and run code manually don't use
//! @c malloc()'ed memory, but instead use @c asmjit::VMem::alloc() to get memory
//! for executing (specify @c canExecute to @c true) or @c asmjit::MemoryManager
//! that provides more effective and comfortable way to allocate virtual memory.
//!
//! See next example how to allocate memory where you can execute code created
//! by @c asmjit::Assembler:
//!
//! @code
//! using namespace asmjit;
//!
//! JitRuntime runtime;
//! Assembler a(&runtime);
//!
//! // ... Your code generation ...
//!
//! // The function prototype
//! typedef void (*MyFunc)();
//!
//! // make your function
//! MyFunc func = asmjit_cast<MyFunc>(a.make());
//!
//! // call your function
//! func();
//!
//! // If you don't need your function again, free it.
//! runtime.free(func);
//! @endcode
//!
//! @c note This was very primitive example how to call generated code.
//! In production code you will never do alloc/free for one run, you will
//! probably store the allocated function and free it when the application
//! ends or when JIT objects does cleanup.
//!
//! @section AsmJit_Assembler_Labels Labels
//!
//! While generating assembler code, you will usually need to create complex
//! code with labels. Labels are fully supported and you can call @c jmp or
//! @c je (and similar) instructions to initialized or yet uninitialized label.
//! Each label expects to be bound into offset. To bind label to specific
//! offset, use @c bind() method.
//!
//! See next example that contains complete code that creates simple memory
//! copy function (in DWord entities).
//!
//! @code
//! // Example: Usage of Label (32-bit code).
//! //
//! // Create simple DWord memory copy function:
//! // ASMJIT_STDCALL void copy32(uint32_t* dst, const uint32_t* src, size_t count);
//! using namespace asmjit;
//!
//! // Assembler instance.
//! JitRuntime runtime;
//! Assembler a(&runtime);
//!
//! // Constants.
//! const int arg_offset = 8; // Arguments offset (STDCALL EBP).
//! const int arg_size = 12;  // Arguments size.
//!
//! // Labels.
//! Label L_Loop(a);
//!
//! // Prolog.
//! a.push(ebp);
//! a.mov(ebp, esp);
//! a.push(esi);
//! a.push(edi);
//!
//! // Fetch arguments
//! a.mov(esi, dword_ptr(ebp, arg_offset + 0)); // Get dst.
//! a.mov(edi, dword_ptr(ebp, arg_offset + 4)); // Get src.
//! a.mov(ecx, dword_ptr(ebp, arg_offset + 8)); // Get count.
//!
//! // Bind L_Loop label to here.
//! a.bind(L_Loop);
//!
//! Copy 4 bytes.
//! a.mov(eax, dword_ptr(esi));
//! a.mov(dword_ptr(edi), eax);
//!
//! // Increment pointers.
//! a.add(esi, 4);
//! a.add(edi, 4);
//!
//! // Repeat loop until (--ecx != 0).
//! a.dec(ecx);
//! a.jz(L_Loop);
//!
//! // Epilog.
//! a.pop(edi);
//! a.pop(esi);
//! a.mov(esp, ebp);
//! a.pop(ebp);
//!
//! // Return: STDCALL convention is to pop stack in called function.
//! a.ret(arg_size);
//! @endcode
//!
//! If you need more abstraction for generating assembler code and you want
//! to hide calling conventions between 32-bit and 64-bit operating systems,
//! look at @c Compiler class that is designed for higher level code
//! generation.
//!
//! @section AsmJit_Assembler_AdvancedCodeGeneration Advanced Code Generation
//!
//! This section describes some advanced generation features of @c Assembler
//! class which can be simply overlooked. The first thing that is very likely
//! needed is generic register support. In previous example the named registers
//! were used. AsmJit contains functions which can convert register index into
//! operand and back.
//!
//! Let's define function which can be used to generate some abstract code:
//!
//! @code
//! // Simple function that generates dword copy.
//! void genCopyDWord(BaseAssembler& a, const GpReg& dst, const GpReg& src, const GpReg& tmp)
//! {
//!   a.mov(tmp, dword_ptr(src));
//!   a.mov(dword_ptr(dst), tmp);
//! }
//! @endcode
//!
//! This function can be called like <code>genCopyDWord(a, edi, esi, ebx)</code>
//! or by using existing @ref GpReg instances. This abstraction allows to join
//! more code sections together without rewriting each to use specific registers.
//! You need to take care only about implicit registers which may be used by
//! several instructions (like mul, imul, div, idiv, shifting, etc...).
//!
//! Next, more advanced, but often needed technique is that you can build your
//! own registers allocator. X86 architecture contains 8 general purpose registers,
//! 8 Mm registers and 8 Xmm registers. The X64 (AMD64) architecture extends count
//! of Gp registers and Xmm registers to 16. Use the @c kRegCountBase constant to
//! get count of Gp or Xmm registers or @c kRegCountGp, @c kRegCountMm and @c
//! kRegCountXmm constants individually.
//!
//! To build register from index (value from 0 inclusive to kRegNumXXX
//! exclusive) use @ref gpd(), @ref gpq() or @ref gpz() functions. To create
//! a 8 or 16-bit register use @ref gpw(), @ref gpb_lo() or @ref gpb_hi().
//! To create other registers there are similar methods like @ref mm(), @ref xmm()
//! and @ref fp().
//!
//! So our function call to genCopyDWord can be also used like this:
//!
//! @code
//! genCopyDWord(a, gpd(kRegIndexDi), gpd(kRegIndexSi), gpd(kRegIndexBx));
//! @endcode
//!
//! kRegIndexXXX are constants defined by @ref kRegIndex enum. You can use your
//! own register allocator (or register slot manager) to alloc / free registers
//! so kRegIndexXXX values can be replaced by your variables (0 to kRegNumXXX-1).
//!
//! @sa @ref Compiler.
struct X86X64Assembler : public BaseAssembler {
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API X86X64Assembler(BaseRuntime* runtime);
  ASMJIT_API virtual ~X86X64Assembler();

  // --------------------------------------------------------------------------
  // [Label]
  // --------------------------------------------------------------------------

  //! @override.
  ASMJIT_API virtual void _bind(const Label& label);

  // --------------------------------------------------------------------------
  // [Embed]
  // --------------------------------------------------------------------------

  //! @brief Add 8-bit integer data to the instuction stream.
  ASMJIT_INLINE void db(uint8_t x) { embed(&x, 1); }
  //! @brief Add 16-bit integer data to the instuction stream.
  ASMJIT_INLINE void dw(uint16_t x) { embed(&x, 2); }
  //! @brief Add 32-bit integer data to the instuction stream.
  ASMJIT_INLINE void dd(uint32_t x) { embed(&x, 4); }
  //! @brief Add 64-bit integer data to the instuction stream.
  ASMJIT_INLINE void dq(uint64_t x) { embed(&x, 8); }

  //! @brief Add 8-bit integer data to the instuction stream.
  ASMJIT_INLINE void dint8(int8_t x) { embed(&x, sizeof(int8_t)); }
  //! @brief Add 8-bit integer data to the instuction stream.
  ASMJIT_INLINE void duint8(uint8_t x) { embed(&x, sizeof(uint8_t)); }

  //! @brief Add 16-bit integer data to the instuction stream.
  ASMJIT_INLINE void dint16(int16_t x) { embed(&x, sizeof(int16_t)); }
  //! @brief Add 16-bit integer data to the instuction stream.
  ASMJIT_INLINE void duint16(uint16_t x) { embed(&x, sizeof(uint16_t)); }

  //! @brief Add 32-bit integer data to the instuction stream.
  ASMJIT_INLINE void dint32(int32_t x) { embed(&x, sizeof(int32_t)); }
  //! @brief Add 32-bit integer data to the instuction stream.
  ASMJIT_INLINE void duint32(uint32_t x) { embed(&x, sizeof(uint32_t)); }

  //! @brief Add 64-bit integer data to the instuction stream.
  ASMJIT_INLINE void dint64(int64_t x) { embed(&x, sizeof(int64_t)); }
  //! @brief Add 64-bit integer data to the instuction stream.
  ASMJIT_INLINE void duint64(uint64_t x) { embed(&x, sizeof(uint64_t)); }

  //! @brief Add float data to the instuction stream.
  ASMJIT_INLINE void dfloat(float x) { embed(&x, sizeof(float)); }
  //! @brief Add double data to the instuction stream.
  ASMJIT_INLINE void ddouble(double x) { embed(&x, sizeof(double)); }

  //! @brief Add pointer data to the instuction stream.
  ASMJIT_INLINE void dptr(void* x) { embed(&x, sizeof(void*)); }

  //! @brief Add Mm data to the instuction stream.
  ASMJIT_INLINE void dmm(const MmData& x) { embed(&x, sizeof(MmData)); }
  //! @brief Add Xmm data to the instuction stream.
  ASMJIT_INLINE void dxmm(const XmmData& x) { embed(&x, sizeof(XmmData)); }

  //! @brief Add data in a given structure instance to the instuction stream.
  template<typename T>
  ASMJIT_INLINE void dstruct(const T& x) { embed(&x, static_cast<uint32_t>(sizeof(T))); }

  //! @brief Embed absolute label pointer (4 or 8 bytes).
  ASMJIT_API Error embedLabel(const Label& op);

  // --------------------------------------------------------------------------
  // [Align]
  // --------------------------------------------------------------------------

  //! @brief Align target buffer to @a m bytes.
  //!
  //! Typical usage of this is to align labels at start of the inner loops.
  //!
  //! Inserts @c nop() instructions or CPU optimized NOPs.
  ASMJIT_API virtual Error _align(uint32_t m);

  // -------------------------------------------------------------------------
  // [Options]
  // -------------------------------------------------------------------------

  //! @brief Force short form of jmp/jcc/other instruction.
  ASMJIT_INLINE X86X64Assembler& short_()
  { _options |= kInstOptionShortForm; return *this; }

  //! @brief Force long form of jmp/jcc/other instruction.
  ASMJIT_INLINE X86X64Assembler& long_()
  { _options |= kInstOptionLongForm; return *this; }

  //! @brief Condition is likely to be taken.
  ASMJIT_INLINE X86X64Assembler& taken()
  { _options |= kInstOptionTaken; return *this; }

  //! @brief Condition is unlikely to be taken.
  ASMJIT_INLINE X86X64Assembler& notTaken()
  { _options |= kInstOptionNotTaken; return *this; }

  //! @brief Lock prefix.
  ASMJIT_INLINE X86X64Assembler& lock()
  { _options |= kInstOptionLock; return *this; }

  // --------------------------------------------------------------------------
  // [Base Instructions]
  // --------------------------------------------------------------------------

  //! @brief Add with Carry.
  INST_2x(adc, kInstAdc, GpReg, GpReg)
  //! @overload
  INST_2x(adc, kInstAdc, GpReg, Mem)
  //! @overload
  INST_2i(adc, kInstAdc, GpReg, Imm)
  //! @overload
  INST_2x(adc, kInstAdc, Mem, GpReg)
  //! @overload
  INST_2i(adc, kInstAdc, Mem, Imm)

  //! @brief Add.
  INST_2x(add, kInstAdd, GpReg, GpReg)
  //! @overload
  INST_2x(add, kInstAdd, GpReg, Mem)
  //! @overload
  INST_2i(add, kInstAdd, GpReg, Imm)
  //! @overload
  INST_2x(add, kInstAdd, Mem, GpReg)
  //! @overload
  INST_2i(add, kInstAdd, Mem, Imm)

  //! @brief And.
  INST_2x(and_, kInstAnd, GpReg, GpReg)
  //! @overload
  INST_2x(and_, kInstAnd, GpReg, Mem)
  //! @overload
  INST_2i(and_, kInstAnd, GpReg, Imm)
  //! @overload
  INST_2x(and_, kInstAnd, Mem, GpReg)
  //! @overload
  INST_2i(and_, kInstAnd, Mem, Imm)

  //! @brief Bit scan forward.
  INST_2x_(bsf, kInstBsf, GpReg, GpReg, !o0.isGpb())
  //! @overload
  INST_2x_(bsf, kInstBsf, GpReg, Mem, !o0.isGpb())

  //! @brief Bit scan reverse.
  INST_2x_(bsr, kInstBsr, GpReg, GpReg, !o0.isGpb())
  //! @overload
  INST_2x_(bsr, kInstBsr, GpReg, Mem, !o0.isGpb())

  //! @brief Byte swap (32-bit or 64-bit registers only) (i486).
  INST_1x_(bswap, kInstBswap, GpReg, o0.getSize() >= 4)

  //! @brief Bit test.
  INST_2x(bt, kInstBt, GpReg, GpReg)
  //! @overload
  INST_2i(bt, kInstBt, GpReg, Imm)
  //! @overload
  INST_2x(bt, kInstBt, Mem, GpReg)
  //! @overload
  INST_2i(bt, kInstBt, Mem, Imm)

  //! @brief Bit test and complement.
  INST_2x(btc, kInstBtc, GpReg, GpReg)
  //! @overload
  INST_2i(btc, kInstBtc, GpReg, Imm)
  //! @overload
  INST_2x(btc, kInstBtc, Mem, GpReg)
  //! @overload
  INST_2i(btc, kInstBtc, Mem, Imm)

  //! @brief Bit test and reset.
  INST_2x(btr, kInstBtr, GpReg, GpReg)
  //! @overload
  INST_2i(btr, kInstBtr, GpReg, Imm)
  //! @overload
  INST_2x(btr, kInstBtr, Mem, GpReg)
  //! @overload
  INST_2i(btr, kInstBtr, Mem, Imm)

  //! @brief Bit test and set.
  INST_2x(bts, kInstBts, GpReg, GpReg)
  //! @overload
  INST_2i(bts, kInstBts, GpReg, Imm)
  //! @overload
  INST_2x(bts, kInstBts, Mem, GpReg)
  //! @overload
  INST_2i(bts, kInstBts, Mem, Imm)

  //! @brief Call.
  INST_1x(call, kInstCall, GpReg)
  //! @overload
  INST_1x(call, kInstCall, Mem)
  //! @overload
  INST_1x(call, kInstCall, Label)
  //! @overload
  INST_1x(call, kInstCall, Imm)
  //! @overload
  ASMJIT_INLINE Error call(void* dst) { return call(Imm((intptr_t)dst)); }

  //! @brief Clear carry flag.
  INST_0x(clc, kInstClc)
  //! @brief Clear direction flag.
  INST_0x(cld, kInstCld)
  //! @brief Complement carry flag.
  INST_0x(cmc, kInstCmc)

  //! @brief Convert byte to word (AX <- Sign Extend AL).
  INST_0x(cbw, kInstCbw)
  //! @brief Convert word to dword (DX:AX <- Sign Extend AX).
  INST_0x(cwd, kInstCwd)
  //! @brief Convert word to dword (EAX <- Sign Extend AX).
  INST_0x(cwde, kInstCwde)
  //! @brief Convert dword to qword (EDX:EAX <- Sign Extend EAX).
  INST_0x(cdq, kInstCdq)

  //! @brief Conditional move.
  INST_2cc(cmov, kInstCmov, condToCmovcc, GpReg, GpReg)
  //! @brief Conditional move.
  INST_2cc(cmov, kInstCmov, condToCmovcc, GpReg, Mem)

  //! @brief Compare two operands.
  INST_2x(cmp, kInstCmp, GpReg, GpReg)
  //! @overload
  INST_2x(cmp, kInstCmp, GpReg, Mem)
  //! @overload
  INST_2i(cmp, kInstCmp, GpReg, Imm)
  //! @overload
  INST_2x(cmp, kInstCmp, Mem, GpReg)
  //! @overload
  INST_2i(cmp, kInstCmp, Mem, Imm)

  //! @brief Compare and exchange (i486).
  INST_2x(cmpxchg, kInstCmpxchg, GpReg, GpReg)
  //! @overload
  INST_2x(cmpxchg, kInstCmpxchg, Mem, GpReg)

  //! @brief Compares the 64-bit value in EDX:EAX with the memory operand (Pentium).
  INST_1x(cmpxchg8b, kInstCmpxchg8b, Mem)

  //! @brief CPU identification (i486).
  INST_0x(cpuid, kInstCpuid)

  //! @brief Accumulate crc32 value (polynomial 0x11EDC6F41) (SSE4.2).
  INST_2x_(crc32, kInstCrc32, GpReg, GpReg, o0.isRegType(kRegTypeGpd) || o0.isRegType(kRegTypeGpq))
  //! @overload
  INST_2x_(crc32, kInstCrc32, GpReg, Mem, o0.isRegType(kRegTypeGpd) || o0.isRegType(kRegTypeGpq))

  //! @brief Decrement by 1.
  INST_1x(dec, kInstDec, GpReg)
  //! @overload
  INST_1x(dec, kInstDec, Mem)

  //! @brief Unsigned divide (xDX:xAX <- xDX:xAX / o0).
  INST_1x(div, kInstDiv, GpReg)
  //! @overload
  INST_1x(div, kInstDiv, Mem)

  //! @brief Make stack frame for procedure parameters.
  INST_2x(enter, kInstEnter, Imm, Imm)

  //! @brief Signed divide (xDX:xAX <- xDX:xAX / op).
  INST_1x(idiv, kInstIdiv, GpReg)
  //! @overload
  INST_1x(idiv, kInstIdiv, Mem)

  //! @brief Signed multiply (xDX:xAX <- xAX * o0).
  INST_1x(imul, kInstImul, GpReg)
  //! @overload
  INST_1x(imul, kInstImul, Mem)

  //! @brief Signed multiply.
  INST_2x(imul, kInstImul, GpReg, GpReg)
  //! @overload
  INST_2x(imul, kInstImul, GpReg, Mem)
  //! @overload
  INST_2i(imul, kInstImul, GpReg, Imm)

  //! @brief Signed multiply.
  INST_3i(imul, kInstImul, GpReg, GpReg, Imm)
  //! @overload
  INST_3i(imul, kInstImul, GpReg, Mem, Imm)

  //! @brief Increment by 1.
  INST_1x(inc, kInstInc, GpReg)
  //! @overload
  INST_1x(inc, kInstInc, Mem)

  //! @brief Interrupt.
  INST_1i(int_, kInstInt, Imm)
  //! @brief Interrupt 3 - trap to debugger.
  ASMJIT_INLINE Error int3() { return int_(3); }

  //! @brief Jump to label @a label if condition @a cc is met.
  INST_1cc(j, kInstJ, condToJcc, Label)

  //! @brief Jump.
  INST_1x(jmp, kInstJmp, GpReg)
  //! @overload
  INST_1x(jmp, kInstJmp, Mem)
  //! @overload.
  INST_1x(jmp, kInstJmp, Label)
  //! @overload
  INST_1x(jmp, kInstJmp, Imm)
  //! @overload
  ASMJIT_INLINE Error jmp(void* dst) { return jmp(Imm((intptr_t)dst)); }

  //! @brief Load AH from flags.
  INST_0x(lahf, kInstLahf)

  //! @brief Load effective address
  INST_2x(lea, kInstLea, GpReg, Mem)

  //! @brief High level procedure exit.
  INST_0x(leave, kInstLeave)

  //! @brief Move.
  INST_2x(mov, kInstMov, GpReg, GpReg)
  //! @overload
  INST_2x(mov, kInstMov, GpReg, Mem)
  //! @overload
  INST_2i(mov, kInstMov, GpReg, Imm)
  //! @overload
  INST_2x(mov, kInstMov, Mem, GpReg)
  //! @overload
  INST_2i(mov, kInstMov, Mem, Imm)

  //! @brief Move from segment register.
  INST_2x(mov, kInstMov, GpReg, SegReg)
  //! @overload
  INST_2x(mov, kInstMov, Mem, SegReg)
  //! @brief Move to segment register.
  INST_2x(mov, kInstMov, SegReg, GpReg)
  //! @overload
  INST_2x(mov, kInstMov, SegReg, Mem)

  //! @brief Move (AL|AX|EAX|RAX <- absolute address in immediate).
  ASMJIT_INLINE Error mov_ptr(const GpReg& dst, void* src) {
    ASMJIT_ASSERT(dst.getRegIndex() == 0);

    Imm imm(static_cast<int64_t>((intptr_t)src));
    return emit(kInstMovptr, dst, imm);
  }

  //! @brief Move (absolute address in immediate <- AL|AX|EAX|RAX).
  ASMJIT_INLINE Error mov_ptr(void* dst, const GpReg& src) {
    ASMJIT_ASSERT(src.getRegIndex() == 0);

    Imm imm(static_cast<int64_t>((intptr_t)dst));
    return emit(kInstMovptr, imm, src);
  }

  //! @brief Move data after dwapping bytes (SSE3 - Atom).
  INST_2x_(movbe, kInstMovbe, GpReg, Mem, !o0.isGpb());
  //! @overload
  INST_2x_(movbe, kInstMovbe, Mem, GpReg, !o1.isGpb());

  //! @brief Move with sign-extension.
  INST_2x(movsx, kInstMovsx, GpReg, GpReg)
  //! @overload
  INST_2x(movsx, kInstMovsx, GpReg, Mem)

  //! @brief Move with zero-extension.
  INST_2x(movzx, kInstMovzx, GpReg, GpReg)
  //! @overload
  INST_2x(movzx, kInstMovzx, GpReg, Mem)

  //! @brief Unsigned multiply (xDX:xAX <- xAX * o0).
  INST_1x(mul, kInstMul, GpReg)
  //! @overload
  INST_1x(mul, kInstMul, Mem)

  //! @brief Two's complement negation.
  INST_1x(neg, kInstNeg, GpReg)
  //! @overload
  INST_1x(neg, kInstNeg, Mem)

  //! @brief No operation.
  INST_0x(nop, kInstNop)

  //! @brief One's complement negation.
  INST_1x(not_, kInstNot, GpReg)
  //! @overload
  INST_1x(not_, kInstNot, Mem)

  //! @brief Or.
  INST_2x(or_, kInstOr, GpReg, GpReg)
  //! @overload
  INST_2x(or_, kInstOr, GpReg, Mem)
  //! @overload
  INST_2i(or_, kInstOr, GpReg, Imm)
  //! @overload
  INST_2x(or_, kInstOr, Mem, GpReg)
  //! @overload
  INST_2i(or_, kInstOr, Mem, Imm)

  //! @brief Pop a value from the stack.
  INST_1x_(pop, kInstPop, GpReg, o0.getSize() == 2 || o0.getSize() == _regSize)
  //! @overload
  INST_1x_(pop, kInstPop, Mem, o0.getSize() == 2 || o0.getSize() == _regSize)

  //! @brief Pop a segment register from the stack.
  //!
  //! @note There is no instruction to pop a cs segment register.
  INST_1x_(pop, kInstPop, SegReg, o0.getRegIndex() != kSegCs);

  //! @brief Pop stack into EFLAGS register (32-bit or 64-bit).
  INST_0x(popf, kInstPopf)

  //! @brief Return the count of number of bits set to 1 (SSE4.2).
  INST_2x_(popcnt, kInstPopcnt, GpReg, GpReg, !o0.isGpb() && o0.getRegType() == o1.getRegType())
  //! @overload
  INST_2x_(popcnt, kInstPopcnt, GpReg, Mem, !o0.isGpb())

  //! @brief Push word/dword/qword on the stack.
  INST_1x_(push, kInstPush, GpReg, o0.getSize() == 2 || o0.getSize() == _regSize)
  //! @brief Push word/dword/qword on the stack.
  INST_1x_(push, kInstPush, Mem, o0.getSize() == 2 || o0.getSize() == _regSize)
  //! @brief Push segment register on the stack.
  INST_1x(push, kInstPush, SegReg)
  //! @brief Push word/dword/qword on the stack.
  INST_1i(push, kInstPush, Imm)

  //! @brief Push EFLAGS register (32-bit or 64-bit) on the stack.
  INST_0x(pushf, kInstPushf)

  //! @brief Rotate bits left.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(rcl, kInstRcl, GpReg, GpReg)
  //! @overload
  INST_2x(rcl, kInstRcl, Mem, GpReg)
  //! @brief Rotate bits left.
  INST_2i(rcl, kInstRcl, GpReg, Imm)
  //! @overload
  INST_2i(rcl, kInstRcl, Mem, Imm)

  //! @brief Rotate bits right.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(rcr, kInstRcr, GpReg, GpReg)
  //! @overload
  INST_2x(rcr, kInstRcr, Mem, GpReg)
  //! @brief Rotate bits right.
  INST_2i(rcr, kInstRcr, GpReg, Imm)
  //! @overload
  INST_2i(rcr, kInstRcr, Mem, Imm)

  //! @brief Read time-stamp counter (Pentium).
  INST_0x(rdtsc, kInstRdtsc)
  //! @brief Read time-stamp counter and processor id (Pentium).
  INST_0x(rdtscp, kInstRdtscp)

  //! @brief Load ECX/RCX bytes from DS:[ESI/RSI] to AL.
  INST_0x(rep_lodsb, kInstRepLodsb)
  //! @brief Load ECX/RCX dwords from DS:[ESI/RSI] to EAX.
  INST_0x(rep_lodsd, kInstRepLodsd)
  //! @brief Load ECX/RCX Words from DS:[ESI/RSI] to AX.
  INST_0x(rep_lodsw, kInstRepLodsw)

  //! @brief Move ECX/RCX bytes from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_0x(rep_movsb, kInstRepMovsb)
  //! @brief Move ECX/RCX dwords from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_0x(rep_movsd, kInstRepMovsd)
  //! @brief Move ECX/RCX words from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_0x(rep_movsw, kInstRepMovsw)

  //! @brief Fill ECX/RCX bytes at ES:[EDI/RDI] with AL.
  INST_0x(rep_stosb, kInstRepStosb)
  //! @brief Fill ECX/RCX dwords at ES:[EDI/RDI] with EAX.
  INST_0x(rep_stosd, kInstRepStosd)
  //! @brief Fill ECX/RCX words at ES:[EDI/RDI] with AX.
  INST_0x(rep_stosw, kInstRepStosw)

  //! @brief Repeated find nonmatching bytes in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repe_cmpsb, kInstRepeCmpsb)
  //! @brief Repeated find nonmatching dwords in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repe_cmpsd, kInstRepeCmpsd)
  //! @brief Repeated find nonmatching words in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repe_cmpsw, kInstRepeCmpsw)

  //! @brief Find non-AL byte starting at ES:[EDI/RDI].
  INST_0x(repe_scasb, kInstRepeScasb)
  //! @brief Find non-EAX dword starting at ES:[EDI/RDI].
  INST_0x(repe_scasd, kInstRepeScasd)
  //! @brief Find non-AX word starting at ES:[EDI/RDI].
  INST_0x(repe_scasw, kInstRepeScasw)

  //! @brief Repeated find nonmatching bytes in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repne_cmpsb, kInstRepneCmpsb)
  //! @brief Repeated find nonmatching dwords in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repne_cmpsd, kInstRepneCmpsd)
  //! @brief Repeated find nonmatching words in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repne_cmpsw, kInstRepneCmpsw)

  //! @brief Find AL, starting at ES:[EDI/RDI].
  INST_0x(repne_scasb, kInstRepneScasb)
  //! @brief Find EAX, starting at ES:[EDI/RDI].
  INST_0x(repne_scasd, kInstRepneScasd)
  //! @brief Find AX, starting at ES:[EDI/RDI].
  INST_0x(repne_scasw, kInstRepneScasw)

  //! @brief Return.
  INST_0x(ret, kInstRet)
  //! @overload
  INST_1i(ret, kInstRet, Imm)

  //! @brief Rotate bits left.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(rol, kInstRol, GpReg, GpReg)
  //! @overload
  INST_2x(rol, kInstRol, Mem, GpReg)
  //! @brief Rotate bits left.
  INST_2i(rol, kInstRol, GpReg, Imm)
  //! @overload
  INST_2i(rol, kInstRol, Mem, Imm)

  //! @brief Rotate bits right.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(ror, kInstRor, GpReg, GpReg)
  //! @overload
  INST_2x(ror, kInstRor, Mem, GpReg)
  //! @brief Rotate bits right.
  INST_2i(ror, kInstRor, GpReg, Imm)
  //! @overload
  INST_2i(ror, kInstRor, Mem, Imm)

  //! @brief Store AH into flags.
  INST_0x(sahf, kInstSahf)

  //! @brief Integer subtraction with borrow.
  INST_2x(sbb, kInstSbb, GpReg, GpReg)
  //! @overload
  INST_2x(sbb, kInstSbb, GpReg, Mem)
  //! @overload
  INST_2i(sbb, kInstSbb, GpReg, Imm)
  //! @overload
  INST_2x(sbb, kInstSbb, Mem, GpReg)
  //! @overload
  INST_2i(sbb, kInstSbb, Mem, Imm)

  //! @brief Shift bits left.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(sal, kInstSal, GpReg, GpReg)
  //! @overload
  INST_2x(sal, kInstSal, Mem, GpReg)
  //! @brief Shift bits left.
  INST_2i(sal, kInstSal, GpReg, Imm)
  //! @overload
  INST_2i(sal, kInstSal, Mem, Imm)

  //! @brief Shift bits right.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(sar, kInstSar, GpReg, GpReg)
  //! @overload
  INST_2x(sar, kInstSar, Mem, GpReg)
  //! @brief Shift bits right.
  INST_2i(sar, kInstSar, GpReg, Imm)
  //! @overload
  INST_2i(sar, kInstSar, Mem, Imm)

  //! @brief Set byte on condition.
  INST_1cc(set, kInstSet, condToSetcc, GpReg)
  //! @brief Set byte on condition.
  INST_1cc(set, kInstSet, condToSetcc, Mem)

  //! @brief Shift bits left.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(shl, kInstShl, GpReg, GpReg)
  //! @overload
  INST_2x(shl, kInstShl, Mem, GpReg)
  //! @brief Shift bits left.
  INST_2i(shl, kInstShl, GpReg, Imm)
  //! @overload
  INST_2i(shl, kInstShl, Mem, Imm)

  //! @brief Shift bits right.
  //!
  //! @note @a o1 register can be only @c cl.
  INST_2x(shr, kInstShr, GpReg, GpReg)
  //! @overload
  INST_2x(shr, kInstShr, Mem, GpReg)
  //! @brief Shift bits right.
  INST_2i(shr, kInstShr, GpReg, Imm)
  //! @overload
  INST_2i(shr, kInstShr, Mem, Imm)

  //! @brief Double precision shift left.
  //!
  //! @note @a o2 register can be only @c cl register.
  INST_3x(shld, kInstShld, GpReg, GpReg, GpReg)
  //! @overload
  INST_3x(shld, kInstShld, Mem, GpReg, GpReg)
  //! @brief Double precision shift left.
  INST_3i(shld, kInstShld, GpReg, GpReg, Imm)
  //! @overload
  INST_3i(shld, kInstShld, Mem, GpReg, Imm)

  //! @brief Double precision shift right.
  //!
  //! @note @a o2 register can be only @c cl register.
  INST_3x(shrd, kInstShrd, GpReg, GpReg, GpReg)
  //! @overload
  INST_3x(shrd, kInstShrd, Mem, GpReg, GpReg)
  //! @brief Double precision shift right.
  INST_3i(shrd, kInstShrd, GpReg, GpReg, Imm)
  //! @overload
  INST_3i(shrd, kInstShrd, Mem, GpReg, Imm)

  //! @brief Set carry flag to 1.
  INST_0x(stc, kInstStc)
  //! @brief Set direction flag to 1.
  INST_0x(std, kInstStd)

  //! @brief Subtract.
  INST_2x(sub, kInstSub, GpReg, GpReg)
  //! @overload
  INST_2x(sub, kInstSub, GpReg, Mem)
  //! @overload
  INST_2i(sub, kInstSub, GpReg, Imm)
  //! @overload
  INST_2x(sub, kInstSub, Mem, GpReg)
  //! @overload
  INST_2i(sub, kInstSub, Mem, Imm)

  //! @brief Logical compare.
  INST_2x(test, kInstTest, GpReg, GpReg)
  //! @overload
  INST_2i(test, kInstTest, GpReg, Imm)
  //! @overload
  INST_2x(test, kInstTest, Mem, GpReg)
  //! @overload
  INST_2i(test, kInstTest, Mem, Imm)

  //! @brief Undefined instruction - Raise #UD exception.
  INST_0x(ud2, kInstUd2)

  //! @brief Exchange and Add.
  INST_2x(xadd, kInstXadd, GpReg, GpReg)
  //! @overload
  INST_2x(xadd, kInstXadd, Mem, GpReg)

  //! @brief Exchange register/memory with register.
  INST_2x(xchg, kInstXchg, GpReg, GpReg)
  //! @overload
  INST_2x(xchg, kInstXchg, Mem, GpReg)
  //! @overload
  INST_2x(xchg, kInstXchg, GpReg, Mem)

  //! @brief Xor.
  INST_2x(xor_, kInstXor, GpReg, GpReg)
  //! @overload
  INST_2x(xor_, kInstXor, GpReg, Mem)
  //! @overload
  INST_2i(xor_, kInstXor, GpReg, Imm)
  //! @overload
  INST_2x(xor_, kInstXor, Mem, GpReg)
  //! @overload
  INST_2i(xor_, kInstXor, Mem, Imm)

  // --------------------------------------------------------------------------
  // [Fpu]
  // --------------------------------------------------------------------------

  //! @brief Compute 2^x - 1 (FPU).
  INST_0x(f2xm1, kInstF2xm1)

  //! @brief Absolute value of fp0 (FPU).
  INST_0x(fabs, kInstFabs)

  //! @brief Add @a o1 to @a o0 and store result in @a o0 (FPU).
  //!
  //! @note One of dst or src must be fp0.
  INST_2x_(fadd, kInstFadd, FpReg, FpReg, o0.getRegIndex() == 0 || o1.getRegIndex() == 0)
  //! @brief Add 4-byte or 8-byte FP @a o0 to fp0 and store result in fp0 (FPU).
  INST_1x(fadd, kInstFadd, Mem)
  //! @brief Add fp0 to @a o0 and POP register stack (FPU).
  INST_1x(faddp, kInstFaddp, FpReg)
  //! @overload.
  INST_0x(faddp, kInstFaddp)

  //! @brief Load binary coded decimal (FPU).
  INST_1x(fbld, kInstFbld, Mem)
  //! @brief Store BCD integer and Pop (FPU).
  INST_1x(fbstp, kInstFbstp, Mem)
  //! @brief Change fp0 sign (FPU).
  INST_0x(fchs, kInstFchs)

  //! @brief Clear exceptions (FPU).
  INST_0x(fclex, kInstFclex)

  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovb, kInstFcmovb, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovbe, kInstFcmovbe, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmove, kInstFcmove, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovnb, kInstFcmovnb, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovnbe, kInstFcmovnbe, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovne, kInstFcmovne, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovnu, kInstFcmovnu, FpReg)
  //! @brief FP Conditional move (FPU).
  INST_1x(fcmovu, kInstFcmovu, FpReg)

  //! @brief Compare fp0 with @a o0 (FPU).
  INST_1x(fcom, kInstFcom, FpReg)
  //! @brief Compare fp0 with fp1 (FPU).
  INST_0x(fcom, kInstFcom)
  //! @brief Compare fp0 with 4-byte or 8-byte FP at @a src (FPU).
  INST_1x(fcom, kInstFcom, Mem)
  //! @brief Compare fp0 with @a o0 and pop the stack (FPU).
  INST_1x(fcomp, kInstFcomp, FpReg)
  //! @brief Compare fp0 with fp1 and pop the stack (FPU).
  INST_0x(fcomp, kInstFcomp)
  //! @brief Compare fp0 with 4-byte or 8-byte FP at @a adr and pop the stack (FPU).
  INST_1x(fcomp, kInstFcomp, Mem)
    //! @brief Compare fp0 with fp1 and pop register stack twice (FPU).
  INST_0x(fcompp, kInstFcompp)
  //! @brief Compare fp0 and @a o0 and Set EFLAGS (FPU).
  INST_1x(fcomi, kInstFcomi, FpReg)
  //! @brief Compare fp0 and @a o0 and Set EFLAGS and pop the stack (FPU).
  INST_1x(fcomip, kInstFcomip, FpReg)

  //! @brief Calculate cosine of fp0 and store result in fp0 (FPU).
  INST_0x(fcos, kInstFcos)

  //! @brief Decrement stack-top pointer (FPU).
  INST_0x(fdecstp, kInstFdecstp)

  //! @brief Divide @a o0 by @a o1 (FPU).
  //!
  //! @note One of @a o0 or @a o1 register must be fp0.
  INST_2x_(fdiv, kInstFdiv, FpReg, FpReg, o0.getRegIndex() == 0 || o1.getRegIndex() == 0)
  //! @brief Divide fp0 by 32-bit or 64-bit FP value (FPU).
  INST_1x(fdiv, kInstFdiv, Mem)
  //! @brief Divide @a o0 by fp0 (FPU).
  INST_1x(fdivp, kInstFdivp, FpReg)
  //! @overload.
  INST_0x(fdivp, kInstFdivp)

  //! @brief Reverse divide @a o0 by @a o1 (FPU).
  //!
  //! @note One of @a o0 or @a src register must be fp0.
  INST_2x_(fdivr, kInstFdivr, FpReg, FpReg, o0.getRegIndex() == 0 || o1.getRegIndex() == 0)
  //! @brief Reverse divide fp0 by 32-bit or 64-bit FP value (FPU).
  INST_1x(fdivr, kInstFdivr, Mem)
  //! @brief Reverse divide @a o0 by fp0 (FPU).
  INST_1x(fdivrp, kInstFdivrp, FpReg)
  //! @overload.
  INST_0x(fdivrp, kInstFdivrp)

  //! @brief Free FP register (FPU).
  //!
  //! Sets the tag in the FPU tag register associated with register @a o0
  //! to empty (11B). The contents of @a o0 and the FPU stack-top pointer
  //! (TOP) are not affected.
  INST_1x(ffree, kInstFfree, FpReg)

  //! @brief Add 16-bit or 32-bit integer to fp0 (FPU).
  INST_1x_(fiadd, kInstFiadd, Mem, o0.getSize() == 2 || o0.getSize() == 4)

  //! @brief Compare fp0 with 16-bit or 32-bit Integer (FPU).
  INST_1x_(ficom, kInstFicom, Mem, o0.getSize() == 2 || o0.getSize() == 4)
  //! @brief Compare fp0 with 16-bit or 32-bit Integer and pop the stack (FPU).
  INST_1x_(ficomp, kInstFicomp, Mem, o0.getSize() == 2 || o0.getSize() == 4)

  //! @brief Divide fp0 by 32-bit or 16-bit integer (@a src) (FPU).
  INST_1x_(fidiv, kInstFidiv, Mem, o0.getSize() == 2 || o0.getSize() == 4)
  //! @brief Reverse divide fp0 by 32-bit or 16-bit integer (@a src) (FPU).
  INST_1x_(fidivr, kInstFidivr, Mem, o0.getSize() == 2 || o0.getSize() == 4)

  //! @brief Load 16-bit, 32-bit or 64-bit Integer and push it to the stack (FPU).
  INST_1x_(fild, kInstFild, Mem, o0.getSize() == 2 || o0.getSize() == 4 || o0.getSize() == 8)

  //! @brief Multiply fp0 by 16-bit or 32-bit integer and store it to fp0 (FPU).
  INST_1x_(fimul, kInstFimul, Mem, o0.getSize() == 2 || o0.getSize() == 4)

  //! @brief Increment stack-top pointer (FPU).
  INST_0x(fincstp, kInstFincstp)

  //! @brief Initialize FPU (FPU).
  INST_0x(finit, kInstFinit)

  //! @brief Subtract 16-bit or 32-bit integer from fp0 and store result to fp0 (FPU).
  INST_1x_(fisub, kInstFisub, Mem, o0.getSize() == 2 || o0.getSize() == 4)
  //! @brief Reverse subtract 16-bit or 32-bit integer from fp0 and store result to fp0 (FPU).
  INST_1x_(fisubr, kInstFisubr, Mem, o0.getSize() == 2 || o0.getSize() == 4)

  //! @brief Initialize FPU without checking for pending unmasked exceptions (FPU).
  INST_0x(fninit, kInstFninit)

  //! @brief Store fp0 as 16-bit or 32-bit Integer to @a o0 (FPU).
  INST_1x_(fist, kInstFist, Mem, o0.getSize() == 2 || o0.getSize() == 4)
  //! @brief Store fp0 as 16-bit, 32-bit or 64-bit Integer to @a o0 and pop stack (FPU).
  INST_1x_(fistp, kInstFistp, Mem, o0.getSize() == 2 || o0.getSize() == 4 || o0.getSize() == 8)

  //! @brief Push 32-bit, 64-bit or 80-bit floating point value on the FPU register stack (FPU).
  INST_1x_(fld, kInstFld, Mem, o0.getSize() == 4 || o0.getSize() == 8 || o0.getSize() == 10)
  //! @brief Push @a o0 on the FPU register stack (FPU).
  INST_1x(fld, kInstFld, FpReg)

  //! @brief Push +1.0 on the FPU register stack (FPU).
  INST_0x(fld1, kInstFld1)
  //! @brief Push log2(10) on the FPU register stack (FPU).
  INST_0x(fldl2t, kInstFldl2t)
  //! @brief Push log2(e) on the FPU register stack (FPU).
  INST_0x(fldl2e, kInstFldl2e)
  //! @brief Push pi on the FPU register stack (FPU).
  INST_0x(fldpi, kInstFldpi)
  //! @brief Push log10(2) on the FPU register stack (FPU).
  INST_0x(fldlg2, kInstFldlg2)
  //! @brief Push ln(2) on the FPU register stack (FPU).
  INST_0x(fldln2, kInstFldln2)
  //! @brief Push +0.0 on the FPU register stack (FPU).
  INST_0x(fldz, kInstFldz)

  //! @brief Load x87 FPU control word (2 bytes) (FPU).
  INST_1x(fldcw, kInstFldcw, Mem)
  //! @brief Load x87 FPU environment (14 or 28 bytes) (FPU).
  INST_1x(fldenv, kInstFldenv, Mem)

  //! @brief Multiply @a o0 by @a o1 and store result in @a o0 (FPU).
  //!
  //! @note One of dst or src must be fp0.
  INST_2x_(fmul, kInstFmul, FpReg, FpReg, o0.getRegIndex() == 0 || o1.getRegIndex() == 0)
  //! @brief Multiply fp0 by 32-bit or 64-bit @a o0 and store result in fp0 (FPU).
  INST_1x(fmul, kInstFmul, Mem)

  //! @brief Multiply fp0 by @a o0 and POP register stack (FPU).
  INST_1x(fmulp, kInstFmulp, FpReg)
  //! @overload.
  INST_0x(fmulp, kInstFmulp)

  //! @brief Clear exceptions (FPU).
  INST_0x(fnclex, kInstFnclex)

  //! @brief No operation (FPU).
  INST_0x(fnop, kInstFnop)

  //! @brief Save FPU state (FPU).
  INST_1x(fnsave, kInstFnsave, Mem)

  //! @brief Store x87 FPU environment (FPU).
  INST_1x(fnstenv, kInstFnstenv, Mem)

  //! @brief Store x87 FPU control word (FPU).
  INST_1x(fnstcw, kInstFnstcw, Mem)

  //! @brief Store x87 FPU status word to @a o0 (AX) (FPU).
  INST_1x_(fnstsw, kInstFnstsw, GpReg, o0.isRegCode(kRegTypeGpw, kRegIndexAx))
  //! @brief Store x87 FPU status word to @a o0 (2 bytes) (FPU).
  INST_1x(fnstsw, kInstFnstsw, Mem)

  //! @brief Calculate arctan(fp1 / fp0) and pop the register stack (FPU).
  INST_0x(fpatan, kInstFpatan)

  //! @brief Calculate fprem(fp0, fp1) and pop the register stack (FPU).
  INST_0x(fprem, kInstFprem)
  //! @brief Calculate IEEE fprem(fp0, fp1) and pop the register stack (FPU).
  INST_0x(fprem1, kInstFprem1)

  //! @brief Calculate arctan(fp0) and pop the register stack (FPU).
  INST_0x(fptan, kInstFptan)
  //! @brief Round fp0 to Integer (FPU).
  INST_0x(frndint, kInstFrndint)

  //! @brief Restore FPU state from @a o0 (94 or 108 bytes) (FPU).
  INST_1x(frstor, kInstFrstor, Mem)

  //! @brief Save FPU state to @a o0 (FPU).
  //!
  //! Store FPU state to 94 or 108-bytes after checking for
  //! pending unmasked FP exceptions. Then reinitialize
  //! the FPU.
  INST_1x(fsave, kInstFsave, Mem)

  //! @brief Scale (FPU).
  //!
  //! Scale fp0 by fp1.
  INST_0x(fscale, kInstFscale)

  //! @brief Calculate sine of fp0 and store result in fp0 (FPU).
  INST_0x(fsin, kInstFsin)

  //! @brief Sine and cosine (FPU).
  //!
  //! Compute the sine and cosine of fp0; replace fp0 with the sine
  //! and push the cosine on the register stack.
  INST_0x(fsincos, kInstFsincos)

  //! @brief Square root (FPU).
  //!
  //! Calculates square root of fp0 and stores the result in fp0.
  INST_0x(fsqrt, kInstFsqrt)

  //! @brief Store floating point value (FPU).
  //!
  //! Store fp0 as 32-bit or 64-bit floating point value to @a o0.
  INST_1x_(fst, kInstFst, Mem, o0.getSize() == 4 || o0.getSize() == 8)

  //! @brief Store floating point value to @a o0 (FPU).
  INST_1x(fst, kInstFst, FpReg)

  //! @brief Store floating point value and pop register stack (FPU).
  //!
  //! Store fp0 as 32-bit or 64-bit floating point value to @a o0
  //! and pop register stack.
  INST_1x_(fstp, kInstFstp, Mem, o0.getSize() == 4 || o0.getSize() == 8 || o0.getSize() == 10)

  //! @brief Store floating point value and pop register stack (FPU).
  //!
  //! Store fp0 to @a o0 and pop register stack.
  INST_1x(fstp, kInstFstp, FpReg)

  //! @brief Store x87 FPU control word (FPU).
  //!
  //! Store FPU control word to @a o0 (2 bytes) after checking for pending
  //! unmasked floating-point exceptions.
  INST_1x(fstcw, kInstFstcw, Mem)

  //! @brief Store x87 FPU environment (FPU).
  //!
  //! Store FPU environment to @a o0 (14 or 28 bytes) after checking for
  //! pending unmasked floating-point exceptions. Then mask all floating
  //! point exceptions.
  INST_1x(fstenv, kInstFstenv, Mem)

  //! @brief Store x87 FPU status word (AX) (FPU).
  INST_1x_(fstsw, kInstFstsw, GpReg, o0.isRegCode(kRegTypeGpw, kRegIndexAx))
  //! @brief Store x87 FPU status sord (2 bytes) (FPU).
  INST_1x(fstsw, kInstFstsw, Mem)

  //! @brief Subtract @a o0 from @a o0 and store result in @a o0 (FPU).
  //!
  //! @note One of dst or src must be fp0.
  INST_2x_(fsub, kInstFsub, FpReg, FpReg, o0.getRegIndex() == 0 || o1.getRegIndex() == 0)
  //! @brief Subtract 32-bit or 64-bit @a o0 from fp0 and store result in fp0 (FPU).
  INST_1x_(fsub, kInstFsub, Mem, o0.getSize() == 4 || o0.getSize() == 8)
  //! @brief Subtract fp0 from @a o0 and POP register stack (FPU).
  INST_1x(fsubp, kInstFsubp, FpReg)
  //! @overload.
  INST_0x(fsubp, kInstFsubp)

  //! @brief Reverse subtract @a o1 from @a o0 and store result in @a o0 (FPU).
  //!
  //! @note One of dst or src must be fp0.
  INST_2x_(fsubr, kInstFsubr, FpReg, FpReg, o0.getRegIndex() == 0 || o1.getRegIndex() == 0)
  //! @brief Reverse subtract 32-bit or 64-bit @a o0 from fp0 and store result in fp0 (FPU).
  INST_1x_(fsubr, kInstFsubr, Mem, o0.getSize() == 4 || o0.getSize() == 8)
  //! @brief Reverse subtract fp0 from @a o0 and POP register stack (FPU).
  INST_1x(fsubrp, kInstFsubrp, FpReg)
  //! @overload.
  INST_0x(fsubrp, kInstFsubrp)

  //! @brief Floating point test - Compare fp0 with 0.0. (FPU).
  INST_0x(ftst, kInstFtst)

  //! @brief Unordered compare fp0 with @a o0 (FPU).
  INST_1x(fucom, kInstFucom, FpReg)
  //! @brief Unordered compare fp0 with fp1 (FPU).
  INST_0x(fucom, kInstFucom)
  //! @brief Unordered compare fp0 and @a o0, check for ordered values and Set EFLAGS (FPU).
  INST_1x(fucomi, kInstFucomi, FpReg)
  //! @brief Unordered compare fp0 and @a o0, Check for ordered values and Set EFLAGS and pop the stack (FPU).
  INST_1x(fucomip, kInstFucomip, FpReg)
  //! @brief Unordered compare fp0 with @a o0 and pop register stack (FPU).
  INST_1x(fucomp, kInstFucomp, FpReg)
  //! @brief Unordered compare fp0 with fp1 and pop register stack (FPU).
  INST_0x(fucomp, kInstFucomp)
  //! @brief Unordered compare fp0 with fp1 and pop register stack twice (FPU).
  INST_0x(fucompp, kInstFucompp)

  INST_0x(fwait, kInstFwait)

  //! @brief Examine fp0 (FPU).
  INST_0x(fxam, kInstFxam)

  //! @brief Exchange register contents (FPU).
  //!
  //! Exchange content of fp0 with @a o0.
  INST_1x(fxch, kInstFxch, FpReg)

  //! @brief Restore FP, MMX and streaming SIMD extension states (FPU, MMX, SSE).
  //!
  //! Load FP and MMX technology and Streaming SIMD Extension state from
  //! src (512 bytes).
  INST_1x(fxrstor, kInstFxrstor, Mem)

  //! @brief Store FP, MMX and streaming SIMD extension states (FPU, MMX, SSE).
  //!
  //! Store FP and MMX technology state and Streaming SIMD Extension state
  //! to dst (512 bytes).
  INST_1x(fxsave, kInstFxsave, Mem)

  //! @brief Extract exponent and significand (FPU).
  //!
  //! Separate value in fp0 into exponent and significand, store exponent
  //! in fp0 and push the significand on the register stack.
  INST_0x(fxtract, kInstFxtract)

  //! @brief Compute y * log2(x).
  //!
  //! Replace fp1 with (fp1 * log2(fp0)) and pop the register stack.
  INST_0x(fyl2x, kInstFyl2x)
  //! @brief Compute y * log_2(x+1).
  //!
  //! Replace fp1 with (fp1 * (log2(fp0)+1)) and pop the register stack.
  INST_0x(fyl2xp1, kInstFyl2xp1)

  // --------------------------------------------------------------------------
  // [MMX]
  // --------------------------------------------------------------------------

  //! @brief Move dword (MMX).
  INST_2x(movd, kInstMovd, Mem, MmReg)
  //! @overload
  INST_2x(movd, kInstMovd, GpReg, MmReg)
  //! @overload
  INST_2x(movd, kInstMovd, MmReg, Mem)
  //! @overload
  INST_2x(movd, kInstMovd, MmReg, GpReg)

  //! @brief Move qword (MMX).
  INST_2x(movq, kInstMovq, MmReg, MmReg)
  //! @overload
  INST_2x(movq, kInstMovq, Mem, MmReg)
  //! @overload
  INST_2x(movq, kInstMovq, MmReg, Mem)

  //! @brief Pack with signed saturation (MMX).
  INST_2x(packsswb, kInstPacksswb, MmReg, MmReg)
  //! @overload
  INST_2x(packsswb, kInstPacksswb, MmReg, Mem)

  //! @brief Pack with signed saturation (MMX).
  INST_2x(packssdw, kInstPackssdw, MmReg, MmReg)
  //! @overload
  INST_2x(packssdw, kInstPackssdw, MmReg, Mem)

  //! @brief Pack with unsigned saturation (MMX).
  INST_2x(packuswb, kInstPackuswb, MmReg, MmReg)
  //! @overload
  INST_2x(packuswb, kInstPackuswb, MmReg, Mem)

  //! @brief Packed byte add (MMX).
  INST_2x(paddb, kInstPaddb, MmReg, MmReg)
  //! @overload
  INST_2x(paddb, kInstPaddb, MmReg, Mem)

  //! @brief Packed word add (MMX).
  INST_2x(paddw, kInstPaddw, MmReg, MmReg)
  //! @overload
  INST_2x(paddw, kInstPaddw, MmReg, Mem)

  //! @brief Packed dword add (MMX).
  INST_2x(paddd, kInstPaddd, MmReg, MmReg)
  //! @overload
  INST_2x(paddd, kInstPaddd, MmReg, Mem)

  //! @brief Packed add with saturation (MMX).
  INST_2x(paddsb, kInstPaddsb, MmReg, MmReg)
  //! @overload
  INST_2x(paddsb, kInstPaddsb, MmReg, Mem)

  //! @brief Packed add with saturation (MMX).
  INST_2x(paddsw, kInstPaddsw, MmReg, MmReg)
  //! @overload
  INST_2x(paddsw, kInstPaddsw, MmReg, Mem)

  //! @brief Packed add unsigned with saturation (MMX).
  INST_2x(paddusb, kInstPaddusb, MmReg, MmReg)
  //! @overload
  INST_2x(paddusb, kInstPaddusb, MmReg, Mem)

  //! @brief Packed add unsigned with saturation (MMX).
  INST_2x(paddusw, kInstPaddusw, MmReg, MmReg)
  //! @overload
  INST_2x(paddusw, kInstPaddusw, MmReg, Mem)

  //! @brief And (MMX).
  INST_2x(pand, kInstPand, MmReg, MmReg)
  //! @overload
  INST_2x(pand, kInstPand, MmReg, Mem)

  //! @brief And-not (MMX).
  INST_2x(pandn, kInstPandn, MmReg, MmReg)
  //! @overload
  INST_2x(pandn, kInstPandn, MmReg, Mem)

  //! @brief Packed compare bytes for equal (MMX).
  INST_2x(pcmpeqb, kInstPcmpeqb, MmReg, MmReg)
  //! @overload
  INST_2x(pcmpeqb, kInstPcmpeqb, MmReg, Mem)

  //! @brief Packed compare words for equal (MMX).
  INST_2x(pcmpeqw, kInstPcmpeqw, MmReg, MmReg)
  //! @overload
  INST_2x(pcmpeqw, kInstPcmpeqw, MmReg, Mem)

  //! @brief Packed compare dwords for equal (MMX).
  INST_2x(pcmpeqd, kInstPcmpeqd, MmReg, MmReg)
  //! @overload
  INST_2x(pcmpeqd, kInstPcmpeqd, MmReg, Mem)

  //! @brief Packed compare bytes for greater than (MMX).
  INST_2x(pcmpgtb, kInstPcmpgtb, MmReg, MmReg)
  //! @overload
  INST_2x(pcmpgtb, kInstPcmpgtb, MmReg, Mem)

  //! @brief Packed compare words for greater than (MMX).
  INST_2x(pcmpgtw, kInstPcmpgtw, MmReg, MmReg)
  //! @overload
  INST_2x(pcmpgtw, kInstPcmpgtw, MmReg, Mem)

  //! @brief Packed compare dqords for greater than (MMX).
  INST_2x(pcmpgtd, kInstPcmpgtd, MmReg, MmReg)
  //! @overload
  INST_2x(pcmpgtd, kInstPcmpgtd, MmReg, Mem)

  //! @brief Packed multiply high (MMX).
  INST_2x(pmulhw, kInstPmulhw, MmReg, MmReg)
  //! @overload
  INST_2x(pmulhw, kInstPmulhw, MmReg, Mem)

  //! @brief Packed multiply low (MMX).
  INST_2x(pmullw, kInstPmullw, MmReg, MmReg)
  //! @overload
  INST_2x(pmullw, kInstPmullw, MmReg, Mem)

  //! @brief Or (MMX).
  INST_2x(por, kInstPor, MmReg, MmReg)
  //! @overload
  INST_2x(por, kInstPor, MmReg, Mem)

  //! @brief Packed multiply and add (MMX).
  INST_2x(pmaddwd, kInstPmaddwd, MmReg, MmReg)
  //! @overload
  INST_2x(pmaddwd, kInstPmaddwd, MmReg, Mem)

  //! @brief Packed shift left logical (MMX).
  INST_2x(pslld, kInstPslld, MmReg, MmReg)
  //! @overload
  INST_2x(pslld, kInstPslld, MmReg, Mem)
  //! @overload
  INST_2i(pslld, kInstPslld, MmReg, Imm)

  //! @brief Packed shift left logical (MMX).
  INST_2x(psllq, kInstPsllq, MmReg, MmReg)
  //! @overload
  INST_2x(psllq, kInstPsllq, MmReg, Mem)
  //! @overload
  INST_2i(psllq, kInstPsllq, MmReg, Imm)

  //! @brief Packed shift left logical (MMX).
  INST_2x(psllw, kInstPsllw, MmReg, MmReg)
  //! @overload
  INST_2x(psllw, kInstPsllw, MmReg, Mem)
  //! @overload
  INST_2i(psllw, kInstPsllw, MmReg, Imm)

  //! @brief Packed shift right arithmetic (MMX).
  INST_2x(psrad, kInstPsrad, MmReg, MmReg)
  //! @overload
  INST_2x(psrad, kInstPsrad, MmReg, Mem)
  //! @overload
  INST_2i(psrad, kInstPsrad, MmReg, Imm)

  //! @brief Packed shift right arithmetic (MMX).
  INST_2x(psraw, kInstPsraw, MmReg, MmReg)
  //! @overload
  INST_2x(psraw, kInstPsraw, MmReg, Mem)
  //! @overload
  INST_2i(psraw, kInstPsraw, MmReg, Imm)

  //! @brief Packed shift right logical (MMX).
  INST_2x(psrld, kInstPsrld, MmReg, MmReg)
  //! @overload
  INST_2x(psrld, kInstPsrld, MmReg, Mem)
  //! @overload
  INST_2i(psrld, kInstPsrld, MmReg, Imm)

  //! @brief Packed shift right logical (MMX).
  INST_2x(psrlq, kInstPsrlq, MmReg, MmReg)
  //! @overload
  INST_2x(psrlq, kInstPsrlq, MmReg, Mem)
  //! @overload
  INST_2i(psrlq, kInstPsrlq, MmReg, Imm)

  //! @brief Packed shift right logical (MMX).
  INST_2x(psrlw, kInstPsrlw, MmReg, MmReg)
  //! @overload
  INST_2x(psrlw, kInstPsrlw, MmReg, Mem)
  //! @overload
  INST_2i(psrlw, kInstPsrlw, MmReg, Imm)

  //! @brief Packed subtract (MMX).
  INST_2x(psubb, kInstPsubb, MmReg, MmReg)
  //! @overload
  INST_2x(psubb, kInstPsubb, MmReg, Mem)

  //! @brief Packed subtract (MMX).
  INST_2x(psubw, kInstPsubw, MmReg, MmReg)
  //! @overload
  INST_2x(psubw, kInstPsubw, MmReg, Mem)

  //! @brief Packed subtract (MMX).
  INST_2x(psubd, kInstPsubd, MmReg, MmReg)
  //! @overload
  INST_2x(psubd, kInstPsubd, MmReg, Mem)

  //! @brief Packed subtract with saturation (MMX).
  INST_2x(psubsb, kInstPsubsb, MmReg, MmReg)
  //! @overload
  INST_2x(psubsb, kInstPsubsb, MmReg, Mem)

  //! @brief Packed subtract with saturation (MMX).
  INST_2x(psubsw, kInstPsubsw, MmReg, MmReg)
  //! @overload
  INST_2x(psubsw, kInstPsubsw, MmReg, Mem)

  //! @brief Packed subtract with unsigned saturation (MMX).
  INST_2x(psubusb, kInstPsubusb, MmReg, MmReg)
  //! @overload
  INST_2x(psubusb, kInstPsubusb, MmReg, Mem)

  //! @brief Packed subtract with unsigned saturation (MMX).
  INST_2x(psubusw, kInstPsubusw, MmReg, MmReg)
  //! @overload
  INST_2x(psubusw, kInstPsubusw, MmReg, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckhbw, kInstPunpckhbw, MmReg, MmReg)
  //! @overload
  INST_2x(punpckhbw, kInstPunpckhbw, MmReg, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckhwd, kInstPunpckhwd, MmReg, MmReg)
  //! @overload
  INST_2x(punpckhwd, kInstPunpckhwd, MmReg, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckhdq, kInstPunpckhdq, MmReg, MmReg)
  //! @overload
  INST_2x(punpckhdq, kInstPunpckhdq, MmReg, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpcklbw, kInstPunpcklbw, MmReg, MmReg)
  //! @overload
  INST_2x(punpcklbw, kInstPunpcklbw, MmReg, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpcklwd, kInstPunpcklwd, MmReg, MmReg)
  //! @overload
  INST_2x(punpcklwd, kInstPunpcklwd, MmReg, Mem)

  //! @brief Unpack high packed data (MMX).
  INST_2x(punpckldq, kInstPunpckldq, MmReg, MmReg)
  //! @overload
  INST_2x(punpckldq, kInstPunpckldq, MmReg, Mem)

  //! @brief Xor (MMX).
  INST_2x(pxor, kInstPxor, MmReg, MmReg)
  //! @overload
  INST_2x(pxor, kInstPxor, MmReg, Mem)

  //! @brief Empty MMX state.
  INST_0x(emms, kInstEmms)

  // -------------------------------------------------------------------------
  // [3dNow]
  // -------------------------------------------------------------------------

  //! @brief Packed SP-FP to integer convert (3dNow!).
  INST_2x(pf2id, kInstPf2id, MmReg, MmReg)
  //! @overload
  INST_2x(pf2id, kInstPf2id, MmReg, Mem)

  //! @brief  Packed SP-FP to integer word convert (3dNow!).
  INST_2x(pf2iw, kInstPf2iw, MmReg, MmReg)
  //! @overload
  INST_2x(pf2iw, kInstPf2iw, MmReg, Mem)

  //! @brief Packed SP-FP accumulate (3dNow!).
  INST_2x(pfacc, kInstPfacc, MmReg, MmReg)
  //! @overload
  INST_2x(pfacc, kInstPfacc, MmReg, Mem)

  //! @brief Packed SP-FP addition (3dNow!).
  INST_2x(pfadd, kInstPfadd, MmReg, MmReg)
  //! @overload
  INST_2x(pfadd, kInstPfadd, MmReg, Mem)

  //! @brief Packed SP-FP compare - dst == src (3dNow!).
  INST_2x(pfcmpeq, kInstPfcmpeq, MmReg, MmReg)
  //! @overload
  INST_2x(pfcmpeq, kInstPfcmpeq, MmReg, Mem)

  //! @brief Packed SP-FP compare - dst >= src (3dNow!).
  INST_2x(pfcmpge, kInstPfcmpge, MmReg, MmReg)
  //! @overload
  INST_2x(pfcmpge, kInstPfcmpge, MmReg, Mem)

  //! @brief Packed SP-FP compare - dst > src (3dNow!).
  INST_2x(pfcmpgt, kInstPfcmpgt, MmReg, MmReg)
  //! @overload
  INST_2x(pfcmpgt, kInstPfcmpgt, MmReg, Mem)

  //! @brief Packed SP-FP maximum (3dNow!).
  INST_2x(pfmax, kInstPfmax, MmReg, MmReg)
  //! @overload
  INST_2x(pfmax, kInstPfmax, MmReg, Mem)

  //! @brief Packed SP-FP minimum (3dNow!).
  INST_2x(pfmin, kInstPfmin, MmReg, MmReg)
  //! @overload
  INST_2x(pfmin, kInstPfmin, MmReg, Mem)

  //! @brief Packed SP-FP multiply (3dNow!).
  INST_2x(pfmul, kInstPfmul, MmReg, MmReg)
  //! @overload
  INST_2x(pfmul, kInstPfmul, MmReg, Mem)

  //! @brief Packed SP-FP negative accumulate (3dNow!).
  INST_2x(pfnacc, kInstPfnacc, MmReg, MmReg)
  //! @overload
  INST_2x(pfnacc, kInstPfnacc, MmReg, Mem)

  //! @brief Packed SP-FP mixed accumulate (3dNow!).
  INST_2x(pfpnacc, kInstPfpnacc, MmReg, MmReg)
  //! @overload
  INST_2x(pfpnacc, kInstPfpnacc, MmReg, Mem)

  //! @brief Packed SP-FP reciprocal Approximation (3dNow!).
  INST_2x(pfrcp, kInstPfrcp, MmReg, MmReg)
  //! @overload
  INST_2x(pfrcp, kInstPfrcp, MmReg, Mem)

  //! @brief Packed SP-FP reciprocal, first iteration step (3dNow!).
  INST_2x(pfrcpit1, kInstPfrcpit1, MmReg, MmReg)
  //! @overload
  INST_2x(pfrcpit1, kInstPfrcpit1, MmReg, Mem)

  //! @brief Packed SP-FP reciprocal, second iteration step (3dNow!).
  INST_2x(pfrcpit2, kInstPfrcpit2, MmReg, MmReg)
  //! @overload
  INST_2x(pfrcpit2, kInstPfrcpit2, MmReg, Mem)

  //! @brief Packed SP-FP reciprocal square root, first iteration step (3dNow!).
  INST_2x(pfrsqit1, kInstPfrsqit1, MmReg, MmReg)
  //! @overload
  INST_2x(pfrsqit1, kInstPfrsqit1, MmReg, Mem)

  //! @brief Packed SP-FP reciprocal square root approximation (3dNow!).
  INST_2x(pfrsqrt, kInstPfrsqrt, MmReg, MmReg)
  //! @overload
  INST_2x(pfrsqrt, kInstPfrsqrt, MmReg, Mem)

  //! @brief Packed SP-FP subtract (3dNow!).
  INST_2x(pfsub, kInstPfsub, MmReg, MmReg)
  //! @overload
  INST_2x(pfsub, kInstPfsub, MmReg, Mem)

  //! @brief Packed SP-FP reverse subtract (3dNow!).
  INST_2x(pfsubr, kInstPfsubr, MmReg, MmReg)
  //! @overload
  INST_2x(pfsubr, kInstPfsubr, MmReg, Mem)

  //! @brief Packed dwords to SP-FP (3dNow!).
  INST_2x(pi2fd, kInstPi2fd, MmReg, MmReg)
  //! @overload
  INST_2x(pi2fd, kInstPi2fd, MmReg, Mem)

  //! @brief Packed words to SP-FP (3dNow!).
  INST_2x(pi2fw, kInstPi2fw, MmReg, MmReg)
  //! @overload
  INST_2x(pi2fw, kInstPi2fw, MmReg, Mem)

  //! @brief Packed swap dword (3dNow!)
  INST_2x(pswapd, kInstPswapd, MmReg, MmReg)
  //! @overload
  INST_2x(pswapd, kInstPswapd, MmReg, Mem)

  //! @brief Prefetch (3dNow!).
  INST_1x(prefetch3dnow, kInstPrefetch3dNow, Mem)

  //! @brief Prefetch and set cache to modified (3dNow!).
  INST_1x(prefetchw3dnow, kInstPrefetchw3dNow, Mem)

  //! @brief Faster EMMS (3dNow!).
  INST_0x(femms, kInstFemms)

  // --------------------------------------------------------------------------
  // [SSE]
  // --------------------------------------------------------------------------

  //! @brief Packed SP-FP add (SSE).
  INST_2x(addps, kInstAddps, XmmReg, XmmReg)
  //! @overload
  INST_2x(addps, kInstAddps, XmmReg, Mem)

  //! @brief Scalar SP-FP add (SSE).
  INST_2x(addss, kInstAddss, XmmReg, XmmReg)
  //! @overload
  INST_2x(addss, kInstAddss, XmmReg, Mem)

  //! @brief And-not for SP-FP (SSE).
  INST_2x(andnps, kInstAndnps, XmmReg, XmmReg)
  //! @overload
  INST_2x(andnps, kInstAndnps, XmmReg, Mem)

  //! @brief And for SP-FP (SSE).
  INST_2x(andps, kInstAndps, XmmReg, XmmReg)
  //! @overload
  INST_2x(andps, kInstAndps, XmmReg, Mem)

  //! @brief Packed SP-FP compare (SSE).
  INST_3i(cmpps, kInstCmpps, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(cmpps, kInstCmpps, XmmReg, Mem, Imm)

  //! @brief Compare scalar SP-FP values (SSE).
  INST_3i(cmpss, kInstCmpss, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(cmpss, kInstCmpss, XmmReg, Mem, Imm)

  //! @brief Scalar ordered SP-FP compare and set EFLAGS (SSE).
  INST_2x(comiss, kInstComiss, XmmReg, XmmReg)
  //! @overload
  INST_2x(comiss, kInstComiss, XmmReg, Mem)

  //! @brief Packed signed INT32 to packed SP-FP conversion (SSE).
  INST_2x(cvtpi2ps, kInstCvtpi2ps, XmmReg, MmReg)
  //! @overload
  INST_2x(cvtpi2ps, kInstCvtpi2ps, XmmReg, Mem)

  //! @brief Packed SP-FP to packed INT32 conversion (SSE).
  INST_2x(cvtps2pi, kInstCvtps2pi, MmReg, XmmReg)
  //! @overload
  INST_2x(cvtps2pi, kInstCvtps2pi, MmReg, Mem)

  //! @brief Scalar signed INT32 to SP-FP conversion (SSE).
  INST_2x(cvtsi2ss, kInstCvtsi2ss, XmmReg, GpReg)
  //! @overload
  INST_2x(cvtsi2ss, kInstCvtsi2ss, XmmReg, Mem)

  //! @brief Scalar SP-FP to signed INT32 conversion (SSE).
  INST_2x(cvtss2si, kInstCvtss2si, GpReg, XmmReg)
  //! @overload
  INST_2x(cvtss2si, kInstCvtss2si, GpReg, Mem)

  //! @brief Packed SP-FP to packed INT32 conversion (truncate) (SSE).
  INST_2x(cvttps2pi, kInstCvttps2pi, MmReg, XmmReg)
  //! @overload
  INST_2x(cvttps2pi, kInstCvttps2pi, MmReg, Mem)

  //! @brief Scalar SP-FP to signed INT32 conversion (truncate) (SSE).
  INST_2x(cvttss2si, kInstCvttss2si, GpReg, XmmReg)
  //! @overload
  INST_2x(cvttss2si, kInstCvttss2si, GpReg, Mem)

  //! @brief Packed SP-FP divide (SSE).
  INST_2x(divps, kInstDivps, XmmReg, XmmReg)
  //! @overload
  INST_2x(divps, kInstDivps, XmmReg, Mem)

  //! @brief Scalar SP-FP divide (SSE).
  INST_2x(divss, kInstDivss, XmmReg, XmmReg)
  //! @overload
  INST_2x(divss, kInstDivss, XmmReg, Mem)

  //! @brief Load streaming SIMD extension control/status (SSE).
  INST_1x(ldmxcsr, kInstLdmxcsr, Mem)

  //! @brief Byte mask write (SSE).
  //!
  //! @note The default memory location is specified by DS:EDI.
  INST_2x(maskmovq, kInstMaskmovq, MmReg, MmReg)

  //! @brief Packed SP-FP maximum (SSE).
  INST_2x(maxps, kInstMaxps, XmmReg, XmmReg)
  //! @overload
  INST_2x(maxps, kInstMaxps, XmmReg, Mem)

  //! @brief Scalar SP-FP maximum (SSE).
  INST_2x(maxss, kInstMaxss, XmmReg, XmmReg)
  //! @overload
  INST_2x(maxss, kInstMaxss, XmmReg, Mem)

  //! @brief Packed SP-FP minimum (SSE).
  INST_2x(minps, kInstMinps, XmmReg, XmmReg)
  //! @overload
  INST_2x(minps, kInstMinps, XmmReg, Mem)

  //! @brief Scalar SP-FP minimum (SSE).
  INST_2x(minss, kInstMinss, XmmReg, XmmReg)
  //! @overload
  INST_2x(minss, kInstMinss, XmmReg, Mem)

  //! @brief Move aligned packed SP-FP values (SSE).
  INST_2x(movaps, kInstMovaps, XmmReg, XmmReg)
  //! @overload
  INST_2x(movaps, kInstMovaps, XmmReg, Mem)
  //! @brief Move aligned packed SP-FP values (SSE).
  INST_2x(movaps, kInstMovaps, Mem, XmmReg)

  //! @brief Move dword.
  INST_2x(movd, kInstMovd, Mem, XmmReg)
  //! @overload
  INST_2x(movd, kInstMovd, GpReg, XmmReg)
  //! @overload
  INST_2x(movd, kInstMovd, XmmReg, Mem)
  //! @overload
  INST_2x(movd, kInstMovd, XmmReg, GpReg)

  //! @brief Move qword (SSE).
  INST_2x(movq, kInstMovq, XmmReg, XmmReg)
  //! @overload
  INST_2x(movq, kInstMovq, Mem, XmmReg)
  //! @overload
  INST_2x(movq, kInstMovq, XmmReg, Mem)

  //! @brief Move 64 Bits non-temporal (SSE).
  INST_2x(movntq, kInstMovntq, Mem, MmReg)

  //! @brief High to low packed SP-FP (SSE).
  INST_2x(movhlps, kInstMovhlps, XmmReg, XmmReg)

  //! @brief Move high packed SP-FP (SSE).
  INST_2x(movhps, kInstMovhps, XmmReg, Mem)
    //! @brief Move high packed SP-FP (SSE).
  INST_2x(movhps, kInstMovhps, Mem, XmmReg)

  //! @brief Move low to high packed SP-FP (SSE).
  INST_2x(movlhps, kInstMovlhps, XmmReg, XmmReg)

  //! @brief Move low packed SP-FP (SSE).
  INST_2x(movlps, kInstMovlps, XmmReg, Mem)
  //! @brief Move low packed SP-FP (SSE).
  INST_2x(movlps, kInstMovlps, Mem, XmmReg)

  //! @brief Move aligned four packed SP-FP non-temporal (SSE).
  INST_2x(movntps, kInstMovntps, Mem, XmmReg)

  //! @brief Move scalar SP-FP (SSE).
  INST_2x(movss, kInstMovss, XmmReg, XmmReg)
  //! @overload
  INST_2x(movss, kInstMovss, XmmReg, Mem)
  //! @overload
  INST_2x(movss, kInstMovss, Mem, XmmReg)

  //! @brief Move unaligned packed SP-FP values (SSE).
  INST_2x(movups, kInstMovups, XmmReg, XmmReg)
  //! @overload
  INST_2x(movups, kInstMovups, XmmReg, Mem)
  //! @overload
  INST_2x(movups, kInstMovups, Mem, XmmReg)

  //! @brief Packed SP-FP multiply (SSE).
  INST_2x(mulps, kInstMulps, XmmReg, XmmReg)
  //! @overload
  INST_2x(mulps, kInstMulps, XmmReg, Mem)

  //! @brief Scalar SP-FP multiply (SSE).
  INST_2x(mulss, kInstMulss, XmmReg, XmmReg)
  //! @overload
  INST_2x(mulss, kInstMulss, XmmReg, Mem)

  //! @brief Or for SP-FP data (SSE).
  INST_2x(orps, kInstOrps, XmmReg, XmmReg)
  //! @overload
  INST_2x(orps, kInstOrps, XmmReg, Mem)

  //! @brief Packed average (SSE).
  INST_2x(pavgb, kInstPavgb, MmReg, MmReg)
  //! @overload
  INST_2x(pavgb, kInstPavgb, MmReg, Mem)

  //! @brief Packed average (SSE).
  INST_2x(pavgw, kInstPavgw, MmReg, MmReg)
  //! @overload
  INST_2x(pavgw, kInstPavgw, MmReg, Mem)

  //! @brief Extract word (SSE).
  INST_3i(pextrw, kInstPextrw, GpReg, MmReg, Imm)

  //! @brief Insert word (SSE).
  INST_3i(pinsrw, kInstPinsrw, MmReg, GpReg, Imm)
  //! @overload
  INST_3i(pinsrw, kInstPinsrw, MmReg, Mem, Imm)

  //! @brief Packed signed integer word maximum (SSE).
  INST_2x(pmaxsw, kInstPmaxsw, MmReg, MmReg)
  //! @overload
  INST_2x(pmaxsw, kInstPmaxsw, MmReg, Mem)

  //! @brief Packed unsigned integer byte maximum (SSE).
  INST_2x(pmaxub, kInstPmaxub, MmReg, MmReg)
  //! @overload
  INST_2x(pmaxub, kInstPmaxub, MmReg, Mem)

  //! @brief Packed signed integer word minimum (SSE).
  INST_2x(pminsw, kInstPminsw, MmReg, MmReg)
  //! @overload
  INST_2x(pminsw, kInstPminsw, MmReg, Mem)

  //! @brief Packed unsigned integer byte minimum (SSE).
  INST_2x(pminub, kInstPminub, MmReg, MmReg)
  //! @overload
  INST_2x(pminub, kInstPminub, MmReg, Mem)

  //! @brief Move Byte mask to integer (SSE).
  INST_2x(pmovmskb, kInstPmovmskb, GpReg, MmReg)

  //! @brief Packed multiply high unsigned (SSE).
  INST_2x(pmulhuw, kInstPmulhuw, MmReg, MmReg)
  //! @overload
  INST_2x(pmulhuw, kInstPmulhuw, MmReg, Mem)

  //! @brief Packed sum of absolute differences (SSE).
  INST_2x(psadbw, kInstPsadbw, MmReg, MmReg)
  //! @overload
  INST_2x(psadbw, kInstPsadbw, MmReg, Mem)

  //! @brief Packed shuffle Word (SSE).
  INST_3i(pshufw, kInstPshufw, MmReg, MmReg, Imm)
  //! @overload
  INST_3i(pshufw, kInstPshufw, MmReg, Mem, Imm)

  //! @brief Packed SP-FP reciprocal (SSE).
  INST_2x(rcpps, kInstRcpps, XmmReg, XmmReg)
  //! @overload
  INST_2x(rcpps, kInstRcpps, XmmReg, Mem)

  //! @brief Scalar SP-FP reciprocal (SSE).
  INST_2x(rcpss, kInstRcpss, XmmReg, XmmReg)
  //! @overload
  INST_2x(rcpss, kInstRcpss, XmmReg, Mem)

  //! @brief Prefetch (SSE).
  INST_2i(prefetch, kInstPrefetch, Mem, Imm)

  //! @brief Compute sum of absolute differences (SSE).
  INST_2x(psadbw, kInstPsadbw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psadbw, kInstPsadbw, XmmReg, Mem)

  //! @brief Packed SP-FP square root reciprocal (SSE).
  INST_2x(rsqrtps, kInstRsqrtps, XmmReg, XmmReg)
  //! @overload
  INST_2x(rsqrtps, kInstRsqrtps, XmmReg, Mem)

  //! @brief Scalar SP-FP square root reciprocal (SSE).
  INST_2x(rsqrtss, kInstRsqrtss, XmmReg, XmmReg)
  //! @overload
  INST_2x(rsqrtss, kInstRsqrtss, XmmReg, Mem)

  //! @brief Store fence (SSE).
  INST_0x(sfence, kInstSfence)

  //! @brief Shuffle SP-FP (SSE).
  INST_3i(shufps, kInstShufps, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(shufps, kInstShufps, XmmReg, Mem, Imm)

  //! @brief Packed SP-FP square root (SSE).
  INST_2x(sqrtps, kInstSqrtps, XmmReg, XmmReg)
  //! @overload
  INST_2x(sqrtps, kInstSqrtps, XmmReg, Mem)

  //! @brief Scalar SP-FP square root (SSE).
  INST_2x(sqrtss, kInstSqrtss, XmmReg, XmmReg)
  //! @overload
  INST_2x(sqrtss, kInstSqrtss, XmmReg, Mem)

  //! @brief Store streaming SIMD extension control/status (SSE).
  INST_1x(stmxcsr, kInstStmxcsr, Mem)

  //! @brief Packed SP-FP subtract (SSE).
  INST_2x(subps, kInstSubps, XmmReg, XmmReg)
  //! @overload
  INST_2x(subps, kInstSubps, XmmReg, Mem)

  //! @brief Scalar SP-FP subtract (SSE).
  INST_2x(subss, kInstSubss, XmmReg, XmmReg)
  //! @overload
  INST_2x(subss, kInstSubss, XmmReg, Mem)

  //! @brief Unordered scalar SP-FP compare and set EFLAGS (SSE).
  INST_2x(ucomiss, kInstUcomiss, XmmReg, XmmReg)
  //! @overload
  INST_2x(ucomiss, kInstUcomiss, XmmReg, Mem)

  //! @brief Unpack high packed SP-FP data (SSE).
  INST_2x(unpckhps, kInstUnpckhps, XmmReg, XmmReg)
  //! @overload
  INST_2x(unpckhps, kInstUnpckhps, XmmReg, Mem)

  //! @brief Unpack low packed SP-FP data (SSE).
  INST_2x(unpcklps, kInstUnpcklps, XmmReg, XmmReg)
  //! @overload
  INST_2x(unpcklps, kInstUnpcklps, XmmReg, Mem)

  //! @brief Xor for SP-FP data (SSE).
  INST_2x(xorps, kInstXorps, XmmReg, XmmReg)
  //! @overload
  INST_2x(xorps, kInstXorps, XmmReg, Mem)

  // --------------------------------------------------------------------------
  // [SSE2]
  // --------------------------------------------------------------------------

  //! @brief Packed DP-FP add (SSE2).
  INST_2x(addpd, kInstAddpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(addpd, kInstAddpd, XmmReg, Mem)

  //! @brief Scalar DP-FP add (SSE2).
  INST_2x(addsd, kInstAddsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(addsd, kInstAddsd, XmmReg, Mem)

  //! @brief And-not for DP-FP (SSE2).
  INST_2x(andnpd, kInstAndnpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(andnpd, kInstAndnpd, XmmReg, Mem)

  //! @brief And for DP-FP (SSE2).
  INST_2x(andpd, kInstAndpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(andpd, kInstAndpd, XmmReg, Mem)

  //! @brief Flush cache line (SSE2).
  INST_1x(clflush, kInstClflush, Mem)

  //! @brief Packed DP-FP compare (SSE2).
  INST_3i(cmppd, kInstCmppd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(cmppd, kInstCmppd, XmmReg, Mem, Imm)

  //! @brief Compare scalar SP-FP values (SSE2).
  INST_3i(cmpsd, kInstCmpsd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(cmpsd, kInstCmpsd, XmmReg, Mem, Imm)

  //! @brief Scalar ordered DP-FP compare and set EFLAGS (SSE2).
  INST_2x(comisd, kInstComisd, XmmReg, XmmReg)
  //! @overload
  INST_2x(comisd, kInstComisd, XmmReg, Mem)

  //! @brief Convert packed qword integers to packed DP-FP values (SSE2).
  INST_2x(cvtdq2pd, kInstCvtdq2pd, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtdq2pd, kInstCvtdq2pd, XmmReg, Mem)

  //! @brief Convert packed qword integers to packed SP-FP values (SSE2).
  INST_2x(cvtdq2ps, kInstCvtdq2ps, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtdq2ps, kInstCvtdq2ps, XmmReg, Mem)

  //! @brief Convert packed DP-FP values to packed qword integers (SSE2).
  INST_2x(cvtpd2dq, kInstCvtpd2dq, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtpd2dq, kInstCvtpd2dq, XmmReg, Mem)

  //! @brief Convert packed DP-FP values to packed qword integers (SSE2).
  INST_2x(cvtpd2pi, kInstCvtpd2pi, MmReg, XmmReg)
  //! @overload
  INST_2x(cvtpd2pi, kInstCvtpd2pi, MmReg, Mem)

  //! @brief Convert packed DP-FP values to packed SP-FP values (SSE2).
  INST_2x(cvtpd2ps, kInstCvtpd2ps, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtpd2ps, kInstCvtpd2ps, XmmReg, Mem)

  //! @brief Convert packed dword integers to packed DP-FP values (SSE2).
  INST_2x(cvtpi2pd, kInstCvtpi2pd, XmmReg, MmReg)
  //! @overload
  INST_2x(cvtpi2pd, kInstCvtpi2pd, XmmReg, Mem)

  //! @brief Convert packed SP-FP values to packed qword integers (SSE2).
  INST_2x(cvtps2dq, kInstCvtps2dq, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtps2dq, kInstCvtps2dq, XmmReg, Mem)

  //! @brief Convert packed SP-FP values to packed DP-FP values (SSE2).
  INST_2x(cvtps2pd, kInstCvtps2pd, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtps2pd, kInstCvtps2pd, XmmReg, Mem)

  //! @brief Convert scalar DP-FP value to dword integer (SSE2).
  INST_2x(cvtsd2si, kInstCvtsd2si, GpReg, XmmReg)
  //! @overload
  INST_2x(cvtsd2si, kInstCvtsd2si, GpReg, Mem)

  //! @brief Convert scalar DP-FP value to scalar SP-FP value (SSE2).
  INST_2x(cvtsd2ss, kInstCvtsd2ss, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtsd2ss, kInstCvtsd2ss, XmmReg, Mem)

  //! @brief Convert dword integer to scalar DP-FP value (SSE2).
  INST_2x(cvtsi2sd, kInstCvtsi2sd, XmmReg, GpReg)
  //! @overload
  INST_2x(cvtsi2sd, kInstCvtsi2sd, XmmReg, Mem)

  //! @brief Convert scalar SP-FP value to scalar DP-FP value (SSE2).
  INST_2x(cvtss2sd, kInstCvtss2sd, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvtss2sd, kInstCvtss2sd, XmmReg, Mem)

  //! @brief Convert with truncation packed DP-FP values to packed dword integers (SSE2).
  INST_2x(cvttpd2pi, kInstCvttpd2pi, MmReg, XmmReg)
  //! @overload
  INST_2x(cvttpd2pi, kInstCvttpd2pi, MmReg, Mem)

  //! @brief Convert with truncation packed DP-FP values to packed qword integers (SSE2).
  INST_2x(cvttpd2dq, kInstCvttpd2dq, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvttpd2dq, kInstCvttpd2dq, XmmReg, Mem)

  //! @brief Convert with truncation packed SP-FP values to packed qword integers (SSE2).
  INST_2x(cvttps2dq, kInstCvttps2dq, XmmReg, XmmReg)
  //! @overload
  INST_2x(cvttps2dq, kInstCvttps2dq, XmmReg, Mem)

  //! @brief Convert with truncation scalar DP-FP value to signed dword integer (SSE2).
  INST_2x(cvttsd2si, kInstCvttsd2si, GpReg, XmmReg)
  //! @overload
  INST_2x(cvttsd2si, kInstCvttsd2si, GpReg, Mem)

  //! @brief Packed DP-FP divide (SSE2).
  INST_2x(divpd, kInstDivpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(divpd, kInstDivpd, XmmReg, Mem)

  //! @brief Scalar DP-FP divide (SSE2).
  INST_2x(divsd, kInstDivsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(divsd, kInstDivsd, XmmReg, Mem)

  //! @brief Load fence (SSE2).
  INST_0x(lfence, kInstLfence)

  //! @brief Store selected bytes of oword (SSE2).
  //!
  //! @note Target is DS:EDI.
  INST_2x(maskmovdqu, kInstMaskmovdqu, XmmReg, XmmReg)

  //! @brief Return maximum packed DP-FP values (SSE2).
  INST_2x(maxpd, kInstMaxpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(maxpd, kInstMaxpd, XmmReg, Mem)

  //! @brief Return maximum scalar DP-FP value (SSE2).
  INST_2x(maxsd, kInstMaxsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(maxsd, kInstMaxsd, XmmReg, Mem)

  //! @brief Memory fence (SSE2).
  INST_0x(mfence, kInstMfence)

  //! @brief Return minimum packed DP-FP values (SSE2).
  INST_2x(minpd, kInstMinpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(minpd, kInstMinpd, XmmReg, Mem)

  //! @brief Return minimum scalar DP-FP value (SSE2).
  INST_2x(minsd, kInstMinsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(minsd, kInstMinsd, XmmReg, Mem)

  //! @brief Move aligned oword (SSE2).
  INST_2x(movdqa, kInstMovdqa, XmmReg, XmmReg)
  //! @overload
  INST_2x(movdqa, kInstMovdqa, XmmReg, Mem)
  //! @overload
  INST_2x(movdqa, kInstMovdqa, Mem, XmmReg)

  //! @brief Move unaligned oword (SSE2).
  INST_2x(movdqu, kInstMovdqu, XmmReg, XmmReg)
  //! @overload
  INST_2x(movdqu, kInstMovdqu, XmmReg, Mem)
  //! @overload
  INST_2x(movdqu, kInstMovdqu, Mem, XmmReg)

  //! @brief Extract packed SP-FP sign mask (SSE2).
  INST_2x(movmskps, kInstMovmskps, GpReg, XmmReg)

  //! @brief Extract packed DP-FP sign mask (SSE2).
  INST_2x(movmskpd, kInstMovmskpd, GpReg, XmmReg)

  //! @brief Move scalar DP-FP value (SSE2).
  INST_2x(movsd, kInstMovsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(movsd, kInstMovsd, XmmReg, Mem)
  //! @overload
  INST_2x(movsd, kInstMovsd, Mem, XmmReg)

  //! @brief Move aligned packed DP-FP values (SSE2).
  INST_2x(movapd, kInstMovapd, XmmReg, XmmReg)
  //! @overload
  INST_2x(movapd, kInstMovapd, XmmReg, Mem)
  //! @overload
  INST_2x(movapd, kInstMovapd, Mem, XmmReg)

  //! @brief Move qword from Xmm to Mm register (SSE2).
  INST_2x(movdq2q, kInstMovdq2q, MmReg, XmmReg)

  //! @brief Move qword from Mm to Xmm register (SSE2).
  INST_2x(movq2dq, kInstMovq2dq, XmmReg, MmReg)

  //! @brief Move high packed DP-FP value (SSE2).
  INST_2x(movhpd, kInstMovhpd, XmmReg, Mem)
  //! @overload
  INST_2x(movhpd, kInstMovhpd, Mem, XmmReg)

  //! @brief Move low packed DP-FP value (SSE2).
  INST_2x(movlpd, kInstMovlpd, XmmReg, Mem)
  //! @overload
  INST_2x(movlpd, kInstMovlpd, Mem, XmmReg)

  //! @brief Store oword using non-temporal hint (SSE2).
  INST_2x(movntdq, kInstMovntdq, Mem, XmmReg)

  //! @brief Store store dword using non-temporal hint (SSE2).
  INST_2x(movnti, kInstMovnti, Mem, GpReg)

  //! @brief Store packed DP-FP values using non-temporal hint (SSE2).
  INST_2x(movntpd, kInstMovntpd, Mem, XmmReg)

  //! @brief Move unaligned packed DP-FP values (SSE2).
  INST_2x(movupd, kInstMovupd, XmmReg, XmmReg)
  //! @overload
  INST_2x(movupd, kInstMovupd, XmmReg, Mem)
  //! @overload
  INST_2x(movupd, kInstMovupd, Mem, XmmReg)

  //! @brief Packed DP-FP multiply (SSE2).
  INST_2x(mulpd, kInstMulpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(mulpd, kInstMulpd, XmmReg, Mem)

  //! @brief Scalar DP-FP multiply (SSE2).
  INST_2x(mulsd, kInstMulsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(mulsd, kInstMulsd, XmmReg, Mem)

  //! @brief Or for DP-FP Data (SSE2).
  INST_2x(orpd, kInstOrpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(orpd, kInstOrpd, XmmReg, Mem)

  //! @brief Pack with signed saturation (SSE2).
  INST_2x(packsswb, kInstPacksswb, XmmReg, XmmReg)
  //! @overload
  INST_2x(packsswb, kInstPacksswb, XmmReg, Mem)

  //! @brief Pack with signed saturation (SSE2).
  INST_2x(packssdw, kInstPackssdw, XmmReg, XmmReg)
  //! @overload
  INST_2x(packssdw, kInstPackssdw, XmmReg, Mem)

  //! @brief Pack with unsigned saturation (SSE2).
  INST_2x(packuswb, kInstPackuswb, XmmReg, XmmReg)
  //! @overload
  INST_2x(packuswb, kInstPackuswb, XmmReg, Mem)

  //! @brief Packed byte Add (SSE2).
  INST_2x(paddb, kInstPaddb, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddb, kInstPaddb, XmmReg, Mem)

  //! @brief Packed word add (SSE2).
  INST_2x(paddw, kInstPaddw, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddw, kInstPaddw, XmmReg, Mem)

  //! @brief Packed dword add (SSE2).
  INST_2x(paddd, kInstPaddd, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddd, kInstPaddd, XmmReg, Mem)

  //! @brief Packed qword add (SSE2).
  INST_2x(paddq, kInstPaddq, MmReg, MmReg)
  //! @overload
  INST_2x(paddq, kInstPaddq, MmReg, Mem)

  //! @brief Packed qword add (SSE2).
  INST_2x(paddq, kInstPaddq, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddq, kInstPaddq, XmmReg, Mem)

  //! @brief Packed add with saturation (SSE2).
  INST_2x(paddsb, kInstPaddsb, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddsb, kInstPaddsb, XmmReg, Mem)

  //! @brief Packed add with saturation (SSE2).
  INST_2x(paddsw, kInstPaddsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddsw, kInstPaddsw, XmmReg, Mem)

  //! @brief Packed add unsigned with saturation (SSE2).
  INST_2x(paddusb, kInstPaddusb, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddusb, kInstPaddusb, XmmReg, Mem)

  //! @brief Packed add unsigned with saturation (SSE2).
  INST_2x(paddusw, kInstPaddusw, XmmReg, XmmReg)
  //! @overload
  INST_2x(paddusw, kInstPaddusw, XmmReg, Mem)

  //! @brief And (SSE2).
  INST_2x(pand, kInstPand, XmmReg, XmmReg)
  //! @overload
  INST_2x(pand, kInstPand, XmmReg, Mem)

  //! @brief And-not (SSE2).
  INST_2x(pandn, kInstPandn, XmmReg, XmmReg)
  //! @overload
  INST_2x(pandn, kInstPandn, XmmReg, Mem)

  //! @brief Spin loop hint (SSE2).
  INST_0x(pause, kInstPause)

  //! @brief Packed average (SSE2).
  INST_2x(pavgb, kInstPavgb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pavgb, kInstPavgb, XmmReg, Mem)

  //! @brief Packed average (SSE2).
  INST_2x(pavgw, kInstPavgw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pavgw, kInstPavgw, XmmReg, Mem)

  //! @brief Packed compare bytes for equal (SSE2).
  INST_2x(pcmpeqb, kInstPcmpeqb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpeqb, kInstPcmpeqb, XmmReg, Mem)

  //! @brief Packed compare words for equal (SSE2).
  INST_2x(pcmpeqw, kInstPcmpeqw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpeqw, kInstPcmpeqw, XmmReg, Mem)

  //! @brief Packed compare dwords for equal (SSE2).
  INST_2x(pcmpeqd, kInstPcmpeqd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpeqd, kInstPcmpeqd, XmmReg, Mem)

  //! @brief Packed compare bytes for greater than (SSE2).
  INST_2x(pcmpgtb, kInstPcmpgtb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpgtb, kInstPcmpgtb, XmmReg, Mem)

  //! @brief Packed compare words for greater than (SSE2).
  INST_2x(pcmpgtw, kInstPcmpgtw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpgtw, kInstPcmpgtw, XmmReg, Mem)

  //! @brief Packed compare dwords for greater than (SSE2).
  INST_2x(pcmpgtd, kInstPcmpgtd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpgtd, kInstPcmpgtd, XmmReg, Mem)

  //! @brief Extract word (SSE2).
  INST_3i(pextrw, kInstPextrw, GpReg, XmmReg, Imm)

  //! @brief Insert word (SSE2).
  INST_3i(pinsrw, kInstPinsrw, XmmReg, GpReg, Imm)
  //! @overload
  INST_3i(pinsrw, kInstPinsrw, XmmReg, Mem, Imm)

  //! @brief Packed signed integer word maximum (SSE2).
  INST_2x(pmaxsw, kInstPmaxsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaxsw, kInstPmaxsw, XmmReg, Mem)

  //! @brief Packed unsigned integer byte maximum (SSE2).
  INST_2x(pmaxub, kInstPmaxub, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaxub, kInstPmaxub, XmmReg, Mem)

  //! @brief Packed signed integer word minimum (SSE2).
  INST_2x(pminsw, kInstPminsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pminsw, kInstPminsw, XmmReg, Mem)

  //! @brief Packed unsigned integer byte minimum (SSE2).
  INST_2x(pminub, kInstPminub, XmmReg, XmmReg)
  //! @overload
  INST_2x(pminub, kInstPminub, XmmReg, Mem)

  //! @brief Move byte mask (SSE2).
  INST_2x(pmovmskb, kInstPmovmskb, GpReg, XmmReg)

  //! @brief Packed multiply high (SSE2).
  INST_2x(pmulhw, kInstPmulhw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmulhw, kInstPmulhw, XmmReg, Mem)

  //! @brief Packed multiply high unsigned (SSE2).
  INST_2x(pmulhuw, kInstPmulhuw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmulhuw, kInstPmulhuw, XmmReg, Mem)

  //! @brief Packed multiply low (SSE2).
  INST_2x(pmullw, kInstPmullw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmullw, kInstPmullw, XmmReg, Mem)

  //! @brief Packed multiply to qword (SSE2).
  INST_2x(pmuludq, kInstPmuludq, MmReg, MmReg)
  //! @overload
  INST_2x(pmuludq, kInstPmuludq, MmReg, Mem)

  //! @brief Packed multiply to qword (SSE2).
  INST_2x(pmuludq, kInstPmuludq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmuludq, kInstPmuludq, XmmReg, Mem)

  //! @brief Or (SSE2).
  INST_2x(por, kInstPor, XmmReg, XmmReg)
  //! @overload
  INST_2x(por, kInstPor, XmmReg, Mem)

  //! @brief Packed shift left logical (SSE2).
  INST_2x(pslld, kInstPslld, XmmReg, XmmReg)
  //! @overload
  INST_2x(pslld, kInstPslld, XmmReg, Mem)
  //! @overload
  INST_2i(pslld, kInstPslld, XmmReg, Imm)

  //! @brief Packed shift left logical (SSE2).
  INST_2x(psllq, kInstPsllq, XmmReg, XmmReg)
  //! @overload
  INST_2x(psllq, kInstPsllq, XmmReg, Mem)
  //! @overload
  INST_2i(psllq, kInstPsllq, XmmReg, Imm)

  //! @brief Packed shift left logical (SSE2).
  INST_2x(psllw, kInstPsllw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psllw, kInstPsllw, XmmReg, Mem)
  //! @overload
  INST_2i(psllw, kInstPsllw, XmmReg, Imm)

  //! @brief Packed shift left logical (SSE2).
  INST_2i(pslldq, kInstPslldq, XmmReg, Imm)

  //! @brief Packed shift right arithmetic (SSE2).
  INST_2x(psrad, kInstPsrad, XmmReg, XmmReg)
  //! @overload
  INST_2x(psrad, kInstPsrad, XmmReg, Mem)
  //! @overload
  INST_2i(psrad, kInstPsrad, XmmReg, Imm)

  //! @brief Packed shift right arithmetic (SSE2).
  INST_2x(psraw, kInstPsraw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psraw, kInstPsraw, XmmReg, Mem)
  //! @overload
  INST_2i(psraw, kInstPsraw, XmmReg, Imm)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubb, kInstPsubb, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubb, kInstPsubb, XmmReg, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubw, kInstPsubw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubw, kInstPsubw, XmmReg, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubd, kInstPsubd, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubd, kInstPsubd, XmmReg, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubq, kInstPsubq, MmReg, MmReg)
  //! @overload
  INST_2x(psubq, kInstPsubq, MmReg, Mem)

  //! @brief Packed subtract (SSE2).
  INST_2x(psubq, kInstPsubq, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubq, kInstPsubq, XmmReg, Mem)

  //! @brief Packed multiply and add (SSE2).
  INST_2x(pmaddwd, kInstPmaddwd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaddwd, kInstPmaddwd, XmmReg, Mem)

  //! @brief Shuffle packed dwords (SSE2).
  INST_3i(pshufd, kInstPshufd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pshufd, kInstPshufd, XmmReg, Mem, Imm)

  //! @brief Shuffle packed high words (SSE2).
  INST_3i(pshufhw, kInstPshufhw, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pshufhw, kInstPshufhw, XmmReg, Mem, Imm)

  //! @brief Shuffle packed low Words (SSE2).
  INST_3i(pshuflw, kInstPshuflw, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pshuflw, kInstPshuflw, XmmReg, Mem, Imm)

  //! @brief Packed shift right logical (SSE2).
  INST_2x(psrld, kInstPsrld, XmmReg, XmmReg)
  //! @overload
  INST_2x(psrld, kInstPsrld, XmmReg, Mem)
  //! @overload
  INST_2i(psrld, kInstPsrld, XmmReg, Imm)

  //! @brief Packed shift right logical (SSE2).
  INST_2x(psrlq, kInstPsrlq, XmmReg, XmmReg)
  //! @overload
  INST_2x(psrlq, kInstPsrlq, XmmReg, Mem)
  //! @overload
  INST_2i(psrlq, kInstPsrlq, XmmReg, Imm)

  //! @brief OWord shift right logical (SSE2).
  INST_2i(psrldq, kInstPsrldq, XmmReg, Imm)

  //! @brief Packed shift right logical (SSE2).
  INST_2x(psrlw, kInstPsrlw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psrlw, kInstPsrlw, XmmReg, Mem)
  //! @overload
  INST_2i(psrlw, kInstPsrlw, XmmReg, Imm)

  //! @brief Packed subtract with saturation (SSE2).
  INST_2x(psubsb, kInstPsubsb, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubsb, kInstPsubsb, XmmReg, Mem)

  //! @brief Packed subtract with saturation (SSE2).
  INST_2x(psubsw, kInstPsubsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubsw, kInstPsubsw, XmmReg, Mem)

  //! @brief Packed subtract with unsigned saturation (SSE2).
  INST_2x(psubusb, kInstPsubusb, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubusb, kInstPsubusb, XmmReg, Mem)

  //! @brief Packed subtract with unsigned saturation (SSE2).
  INST_2x(psubusw, kInstPsubusw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psubusw, kInstPsubusw, XmmReg, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhbw, kInstPunpckhbw, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpckhbw, kInstPunpckhbw, XmmReg, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhwd, kInstPunpckhwd, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpckhwd, kInstPunpckhwd, XmmReg, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhdq, kInstPunpckhdq, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpckhdq, kInstPunpckhdq, XmmReg, Mem)

  //! @brief Unpack high data (SSE2).
  INST_2x(punpckhqdq, kInstPunpckhqdq, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpckhqdq, kInstPunpckhqdq, XmmReg, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpcklbw, kInstPunpcklbw, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpcklbw, kInstPunpcklbw, XmmReg, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpcklwd, kInstPunpcklwd, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpcklwd, kInstPunpcklwd, XmmReg, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpckldq, kInstPunpckldq, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpckldq, kInstPunpckldq, XmmReg, Mem)

  //! @brief Unpack low data (SSE2).
  INST_2x(punpcklqdq, kInstPunpcklqdq, XmmReg, XmmReg)
  //! @overload
  INST_2x(punpcklqdq, kInstPunpcklqdq, XmmReg, Mem)

  //! @brief Xor (SSE2).
  INST_2x(pxor, kInstPxor, XmmReg, XmmReg)
  //! @overload
  INST_2x(pxor, kInstPxor, XmmReg, Mem)

  //! @brief Shuffle DP-FP (SSE2).
  INST_3i(shufpd, kInstShufpd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(shufpd, kInstShufpd, XmmReg, Mem, Imm)

  //! @brief Compute square roots of packed DP-FP Values (SSE2).
  INST_2x(sqrtpd, kInstSqrtpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(sqrtpd, kInstSqrtpd, XmmReg, Mem)

  //! @brief Compute square root of scalar DP-FP value (SSE2).
  INST_2x(sqrtsd, kInstSqrtsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(sqrtsd, kInstSqrtsd, XmmReg, Mem)

  //! @brief Packed DP-FP subtract (SSE2).
  INST_2x(subpd, kInstSubpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(subpd, kInstSubpd, XmmReg, Mem)

  //! @brief Scalar DP-FP subtract (SSE2).
  INST_2x(subsd, kInstSubsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(subsd, kInstSubsd, XmmReg, Mem)

  //! @brief Scalar unordered DP-FP compare and set EFLAGS (SSE2).
  INST_2x(ucomisd, kInstUcomisd, XmmReg, XmmReg)
  //! @overload
  INST_2x(ucomisd, kInstUcomisd, XmmReg, Mem)

  //! @brief Unpack and interleave high packed DP-FP values (SSE2).
  INST_2x(unpckhpd, kInstUnpckhpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(unpckhpd, kInstUnpckhpd, XmmReg, Mem)

  //! @brief Unpack and interleave low packed DP-FP values (SSE2).
  INST_2x(unpcklpd, kInstUnpcklpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(unpcklpd, kInstUnpcklpd, XmmReg, Mem)

  //! @brief Xor for DP-FP data (SSE2).
  INST_2x(xorpd, kInstXorpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(xorpd, kInstXorpd, XmmReg, Mem)

  // --------------------------------------------------------------------------
  // [SSE3]
  // --------------------------------------------------------------------------

  //! @brief Packed DP-FP add/subtract (SSE3).
  INST_2x(addsubpd, kInstAddsubpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(addsubpd, kInstAddsubpd, XmmReg, Mem)

  //! @brief Packed SP-FP add/subtract (SSE3).
  INST_2x(addsubps, kInstAddsubps, XmmReg, XmmReg)
  //! @overload
  INST_2x(addsubps, kInstAddsubps, XmmReg, Mem)

  //! @brief Store integer with truncation (SSE3).
  INST_1x(fisttp, kInstFisttp, Mem)

  //! @brief Packed DP-FP horizontal add (SSE3).
  INST_2x(haddpd, kInstHaddpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(haddpd, kInstHaddpd, XmmReg, Mem)

  //! @brief Packed SP-FP horizontal add (SSE3).
  INST_2x(haddps, kInstHaddps, XmmReg, XmmReg)
  //! @overload
  INST_2x(haddps, kInstHaddps, XmmReg, Mem)

  //! @brief Packed DP-FP horizontal subtract (SSE3).
  INST_2x(hsubpd, kInstHsubpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(hsubpd, kInstHsubpd, XmmReg, Mem)

  //! @brief Packed SP-FP horizontal subtract (SSE3).
  INST_2x(hsubps, kInstHsubps, XmmReg, XmmReg)
  //! @overload
  INST_2x(hsubps, kInstHsubps, XmmReg, Mem)

  //! @brief Load unaligned integer 128 bits (SSE3).
  INST_2x(lddqu, kInstLddqu, XmmReg, Mem)

  //! @brief Setup monitor address (SSE3).
  INST_0x(monitor, kInstMonitor)

  //! @brief Move one DP-FP and duplicate (SSE3).
  INST_2x(movddup, kInstMovddup, XmmReg, XmmReg)
  //! @overload
  INST_2x(movddup, kInstMovddup, XmmReg, Mem)

  //! @brief Move packed SP-FP high and duplicate (SSE3).
  INST_2x(movshdup, kInstMovshdup, XmmReg, XmmReg)
  //! @overload
  INST_2x(movshdup, kInstMovshdup, XmmReg, Mem)

  //! @brief Move packed SP-FP low and duplicate (SSE3).
  INST_2x(movsldup, kInstMovsldup, XmmReg, XmmReg)
  //! @overload
  INST_2x(movsldup, kInstMovsldup, XmmReg, Mem)

  //! @brief Monitor wait (SSE3).
  INST_0x(mwait, kInstMwait)

  // --------------------------------------------------------------------------
  // [SSSE3]
  // --------------------------------------------------------------------------

  //! @brief Packed sign (SSSE3).
  INST_2x(psignb, kInstPsignb, MmReg, MmReg)
  //! @overload
  INST_2x(psignb, kInstPsignb, MmReg, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignb, kInstPsignb, XmmReg, XmmReg)
  //! @overload
  INST_2x(psignb, kInstPsignb, XmmReg, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignw, kInstPsignw, MmReg, MmReg)
  //! @overload
  INST_2x(psignw, kInstPsignw, MmReg, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignw, kInstPsignw, XmmReg, XmmReg)
  //! @overload
  INST_2x(psignw, kInstPsignw, XmmReg, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignd, kInstPsignd, MmReg, MmReg)
  //! @overload
  INST_2x(psignd, kInstPsignd, MmReg, Mem)

  //! @brief Packed sign (SSSE3).
  INST_2x(psignd, kInstPsignd, XmmReg, XmmReg)
  //! @overload
  INST_2x(psignd, kInstPsignd, XmmReg, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddw, kInstPhaddw, MmReg, MmReg)
  //! @overload
  INST_2x(phaddw, kInstPhaddw, MmReg, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddw, kInstPhaddw, XmmReg, XmmReg)
  //! @overload
  INST_2x(phaddw, kInstPhaddw, XmmReg, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddd, kInstPhaddd, MmReg, MmReg)
  //! @overload
  INST_2x(phaddd, kInstPhaddd, MmReg, Mem)

  //! @brief Packed horizontal add (SSSE3).
  INST_2x(phaddd, kInstPhaddd, XmmReg, XmmReg)
  //! @overload
  INST_2x(phaddd, kInstPhaddd, XmmReg, Mem)

  //! @brief Packed horizontal add and saturate (SSSE3).
  INST_2x(phaddsw, kInstPhaddsw, MmReg, MmReg)
  //! @overload
  INST_2x(phaddsw, kInstPhaddsw, MmReg, Mem)

  //! @brief Packed horizontal add and saturate (SSSE3).
  INST_2x(phaddsw, kInstPhaddsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(phaddsw, kInstPhaddsw, XmmReg, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubw, kInstPhsubw, MmReg, MmReg)
  //! @overload
  INST_2x(phsubw, kInstPhsubw, MmReg, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubw, kInstPhsubw, XmmReg, XmmReg)
  //! @overload
  INST_2x(phsubw, kInstPhsubw, XmmReg, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubd, kInstPhsubd, MmReg, MmReg)
  //! @overload
  INST_2x(phsubd, kInstPhsubd, MmReg, Mem)

  //! @brief Packed horizontal subtract (SSSE3).
  INST_2x(phsubd, kInstPhsubd, XmmReg, XmmReg)
  //! @overload
  INST_2x(phsubd, kInstPhsubd, XmmReg, Mem)

  //! @brief Packed horizontal subtract and saturate (SSSE3).
  INST_2x(phsubsw, kInstPhsubsw, MmReg, MmReg)
  //! @overload
  INST_2x(phsubsw, kInstPhsubsw, MmReg, Mem)

  //! @brief Packed horizontal subtract and saturate (SSSE3).
  INST_2x(phsubsw, kInstPhsubsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(phsubsw, kInstPhsubsw, XmmReg, Mem)

  //! @brief Multiply and add packed signed and unsigned bytes (SSSE3).
  INST_2x(pmaddubsw, kInstPmaddubsw, MmReg, MmReg)
  //! @overload
  INST_2x(pmaddubsw, kInstPmaddubsw, MmReg, Mem)

  //! @brief Multiply and add packed signed and unsigned bytes (SSSE3).
  INST_2x(pmaddubsw, kInstPmaddubsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaddubsw, kInstPmaddubsw, XmmReg, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsb, kInstPabsb, MmReg, MmReg)
  //! @overload
  INST_2x(pabsb, kInstPabsb, MmReg, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsb, kInstPabsb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pabsb, kInstPabsb, XmmReg, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsw, kInstPabsw, MmReg, MmReg)
  //! @overload
  INST_2x(pabsw, kInstPabsw, MmReg, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsw, kInstPabsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pabsw, kInstPabsw, XmmReg, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsd, kInstPabsd, MmReg, MmReg)
  //! @overload
  INST_2x(pabsd, kInstPabsd, MmReg, Mem)

  //! @brief Packed absolute value (SSSE3).
  INST_2x(pabsd, kInstPabsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pabsd, kInstPabsd, XmmReg, Mem)

  //! @brief Packed multiply high with round and scale (SSSE3).
  INST_2x(pmulhrsw, kInstPmulhrsw, MmReg, MmReg)
  //! @overload
  INST_2x(pmulhrsw, kInstPmulhrsw, MmReg, Mem)

  //! @brief Packed multiply high with round and scale (SSSE3).
  INST_2x(pmulhrsw, kInstPmulhrsw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmulhrsw, kInstPmulhrsw, XmmReg, Mem)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_2x(pshufb, kInstPshufb, MmReg, MmReg)
  //! @overload
  INST_2x(pshufb, kInstPshufb, MmReg, Mem)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_2x(pshufb, kInstPshufb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pshufb, kInstPshufb, XmmReg, Mem)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_3i(palignr, kInstPalignr, MmReg, MmReg, Imm)
  //! @overload
  INST_3i(palignr, kInstPalignr, MmReg, Mem, Imm)

  //! @brief Packed shuffle bytes (SSSE3).
  INST_3i(palignr, kInstPalignr, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(palignr, kInstPalignr, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [SSE4.1]
  // --------------------------------------------------------------------------

  //! @brief Blend packed DP-FP values (SSE4.1).
  INST_3i(blendpd, kInstBlendpd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(blendpd, kInstBlendpd, XmmReg, Mem, Imm)

  //! @brief Blend packed SP-FP values (SSE4.1).
  INST_3i(blendps, kInstBlendps, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(blendps, kInstBlendps, XmmReg, Mem, Imm)

  //! @brief Variable blend packed DP-FP values (SSE4.1).
  INST_2x(blendvpd, kInstBlendvpd, XmmReg, XmmReg)
  //! @overload
  INST_2x(blendvpd, kInstBlendvpd, XmmReg, Mem)

  //! @brief Variable blend packed SP-FP values (SSE4.1).
  INST_2x(blendvps, kInstBlendvps, XmmReg, XmmReg)
  //! @overload
  INST_2x(blendvps, kInstBlendvps, XmmReg, Mem)

  //! @brief Dot product of packed DP-FP values (SSE4.1).
  INST_3i(dppd, kInstDppd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(dppd, kInstDppd, XmmReg, Mem, Imm)

  //! @brief Dot product of packed SP-FP values (SSE4.1).
  INST_3i(dpps, kInstDpps, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(dpps, kInstDpps, XmmReg, Mem, Imm)

  //! @brief Extract packed SP-FP value (SSE4.1).
  INST_3i(extractps, kInstExtractps, GpReg, XmmReg, Imm)
  //! @overload
  INST_3i(extractps, kInstExtractps, Mem, XmmReg, Imm)

  //! @brief Load oword non-temporal aligned hint (SSE4.1).
  INST_2x(movntdqa, kInstMovntdqa, XmmReg, Mem)

  //! @brief Compute multiple packed sums of absolute difference (SSE4.1).
  INST_3i(mpsadbw, kInstMpsadbw, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(mpsadbw, kInstMpsadbw, XmmReg, Mem, Imm)

  //! @brief Pack with unsigned saturation (SSE4.1).
  INST_2x(packusdw, kInstPackusdw, XmmReg, XmmReg)
  //! @overload
  INST_2x(packusdw, kInstPackusdw, XmmReg, Mem)

  //! @brief Variable blend packed bytes (SSE4.1).
  INST_2x(pblendvb, kInstPblendvb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pblendvb, kInstPblendvb, XmmReg, Mem)

  //! @brief Blend packed words (SSE4.1).
  INST_3i(pblendw, kInstPblendw, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pblendw, kInstPblendw, XmmReg, Mem, Imm)

  //! @brief Compare packed qword data for equal (SSE4.1).
  INST_2x(pcmpeqq, kInstPcmpeqq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpeqq, kInstPcmpeqq, XmmReg, Mem)

  //! @brief Extract byte (SSE4.1).
  INST_3i(pextrb, kInstPextrb, GpReg, XmmReg, Imm)
  //! @overload
  INST_3i(pextrb, kInstPextrb, Mem, XmmReg, Imm)

  //! @brief Extract dword (SSE4.1).
  INST_3i(pextrd, kInstPextrd, GpReg, XmmReg, Imm)
  //! @overload
  INST_3i(pextrd, kInstPextrd, Mem, XmmReg, Imm)

  //! @brief Extract qword (SSE4.1).
  INST_3i(pextrq, kInstPextrq, GpReg, XmmReg, Imm)
  //! @overload
  INST_3i(pextrq, kInstPextrq, Mem, XmmReg, Imm)

  //! @brief Extract word (SSE4.1).
  INST_3i(pextrw, kInstPextrw, Mem, XmmReg, Imm)

  //! @brief Packed horizontal word minimum (SSE4.1).
  INST_2x(phminposuw, kInstPhminposuw, XmmReg, XmmReg)
  //! @overload
  INST_2x(phminposuw, kInstPhminposuw, XmmReg, Mem)

  //! @brief Insert byte (SSE4.1).
  INST_3i(pinsrb, kInstPinsrb, XmmReg, GpReg, Imm)
  //! @overload
  INST_3i(pinsrb, kInstPinsrb, XmmReg, Mem, Imm)

  //! @brief Insert dword (SSE4.1).
  INST_3i(pinsrd, kInstPinsrd, XmmReg, GpReg, Imm)
  //! @overload
  INST_3i(pinsrd, kInstPinsrd, XmmReg, Mem, Imm)

  //! @brief Insert qword (SSE4.1).
  INST_3i(pinsrq, kInstPinsrq, XmmReg, GpReg, Imm)
  //! @overload
  INST_3i(pinsrq, kInstPinsrq, XmmReg, Mem, Imm)

  //! @brief Maximum of packed word integers (SSE4.1).
  INST_2x(pmaxuw, kInstPmaxuw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaxuw, kInstPmaxuw, XmmReg, Mem)

  //! @brief Maximum of packed signed byte integers (SSE4.1).
  INST_2x(pmaxsb, kInstPmaxsb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaxsb, kInstPmaxsb, XmmReg, Mem)

  //! @brief Maximum of packed signed dword integers (SSE4.1).
  INST_2x(pmaxsd, kInstPmaxsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaxsd, kInstPmaxsd, XmmReg, Mem)

  //! @brief Maximum of packed unsigned dword integers (SSE4.1).
  INST_2x(pmaxud, kInstPmaxud, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmaxud,kInstPmaxud , XmmReg, Mem)

  //! @brief Minimum of packed signed byte integers (SSE4.1).
  INST_2x(pminsb, kInstPminsb, XmmReg, XmmReg)
  //! @overload
  INST_2x(pminsb, kInstPminsb, XmmReg, Mem)

  //! @brief Minimum of packed word integers (SSE4.1).
  INST_2x(pminuw, kInstPminuw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pminuw, kInstPminuw, XmmReg, Mem)

  //! @brief Minimum of packed dword integers (SSE4.1).
  INST_2x(pminud, kInstPminud, XmmReg, XmmReg)
  //! @overload
  INST_2x(pminud, kInstPminud, XmmReg, Mem)

  //! @brief Minimum of packed dword integers (SSE4.1).
  INST_2x(pminsd, kInstPminsd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pminsd, kInstPminsd, XmmReg, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxbw, kInstPmovsxbw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovsxbw, kInstPmovsxbw, XmmReg, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxbd, kInstPmovsxbd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovsxbd, kInstPmovsxbd, XmmReg, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxbq, kInstPmovsxbq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovsxbq, kInstPmovsxbq, XmmReg, Mem)

  //! @brief Packed move with sign extend (SSE4.1).
  INST_2x(pmovsxwd, kInstPmovsxwd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovsxwd, kInstPmovsxwd, XmmReg, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovsxwq, kInstPmovsxwq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovsxwq, kInstPmovsxwq, XmmReg, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovsxdq, kInstPmovsxdq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovsxdq, kInstPmovsxdq, XmmReg, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxbw, kInstPmovzxbw, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovzxbw, kInstPmovzxbw, XmmReg, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxbd, kInstPmovzxbd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovzxbd, kInstPmovzxbd, XmmReg, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxbq, kInstPmovzxbq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovzxbq, kInstPmovzxbq, XmmReg, Mem)

  //! @brief Packed move with zero extend (SSE4.1).
  INST_2x(pmovzxwd, kInstPmovzxwd, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovzxwd, kInstPmovzxwd, XmmReg, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovzxwq, kInstPmovzxwq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovzxwq, kInstPmovzxwq, XmmReg, Mem)

  //! @brief (SSE4.1).
  INST_2x(pmovzxdq, kInstPmovzxdq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmovzxdq, kInstPmovzxdq, XmmReg, Mem)

  //! @brief Multiply packed signed qword integers (SSE4.1).
  INST_2x(pmuldq, kInstPmuldq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmuldq, kInstPmuldq, XmmReg, Mem)

  //! @brief Multiply packed signed integers and store low result (SSE4.1).
  INST_2x(pmulld, kInstPmulld, XmmReg, XmmReg)
  //! @overload
  INST_2x(pmulld, kInstPmulld, XmmReg, Mem)

  //! @brief Logical compare (SSE4.1).
  INST_2x(ptest, kInstPtest, XmmReg, XmmReg)
  //! @overload
  INST_2x(ptest, kInstPtest, XmmReg, Mem)

  //! @brief Round packed SP-FP values (SSE4.1).
  INST_3i(roundps, kInstRoundps, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(roundps, kInstRoundps, XmmReg, Mem, Imm)

  //! @brief Round scalar SP-FP values (SSE4.1).
  INST_3i(roundss, kInstRoundss, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(roundss, kInstRoundss, XmmReg, Mem, Imm)

  //! @brief Round packed DP-FP values (SSE4.1).
  INST_3i(roundpd, kInstRoundpd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(roundpd, kInstRoundpd, XmmReg, Mem, Imm)

  //! @brief Round scalar DP-FP values (SSE4.1).
  INST_3i(roundsd, kInstRoundsd, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(roundsd, kInstRoundsd, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [SSE4.2]
  // --------------------------------------------------------------------------

  //! @brief Packed compare explicit length strings, return index (SSE4.2).
  INST_3i(pcmpestri, kInstPcmpestri, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pcmpestri, kInstPcmpestri, XmmReg, Mem, Imm)

  //! @brief Packed compare explicit length strings, return mask (SSE4.2).
  INST_3i(pcmpestrm, kInstPcmpestrm, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pcmpestrm, kInstPcmpestrm, XmmReg, Mem, Imm)

  //! @brief Packed compare implicit length strings, return index (SSE4.2).
  INST_3i(pcmpistri, kInstPcmpistri, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pcmpistri, kInstPcmpistri, XmmReg, Mem, Imm)

  //! @brief Packed compare implicit length strings, return mask (SSE4.2).
  INST_3i(pcmpistrm, kInstPcmpistrm, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pcmpistrm, kInstPcmpistrm, XmmReg, Mem, Imm)

  //! @brief Compare packed data for greater than (SSE4.2).
  INST_2x(pcmpgtq, kInstPcmpgtq, XmmReg, XmmReg)
  //! @overload
  INST_2x(pcmpgtq, kInstPcmpgtq, XmmReg, Mem)

  // --------------------------------------------------------------------------
  // [AESNI]
  // --------------------------------------------------------------------------

  //! @brief Perform a single round of the AES decryption flow.
  INST_2x(aesdec, kInstAesdec, XmmReg, XmmReg)
  //! @overload
  INST_2x(aesdec, kInstAesdec, XmmReg, Mem)

  //! @brief Perform the last round of the AES decryption flow.
  INST_2x(aesdeclast, kInstAesdeclast, XmmReg, XmmReg)
  //! @overload
  INST_2x(aesdeclast, kInstAesdeclast, XmmReg, Mem)

  //! @brief Perform a single round of the AES encryption flow.
  INST_2x(aesenc, kInstAesenc, XmmReg, XmmReg)
  //! @overload
  INST_2x(aesenc, kInstAesenc, XmmReg, Mem)

  //! @brief Perform the last round of the AES encryption flow.
  INST_2x(aesenclast, kInstAesenclast, XmmReg, XmmReg)
  //! @overload
  INST_2x(aesenclast, kInstAesenclast, XmmReg, Mem)

  //! @brief Perform the InvMixColumns transformation.
  INST_2x(aesimc, kInstAesimc, XmmReg, XmmReg)
  //! @overload
  INST_2x(aesimc, kInstAesimc, XmmReg, Mem)

  //! @brief Assist in expanding the AES cipher key.
  INST_3i(aeskeygenassist, kInstAeskeygenassist, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(aeskeygenassist, kInstAeskeygenassist, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [PCLMULQDQ]
  // --------------------------------------------------------------------------

  //! @brief Carry-less multiplication quadword.
  INST_3i(pclmulqdq, kInstPclmulqdq, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(pclmulqdq, kInstPclmulqdq, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [AVX]
  // --------------------------------------------------------------------------

  INST_3x(vaddpd, kInstVaddpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vaddpd, kInstVaddpd, XmmReg, XmmReg, Mem)
  INST_3x(vaddpd, kInstVaddpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vaddpd, kInstVaddpd, YmmReg, YmmReg, Mem)

  INST_3x(vaddps, kInstVaddps, XmmReg, XmmReg, XmmReg)
  INST_3x(vaddps, kInstVaddps, XmmReg, XmmReg, Mem)
  INST_3x(vaddps, kInstVaddps, YmmReg, YmmReg, YmmReg)
  INST_3x(vaddps, kInstVaddps, YmmReg, YmmReg, Mem)

  INST_3x(vaddsd, kInstVaddsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vaddsd, kInstVaddsd, XmmReg, XmmReg, Mem)

  INST_3x(vaddss, kInstVaddss, XmmReg, XmmReg, XmmReg)
  INST_3x(vaddss, kInstVaddss, XmmReg, XmmReg, Mem)

  INST_3x(vaddsubpd, kInstVaddsubpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vaddsubpd, kInstVaddsubpd, XmmReg, XmmReg, Mem)
  INST_3x(vaddsubpd, kInstVaddsubpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vaddsubpd, kInstVaddsubpd, YmmReg, YmmReg, Mem)

  INST_3x(vaddsubps, kInstVaddsubps, XmmReg, XmmReg, XmmReg)
  INST_3x(vaddsubps, kInstVaddsubps, XmmReg, XmmReg, Mem)
  INST_3x(vaddsubps, kInstVaddsubps, YmmReg, YmmReg, YmmReg)
  INST_3x(vaddsubps, kInstVaddsubps, YmmReg, YmmReg, Mem)

  INST_3x(vandpd, kInstVandpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vandpd, kInstVandpd, XmmReg, XmmReg, Mem)
  INST_3x(vandpd, kInstVandpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vandpd, kInstVandpd, YmmReg, YmmReg, Mem)

  INST_3x(vandps, kInstVandps, XmmReg, XmmReg, XmmReg)
  INST_3x(vandps, kInstVandps, XmmReg, XmmReg, Mem)
  INST_3x(vandps, kInstVandps, YmmReg, YmmReg, YmmReg)
  INST_3x(vandps, kInstVandps, YmmReg, YmmReg, Mem)

  INST_3x(vandnpd, kInstVandnpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vandnpd, kInstVandnpd, XmmReg, XmmReg, Mem)
  INST_3x(vandnpd, kInstVandnpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vandnpd, kInstVandnpd, YmmReg, YmmReg, Mem)

  INST_3x(vandnps, kInstVandnps, XmmReg, XmmReg, XmmReg)
  INST_3x(vandnps, kInstVandnps, XmmReg, XmmReg, Mem)
  INST_3x(vandnps, kInstVandnps, YmmReg, YmmReg, YmmReg)
  INST_3x(vandnps, kInstVandnps, YmmReg, YmmReg, Mem)

  INST_4i(vblendpd, kInstVblendpd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vblendpd, kInstVblendpd, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vblendpd, kInstVblendpd, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vblendpd, kInstVblendpd, YmmReg, YmmReg, Mem, Imm)

  INST_4i(vblendps, kInstVblendps, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vblendps, kInstVblendps, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vblendps, kInstVblendps, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vblendps, kInstVblendps, YmmReg, YmmReg, Mem, Imm)

  INST_4x(vblendvpd, kInstVblendvpd, XmmReg, XmmReg, XmmReg, XmmReg)
  INST_4x(vblendvpd, kInstVblendvpd, XmmReg, XmmReg, Mem, XmmReg)
  INST_4x(vblendvpd, kInstVblendvpd, YmmReg, YmmReg, YmmReg, YmmReg)
  INST_4x(vblendvpd, kInstVblendvpd, YmmReg, YmmReg, Mem, YmmReg)

  INST_4x(vblendvps, kInstVblendvps, XmmReg, XmmReg, XmmReg, XmmReg)
  INST_4x(vblendvps, kInstVblendvps, XmmReg, XmmReg, Mem, XmmReg)
  INST_4x(vblendvps, kInstVblendvps, YmmReg, YmmReg, YmmReg, YmmReg)
  INST_4x(vblendvps, kInstVblendvps, YmmReg, YmmReg, Mem, YmmReg)

  INST_2x(vbroadcastf128, kInstVbroadcastf128, YmmReg, Mem)

  INST_2x(vbroadcastsd, kInstVbroadcastsd, YmmReg, Mem)

  INST_2x(vbroadcastss, kInstVbroadcastss, XmmReg, Mem)
  INST_2x(vbroadcastss, kInstVbroadcastss, YmmReg, Mem)

  INST_4i(vcmppd, kInstVcmppd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vcmppd, kInstVcmppd, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vcmppd, kInstVcmppd, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vcmppd, kInstVcmppd, YmmReg, YmmReg, Mem, Imm)

  INST_4i(vcmpps, kInstVcmpps, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vcmpps, kInstVcmpps, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vcmpps, kInstVcmpps, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vcmpps, kInstVcmpps, YmmReg, YmmReg, Mem, Imm)

  INST_4i(vcmpsd, kInstVcmpsd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vcmpsd, kInstVcmpsd, XmmReg, XmmReg, Mem, Imm)

  INST_4i(vcmpss, kInstVcmpss, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vcmpss, kInstVcmpss, XmmReg, XmmReg, Mem, Imm)

  INST_2x(vcomisd, kInstVcomisd, XmmReg, XmmReg)
  INST_2x(vcomisd, kInstVcomisd, XmmReg, Mem)

  INST_2x(vcomiss, kInstVcomiss, XmmReg, XmmReg)
  INST_2x(vcomiss, kInstVcomiss, XmmReg, Mem)

  INST_2x(vcvtdq2pd, kInstVcvtdq2pd, XmmReg, XmmReg)
  INST_2x(vcvtdq2pd, kInstVcvtdq2pd, XmmReg, Mem)
  INST_2x(vcvtdq2pd, kInstVcvtdq2pd, YmmReg, XmmReg)
  INST_2x(vcvtdq2pd, kInstVcvtdq2pd, YmmReg, Mem)

  INST_2x(vcvtdq2ps, kInstVcvtdq2ps, XmmReg, XmmReg)
  INST_2x(vcvtdq2ps, kInstVcvtdq2ps, XmmReg, Mem)
  INST_2x(vcvtdq2ps, kInstVcvtdq2ps, YmmReg, YmmReg)
  INST_2x(vcvtdq2ps, kInstVcvtdq2ps, YmmReg, Mem)

  INST_2x(vcvtpd2dq, kInstVcvtpd2dq, XmmReg, XmmReg)
  INST_2x(vcvtpd2dq, kInstVcvtpd2dq, XmmReg, YmmReg)
  INST_2x(vcvtpd2dq, kInstVcvtpd2dq, XmmReg, Mem)

  INST_2x(vcvtpd2ps, kInstVcvtpd2ps, XmmReg, XmmReg)
  INST_2x(vcvtpd2ps, kInstVcvtpd2ps, XmmReg, YmmReg)
  INST_2x(vcvtpd2ps, kInstVcvtpd2ps, XmmReg, Mem)

  INST_2x(vcvtps2dq, kInstVcvtps2dq, XmmReg, XmmReg)
  INST_2x(vcvtps2dq, kInstVcvtps2dq, XmmReg, Mem)
  INST_2x(vcvtps2dq, kInstVcvtps2dq, YmmReg, YmmReg)
  INST_2x(vcvtps2dq, kInstVcvtps2dq, YmmReg, Mem)

  INST_2x(vcvtps2pd, kInstVcvtps2pd, XmmReg, XmmReg)
  INST_2x(vcvtps2pd, kInstVcvtps2pd, XmmReg, Mem)
  INST_2x(vcvtps2pd, kInstVcvtps2pd, YmmReg, XmmReg)
  INST_2x(vcvtps2pd, kInstVcvtps2pd, YmmReg, Mem)

  INST_2x(vcvtsd2si, kInstVcvtsd2si, GpReg, XmmReg)
  INST_2x(vcvtsd2si, kInstVcvtsd2si, GpReg, Mem)

  INST_3x(vcvtsd2ss, kInstVcvtsd2ss, XmmReg, XmmReg, XmmReg)
  INST_3x(vcvtsd2ss, kInstVcvtsd2ss, XmmReg, XmmReg, Mem)

  INST_3x(vcvtsi2sd, kInstVcvtsi2sd, XmmReg, XmmReg, GpReg)
  INST_3x(vcvtsi2sd, kInstVcvtsi2sd, XmmReg, XmmReg, Mem)

  INST_3x(vcvtsi2ss, kInstVcvtsi2ss, XmmReg, XmmReg, GpReg)
  INST_3x(vcvtsi2ss, kInstVcvtsi2ss, XmmReg, XmmReg, Mem)

  INST_3x(vcvtss2sd, kInstVcvtss2sd, XmmReg, XmmReg, XmmReg)
  INST_3x(vcvtss2sd, kInstVcvtss2sd, XmmReg, XmmReg, Mem)

  INST_2x(vcvtss2si, kInstVcvtss2si, GpReg, XmmReg)
  INST_2x(vcvtss2si, kInstVcvtss2si, GpReg, Mem)

  INST_2x(vcvttpd2dq, kInstVcvttpd2dq, XmmReg, XmmReg)
  INST_2x(vcvttpd2dq, kInstVcvttpd2dq, XmmReg, YmmReg)
  INST_2x(vcvttpd2dq, kInstVcvttpd2dq, XmmReg, Mem)

  INST_2x(vcvttps2dq, kInstVcvttps2dq, XmmReg, XmmReg)
  INST_2x(vcvttps2dq, kInstVcvttps2dq, XmmReg, Mem)
  INST_2x(vcvttps2dq, kInstVcvttps2dq, YmmReg, YmmReg)
  INST_2x(vcvttps2dq, kInstVcvttps2dq, YmmReg, Mem)

  INST_2x(vcvttsd2si, kInstVcvttsd2si, GpReg, XmmReg)
  INST_2x(vcvttsd2si, kInstVcvttsd2si, GpReg, Mem)

  INST_2x(vcvttss2si, kInstVcvttss2si, GpReg, XmmReg)
  INST_2x(vcvttss2si, kInstVcvttss2si, GpReg, Mem)

  INST_3x(vdivpd, kInstVdivpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vdivpd, kInstVdivpd, XmmReg, XmmReg, Mem)
  INST_3x(vdivpd, kInstVdivpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vdivpd, kInstVdivpd, YmmReg, YmmReg, Mem)

  INST_3x(vdivps, kInstVdivps, XmmReg, XmmReg, XmmReg)
  INST_3x(vdivps, kInstVdivps, XmmReg, XmmReg, Mem)
  INST_3x(vdivps, kInstVdivps, YmmReg, YmmReg, YmmReg)
  INST_3x(vdivps, kInstVdivps, YmmReg, YmmReg, Mem)

  INST_3x(vdivsd, kInstVdivsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vdivsd, kInstVdivsd, XmmReg, XmmReg, Mem)

  INST_3x(vdivss, kInstVdivss, XmmReg, XmmReg, XmmReg)
  INST_3x(vdivss, kInstVdivss, XmmReg, XmmReg, Mem)

  INST_4i(vdppd, kInstVdppd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vdppd, kInstVdppd, XmmReg, XmmReg, Mem, Imm)

  INST_4i(vdpps, kInstVdpps, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vdpps, kInstVdpps, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vdpps, kInstVdpps, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vdpps, kInstVdpps, YmmReg, YmmReg, Mem, Imm)

  INST_3i(vextractf128, kInstVextractf128, XmmReg, YmmReg, Imm)
  INST_3i(vextractf128, kInstVextractf128, Mem, YmmReg, Imm)

  INST_3i(vextractps, kInstVextractps, GpReg, XmmReg, Imm)
  INST_3i(vextractps, kInstVextractps, Mem, XmmReg, Imm)

  INST_3x(vhaddpd, kInstVhaddpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vhaddpd, kInstVhaddpd, XmmReg, XmmReg, Mem)
  INST_3x(vhaddpd, kInstVhaddpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vhaddpd, kInstVhaddpd, YmmReg, YmmReg, Mem)

  INST_3x(vhaddps, kInstVhaddps, XmmReg, XmmReg, XmmReg)
  INST_3x(vhaddps, kInstVhaddps, XmmReg, XmmReg, Mem)
  INST_3x(vhaddps, kInstVhaddps, YmmReg, YmmReg, YmmReg)
  INST_3x(vhaddps, kInstVhaddps, YmmReg, YmmReg, Mem)

  INST_3x(vhsubpd, kInstVhsubpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vhsubpd, kInstVhsubpd, XmmReg, XmmReg, Mem)
  INST_3x(vhsubpd, kInstVhsubpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vhsubpd, kInstVhsubpd, YmmReg, YmmReg, Mem)

  INST_3x(vhsubps, kInstVhsubps, XmmReg, XmmReg, XmmReg)
  INST_3x(vhsubps, kInstVhsubps, XmmReg, XmmReg, Mem)
  INST_3x(vhsubps, kInstVhsubps, YmmReg, YmmReg, YmmReg)
  INST_3x(vhsubps, kInstVhsubps, YmmReg, YmmReg, Mem)

  INST_4i(vinsertf128, kInstVinsertf128, YmmReg, YmmReg, XmmReg, Imm)
  INST_4i(vinsertf128, kInstVinsertf128, YmmReg, YmmReg, Mem, Imm)

  INST_4i(vinsertps, kInstVinsertps, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vinsertps, kInstVinsertps, XmmReg, XmmReg, Mem, Imm)

  INST_2x(vlddqu, kInstVlddqu, XmmReg, Mem)
  INST_2x(vlddqu, kInstVlddqu, YmmReg, Mem)

  INST_1x(vldmxcsr, kInstVldmxcsr, Mem)

  INST_2x(vmaskmovdqu, kInstVmaskmovdqu, XmmReg, XmmReg)

  INST_3x(vmaskmovps, kInstVmaskmovps, XmmReg, XmmReg, Mem)
  INST_3x(vmaskmovps, kInstVmaskmovps, YmmReg, YmmReg, Mem)

  INST_3x(vmaskmovpd, kInstVmaskmovpd, XmmReg, XmmReg, Mem)
  INST_3x(vmaskmovpd, kInstVmaskmovpd, YmmReg, YmmReg, Mem)

  INST_3x(vmaskmovps, kInstVmaskmovps, Mem, XmmReg, XmmReg)
  INST_3x(vmaskmovps, kInstVmaskmovps, Mem, YmmReg, YmmReg)

  INST_3x(vmaskmovpd, kInstVmaskmovpd, Mem, XmmReg, XmmReg)
  INST_3x(vmaskmovpd, kInstVmaskmovpd, Mem, YmmReg, YmmReg)

  INST_3x(vmaxpd, kInstVmaxpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vmaxpd, kInstVmaxpd, XmmReg, XmmReg, Mem)
  INST_3x(vmaxpd, kInstVmaxpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vmaxpd, kInstVmaxpd, YmmReg, YmmReg, Mem)

  INST_3x(vmaxps, kInstVmaxps, XmmReg, XmmReg, XmmReg)
  INST_3x(vmaxps, kInstVmaxps, XmmReg, XmmReg, Mem)
  INST_3x(vmaxps, kInstVmaxps, YmmReg, YmmReg, YmmReg)
  INST_3x(vmaxps, kInstVmaxps, YmmReg, YmmReg, Mem)

  INST_3x(vmaxsd, kInstVmaxsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vmaxsd, kInstVmaxsd, XmmReg, XmmReg, Mem)

  INST_3x(vmaxss, kInstVmaxss, XmmReg, XmmReg, XmmReg)
  INST_3x(vmaxss, kInstVmaxss, XmmReg, XmmReg, Mem)

  INST_3x(vminpd, kInstVminpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vminpd, kInstVminpd, XmmReg, XmmReg, Mem)
  INST_3x(vminpd, kInstVminpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vminpd, kInstVminpd, YmmReg, YmmReg, Mem)

  INST_3x(vminps, kInstVminps, XmmReg, XmmReg, XmmReg)
  INST_3x(vminps, kInstVminps, XmmReg, XmmReg, Mem)
  INST_3x(vminps, kInstVminps, YmmReg, YmmReg, YmmReg)
  INST_3x(vminps, kInstVminps, YmmReg, YmmReg, Mem)

  INST_3x(vminsd, kInstVminsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vminsd, kInstVminsd, XmmReg, XmmReg, Mem)

  INST_3x(vminss, kInstVminss, XmmReg, XmmReg, XmmReg)
  INST_3x(vminss, kInstVminss, XmmReg, XmmReg, Mem)

  INST_2x(vmovapd, kInstVmovapd, XmmReg, XmmReg)
  INST_2x(vmovapd, kInstVmovapd, XmmReg, Mem)
  INST_2x(vmovapd, kInstVmovapd, Mem, XmmReg)
  INST_2x(vmovapd, kInstVmovapd, YmmReg, YmmReg)
  INST_2x(vmovapd, kInstVmovapd, YmmReg, Mem)
  INST_2x(vmovapd, kInstVmovapd, Mem, YmmReg)

  INST_2x(vmovaps, kInstVmovaps, XmmReg, XmmReg)
  INST_2x(vmovaps, kInstVmovaps, XmmReg, Mem)
  INST_2x(vmovaps, kInstVmovaps, Mem, XmmReg)
  INST_2x(vmovaps, kInstVmovaps, YmmReg, YmmReg)
  INST_2x(vmovaps, kInstVmovaps, YmmReg, Mem)
  INST_2x(vmovaps, kInstVmovaps, Mem, YmmReg)

  INST_2x(vmovd, kInstVmovd, XmmReg, GpReg)
  INST_2x(vmovd, kInstVmovd, XmmReg, Mem)
  INST_2x(vmovd, kInstVmovd, GpReg, XmmReg)
  INST_2x(vmovd, kInstVmovd, Mem, XmmReg)
  INST_2x(vmovq, kInstVmovq, XmmReg, XmmReg)
  INST_2x(vmovq, kInstVmovq, XmmReg, Mem)
  INST_2x(vmovq, kInstVmovq, Mem, XmmReg)

  INST_2x(vmovddup, kInstVmovddup, XmmReg, XmmReg)
  INST_2x(vmovddup, kInstVmovddup, XmmReg, Mem)
  INST_2x(vmovddup, kInstVmovddup, YmmReg, YmmReg)
  INST_2x(vmovddup, kInstVmovddup, YmmReg, Mem)

  INST_2x(vmovdqa, kInstVmovdqa, XmmReg, XmmReg)
  INST_2x(vmovdqa, kInstVmovdqa, XmmReg, Mem)
  INST_2x(vmovdqa, kInstVmovdqa, Mem, XmmReg)
  INST_2x(vmovdqa, kInstVmovdqa, YmmReg, YmmReg)
  INST_2x(vmovdqa, kInstVmovdqa, YmmReg, Mem)
  INST_2x(vmovdqa, kInstVmovdqa, Mem, YmmReg)

  INST_2x(vmovdqu, kInstVmovdqu, XmmReg, XmmReg)
  INST_2x(vmovdqu, kInstVmovdqu, XmmReg, Mem)
  INST_2x(vmovdqu, kInstVmovdqu, Mem, XmmReg)
  INST_2x(vmovdqu, kInstVmovdqu, YmmReg, YmmReg)
  INST_2x(vmovdqu, kInstVmovdqu, YmmReg, Mem)
  INST_2x(vmovdqu, kInstVmovdqu, Mem, YmmReg)

  INST_3x(vmovhlps, kInstVmovhlps, XmmReg, XmmReg, XmmReg)

  INST_3x(vmovhpd, kInstVmovhpd, XmmReg, XmmReg, Mem)
  INST_2x(vmovhpd, kInstVmovhpd, Mem, XmmReg)

  INST_3x(vmovhps, kInstVmovhps, XmmReg, XmmReg, Mem)
  INST_2x(vmovhps, kInstVmovhps, Mem, XmmReg)

  INST_3x(vmovlhps, kInstVmovlhps, XmmReg, XmmReg, XmmReg)

  INST_3x(vmovlpd, kInstVmovlpd, XmmReg, XmmReg, Mem)
  INST_2x(vmovlpd, kInstVmovlpd, Mem, XmmReg)

  INST_3x(vmovlps, kInstVmovlps, XmmReg, XmmReg, Mem)
  INST_2x(vmovlps, kInstVmovlps, Mem, XmmReg)

  INST_2x(vmovmskpd, kInstVmovmskpd, GpReg, XmmReg)
  INST_2x(vmovmskpd, kInstVmovmskpd, GpReg, YmmReg)

  INST_2x(vmovmskps, kInstVmovmskps, GpReg, XmmReg)
  INST_2x(vmovmskps, kInstVmovmskps, GpReg, YmmReg)

  INST_2x(vmovntdq, kInstVmovntdq, Mem, XmmReg)
  INST_2x(vmovntdq, kInstVmovntdq, Mem, YmmReg)

  INST_2x(vmovntdqa, kInstVmovntdqa, XmmReg, Mem)

  INST_2x(vmovntpd, kInstVmovntpd, Mem, XmmReg)
  INST_2x(vmovntpd, kInstVmovntpd, Mem, YmmReg)

  INST_2x(vmovntps, kInstVmovntps, Mem, XmmReg)
  INST_2x(vmovntps, kInstVmovntps, Mem, YmmReg)

  INST_3x(vmovsd, kInstVmovsd, XmmReg, XmmReg, XmmReg)
  INST_2x(vmovsd, kInstVmovsd, XmmReg, Mem)
  INST_2x(vmovsd, kInstVmovsd, Mem, XmmReg)

  INST_2x(vmovshdup, kInstVmovshdup, XmmReg, XmmReg)
  INST_2x(vmovshdup, kInstVmovshdup, XmmReg, Mem)
  INST_2x(vmovshdup, kInstVmovshdup, YmmReg, YmmReg)
  INST_2x(vmovshdup, kInstVmovshdup, YmmReg, Mem)

  INST_2x(vmovsldup, kInstVmovsldup, XmmReg, XmmReg)
  INST_2x(vmovsldup, kInstVmovsldup, XmmReg, Mem)
  INST_2x(vmovsldup, kInstVmovsldup, YmmReg, YmmReg)
  INST_2x(vmovsldup, kInstVmovsldup, YmmReg, Mem)

  INST_3x(vmovss, kInstVmovss, XmmReg, XmmReg, XmmReg)
  INST_2x(vmovss, kInstVmovss, XmmReg, Mem)
  INST_2x(vmovss, kInstVmovss, Mem, XmmReg)

  INST_2x(vmovupd, kInstVmovupd, XmmReg, XmmReg)
  INST_2x(vmovupd, kInstVmovupd, XmmReg, Mem)
  INST_2x(vmovupd, kInstVmovupd, Mem, XmmReg)
  INST_2x(vmovupd, kInstVmovupd, YmmReg, YmmReg)
  INST_2x(vmovupd, kInstVmovupd, YmmReg, Mem)
  INST_2x(vmovupd, kInstVmovupd, Mem, YmmReg)

  INST_2x(vmovups, kInstVmovups, XmmReg, XmmReg)
  INST_2x(vmovups, kInstVmovups, XmmReg, Mem)
  INST_2x(vmovups, kInstVmovups, Mem, XmmReg)
  INST_2x(vmovups, kInstVmovups, YmmReg, YmmReg)
  INST_2x(vmovups, kInstVmovups, YmmReg, Mem)
  INST_2x(vmovups, kInstVmovups, Mem, YmmReg)

  INST_4i(vmpsadbw, kInstVmpsadbw, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vmpsadbw, kInstVmpsadbw, XmmReg, XmmReg, Mem, Imm)

  INST_3x(vmulpd, kInstVmulpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vmulpd, kInstVmulpd, XmmReg, XmmReg, Mem)
  INST_3x(vmulpd, kInstVmulpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vmulpd, kInstVmulpd, YmmReg, YmmReg, Mem)

  INST_3x(vmulps, kInstVmulps, XmmReg, XmmReg, XmmReg)
  INST_3x(vmulps, kInstVmulps, XmmReg, XmmReg, Mem)
  INST_3x(vmulps, kInstVmulps, YmmReg, YmmReg, YmmReg)
  INST_3x(vmulps, kInstVmulps, YmmReg, YmmReg, Mem)

  INST_3x(vmulsd, kInstVmulsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vmulsd, kInstVmulsd, XmmReg, XmmReg, Mem)

  INST_3x(vmulss, kInstVmulss, XmmReg, XmmReg, XmmReg)
  INST_3x(vmulss, kInstVmulss, XmmReg, XmmReg, Mem)

  INST_3x(vorpd, kInstVorpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vorpd, kInstVorpd, XmmReg, XmmReg, Mem)
  INST_3x(vorpd, kInstVorpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vorpd, kInstVorpd, YmmReg, YmmReg, Mem)

  INST_3x(vorps, kInstVorps, XmmReg, XmmReg, XmmReg)
  INST_3x(vorps, kInstVorps, XmmReg, XmmReg, Mem)
  INST_3x(vorps, kInstVorps, YmmReg, YmmReg, YmmReg)
  INST_3x(vorps, kInstVorps, YmmReg, YmmReg, Mem)

  INST_2x(vpabsb, kInstVpabsb, XmmReg, XmmReg)
  INST_2x(vpabsb, kInstVpabsb, XmmReg, Mem)

  INST_2x(vpabsd, kInstVpabsd, XmmReg, XmmReg)
  INST_2x(vpabsd, kInstVpabsd, XmmReg, Mem)

  INST_2x(vpabsw, kInstVpabsw, XmmReg, XmmReg)
  INST_2x(vpabsw, kInstVpabsw, XmmReg, Mem)

  INST_3x(vpackssdw, kInstVpackssdw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpackssdw, kInstVpackssdw, XmmReg, XmmReg, Mem)

  INST_3x(vpacksswb, kInstVpacksswb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpacksswb, kInstVpacksswb, XmmReg, XmmReg, Mem)

  INST_3x(vpackusdw, kInstVpackusdw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpackusdw, kInstVpackusdw, XmmReg, XmmReg, Mem)

  INST_3x(vpackuswb, kInstVpackuswb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpackuswb, kInstVpackuswb, XmmReg, XmmReg, Mem)

  INST_3x(vpaddb, kInstVpaddb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddb, kInstVpaddb, XmmReg, XmmReg, Mem)

  INST_3x(vpaddd, kInstVpaddd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddd, kInstVpaddd, XmmReg, XmmReg, Mem)

  INST_3x(vpaddq, kInstVpaddq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddq, kInstVpaddq, XmmReg, XmmReg, Mem)

  INST_3x(vpaddw, kInstVpaddw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddw, kInstVpaddw, XmmReg, XmmReg, Mem)

  INST_3x(vpaddsb, kInstVpaddsb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddsb, kInstVpaddsb, XmmReg, XmmReg, Mem)

  INST_3x(vpaddsw, kInstVpaddsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddsw, kInstVpaddsw, XmmReg, XmmReg, Mem)

  INST_3x(vpaddusb, kInstVpaddusb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddusb, kInstVpaddusb, XmmReg, XmmReg, Mem)

  INST_3x(vpaddusw, kInstVpaddusw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpaddusw, kInstVpaddusw, XmmReg, XmmReg, Mem)

  INST_4i(vpalignr, kInstVpalignr, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vpalignr, kInstVpalignr, XmmReg, XmmReg, Mem, Imm)

  INST_3x(vpand, kInstVpand, XmmReg, XmmReg, XmmReg)
  INST_3x(vpand, kInstVpand, XmmReg, XmmReg, Mem)

  INST_3x(vpandn, kInstVpandn, XmmReg, XmmReg, XmmReg)
  INST_3x(vpandn, kInstVpandn, XmmReg, XmmReg, Mem)

  INST_3x(vpavgb, kInstVpavgb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpavgb, kInstVpavgb, XmmReg, XmmReg, Mem)

  INST_3x(vpavgw, kInstVpavgw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpavgw, kInstVpavgw, XmmReg, XmmReg, Mem)

  INST_4x(vpblendvb, kInstVpblendvb, XmmReg, XmmReg, XmmReg, XmmReg)
  INST_4x(vpblendvb, kInstVpblendvb, XmmReg, XmmReg, Mem, XmmReg)

  INST_4i(vpblendw, kInstVpblendw, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vpblendw, kInstVpblendw, XmmReg, XmmReg, Mem, Imm)

  INST_3x(vpcmpeqb, kInstVpcmpeqb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpeqb, kInstVpcmpeqb, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpeqd, kInstVpcmpeqd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpeqd, kInstVpcmpeqd, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpeqq, kInstVpcmpeqq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpeqq, kInstVpcmpeqq, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpeqw, kInstVpcmpeqw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpeqw, kInstVpcmpeqw, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpgtb, kInstVpcmpgtb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpgtb, kInstVpcmpgtb, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpgtd, kInstVpcmpgtd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpgtd, kInstVpcmpgtd, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpgtq, kInstVpcmpgtq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpgtq, kInstVpcmpgtq, XmmReg, XmmReg, Mem)

  INST_3x(vpcmpgtw, kInstVpcmpgtw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpcmpgtw, kInstVpcmpgtw, XmmReg, XmmReg, Mem)

  INST_3i(vpcmpestri, kInstVpcmpestri, XmmReg, XmmReg, Imm)
  INST_3i(vpcmpestri, kInstVpcmpestri, XmmReg, Mem, Imm)

  INST_3i(vpcmpestrm, kInstVpcmpestrm, XmmReg, XmmReg, Imm)
  INST_3i(vpcmpestrm, kInstVpcmpestrm, XmmReg, Mem, Imm)

  INST_3i(vpcmpistri, kInstVpcmpistri, XmmReg, XmmReg, Imm)
  INST_3i(vpcmpistri, kInstVpcmpistri, XmmReg, Mem, Imm)

  INST_3i(vpcmpistrm, kInstVpcmpistrm, XmmReg, XmmReg, Imm)
  INST_3i(vpcmpistrm, kInstVpcmpistrm, XmmReg, Mem, Imm)

  INST_3x(vpermilpd, kInstVpermilpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpermilpd, kInstVpermilpd, XmmReg, XmmReg, Mem)
  INST_3x(vpermilpd, kInstVpermilpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vpermilpd, kInstVpermilpd, YmmReg, YmmReg, Mem)
  INST_3i(vpermilpd, kInstVpermilpd, XmmReg, XmmReg, Imm)
  INST_3i(vpermilpd, kInstVpermilpd, XmmReg, Mem, Imm)
  INST_3i(vpermilpd, kInstVpermilpd, YmmReg, YmmReg, Imm)
  INST_3i(vpermilpd, kInstVpermilpd, YmmReg, Mem, Imm)

  INST_3x(vpermilps, kInstVpermilps, XmmReg, XmmReg, XmmReg)
  INST_3x(vpermilps, kInstVpermilps, XmmReg, XmmReg, Mem)
  INST_3x(vpermilps, kInstVpermilps, YmmReg, YmmReg, YmmReg)
  INST_3x(vpermilps, kInstVpermilps, YmmReg, YmmReg, Mem)
  INST_3i(vpermilps, kInstVpermilps, XmmReg, XmmReg, Imm)
  INST_3i(vpermilps, kInstVpermilps, XmmReg, Mem, Imm)
  INST_3i(vpermilps, kInstVpermilps, YmmReg, YmmReg, Imm)
  INST_3i(vpermilps, kInstVpermilps, YmmReg, Mem, Imm)

  INST_4i(vperm2f128, kInstVperm2f128, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vperm2f128, kInstVperm2f128, YmmReg, YmmReg, Mem, Imm)

  INST_3i(vpextrb, kInstVpextrb, GpReg, XmmReg, Imm)
  INST_3i(vpextrb, kInstVpextrb, Mem, XmmReg, Imm)

  INST_3i(vpextrd, kInstVpextrd, GpReg, XmmReg, Imm)
  INST_3i(vpextrd, kInstVpextrd, Mem, XmmReg, Imm)

  INST_3i(vpextrw, kInstVpextrw, GpReg, XmmReg, Imm)
  INST_3i(vpextrw, kInstVpextrw, Mem, XmmReg, Imm)

  INST_3x(vphaddd, kInstVphaddd, XmmReg, XmmReg, XmmReg)
  INST_3x(vphaddd, kInstVphaddd, XmmReg, XmmReg, Mem)

  INST_3x(vphaddsw, kInstVphaddsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vphaddsw, kInstVphaddsw, XmmReg, XmmReg, Mem)

  INST_3x(vphaddw, kInstVphaddw, XmmReg, XmmReg, XmmReg)
  INST_3x(vphaddw, kInstVphaddw, XmmReg, XmmReg, Mem)

  INST_2x(vphminposuw, kInstVphminposuw, XmmReg, XmmReg)
  INST_2x(vphminposuw, kInstVphminposuw, XmmReg, Mem)

  INST_3x(vphsubd, kInstVphsubd, XmmReg, XmmReg, XmmReg)
  INST_3x(vphsubd, kInstVphsubd, XmmReg, XmmReg, Mem)

  INST_3x(vphsubsw, kInstVphsubsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vphsubsw, kInstVphsubsw, XmmReg, XmmReg, Mem)

  INST_3x(vphsubw, kInstVphsubw, XmmReg, XmmReg, XmmReg)
  INST_3x(vphsubw, kInstVphsubw, XmmReg, XmmReg, Mem)

  INST_4i(vpinsrb, kInstVpinsrb, XmmReg, XmmReg, GpReg, Imm)
  INST_4i(vpinsrb, kInstVpinsrb, XmmReg, XmmReg, Mem, Imm)

  INST_4i(vpinsrd, kInstVpinsrd, XmmReg, XmmReg, GpReg, Imm)
  INST_4i(vpinsrd, kInstVpinsrd, XmmReg, XmmReg, Mem, Imm)

  INST_4i(vpinsrw, kInstVpinsrw, XmmReg, XmmReg, GpReg, Imm)
  INST_4i(vpinsrw, kInstVpinsrw, XmmReg, XmmReg, Mem, Imm)

  INST_3x(vpmaddubsw, kInstVpmaddubsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaddubsw, kInstVpmaddubsw, XmmReg, XmmReg, Mem)

  INST_3x(vpmaddwd, kInstVpmaddwd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaddwd, kInstVpmaddwd, XmmReg, XmmReg, Mem)

  INST_3x(vpmaxsb, kInstVpmaxsb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaxsb, kInstVpmaxsb, XmmReg, XmmReg, Mem)

  INST_3x(vpmaxsd, kInstVpmaxsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaxsd, kInstVpmaxsd, XmmReg, XmmReg, Mem)

  INST_3x(vpmaxsw, kInstVpmaxsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaxsw, kInstVpmaxsw, XmmReg, XmmReg, Mem)

  INST_3x(vpmaxub, kInstVpmaxub, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaxub, kInstVpmaxub, XmmReg, XmmReg, Mem)

  INST_3x(vpmaxud, kInstVpmaxud, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaxud, kInstVpmaxud, XmmReg, XmmReg, Mem)

  INST_3x(vpmaxuw, kInstVpmaxuw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmaxuw, kInstVpmaxuw, XmmReg, XmmReg, Mem)

  INST_3x(vpminsb, kInstVpminsb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpminsb, kInstVpminsb, XmmReg, XmmReg, Mem)

  INST_3x(vpminsd, kInstVpminsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpminsd, kInstVpminsd, XmmReg, XmmReg, Mem)

  INST_3x(vpminsw, kInstVpminsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpminsw, kInstVpminsw, XmmReg, XmmReg, Mem)

  INST_3x(vpminub, kInstVpminub, XmmReg, XmmReg, XmmReg)
  INST_3x(vpminub, kInstVpminub, XmmReg, XmmReg, Mem)

  INST_3x(vpminud, kInstVpminud, XmmReg, XmmReg, XmmReg)
  INST_3x(vpminud, kInstVpminud, XmmReg, XmmReg, Mem)

  INST_3x(vpminuw, kInstVpminuw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpminuw, kInstVpminuw, XmmReg, XmmReg, Mem)

  INST_2x(vpmovmskb, kInstVpmovmskb, GpReg, XmmReg)

  INST_2x(vpmovsxbd, kInstVpmovsxbd, XmmReg, XmmReg)
  INST_2x(vpmovsxbd, kInstVpmovsxbd, XmmReg, Mem)

  INST_2x(vpmovsxbq, kInstVpmovsxbq, XmmReg, XmmReg)
  INST_2x(vpmovsxbq, kInstVpmovsxbq, XmmReg, Mem)

  INST_2x(vpmovsxbw, kInstVpmovsxbw, XmmReg, XmmReg)
  INST_2x(vpmovsxbw, kInstVpmovsxbw, XmmReg, Mem)

  INST_2x(vpmovsxdq, kInstVpmovsxdq, XmmReg, XmmReg)
  INST_2x(vpmovsxdq, kInstVpmovsxdq, XmmReg, Mem)

  INST_2x(vpmovsxwd, kInstVpmovsxwd, XmmReg, XmmReg)
  INST_2x(vpmovsxwd, kInstVpmovsxwd, XmmReg, Mem)

  INST_2x(vpmovsxwq, kInstVpmovsxwq, XmmReg, XmmReg)
  INST_2x(vpmovsxwq, kInstVpmovsxwq, XmmReg, Mem)

  INST_2x(vpmovzxbd, kInstVpmovzxbd, XmmReg, XmmReg)
  INST_2x(vpmovzxbd, kInstVpmovzxbd, XmmReg, Mem)

  INST_2x(vpmovzxbq, kInstVpmovzxbq, XmmReg, XmmReg)
  INST_2x(vpmovzxbq, kInstVpmovzxbq, XmmReg, Mem)

  INST_2x(vpmovzxbw, kInstVpmovzxbw, XmmReg, XmmReg)
  INST_2x(vpmovzxbw, kInstVpmovzxbw, XmmReg, Mem)

  INST_2x(vpmovzxdq, kInstVpmovzxdq, XmmReg, XmmReg)
  INST_2x(vpmovzxdq, kInstVpmovzxdq, XmmReg, Mem)

  INST_2x(vpmovzxwd, kInstVpmovzxwd, XmmReg, XmmReg)
  INST_2x(vpmovzxwd, kInstVpmovzxwd, XmmReg, Mem)

  INST_2x(vpmovzxwq, kInstVpmovzxwq, XmmReg, XmmReg)
  INST_2x(vpmovzxwq, kInstVpmovzxwq, XmmReg, Mem)

  INST_3x(vpmuldq, kInstVpmuldq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmuldq, kInstVpmuldq, XmmReg, XmmReg, Mem)

  INST_3x(vpmulhrsw, kInstVpmulhrsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmulhrsw, kInstVpmulhrsw, XmmReg, XmmReg, Mem)

  INST_3x(vpmulhuw, kInstVpmulhuw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmulhuw, kInstVpmulhuw, XmmReg, XmmReg, Mem)

  INST_3x(vpmulhw, kInstVpmulhw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmulhw, kInstVpmulhw, XmmReg, XmmReg, Mem)

  INST_3x(vpmulld, kInstVpmulld, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmulld, kInstVpmulld, XmmReg, XmmReg, Mem)

  INST_3x(vpmullw, kInstVpmullw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmullw, kInstVpmullw, XmmReg, XmmReg, Mem)

  INST_3x(vpmuludq, kInstVpmuludq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpmuludq, kInstVpmuludq, XmmReg, XmmReg, Mem)

  INST_3x(vpor, kInstVpor, XmmReg, XmmReg, XmmReg)
  INST_3x(vpor, kInstVpor, XmmReg, XmmReg, Mem)

  INST_3x(vpsadbw, kInstVpsadbw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsadbw, kInstVpsadbw, XmmReg, XmmReg, Mem)

  INST_3x(vpshufb, kInstVpshufb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpshufb, kInstVpshufb, XmmReg, XmmReg, Mem)

  INST_3i(vpshufd, kInstVpshufd, XmmReg, XmmReg, Imm)
  INST_3i(vpshufd, kInstVpshufd, XmmReg, Mem, Imm)

  INST_3i(vpshufhw, kInstVpshufhw, XmmReg, XmmReg, Imm)
  INST_3i(vpshufhw, kInstVpshufhw, XmmReg, Mem, Imm)

  INST_3i(vpshuflw, kInstVpshuflw, XmmReg, XmmReg, Imm)
  INST_3i(vpshuflw, kInstVpshuflw, XmmReg, Mem, Imm)

  INST_3x(vpsignb, kInstVpsignb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsignb, kInstVpsignb, XmmReg, XmmReg, Mem)

  INST_3x(vpsignd, kInstVpsignd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsignd, kInstVpsignd, XmmReg, XmmReg, Mem)

  INST_3x(vpsignw, kInstVpsignw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsignw, kInstVpsignw, XmmReg, XmmReg, Mem)

  INST_3x(vpslld, kInstVpslld, XmmReg, XmmReg, XmmReg)
  INST_3x(vpslld, kInstVpslld, XmmReg, XmmReg, Mem)
  INST_3i(vpslld, kInstVpslld, XmmReg, XmmReg, Imm)

  INST_3i(vpslldq, kInstVpslldq, XmmReg, XmmReg, Imm)

  INST_3x(vpsllq, kInstVpsllq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsllq, kInstVpsllq, XmmReg, XmmReg, Mem)
  INST_3i(vpsllq, kInstVpsllq, XmmReg, XmmReg, Imm)

  INST_3x(vpsllw, kInstVpsllw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsllw, kInstVpsllw, XmmReg, XmmReg, Mem)
  INST_3i(vpsllw, kInstVpsllw, XmmReg, XmmReg, Imm)

  INST_3x(vpsrad, kInstVpsrad, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsrad, kInstVpsrad, XmmReg, XmmReg, Mem)
  INST_3i(vpsrad, kInstVpsrad, XmmReg, XmmReg, Imm)

  INST_3x(vpsraw, kInstVpsraw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsraw, kInstVpsraw, XmmReg, XmmReg, Mem)
  INST_3i(vpsraw, kInstVpsraw, XmmReg, XmmReg, Imm)

  INST_3x(vpsrld, kInstVpsrld, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsrld, kInstVpsrld, XmmReg, XmmReg, Mem)
  INST_3i(vpsrld, kInstVpsrld, XmmReg, XmmReg, Imm)

  INST_3i(vpsrldq, kInstVpsrldq, XmmReg, XmmReg, Imm)

  INST_3x(vpsrlq, kInstVpsrlq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsrlq, kInstVpsrlq, XmmReg, XmmReg, Mem)
  INST_3i(vpsrlq, kInstVpsrlq, XmmReg, XmmReg, Imm)

  INST_3x(vpsrlw, kInstVpsrlw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsrlw, kInstVpsrlw, XmmReg, XmmReg, Mem)
  INST_3i(vpsrlw, kInstVpsrlw, XmmReg, XmmReg, Imm)

  INST_3x(vpsubb, kInstVpsubb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubb, kInstVpsubb, XmmReg, XmmReg, Mem)

  INST_3x(vpsubd, kInstVpsubd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubd, kInstVpsubd, XmmReg, XmmReg, Mem)

  INST_3x(vpsubq, kInstVpsubq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubq, kInstVpsubq, XmmReg, XmmReg, Mem)

  INST_3x(vpsubw, kInstVpsubw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubw, kInstVpsubw, XmmReg, XmmReg, Mem)

  INST_3x(vpsubsb, kInstVpsubsb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubsb, kInstVpsubsb, XmmReg, XmmReg, Mem)

  INST_3x(vpsubsw, kInstVpsubsw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubsw, kInstVpsubsw, XmmReg, XmmReg, Mem)

  INST_3x(vpsubusb, kInstVpsubusb, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubusb, kInstVpsubusb, XmmReg, XmmReg, Mem)

  INST_3x(vpsubusw, kInstVpsubusw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsubusw, kInstVpsubusw, XmmReg, XmmReg, Mem)

  INST_2x(vptest, kInstVptest, XmmReg, XmmReg)
  INST_2x(vptest, kInstVptest, XmmReg, Mem)
  INST_2x(vptest, kInstVptest, YmmReg, YmmReg)
  INST_2x(vptest, kInstVptest, YmmReg, Mem)

  INST_3x(vpunpckhbw, kInstVpunpckhbw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpckhbw, kInstVpunpckhbw, XmmReg, XmmReg, Mem)

  INST_3x(vpunpckhdq, kInstVpunpckhdq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpckhdq, kInstVpunpckhdq, XmmReg, XmmReg, Mem)

  INST_3x(vpunpckhqdq, kInstVpunpckhqdq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpckhqdq, kInstVpunpckhqdq, XmmReg, XmmReg, Mem)

  INST_3x(vpunpckhwd, kInstVpunpckhwd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpckhwd, kInstVpunpckhwd, XmmReg, XmmReg, Mem)

  INST_3x(vpunpcklbw, kInstVpunpcklbw, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpcklbw, kInstVpunpcklbw, XmmReg, XmmReg, Mem)

  INST_3x(vpunpckldq, kInstVpunpckldq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpckldq, kInstVpunpckldq, XmmReg, XmmReg, Mem)

  INST_3x(vpunpcklqdq, kInstVpunpcklqdq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpcklqdq, kInstVpunpcklqdq, XmmReg, XmmReg, Mem)

  INST_3x(vpunpcklwd, kInstVpunpcklwd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpunpcklwd, kInstVpunpcklwd, XmmReg, XmmReg, Mem)

  INST_3x(vpxor, kInstVpxor, XmmReg, XmmReg, XmmReg)
  INST_3x(vpxor, kInstVpxor, XmmReg, XmmReg, Mem)

  INST_2x(vrcpps, kInstVrcpps, XmmReg, XmmReg)
  INST_2x(vrcpps, kInstVrcpps, XmmReg, Mem)
  INST_2x(vrcpps, kInstVrcpps, YmmReg, YmmReg)
  INST_2x(vrcpps, kInstVrcpps, YmmReg, Mem)

  INST_3x(vrcpss, kInstVrcpss, XmmReg, XmmReg, XmmReg)
  INST_3x(vrcpss, kInstVrcpss, XmmReg, XmmReg, Mem)

  INST_2x(vrsqrtps, kInstVrsqrtps, XmmReg, XmmReg)
  INST_2x(vrsqrtps, kInstVrsqrtps, XmmReg, Mem)
  INST_2x(vrsqrtps, kInstVrsqrtps, YmmReg, YmmReg)
  INST_2x(vrsqrtps, kInstVrsqrtps, YmmReg, Mem)

  INST_3x(vrsqrtss, kInstVrsqrtss, XmmReg, XmmReg, XmmReg)
  INST_3x(vrsqrtss, kInstVrsqrtss, XmmReg, XmmReg, Mem)

  INST_3i(vroundpd, kInstVroundpd, XmmReg, XmmReg, Imm)
  INST_3i(vroundpd, kInstVroundpd, XmmReg, Mem, Imm)
  INST_3i(vroundpd, kInstVroundpd, YmmReg, YmmReg, Imm)
  INST_3i(vroundpd, kInstVroundpd, YmmReg, Mem, Imm)

  INST_3i(vroundps, kInstVroundps, XmmReg, XmmReg, Imm)
  INST_3i(vroundps, kInstVroundps, XmmReg, Mem, Imm)
  INST_3i(vroundps, kInstVroundps, YmmReg, YmmReg, Imm)
  INST_3i(vroundps, kInstVroundps, YmmReg, Mem, Imm)

  INST_4i(vroundsd, kInstVroundsd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vroundsd, kInstVroundsd, XmmReg, XmmReg, Mem, Imm)

  INST_4i(vroundss, kInstVroundss, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vroundss, kInstVroundss, XmmReg, XmmReg, Mem, Imm)

  INST_4i(vshufpd, kInstVshufpd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vshufpd, kInstVshufpd, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vshufpd, kInstVshufpd, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vshufpd, kInstVshufpd, YmmReg, YmmReg, Mem, Imm)

  INST_4i(vshufps, kInstVshufps, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vshufps, kInstVshufps, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vshufps, kInstVshufps, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vshufps, kInstVshufps, YmmReg, YmmReg, Mem, Imm)

  INST_2x(vsqrtpd, kInstVsqrtpd, XmmReg, XmmReg)
  INST_2x(vsqrtpd, kInstVsqrtpd, XmmReg, Mem)
  INST_2x(vsqrtpd, kInstVsqrtpd, YmmReg, YmmReg)
  INST_2x(vsqrtpd, kInstVsqrtpd, YmmReg, Mem)

  INST_2x(vsqrtps, kInstVsqrtps, XmmReg, XmmReg)
  INST_2x(vsqrtps, kInstVsqrtps, XmmReg, Mem)
  INST_2x(vsqrtps, kInstVsqrtps, YmmReg, YmmReg)
  INST_2x(vsqrtps, kInstVsqrtps, YmmReg, Mem)

  INST_3x(vsqrtsd, kInstVsqrtsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vsqrtsd, kInstVsqrtsd, XmmReg, XmmReg, Mem)

  INST_3x(vsqrtss, kInstVsqrtss, XmmReg, XmmReg, XmmReg)
  INST_3x(vsqrtss, kInstVsqrtss, XmmReg, XmmReg, Mem)

  INST_1x(vstmxcsr, kInstVstmxcsr, Mem)

  INST_3x(vsubpd, kInstVsubpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vsubpd, kInstVsubpd, XmmReg, XmmReg, Mem)
  INST_3x(vsubpd, kInstVsubpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vsubpd, kInstVsubpd, YmmReg, YmmReg, Mem)

  INST_3x(vsubps, kInstVsubps, XmmReg, XmmReg, XmmReg)
  INST_3x(vsubps, kInstVsubps, XmmReg, XmmReg, Mem)
  INST_3x(vsubps, kInstVsubps, YmmReg, YmmReg, YmmReg)
  INST_3x(vsubps, kInstVsubps, YmmReg, YmmReg, Mem)

  INST_3x(vsubsd, kInstVsubsd, XmmReg, XmmReg, XmmReg)
  INST_3x(vsubsd, kInstVsubsd, XmmReg, XmmReg, Mem)

  INST_3x(vsubss, kInstVsubss, XmmReg, XmmReg, XmmReg)
  INST_3x(vsubss, kInstVsubss, XmmReg, XmmReg, Mem)

  INST_2x(vtestps, kInstVtestps, XmmReg, XmmReg)
  INST_2x(vtestps, kInstVtestps, XmmReg, Mem)
  INST_2x(vtestps, kInstVtestps, YmmReg, YmmReg)
  INST_2x(vtestps, kInstVtestps, YmmReg, Mem)

  INST_2x(vtestpd, kInstVtestpd, XmmReg, XmmReg)
  INST_2x(vtestpd, kInstVtestpd, XmmReg, Mem)
  INST_2x(vtestpd, kInstVtestpd, YmmReg, YmmReg)
  INST_2x(vtestpd, kInstVtestpd, YmmReg, Mem)

  INST_2x(vucomisd, kInstVucomisd, XmmReg, XmmReg)
  INST_2x(vucomisd, kInstVucomisd, XmmReg, Mem)

  INST_2x(vucomiss, kInstVucomiss, XmmReg, XmmReg)
  INST_2x(vucomiss, kInstVucomiss, XmmReg, Mem)

  INST_3x(vunpckhpd, kInstVunpckhpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vunpckhpd, kInstVunpckhpd, XmmReg, XmmReg, Mem)
  INST_3x(vunpckhpd, kInstVunpckhpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vunpckhpd, kInstVunpckhpd, YmmReg, YmmReg, Mem)

  INST_3x(vunpckhps, kInstVunpckhps, XmmReg, XmmReg, XmmReg)
  INST_3x(vunpckhps, kInstVunpckhps, XmmReg, XmmReg, Mem)
  INST_3x(vunpckhps, kInstVunpckhps, YmmReg, YmmReg, YmmReg)
  INST_3x(vunpckhps, kInstVunpckhps, YmmReg, YmmReg, Mem)

  INST_3x(vunpcklpd, kInstVunpcklpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vunpcklpd, kInstVunpcklpd, XmmReg, XmmReg, Mem)
  INST_3x(vunpcklpd, kInstVunpcklpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vunpcklpd, kInstVunpcklpd, YmmReg, YmmReg, Mem)

  INST_3x(vunpcklps, kInstVunpcklps, XmmReg, XmmReg, XmmReg)
  INST_3x(vunpcklps, kInstVunpcklps, XmmReg, XmmReg, Mem)
  INST_3x(vunpcklps, kInstVunpcklps, YmmReg, YmmReg, YmmReg)
  INST_3x(vunpcklps, kInstVunpcklps, YmmReg, YmmReg, Mem)

  INST_3x(vxorpd, kInstVxorpd, XmmReg, XmmReg, XmmReg)
  INST_3x(vxorpd, kInstVxorpd, XmmReg, XmmReg, Mem)
  INST_3x(vxorpd, kInstVxorpd, YmmReg, YmmReg, YmmReg)
  INST_3x(vxorpd, kInstVxorpd, YmmReg, YmmReg, Mem)

  INST_3x(vxorps, kInstVxorps, XmmReg, XmmReg, XmmReg)
  INST_3x(vxorps, kInstVxorps, XmmReg, XmmReg, Mem)
  INST_3x(vxorps, kInstVxorps, YmmReg, YmmReg, YmmReg)
  INST_3x(vxorps, kInstVxorps, YmmReg, YmmReg, Mem)

  INST_0x(vzeroall, kInstVzeroall)
  INST_0x(vzeroupper, kInstVzeroupper)

  // --------------------------------------------------------------------------
  // [AVX+AESNI]
  // --------------------------------------------------------------------------

  //! @brief Perform a single round of the AES decryption flow (AVX+AESNI).
  INST_3x(vaesdec, kInstVaesdec, XmmReg, XmmReg, XmmReg)
  //! @overload
  INST_3x(vaesdec, kInstVaesdec, XmmReg, XmmReg, Mem)

  //! @brief Perform the last round of the AES decryption flow (AVX+AESNI).
  INST_3x(vaesdeclast, kInstVaesdeclast, XmmReg, XmmReg, XmmReg)
  //! @overload
  INST_3x(vaesdeclast, kInstVaesdeclast, XmmReg, XmmReg, Mem)

  //! @brief Perform a single round of the AES encryption flow (AVX+AESNI).
  INST_3x(vaesenc, kInstVaesenc, XmmReg, XmmReg, XmmReg)
  //! @overload
  INST_3x(vaesenc, kInstVaesenc, XmmReg, XmmReg, Mem)

  //! @brief Perform the last round of the AES encryption flow (AVX+AESNI).
  INST_3x(vaesenclast, kInstVaesenclast, XmmReg, XmmReg, XmmReg)
  //! @overload
  INST_3x(vaesenclast, kInstVaesenclast, XmmReg, XmmReg, Mem)

  //! @brief Perform the InvMixColumns transformation (AVX+AESNI).
  INST_2x(vaesimc, kInstVaesimc, XmmReg, XmmReg)
  //! @overload
  INST_2x(vaesimc, kInstVaesimc, XmmReg, Mem)

  //! @brief Assist in expanding the AES cipher key (AVX+AESNI).
  INST_3i(vaeskeygenassist, kInstVaeskeygenassist, XmmReg, XmmReg, Imm)
  //! @overload
  INST_3i(vaeskeygenassist, kInstVaeskeygenassist, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [AVX+PCLMULQDQ]
  // --------------------------------------------------------------------------

  INST_4i(vpclmulqdq, kInstVpclmulqdq, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vpclmulqdq, kInstVpclmulqdq, XmmReg, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [AVX2]
  // --------------------------------------------------------------------------

  INST_2x(vbroadcasti128, kInstVbroadcasti128, YmmReg, Mem)

  INST_2x(vbroadcastsd, kInstVbroadcastsd, YmmReg, XmmReg)

  INST_2x(vbroadcastss, kInstVbroadcastss, XmmReg, XmmReg)
  INST_2x(vbroadcastss, kInstVbroadcastss, YmmReg, XmmReg)

  INST_3i(vextracti128, kInstVextracti128, XmmReg, YmmReg, Imm)
  INST_3i(vextracti128, kInstVextracti128, Mem, YmmReg, Imm)

  INST_3x(vgatherdpd, kInstVgatherdpd, XmmReg, Mem, XmmReg)
  INST_3x(vgatherdpd, kInstVgatherdpd, YmmReg, Mem, YmmReg)

  INST_3x(vgatherdps, kInstVgatherdps, XmmReg, Mem, XmmReg)
  INST_3x(vgatherdps, kInstVgatherdps, YmmReg, Mem, YmmReg)

  INST_3x(vgatherqpd, kInstVgatherqpd, XmmReg, Mem, XmmReg)
  INST_3x(vgatherqpd, kInstVgatherqpd, YmmReg, Mem, YmmReg)

  INST_3x(vgatherqps, kInstVgatherqps, XmmReg, Mem, XmmReg)

  INST_4i(vinserti128, kInstVinserti128, YmmReg, YmmReg, XmmReg, Imm)
  INST_4i(vinserti128, kInstVinserti128, YmmReg, YmmReg, Mem, Imm)

  INST_2x(vmovntdqa, kInstVmovntdqa, YmmReg, Mem)

  INST_4i(vmpsadbw, kInstVmpsadbw, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vmpsadbw, kInstVmpsadbw, YmmReg, YmmReg, Mem, Imm)

  INST_2x(vpabsb, kInstVpabsb, YmmReg, YmmReg)
  INST_2x(vpabsb, kInstVpabsb, YmmReg, Mem)

  INST_2x(vpabsd, kInstVpabsd, YmmReg, YmmReg)
  INST_2x(vpabsd, kInstVpabsd, YmmReg, Mem)

  INST_2x(vpabsw, kInstVpabsw, YmmReg, YmmReg)
  INST_2x(vpabsw, kInstVpabsw, YmmReg, Mem)

  INST_3x(vpackssdw, kInstVpackssdw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpackssdw, kInstVpackssdw, YmmReg, YmmReg, Mem)

  INST_3x(vpacksswb, kInstVpacksswb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpacksswb, kInstVpacksswb, YmmReg, YmmReg, Mem)

  INST_3x(vpackusdw, kInstVpackusdw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpackusdw, kInstVpackusdw, YmmReg, YmmReg, Mem)

  INST_3x(vpackuswb, kInstVpackuswb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpackuswb, kInstVpackuswb, YmmReg, YmmReg, Mem)

  INST_3x(vpaddb, kInstVpaddb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddb, kInstVpaddb, YmmReg, YmmReg, Mem)

  INST_3x(vpaddd, kInstVpaddd, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddd, kInstVpaddd, YmmReg, YmmReg, Mem)

  INST_3x(vpaddq, kInstVpaddq, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddq, kInstVpaddq, YmmReg, YmmReg, Mem)

  INST_3x(vpaddw, kInstVpaddw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddw, kInstVpaddw, YmmReg, YmmReg, Mem)

  INST_3x(vpaddsb, kInstVpaddsb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddsb, kInstVpaddsb, YmmReg, YmmReg, Mem)

  INST_3x(vpaddsw, kInstVpaddsw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddsw, kInstVpaddsw, YmmReg, YmmReg, Mem)

  INST_3x(vpaddusb, kInstVpaddusb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddusb, kInstVpaddusb, YmmReg, YmmReg, Mem)

  INST_3x(vpaddusw, kInstVpaddusw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpaddusw, kInstVpaddusw, YmmReg, YmmReg, Mem)

  INST_4i(vpalignr, kInstVpalignr, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vpalignr, kInstVpalignr, YmmReg, YmmReg, Mem, Imm)

  INST_3x(vpand, kInstVpand, YmmReg, YmmReg, YmmReg)
  INST_3x(vpand, kInstVpand, YmmReg, YmmReg, Mem)

  INST_3x(vpandn, kInstVpandn, YmmReg, YmmReg, YmmReg)
  INST_3x(vpandn, kInstVpandn, YmmReg, YmmReg, Mem)

  INST_3x(vpavgb, kInstVpavgb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpavgb, kInstVpavgb, YmmReg, YmmReg, Mem)

  INST_3x(vpavgw, kInstVpavgw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpavgw, kInstVpavgw, YmmReg, YmmReg, Mem)

  INST_4i(vpblendd, kInstVpblendd, XmmReg, XmmReg, XmmReg, Imm)
  INST_4i(vpblendd, kInstVpblendd, XmmReg, XmmReg, Mem, Imm)
  INST_4i(vpblendd, kInstVpblendd, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vpblendd, kInstVpblendd, YmmReg, YmmReg, Mem, Imm)

  INST_4x(vpblendvb, kInstVpblendvb, YmmReg, YmmReg, YmmReg, YmmReg)
  INST_4x(vpblendvb, kInstVpblendvb, YmmReg, YmmReg, Mem, YmmReg)

  INST_4i(vpblendw, kInstVpblendw, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vpblendw, kInstVpblendw, YmmReg, YmmReg, Mem, Imm)

  INST_2x(vpbroadcastb, kInstVpbroadcastb, XmmReg, XmmReg)
  INST_2x(vpbroadcastb, kInstVpbroadcastb, XmmReg, Mem)
  INST_2x(vpbroadcastb, kInstVpbroadcastb, YmmReg, XmmReg)
  INST_2x(vpbroadcastb, kInstVpbroadcastb, YmmReg, Mem)

  INST_2x(vpbroadcastd, kInstVpbroadcastd, XmmReg, XmmReg)
  INST_2x(vpbroadcastd, kInstVpbroadcastd, XmmReg, Mem)
  INST_2x(vpbroadcastd, kInstVpbroadcastd, YmmReg, XmmReg)
  INST_2x(vpbroadcastd, kInstVpbroadcastd, YmmReg, Mem)

  INST_2x(vpbroadcastq, kInstVpbroadcastq, XmmReg, XmmReg)
  INST_2x(vpbroadcastq, kInstVpbroadcastq, XmmReg, Mem)
  INST_2x(vpbroadcastq, kInstVpbroadcastq, YmmReg, XmmReg)
  INST_2x(vpbroadcastq, kInstVpbroadcastq, YmmReg, Mem)

  INST_2x(vpbroadcastw, kInstVpbroadcastw, XmmReg, XmmReg)
  INST_2x(vpbroadcastw, kInstVpbroadcastw, XmmReg, Mem)
  INST_2x(vpbroadcastw, kInstVpbroadcastw, YmmReg, XmmReg)
  INST_2x(vpbroadcastw, kInstVpbroadcastw, YmmReg, Mem)

  INST_3x(vpcmpeqb, kInstVpcmpeqb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpeqb, kInstVpcmpeqb, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpeqd, kInstVpcmpeqd, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpeqd, kInstVpcmpeqd, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpeqq, kInstVpcmpeqq, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpeqq, kInstVpcmpeqq, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpeqw, kInstVpcmpeqw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpeqw, kInstVpcmpeqw, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpgtb, kInstVpcmpgtb, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpgtb, kInstVpcmpgtb, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpgtd, kInstVpcmpgtd, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpgtd, kInstVpcmpgtd, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpgtq, kInstVpcmpgtq, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpgtq, kInstVpcmpgtq, YmmReg, YmmReg, Mem)

  INST_3x(vpcmpgtw, kInstVpcmpgtw, YmmReg, YmmReg, YmmReg)
  INST_3x(vpcmpgtw, kInstVpcmpgtw, YmmReg, YmmReg, Mem)

  INST_4i(vperm2i128, kInstVperm2i128, YmmReg, YmmReg, YmmReg, Imm)
  INST_4i(vperm2i128, kInstVperm2i128, YmmReg, YmmReg, Mem, Imm)

  INST_3x(vpermd, kInstVpermd, YmmReg, YmmReg, YmmReg)
  INST_3x(vpermd, kInstVpermd, YmmReg, YmmReg, Mem)

  INST_3x(vpermps, kInstVpermps, YmmReg, YmmReg, YmmReg)
  INST_3x(vpermps, kInstVpermps, YmmReg, YmmReg, Mem)

  INST_3i(vpermpd, kInstVpermpd, YmmReg, YmmReg, Imm)
  INST_3i(vpermpd, kInstVpermpd, YmmReg, Mem, Imm)

  INST_3i(vpermq, kInstVpermq, YmmReg, YmmReg, Imm)
  INST_3i(vpermq, kInstVpermq, YmmReg, Mem, Imm)

  INST_3x(vpgatherdd, kInstVpgatherdd, XmmReg, Mem, XmmReg)
  INST_3x(vpgatherdd, kInstVpgatherdd, YmmReg, Mem, YmmReg)

  INST_3x(vpgatherdq, kInstVpgatherdq, XmmReg, Mem, XmmReg)
  INST_3x(vpgatherdq, kInstVpgatherdq, YmmReg, Mem, YmmReg)

  INST_3x(vpgatherqd, kInstVpgatherqd, XmmReg, Mem, XmmReg)

  INST_3x(vpgatherqq, kInstVpgatherqq, XmmReg, Mem, XmmReg)
  INST_3x(vpgatherqq, kInstVpgatherqq, YmmReg, Mem, YmmReg)

  INST_2x(vpmovmskb, kInstVpmovmskb, GpReg, YmmReg)

  INST_2x(vpmovsxbd, kInstVpmovsxbd, YmmReg, Mem)
  INST_2x(vpmovsxbd, kInstVpmovsxbd, YmmReg, XmmReg)

  INST_2x(vpmovsxbq, kInstVpmovsxbq, YmmReg, Mem)
  INST_2x(vpmovsxbq, kInstVpmovsxbq, YmmReg, XmmReg)

  INST_2x(vpmovsxbw, kInstVpmovsxbw, YmmReg, Mem)
  INST_2x(vpmovsxbw, kInstVpmovsxbw, YmmReg, XmmReg)

  INST_2x(vpmovsxdq, kInstVpmovsxdq, YmmReg, Mem)
  INST_2x(vpmovsxdq, kInstVpmovsxdq, YmmReg, XmmReg)

  INST_2x(vpmovsxwd, kInstVpmovsxwd, YmmReg, Mem)
  INST_2x(vpmovsxwd, kInstVpmovsxwd, YmmReg, XmmReg)

  INST_2x(vpmovsxwq, kInstVpmovsxwq, YmmReg, Mem)
  INST_2x(vpmovsxwq, kInstVpmovsxwq, YmmReg, XmmReg)

  INST_2x(vpmovzxbd, kInstVpmovzxbd, YmmReg, Mem)
  INST_2x(vpmovzxbd, kInstVpmovzxbd, YmmReg, XmmReg)

  INST_2x(vpmovzxbq, kInstVpmovzxbq, YmmReg, Mem)
  INST_2x(vpmovzxbq, kInstVpmovzxbq, YmmReg, XmmReg)

  INST_2x(vpmovzxbw, kInstVpmovzxbw, YmmReg, Mem)
  INST_2x(vpmovzxbw, kInstVpmovzxbw, YmmReg, XmmReg)

  INST_2x(vpmovzxdq, kInstVpmovzxdq, YmmReg, Mem)
  INST_2x(vpmovzxdq, kInstVpmovzxdq, YmmReg, XmmReg)

  INST_2x(vpmovzxwd, kInstVpmovzxwd, YmmReg, Mem)
  INST_2x(vpmovzxwd, kInstVpmovzxwd, YmmReg, XmmReg)

  INST_2x(vpmovzxwq, kInstVpmovzxwq, YmmReg, Mem)
  INST_2x(vpmovzxwq, kInstVpmovzxwq, YmmReg, XmmReg)

  INST_3i(vpshufd, kInstVpshufd, YmmReg, Mem, Imm)
  INST_3i(vpshufd, kInstVpshufd, YmmReg, YmmReg, Imm)

  INST_3i(vpshufhw, kInstVpshufhw, YmmReg, Mem, Imm)
  INST_3i(vpshufhw, kInstVpshufhw, YmmReg, YmmReg, Imm)

  INST_3i(vpshuflw, kInstVpshuflw, YmmReg, Mem, Imm)
  INST_3i(vpshuflw, kInstVpshuflw, YmmReg, YmmReg, Imm)

  INST_3i(vpslld, kInstVpslld, YmmReg, YmmReg, Imm)

  INST_3i(vpslldq, kInstVpslldq, YmmReg, YmmReg, Imm)

  INST_3i(vpsllq, kInstVpsllq, YmmReg, YmmReg, Imm)

  INST_3i(vpsllw, kInstVpsllw, YmmReg, YmmReg, Imm)

  INST_3i(vpsrad, kInstVpsrad, YmmReg, YmmReg, Imm)

  INST_3i(vpsraw, kInstVpsraw, YmmReg, YmmReg, Imm)

  INST_3i(vpsrld, kInstVpsrld, YmmReg, YmmReg, Imm)

  INST_3i(vpsrldq, kInstVpsrldq, YmmReg, YmmReg, Imm)

  INST_3i(vpsrlq, kInstVpsrlq, YmmReg, YmmReg, Imm)

  INST_3i(vpsrlw, kInstVpsrlw, YmmReg, YmmReg, Imm)

  INST_3x(vphaddd, kInstVphaddd, YmmReg, YmmReg, Mem)
  INST_3x(vphaddd, kInstVphaddd, YmmReg, YmmReg, YmmReg)

  INST_3x(vphaddsw, kInstVphaddsw, YmmReg, YmmReg, Mem)
  INST_3x(vphaddsw, kInstVphaddsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vphaddw, kInstVphaddw, YmmReg, YmmReg, Mem)
  INST_3x(vphaddw, kInstVphaddw, YmmReg, YmmReg, YmmReg)

  INST_3x(vphsubd, kInstVphsubd, YmmReg, YmmReg, Mem)
  INST_3x(vphsubd, kInstVphsubd, YmmReg, YmmReg, YmmReg)

  INST_3x(vphsubsw, kInstVphsubsw, YmmReg, YmmReg, Mem)
  INST_3x(vphsubsw, kInstVphsubsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vphsubw, kInstVphsubw, YmmReg, YmmReg, Mem)
  INST_3x(vphsubw, kInstVphsubw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaddubsw, kInstVpmaddubsw, YmmReg, YmmReg, Mem)
  INST_3x(vpmaddubsw, kInstVpmaddubsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaddwd, kInstVpmaddwd, YmmReg, YmmReg, Mem)
  INST_3x(vpmaddwd, kInstVpmaddwd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaskmovd, kInstVpmaskmovd, Mem, XmmReg, XmmReg)
  INST_3x(vpmaskmovd, kInstVpmaskmovd, Mem, YmmReg, YmmReg)
  INST_3x(vpmaskmovd, kInstVpmaskmovd, XmmReg, XmmReg, Mem)
  INST_3x(vpmaskmovd, kInstVpmaskmovd, YmmReg, YmmReg, Mem)

  INST_3x(vpmaskmovq, kInstVpmaskmovq, Mem, XmmReg, XmmReg)
  INST_3x(vpmaskmovq, kInstVpmaskmovq, Mem, YmmReg, YmmReg)
  INST_3x(vpmaskmovq, kInstVpmaskmovq, XmmReg, XmmReg, Mem)
  INST_3x(vpmaskmovq, kInstVpmaskmovq, YmmReg, YmmReg, Mem)

  INST_3x(vpmaxsb, kInstVpmaxsb, YmmReg, YmmReg, Mem)
  INST_3x(vpmaxsb, kInstVpmaxsb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaxsd, kInstVpmaxsd, YmmReg, YmmReg, Mem)
  INST_3x(vpmaxsd, kInstVpmaxsd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaxsw, kInstVpmaxsw, YmmReg, YmmReg, Mem)
  INST_3x(vpmaxsw, kInstVpmaxsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaxub, kInstVpmaxub, YmmReg, YmmReg, Mem)
  INST_3x(vpmaxub, kInstVpmaxub, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaxud, kInstVpmaxud, YmmReg, YmmReg, Mem)
  INST_3x(vpmaxud, kInstVpmaxud, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmaxuw, kInstVpmaxuw, YmmReg, YmmReg, Mem)
  INST_3x(vpmaxuw, kInstVpmaxuw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpminsb, kInstVpminsb, YmmReg, YmmReg, Mem)
  INST_3x(vpminsb, kInstVpminsb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpminsd, kInstVpminsd, YmmReg, YmmReg, Mem)
  INST_3x(vpminsd, kInstVpminsd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpminsw, kInstVpminsw, YmmReg, YmmReg, Mem)
  INST_3x(vpminsw, kInstVpminsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpminub, kInstVpminub, YmmReg, YmmReg, Mem)
  INST_3x(vpminub, kInstVpminub, YmmReg, YmmReg, YmmReg)

  INST_3x(vpminud, kInstVpminud, YmmReg, YmmReg, Mem)
  INST_3x(vpminud, kInstVpminud, YmmReg, YmmReg, YmmReg)

  INST_3x(vpminuw, kInstVpminuw, YmmReg, YmmReg, Mem)
  INST_3x(vpminuw, kInstVpminuw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmuldq, kInstVpmuldq, YmmReg, YmmReg, Mem)
  INST_3x(vpmuldq, kInstVpmuldq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmulhrsw, kInstVpmulhrsw, YmmReg, YmmReg, Mem)
  INST_3x(vpmulhrsw, kInstVpmulhrsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmulhuw, kInstVpmulhuw, YmmReg, YmmReg, Mem)
  INST_3x(vpmulhuw, kInstVpmulhuw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmulhw, kInstVpmulhw, YmmReg, YmmReg, Mem)
  INST_3x(vpmulhw, kInstVpmulhw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmulld, kInstVpmulld, YmmReg, YmmReg, Mem)
  INST_3x(vpmulld, kInstVpmulld, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmullw, kInstVpmullw, YmmReg, YmmReg, Mem)
  INST_3x(vpmullw, kInstVpmullw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpmuludq, kInstVpmuludq, YmmReg, YmmReg, Mem)
  INST_3x(vpmuludq, kInstVpmuludq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpor, kInstVpor, YmmReg, YmmReg, Mem)
  INST_3x(vpor, kInstVpor, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsadbw, kInstVpsadbw, YmmReg, YmmReg, Mem)
  INST_3x(vpsadbw, kInstVpsadbw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpshufb, kInstVpshufb, YmmReg, YmmReg, Mem)
  INST_3x(vpshufb, kInstVpshufb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsignb, kInstVpsignb, YmmReg, YmmReg, Mem)
  INST_3x(vpsignb, kInstVpsignb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsignd, kInstVpsignd, YmmReg, YmmReg, Mem)
  INST_3x(vpsignd, kInstVpsignd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsignw, kInstVpsignw, YmmReg, YmmReg, Mem)
  INST_3x(vpsignw, kInstVpsignw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpslld, kInstVpslld, YmmReg, YmmReg, Mem)
  INST_3x(vpslld, kInstVpslld, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsllq, kInstVpsllq, YmmReg, YmmReg, Mem)
  INST_3x(vpsllq, kInstVpsllq, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsllvd, kInstVpsllvd, XmmReg, XmmReg, Mem)
  INST_3x(vpsllvd, kInstVpsllvd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsllvd, kInstVpsllvd, YmmReg, YmmReg, Mem)
  INST_3x(vpsllvd, kInstVpsllvd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsllvq, kInstVpsllvq, XmmReg, XmmReg, Mem)
  INST_3x(vpsllvq, kInstVpsllvq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsllvq, kInstVpsllvq, YmmReg, YmmReg, Mem)
  INST_3x(vpsllvq, kInstVpsllvq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsllw, kInstVpsllw, YmmReg, YmmReg, Mem)
  INST_3x(vpsllw, kInstVpsllw, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsrad, kInstVpsrad, YmmReg, YmmReg, Mem)
  INST_3x(vpsrad, kInstVpsrad, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsravd, kInstVpsravd, XmmReg, XmmReg, Mem)
  INST_3x(vpsravd, kInstVpsravd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsravd, kInstVpsravd, YmmReg, YmmReg, Mem)
  INST_3x(vpsravd, kInstVpsravd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsraw, kInstVpsraw, YmmReg, YmmReg, Mem)
  INST_3x(vpsraw, kInstVpsraw, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsrld, kInstVpsrld, YmmReg, YmmReg, Mem)
  INST_3x(vpsrld, kInstVpsrld, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsrlq, kInstVpsrlq, YmmReg, YmmReg, Mem)
  INST_3x(vpsrlq, kInstVpsrlq, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsrlvd, kInstVpsrlvd, XmmReg, XmmReg, Mem)
  INST_3x(vpsrlvd, kInstVpsrlvd, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsrlvd, kInstVpsrlvd, YmmReg, YmmReg, Mem)
  INST_3x(vpsrlvd, kInstVpsrlvd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsrlvq, kInstVpsrlvq, XmmReg, XmmReg, Mem)
  INST_3x(vpsrlvq, kInstVpsrlvq, XmmReg, XmmReg, XmmReg)
  INST_3x(vpsrlvq, kInstVpsrlvq, YmmReg, YmmReg, Mem)
  INST_3x(vpsrlvq, kInstVpsrlvq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsrlw, kInstVpsrlw, YmmReg, YmmReg, Mem)
  INST_3x(vpsrlw, kInstVpsrlw, YmmReg, YmmReg, XmmReg)

  INST_3x(vpsubb, kInstVpsubb, YmmReg, YmmReg, Mem)
  INST_3x(vpsubb, kInstVpsubb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubd, kInstVpsubd, YmmReg, YmmReg, Mem)
  INST_3x(vpsubd, kInstVpsubd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubq, kInstVpsubq, YmmReg, YmmReg, Mem)
  INST_3x(vpsubq, kInstVpsubq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubsb, kInstVpsubsb, YmmReg, YmmReg, Mem)
  INST_3x(vpsubsb, kInstVpsubsb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubsw, kInstVpsubsw, YmmReg, YmmReg, Mem)
  INST_3x(vpsubsw, kInstVpsubsw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubusb, kInstVpsubusb, YmmReg, YmmReg, Mem)
  INST_3x(vpsubusb, kInstVpsubusb, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubusw, kInstVpsubusw, YmmReg, YmmReg, Mem)
  INST_3x(vpsubusw, kInstVpsubusw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpsubw, kInstVpsubw, YmmReg, YmmReg, Mem)
  INST_3x(vpsubw, kInstVpsubw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpckhbw, kInstVpunpckhbw, YmmReg, YmmReg, Mem)
  INST_3x(vpunpckhbw, kInstVpunpckhbw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpckhdq, kInstVpunpckhdq, YmmReg, YmmReg, Mem)
  INST_3x(vpunpckhdq, kInstVpunpckhdq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpckhqdq, kInstVpunpckhqdq, YmmReg, YmmReg, Mem)
  INST_3x(vpunpckhqdq, kInstVpunpckhqdq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpckhwd, kInstVpunpckhwd, YmmReg, YmmReg, Mem)
  INST_3x(vpunpckhwd, kInstVpunpckhwd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpcklbw, kInstVpunpcklbw, YmmReg, YmmReg, Mem)
  INST_3x(vpunpcklbw, kInstVpunpcklbw, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpckldq, kInstVpunpckldq, YmmReg, YmmReg, Mem)
  INST_3x(vpunpckldq, kInstVpunpckldq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpcklqdq, kInstVpunpcklqdq, YmmReg, YmmReg, Mem)
  INST_3x(vpunpcklqdq, kInstVpunpcklqdq, YmmReg, YmmReg, YmmReg)

  INST_3x(vpunpcklwd, kInstVpunpcklwd, YmmReg, YmmReg, Mem)
  INST_3x(vpunpcklwd, kInstVpunpcklwd, YmmReg, YmmReg, YmmReg)

  INST_3x(vpxor, kInstVpxor, YmmReg, YmmReg, Mem)
  INST_3x(vpxor, kInstVpxor, YmmReg, YmmReg, YmmReg)

  // --------------------------------------------------------------------------
  // [FMA3]
  // --------------------------------------------------------------------------

  INST_3x(vfmadd132pd, kInstVfmadd132pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd132pd, kInstVfmadd132pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmadd132pd, kInstVfmadd132pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmadd132pd, kInstVfmadd132pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmadd132ps, kInstVfmadd132ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd132ps, kInstVfmadd132ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmadd132ps, kInstVfmadd132ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmadd132ps, kInstVfmadd132ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmadd132sd, kInstVfmadd132sd, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd132sd, kInstVfmadd132sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmadd132ss, kInstVfmadd132ss, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd132ss, kInstVfmadd132ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmadd213pd, kInstVfmadd213pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd213pd, kInstVfmadd213pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmadd213pd, kInstVfmadd213pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmadd213pd, kInstVfmadd213pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmadd213ps, kInstVfmadd213ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd213ps, kInstVfmadd213ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmadd213ps, kInstVfmadd213ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmadd213ps, kInstVfmadd213ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmadd213sd, kInstVfmadd213sd, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd213sd, kInstVfmadd213sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmadd213ss, kInstVfmadd213ss, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd213ss, kInstVfmadd213ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmadd231pd, kInstVfmadd231pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd231pd, kInstVfmadd231pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmadd231pd, kInstVfmadd231pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmadd231pd, kInstVfmadd231pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmadd231ps, kInstVfmadd231ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd231ps, kInstVfmadd231ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmadd231ps, kInstVfmadd231ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmadd231ps, kInstVfmadd231ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmadd231sd, kInstVfmadd231sd, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd231sd, kInstVfmadd231sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmadd231ss, kInstVfmadd231ss, XmmReg, XmmReg, Mem)
  INST_3x(vfmadd231ss, kInstVfmadd231ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmaddsub132pd, kInstVfmaddsub132pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmaddsub132pd, kInstVfmaddsub132pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmaddsub132pd, kInstVfmaddsub132pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmaddsub132pd, kInstVfmaddsub132pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmaddsub132ps, kInstVfmaddsub132ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmaddsub132ps, kInstVfmaddsub132ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmaddsub132ps, kInstVfmaddsub132ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmaddsub132ps, kInstVfmaddsub132ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmaddsub213pd, kInstVfmaddsub213pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmaddsub213pd, kInstVfmaddsub213pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmaddsub213pd, kInstVfmaddsub213pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmaddsub213pd, kInstVfmaddsub213pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmaddsub213ps, kInstVfmaddsub213ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmaddsub213ps, kInstVfmaddsub213ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmaddsub213ps, kInstVfmaddsub213ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmaddsub213ps, kInstVfmaddsub213ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmaddsub231pd, kInstVfmaddsub231pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmaddsub231pd, kInstVfmaddsub231pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmaddsub231pd, kInstVfmaddsub231pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmaddsub231pd, kInstVfmaddsub231pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmaddsub231ps, kInstVfmaddsub231ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmaddsub231ps, kInstVfmaddsub231ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmaddsub231ps, kInstVfmaddsub231ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmaddsub231ps, kInstVfmaddsub231ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub132pd, kInstVfmsub132pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub132pd, kInstVfmsub132pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsub132pd, kInstVfmsub132pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmsub132pd, kInstVfmsub132pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub132ps, kInstVfmsub132ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub132ps, kInstVfmsub132ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsub132ps, kInstVfmsub132ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmsub132ps, kInstVfmsub132ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub132sd, kInstVfmsub132sd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub132sd, kInstVfmsub132sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmsub132ss, kInstVfmsub132ss, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub132ss, kInstVfmsub132ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmsub213pd, kInstVfmsub213pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub213pd, kInstVfmsub213pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsub213pd, kInstVfmsub213pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmsub213pd, kInstVfmsub213pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub213ps, kInstVfmsub213ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub213ps, kInstVfmsub213ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsub213ps, kInstVfmsub213ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmsub213ps, kInstVfmsub213ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub213sd, kInstVfmsub213sd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub213sd, kInstVfmsub213sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmsub213ss, kInstVfmsub213ss, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub213ss, kInstVfmsub213ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmsub231pd, kInstVfmsub231pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub231pd, kInstVfmsub231pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsub231pd, kInstVfmsub231pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmsub231pd, kInstVfmsub231pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub231ps, kInstVfmsub231ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub231ps, kInstVfmsub231ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsub231ps, kInstVfmsub231ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmsub231ps, kInstVfmsub231ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsub231sd, kInstVfmsub231sd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub231sd, kInstVfmsub231sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmsub231ss, kInstVfmsub231ss, XmmReg, XmmReg, Mem)
  INST_3x(vfmsub231ss, kInstVfmsub231ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfmsubadd132pd, kInstVfmsubadd132pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsubadd132pd, kInstVfmsubadd132pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsubadd132pd, kInstVfmsubadd132pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmsubadd132pd, kInstVfmsubadd132pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsubadd132ps, kInstVfmsubadd132ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmsubadd132ps, kInstVfmsubadd132ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsubadd132ps, kInstVfmsubadd132ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmsubadd132ps, kInstVfmsubadd132ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsubadd213pd, kInstVfmsubadd213pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsubadd213pd, kInstVfmsubadd213pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsubadd213pd, kInstVfmsubadd213pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmsubadd213pd, kInstVfmsubadd213pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsubadd213ps, kInstVfmsubadd213ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmsubadd213ps, kInstVfmsubadd213ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsubadd213ps, kInstVfmsubadd213ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmsubadd213ps, kInstVfmsubadd213ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsubadd231pd, kInstVfmsubadd231pd, XmmReg, XmmReg, Mem)
  INST_3x(vfmsubadd231pd, kInstVfmsubadd231pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsubadd231pd, kInstVfmsubadd231pd, YmmReg, YmmReg, Mem)
  INST_3x(vfmsubadd231pd, kInstVfmsubadd231pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfmsubadd231ps, kInstVfmsubadd231ps, XmmReg, XmmReg, Mem)
  INST_3x(vfmsubadd231ps, kInstVfmsubadd231ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfmsubadd231ps, kInstVfmsubadd231ps, YmmReg, YmmReg, Mem)
  INST_3x(vfmsubadd231ps, kInstVfmsubadd231ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd132pd, kInstVfnmadd132pd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd132pd, kInstVfnmadd132pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmadd132pd, kInstVfnmadd132pd, YmmReg, YmmReg, Mem)
  INST_3x(vfnmadd132pd, kInstVfnmadd132pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd132ps, kInstVfnmadd132ps, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd132ps, kInstVfnmadd132ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmadd132ps, kInstVfnmadd132ps, YmmReg, YmmReg, Mem)
  INST_3x(vfnmadd132ps, kInstVfnmadd132ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd132sd, kInstVfnmadd132sd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd132sd, kInstVfnmadd132sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmadd132ss, kInstVfnmadd132ss, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd132ss, kInstVfnmadd132ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmadd213pd, kInstVfnmadd213pd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd213pd, kInstVfnmadd213pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmadd213pd, kInstVfnmadd213pd, YmmReg, YmmReg, Mem)
  INST_3x(vfnmadd213pd, kInstVfnmadd213pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd213ps, kInstVfnmadd213ps, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd213ps, kInstVfnmadd213ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmadd213ps, kInstVfnmadd213ps, YmmReg, YmmReg, Mem)
  INST_3x(vfnmadd213ps, kInstVfnmadd213ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd213sd, kInstVfnmadd213sd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd213sd, kInstVfnmadd213sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmadd213ss, kInstVfnmadd213ss, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd213ss, kInstVfnmadd213ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmadd231pd, kInstVfnmadd231pd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd231pd, kInstVfnmadd231pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmadd231pd, kInstVfnmadd231pd, YmmReg, YmmReg, Mem)
  INST_3x(vfnmadd231pd, kInstVfnmadd231pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd231ps, kInstVfnmadd231ps, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd231ps, kInstVfnmadd231ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmadd231ps, kInstVfnmadd231ps, YmmReg, YmmReg, Mem)
  INST_3x(vfnmadd231ps, kInstVfnmadd231ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmadd231sd, kInstVfnmadd231sd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd231sd, kInstVfnmadd231sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmadd231ss, kInstVfnmadd231ss, XmmReg, XmmReg, Mem)
  INST_3x(vfnmadd231ss, kInstVfnmadd231ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmsub132pd, kInstVfnmsub132pd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub132pd, kInstVfnmsub132pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmsub132pd, kInstVfnmsub132pd, YmmReg, YmmReg, Mem)
  INST_3x(vfnmsub132pd, kInstVfnmsub132pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmsub132ps, kInstVfnmsub132ps, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub132ps, kInstVfnmsub132ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmsub132ps, kInstVfnmsub132ps, YmmReg, YmmReg, Mem)
  INST_3x(vfnmsub132ps, kInstVfnmsub132ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmsub132sd, kInstVfnmsub132sd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub132sd, kInstVfnmsub132sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmsub132ss, kInstVfnmsub132ss, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub132ss, kInstVfnmsub132ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmsub213pd, kInstVfnmsub213pd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub213pd, kInstVfnmsub213pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmsub213pd, kInstVfnmsub213pd, YmmReg, YmmReg, Mem)
  INST_3x(vfnmsub213pd, kInstVfnmsub213pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmsub213ps, kInstVfnmsub213ps, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub213ps, kInstVfnmsub213ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmsub213ps, kInstVfnmsub213ps, YmmReg, YmmReg, Mem)
  INST_3x(vfnmsub213ps, kInstVfnmsub213ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmsub213sd, kInstVfnmsub213sd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub213sd, kInstVfnmsub213sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmsub213ss, kInstVfnmsub213ss, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub213ss, kInstVfnmsub213ss, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmsub231pd, kInstVfnmsub231pd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub231pd, kInstVfnmsub231pd, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmsub231pd, kInstVfnmsub231pd, YmmReg, YmmReg, Mem)
  INST_3x(vfnmsub231pd, kInstVfnmsub231pd, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmsub231ps, kInstVfnmsub231ps, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub231ps, kInstVfnmsub231ps, XmmReg, XmmReg, XmmReg)
  INST_3x(vfnmsub231ps, kInstVfnmsub231ps, YmmReg, YmmReg, Mem)
  INST_3x(vfnmsub231ps, kInstVfnmsub231ps, YmmReg, YmmReg, YmmReg)

  INST_3x(vfnmsub231sd, kInstVfnmsub231sd, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub231sd, kInstVfnmsub231sd, XmmReg, XmmReg, XmmReg)

  INST_3x(vfnmsub231ss, kInstVfnmsub231ss, XmmReg, XmmReg, Mem)
  INST_3x(vfnmsub231ss, kInstVfnmsub231ss, XmmReg, XmmReg, XmmReg)

  // --------------------------------------------------------------------------
  // [BMI]
  // --------------------------------------------------------------------------

  INST_3x(andn, kInstAndn, GpReg, GpReg, GpReg)
  INST_3x(andn, kInstAndn, GpReg, GpReg, Mem)

  INST_3x(bextr, kInstBextr, GpReg, GpReg, GpReg)
  INST_3x(bextr, kInstBextr, GpReg, Mem, GpReg)

  INST_2x(blsi, kInstBlsi, GpReg, GpReg)
  INST_2x(blsi, kInstBlsi, GpReg, Mem)

  INST_2x(blsmsk, kInstBlsmsk, GpReg, GpReg)
  INST_2x(blsmsk, kInstBlsmsk, GpReg, Mem)

  INST_2x(blsr, kInstBlsr, GpReg, GpReg)
  INST_2x(blsr, kInstBlsr, GpReg, Mem)

  INST_2x(tzcnt, kInstTzcnt, GpReg, GpReg)
  INST_2x(tzcnt, kInstTzcnt, GpReg, Mem)

  // --------------------------------------------------------------------------
  // [LZCNT]
  // --------------------------------------------------------------------------

  INST_2x(lzcnt, kInstLzcnt, GpReg, GpReg)
  INST_2x(lzcnt, kInstLzcnt, GpReg, Mem)

  // --------------------------------------------------------------------------
  // [BMI2]
  // --------------------------------------------------------------------------

  INST_3x(bzhi, kInstBzhi, GpReg, GpReg, GpReg)
  INST_3x(bzhi, kInstBzhi, GpReg, Mem, GpReg)

  INST_3x(mulx, kInstMulx, GpReg, GpReg, GpReg)
  INST_3x(mulx, kInstMulx, GpReg, GpReg, Mem)

  INST_3x(pdep, kInstPdep, GpReg, GpReg, GpReg)
  INST_3x(pdep, kInstPdep, GpReg, GpReg, Mem)

  INST_3x(pext, kInstPext, GpReg, GpReg, GpReg)
  INST_3x(pext, kInstPext, GpReg, GpReg, Mem)

  INST_3i(rorx, kInstRorx, GpReg, GpReg, Imm)
  INST_3i(rorx, kInstRorx, GpReg, Mem, Imm)

  INST_3x(sarx, kInstSarx, GpReg, GpReg, GpReg)
  INST_3x(sarx, kInstSarx, GpReg, Mem, GpReg)

  INST_3x(shlx, kInstShlx, GpReg, GpReg, GpReg)
  INST_3x(shlx, kInstShlx, GpReg, Mem, GpReg)

  INST_3x(shrx, kInstShrx, GpReg, GpReg, GpReg)
  INST_3x(shrx, kInstShrx, GpReg, Mem, GpReg)

  // --------------------------------------------------------------------------
  // [RDRAND]
  // --------------------------------------------------------------------------

  INST_1x(rdrand, kInstRdrand, GpReg)

  // --------------------------------------------------------------------------
  // [F16C]
  // --------------------------------------------------------------------------

  INST_2x(vcvtph2ps, kInstVcvtph2ps, XmmReg, XmmReg)
  INST_2x(vcvtph2ps, kInstVcvtph2ps, XmmReg, Mem)
  INST_2x(vcvtph2ps, kInstVcvtph2ps, YmmReg, XmmReg)
  INST_2x(vcvtph2ps, kInstVcvtph2ps, YmmReg, Mem)

  INST_3i(vcvtps2ph, kInstVcvtps2ph, XmmReg, XmmReg, Imm)
  INST_3i(vcvtps2ph, kInstVcvtps2ph, Mem, XmmReg, Imm)
  INST_3i(vcvtps2ph, kInstVcvtps2ph, XmmReg, YmmReg, Imm)
  INST_3i(vcvtps2ph, kInstVcvtps2ph, Mem, YmmReg, Imm)
};

//! @}

} // x86x64 namespace
} // asmjit namespace

// ============================================================================
// [asmjit::x86]
// ============================================================================

#if defined(ASMJIT_BUILD_X86)

namespace asmjit {
namespace x86 {

//! @addtogroup asmjit_x86x64
//! @{

//! @brief X86-only assembler.
struct Assembler : public X86X64Assembler {
  ASMJIT_NO_COPY(Assembler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API Assembler(BaseRuntime* runtime);
  ASMJIT_API virtual ~Assembler();

  // --------------------------------------------------------------------------
  // [Reloc]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual size_t _relocCode(void* dst, Ptr base) const;

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error _emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3);

  // -------------------------------------------------------------------------
  // [Options]
  // -------------------------------------------------------------------------

  //! @overload
  ASMJIT_INLINE Assembler& short_()
  { _options |= kInstOptionShortForm; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& long_()
  { _options |= kInstOptionLongForm; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& taken()
  { _options |= kInstOptionTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& notTaken()
  { _options |= kInstOptionNotTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& lock()
  { _options |= kInstOptionLock; return *this; }

  //! @brief Force rex prefix.
  ASMJIT_INLINE Assembler& vex3()
  { _options |= kInstOptionVex3; return *this; }

  // --------------------------------------------------------------------------
  // [X86-Only Instructions]
  // --------------------------------------------------------------------------

  //! @brief Decimal adjust AL after addition (32-bit).
  INST_0x(daa, kInstDaa)
  //! @brief Decimal adjust AL after subtraction (32-bit).
  INST_0x(das, kInstDas)

  //! @brief Pop all Gp registers (EDI|ESI|EBP|EBX|EDX|ECX|EAX).
  INST_0x(popa, kInstPopa)

  //! @brief Push all Gp registers (EAX|ECX|EDX|EBX|original ESP|EBP|ESI|EDI).
  INST_0x(pusha, kInstPusha)
};

//! @}

} // x86 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X86

// ============================================================================
// [asmjit::x64]
// ============================================================================

#if defined(ASMJIT_BUILD_X64)

namespace asmjit {
namespace x64 {

//! @addtogroup asmjit_x86x64
//! @{

//! @brief X64-only assembler.
struct Assembler : public X86X64Assembler {
  ASMJIT_NO_COPY(Assembler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API Assembler(BaseRuntime* runtime);
  ASMJIT_API virtual ~Assembler();

  // --------------------------------------------------------------------------
  // [Reloc]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual size_t _relocCode(void* dst, Ptr base) const;

  // --------------------------------------------------------------------------
  // [Emit]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error _emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3);

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! @overload
  ASMJIT_INLINE Assembler& short_()
  { _options |= kInstOptionShortForm; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& long_()
  { _options |= kInstOptionLongForm; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& taken()
  { _options |= kInstOptionTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& notTaken()
  { _options |= kInstOptionNotTaken; return *this; }

  //! @overload
  ASMJIT_INLINE Assembler& lock()
  { _options |= kInstOptionLock; return *this; }

  //! @brief Force rex prefix.
  ASMJIT_INLINE Assembler& rex()
  { _options |= kInstOptionRex; return *this; }

  //! @brief Force rex prefix.
  ASMJIT_INLINE Assembler& vex3()
  { _options |= kInstOptionVex3; return *this; }

  // --------------------------------------------------------------------------
  // [X64-Only Instructions]
  // --------------------------------------------------------------------------

  //! @brief Convert dword to qword (RAX <- Sign Extend EAX).
  INST_0x(cdqe, kInstCdqe)
  //! @brief Convert qword to oword (RDX:RAX <- Sign Extend RAX).
  INST_0x(cqo, kInstCqo)

  //! @brief Compares the 128-bit value in RDX:RAX with the memory operand (64-bit).
  INST_1x(cmpxchg16b, kInstCmpxchg16b, Mem)

  //! @brief Move dword to qword with sign-extension.
  INST_2x(movsxd, kInstMovsxd, GpReg, GpReg)
  //! @overload
  INST_2x(movsxd, kInstMovsxd, GpReg, Mem)

  //! @brief Load ECX/RCX qwords from DS:[ESI/RSI] to RAX.
  INST_0x(rep_lodsq, kInstRepLodsq)

  //! @brief Move ECX/RCX qwords from DS:[ESI/RSI] to ES:[EDI/RDI].
  INST_0x(rep_movsq, kInstRepMovsq)

  //! @brief Fill ECX/RCX qwords at ES:[EDI/RDI] with RAX.
  INST_0x(rep_stosq, kInstRepStosq)

  //! @brief Repeated find nonmatching qwords in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repe_cmpsq, kInstRepeCmpsq)

  //! @brief Find non-rax qword starting at ES:[EDI/RDI].
  INST_0x(repe_scasq, kInstRepeScasq)

  //! @brief Repeated find nonmatching qwords in ES:[EDI/RDI] and DS:[ESI/RDI].
  INST_0x(repne_cmpsq, kInstRepneCmpsq)

  //! @brief Find RAX, starting at ES:[EDI/RDI].
  INST_0x(repne_scasq, kInstRepneScasq)

  using X86X64Assembler::movq;

  //! @overload
  INST_2x(movq, kInstMovq, GpReg, MmReg)
  //! @overload
  INST_2x(movq, kInstMovq, MmReg, GpReg)

  //! @overload
  INST_2x(movq, kInstMovq, GpReg, XmmReg)
  //! @overload
  INST_2x(movq, kInstMovq, XmmReg, GpReg)

  // --------------------------------------------------------------------------
  // [AVX]
  // --------------------------------------------------------------------------

  INST_2x(vmovq, kInstVmovq, XmmReg, GpReg)
  INST_2x(vmovq, kInstVmovq, GpReg, XmmReg)

  INST_3i(vpextrq, kInstVpextrq, GpReg, XmmReg, Imm)
  INST_3i(vpextrq, kInstVpextrq, Mem, XmmReg, Imm)

  INST_4i(vpinsrq, kInstVpinsrq, XmmReg, XmmReg, GpReg, Imm)
  INST_4i(vpinsrq, kInstVpinsrq, XmmReg, XmmReg, Mem, Imm)

  // --------------------------------------------------------------------------
  // [FSGSBASE]
  // --------------------------------------------------------------------------

  INST_1x(rdfsbase, kInstRdfsbase, GpReg)
  INST_1x(rdgsbase, kInstRdgsbase, GpReg)
  INST_1x(wrfsbase, kInstWrfsbase, GpReg)
  INST_1x(wrgsbase, kInstWrgsbase, GpReg)
};

//! @}

} // x64 namespace
} // asmjit namespace

#endif // ASMJIT_BUILD_X64

// ============================================================================
// [CodeGen-End]
// ============================================================================

#undef INST_0x

#undef INST_1x
#undef INST_1x_
#undef INST_1i
#undef INST_1i_
#undef INST_1cc

#undef INST_2x
#undef INST_2x_
#undef INST_2i
#undef INST_2i_
#undef INST_2cc

#undef INST_3x
#undef INST_3x_
#undef INST_3i
#undef INST_3i_

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_X86_X86ASSEMBLER_H
