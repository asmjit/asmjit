// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/assembler.h"
#include "../core/codecompiler.h"
#include "../core/cpuinfo.h"
#include "../core/intutils.h"
#include "../core/logging.h"
#include "../core/rapass_p.h"
#include "../core/type.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::CCFuncCall - Arg / Ret]
// ============================================================================

bool CCFuncCall::_setArg(uint32_t i, const Operand_& op) noexcept {
  if ((i & ~kFuncArgHi) >= _funcDetail.getArgCount())
    return false;

  _args[i] = op;
  return true;
}

bool CCFuncCall::_setRet(uint32_t i, const Operand_& op) noexcept {
  if (i >= 2)
    return false;

  _ret[i] = op;
  return true;
}

// ============================================================================
// [asmjit::CodeCompiler - Construction / Destruction]
// ============================================================================

CodeCompiler::CodeCompiler() noexcept
  : CodeBuilder(),
    _func(nullptr),
    _vRegZone(4096 - Zone::kZoneOverhead),
    _vRegArray(),
    _localConstPool(nullptr),
    _globalConstPool(nullptr) {

  _type = kTypeCompiler;
}
CodeCompiler::~CodeCompiler() noexcept {}

// ============================================================================
// [asmjit::CodeCompiler - Func]
// ============================================================================

CCFunc* CodeCompiler::newFunc(const FuncSignature& sign) noexcept {
  Error err;

  CCFunc* func = newNodeT<CCFunc>();
  if (ASMJIT_UNLIKELY(!func)) {
    reportError(DebugUtils::errored(kErrorNoHeapMemory));
    return nullptr;
  }

  err = registerLabelNode(func);
  if (ASMJIT_UNLIKELY(err)) {
    // TODO: Calls reportError, maybe rethink noexcept?
    reportError(err);
    return nullptr;
  }

  // Create helper nodes.
  func->_exitNode = newLabelNode();
  func->_end = newNodeT<CBSentinel>(CBSentinel::kSentinelFuncEnd);

  if (ASMJIT_UNLIKELY(!func->_exitNode || !func->_end)) {
    reportError(DebugUtils::errored(kErrorNoHeapMemory));
    return nullptr;
  }

  // Initialize the function info.
  err = func->getDetail().init(sign);
  if (ASMJIT_UNLIKELY(err)) {
    reportError(err);
    return nullptr;
  }

  // If the Target guarantees greater stack alignment than required by the
  // calling convention then override it as we can prevent having to perform
  // dynamic stack alignment
  if (func->_funcDetail._callConv.getNaturalStackAlignment() < _codeInfo.getStackAlignment())
    func->_funcDetail._callConv.setNaturalStackAlignment(_codeInfo.getStackAlignment());

  // Initialize the function frame.
  err = func->_frame.init(func->_funcDetail);
  if (ASMJIT_UNLIKELY(err)) {
    reportError(err);
    return nullptr;
  }

  // Allocate space for function arguments.
  func->_args = nullptr;
  if (func->getArgCount() != 0) {
    func->_args = _allocator.allocT<VirtReg*>(func->getArgCount() * sizeof(VirtReg*));
    if (ASMJIT_UNLIKELY(!func->_args)) {
      reportError(DebugUtils::errored(kErrorNoHeapMemory));
      return nullptr;
    }

    std::memset(func->_args, 0, func->getArgCount() * sizeof(VirtReg*));
  }

  return func;
}

CCFunc* CodeCompiler::addFunc(CCFunc* func) {
  ASMJIT_ASSERT(_func == nullptr);
  _func = func;

  addNode(func);                 // Function node.
  CBNode* cursor = getCursor();  // {CURSOR}.
  addNode(func->getExitNode());  // Function exit label.
  addNode(func->getEnd());       // Function end marker.

  _setCursor(cursor);
  return func;
}

CCFunc* CodeCompiler::addFunc(const FuncSignature& sign) {
  CCFunc* func = newFunc(sign);

  if (!func) {
    reportError(DebugUtils::errored(kErrorNoHeapMemory));
    return nullptr;
  }

  return addFunc(func);
}

