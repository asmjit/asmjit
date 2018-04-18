// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_H
#define _ASMJIT_CORE_H

// [Dependencies]
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
#include "./core/jitutils.h"
#include "./core/logging.h"
#include "./core/memmgr.h"
#include "./core/operand.h"
#include "./core/osutils.h"
#include "./core/stringbuilder.h"
#include "./core/stringutils.h"
#include "./core/support.h"
#include "./core/target.h"
#include "./core/type.h"
#include "./core/zone.h"
#include "./core/zonehash.h"
#include "./core/zonelist.h"
#include "./core/zonetree.h"
#include "./core/zonestack.h"
#include "./core/zonestring.h"
#include "./core/zonevector.h"

// DEPRECATED:
ASMJIT_BEGIN_NAMESPACE

#ifndef ASMJIT_DISABLE_BUILDER
typedef ASMJIT_DEPRECATED(BaseBuilder   CodeBuilder , "Use `BaseBuilder`");
typedef ASMJIT_DEPRECATED(BaseNode      CBNode      , "Use `BaseNode`");
typedef ASMJIT_DEPRECATED(InstNode      CBInst      , "Use `InstNode`");
typedef ASMJIT_DEPRECATED(LabelNode     CBLabel     , "Use `LabelNode`");
typedef ASMJIT_DEPRECATED(AlignNode     CBAlign     , "Use `AlignNode`");
typedef ASMJIT_DEPRECATED(EmbedDataNode CBData      , "Use `EmbedDataNode`");
typedef ASMJIT_DEPRECATED(LabelDataNode CBLabelData , "Use `LabelDataNode`");
typedef ASMJIT_DEPRECATED(ConstPoolNode CBConstPool , "Use `ConstPoolNode`");
typedef ASMJIT_DEPRECATED(CommentNode   CBComment   , "Use `CommentNode`");
typedef ASMJIT_DEPRECATED(SentinelNode  CBSentinel  , "Use `SentinelNode`");
typedef ASMJIT_DEPRECATED(Pass          CBPass      , "Use `Pass`");
#endif

#ifndef ASMJIT_DISABLE_COMPILER
typedef ASMJIT_DEPRECATED(BaseCompiler  CodeCompiler, "Use `BaseCompiler`");
typedef ASMJIT_DEPRECATED(FuncNode      CCFunc      , "Use `FuncNode`");
typedef ASMJIT_DEPRECATED(FuncRetNode   CCFuncRet   , "Use `FuncRetNode`");
typedef ASMJIT_DEPRECATED(FuncCallNode  CCFuncCall  , "Use `FuncCallNode`");
#endif
ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_H
