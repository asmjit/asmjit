// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONELIST_H
#define _ASMJIT_CORE_ZONELIST_H

// [Dependencies]
#include "../core/intutils.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

// ============================================================================
// [asmjit::ZoneListNode]
// ============================================================================

template<typename NODE_T>
class ZoneListNode {
public:
  ASMJIT_NONCOPYABLE(ZoneListNode)

  inline ZoneListNode() noexcept
    : _listNodes { nullptr, nullptr } {}

  inline ZoneListNode(ZoneListNode&& other) noexcept
    : _listNodes{ other._listNodes[0], other._listNodes[1] }
  {
    other._listNodes[Globals::kLinkPrev] = nullptr;
    other._listNodes[Globals::kLinkNext] = nullptr;
  }

  inline bool hasPrev() const noexcept { return _listNodes[Globals::kLinkPrev] != nullptr; }
  inline bool hasNext() const noexcept { return _listNodes[Globals::kLinkNext] != nullptr; }

  inline NODE_T* prev() const noexcept { return _listNodes[Globals::kLinkPrev]; }
  inline NODE_T* next() const noexcept { return _listNodes[Globals::kLinkNext]; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  NODE_T* _listNodes[Globals::kLinkCount];
};

// ============================================================================
// [asmjit::ZoneList<T>]
// ============================================================================

template <typename NODE_T>
class ZoneList {
public:
  ASMJIT_NONCOPYABLE(ZoneList)

  inline ZoneList() noexcept
    : _bounds { nullptr, nullptr } {}

  inline ZoneList(ZoneList&& other) noexcept
    : _bounds { other._bounds[0], other._bounds[1] }
  {
    other.reset();
  }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  inline void reset() noexcept {
    _bounds[0] = nullptr;
    _bounds[1] = nullptr;
  }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool empty() const noexcept { return _bounds[0] == nullptr; }
  inline NODE_T* first() const noexcept { return _bounds[Globals::kLinkFirst]; }
  inline NODE_T* last() const noexcept { return _bounds[Globals::kLinkLast]; }

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  // Can be used to both prepend and append.
  inline void _addNode(NODE_T* node, size_t dir) noexcept {
    NODE_T* prev = _bounds[dir];

    node->_listNodes[!dir] = prev;
    _bounds[dir] = node;
    if (prev)
      prev->_listNodes[dir] = node;
    else
      _bounds[!dir] = node;
  }

  // Can be used to both prepend and append.
  inline void _insertNode(NODE_T* ref, NODE_T* node, size_t dir) noexcept {
    ASMJIT_ASSERT(ref != nullptr);

    NODE_T* prev = ref;
    NODE_T* next = ref->_listNodes[dir];

    prev->_listNodes[dir] = node;
    if (next)
      next->_listNodes[!dir] = node;
    else
      _bounds[dir] = node;

    node->_listNodes[!dir] = prev;
    node->_listNodes[ dir] = next;
  }

  inline void append(NODE_T* node) noexcept { _addNode(node, Globals::kLinkLast); }
  inline void prepend(NODE_T* node) noexcept { _addNode(node, Globals::kLinkFirst); }

  inline void insertAfter(NODE_T* ref, NODE_T* node) noexcept { _insertNode(ref, node, Globals::kLinkNext); }
  inline void insertBefore(NODE_T* ref, NODE_T* node) noexcept { _insertNode(ref, node, Globals::kLinkPrev); }

  inline NODE_T* unlink(NODE_T* node) noexcept {
    NODE_T* prev = node->prev();
    NODE_T* next = node->next();

    if (prev) { prev->_listNodes[Globals::kLinkNext] = next; node->_listNodes[0] = nullptr; } else { _bounds[Globals::kLinkFirst] = next; }
    if (next) { next->_listNodes[Globals::kLinkPrev] = prev; node->_listNodes[1] = nullptr; } else { _bounds[Globals::kLinkLast ] = prev; }

    node->_listNodes[0] = nullptr;
    node->_listNodes[1] = nullptr;

    return node;
  }

  inline NODE_T* popFirst() noexcept {
    NODE_T* node = _bounds[Globals::kLinkFirst];
    ASMJIT_ASSERT(node != nullptr);

    NODE_T* next = node->next();
    _bounds[Globals::kLinkFirst] = next;

    if (next) {
      next->_listNodes[Globals::kLinkPrev] = nullptr;
      node->_listNodes[Globals::kLinkNext] = nullptr;
    }
    else {
      _bounds[Globals::kLinkLast] = nullptr;
    }

    return node;
  }

  inline NODE_T* pop() noexcept {
    NODE_T* node = _bounds[Globals::kLinkLast];
    ASMJIT_ASSERT(node != nullptr);

    NODE_T* prev = node->prev();
    _bounds[Globals::kLinkLast] = prev;

    if (prev) {
      prev->_listNodes[Globals::kLinkNext] = nullptr;
      node->_listNodes[Globals::kLinkPrev] = nullptr;
    }
    else {
      _bounds[Globals::kLinkFirst] = nullptr;
    }

    return node;
  }

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneList& other) noexcept {
    std::swap(_bounds[0], other._bounds[0]);
    std::swap(_bounds[1], other._bounds[1]);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  NODE_T* _bounds[Globals::kLinkCount];
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONELIST_H
