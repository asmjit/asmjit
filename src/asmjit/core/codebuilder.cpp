// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/codebuilder.h"
#include "../core/logging.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::PostponedErrorHandler (Internal)]
// ============================================================================

//! \internal
//!
//! Postponed error handler that never throws. Used as a temporal error handler
//! to run passes. If error occurs, the caller is notified and will call the
//! real error handler, that can throw.
class PostponedErrorHandler : public ErrorHandler {
public:
  void handleError(Error err, const char* message, CodeEmitter* origin) override {
    ASMJIT_UNUSED(err);
    ASMJIT_UNUSED(origin);

    _message.setString(message);
  }

  StringBuilderTmp<128> _message;
};

// ============================================================================
// [asmjit::CodeBuilder - Construction / Destruction]
// ============================================================================

CodeBuilder::CodeBuilder() noexcept
  : CodeEmitter(kTypeBuilder),
    _codeZone(32768 - Zone::kZoneOverhead),
    _dataZone(16384 - Zone::kZoneOverhead),
    _passZone(65536 - Zone::kZoneOverhead),
    _allocator(&_codeZone),
    _passes(),
    _labelNodes(),
    _firstNode(nullptr),
    _lastNode(nullptr),
    _cursor(nullptr),
    _nodeFlags(0) {}
CodeBuilder::~CodeBuilder() noexcept {}

// ============================================================================
// [asmjit::CodeBuilder - Node Management]
// ============================================================================

CBLabel* CodeBuilder::newLabelNode() noexcept {
  CBLabel* node = newNodeT<CBLabel>();
  if (!node || registerLabelNode(node) != kErrorOk)
    return nullptr;
  return node;
}

CBAlign* CodeBuilder::newAlignNode(uint32_t mode, uint32_t alignment) noexcept {
  return newNodeT<CBAlign>(mode, alignment);
}

CBData* CodeBuilder::newDataNode(const void* data, uint32_t size) noexcept {
  if (size > CBData::kInlineBufferSize) {
    void* cloned = _dataZone.alloc(size);
    if (ASMJIT_UNLIKELY(!cloned))
      return nullptr;

    if (data)
      std::memcpy(cloned, data, size);
    data = cloned;
  }

  return newNodeT<CBData>(const_cast<void*>(data), size);
}

CBConstPool* CodeBuilder::newConstPoolNode() noexcept {
  CBConstPool* node = newNodeT<CBConstPool>();
  if (!node || registerLabelNode(node) != kErrorOk)
    return nullptr;
  return node;
}

CBComment* CodeBuilder::newCommentNode(const char* s, size_t len) noexcept {
  if (s) {
    if (len == Globals::kNullTerminated)
      len = std::strlen(s);

    if (len > 0) {
      s = static_cast<char*>(_dataZone.dup(s, len, true));
      if (!s) return nullptr;
    }
  }

  return newNodeT<CBComment>(s);
}

