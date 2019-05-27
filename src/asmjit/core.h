// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_H
#define _ASMJIT_CORE_H

//! \defgroup asmjit_core Core
//! \brief Core API.

//! \defgroup asmjit_func Function
//! \brief Function API.

//! \defgroup asmjit_jit JIT
//! \brief JIT API and Virtual Memory Management.

//! \defgroup asmjit_support Support
//! \brief Support API.

//! \defgroup asmjit_zone Zone
//! \brief Zone allocator and zone allocated containers.

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
