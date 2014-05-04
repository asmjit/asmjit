// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ASMJIT_H
#define _ASMJIT_ASMJIT_H

// ============================================================================
// [asmjit_mainpage]
// ============================================================================

//! @mainpage
//!
//! AsmJit - Complete x86/x64 JIT and Remote Assembler for C++.
//!
//! AsmJit is a complete JIT and remote assembler for C++ language. It can
//! generate native code for x86 and x64 architectures having support for
//! a full instruction set, from legacy MMX to the newest AVX2. It has a
//! type-safe API that allows C++ compiler to do a semantic checks at
//! compile-time even before the assembled code is generated or run.
//!
//! AsmJit is not a virtual machine (VM). It doesn't have functionality to
//! implement VM out of the box; however, it can be be used as a JIT backend
//! for your own VM. The usage of AsmJit is not limited at all; it's suitable
//! for multimedia, VM backends or remote code generation.
//!
//! @section AsmJit_Concepts Code Generation Concepts
//!
//! AsmJit has two completely different code generation concepts. The difference
//! is in how the code is generated. The first concept, also referred as the low
//! level concept, is called 'Assembler' and it's the same as writing RAW
//! assembly by using physical registers directly. In this case AsmJit does only
//! instruction encoding, verification and relocation.
//!
//! The second concept, also referred as the high level concept, is called
//! 'Compiler'. Compiler lets you use virtually unlimited number of registers
//! (called variables) significantly simplifying the code generation process.
//! Compiler allocates these virtual registers to physical registers after the
//! code generation is done. This requires some extra effort - Compiler has to
//! generate information for each node (instruction, function declaration,
//! function call) in the code, perform a variable liveness analysis and
//! translate the code having variables into code having only registers.
//!
//! In addition, Compiler understands functions and function calling conventions.
//! It has been designed in a way that the code generated is always a function
//! having prototype like in a programming language. By having a function
//! prototype the Compiler is able to insert prolog and epilog to a function
//! being generated and it is able to call a function inside a generated one.
//!
//! There is no conclusion on which concept is better. Assembler brings full
//! control on how the code is generated, while Compiler makes the generation
//! more portable.
//!
//! @section AsmJit_Main_CodeGeneration Code Generation
//!
//! - @ref asmjit_base "Assembler core" - Operands, intrinsics and low-level assembler.
//! - @ref asmjit_compiler "Compiler" - High level code generation.
//! - @ref asmjit_cpuinfo "Cpu Information" - Get information about host processor.
//! - @ref asmjit_logging "Logging" - Logging and error handling.
//! - @ref AsmJit_MemoryManagement "Memory Management" - Virtual memory management.
//!
//! @section AsmJit_Main_Configuration Configuration, Definitions and Utilities
//!
//! - @ref asmjit_config "Configuration" - Macros used to configure AsmJit.
//!
//! @section AsmJit_Main_HomePage AsmJit Homepage
//!
//! - http://code.google.com/p/asmjit/

// [Dependencies - Base]
#include "base.h"

// [Dependencies - X86/X64]
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
#include "x86.h"
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64

// [Dependencies - Host]
#include "host.h"

// [Guard]
#endif // _ASMJIT_ASMJIT_H
