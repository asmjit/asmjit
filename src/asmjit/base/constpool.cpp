// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/constpool.h"
#include "../base/intutil.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// Binary tree code is based on Julienne Walker's "Andersson Binary Trees"
// article and implementation. However, only three operations are implemented -
// get, insert and traverse.

// ============================================================================
// [asmjit::ConstPoolTree - Ops]
// ============================================================================

const ConstPoolNode ConstPoolTree::_sentinel = { {
  const_cast<ConstPoolNode*>(&ConstPoolTree::_sentinel),
  const_cast<ConstPoolNode*>(&ConstPoolTree::_sentinel)
}, 0, 0, 0 };

//! @internal
//!
//! @brief Remove left horizontal links.
static ASMJIT_INLINE ConstPoolNode* ConstPoolTree_skewNode(ConstPoolNode* node) {
  if (node->_link[0]->_level == node->_level && node->_level != 0 ) {
    ConstPoolNode *save = node->_link[0];
    node->_link[0] = save->_link[1];
    save->_link[1] = node;
    node = save;
  }

  return node;
}

//! @internal
//!
//! @brief Remove consecutive horizontal links.
static ASMJIT_INLINE ConstPoolNode* ConstPoolTree_splitNode(ConstPoolNode* node) {
  if (node->_link[1]->_link[1]->_level == node->_level && node->_level != 0) {
    ConstPoolNode *save = node->_link[1];
    node->_link[1] = save->_link[0];
    save->_link[0] = node;
    node = save;
    node->_level++;
  }

  return node;
}

ConstPoolNode* ConstPoolTree::get(const void* data) {
  ConstPoolNode* sentinel = const_cast<ConstPoolNode*>(&_sentinel);
  ConstPoolNode* node = _root;
  size_t dataSize = _dataSize;

  while (node != sentinel) {
    int c = ::memcmp(node->getData(), data, dataSize);
    if (c == 0)
      return node;
    node = node->_link[c < 0];
  }

  return NULL;
}

void ConstPoolTree::put(ConstPoolNode* newNode) {
  ConstPoolNode* sentinel = const_cast<ConstPoolNode*>(&_sentinel);
  size_t dataSize = _dataSize;

  _length++;
  if (_root == sentinel) {
    _root = newNode;
    return;
  }

  ConstPoolNode* node = _root;
  ConstPoolNode* stack[kHeightLimit];

  unsigned int top = 0;
  unsigned int dir;

  // Find a spot and save the stack.
  for (;;) {
    stack[top++] = node;
    dir = ::memcmp(node->getData(), newNode->getData(), dataSize) < 0;
    if (node->_link[dir] == sentinel)
      break;
    node = node->_link[dir];
  }

  // Link and rebalance.
  node->_link[dir] = newNode;

  while (top > 0) {
    // Which child?
    node = stack[--top];

    if (top != 0)
      dir = stack[top - 1]->_link[1] == node;

    node = ConstPoolTree_skewNode(node);
    node = ConstPoolTree_splitNode(node);

    // Fix the parent.
    if (top != 0)
      stack[top - 1]->_link[dir] = node;
    else
      _root = node;
  }
}

// ============================================================================
// [asmjit::ConstPool - Construction / Destruction]
// ============================================================================

ConstPool::ConstPool(Zone* zone) {
  _zone = zone;

  size_t dataSize = 1;
  for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_tree); i++) {
    _tree[i].setDataSize(dataSize);
    _gaps[i] = NULL;
    dataSize <<= 1;
  }

  _gapPool = NULL;
  _size = 0;
  _alignment = 0;
}

ConstPool::~ConstPool() {}

// ============================================================================
// [asmjit::ConstPool - Reset]
// ============================================================================

void ConstPool::reset() {
  for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_tree); i++) {
    _tree[i].reset();
    _gaps[i] = NULL;
  }

  _gapPool = NULL;
  _size = 0;
  _alignment = 0;
}

// ============================================================================
// [asmjit::ConstPool - Ops]
// ============================================================================

ASMJIT_INLINE size_t ConstPool_getGapIndex(size_t size) {
  if (size <=  1)
    return ConstPool::kIndex1;
  else if (size <=  3)
    return ConstPool::kIndex2;
  else if (size <=  7)
    return ConstPool::kIndex4;
  else if (size <= 15)
    return ConstPool::kIndex8;
  else
    return ConstPool::kIndex16;
}

ASMJIT_INLINE ConstPoolGap* ConstPool_allocGap(ConstPool* self) {
  ConstPoolGap* gap = self->_gapPool;
  if (gap == NULL)
    return self->_zone->allocT<ConstPoolGap>();

  self->_gapPool = gap->_next;
  return gap;
}

ASMJIT_INLINE void ConstPool_freeGap(ConstPool* self,  ConstPoolGap* gap) {
  gap->_next = self->_gapPool;
  self->_gapPool = gap;
}

