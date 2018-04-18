// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifndef ASMJIT_DISABLE_BUILDER

// [Dependencies]
#include "../core/builder.h"
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
  void handleError(Error err, const char* message, BaseEmitter* origin) override {
    ASMJIT_UNUSED(err);
    ASMJIT_UNUSED(origin);

    _message.setString(message);
  }

  StringBuilderTmp<128> _message;
};

// ============================================================================
// [asmjit::BaseBuilder - Construction / Destruction]
// ============================================================================

BaseBuilder::BaseBuilder() noexcept
  : BaseEmitter(kTypeBuilder),
    _codeZone(32768 - Zone::kBlockOverhead),
    _dataZone(16384 - Zone::kBlockOverhead),
    _passZone(65536 - Zone::kBlockOverhead),
    _allocator(&_codeZone),
    _passes(),
    _labelNodes(),
    _firstNode(nullptr),
    _lastNode(nullptr),
    _cursor(nullptr),
    _nodeFlags(0) {}
BaseBuilder::~BaseBuilder() noexcept {}

// ============================================================================
// [asmjit::BaseBuilder - Node Management]
// ============================================================================

LabelNode* BaseBuilder::newLabelNode() noexcept {
  LabelNode* node = newNodeT<LabelNode>();
  if (!node || registerLabelNode(node) != kErrorOk)
    return nullptr;
  return node;
}

AlignNode* BaseBuilder::newAlignNode(uint32_t alignMode, uint32_t alignment) noexcept {
  return newNodeT<AlignNode>(alignMode, alignment);
}

EmbedDataNode* BaseBuilder::newEmbedDataNode(const void* data, uint32_t size) noexcept {
  if (size > EmbedDataNode::kInlineBufferSize) {
    void* cloned = _dataZone.alloc(size);
    if (ASMJIT_UNLIKELY(!cloned))
      return nullptr;

    if (data)
      std::memcpy(cloned, data, size);
    data = cloned;
  }

  return newNodeT<EmbedDataNode>(const_cast<void*>(data), size);
}

ConstPoolNode* BaseBuilder::newConstPoolNode() noexcept {
  ConstPoolNode* node = newNodeT<ConstPoolNode>();
  if (!node || registerLabelNode(node) != kErrorOk)
    return nullptr;
  return node;
}

CommentNode* BaseBuilder::newCommentNode(const char* data, size_t size) noexcept {
  if (data) {
    if (size == Globals::kNullTerminated)
      size = std::strlen(data);

    if (size > 0) {
      data = static_cast<char*>(_dataZone.dup(data, size, true));
      if (!data) return nullptr;
    }
  }

  return newNodeT<CommentNode>(data);
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0) noexcept {
  uint32_t opCount = 1;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1) noexcept {
  uint32_t opCount = 2;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2) noexcept {
  uint32_t opCount = 3;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

InstNode* BaseBuilder::newInstNode(uint32_t instId, uint32_t instOptions, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) noexcept {
  uint32_t opCount = 4;
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;

  node = new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  for (uint32_t i = opCount; i < opCapacity; i++) node->resetOp(i);
  return node;
}

InstNode* BaseBuilder::newInstNodeRaw(uint32_t instId, uint32_t instOptions, uint32_t opCount) noexcept {
  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node))
    return nullptr;
  return new(node) InstNode(this, instId, instOptions, opCount, opCapacity);
}

