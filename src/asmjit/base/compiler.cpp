// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if !defined(ASMJIT_DISABLE_COMPILER)

// [Dependencies - AsmJit]
#include "../base/assembler.h"
#include "../base/compiler.h"
#include "../base/compilercontext_p.h"
#include "../base/cpuinfo.h"
#include "../base/logger.h"
#include "../base/utils.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [Constants]
// ============================================================================

static const char noName[1] = { '\0' };
enum { kCompilerDefaultLookAhead = 64 };

// ============================================================================
// [asmjit::Compiler - Construction / Destruction]
// ============================================================================

Compiler::Compiler() :
  _features(0),
  _maxLookAhead(kCompilerDefaultLookAhead),
  _instOptions(0),
  _tokenGenerator(0),
  _nodeFlowId(0),
  _nodeFlags(0),
  _targetVarMapping(NULL),
  _firstNode(NULL),
  _lastNode(NULL),
  _cursor(NULL),
  _func(NULL),
  _zoneAllocator(8192 - Zone::kZoneOverhead),
  _varAllocator(4096 - Zone::kZoneOverhead),
  _stringAllocator(4096 - Zone::kZoneOverhead),
  _constAllocator(4096 - Zone::kZoneOverhead),
  _localConstPool(&_constAllocator),
  _globalConstPool(&_zoneAllocator) {}
Compiler::~Compiler() {}

// ============================================================================
// [asmjit::Compiler - Attach / Reset]
// ============================================================================

void Compiler::reset(bool releaseMemory) {
  Assembler* assembler = getAssembler();
  if (assembler != NULL)
    assembler->_detached(this);

  _arch = kArchNone;
  _regSize = 0;
  _finalized = false;
  _lastError = kErrorNotInitialized;

  _features = 0;
  _maxLookAhead = kCompilerDefaultLookAhead;

  _instOptions = 0;
  _tokenGenerator = 0;

  _nodeFlowId = 0;
  _nodeFlags = 0;

  _firstNode = NULL;
  _lastNode = NULL;

  _cursor = NULL;
  _func = NULL;

  _localConstPool.reset();
  _globalConstPool.reset();

  _localConstPoolLabel.reset();
  _globalConstPoolLabel.reset();

  _zoneAllocator.reset(releaseMemory);
  _varAllocator.reset(releaseMemory);
  _stringAllocator.reset(releaseMemory);
  _constAllocator.reset(releaseMemory);

  _varList.reset(releaseMemory);
}

// ============================================================================
// [asmjit::Compiler - Node-Factory]
// ============================================================================

HLData* Compiler::newDataNode(const void* data, uint32_t size) {
  if (size > HLData::kInlineBufferSize) {
    void* clonedData = _stringAllocator.alloc(size);
    if (clonedData == NULL)
      return NULL;

    if (data != NULL)
      ::memcpy(clonedData, data, size);
    data = clonedData;
  }

  return newNode<HLData>(const_cast<void*>(data), size);
}

HLAlign* Compiler::newAlignNode(uint32_t alignMode, uint32_t offset) {
  return newNode<HLAlign>(alignMode, offset);
}

HLLabel* Compiler::newLabelNode() {
  Assembler* assembler = getAssembler();
  if (assembler == NULL) return NULL;

  uint32_t id = assembler->_newLabelId();
  LabelData* ld = assembler->getLabelData(id);

  HLLabel* node = newNode<HLLabel>(id);
  if (node == NULL) return NULL;

  // These have to be zero now.
  ASMJIT_ASSERT(ld->hlId == 0);
  ASMJIT_ASSERT(ld->hlData == NULL);

  ld->hlId = _hlId;
  ld->hlData = node;

  return node;
}

HLComment* Compiler::newCommentNode(const char* str) {
  if (str != NULL && str[0]) {
    str = _stringAllocator.sdup(str);
    if (str == NULL)
      return NULL;
  }

  return newNode<HLComment>(str);
}

HLHint* Compiler::newHintNode(Var& var, uint32_t hint, uint32_t value) {
  if (var.getId() == kInvalidValue)
    return NULL;

  VarData* vd = getVd(var);
  return newNode<HLHint>(vd, hint, value);
}

// ============================================================================
// [asmjit::Compiler - Code-Stream]
// ============================================================================

HLNode* Compiler::addNode(HLNode* node) {
  ASMJIT_ASSERT(node != NULL);
  ASMJIT_ASSERT(node->_prev == NULL);
  ASMJIT_ASSERT(node->_next == NULL);

  if (_cursor == NULL) {
    if (_firstNode == NULL) {
      _firstNode = node;
      _lastNode = node;
    }
    else {
      node->_next = _firstNode;
      _firstNode->_prev = node;
      _firstNode = node;
    }
  }
  else {
    HLNode* prev = _cursor;
    HLNode* next = _cursor->_next;

    node->_prev = prev;
    node->_next = next;

    prev->_next = node;
    if (next)
      next->_prev = node;
    else
      _lastNode = node;
  }

  _cursor = node;
  return node;
}

