// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER) && (defined(ASMJIT_BUILD_X86) || defined(ASMJIT_BUILD_X64))

// [Dependencies - AsmJit]
#include "../base/containers.h"
#include "../base/utils.h"
#include "../x86/x86assembler.h"
#include "../x86/x86compiler.h"
#include "../x86/x86compilercontext_p.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Debug]
// ============================================================================

#if !defined(ASMJIT_DEBUG)
#define ASMJIT_ASSERT_OPERAND(op) \
  do {} while(0)
#else
#define ASMJIT_ASSERT_OPERAND(op) \
  do { \
    if (op.isVar() || op.isLabel()) { \
      ASMJIT_ASSERT(op.getId() != kInvalidValue); \
    } \
  } while(0)
#endif

// ============================================================================
// [asmjit::X86VarInfo]
// ============================================================================

#define C(_Class_) kX86RegClass##_Class_
#define D(_Desc_) kVarFlag##_Desc_

const X86VarInfo _x86VarInfo[] = {
  /* 00: kVarTypeInt8     */ { kX86RegTypeGpbLo, 1 , C(Gp) , 0                , "gpb" },
  /* 01: kVarTypeUInt8    */ { kX86RegTypeGpbLo, 1 , C(Gp) , 0                , "gpb" },
  /* 02: kVarTypeInt16    */ { kX86RegTypeGpw  , 2 , C(Gp) , 0                , "gpw" },
  /* 03: kVarTypeUInt16   */ { kX86RegTypeGpw  , 2 , C(Gp) , 0                , "gpw" },
  /* 04: kVarTypeInt32    */ { kX86RegTypeGpd  , 4 , C(Gp) , 0                , "gpd" },
  /* 05: kVarTypeUInt32   */ { kX86RegTypeGpd  , 4 , C(Gp) , 0                , "gpd" },
  /* 06: kVarTypeInt64    */ { kX86RegTypeGpq  , 8 , C(Gp) , 0                , "gpq" },
  /* 07: kVarTypeUInt64   */ { kX86RegTypeGpq  , 8 , C(Gp) , 0                , "gpq" },
  /* 08: kVarTypeIntPtr   */ { 0               , 0 , C(Gp) , 0                , ""    }, // Remapped.
  /* 09: kVarTypeUIntPtr  */ { 0               , 0 , C(Gp) , 0                , ""    }, // Remapped.
  /* 10: kVarTypeFp32     */ { kX86RegTypeFp   , 4 , C(Fp) , D(Sp)            , "fp"  },
  /* 11: kVarTypeFp64     */ { kX86RegTypeFp   , 8 , C(Fp) , D(Dp)            , "fp"  },
  /* 12: kX86VarTypeMm    */ { kX86RegTypeMm   , 8 , C(Mm) , 0                , "mm"  },
  /* 13: kX86VarTypeK     */ { kX86RegTypeK    , 8 , C(K)  , 0                , "k"   },
  /* 14: kX86VarTypeXmm   */ { kX86RegTypeXmm  , 16, C(Xyz), 0                , "xmm" },
  /* 15: kX86VarTypeXmmSs */ { kX86RegTypeXmm  , 4 , C(Xyz), D(Sp)            , "xmm" },
  /* 16: kX86VarTypeXmmPs */ { kX86RegTypeXmm  , 16, C(Xyz), D(Sp) | D(Packed), "xmm" },
  /* 17: kX86VarTypeXmmSd */ { kX86RegTypeXmm  , 8 , C(Xyz), D(Dp)            , "xmm" },
  /* 18: kX86VarTypeXmmPd */ { kX86RegTypeXmm  , 16, C(Xyz), D(Dp) | D(Packed), "xmm" },
  /* 19: kX86VarTypeYmm   */ { kX86RegTypeYmm  , 32, C(Xyz), 0                , "ymm" },
  /* 20: kX86VarTypeYmmPs */ { kX86RegTypeYmm  , 32, C(Xyz), D(Sp) | D(Packed), "ymm" },
  /* 21: kX86VarTypeYmmPd */ { kX86RegTypeYmm  , 32, C(Xyz), D(Dp) | D(Packed), "ymm" },
  /* 22: kX86VarTypeZmm   */ { kX86RegTypeZmm  , 64, C(Xyz), 0                , "zmm" },
  /* 23: kX86VarTypeZmmPs */ { kX86RegTypeZmm  , 64, C(Xyz), D(Sp) | D(Packed), "zmm" },
  /* 24: kX86VarTypeZmmPd */ { kX86RegTypeZmm  , 64, C(Xyz), D(Dp) | D(Packed), "zmm" }
};