BaseNode* BaseBuilder::addNode(BaseNode* node) noexcept {
  ASMJIT_ASSERT(node);
  ASMJIT_ASSERT(node->prev() == nullptr);
  ASMJIT_ASSERT(node->next() == nullptr);

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
    BaseNode* prev = _cursor;
    BaseNode* next = _cursor->next();

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

BaseNode* BaseBuilder::addAfter(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(node);
  ASMJIT_ASSERT(ref);

  ASMJIT_ASSERT(node->prev() == nullptr);
  ASMJIT_ASSERT(node->next() == nullptr);

  BaseNode* prev = ref;
  BaseNode* next = ref->next();

  node->_setPrev(prev);
  node->_setNext(next);

  prev->_setNext(node);
  if (next)
    next->_setPrev(node);
  else
    _lastNode = node;

  return node;
}

BaseNode* BaseBuilder::addBefore(BaseNode* node, BaseNode* ref) noexcept {
  ASMJIT_ASSERT(node != nullptr);
  ASMJIT_ASSERT(node->prev() == nullptr);
  ASMJIT_ASSERT(node->next() == nullptr);
  ASMJIT_ASSERT(ref != nullptr);

  BaseNode* prev = ref->prev();
  BaseNode* next = ref;

  node->_setPrev(prev);
  node->_setNext(next);

  next->_setPrev(node);
  if (prev)
    prev->_setNext(node);
  else
    _firstNode = node;

  return node;
}

BaseNode* BaseBuilder::removeNode(BaseNode* node) noexcept {
  BaseNode* prev = node->prev();
  BaseNode* next = node->next();

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

void BaseBuilder::removeNodes(BaseNode* first, BaseNode* last) noexcept {
  if (first == last) {
    removeNode(first);
    return;
  }

  BaseNode* prev = first->prev();
  BaseNode* next = last->next();

  if (_firstNode == first)
    _firstNode = next;
  else
    prev->_setNext(next);

  if (_lastNode == last)
    _lastNode  = prev;
  else
    next->_setPrev(prev);

  BaseNode* node = first;
  for (;;) {
    next = node->next();
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

BaseNode* BaseBuilder::setCursor(BaseNode* node) noexcept {
  BaseNode* old = _cursor;
  _cursor = node;
  return old;
}

// ============================================================================
// [asmjit::BaseBuilder - Label Management]
// ============================================================================

Error BaseBuilder::labelNodeOf(LabelNode** pOut, uint32_t id) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  uint32_t index = Operand::unpackId(id);

  if (ASMJIT_UNLIKELY(index >= _code->labelCount()))
    return DebugUtils::errored(kErrorInvalidLabel);

  if (index >= _labelNodes.size())
    ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, index + 1));

  LabelNode* node = _labelNodes[index];
  if (!node) {
    node = newNodeT<LabelNode>(id);
    if (ASMJIT_UNLIKELY(!node))
      return DebugUtils::errored(kErrorNoHeapMemory);
    _labelNodes[index] = node;
  }

  *pOut = node;
  return kErrorOk;
}

Error BaseBuilder::registerLabelNode(LabelNode* node) noexcept {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  // Don't call `reportError()` from here, we are noexcept and we are called
  // by `newLabelNode()` and `newFuncNode()`, which are noexcept as well.
  uint32_t id;
  ASMJIT_PROPAGATE(_code->newLabelId(id));
  uint32_t index = Operand::unpackId(id);

  // We just added one label so it must be true.
  ASMJIT_ASSERT(_labelNodes.size() < index + 1);
  ASMJIT_PROPAGATE(_labelNodes.resize(&_allocator, index + 1));

  _labelNodes[index] = node;
  node->_id = id;
  return kErrorOk;
}

Label BaseBuilder::newLabel() {
  uint32_t id = 0;
  if (_code) {
    LabelNode* node = newNodeT<LabelNode>(id);
    if (ASMJIT_UNLIKELY(!node)) {
      reportError(DebugUtils::errored(kErrorNoHeapMemory));
    }
    else {
      Error err = registerLabelNode(node);
      if (ASMJIT_UNLIKELY(err))
        reportError(err);
      else
        id = node->id();
    }
  }
  return Label(id);
}

Label BaseBuilder::newNamedLabel(const char* name, size_t nameSize, uint32_t type, uint32_t parentId) {
  uint32_t id = 0;
  if (_code) {
    LabelNode* node = newNodeT<LabelNode>(id);
    if (ASMJIT_UNLIKELY(!node)) {
      reportError(DebugUtils::errored(kErrorNoHeapMemory));
    }
    else {
      Error err = _code->newNamedLabelId(id, name, nameSize, type, parentId);
      if (ASMJIT_UNLIKELY(err))
        reportError(err);
      else
        id = node->id();
    }
  }
  return Label(id);
}

Error BaseBuilder::bind(const Label& label) {
  LabelNode* node;
  Error err = labelNodeOf(&node, label);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err);

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Pass Management]
// ============================================================================

