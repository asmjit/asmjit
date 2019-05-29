// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#define ASMJIT_EXPORTS

#include "../core/support.h"
#include "../core/zone.h"
#include "../core/zonehash.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ZoneHashBase - Helpers]
// ============================================================================

static uint32_t ZoneHash_getClosestPrime(uint32_t x) noexcept {
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
// [asmjit::ZoneHashBase - Rehash]
// ============================================================================

void ZoneHashBase::_rehash(ZoneAllocator* allocator, uint32_t newCount) noexcept {
  ZoneHashNode** oldData = _data;
  ZoneHashNode** newData = reinterpret_cast<ZoneHashNode**>(
    allocator->allocZeroed(size_t(newCount) * sizeof(ZoneHashNode*)));

  // We can still store nodes into the table, but it will degrade.
  if (ASMJIT_UNLIKELY(newData == nullptr))
    return;

  uint32_t i;
  uint32_t oldCount = _bucketsCount;

  for (i = 0; i < oldCount; i++) {
    ZoneHashNode* node = oldData[i];
    while (node) {
      ZoneHashNode* next = node->_hashNext;
      uint32_t hMod = node->_hashCode % newCount;

      node->_hashNext = newData[hMod];
      newData[hMod] = node;
      node = next;
    }
  }

  // 90% is the maximum occupancy, can't overflow since the maximum capacity
  // is limited to the last prime number stored in the prime table.
  if (oldData != _embedded)
    allocator->release(oldData, _bucketsCount * sizeof(ZoneHashNode*));

  _bucketsCount = newCount;
  _bucketsGrow = newCount * 9 / 10;
  _data = newData;
}

// ============================================================================
// [asmjit::ZoneHashBase - Ops]
// ============================================================================

ZoneHashNode* ZoneHashBase::_insert(ZoneAllocator* allocator, ZoneHashNode* node) noexcept {
  uint32_t hMod = node->_hashCode % _bucketsCount;
  ZoneHashNode* next = _data[hMod];

  node->_hashNext = next;
  _data[hMod] = node;

  if (++_size >= _bucketsGrow && next) {
    uint32_t newCapacity = ZoneHash_getClosestPrime(_bucketsCount);
    if (newCapacity != _bucketsCount)
      _rehash(allocator, newCapacity);
  }

  return node;
}

ZoneHashNode* ZoneHashBase::_remove(ZoneAllocator* allocator, ZoneHashNode* node) noexcept {
  ASMJIT_UNUSED(allocator);
  uint32_t hMod = node->_hashCode % _bucketsCount;

  ZoneHashNode** pPrev = &_data[hMod];
  ZoneHashNode* p = *pPrev;

  while (p) {
    if (p == node) {
      *pPrev = p->_hashNext;
      _size--;
      return node;
    }

    pPrev = &p->_hashNext;
    p = *pPrev;
  }

  return nullptr;
}

// ============================================================================
// [asmjit::ZoneHash - Unit]
// ============================================================================

#if defined(ASMJIT_TEST)
struct MyHashNode : public ZoneHashNode {
  inline MyHashNode(uint32_t key) noexcept
    : ZoneHashNode(key),
      _key(key) {}

  uint32_t _key;
};

struct MyKeyMatcher {
  inline MyKeyMatcher(uint32_t key) noexcept
    : _key(key) {}

  inline uint32_t hashCode() const noexcept { return _key; }
  inline bool matches(const MyHashNode* node) const noexcept { return node->_key == _key; }

  uint32_t _key;
};

UNIT(asmjit_zone_hash) {
  uint32_t kCount = BrokenAPI::hasArg("--quick") ? 1000 : 10000;

  Zone zone(4096);
  ZoneAllocator allocator(&zone);

  ZoneHash<MyHashNode> hashTable;

  uint32_t key;
  INFO("Inserting %u elements to HashTable", unsigned(kCount));
  for (key = 0; key < kCount; key++) {
    hashTable.insert(&allocator, zone.newT<MyHashNode>(key));
  }

  uint32_t count = kCount;
  INFO("Removing %u elements from HashTable and validating each operation", unsigned(kCount));
  do {
    MyHashNode* node;

    for (key = 0; key < count; key++) {
      node = hashTable.get(MyKeyMatcher(key));
      EXPECT(node != nullptr);
      EXPECT(node->_key == key);
    }

    {
      count--;
      node = hashTable.get(MyKeyMatcher(count));
      hashTable.remove(&allocator, node);

      node = hashTable.get(MyKeyMatcher(count));
      EXPECT(node == nullptr);
    }
  } while (count);

  EXPECT(hashTable.empty());
}
#endif

ASMJIT_END_NAMESPACE