#undef D
#undef C

#if defined(ASMJIT_BUILD_X86)
const uint8_t _x86VarMapping[kX86VarTypeCount] = {
  /* 00: kVarTypeInt8     */ kVarTypeInt8,
  /* 01: kVarTypeUInt8    */ kVarTypeUInt8,
  /* 02: kVarTypeInt16    */ kVarTypeInt16,
  /* 03: kVarTypeUInt16   */ kVarTypeUInt16,
  /* 04: kVarTypeInt32    */ kVarTypeInt32,
  /* 05: kVarTypeUInt32   */ kVarTypeUInt32,
  /* 06: kVarTypeInt64    */ kInvalidVar,     // Invalid in 32-bit mode.
  /* 07: kVarTypeUInt64   */ kInvalidVar,     // Invalid in 32-bit mode.
  /* 08: kVarTypeIntPtr   */ kVarTypeInt32,   // Remapped to Int32.
  /* 09: kVarTypeUIntPtr  */ kVarTypeUInt32,  // Remapped to UInt32.
  /* 10: kVarTypeFp32     */ kVarTypeFp32,
  /* 11: kVarTypeFp64     */ kVarTypeFp64,
  /* 12: kX86VarTypeMm    */ kX86VarTypeMm,
  /* 13: kX86VarTypeK     */ kX86VarTypeK,
  /* 14: kX86VarTypeXmm   */ kX86VarTypeXmm,
  /* 15: kX86VarTypeXmmSs */ kX86VarTypeXmmSs,
  /* 16: kX86VarTypeXmmPs */ kX86VarTypeXmmPs,
  /* 17: kX86VarTypeXmmSd */ kX86VarTypeXmmSd,
  /* 18: kX86VarTypeXmmPd */ kX86VarTypeXmmPd,
  /* 19: kX86VarTypeYmm   */ kX86VarTypeYmm,
  /* 20: kX86VarTypeYmmPs */ kX86VarTypeYmmPs,
  /* 21: kX86VarTypeYmmPd */ kX86VarTypeYmmPd,
  /* 22: kX86VarTypeZmm   */ kX86VarTypeZmm,
  /* 23: kX86VarTypeZmmPs */ kX86VarTypeZmmPs,
  /* 24: kX86VarTypeZmmPd */ kX86VarTypeZmmPd
};
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
const uint8_t _x64VarMapping[kX86VarTypeCount] = {
  /* 00: kVarTypeInt8     */ kVarTypeInt8,
  /* 01: kVarTypeUInt8    */ kVarTypeUInt8,
  /* 02: kVarTypeInt16    */ kVarTypeInt16,
  /* 03: kVarTypeUInt16   */ kVarTypeUInt16,
  /* 04: kVarTypeInt32    */ kVarTypeInt32,
  /* 05: kVarTypeUInt32   */ kVarTypeUInt32,
  /* 06: kVarTypeInt64    */ kVarTypeInt64,
  /* 07: kVarTypeUInt64   */ kVarTypeUInt64,
  /* 08: kVarTypeIntPtr   */ kVarTypeInt64,   // Remapped to Int64.
  /* 09: kVarTypeUIntPtr  */ kVarTypeUInt64,  // Remapped to UInt64.
  /* 10: kVarTypeFp32     */ kVarTypeFp32,
  /* 11: kVarTypeFp64     */ kVarTypeFp64,
  /* 12: kX86VarTypeMm    */ kX86VarTypeMm,
  /* 13: kX86VarTypeK     */ kX86VarTypeK,
  /* 14: kX86VarTypeXmm   */ kX86VarTypeXmm,
  /* 15: kX86VarTypeXmmSs */ kX86VarTypeXmmSs,
  /* 16: kX86VarTypeXmmPs */ kX86VarTypeXmmPs,
  /* 17: kX86VarTypeXmmSd */ kX86VarTypeXmmSd,
  /* 18: kX86VarTypeXmmPd */ kX86VarTypeXmmPd,
  /* 19: kX86VarTypeYmm   */ kX86VarTypeYmm,
  /* 20: kX86VarTypeYmmPs */ kX86VarTypeYmmPs,
  /* 21: kX86VarTypeYmmPd */ kX86VarTypeYmmPd,
  /* 22: kX86VarTypeZmm   */ kX86VarTypeZmm,
  /* 23: kX86VarTypeZmmPs */ kX86VarTypeZmmPs,
  /* 24: kX86VarTypeZmmPd */ kX86VarTypeZmmPd
};
#endif // ASMJIT_BUILD_X64