CBInst* CodeBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0) noexcept {
  uint32_t opCount = 1;
  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) CBInst(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

CBInst* CodeBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1) noexcept {
  uint32_t opCount = 2;
  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) CBInst(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

CBInst* CodeBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  uint32_t opCount = 3;
  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) CBInst(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

CBInst* CodeBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  uint32_t opCount = 4;
  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) CBInst(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

CBInst* CodeBuilder::newInstNodeRaw(uint32_t instId, uint32_t instOptions, uint32_t opCount) noexcept {
  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;
  return new(node) CBInst(this, instId, instOptions, opCount, opCapacity);
}

CBNode* CodeBuilder::addNode(CBNode* node) noexcept {
  ASMJIT_ASSERT(node);
  ASMJIT_ASSERT(node->getPrev() == nullptr);
  ASMJIT_ASSERT(node->getNext() == nullptr);

  if (!_cursor) {
    if (!_firstNode) {
      _firstNode = node;
      _lastNode = node;
    }
    else {
      node->_setNext(_firstNode);
      _firstNode->_setPrev(node);
      _firstNode = node;
    }
  }
  else {
    CBNode* prev = _cursor;
    CBNode* next = _cursor->getNext();

    node->_setPrev(prev);
    node->_setNext(next);

    prev->_setNext(node);
    if (next)
      next->_setPrev(node);
    else
      _lastNode = node;
  }

  _cursor = node;
  return node;
}

CBNode* CodeBuilder::addAfter(CBNode* node, CBNode* ref) noexcept {
  ASMJIT_ASSERT(node);
  ASMJIT_ASSERT(ref);

  ASMJIT_ASSERT(node->getPrev() == nullptr);
  ASMJIT_ASSERT(node->getNext() == nullptr);

  CBNode* prev = ref;
  CBNode* next = ref->getNext();

  node->_setPrev(prev);
  node->_setNext(next);

  prev->_setNext(node);
  if (next)
    next->_setPrev(node);
  else
    _lastNode = node;

  return node;
}

CBNode* CodeBuilder::addBefore(CBNode* node, CBNode* ref) noexcept {
  ASMJIT_ASSERT(node != nullptr);
  ASMJIT_ASSERT(node->getPrev() == nullptr);
  ASMJIT_ASSERT(node->getNext() == nullptr);
  ASMJIT_ASSERT(ref != nullptr);

  CBNode* prev = ref->getPrev();
  CBNode* next = ref;

  node->_setPrev(prev);
  node->_setNext(next);

  next->_setPrev(node);
  if (prev)
    prev->_setNext(node);
  else
    _firstNode = node;

  return node;
}

CBNode* CodeBuilder::removeNode(CBNode* node) noexcept {
  CBNode* prev = node->getPrev();
  CBNode* next = node->getNext();

  if (_firstNode == node)
    _firstNode = next;
  else
    prev->_setNext(next);

  if (_lastNode == node)
    _lastNode  = prev;
  else
    next->_setPrev(prev);

  node->_setPrev(nullptr);
  node->_setNext(nullptr);

  if (_cursor == node)
    _cursor = prev;

  return node;
}

void CodeBuilder::removeNodes(CBNode* first, CBNode* last) noexcept {
  if (first == last) {
    removeNode(first);
    return;
  }

  CBNode* prev = first->getPrev();
  CBNode* next = last->getNext();

  if (_firstNode == first)
    _firstNode = next;
  else
    prev->_setNext(next);

  if (_lastNode == last)
    _lastNode  = prev;
  else
    next->_setPrev(prev);

  CBNode* node = first;
  for (;;) {
    next = node->getNext();
    ASMJIT_ASSERT(next != nullptr);

    node->_setPrev(nullptr);
    node->_setNext(nullptr);

    if (_cursor == node)
      _cursor = prev;

    if (node == last)
      break;
    node = next;
  }
}

CBNode* CodeBuilder::setCursor(CBNode* node) noexcept {
  CBNode* old = _cursor;
  _cursor = node;
  return old;
}

// ============================================================================
// [asmjit::CodeBuilder - Label Management]
// ============================================================================

Error CodeBuilder::getLabelNode(CBLabel** pOut, uint32_t id) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  uint32_t index = Operand::unpackId(id);

  if (ASMJIT_UNLIKELY(index >= _code->getLabelCount()))
    return DebugUtils::errored(kErrorInvalidLabel);

  if (index >= _labelNodes.getLength())
    ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, index + 1));

  CBLabel* node = _labelNodes[index];
  if (!node) {
    node = newNodeT<CBLabel>(id);
    if (ASMJIT_UNLIKELY(!node))
      return DebugUtils::errored(kErrorNoHeapMemory);
    _labelNodes[index] = node;
  }

  *pOut = node;
  return kErrorOk;
}

Error CodeBuilder::registerLabelNode(CBLabel* node) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  // Don't call `reportError()` from here, we are noexcept and we are called
  // by `newLabelNode()` and `newFuncNode()`, which are noexcept as well.
  uint32_t id;
  ASMJIT_PROPAGATE(_code->newLabelId(id));
  uint32_t index = Operand::unpackId(id);

  // We just added one label so it must be true.
  ASMJIT_ASSERT(_labelNodes.getLength() < index + 1);
  ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, index + 1));

  _labelNodes[index] = node;
  node->_id = id;
  return kErrorOk;
}

