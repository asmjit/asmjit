// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_H
#define _ASMJIT_CORE_H

//! \defgroup asmjit_core Core
//! \brief Core API.
//!
//! API that provides classes and functions not specific to any architecture.

//! \defgroup asmjit_builder Builder
//! \brief Builder API.
//!
//! Both Builder and Compiler are emitters that emit everything to a representation
//! that allows further processing. The code stored in such representation is
//! completely safe to be patched, simplified, reordered, obfuscated, removed,
//! injected, analyzed, or processed some other way. Each instruction, label,
//! directive, or other building block is stored as \ref BaseNode (or derived
//! class like \ref InstNode or \ref LabelNode) and contains all the information
//! necessary to pass that node later to the Assembler.

//! \defgroup asmjit_compiler Compiler
//! \brief Compiler API.
//!
//! Compiler tool is built on top of a \ref asmjit_builder API and adds register
//! allocation and support for defining and calling functions into it. At the
//! moment it's the easiest way to generate some code as most architecture and
//! OS specific stuff is properly abstracted, however, abstractions also mean
//! that not everything is possible with the Compiler.

//! \defgroup asmjit_func Function
//! \brief Function API.

//! \defgroup asmjit_jit JIT
//! \brief JIT API and Virtual Memory Management.

//! \defgroup asmjit_zone Zone
//! \brief Zone allocator and zone allocated containers.

//! \defgroup asmjit_support Support
//! \brief Support API.

//! \cond INTERNAL
//! \defgroup asmjit_ra RA
//! \brief Register allocator internals.
//! \endcond

#include "./core/globals.h"

#include "./core/arch.h"
#include "./core/assembler.h"
#include "./core/builder.h"
#include "./core/callconv.h"
#include "./core/codeholder.h"
#include "./core/compiler.h"
#include "./core/constpool.h"
#include "./core/cpuinfo.h"
#include "./core/datatypes.h"
#include "./core/emitter.h"
#include "./core/features.h"
#include "./core/func.h"
#include "./core/inst.h"
#include "./core/jitallocator.h"
#include "./core/jitruntime.h"
#include "./core/logging.h"
#include "./core/operand.h"
#include "./core/osutils.h"
#include "./core/string.h"
#include "./core/support.h"
#include "./core/target.h"
#include "./core/type.h"
#include "./core/virtmem.h"
#include "./core/zone.h"
#include "./core/zonehash.h"
#include "./core/zonelist.h"
#include "./core/zonetree.h"
#include "./core/zonestack.h"
#include "./core/zonestring.h"
#include "./core/zonevector.h"

#endif // _ASMJIT_CORE_H
