// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_H
#define _ASMJIT_CORE_H

//! \defgroup asmjit_core Core
//! \brief AsmJit Core.

//! \defgroup asmjit_core_api Core
//! \ingroup asmjit_core
//! \brief Core API.

//! \defgroup asmjit_core_func Function
//! \ingroup asmjit_core
//! \brief Function API.

//! \defgroup asmjit_core_jit JIT
//! \ingroup asmjit_core
//! \brief JIT API.

//! \defgroup asmjit_core_support Support
//! \ingroup asmjit_core
//! \brief Support API.

//! \defgroup asmjit_core_ra RA Internals
//! \ingroup asmjit_core
//! \brief Register allocator internals.

//! \defgroup asmjit_core_zone Zone
//! \ingroup asmjit_core
//! \brief  Zone allocators and zone-allocated containers.

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
