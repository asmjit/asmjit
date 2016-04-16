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
#include "../base/compilercontext_p.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::Context - Construction / Destruction]
// ============================================================================

Context::Context(Compiler* compiler) :
  _compiler(compiler),
  _zoneAllocator(8192 - Zone::kZoneOverhead),
  _traceNode(nullptr),
  _varMapToVaListOffset(0) {

  Context::reset();
}
Context::~Context() {}

// ============================================================================
// [asmjit::Context - Reset]
// ============================================================================

void Context::reset(bool releaseMemory) {
  _zoneAllocator.reset(releaseMemory);

  _func = nullptr;
  _start = nullptr;
  _end = nullptr;
  _extraBlock = nullptr;
  _stop = nullptr;

  _unreachableList.reset();
  _returningList.reset();
  _jccList.reset();
  _contextVd.reset(releaseMemory);

  _memVarCells = nullptr;
  _memStackCells = nullptr;

  _mem1ByteVarsUsed = 0;
  _mem2ByteVarsUsed = 0;
  _mem4ByteVarsUsed = 0;
  _mem8ByteVarsUsed = 0;
  _mem16ByteVarsUsed = 0;
  _mem32ByteVarsUsed = 0;
  _mem64ByteVarsUsed = 0;
  _memStackCellsUsed = 0;

  _memMaxAlign = 0;
  _memVarTotal = 0;
  _memStackTotal = 0;
  _memAllTotal = 0;
  _annotationLength = 12;

  _state = nullptr;
}

// ============================================================================
// [asmjit::Context - Mem]
// ============================================================================

static ASMJIT_INLINE uint32_t BaseContext_getDefaultAlignment(uint32_t size) {
  if (size > 32)
    return 64;
  else if (size > 16)
    return 32;
  else if (size > 8)
    return 16;
  else if (size > 4)
    return 8;
  else if (size > 2)
    return 4;
  else if (size > 1)
    return 2;
  else
    return 1;
}

VarCell* Context::_newVarCell(VarData* vd) {
  ASMJIT_ASSERT(vd->_memCell == nullptr);

  VarCell* cell;
  uint32_t size = vd->getSize();

  if (vd->isStack()) {
    cell = _newStackCell(size, vd->getAlignment());

    if (cell == nullptr)
      return nullptr;
  }
  else {
    cell = static_cast<VarCell*>(_zoneAllocator.alloc(sizeof(VarCell)));
    if (cell == nullptr)
      goto _NoMemory;

    cell->_next = _memVarCells;
    _memVarCells = cell;

    cell->_offset = 0;
    cell->_size = size;
    cell->_alignment = size;

    _memMaxAlign = Utils::iMax<uint32_t>(_memMaxAlign, size);
    _memVarTotal += size;

    switch (size) {
      case  1: _mem1ByteVarsUsed++ ; break;
      case  2: _mem2ByteVarsUsed++ ; break;
      case  4: _mem4ByteVarsUsed++ ; break;
      case  8: _mem8ByteVarsUsed++ ; break;
      case 16: _mem16ByteVarsUsed++; break;
      case 32: _mem32ByteVarsUsed++; break;
      case 64: _mem64ByteVarsUsed++; break;

      default:
        ASMJIT_NOT_REACHED();
    }
  }

  vd->_memCell = cell;
  return cell;

_NoMemory:
  _compiler->setLastError(kErrorNoHeapMemory);
  return nullptr;
}

VarCell* Context::_newStackCell(uint32_t size, uint32_t alignment) {
  VarCell* cell = static_cast<VarCell*>(_zoneAllocator.alloc(sizeof(VarCell)));
  if (cell == nullptr)
    goto _NoMemory;

  if (alignment == 0)
    alignment = BaseContext_getDefaultAlignment(size);

  if (alignment > 64)
    alignment = 64;

  ASMJIT_ASSERT(Utils::isPowerOf2(alignment));
  size = Utils::alignTo<uint32_t>(size, alignment);

  // Insert it sorted according to the alignment and size.
  {
    VarCell** pPrev = &_memStackCells;
    VarCell* cur = *pPrev;

    while (cur != nullptr) {
      if ((cur->getAlignment() > alignment) ||
          (cur->getAlignment() == alignment && cur->getSize() > size)) {
        pPrev = &cur->_next;
        cur = *pPrev;
        continue;
      }

      break;
    }

    cell->_next = cur;
    cell->_offset = 0;
    cell->_size = size;
    cell->_alignment = alignment;

    *pPrev = cell;
    _memStackCellsUsed++;

    _memMaxAlign = Utils::iMax<uint32_t>(_memMaxAlign, alignment);
    _memStackTotal += size;
  }

  return cell;

_NoMemory:
  _compiler->setLastError(kErrorNoHeapMemory);
  return nullptr;
}

