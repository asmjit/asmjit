// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONEHASH_H
#define _ASMJIT_CORE_ZONEHASH_H

// [Dependencies]
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
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

  ZoneHashNode* _hashNext;               //!< Next node in the chain, null if it terminates the chain.
  uint32_t _hashCode;                    //!< Precalculated hash-code of key.
  uint32_t _customData;                  //!< Padding, can be reused by any Node that inherits `ZoneHashNode`.
};

// ============================================================================
// [asmjit::ZoneHashBase]
// ============================================================================

class ZoneHashBase {
public:
  ASMJIT_NONCOPYABLE(ZoneHashBase)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneHashBase() noexcept {
    _size = 0;
    _bucketsCount = 1;
    _bucketsGrow = 1;
    _data = _embedded;
    _embedded[0] = nullptr;
  }

  inline ZoneHashBase(ZoneHashBase&& other) noexcept : _size{ other._size }, _bucketsCount{ other._bucketsCount }, _bucketsGrow{ other._bucketsGrow }, _data{ other._data } {
    _embedded[0] = other._embedded[0];

    other._size = 0;
    other._bucketsCount = 0;
    other._bucketsGrow = 0;
    other._data = nullptr;
    other._embedded[0] = nullptr;
  }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

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

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool empty() const noexcept { return _size == 0; }
  inline size_t size() const noexcept { return _size; }

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  ASMJIT_API void _rehash(ZoneAllocator* allocator, uint32_t newCount) noexcept;
  ASMJIT_API ZoneHashNode* _insert(ZoneAllocator* allocator, ZoneHashNode* node) noexcept;
  ASMJIT_API ZoneHashNode* _remove(ZoneAllocator* allocator, ZoneHashNode* node) noexcept;

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneHashBase& other) noexcept {
    std::swap(_size, other._size);
    std::swap(_bucketsCount, other._bucketsCount);
    std::swap(_bucketsGrow, other._bucketsGrow);
    std::swap(_data, other._data);
    std::swap(_embedded[0], other._embedded[0]);

    if (_data == other._embedded) _data = _embedded;
    if (other._data == _embedded) other._data = other._embedded;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  size_t _size;                          //!< Count of records inserted into the hash table.
  uint32_t _bucketsCount;                //!< Count of hash buckets.
  uint32_t _bucketsGrow;                 //!< When buckets array should grow.

  ZoneHashNode** _data;                  //!< Buckets data.
  ZoneHashNode* _embedded[1];            //!< Embedded data, used by empty hash tables.
};

// ============================================================================
// [asmjit::ZoneHash<NODE>]
// ============================================================================

//! Low-level hash table specialized for storing string keys and POD values.
//!
//! This hash table allows duplicates to be inserted (the API is so low
//! level that it's up to you if you allow it or not, as you should first
//! `get()` the node and then modify it or insert a new node by using `insert()`,
//! depending on the intention).
template<typename NODE>
class ZoneHash : public ZoneHashBase {
public:
  ASMJIT_NONCOPYABLE(ZoneHash<NODE>)

  typedef NODE Node;

  inline ZoneHash() noexcept
    : ZoneHashBase() {}

  inline ZoneHash(ZoneHash&& other) noexcept
    : ZoneHashBase(std::move(other)) {}

  template<typename KEY>
  inline NODE* get(const KEY& key) const noexcept {
    uint32_t hMod = key.hashCode() % _bucketsCount;
    NODE* node = static_cast<NODE*>(_data[hMod]);

    while (node && !key.matches(node))
      node = static_cast<NODE*>(node->_hashNext);
    return node;
  }

  inline NODE* insert(ZoneAllocator* allocator, NODE* node) noexcept { return static_cast<NODE*>(_insert(allocator, node)); }
  inline NODE* remove(ZoneAllocator* allocator, NODE* node) noexcept { return static_cast<NODE*>(_remove(allocator, node)); }

  inline void swapWith(ZoneHash& other) noexcept { ZoneHashBase::swapWith(other); }
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONEHASH_H