Label CodeBuilder::newLabel() {
  uint32_t id = 0;
  if (_code) {
    CBLabel* node = newNodeT<CBLabel>(id);
    if (ASMJIT_UNLIKELY(!node)) {
      reportError(DebugUtils::errored(kErrorNoHeapMemory));
    }
    else {
      Error err = registerLabelNode(node);
      if (ASMJIT_UNLIKELY(err))
        reportError(err);
      else
        id = node->getId();
    }
  }
  return Label(id);
}

Label CodeBuilder::newNamedLabel(const char* name, size_t nameLength, uint32_t type, uint32_t parentId) {
  uint32_t id = 0;
  if (_code) {
    CBLabel* node = newNodeT<CBLabel>(id);
    if (ASMJIT_UNLIKELY(!node)) {
      reportError(DebugUtils::errored(kErrorNoHeapMemory));
    }
    else {
      Error err = _code->newNamedLabelId(id, name, nameLength, type, parentId);
      if (ASMJIT_UNLIKELY(err))
        reportError(err);
      else
        id = node->getId();
    }
  }
  return Label(id);
}

Error CodeBuilder::bind(const Label& label) {
  CBLabel* node;
  Error err = getLabelNode(&node, label);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeBuilder - Pass Management]
// ============================================================================

ASMJIT_FAVOR_SIZE CBPass* CodeBuilder::getPassByName(const char* name) const noexcept {
  for (uint32_t i = 0, len = _passes.getLength(); i < len; i++) {
    CBPass* pass = _passes[i];
    if (std::strcmp(pass->getName(), name) == 0)
      return pass;
  }
  return nullptr;
}

ASMJIT_FAVOR_SIZE Error CodeBuilder::addPass(CBPass* pass) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(pass == nullptr)) {
    // Since this is directly called by `addPassT()` we treat `null` argument
    // as out-of-memory condition. Otherwise it would be API misuse.
    return DebugUtils::errored(kErrorNoHeapMemory);
  }
  else if (ASMJIT_UNLIKELY(pass->_cb)) {
    // Kinda weird, but okay...
    if (pass->_cb == this)
      return kErrorOk;
    return DebugUtils::errored(kErrorInvalidState);
  }

  ASMJIT_PROPAGATE(_passes.append(&_allocator, pass));
  pass->_cb = this;
  return kErrorOk;
}

ASMJIT_FAVOR_SIZE Error CodeBuilder::deletePass(CBPass* pass) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (ASMJIT_UNLIKELY(pass == nullptr))
    return DebugUtils::errored(kErrorInvalidArgument);

  if (pass->_cb != nullptr) {
    if (pass->_cb != this)
      return DebugUtils::errored(kErrorInvalidState);

    uint32_t index = _passes.indexOf(pass);
    ASMJIT_ASSERT(index != Globals::kNotFound);

    pass->_cb = nullptr;
    _passes.removeAt(index);
  }

  pass->~CBPass();
  return kErrorOk;
}

Error CodeBuilder::runPasses() {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  ZoneVector<CBPass*>& passes = _passes;
  uint32_t len = passes.getLength();

  if (!len)
    return kErrorOk;

  Logger* logger = getCode()->getLogger();
  ErrorHandler* prev = getErrorHandler();
  PostponedErrorHandler postponed;

  Error err = kErrorOk;
  setErrorHandler(&postponed);

  for (uint32_t i = 0; i < len; i++) {
    CBPass* pass = passes[i];

    _passZone.reset();
    err = pass->run(&_passZone, logger);
    if (err) break;
  }
  _passZone.reset();
  setErrorHandler(prev);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err, !postponed._message.isEmpty() ? postponed._message.getData() : nullptr);

  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeBuilder - Emit]
// ============================================================================