ASMJIT_FAVOR_SIZE Pass* BaseBuilder::passByName(const char* name) const noexcept {
  for (Pass* pass : _passes)
    if (std::strcmp(pass->name(), name) == 0)
      return pass;
  return nullptr;
}

ASMJIT_FAVOR_SIZE Error BaseBuilder::addPass(Pass* pass) noexcept {
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

ASMJIT_FAVOR_SIZE Error BaseBuilder::deletePass(Pass* pass) noexcept {
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

  pass->~Pass();
  return kErrorOk;
}

Error BaseBuilder::runPasses() {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (_passes.empty())
    return kErrorOk;

  Logger* logger = code()->logger();
  ErrorHandler* prev = errorHandler();
  PostponedErrorHandler postponed;

  Error err = kErrorOk;
  setErrorHandler(&postponed);

  for (Pass* pass : _passes) {
    _passZone.reset();
    err = pass->run(&_passZone, logger);
    if (err) break;
  }
  _passZone.reset();
  setErrorHandler(prev);

  if (ASMJIT_UNLIKELY(err))
    return reportError(err, !postponed._message.empty() ? postponed._message.data() : nullptr);

  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Emit]
// ============================================================================

Error BaseBuilder::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3) {
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

  uint32_t options = instOptions() | globalInstOptions();
  if (options & BaseInst::kOptionReserved) {
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

      Error err = BaseInst::validate(archId(), BaseInst(instId, options, _extraReg), opArray, opCount);
      if (ASMJIT_UNLIKELY(err)) {
        resetInstOptions();
        resetExtraReg();
        resetInlineComment();
        return reportError(err);
      }
    }
    #endif

    // Clear options that should never be part of `InstNode`.
    options &= ~BaseInst::kOptionReserved;
  }

  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= 4);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node)) {
    resetInstOptions();
    resetExtraReg();
    resetInlineComment();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  node = new(node) InstNode(this, instId, options, opCount, opCapacity);
  node->setExtraReg(extraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);

  for (uint32_t i = 4; i < InstNode::kBaseOpCapacity; i++)
    node->resetOp(i);

  const char* comment = inlineComment();
  if (comment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(comment, std::strlen(comment), true)));

  resetInstOptions();
  resetExtraReg();
  resetInlineComment();

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::_emit(uint32_t instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_& o3, const Operand_& o4, const Operand_& o5) {
  uint32_t opCount = Globals::kMaxOpCount;
  if (o5.isNone()) {
    opCount = 5;
    if (o4.isNone())
      return _emit(instId, o0, o1, o2, o3);
  }

  uint32_t options = instOptions() | globalInstOptions();
  if (ASMJIT_UNLIKELY(options & BaseInst::kOptionReserved)) {
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

      Error err = BaseInst::validate(archId(), BaseInst(instId, options, _extraReg), opArray, opCount);
      if (ASMJIT_UNLIKELY(err)) {
        resetInstOptions();
        resetExtraReg();
        resetInlineComment();
        return reportError(err);
      }
    }
    #endif

    // Clear options that should never be part of `InstNode`.
    options &= ~BaseInst::kOptionReserved;
  }

  uint32_t opCapacity = InstNode::capacityOfOpCount(opCount);
  ASMJIT_ASSERT(opCapacity >= opCount);

  InstNode* node = _allocator.allocT<InstNode>(InstNode::nodeSizeOfOpCapacity(opCapacity));
  if (ASMJIT_UNLIKELY(!node)) {
    resetInstOptions();
    resetExtraReg();
    resetInlineComment();
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));
  }

  node = new(node) InstNode(this, instId, options, opCount, opCapacity);
  node->setExtraReg(extraReg());
  node->setOp(0, o0);
  node->setOp(1, o1);
  node->setOp(2, o2);
  node->setOp(3, o3);
  node->setOp(4, o4);

  if (opCapacity > 5)
    node->setOp(5, o5);

  const char* comment = inlineComment();
  if (comment)
    node->setInlineComment(static_cast<char*>(_dataZone.dup(comment, std::strlen(comment), true)));

  resetInstOptions();
  resetExtraReg();
  resetInlineComment();

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Align]
// ============================================================================