// ============================================================================
// [asmjit::X86CallNode - Arg / Ret]
// ============================================================================

bool X86CallNode::_setArg(uint32_t i, const Operand& op) {
  if ((i & ~kFuncArgHi) >= _x86Decl.getNumArgs())
    return false;

  _args[i] = op;
  return true;
}

bool X86CallNode::_setRet(uint32_t i, const Operand& op) {
  if (i >= 2)
    return false;

  _ret[i] = op;
  return true;
}

// ============================================================================
// [asmjit::X86Compiler - Construction / Destruction]
// ============================================================================

X86Compiler::X86Compiler(X86Assembler* assembler)
  : Compiler(),
    zax(NoInit),
    zcx(NoInit),
    zdx(NoInit),
    zbx(NoInit),
    zsp(NoInit),
    zbp(NoInit),
    zsi(NoInit),
    zdi(NoInit) {

  _regCount.reset();
  zax = x86::noGpReg;
  zcx = x86::noGpReg;
  zdx = x86::noGpReg;
  zbx = x86::noGpReg;
  zsp = x86::noGpReg;
  zbp = x86::noGpReg;
  zsi = x86::noGpReg;
  zdi = x86::noGpReg;

  if (assembler != NULL)
    attach(assembler);
}

X86Compiler::~X86Compiler() {
  reset(true);
}

// ============================================================================
// [asmjit::X86Compiler - Attach / Reset]
// ============================================================================

Error X86Compiler::attach(Assembler* assembler) {
  ASMJIT_ASSERT(assembler != NULL);

  if (_assembler != NULL)
    return kErrorInvalidState;

  uint32_t arch = assembler->getArch();
  switch (arch) {
#if defined(ASMJIT_BUILD_X86)
    case kArchX86:
      _targetVarMapping = _x86VarMapping;
      break;
#endif // ASMJIT_BUILD_X86

#if defined(ASMJIT_BUILD_X64)
    case kArchX64:
      _targetVarMapping = _x64VarMapping;
      break;
#endif // ASMJIT_BUILD_X64

    default:
      return kErrorInvalidArch;
  }

  assembler->_attached(this);

  _arch = static_cast<uint8_t>(arch);
  _regSize = static_cast<uint8_t>(assembler->getRegSize());
  _regCount = static_cast<X86Assembler*>(assembler)->getRegCount();
  _finalized = false;

  zax = static_cast<X86Assembler*>(assembler)->zax;
  zcx = static_cast<X86Assembler*>(assembler)->zcx;
  zdx = static_cast<X86Assembler*>(assembler)->zdx;
  zbx = static_cast<X86Assembler*>(assembler)->zbx;
  zsp = static_cast<X86Assembler*>(assembler)->zsp;
  zbp = static_cast<X86Assembler*>(assembler)->zbp;
  zsi = static_cast<X86Assembler*>(assembler)->zsi;
  zdi = static_cast<X86Assembler*>(assembler)->zdi;

  return kErrorOk;
}

void X86Compiler::reset(bool releaseMemory) {
  Compiler::reset(releaseMemory);

  _regCount.reset();
  zax = x86::noGpReg;
  zcx = x86::noGpReg;
  zdx = x86::noGpReg;
  zbx = x86::noGpReg;
  zsp = x86::noGpReg;
  zbp = x86::noGpReg;
  zsi = x86::noGpReg;
  zdi = x86::noGpReg;
}

// ============================================================================
// [asmjit::X86Compiler - Finalize]
// ============================================================================