Error CodeCompiler::endFunc() {
  CCFunc* func = getFunc();
  if (ASMJIT_UNLIKELY(!func))
    return reportError(DebugUtils::errored(kErrorInvalidState));

  // Add the local constant pool at the end of the function (if exists).
  if (_localConstPool) {
    setCursor(func->getEnd()->getPrev());
    addNode(_localConstPool);
    _localConstPool = nullptr;
  }

  // Mark as finished.
  _func = nullptr;

  CBSentinel* end = func->getEnd();
  setCursor(end);
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeCompiler - Ret]
// ============================================================================

CCFuncRet* CodeCompiler::newRet(const Operand_& o0, const Operand_& o1) noexcept {
  CCFuncRet* node = newNodeT<CCFuncRet>();
  if (!node) {
    reportError(DebugUtils::errored(kErrorNoHeapMemory));
    return nullptr;
  }

  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOpCount(!o1.isNone() ? 2U : !o0.isNone() ? 1U : 0U);

  return node;
}

CCFuncRet* CodeCompiler::addRet(const Operand_& o0, const Operand_& o1) noexcept {
  CCFuncRet* node = newRet(o0, o1);
  if (!node) return nullptr;
  return addNode(node)->as<CCFuncRet>();
}

// ============================================================================
// [asmjit::CodeCompiler - Call]
// ============================================================================

CCFuncCall* CodeCompiler::newCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept {
  CCFuncCall* node = newNodeT<CCFuncCall>(instId, 0);
  if (ASMJIT_UNLIKELY(!node)) {
    reportError(DebugUtils::errored(kErrorNoHeapMemory));
    return nullptr;
  }

  node->setOpCount(1);
  node->setOp(0, o0);
  node->resetOp(1);
  node->resetOp(2);
  node->resetOp(3);

  Error err = node->getDetail().init(sign);
  if (ASMJIT_UNLIKELY(err)) {
    reportError(err);
    return nullptr;
  }

  // If there are no arguments skip the allocation.
  uint32_t nArgs = sign.getArgCount();
  if (!nArgs) return node;

  node->_args = static_cast<Operand*>(_allocator.alloc(nArgs * sizeof(Operand)));
  if (!node->_args) {
    reportError(DebugUtils::errored(kErrorNoHeapMemory));
    return nullptr;
  }

  std::memset(node->_args, 0, nArgs * sizeof(Operand));
  return node;
}

CCFuncCall* CodeCompiler::addCall(uint32_t instId, const Operand_& o0, const FuncSignature& sign) noexcept {
  CCFuncCall* node = newCall(instId, o0, sign);
  if (!node) return nullptr;
  return addNode(node)->as<CCFuncCall>();
}

// ============================================================================
// [asmjit::CodeCompiler - Vars]
// ============================================================================

Error CodeCompiler::setArg(uint32_t argIndex, const Reg& r) {
  CCFunc* func = getFunc();

  if (ASMJIT_UNLIKELY(!func))
    return reportError(DebugUtils::errored(kErrorInvalidState));

  if (ASMJIT_UNLIKELY(!isVirtRegValid(r)))
    return reportError(DebugUtils::errored(kErrorInvalidVirtId));

  VirtReg* vReg = getVirtReg(r);
  func->setArg(argIndex, vReg);

  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeCompiler - Vars]
// ============================================================================

static void CodeCompiler_assignGenericName(CodeCompiler* self, VirtReg* vReg) {
  uint32_t index = unsigned(Operand::unpackId(vReg->_id));

  char buf[64];
  int len = std::snprintf(buf, ASMJIT_ARRAY_SIZE(buf), "%%%u", unsigned(index));

  ASMJIT_ASSERT(len > 0 && len < int(ASMJIT_ARRAY_SIZE(buf)));
  vReg->_name.setData(&self->_dataZone, buf, unsigned(len));
}

