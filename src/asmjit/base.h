// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_H
#define _ASMJIT_BASE_H

// ============================================================================
// [asmjit_base]
// ============================================================================

//! @defgroup asmjit_base Base
//!
//! @brief AsmJit Base API.
//!
//! Contains all `asmjit` classes and helper functions that are architecture
//! independent or abstract. Abstract classes are implemented by the backend,
//! for example `BaseAssembler` is implemented by `x86x64::X86X64Assembler`.
//!
//! - See `BaseAssembler` for low level code generation documentation.
//! - See `BaseCompiler` for high level code generation documentation.
//! - See `Operand` for operand's overview.
//!
//! @section AsmJit_Core_Registers Registers
//!
//! There are static objects that represents X86 and X64 registers. They can
//! be used directly (like `eax`, `mm`, `xmm`, ...) or created through
//! these functions:
//!
//! - `asmjit::gpb_lo()` - Get Gpb-lo register.
//! - `asmjit::gpb_hi()` - Get Gpb-hi register.
//! - `asmjit::gpw()` - Get Gpw register.
//! - `asmjit::gpd()` - Get Gpd register.
//! - `asmjit::gpq()` - Get Gpq Gp register.
//! - `asmjit::gpz()` - Get Gpd/Gpq register.
//! - `asmjit::fp()`  - Get Fp register.
//! - `asmjit::mm()`  - Get Mm register.
//! - `asmjit::xmm()` - Get Xmm register.
//! - `asmjit::ymm()` - Get Ymm register.
//!
//! @section AsmJit_Core_Addressing Addressing
//!
//! X86 and x64 architectures contains several addressing modes and most ones
//! are possible with AsmJit library. Memory represents are represented by
//! `BaseMem` class. These functions are used to make operands that represents
//! memory addresses:
//!
//! - `asmjit::ptr()`
//! - `asmjit::byte_ptr()`
//! - `asmjit::word_ptr()`
//! - `asmjit::dword_ptr()`
//! - `asmjit::qword_ptr()`
//! - `asmjit::tword_ptr()`
//! - `asmjit::oword_ptr()`
//! - `asmjit::yword_ptr()`
//! - `asmjit::intptr_ptr()`
//!
//! Most useful function to make pointer should be `asmjit::ptr()`. It creates
//! pointer to the target with unspecified size. Unspecified size works in all
//! intrinsics where are used registers (this means that size is specified by
//! register operand or by instruction itself). For example `asmjit::ptr()`
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

// ============================================================================
// [asmjit_base_globals]
// ============================================================================

//! @defgroup asmjit_base_globals Globals 
//! @ingroup asmjit_base
//!
//! @brief Global definitions, macros and functions.

// ============================================================================
// [asmjit_base_codegen]
// ============================================================================

//! @defgroup asmjit_base_codegen Code Generation (Base)
//! @ingroup asmjit_base
//!
//! @brief Low-level and high-level code generation.

// ============================================================================
// [asmjit_base_cpu_info]
// ============================================================================

//! @defgroup asmjit_base_cpu_info CPU Information (Base)
//! @ingroup asmjit_base
//!
//! @brief CPU Information Interface, platform neutral.

// ============================================================================
// [asmjit_base_logging_and_errors]
// ============================================================================

//! @defgroup asmjit_base_logging_and_errors Logging and Error Handling
//! @ingroup asmjit_base
//!
//! @brief Logging and error handling.
//!
//! AsmJit contains robust interface that can be used to log the generated code
//! and to handle possible errors. Base logging interface is defined in `Logger`
//! class that is abstract and can be overridden. AsmJit contains two loggers
//! that can be used out-of-the box - `FileLogger` that logs into a  `FILE*`
//! and `StringLogger` that just concatenates all log messages without sending
//! them to a stream.
//! 
//! The following snippet shows how to setup a logger that logs to `stderr`:
//!
//! ~~~
//! // `FileLogger` instance.
//! FileLogger logger(stderr);
//!
//! // `Compiler` or any other `CodeGen` interface.
//! host::Compiler c;
//!
//! // use `setLogger` to replace the `CodeGen` logger.
//! c.setLogger(&logger);
//! ~~~
//!
//! @sa @ref `Logger`, @ref `FileLogger`, @ref `StringLogger`.

// ============================================================================
// [asmjit_base_util]
// ============================================================================

//! @defgroup asmjit_base_util Utilities
//! @ingroup asmjit_base
//!
//! @brief Utilities inside AsmJit made public.
//!
//! AsmJit contains numerous utility classes that are needed by the library
//! itself. The most useful ones have been made public and are now exported.
//!
//! POD-Containers
//! --------------
//!
//! TODO: Documentation
//!
//! String Builder
//! --------------
//!
//! TODO: Documentation
//!
//! Integer Utilities
//! -----------------
//!
//! TODO: Documentation
//!
//! Zone Memory Allocator
//! ---------------------
//!
//! TODO: Documentation
//!
//! CPU Ticks
//! ---------
//!
//! TODO: Documentation

// ============================================================================
// [asmjit_base_vectypes]
// ============================================================================

//! @defgroup asmjit_base_vectypes Vector Types
//! @ingroup asmjit_base
//!
//! @brief Vector types can be used to create a data which is stored in the 
//! machine code.

// ============================================================================
// [asmjit::]
// ============================================================================

//! @namespace asmjit
//! Main AsmJit library namespace.
//!
//! There are not other namespaces used in AsmJit library.

// ============================================================================
// [Dependencies - AsmJit]
// ============================================================================

#include "build.h"

#include "base/assembler.h"
#include "base/codegen.h"
#include "base/compiler.h"
#include "base/constpool.h"
#include "base/cpuinfo.h"
#include "base/cputicks.h"
#include "base/defs.h"
#include "base/error.h"
#include "base/func.h"
#include "base/globals.h"
#include "base/intutil.h"
#include "base/lock.h"
#include "base/logger.h"
#include "base/podlist.h"
#include "base/podvector.h"
#include "base/string.h"
#include "base/vectypes.h"
#include "base/vmem.h"
#include "base/zone.h"

// [Guard]
#endif // _ASMJIT_BASE_H