Error BaseBuilder::align(uint32_t alignMode, uint32_t alignment) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  AlignNode* node = newAlignNode(alignMode, alignment);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Embed]
// ============================================================================

Error BaseBuilder::embed(const void* data, uint32_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  EmbedDataNode* node = newEmbedDataNode(data, size);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedLabel(const Label& label) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  LabelDataNode* node = newNodeT<LabelDataNode>(label.id());
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

Error BaseBuilder::embedConstPool(const Label& label, const ConstPool& pool) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  if (!isLabelValid(label))
    return reportError(DebugUtils::errored(kErrorInvalidLabel));

  ASMJIT_PROPAGATE(align(kAlignData, uint32_t(pool.alignment())));
  ASMJIT_PROPAGATE(bind(label));

  EmbedDataNode* node = newEmbedDataNode(nullptr, uint32_t(pool.size()));
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  pool.fill(node->data());
  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Comment]
// ============================================================================

Error BaseBuilder::comment(const char* data, size_t size) {
  if (ASMJIT_UNLIKELY(!_code))
    return DebugUtils::errored(kErrorNotInitialized);

  CommentNode* node = newCommentNode(data, size);
  if (ASMJIT_UNLIKELY(!node))
    return reportError(DebugUtils::errored(kErrorNoHeapMemory));

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::BaseBuilder - Serialize]
// ============================================================================

Error BaseBuilder::serialize(BaseEmitter* dst) {
  Error err = kErrorOk;
  BaseNode* node_ = firstNode();

  do {
    dst->setInlineComment(node_->inlineComment());

    if (node_->isInst()) {
      InstNode* node = node_->as<InstNode>();
      err = dst->emitInst(node->baseInst(), node->operands(), node->opCount());
    }
    else if (node_->isLabel()) {
      if (node_->isConstPool()) {
        ConstPoolNode* node = node_->as<ConstPoolNode>();
        err = dst->embedConstPool(node->label(), node->constPool());
      }
      else {
        LabelNode* node = node_->as<LabelNode>();
        err = dst->bind(node->label());
      }
    }
    else if (node_->isAlign()) {
      AlignNode* node = node_->as<AlignNode>();
      err = dst->align(node->alignMode(), node->alignment());
    }
    else if (node_->isEmbedData()) {
      EmbedDataNode* node = node_->as<EmbedDataNode>();
      err = dst->embed(node->data(), node->size());
    }
    else if (node_->isLabelData()) {
      LabelDataNode* node = node_->as<LabelDataNode>();
      err = dst->embedLabel(node->label());
    }
    else if (node_->isComment()) {
      CommentNode* node = node_->as<CommentNode>();
      err = dst->comment(node->inlineComment());
    }

    if (err) break;
    node_ = node_->next();
  } while (node_);

  return err;
}

// ============================================================================
// [asmjit::BaseBuilder - Logging]
// ============================================================================

#ifndef ASMJIT_DISABLE_LOGGING
Error BaseBuilder::dump(StringBuilder& sb, uint32_t flags) const noexcept {
  BaseNode* node = firstNode();
  while (node) {
    ASMJIT_PROPAGATE(Logging::formatNode(sb, flags, this, node));
    sb.appendChar('\n');
    node = node->next();
  }

  return kErrorOk;
}
#endif

// ============================================================================
// [asmjit::BaseBuilder - Events]
// ============================================================================

Error BaseBuilder::onAttach(CodeHolder* code) noexcept {
  return Base::onAttach(code);
}

Error BaseBuilder::onDetach(CodeHolder* code) noexcept {
  _passes.reset();
  _labelNodes.reset();

  _allocator.reset(&_codeZone);
  _codeZone.reset();
  _dataZone.reset();
  _passZone.reset();

  _nodeFlags = 0;

  _firstNode = nullptr;
  _lastNode = nullptr;
  _cursor = nullptr;

  return Base::onDetach(code);
}

// ============================================================================
// [asmjit::Pass - Construction / Destruction]
// ============================================================================

Pass::Pass(const char* name) noexcept
  : _cb(nullptr),
    _name(name) {}
Pass::~Pass() noexcept {}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_BUILDER