Error CodeBuilder::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {
  uint32_t opCount = 4;

  if (o3.isNone()) {
    opCount = 3;
    if (o2.isNone()) {
      opCount = 2;
      if (o1.isNone()) {
        opCount = 1;
        if (o0.isNone())
          opCount = 0;
      }
    }
  }

  uint32_t options = getInstOptions() | getGlobalInstOptions();
  if (options & Inst::kOptionReserved) {
    if (ASMJIT_UNLIKELY(!_code))
      return DebugUtils::errored(kErrorNotInitialized);

    // Strict validation.
    #ifndef ASMJIT_DISABLE_INST_API
    if (hasEmitterOption(kOptionStrictValidation)) {
      Operand_ opArray[4];
      opArray[0].copyFrom(o0);
      opArray[1].copyFrom(o1);
      opArray[2].copyFrom(o2);
      opArray[3].copyFrom(o3);

      Error err = Inst::validate(getArchType(), Inst::Detail(instId, options, _extraReg), opArray, opCount);
      if (ASMJIT_UNLIKELY(err)) {
        resetInstOptions();
        resetExtraReg();
        resetInlineComment();
        return reportError(err);
      }
    }
    #endif

    // Clear options that should never be part of `CBInst`.
    options &= ~Inst::kOptionReserved;
  }

  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node)) {
    resetInstOptions();
    resetExtraReg();
    resetInlineComment();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  node = new(node) CBInst(this, instId, options, opCount, opCapacity);
  node->setExtraReg(getExtraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);

  for (uint32_t i = 4; i < CBInst::kBaseOpCapacity; i++)
    node->resetOp(i);

  const char* inlineComment = getInlineComment();
  if (inlineComment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(inlineComment, std::strlen(inlineComment), true)));

  resetInstOptions();
  resetExtraReg();
  resetInlineComment();

  addNode(node);
  return kErrorOk;
}

Error CodeBuilder::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) {
  uint32_t opCount = Globals::kMaxOpCount;
  if (o5.isNone()) {
    opCount = 5;
    if (o4.isNone())
      return _emit(instId, o0, o1, o2, o3);
  }

  uint32_t options = getInstOptions() | getGlobalInstOptions();
  if (ASMJIT_UNLIKELY(options & Inst::kOptionReserved)) {
    if (ASMJIT_UNLIKELY(!_code))
      return DebugUtils::errored(kErrorNotInitialized);

    // Strict validation.
    #ifndef ASMJIT_DISABLE_INST_API
    if (hasEmitterOption(kOptionStrictValidation)) {
      Operand_ opArray[Globals::kMaxOpCount];
      opArray[0].copyFrom(o0);
      opArray[1].copyFrom(o1);
      opArray[2].copyFrom(o2);
      opArray[3].copyFrom(o3);
      opArray[4].copyFrom(o4);
      opArray[5].copyFrom(o5);

      Error err = Inst::validate(getArchType(), Inst::Detail(instId, options, _extraReg), opArray, opCount);
      if (ASMJIT_UNLIKELY(err)) {
        resetInstOptions();
        resetExtraReg();
        resetInlineComment();
        return reportError(err);
      }
    }
    #endif

    // Clear options that should never be part of `CBInst`.
    options &= ~Inst::kOptionReserved;
  }

  uint32_t opCapacity = CBInst::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= opCount);

  CBInst* node = _allocator.allocT<CBInst>(CBInst::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node)) {
    resetInstOptions();
    resetExtraReg();
    resetInlineComment();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  node = new(node) CBInst(this, instId, options, opCount, opCapacity);
  node->setExtraReg(getExtraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  node->setOp(4, o4);

  if (opCapacity > 5)
    node->setOp(5, o5);

  const char* inlineComment = getInlineComment();
  if (inlineComment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(inlineComment, std::strlen(inlineComment), true)));

  resetInstOptions();
  resetExtraReg();
  resetInlineComment();

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeBuilder - Align]
// ============================================================================

Error CodeBuilder::align(uint32_t mode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  CBAlign* node = newAlignNode(mode, alignment);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeBuilder - Embed]
// ============================================================================