Error Context::resolveCellOffsets() {
  VarCell* varCell = _memVarCells;
  VarCell* stackCell = _memStackCells;

  uint32_t stackAlignment = 0;
  if (stackCell != nullptr)
    stackAlignment = stackCell->getAlignment();

  uint32_t pos64 = 0;
  uint32_t pos32 = pos64 + _mem64ByteVarsUsed * 64;
  uint32_t pos16 = pos32 + _mem32ByteVarsUsed * 32;
  uint32_t pos8  = pos16 + _mem16ByteVarsUsed * 16;
  uint32_t pos4  = pos8  + _mem8ByteVarsUsed  * 8 ;
  uint32_t pos2  = pos4  + _mem4ByteVarsUsed  * 4 ;
  uint32_t pos1  = pos2  + _mem2ByteVarsUsed  * 2 ;

  uint32_t stackPos = pos1 + _mem1ByteVarsUsed;

  uint32_t gapAlignment = stackAlignment;
  uint32_t gapSize = 0;

  // TODO: Not used!
  if (gapAlignment)
    Utils::alignDiff(stackPos, gapAlignment);
  stackPos += gapSize;

  uint32_t gapPos = stackPos;
  uint32_t allTotal = stackPos;

  // Vars - Allocated according to alignment/width.
  while (varCell != nullptr) {
    uint32_t size = varCell->getSize();
    uint32_t offset = 0;

    switch (size) {
      case  1: offset = pos1 ; pos1  += 1 ; break;
      case  2: offset = pos2 ; pos2  += 2 ; break;
      case  4: offset = pos4 ; pos4  += 4 ; break;
      case  8: offset = pos8 ; pos8  += 8 ; break;
      case 16: offset = pos16; pos16 += 16; break;
      case 32: offset = pos32; pos32 += 32; break;
      case 64: offset = pos64; pos64 += 64; break;

      default:
        ASMJIT_NOT_REACHED();
    }

    varCell->setOffset(static_cast<int32_t>(offset));
    varCell = varCell->_next;
  }

  // Stack - Allocated according to alignment/width.
  while (stackCell != nullptr) {
    uint32_t size = stackCell->getSize();
    uint32_t alignment = stackCell->getAlignment();
    uint32_t offset;

    // Try to fill the gap between variables/stack first.
    if (size <= gapSize && alignment <= gapAlignment) {
      offset = gapPos;

      gapSize -= size;
      gapPos -= size;

      if (alignment < gapAlignment)
        gapAlignment = alignment;
    }
    else {
      offset = stackPos;

      stackPos += size;
      allTotal += size;
    }

    stackCell->setOffset(offset);
    stackCell = stackCell->_next;
  }

  _memAllTotal = allTotal;
  return kErrorOk;
}

// ============================================================================
// [asmjit::Context - RemoveUnreachableCode]
// ============================================================================