HLNode* Compiler::addNodeBefore(HLNode* node, HLNode* ref) {
  ASMJIT_ASSERT(node != NULL);
  ASMJIT_ASSERT(node->_prev == NULL);
  ASMJIT_ASSERT(node->_next == NULL);
  ASMJIT_ASSERT(ref != NULL);

  HLNode* prev = ref->_prev;
  HLNode* next = ref;

  node->_prev = prev;
  node->_next = next;

  next->_prev = node;
  if (prev)
    prev->_next = node;
  else
    _firstNode = node;

  return node;
}

HLNode* Compiler::addNodeAfter(HLNode* node, HLNode* ref) {
  ASMJIT_ASSERT(node != NULL);
  ASMJIT_ASSERT(node->_prev == NULL);
  ASMJIT_ASSERT(node->_next == NULL);
  ASMJIT_ASSERT(ref != NULL);

  HLNode* prev = ref;
  HLNode* next = ref->_next;

  node->_prev = prev;
  node->_next = next;

  prev->_next = node;
  if (next)
    next->_prev = node;
  else
    _lastNode = node;

  return node;
}

static ASMJIT_INLINE void Compiler_nodeRemoved(Compiler* self, HLNode* node_) {
  if (node_->isJmpOrJcc()) {
    HLJump* node = static_cast<HLJump*>(node_);
    HLLabel* label = node->getTarget();

    if (label != NULL) {
      // Disconnect.
      HLJump** pPrev = &label->_from;
      for (;;) {
        ASMJIT_ASSERT(*pPrev != NULL);
        HLJump* current = *pPrev;

        if (current == NULL)
          break;

        if (current == node) {
          *pPrev = node->_jumpNext;
          break;
        }

        pPrev = &current->_jumpNext;
      }

      label->subNumRefs();
    }
  }
}

HLNode* Compiler::removeNode(HLNode* node) {
  HLNode* prev = node->_prev;
  HLNode* next = node->_next;

  if (_firstNode == node)
    _firstNode = next;
  else
    prev->_next = next;

  if (_lastNode == node)
    _lastNode  = prev;
  else
    next->_prev = prev;

  node->_prev = NULL;
  node->_next = NULL;

  if (_cursor == node)
    _cursor = prev;
  Compiler_nodeRemoved(this, node);

  return node;
}

void Compiler::removeNodes(HLNode* first, HLNode* last) {
  if (first == last) {
    removeNode(first);
    return;
  }

  HLNode* prev = first->_prev;
  HLNode* next = last->_next;

  if (_firstNode == first)
    _firstNode = next;
  else
    prev->_next = next;

  if (_lastNode == last)
    _lastNode  = prev;
  else
    next->_prev = prev;

  HLNode* node = first;
  for (;;) {
    HLNode* next = node->getNext();
    ASMJIT_ASSERT(next != NULL);

    node->_prev = NULL;
    node->_next = NULL;

    if (_cursor == node)
      _cursor = prev;
    Compiler_nodeRemoved(this, node);

    if (node == last)
      break;
    node = next;
  }
}

HLNode* Compiler::setCursor(HLNode* node) {
  HLNode* old = _cursor;
  _cursor = node;
  return old;
}

// ============================================================================
// [asmjit::Compiler - Align]
// ============================================================================

Error Compiler::align(uint32_t alignMode, uint32_t offset) {
  HLAlign* node = newAlignNode(alignMode, offset);
  if (node == NULL)
    return setLastError(kErrorNoHeapMemory);

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::Compiler - Label]
// ============================================================================

HLLabel* Compiler::getHLLabel(uint32_t id) const {
  Assembler* assembler = getAssembler();
  if (assembler == NULL) return NULL;

  LabelData* ld = assembler->getLabelData(id);
  if (ld->hlId == _hlId)
    return static_cast<HLLabel*>(ld->hlData);
  else
    return NULL;
}

bool Compiler::isLabelValid(uint32_t id) const {
  Assembler* assembler = getAssembler();
  if (assembler == NULL) return false;

  return static_cast<size_t>(id) < assembler->getLabelsCount();
}

uint32_t Compiler::_newLabelId() {
  HLLabel* node = newLabelNode();
  if (node == NULL) {
    setLastError(kErrorNoHeapMemory);
    return kInvalidValue;
  }

  return node->getLabelId();
}

Error Compiler::bind(const Label& label) {
  HLLabel* node = getHLLabel(label);
  if (node == NULL)
    return setLastError(kErrorInvalidState);
  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::Compiler - Embed]
// ============================================================================

Error Compiler::embed(const void* data, uint32_t size) {
  HLData* node = newDataNode(data, size);
  if (node == NULL)
    return setLastError(kErrorNoHeapMemory);

  addNode(node);
  return kErrorOk;
}

Error Compiler::embedConstPool(const Label& label, const ConstPool& pool) {
  if (label.getId() == kInvalidValue)
    return kErrorInvalidState;

  align(kAlignData, static_cast<uint32_t>(pool.getAlignment()));
  bind(label);

  HLData* embedNode = newDataNode(NULL, static_cast<uint32_t>(pool.getSize()));
  if (embedNode == NULL)
    return kErrorNoHeapMemory;

  pool.fill(embedNode->getData());
  addNode(embedNode);

  return kErrorOk;
}