Error X86Compiler::finalize() {
  X86Assembler* assembler = getAssembler();
  if (assembler == NULL)
    return kErrorOk;

  // Flush the global constant pool.
  if (_globalConstPoolLabel.isInitialized()) {
    embedConstPool(_globalConstPoolLabel, _globalConstPool);

    _globalConstPoolLabel.reset();
    _globalConstPool.reset();
  }

  if (_firstNode == NULL)
    return kErrorOk;

  X86Context context(this);
  Error error = kErrorOk;

  HLNode* node = _firstNode;
  HLNode* start;

  // Find all functions and use the `X86Context` to translate/emit them.
  do {
    start = node;
    _resetTokenGenerator();

    if (node->getType() == kHLNodeTypeFunc) {
      node = static_cast<X86FuncNode*>(start)->getEnd();
      error = context.compile(static_cast<X86FuncNode*>(start));

      if (error != kErrorOk)
        break;
    }

    do {
      node = node->getNext();
    } while (node != NULL && node->getType() != kHLNodeTypeFunc);

    error = context.serialize(assembler, start, node);
    context.cleanup();

    if (error != kErrorOk)
      break;
  } while (node != NULL);

  reset(false);
  return error;
}

// ============================================================================
// [asmjit::X86Compiler - Inst]
// ============================================================================

//! Get compiler instruction item size without operands assigned.
static ASMJIT_INLINE size_t X86Compiler_getInstSize(uint32_t code) {
  return Utils::inInterval<uint32_t>(code, _kX86InstIdJbegin, _kX86InstIdJend) ? sizeof(HLJump) : sizeof(HLInst);
}

static HLInst* X86Compiler_newInst(X86Compiler* self, void* p, uint32_t code, uint32_t options, Operand* opList, uint32_t opCount) {
  if (Utils::inInterval<uint32_t>(code, _kX86InstIdJbegin, _kX86InstIdJend)) {
    HLJump* node = new(p) HLJump(self, code, options, opList, opCount);
    HLLabel* jTarget = NULL;

    if ((options & kInstOptionUnfollow) == 0) {
      if (opList[0].isLabel())
        jTarget = self->getHLLabel(static_cast<Label&>(opList[0]));
      else
        options |= kInstOptionUnfollow;
    }

    node->orFlags(code == kX86InstIdJmp ? kHLNodeFlagIsJmp | kHLNodeFlagIsTaken : kHLNodeFlagIsJcc);
    node->_target = jTarget;
    node->_jumpNext = NULL;

    if (jTarget) {
      node->_jumpNext = static_cast<HLJump*>(jTarget->_from);
      jTarget->_from = node;
      jTarget->addNumRefs();
    }

    // The 'jmp' is always taken, conditional jump can contain hint, we detect it.
    if (code == kX86InstIdJmp)
      node->orFlags(kHLNodeFlagIsTaken);
    else if (options & kInstOptionTaken)
      node->orFlags(kHLNodeFlagIsTaken);

    node->addOptions(options);
    return node;
  }
  else {
    HLInst* node = new(p) HLInst(self, code, options, opList, opCount);
    node->addOptions(options);
    return node;
  }
}

