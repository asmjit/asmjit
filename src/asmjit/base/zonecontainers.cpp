// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../base/utils.h"
#include "../base/zonecontainers.h"
#include "../base/zoneheap.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::ZoneVectorBase - Reset]
// ============================================================================

//! Clear vector data and free internal buffer.
void ZoneVectorBase::_reset(size_t sizeOfT, ZoneHeap* heap) noexcept {
  if (_data) {
    ASMJIT_ASSERT(_heap != nullptr);
    _heap->release(_data, _capacity * sizeOfT);
  }

  _heap = heap;
  _length = 0;
  _capacity = 0;
  _data = nullptr;
}

// ============================================================================
// [asmjit::ZoneVectorBase - Helpers]
// ============================================================================

Error ZoneVectorBase::_grow(size_t sizeOfT, size_t n) noexcept {
  size_t threshold = kMemAllocGrowMax / sizeOfT;
  size_t capacity = _capacity;
  size_t after = _length;

  if (IntTraits<size_t>::maxValue() - n < after)
    return DebugUtils::errored(kErrorNoHeapMemory);

  after += n;
  if (capacity >= after) return kErrorOk;

  // ZoneVector is used as an array to hold short-lived data structures used
  // during code generation. The growing strategy is simple - use small capacity
  // at the beginning (very good for ZoneHeap) and then grow quicker to prevent
  // successive reallocations.
  if (capacity < 4)
    capacity = 4;
  else if (capacity < 8)
    capacity = 8;
  else if (capacity < 16)
    capacity = 16;
  else if (capacity < 64)
    capacity = 64;
  else if (capacity < 256)
    capacity = 256;

  while (capacity < after) {
    if (capacity < threshold)
      capacity *= 2;
    else
      capacity += threshold;
  }

  return _reserve(sizeOfT, capacity);
}

Error ZoneVectorBase::_reserve(size_t sizeOfT, size_t n) noexcept {
  size_t oldCapacity = _capacity;
  if (oldCapacity >= n) return kErrorOk;

  size_t nBytes = n * sizeOfT;
  if (nBytes < n) return DebugUtils::errored(kErrorNoHeapMemory);

  size_t allocatedBytes;
  uint8_t* newData = static_cast<uint8_t*>(_heap->alloc(nBytes, allocatedBytes));

  if (ASMJIT_UNLIKELY(!newData))
    return DebugUtils::errored(kErrorNoHeapMemory);

  void* oldData = _data;
  if (_length)
    ::memcpy(newData, oldData, _length * sizeOfT);

  if (oldData)
    _heap->release(oldData, oldCapacity * sizeOfT);

  _capacity = allocatedBytes / sizeOfT;
  ASMJIT_ASSERT(_capacity >= n);

  _data = newData;
  return kErrorOk;
}

Error ZoneVectorBase::_resize(size_t sizeOfT, size_t n) noexcept {
  size_t length = _length;
  if (_capacity < n) {
    ASMJIT_PROPAGATE(_grow(sizeOfT, n - length));
    ASMJIT_ASSERT(_capacity >= n);
  }

  if (length < n)
    ::memset(static_cast<uint8_t*>(_data) + length * sizeOfT, 0, (n - length) * sizeOfT);

  _length = n;
  return kErrorOk;
}

// ============================================================================
// [asmjit::ZoneHashBase - Utilities]
// ============================================================================

static uint32_t ZoneHashGetClosestPrime(uint32_t x) noexcept {
  static const uint32_t primeTable[] = {
    23, 53, 193, 389, 769, 1543, 3079, 6151, 12289, 24593
  };

  size_t i = 0;
  uint32_t p;

  do {
    if ((p = primeTable[i]) > x)
      break;
  } while (++i < ASMJIT_ARRAY_SIZE(primeTable));

  return p;
}

// ============================================================================
// [asmjit::ZoneHashBase - Reset]
// ============================================================================

void ZoneHashBase::reset(ZoneHeap* heap) noexcept {
  ZoneHashNode** oldData = _data;
  if (oldData != _embedded)
    _heap->release(oldData, _bucketsCount * sizeof(ZoneHashNode*));

  _heap = heap;
  _size = 0;
  _bucketsCount = 1;
  _bucketsGrow = 1;
  _data = _embedded;
  _embedded[0] = nullptr;
}

// ============================================================================
// [asmjit::ZoneHashBase - Rehash]
// ============================================================================

void ZoneHashBase::_rehash(uint32_t newCount) noexcept {
  ASMJIT_ASSERT(isInitialized());

  ZoneHashNode** oldData = _data;
  ZoneHashNode** newData = reinterpret_cast<ZoneHashNode**>(
    _heap->alloc(static_cast<size_t>(newCount) * sizeof(ZoneHashNode*)));

  // We can still store nodes into the table, but it will degrade.
  if (ASMJIT_UNLIKELY(newData == nullptr))
    return;

  uint32_t i;
  uint32_t oldCount = _bucketsCount;

  for (i = 0; i < oldCount; i++) {
    ZoneHashNode* node = oldData[i];
    while (node) {
      ZoneHashNode* next = node->_hashNext;
      uint32_t hMod = node->_hVal % newCount;

      node->_hashNext = newData[hMod];
      newData[hMod] = node;

      node = next;
    }
  }

  // 90% is the maximum occupancy, can't overflow since the maximum capacity
  // is limited to the last prime number stored in the prime table.
  if (oldData != _embedded)
    _heap->release(oldData, _bucketsCount * sizeof(ZoneHashNode*));

  _bucketsCount = newCount;
  _bucketsGrow = newCount * 9 / 10;

  _data = newData;
}

// ============================================================================
// [asmjit::ZoneHashBase - Ops]
// ============================================================================

ZoneHashNode* ZoneHashBase::_put(ZoneHashNode* node) noexcept {
  uint32_t hMod = node->_hVal % _bucketsCount;
  ZoneHashNode* next = _data[hMod];

  node->_hashNext = next;
  _data[hMod] = node;

  if (++_size >= _bucketsGrow && next) {
    uint32_t newCapacity = ZoneHashGetClosestPrime(_bucketsCount);
    if (newCapacity != _bucketsCount)
      _rehash(newCapacity);
  }

  return node;
}

ZoneHashNode* ZoneHashBase::_del(ZoneHashNode* node) noexcept {
  uint32_t hMod = node->_hVal % _bucketsCount;

  ZoneHashNode** pPrev = &_data[hMod];
  ZoneHashNode* p = *pPrev;

  while (p) {
    if (p == node) {
      *pPrev = p->_hashNext;
      return node;
    }

    pPrev = &p->_hashNext;
    p = *pPrev;
  }

  return nullptr;
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"