Error CodeBuilder::embed(const void* data, uint32_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  CBData* node = newDataNode(data, size);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

Error CodeBuilder::embedLabel(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  CBLabelData* node = newNodeT<CBLabelData>(label.getId());
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

Error CodeBuilder::embedConstPool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (!isLabelValid(label))
    return reportError(DebugUtils::errored(kErrorInvalidLabel));

  ASMJIT_PROPAGATE(align(kAlignData, uint32_t(pool.getAlignment())));
  ASMJIT_PROPAGATE(bind(label));

  CBData* node = newDataNode(nullptr, uint32_t(pool.getSize()));
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  pool.fill(node->getData());
  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeBuilder - Comment]
// ============================================================================

Error CodeBuilder::comment(const char* s, size_t len) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  CBComment* node = newCommentNode(s, len);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::CodeBuilder - Serialize]
// ============================================================================

Error CodeBuilder::serialize(CodeEmitter* dst) {
  Error err = kErrorOk;
  CBNode* node_ = getFirstNode();

  do {
    dst->setInlineComment(node_->getInlineComment());

    switch (node_->getType()) {
      case CBNode::kNodeInst: {
OnInst:
        CBInst* node = node_->as<CBInst>();
        ASMJIT_ASSERT(node->getOpCapacity() >= 4);

        dst->setInstOptions(node->getInstOptions());
        dst->setExtraReg(node->getExtraReg());

        err = dst->emitOpArray(node->getInstId(), node->getOpArray(), node->getOpCount());
        break;
      }

      case CBNode::kNodeData: {
        CBData* node = node_->as<CBData>();
        err = dst->embed(node->getData(), node->getSize());
        break;
      }

      case CBNode::kNodeAlign: {
        CBAlign* node = node_->as<CBAlign>();
        err = dst->align(node->getMode(), node->getAlignment());
        break;
      }

      case CBNode::kNodeLabel: {
OnLabel:
        CBLabel* node = node_->as<CBLabel>();
        err = dst->bind(node->getLabel());
        break;
      }

      case CBNode::kNodeLabelData: {
        CBLabelData* node = node_->as<CBLabelData>();
        err = dst->embedLabel(node->getLabel());
        break;
      }

      case CBNode::kNodeConstPool: {
        CBConstPool* node = node_->as<CBConstPool>();
        err = dst->embedConstPool(node->getLabel(), node->getConstPool());
        break;
      }

      case CBNode::kNodeComment: {
        CBComment* node = node_->as<CBComment>();
        err = dst->comment(node->getInlineComment());
        break;
      }

      default:
        if (node_->actsAsInst()) goto OnInst;
        if (node_->actsAsLabel()) goto OnLabel;
        break;
    }

    if (err) break;
    node_ = node_->getNext();
  } while (node_);

  return err;
}

// ============================================================================
// [asmjit::CodeBuilder - Logging]
// ============================================================================

#ifndef ASMJIT_DISABLE_LOGGING
Error CodeBuilder::dump(StringBuilder& sb, uint32_t logOptions) const noexcept {
  CBNode* node = getFirstNode();
  while (node) {
    ASMJIT_PROPAGATE(Logging::formatNode(sb, logOptions, this, node));
    sb.appendChar('\n');

    node = node->getNext();
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::CodeBuilder - Events]
// ============================================================================

Error CodeBuilder::onAttach(CodeHolder* code) noexcept {
  return Base::onAttach(code);
}

Error CodeBuilder::onDetach(CodeHolder* code) noexcept {
  _passes.reset();
  _labelNodes.reset();

  _allocator.reset(&_codeZone);
  _codeZone.reset(false);
  _dataZone.reset(false);
  _passZone.reset(false);

  _nodeFlags = 0;

  _firstNode = nullptr;
  _lastNode = nullptr;
  _cursor = nullptr;

  return Base::onDetach(code);
}

// ============================================================================
// [asmjit::CBPass - Construction / Destruction]
// ============================================================================

CBPass::CBPass(const char* name) noexcept
  : _cb(nullptr),
    _name(name) {}
CBPass::~CBPass() noexcept {}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