// ============================================================================
// [asmjit::Compiler - Comment]
// ============================================================================

Error Compiler::comment(const char* fmt, ...) {
  char buf[256];
  char* p = buf;

  if (fmt) {
    va_list ap;
    va_start(ap, fmt);
    p += vsnprintf(p, 254, fmt, ap);
    va_end(ap);
  }

  p[0] = '\0';

  HLComment* node = newCommentNode(buf);
  if (node == NULL)
    return setLastError(kErrorNoHeapMemory);

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::Compiler - Hint]
// ============================================================================

Error Compiler::_hint(Var& var, uint32_t hint, uint32_t value) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;

  HLHint* node = newHintNode(var, hint, value);
  if (node == NULL)
    return setLastError(kErrorNoHeapMemory);

  addNode(node);
  return kErrorOk;
}

// ============================================================================
// [asmjit::Compiler - Vars]
// ============================================================================

VarData* Compiler::_newVd(uint32_t type, uint32_t size, uint32_t c, const char* name) {
  VarData* vd = reinterpret_cast<VarData*>(_varAllocator.alloc(sizeof(VarData)));
  if (vd == NULL)
    goto _NoMemory;

  vd->_name = noName;
  vd->_id = OperandUtil::makeVarId(static_cast<uint32_t>(_varList.getLength()));
  vd->_localId = kInvalidValue;

#if !defined(ASMJIT_DISABLE_LOGGER)
  if (name != NULL && name[0] != '\0') {
    vd->_name = _stringAllocator.sdup(name);
  }
#endif // !ASMJIT_DISABLE_LOGGER

  vd->_type = static_cast<uint8_t>(type);
  vd->_class = static_cast<uint8_t>(c);
  vd->_flags = 0;
  vd->_priority = 10;

  vd->_state = kVarStateNone;
  vd->_regIndex = kInvalidReg;
  vd->_isStack = false;
  vd->_isMemArg = false;
  vd->_isCalculated = false;
  vd->_saveOnUnuse = false;
  vd->_modified = false;
  vd->_reserved0 = 0;
  vd->_alignment = static_cast<uint8_t>(Utils::iMin<uint32_t>(size, 64));

  vd->_size = size;
  vd->_homeMask = 0;

  vd->_memOffset = 0;
  vd->_memCell = NULL;

  vd->rReadCount = 0;
  vd->rWriteCount = 0;
  vd->mReadCount = 0;
  vd->mWriteCount = 0;

  vd->_va = NULL;

  if (_varList.append(vd) != kErrorOk)
    goto _NoMemory;
  return vd;

_NoMemory:
  setLastError(kErrorNoHeapMemory);
  return NULL;
}

Error Compiler::alloc(Var& var) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;
  return _hint(var, kVarHintAlloc, kInvalidValue);
}

Error Compiler::alloc(Var& var, uint32_t regIndex) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;
  return _hint(var, kVarHintAlloc, regIndex);
}

Error Compiler::alloc(Var& var, const Reg& reg) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;
  return _hint(var, kVarHintAlloc, reg.getRegIndex());
}

Error Compiler::save(Var& var) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;
  return _hint(var, kVarHintSave, kInvalidValue);
}

Error Compiler::spill(Var& var) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;
  return _hint(var, kVarHintSpill, kInvalidValue);
}

Error Compiler::unuse(Var& var) {
  if (var.getId() == kInvalidValue)
    return kErrorOk;
  return _hint(var, kVarHintUnuse, kInvalidValue);
}

uint32_t Compiler::getPriority(Var& var) const {
  if (var.getId() == kInvalidValue)
    return kInvalidValue;

  VarData* vd = getVdById(var.getId());
  return vd->getPriority();
}

void Compiler::setPriority(Var& var, uint32_t priority) {
  if (var.getId() == kInvalidValue)
    return;

  if (priority > 255)
    priority = 255;

  VarData* vd = getVdById(var.getId());
  vd->_priority = static_cast<uint8_t>(priority);
}

bool Compiler::getSaveOnUnuse(Var& var) const {
  if (var.getId() == kInvalidValue)
    return false;

  VarData* vd = getVdById(var.getId());
  return static_cast<bool>(vd->_saveOnUnuse);
}

void Compiler::setSaveOnUnuse(Var& var, bool value) {
  if (var.getId() == kInvalidValue)
    return;

  VarData* vd = getVdById(var.getId());
  vd->_saveOnUnuse = value;
}

void Compiler::rename(Var& var, const char* fmt, ...) {
  if (var.getId() == kInvalidValue)
    return;

  VarData* vd = getVdById(var.getId());
  vd->_name = noName;

  if (fmt != NULL && fmt[0] != '\0') {
    char buf[64];

    va_list ap;
    va_start(ap, fmt);

    vsnprintf(buf, ASMJIT_ARRAY_SIZE(buf), fmt, ap);
    buf[ASMJIT_ARRAY_SIZE(buf) - 1] = '\0';

    vd->_name = _stringAllocator.sdup(buf);
    va_end(ap);
  }
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