HLInst* X86Compiler::newInst(uint32_t code) {
  size_t size = X86Compiler_getInstSize(code);
  HLInst* inst = static_cast<HLInst*>(_zoneAllocator.alloc(size));

  if (inst == NULL)
    goto _NoMemory;

  return X86Compiler_newInst(this, inst, code, getInstOptionsAndReset(), NULL, 0);

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLInst* X86Compiler::newInst(uint32_t code, const Operand& o0) {
  size_t size = X86Compiler_getInstSize(code);
  HLInst* inst = static_cast<HLInst*>(_zoneAllocator.alloc(size + 1 * sizeof(Operand)));

  if (inst == NULL)
    goto _NoMemory;

  {
    Operand* opList = reinterpret_cast<Operand*>(reinterpret_cast<uint8_t*>(inst) + size);
    opList[0] = o0;
    ASMJIT_ASSERT_OPERAND(o0);
    return X86Compiler_newInst(this, inst, code, getInstOptionsAndReset(), opList, 1);
  }

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLInst* X86Compiler::newInst(uint32_t code, const Operand& o0, const Operand& o1) {
  size_t size = X86Compiler_getInstSize(code);
  HLInst* inst = static_cast<HLInst*>(_zoneAllocator.alloc(size + 2 * sizeof(Operand)));

  if (inst == NULL)
    goto _NoMemory;

  {
    Operand* opList = reinterpret_cast<Operand*>(reinterpret_cast<uint8_t*>(inst) + size);
    opList[0] = o0;
    opList[1] = o1;
    ASMJIT_ASSERT_OPERAND(o0);
    ASMJIT_ASSERT_OPERAND(o1);
    return X86Compiler_newInst(this, inst, code, getInstOptionsAndReset(), opList, 2);
  }

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLInst* X86Compiler::newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2) {
  size_t size = X86Compiler_getInstSize(code);
  HLInst* inst = static_cast<HLInst*>(_zoneAllocator.alloc(size + 3 * sizeof(Operand)));

  if (inst == NULL)
    goto _NoMemory;

  {
    Operand* opList = reinterpret_cast<Operand*>(reinterpret_cast<uint8_t*>(inst) + size);
    opList[0] = o0;
    opList[1] = o1;
    opList[2] = o2;
    ASMJIT_ASSERT_OPERAND(o0);
    ASMJIT_ASSERT_OPERAND(o1);
    ASMJIT_ASSERT_OPERAND(o2);
    return X86Compiler_newInst(this, inst, code, getInstOptionsAndReset(), opList, 3);
  }

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLInst* X86Compiler::newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3) {
  size_t size = X86Compiler_getInstSize(code);
  HLInst* inst = static_cast<HLInst*>(_zoneAllocator.alloc(size + 4 * sizeof(Operand)));

  if (inst == NULL)
    goto _NoMemory;

  {
    Operand* opList = reinterpret_cast<Operand*>(reinterpret_cast<uint8_t*>(inst) + size);
    opList[0] = o0;
    opList[1] = o1;
    opList[2] = o2;
    opList[3] = o3;
    ASMJIT_ASSERT_OPERAND(o0);
    ASMJIT_ASSERT_OPERAND(o1);
    ASMJIT_ASSERT_OPERAND(o2);
    ASMJIT_ASSERT_OPERAND(o3);
    return X86Compiler_newInst(this, inst, code, getInstOptionsAndReset(), opList, 4);
  }

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLInst* X86Compiler::newInst(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3, const Operand& o4) {
  size_t size = X86Compiler_getInstSize(code);
  HLInst* inst = static_cast<HLInst*>(_zoneAllocator.alloc(size + 5 * sizeof(Operand)));

  if (inst == NULL)
    goto _NoMemory;

  {
    Operand* opList = reinterpret_cast<Operand*>(reinterpret_cast<uint8_t*>(inst) + size);
    opList[0] = o0;
    opList[1] = o1;
    opList[2] = o2;
    opList[3] = o3;
    opList[4] = o4;
    ASMJIT_ASSERT_OPERAND(o0);
    ASMJIT_ASSERT_OPERAND(o1);
    ASMJIT_ASSERT_OPERAND(o2);
    ASMJIT_ASSERT_OPERAND(o3);
    ASMJIT_ASSERT_OPERAND(o4);
    return X86Compiler_newInst(this, inst, code, getInstOptionsAndReset(), opList, 5);
  }

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLInst* X86Compiler::emit(uint32_t code) {
  HLInst* node = newInst(code);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0) {
  HLInst* node = newInst(code, o0);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1){
  HLInst* node = newInst(code, o0, o1);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2) {
  HLInst* node = newInst(code, o0, o1, o2);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3){
  HLInst* node = newInst(code, o0, o1, o2, o3);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, const Operand& o3, const Operand& o4) {
  HLInst* node = newInst(code, o0, o1, o2, o3, o4);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, int o0_) {
  Imm o0(o0_);
  HLInst* node = newInst(code, o0);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, uint64_t o0_) {
  Imm o0(o0_);
  HLInst* node = newInst(code, o0);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, int o1_) {
  Imm o1(o1_);
  HLInst* node = newInst(code, o0, o1);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, uint64_t o1_) {
  Imm o1(o1_);
  HLInst* node = newInst(code, o0, o1);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, int o2_) {
  Imm o2(o2_);
  HLInst* node = newInst(code, o0, o1, o2);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, uint64_t o2_) {
  Imm o2(o2_);
  HLInst* node = newInst(code, o0, o1, o2);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, int o3_) {
  Imm o3(o3_);
  HLInst* node = newInst(code, o0, o1, o2, o3);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

HLInst* X86Compiler::emit(uint32_t code, const Operand& o0, const Operand& o1, const Operand& o2, uint64_t o3_) {
  Imm o3(o3_);
  HLInst* node = newInst(code, o0, o1, o2, o3);
  if (node == NULL)
    return NULL;
  return static_cast<HLInst*>(addNode(node));
}

// ============================================================================
// [asmjit::X86Compiler - Func]
// ============================================================================

X86FuncNode* X86Compiler::newFunc(const FuncPrototype& p) {
  X86FuncNode* func = newNode<X86FuncNode>();
  Error error;

  if (func == NULL)
    goto _NoMemory;

  // Create helper nodes.
  func->_entryNode = newLabelNode();
  func->_exitNode = newLabelNode();
  func->_end = newNode<HLSentinel>();

  if (func->_entryNode == NULL || func->_exitNode == NULL || func->_end == NULL)
    goto _NoMemory;

  // Function prototype.
  if ((error = func->_x86Decl.setPrototype(p)) != kErrorOk) {
    setLastError(error);
    return NULL;
  }

  // Function arguments stack size. Since function requires _argStackSize to be
  // set, we have to copy it from X86FuncDecl.
  func->_argStackSize = func->_x86Decl.getArgStackSize();
  func->_redZoneSize = static_cast<uint16_t>(func->_x86Decl.getRedZoneSize());
  func->_spillZoneSize = static_cast<uint16_t>(func->_x86Decl.getSpillZoneSize());

  // Expected/Required stack alignment.
  func->_expectedStackAlignment = getRuntime()->getStackAlignment();
  func->_requiredStackAlignment = 0;

  // Allocate space for function arguments.
  func->_args = NULL;
  if (func->getNumArgs() != 0) {
    func->_args = _zoneAllocator.allocT<VarData*>(func->getNumArgs() * sizeof(VarData*));
    if (func->_args == NULL)
      goto _NoMemory;
    ::memset(func->_args, 0, func->getNumArgs() * sizeof(VarData*));
  }

  return func;

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

X86FuncNode* X86Compiler::addFunc(const FuncPrototype& p) {
  X86FuncNode* func = newFunc(p);

  if (func == NULL) {
    setLastError(kErrorNoHeapMemory);
    return NULL;
  }

  ASMJIT_ASSERT(_func == NULL);
  _func = func;

  addNode(func);                 // Add function node.
  addNode(func->getEntryNode()); // Add function entry.
  HLNode* cursor = getCursor();

  addNode(func->getExitNode());  // Add function exit / epilog marker.
  addNode(func->getEnd());       // Add function end.
  setCursor(cursor);

  return func;
}

HLSentinel* X86Compiler::endFunc() {
  X86FuncNode* func = getFunc();
  ASMJIT_ASSERT(func != NULL);

  // Add local constant pool at the end of the function (if exist).
  setCursor(func->getExitNode());

  if (_localConstPoolLabel.isInitialized()) {
    embedConstPool(_localConstPoolLabel, _localConstPool);
    _localConstPoolLabel.reset();
    _localConstPool.reset();
  }

  // Finalize.
  func->addFuncFlags(kFuncFlagIsFinished);
  _func = NULL;

  setCursor(func->getEnd());
  return func->getEnd();
}

// ============================================================================
// [asmjit::X86Compiler - Ret]
// ============================================================================

HLRet* X86Compiler::newRet(const Operand& o0, const Operand& o1) {
  HLRet* node = newNode<HLRet>(o0, o1);
  if (node == NULL)
    goto _NoMemory;
  return node;

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

HLRet* X86Compiler::addRet(const Operand& o0, const Operand& o1) {
  HLRet* node = newRet(o0, o1);
  if (node == NULL)
    return node;
  return static_cast<HLRet*>(addNode(node));
}

// ============================================================================
// [asmjit::X86Compiler - Call]
// ============================================================================

X86CallNode* X86Compiler::newCall(const Operand& o0, const FuncPrototype& p) {
  X86CallNode* node = newNode<X86CallNode>(o0);
  Error error;
  uint32_t nArgs;

  if (node == NULL)
    goto _NoMemory;

  if ((error = node->_x86Decl.setPrototype(p)) != kErrorOk) {
    setLastError(error);
    return NULL;
  }

  // If there are no arguments skip the allocation.
  if ((nArgs = p.getNumArgs()) == 0)
    return node;

  node->_args = static_cast<Operand*>(_zoneAllocator.alloc(nArgs * sizeof(Operand)));
  if (node->_args == NULL)
    goto _NoMemory;

  ::memset(node->_args, 0, nArgs * sizeof(Operand));
  return node;

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

X86CallNode* X86Compiler::addCall(const Operand& o0, const FuncPrototype& p) {
  X86CallNode* node = newCall(o0, p);
  if (node == NULL)
    return NULL;
  return static_cast<X86CallNode*>(addNode(node));
}

// ============================================================================
// [asmjit::X86Compiler - Vars]
// ============================================================================

Error X86Compiler::setArg(uint32_t argIndex, const Var& var) {
  X86FuncNode* func = getFunc();

  if (func == NULL)
    return kErrorInvalidArgument;

  if (!isVarValid(var))
    return kErrorInvalidState;

  VarData* vd = getVd(var);
  func->setArg(argIndex, vd);

  return kErrorOk;
}

Error X86Compiler::_newVar(Var* var, uint32_t vType, const char* name, va_list ap) {
  ASMJIT_ASSERT(vType < kX86VarTypeCount);
  vType = _targetVarMapping[vType];
  ASMJIT_ASSERT(vType != kInvalidVar);

  // The assertion won't be compiled in release build, however, we want to check
  // this anyway.
  if (vType == kInvalidVar) {
    static_cast<X86Var*>(var)->reset();
    return kErrorInvalidArgument;
  }

  const X86VarInfo& vInfo = _x86VarInfo[vType];
  char buf[64];

  // Format the name if `ap` is given.
  if (ap) {
    vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), name, ap);
    buf[ASMJIT_ARRAY_SIZE(buf) - 1] = '\0';
    name = buf;
  }

  VarData* vd = _newVd(vType, vInfo.getSize(), vInfo.getClass(), name);
  if (vd == NULL) {
    static_cast<X86Var*>(var)->reset();
    return getLastError();
  }

  var->_init_packed_op_sz_w0_id(kOperandTypeVar, vInfo.getSize(), vInfo.getReg() << 8, vd->getId());
  var->_vreg.vType = vType;
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Compiler - Stack]
// ============================================================================

Error X86Compiler::_newStack(BaseMem* mem, uint32_t size, uint32_t alignment, const char* name) {
  if (size == 0)
    return kErrorInvalidArgument;

  if (alignment > 64)
    alignment = 64;

  VarData* vd = _newVd(kInvalidVar, size, kInvalidReg, name);
  if (vd == NULL) {
    static_cast<X86Mem*>(mem)->reset();
    return getLastError();
  }

  vd->_isStack = true;
  vd->_alignment = static_cast<uint8_t>(alignment);

  static_cast<X86Mem*>(mem)->_init(kMemTypeStackIndex, vd->getId(), 0, 0);
  return kErrorOk;
}

// ============================================================================
// [asmjit::X86Compiler - Const]
// ============================================================================

Error X86Compiler::_newConst(BaseMem* mem, uint32_t scope, const void* data, size_t size) {
  Error error = kErrorOk;
  size_t offset;

  Label* dstLabel;
  ConstPool* dstPool;

  if (scope == kConstScopeLocal) {
    dstLabel = &_localConstPoolLabel;
    dstPool = &_localConstPool;
  }
  else if (scope == kConstScopeGlobal) {
    dstLabel = &_globalConstPoolLabel;
    dstPool = &_globalConstPool;
  }
  else {
    error = kErrorInvalidArgument;
    goto _OnError;
  }

  error = dstPool->add(data, size, offset);
  if (error != kErrorOk)
    goto _OnError;

  if (dstLabel->getId() == kInvalidValue) {
    *dstLabel = newLabel();
    if (!dstLabel->isInitialized()) {
      error = kErrorNoHeapMemory;
      goto _OnError;
    }
  }

  *static_cast<X86Mem*>(mem) = x86::ptr(*dstLabel, static_cast<int32_t>(offset), static_cast<uint32_t>(size));
  return kErrorOk;

_OnError:
  return error;
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER && (ASMJIT_BUILD_X86 || ASMJIT_BUILD_X64)