VirtReg* CodeCompiler::newVirtReg(uint32_t typeId, uint32_t signature, const char* name) noexcept {
  uint32_t index = _vRegArray.getLength();
  if (ASMJIT_UNLIKELY(index >= uint32_t(Operand::kPackedIdCount)))
    return nullptr;

  if (_vRegArray.willGrow(&_allocator) != kErrorOk)
    return nullptr;

  VirtReg* vReg = _vRegZone.allocZeroedT<VirtReg>();
  if (ASMJIT_UNLIKELY(!vReg)) return nullptr;

  uint32_t size = Type::sizeOf(typeId);
  uint32_t alignment = std::min<uint32_t>(size, 64);

  vReg = new(vReg) VirtReg(Operand::packId(index), signature, size, alignment, typeId);

  #ifndef ASMJIT_DISABLE_LOGGING
  if (name && name[0] != '\0')
    vReg->_name.setData(&_dataZone, name, Globals::kNullTerminated);
  else
    CodeCompiler_assignGenericName(this, vReg);
  #endif

  _vRegArray.appendUnsafe(vReg);
  return vReg;
}

Error CodeCompiler::_newReg(Reg& out, uint32_t typeId, const char* name) {
  RegInfo regInfo;

  Error err = ArchUtils::typeIdToRegInfo(getArchType(), typeId, regInfo);
  if (ASMJIT_UNLIKELY(err)) return reportError(err);

  VirtReg* vReg = newVirtReg(typeId, regInfo.getSignature(), name);
  if (ASMJIT_UNLIKELY(!vReg)) {
    out.reset();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  out._initReg(regInfo.getSignature(), vReg->getId());
  return kErrorOk;
}

Error CodeCompiler::_newReg(Reg& out, uint32_t typeId, const char* fmt, std::va_list ap) {
  StringBuilderTmp<256> sb;
  sb.appendFormatVA(fmt, ap);
  return _newReg(out, typeId, sb.getData());
}

Error CodeCompiler::_newReg(Reg& out, const Reg& ref, const char* name) {
  RegInfo regInfo;
  uint32_t typeId;

  if (isVirtRegValid(ref)) {
    VirtReg* vRef = getVirtReg(ref);
    typeId = vRef->getTypeId();

    // NOTE: It's possible to cast one register type to another if it's the
    // same register group. However, VirtReg always contains the TypeId that
    // was used to create the register. This means that in some cases we may
    // end up having different size of `ref` and `vRef`. In such case we
    // adjust the TypeId to match the `ref` register type instead of the
    // original register type, which should be the expected behavior.
    uint32_t typeSize = Type::sizeOf(typeId);
    uint32_t refSize = ref.getSize();

    if (typeSize != refSize) {
      if (Type::isInt(typeId)) {
        // GP register - change TypeId to match `ref`, but keep sign of `vRef`.
        switch (refSize) {
          case  1: typeId = Type::kIdI8  | (typeId & 1); break;
          case  2: typeId = Type::kIdI16 | (typeId & 1); break;
          case  4: typeId = Type::kIdI32 | (typeId & 1); break;
          case  8: typeId = Type::kIdI64 | (typeId & 1); break;
          default: typeId = Type::kIdVoid; break;
        }
      }
      else if (Type::isMmx(typeId)) {
        // MMX register - always use 64-bit.
        typeId = Type::kIdMmx64;
      }
      else if (Type::isMask(typeId)) {
        // Mask register - change TypeId to match `ref` size.
        switch (refSize) {
          case  1: typeId = Type::kIdMask8; break;
          case  2: typeId = Type::kIdMask16; break;
          case  4: typeId = Type::kIdMask32; break;
          case  8: typeId = Type::kIdMask64; break;
          default: typeId = Type::kIdVoid; break;
        }
      }
      else {
        // VEC register - change TypeId to match `ref` size, keep vector metadata.
        uint32_t elementTypeId = Type::baseOf(typeId);

        switch (refSize) {
          case 16: typeId = Type::_kIdVec128Start + (elementTypeId - Type::kIdI8); break;
          case 32: typeId = Type::_kIdVec256Start + (elementTypeId - Type::kIdI8); break;
          case 64: typeId = Type::_kIdVec512Start + (elementTypeId - Type::kIdI8); break;
          default: typeId = Type::kIdVoid; break;
        }
      }

      if (typeId == Type::kIdVoid)
        return reportError(DebugUtils::errored(kErrorInvalidState));
    }
  }
  else {
    typeId = ref.getType();
  }

  Error err = ArchUtils::typeIdToRegInfo(getArchType(), typeId, regInfo);
  if (ASMJIT_UNLIKELY(err)) return reportError(err);

  VirtReg* vReg = newVirtReg(typeId, regInfo.getSignature(), name);
  if (ASMJIT_UNLIKELY(!vReg)) {
    out.reset();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  out._initReg(regInfo.getSignature(), vReg->getId());
  return kErrorOk;
}

Error CodeCompiler::_newReg(Reg& out, const Reg& ref, const char* fmt, std::va_list ap) {
  StringBuilderTmp<256> sb;
  sb.appendFormatVA(fmt, ap);
  return _newReg(out, ref, sb.getData());
}

Error CodeCompiler::_newStack(Mem& out, uint32_t size, uint32_t alignment, const char* name) {
  if (size == 0)
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment == 0) alignment = 1;
  if (!IntUtils::isPowerOf2(alignment))
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  if (alignment > 64) alignment = 64;

  VirtReg* vReg = newVirtReg(0, 0, name);
  if (ASMJIT_UNLIKELY(!vReg)) {
    out.reset();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  vReg->_virtSize = size;
  vReg->_isStack = true;
  vReg->_alignment = uint8_t(alignment);

  // Set the memory operand to GPD/GPQ and its id to VirtReg.
  out = Mem(Globals::Init, _gpRegInfo.getType(), vReg->getId(), Reg::kRegNone, 0, 0, 0, Mem::kSignatureMemRegHomeFlag);
  return kErrorOk;
}

Error CodeCompiler::_newConst(Mem& out, uint32_t scope, const void* data, size_t size) {
  CBConstPool** pPool;
  if (scope == kConstScopeLocal)
    pPool = &_localConstPool;
  else if (scope == kConstScopeGlobal)
    pPool = &_globalConstPool;
  else
    return reportError(DebugUtils::errored(kErrorInvalidArgument));

  CBConstPool* pool = *pPool;
  if (!pool) {
    pool = newConstPoolNode();
    if (ASMJIT_UNLIKELY(!pool))
      return reportError(DebugUtils::errored(kErrorNoHeapMemory));
    *pPool = pool;
  }

  size_t off;
  Error err = pool->add(data, size, off);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  out = Mem(Globals::Init,
    Label::kLabelTag,      // Base type.
    pool->getId(),         // Base id.
    0,                     // Index type.
    0,                     // Index id.
    int32_t(off),          // Offset.
    uint32_t(size),        // Size.
    0);                    // Flags.
  return kErrorOk;
}

void CodeCompiler::rename(Reg& reg, const char* fmt, ...) {
  if (!reg.isVirtReg()) return;

  VirtReg* vReg = getVirtRegById(reg.getId());
  if (!vReg) return;

  if (fmt && fmt[0] != '\0') {
    char buf[128];
    std::va_list ap;

    va_start(ap, fmt);
    std::vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
    va_end(ap);

    vReg->_name.setData(&_dataZone, buf, Globals::kNullTerminated);
  }
  else {
    CodeCompiler_assignGenericName(this, vReg);
  }
}

// ============================================================================
// [asmjit::CodeCompiler - Events]
// ============================================================================

Error CodeCompiler::onAttach(CodeHolder* code) noexcept {
  return Base::onAttach(code);
}

Error CodeCompiler::onDetach(CodeHolder* code) noexcept {
  _func = nullptr;
  _localConstPool = nullptr;
  _globalConstPool = nullptr;

  _vRegArray.reset();
  _vRegZone.reset(false);

  return Base::onDetach(code);
}

// ============================================================================
// [asmjit::CCFuncPass - Construction / Destruction]
// ============================================================================

CCFuncPass::CCFuncPass(const char* name) noexcept
  : CBPass(name) {}

// ============================================================================
// [asmjit::CCFuncPass - Run]
// ============================================================================

Error CCFuncPass::run(Zone* zone, Logger* logger) noexcept {
  CBNode* node = cb()->getFirstNode();
  if (!node) return kErrorOk;

  do {
    if (node->getType() == CBNode::kNodeFunc) {
      CCFunc* func = node->as<CCFunc>();
      node = func->getEnd();
      ASMJIT_PROPAGATE(runOnFunction(zone, logger, func));
    }

    // Find a function by skipping all nodes that are not `kNodeFunc`.
    do {
      node = node->getNext();
    } while (node && node->getType() != CBNode::kNodeFunc);
  } while (node);

  return kErrorOk;
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
