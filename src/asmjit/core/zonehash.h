// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_ZONEHASH_H
#define _ASMJIT_CORE_ZONEHASH_H

#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_zone
//! \{

// ============================================================================
// [asmjit::ZoneHashNode]
// ============================================================================

//! Node used by `ZoneHash<>` template.
//!
//! You must provide function `bool eq(const Key& key)` in order to make
//! `ZoneHash::get()` working.
class ZoneHashNode {
public:
  ASMJIT_NONCOPYABLE(ZoneHashNode)

  inline ZoneHashNode(uint32_t hashCode = 0) noexcept
    : _hashNext(nullptr),
      _hashCode(hashCode),
      _customData(0) {}

  //! Next node in the chain, null if it terminates the chain.
  ZoneHashNode* _hashNext;
  //! Precalculated hash-code of key.
  uint32_t _hashCode;
  //! Padding, can be reused by any Node that inherits `ZoneHashNode`.
  uint32_t _customData;
};

// ============================================================================
// [asmjit::ZoneHashBase]
// ============================================================================

class ZoneHashBase {
public:
  ASMJIT_NONCOPYABLE(ZoneHashBase)

  //! Count of records inserted into the hash table.
  size_t _size;
  //! Count of hash buckets.
  uint32_t _bucketsCount;
  //! When buckets array should grow.
  uint32_t _bucketsGrow;

  //! Buckets data.
  ZoneHashNode** _data;
  //! Embedded data, used by empty hash tables.
  ZoneHashNode* _embedded[1];

  //! \name Construction & Destruction
  //! \{

  inline ZoneHashBase() noexcept {
    _size = 0;
    _bucketsCount = 1;
    _bucketsGrow = 1;
    _data = _embedded;
    _embedded[0] = nullptr;
  }

  inline ZoneHashBase(ZoneHashBase&& other) noexcept {
    _size = other._size;
    _bucketsCount = other._bucketsCount;
    _bucketsGrow = other._bucketsGrow;
    _data = other._data;
    _embedded[0] = other._embedded[0];

    if (_data == other._embedded) _data = _embedded;
  }

  inline void reset() noexcept {
    _size = 0;
    _bucketsCount = 1;
    _bucketsGrow = 1;
    _data = _embedded;
    _embedded[0] = nullptr;
  }

  inline void release(ZoneAllocator* allocator) noexcept {
    ZoneHashNode** oldData = _data;
    if (oldData != _embedded)
      allocator->release(oldData, _bucketsCount * sizeof(ZoneHashNode*));
    reset();
  }

  //! \}

  //! \name Accessors
  //! \{

  inline bool empty() const noexcept { return _size == 0; }
  inline size_t size() const noexcept { return _size; }

  //! \}

  //! \name Utilities
  //! \{

  inline void _swap(ZoneHashBase& other) noexcept {
    std::swap(_size, other._size);
    std::swap(_bucketsCount, other._bucketsCount);
    std::swap(_bucketsGrow, other._bucketsGrow);
    std::swap(_data, other._data);
    std::swap(_embedded[0], other._embedded[0]);

    if (_data == other._embedded) _data = _embedded;
    if (other._data == _embedded) other._data = other._embedded;
  }

  //! \cond INTERNAL
  ASMJIT_API void _rehash(ZoneAllocator* allocator, uint32_t newCount) noexcept;
  ASMJIT_API ZoneHashNode* _insert(ZoneAllocator* allocator, ZoneHashNode* node) noexcept;
  ASMJIT_API ZoneHashNode* _remove(ZoneAllocator* allocator, ZoneHashNode* node) noexcept;
  //! \endcond

  //! \}
};

// ============================================================================
// [asmjit::ZoneHash]
// ============================================================================

//! Low-level hash table specialized for storing string keys and POD values.
//!
//! This hash table allows duplicates to be inserted (the API is so low
//! level that it's up to you if you allow it or not, as you should first
//! `get()` the node and then modify it or insert a new node by using `insert()`,
//! depending on the intention).
template<typename NodeT>
class ZoneHash : public ZoneHashBase {
public:
  ASMJIT_NONCOPYABLE(ZoneHash<NodeT>)

  typedef NodeT Node;

  //! \name Construction & Destruction
  //! \{

  inline ZoneHash() noexcept
    : ZoneHashBase() {}

  inline ZoneHash(ZoneHash&& other) noexcept
    : ZoneHash(other) {}

  //! \}

  //! \name Utilities
  //! \{

  inline void swap(ZoneHash& other) noexcept { ZoneHashBase::_swap(other); }

  template<typename KeyT>
  inline NodeT* get(const KeyT& key) const noexcept {
    uint32_t hMod = key.hashCode() % _bucketsCount;
    NodeT* node = static_cast<NodeT*>(_data[hMod]);

    while (node && !key.matches(node))
      node = static_cast<NodeT*>(node->_hashNext);
    return node;
  }

  inline NodeT* insert(ZoneAllocator* allocator, NodeT* node) noexcept { return static_cast<NodeT*>(_insert(allocator, node)); }
  inline NodeT* remove(ZoneAllocator* allocator, NodeT* node) noexcept { return static_cast<NodeT*>(_remove(allocator, node)); }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_ZONEHASH_H
