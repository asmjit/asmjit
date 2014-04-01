// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_ASMJIT_H
#define _ASMJIT_ASMJIT_H

//! @mainpage
//!
//! @brief AsmJit - Complete x86/x64 JIT and Remote Assembler for C++.
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
//!
//! @section AsmJit_Main_ResourcesX86 External X86/X64 Assembler Resources
//! - http://www.agner.org/optimize/
//! - http://www.mark.masmcode.com/ (Assembler Tips)
//! - http://avisynth.org/mediawiki/Filter_SDK/Assembler_optimizing (Optimizing)
//! - http://www.ragestorm.net/distorm/ (Disassembling)
//!
//! @section AsmJit_Main_Terminology Terminology
//!
//! - <b>Non-volatile (preserved) register</b> - Register that can't be changed
//!   by callee (callee must save and restore it if it want to use it inside).
//!
//! - <b>Volatile (non-preserved) register</b> - The opossite. Register that can
//!   be freely used by callee. The caller must free all registers before calling
//!   other function.

//! @defgroup asmjit_base Base - base (backend neutral) classes.
//!
//! Contains all AsmJit classes and helper functions that are neutral or
//! abstract. All abstract classes are reimplemented for every supported
//! architecture.
//!
//! - See @c asmjit::Assembler class for low level code generation
//!   documentation.
//! - See @c asmjit::Operand for AsmJit operand's overview.
//!
//! @section AsmJit_Core_Registers Registers
//!
//! There are static objects that represents X86 and X64 registers. They can
//! be used directly (like @c eax, @c mm, @c xmm, ...) or created through
//! these functions:
//!
//! - @c asmjit::gpb_lo() - Get Gpb-lo register.
//! - @c asmjit::gpb_hi() - Get Gpb-hi register.
//! - @c asmjit::gpw() - Get Gpw register.
//! - @c asmjit::gpd() - Get Gpd register.
//! - @c asmjit::gpq() - Get Gpq Gp register.
//! - @c asmjit::gpz() - Get Gpd/Gpq register.
//! - @c asmjit::fp()  - Get Fp register.
//! - @c asmjit::mm()  - Get Mm register.
//! - @c asmjit::xmm() - Get Xmm register.
//! - @c asmjit::ymm() - Get Ymm register.
//!
//! @section AsmJit_Core_Addressing Addressing
//!
//! X86 and x64 architectures contains several addressing modes and most ones
//! are possible with AsmJit library. Memory represents are represented by
//! @c asmjit::BaseMem class. These functions are used to make operands that
//! represents memory addresses:
//!
//! - @c asmjit::ptr()
//! - @c asmjit::byte_ptr()
//! - @c asmjit::word_ptr()
//! - @c asmjit::dword_ptr()
//! - @c asmjit::qword_ptr()
//! - @c asmjit::tword_ptr()
//! - @c asmjit::oword_ptr()
//! - @c asmjit::yword_ptr()
//! - @c asmjit::intptr_ptr()
//!
//! Most useful function to make pointer should be @c asmjit::ptr(). It creates
//! pointer to the target with unspecified size. Unspecified size works in all
//! intrinsics where are used registers (this means that size is specified by
//! register operand or by instruction itself). For example @c asmjit::ptr()
//! can't be used with @c asmjit::Assembler::inc() instruction. In this case
//! size must be specified and it's also reason to make difference between
//! pointer sizes.
//!
//! Supported are simple address forms (register + displacement) and complex
//! address forms (register + (register << shift) + displacement).
//!
//! @section AsmJit_Core_Immediates Immediates
//!
//! Immediate values are constants thats passed directly after instruction
//! opcode. To create such value use @c asmjit::imm() or @c asmjit::imm_u()
//! methods to create signed or unsigned immediate value.
//!
//! @sa @c asmjit::BaseCompiler.

//! @defgroup asmjit_compiler Compiler (high-level code generation).
//!
//! Contains classes related to @c asmjit::Compiler that can be used
//! to generate code using high-level constructs.
//!
//! - See @c Compiler class for high level code generation
//!   documentation - calling conventions, function declaration
//!   and variables management.

//! @defgroup asmjit_config Configuration.
//!
//! Contains macros that can be redefined to fit into any project.