static void ConstPool_addGap(ConstPool* self, size_t offset, size_t length) {
  ASMJIT_ASSERT(length > 0);

  while (length > 0) {
    size_t gapIndex;
    size_t gapLength;

    if (length >= 16 && IntUtil::isAligned<size_t>(offset, 16)) {
      gapIndex = ConstPool::kIndex16;
      gapLength = 16;
    }
    else if (length >= 8 && IntUtil::isAligned<size_t>(offset, 8)) {
      gapIndex = ConstPool::kIndex8;
      gapLength = 8;
    }
    else if (length >= 4 && IntUtil::isAligned<size_t>(offset, 4)) {
      gapIndex = ConstPool::kIndex4;
      gapLength = 4;
    }
    else if (length >= 2 && IntUtil::isAligned<size_t>(offset, 2)) {
      gapIndex = ConstPool::kIndex2;
      gapLength = 2;
    }
    else {
      gapIndex = ConstPool::kIndex1;
      gapLength = 1;
    }

    // We don't have to check for errors here, if this failed nothing really
    // happened (just the gap won't be visible) and it will fail again at
    // place where checking will cause kErrorNoHeapMemory.
    ConstPoolGap* gap = ConstPool_allocGap(self);
    if (gap == NULL)
      return;

    gap->_next = self->_gaps[gapIndex];
    self->_gaps[gapIndex] = gap;

    gap->_offset = offset;
    gap->_length = gapLength;

    offset += gapLength;
    length -= gapLength;
  }
}

Error ConstPool::add(const void* data, size_t size, size_t& dstOffset) {
  size_t treeIndex;

  if (size == 32)
    treeIndex = kIndex32;
  else if (size == 16)
    treeIndex = kIndex16;
  else if (size == 8)
    treeIndex = kIndex8;
  else if (size == 4)
    treeIndex = kIndex4;
  else if (size == 2)
    treeIndex = kIndex2;
  else if (size == 1)
    treeIndex = kIndex1;
  else
    return kErrorInvalidArgument;

  ConstPoolNode* node = _tree[treeIndex].get(data);
  if (node != NULL) {
    dstOffset = node->_offset;
    return kErrorOk;
  }

  // Before incrementing the current offset try if there is a gap that can
  // be used for the requested data.
  size_t offset = ~static_cast<size_t>(0);
  size_t gapIndex = treeIndex;

  while (gapIndex != kIndexCount - 1) {
    ConstPoolGap* gap = _gaps[treeIndex];

    // Check if there is a gap.
    if (gap != NULL) {
      size_t gapOffset = gap->_offset;
      size_t gapLength = gap->_length;

      // Destroy the gap for now.
      _gaps[treeIndex] = gap->_next;
      ConstPool_freeGap(this, gap);

      offset = gapOffset;
      ASMJIT_ASSERT(IntUtil::isAligned<size_t>(offset, size));

      gapLength -= size;
      if (gapLength > 0)
        ConstPool_addGap(this, gapOffset, gapLength);
    }

    gapIndex++;
  }

  if (offset == ~static_cast<size_t>(0)) {
    // Get how many bytes have to be skipped so the address is aligned accordingly
    // to the 'size'.
    size_t deltaTo = IntUtil::deltaTo<size_t>(_size, size);

    if (deltaTo != 0) {
      ConstPool_addGap(this, _size, deltaTo);
      _size += deltaTo;
    }

    offset = _size;
    _size += size;
  }

  // Add the initial node to the right index.
  node = ConstPoolTree::_newNode(_zone, data, size, offset, false);
  if (node == NULL)
    return kErrorNoHeapMemory;

  _tree[treeIndex].put(node);
  _alignment = IntUtil::iMax<size_t>(_alignment, size);

  dstOffset = offset;

  // Now create a bunch of shared constants that are based on the data pattern.
  // We stop at size 4, it probably doesn't make sense to split constants down
  // to 1 byte.
  size_t pCount = 1;
  while (size > 4) {
    size >>= 1;
    pCount <<= 1;

    ASMJIT_ASSERT(treeIndex != 0);
    treeIndex--;

    const uint8_t* pData = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < pCount; i++, pData += size) {
      node = _tree[treeIndex].get(pData);

      if (node != NULL)
        continue;

      node = ConstPoolTree::_newNode(_zone, pData, size, offset + (i * size), true);
      _tree[treeIndex].put(node);
    }
  }

  return kErrorOk;
}

// ============================================================================
// [asmjit::ConstPool - Reset]
// ============================================================================

struct ConstPoolFill {
  ASMJIT_INLINE ConstPoolFill(uint8_t* dst, size_t dataSize) :
    _dst(dst),
    _dataSize(dataSize) {}

  ASMJIT_INLINE void visit(const ConstPoolNode* node) {
    if (!node->_shared)
      ::memcpy(_dst + node->_offset, node->getData(), _dataSize);
  }

  uint8_t* _dst;
  size_t _dataSize;
};

void ConstPool::fill(void* dst) {
  // Clears possible gaps, asmjit should never emit garbage to the output.
  ::memset(dst, 0, _size);

  ConstPoolFill filler(static_cast<uint8_t*>(dst), 1);
  for (size_t i = 0; i < ASMJIT_ARRAY_SIZE(_tree); i++) {
    _tree[i].iterate(filler);
    filler._dataSize <<= 1;
  }
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