Error Context::removeUnreachableCode() {
  Compiler* compiler = getCompiler();

  PodList<HLNode*>::Link* link = _unreachableList.getFirst();
  HLNode* stop = getStop();

  while (link != nullptr) {
    HLNode* node = link->getValue();
    if (node != nullptr && node->getPrev() != nullptr && node != stop) {
      // Locate all unreachable nodes.
      HLNode* first = node;
      do {
        if (node->isFetched())
          break;
        node = node->getNext();
      } while (node != stop);

      // Remove unreachable nodes that are neither informative nor directives.
      if (node != first) {
        HLNode* end = node;
        node = first;

        // NOTE: The strategy is as follows:
        // 1. The algorithm removes everything until it finds a first label.
        // 2. After the first label is found it removes only removable nodes.
        bool removeEverything = true;
        do {
          HLNode* next = node->getNext();
          bool remove = node->isRemovable();

          if (!remove) {
            if (node->isLabel())
              removeEverything = false;
            remove = removeEverything;
          }

          if (remove) {
            ASMJIT_TSEC({
              this->_traceNode(this, node, "[REMOVED UNREACHABLE] ");
            });
            compiler->removeNode(node);
          }

          node = next;
        } while (node != end);
      }
    }

    link = link->getNext();
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::Context - Liveness Analysis]
// ============================================================================

//! \internal
struct LivenessTarget {
  //! Previous target.
  LivenessTarget* prev;

  //! Target node.
  HLLabel* node;
  //! Jumped from.
  HLJump* from;
};

Error Context::livenessAnalysis() {
  uint32_t bLen = static_cast<uint32_t>(
    ((_contextVd.getLength() + BitArray::kEntityBits - 1) / BitArray::kEntityBits));

  // No variables.
  if (bLen == 0)
    return kErrorOk;

  HLFunc* func = getFunc();
  HLJump* from = nullptr;

  LivenessTarget* ltCur = nullptr;
  LivenessTarget* ltUnused = nullptr;

  PodList<HLNode*>::Link* retPtr = _returningList.getFirst();
  ASMJIT_ASSERT(retPtr != nullptr);

  HLNode* node = retPtr->getValue();

  size_t varMapToVaListOffset = _varMapToVaListOffset;
  BitArray* bCur = newBits(bLen);

  if (bCur == nullptr)
    goto _NoMemory;

  // Allocate bits for code visited first time.
_OnVisit:
  for (;;) {
    if (node->hasLiveness()) {
      if (bCur->_addBitsDelSource(node->getLiveness(), bCur, bLen))
        goto _OnPatch;
      else
        goto _OnDone;
    }

    BitArray* bTmp = copyBits(bCur, bLen);
    if (bTmp == nullptr)
      goto _NoMemory;

    node->setLiveness(bTmp);
    VarMap* map = node->getMap();

    if (map != nullptr) {
      uint32_t vaCount = map->getVaCount();
      VarAttr* vaList = reinterpret_cast<VarAttr*>(((uint8_t*)map) + varMapToVaListOffset);

      for (uint32_t i = 0; i < vaCount; i++) {
        VarAttr* va = &vaList[i];
        VarData* vd = va->getVd();

        uint32_t flags = va->getFlags();
        uint32_t localId = vd->getLocalId();

        if ((flags & kVarAttrWAll) && !(flags & kVarAttrRAll)) {
          // Write-Only.
          bTmp->setBit(localId);
          bCur->delBit(localId);
        }
        else {
          // Read-Only or Read/Write.
          bTmp->setBit(localId);
          bCur->setBit(localId);
        }
      }
    }

    if (node->getType() == HLNode::kTypeLabel)
      goto _OnTarget;

    if (node == func)
      goto _OnDone;

    ASMJIT_ASSERT(node->getPrev());
    node = node->getPrev();
  }

  // Patch already generated liveness bits.
_OnPatch:
  for (;;) {
    ASMJIT_ASSERT(node->hasLiveness());
    BitArray* bNode = node->getLiveness();

    if (!bNode->_addBitsDelSource(bCur, bLen))
      goto _OnDone;

    if (node->getType() == HLNode::kTypeLabel)
      goto _OnTarget;

    if (node == func)
      goto _OnDone;

    node = node->getPrev();
  }

_OnTarget:
  if (static_cast<HLLabel*>(node)->getNumRefs() != 0) {
    // Push a new LivenessTarget onto the stack if needed.
    if (ltCur == nullptr || ltCur->node != node) {
      // Allocate a new LivenessTarget object (from pool or zone).
      LivenessTarget* ltTmp = ltUnused;

      if (ltTmp != nullptr) {
        ltUnused = ltUnused->prev;
      }
      else {
        ltTmp = _zoneAllocator.allocT<LivenessTarget>(
          sizeof(LivenessTarget) - sizeof(BitArray) + bLen * sizeof(uintptr_t));

        if (ltTmp == nullptr)
          goto _NoMemory;
      }

      // Initialize and make current - ltTmp->from will be set later on.
      ltTmp->prev = ltCur;
      ltTmp->node = static_cast<HLLabel*>(node);
      ltCur = ltTmp;

      from = static_cast<HLLabel*>(node)->getFrom();
      ASMJIT_ASSERT(from != nullptr);
    }
    else {
      from = ltCur->from;
      goto _OnJumpNext;
    }

    // Visit/Patch.
    do {
      ltCur->from = from;
      bCur->copyBits(node->getLiveness(), bLen);

      if (!from->hasLiveness()) {
        node = from;
        goto _OnVisit;
      }

      // Issue #25: Moved '_OnJumpNext' here since it's important to patch
      // code again if there are more live variables than before.
_OnJumpNext:
      if (bCur->delBits(from->getLiveness(), bLen)) {
        node = from;
        goto _OnPatch;
      }

      from = from->getJumpNext();
    } while (from != nullptr);

    // Pop the current LivenessTarget from the stack.
    {
      LivenessTarget* ltTmp = ltCur;

      ltCur = ltCur->prev;
      ltTmp->prev = ltUnused;
      ltUnused = ltTmp;
    }
  }

  bCur->copyBits(node->getLiveness(), bLen);
  node = node->getPrev();

  if (node->isJmp() || !node->isFetched())
    goto _OnDone;

  if (!node->hasLiveness())
    goto _OnVisit;

  if (bCur->delBits(node->getLiveness(), bLen))
    goto _OnPatch;

_OnDone:
  if (ltCur != nullptr) {
    node = ltCur->node;
    from = ltCur->from;

    goto _OnJumpNext;
  }

  retPtr = retPtr->getNext();
  if (retPtr != nullptr) {
    node = retPtr->getValue();
    goto _OnVisit;
  }

  return kErrorOk;

_NoMemory:
  return setLastError(kErrorNoHeapMemory);
}

// ============================================================================
// [asmjit::Context - Annotate]
// ============================================================================

Error Context::formatInlineComment(StringBuilder& dst, HLNode* node) {
#if !defined(ASMJIT_DISABLE_LOGGER)
  if (node->getComment())
    dst.appendString(node->getComment());

  if (node->hasLiveness()) {
    if (dst.getLength() < _annotationLength)
      dst.appendChars(' ', _annotationLength - dst.getLength());

    uint32_t vdCount = static_cast<uint32_t>(_contextVd.getLength());
    size_t offset = dst.getLength() + 1;

    dst.appendChar('[');
    dst.appendChars(' ', vdCount);
    dst.appendChar(']');

    BitArray* liveness = node->getLiveness();
    VarMap* map = node->getMap();

    uint32_t i;
    for (i = 0; i < vdCount; i++) {
      if (liveness->getBit(i))
        dst.getData()[offset + i] = '.';
    }

    if (map != nullptr) {
      uint32_t vaCount = map->getVaCount();
      VarAttr* vaList = reinterpret_cast<VarAttr*>(((uint8_t*)map) + _varMapToVaListOffset);

      for (i = 0; i < vaCount; i++) {
        VarAttr* va = &vaList[i];
        VarData* vd = va->getVd();

        uint32_t flags = va->getFlags();
        char c = 'u';

        if ( (flags & kVarAttrRAll) && !(flags & kVarAttrWAll)) c = 'r';
        if (!(flags & kVarAttrRAll) &&  (flags & kVarAttrWAll)) c = 'w';
        if ( (flags & kVarAttrRAll) &&  (flags & kVarAttrWAll)) c = 'x';

        // Uppercase if unused.
        if ((flags & kVarAttrUnuse))
          c -= 'a' - 'A';

        ASMJIT_ASSERT(offset + vd->getLocalId() < dst.getLength());
        dst._data[offset + vd->getLocalId()] = c;
      }
    }
  }
#endif // !ASMJIT_DISABLE_LOGGER

  return kErrorOk;
}

// ============================================================================
// [asmjit::Context - Cleanup]
// ============================================================================

void Context::cleanup() {
  VarData** array = _contextVd.getData();
  size_t length = _contextVd.getLength();

  for (size_t i = 0; i < length; i++) {
    VarData* vd = array[i];
    vd->resetLocalId();
    vd->resetRegIndex();
  }

  _contextVd.reset(false);
  _extraBlock = nullptr;
}

// ============================================================================
// [asmjit::Context - CompileFunc]
// ============================================================================

Error Context::compile(HLFunc* func) {
  HLNode* end = func->getEnd();
  HLNode* stop = end->getNext();

  _func = func;
  _stop = stop;
  _extraBlock = end;

  ASMJIT_PROPAGATE_ERROR(fetch());
  ASMJIT_PROPAGATE_ERROR(removeUnreachableCode());
  ASMJIT_PROPAGATE_ERROR(livenessAnalysis());

  Compiler* compiler = getCompiler();

#if !defined(ASMJIT_DISABLE_LOGGER)
  if (compiler->getAssembler()->hasLogger())
    ASMJIT_PROPAGATE_ERROR(annotate());
#endif // !ASMJIT_DISABLE_LOGGER

  ASMJIT_PROPAGATE_ERROR(translate());

  // We alter the compiler cursor, because it doesn't make sense to reference
  // it after compilation - some nodes may disappear and it's forbidden to add
  // new code after the compilation is done.
  compiler->_setCursor(nullptr);

  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