//! @defgroup asmjit_cpuinfo Cpu information.
//!
//! X86 or x64 cpuid instruction allows to get information about processor
//! vendor and it's features. It's always used to detect features like MMX,
//! SSE and other newer ones.
//!
//! AsmJit library supports low level cpuid call implemented internally as
//! C++ function using inline assembler or intrinsics and also higher level
//! CPU features detection. The low level function (also used by higher level
//! one) is @c asmjit::cpuid().
//!
//! AsmJit library also contains higher level function @c asmjit::getCpu()
//! that returns features detected by the library. The detection process is
//! done only once and the returned object is always the same. @c BaseCpuInfo
//! structure does not contain only information through @c asmjit::cpuid(), but
//! there is also small multiplatform code to detect number of processors
//! (or cores) through operating system API.
//!
//! It's recommended to use @c asmjit::cpuInfo to detect and check for
//! host processor features.
//!
//! Example how to use asmjit::cpuid():
//!
//! @code
//! // All functions and structures are in asmjit namesapce.
//! using namespace asmjit;
//!
//! // Here will be retrieved result of cpuid call.
//! CpuId out;
//!
//! // Use cpuid function to do the job.
//! cpuid(0 /* eax */, &out /* eax, ebx, ecx, edx */);
//!
//! // If eax argument to cpuid is 0, ebx, ecx and edx registers
//! // are filled with cpu vendor.
//! char vendor[13];
//! memcpy(i->vendor, &out.ebx, 4);
//! memcpy(i->vendor + 4, &out.edx, 4);
//! memcpy(i->vendor + 8, &out.ecx, 4);
//! vendor[12] = '\0';
//!
//! // Print vendor
//! puts(vendor);
//! @endcode
//!
//! If the high-level interface of asmjit::BaseCpuInfo is not enough, you can
//! use low-level asmjit::cpuid() when running on x86/x64 host, but please read
//! processor manuals provided by Intel, AMD or other manufacturer for cpuid
//! details.
//!
//! Example of using @c BaseCpuInfo::getHost():
//!
//! @code
//! // All functions and structures are in asmjit namesapce.
//! using namespace asmjit;
//!
//! // Call to cpuInfo return BaseCpuInfo structure that shouldn't be modified.
//! // Make it const by default.
//! const BaseCpuInfo* cpuInfo = BaseCpuInfo::getHost();
//!
//! // Now you are able to get specific features.
//!
//! // Processor has SSE2
//! if (cpuInfo->hasFeature(kCpuFeatureSse2)) {
//!   // your code...
//! }
//! // Processor has MMX
//! else if (cpuInfo->hasFeature(kCpuFeature_MMX)) {
//!   // your code...
//! }
//! // Processor is old, no SSE2 or MMX support.
//! else {
//!   // your code...
//! }
//! @endcode
//!
//! Better example is in app/test/testcpu.cpp file.

//! @defgroup asmjit_logging Logging and error handling.
//!
//! Contains classes related to loging. Currently logging is implemented in
//! @ref asmjit::BaseLogger class. The function @ref asmjit::BaseLogger::log()
//! can be overridden to redirect logging into any user-defined stream.
//!
//! To log your assembler output to FILE stream use this code:
//!
//! @code
//! // Create assembler
//! Assembler a;
//!
//! // Create and set file based logger
//! FileLogger logger(stderr);
//! a.setLogger(&logger);
//! @endcode
//!
//! You can see that logging goes through @c Assembler. If you are using
//! @c Compiler and you want to log messages in correct assembler order,
//! you should look at @ref Compiler::comment() method. It allows  you to
//! insert text message into items stream so the @c Compiler is able to
//! send messages to @ref Assembler in correct order.
//!
//! @sa @c asmjit::BaseLogger, @c asmjit::FileLogger.


//! @defgroup AsmJit_MemoryManagement Virtual memory management.
//!
//! Using @c asmjit::Assembler or @c asmjit::Compiler to generate machine
//! code is not final step. Each generated code needs to run in memory
//! that is not protected against code execution. To alloc this code it's
//! needed to use operating system functions provided to enable execution
//! code in specified memory block or to allocate memory that is not
//! protected. The solution is always to use @c See asmjit::Assembler::make()
//! and @c asmjit::Compiler::make() functions that can allocate memory and
//! relocate code for you. But AsmJit also contains classes for manual memory
//! management thats internally used by AsmJit but can be used by programmers
//! too.
//!
//! Memory management contains low level and high level classes related to
//! allocating and freeing virtual memory. Low level class is
//! @c asmjit::VMem that can allocate and free full pages of virtual memory
//! provided by operating system. Higher level class is @c asmjit::MemoryManager
//! that is able to manage complete allocation and free mechanism. It
//! internally uses larger chunks of memory to make allocation fast and
//! effective.
//!
//! Using @c asmjit::VMem::alloc() is cross-platform way how to allocate this
//! kind of memory without worrying about operating system and it's API. Each
//! memory block that is no longer needed should be released by @ref
//! asmjit::VMem::release() method. Higher-level interface for virtual memory
//! allocation can be found at asmjit::MemoryManager class.
//!
//! @sa @c asmjit::VMem, @ asmjit::MemoryManager.


//! @addtogroup asmjit_config
//! @{

//! @def ASMJIT_OS_WINDOWS
//! @brief Macro that is declared if AsmJit is compiled for Windows.

//! @def ASMJIT_OS_POSIX
//! @brief Macro that is declared if AsmJit is compiled for unix like
//! operating system.

//! @def ASMJIT_API
//! @brief Attribute that's added to classes that can be exported if AsmJit
//! is compiled as a dll library.

//! @def ASMJIT_ASSERT
//! @brief Assertion macro. Default implementation calls
//! @c asmjit::assertionFailed() function.

//! @}


//! @namespace asmjit
//! @brief Main AsmJit library namespace.
//!
//! There are not other namespaces used in AsmJit library.

// [Dependencies - Core]
#include "base.h"

// [Dependencies - X86/X64]
#if defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64)
#include "x86.h"
#endif // ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64

// [Dependencies - Host]
#include "host.h"

// [Guard]
#endif // _ASMJIT_ASMJIT_H
