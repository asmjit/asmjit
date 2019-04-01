// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_ZONELIST_H
#define _ASMJIT_CORE_ZONELIST_H

#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_zone
//! \{

// ============================================================================
// [asmjit::ZoneListNode]
// ============================================================================

template<typename NodeT>
class ZoneListNode {
public:
  ASMJIT_NONCOPYABLE(ZoneListNode)

  inline ZoneListNode() noexcept
    : _listNodes { nullptr, nullptr } {}

  inline ZoneListNode(ZoneListNode&& other) noexcept
    : _listNodes { other._listNodes[0], other._listNodes[1] } {}

  inline bool hasPrev() const noexcept { return _listNodes[Globals::kLinkPrev] != nullptr; }
  inline bool hasNext() const noexcept { return _listNodes[Globals::kLinkNext] != nullptr; }

  inline NodeT* prev() const noexcept { return _listNodes[Globals::kLinkPrev]; }
  inline NodeT* next() const noexcept { return _listNodes[Globals::kLinkNext]; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  NodeT* _listNodes[Globals::kLinkCount];
};

// ============================================================================
// [asmjit::ZoneList<T>]
// ============================================================================

template <typename NodeT>
class ZoneList {
public:
  ASMJIT_NONCOPYABLE(ZoneList)

  inline ZoneList() noexcept
    : _bounds { nullptr, nullptr } {}

  inline ZoneList(ZoneList&& other) noexcept
    : _bounds { other._bounds[0], other._bounds[1] } {}

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
  inline NodeT* first() const noexcept { return _bounds[Globals::kLinkFirst]; }
  inline NodeT* last() const noexcept { return _bounds[Globals::kLinkLast]; }

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  // Can be used to both prepend and append.
  inline void _addNode(NodeT* node, size_t dir) noexcept {
    NodeT* prev = _bounds[dir];

    node->_listNodes[!dir] = prev;
    _bounds[dir] = node;
    if (prev)
      prev->_listNodes[dir] = node;
    else
      _bounds[!dir] = node;
  }

  // Can be used to both prepend and append.
  inline void _insertNode(NodeT* ref, NodeT* node, size_t dir) noexcept {
    ASMJIT_ASSERT(ref != nullptr);

    NodeT* prev = ref;
    NodeT* next = ref->_listNodes[dir];

    prev->_listNodes[dir] = node;
    if (next)
      next->_listNodes[!dir] = node;
    else
      _bounds[dir] = node;

    node->_listNodes[!dir] = prev;
    node->_listNodes[ dir] = next;
  }

  inline void append(NodeT* node) noexcept { _addNode(node, Globals::kLinkLast); }
  inline void prepend(NodeT* node) noexcept { _addNode(node, Globals::kLinkFirst); }

  inline void insertAfter(NodeT* ref, NodeT* node) noexcept { _insertNode(ref, node, Globals::kLinkNext); }
  inline void insertBefore(NodeT* ref, NodeT* node) noexcept { _insertNode(ref, node, Globals::kLinkPrev); }

  inline NodeT* unlink(NodeT* node) noexcept {
    NodeT* prev = node->prev();
    NodeT* next = node->next();

    if (prev) { prev->_listNodes[1] = next; node->_listNodes[0] = nullptr; } else { _bounds[0] = next; }
    if (next) { next->_listNodes[0] = prev; node->_listNodes[1] = nullptr; } else { _bounds[1] = prev; }

    node->_listNodes[0] = nullptr;
    node->_listNodes[1] = nullptr;

    return node;
  }

  inline NodeT* popFirst() noexcept {
    NodeT* node = _bounds[0];
    ASMJIT_ASSERT(node != nullptr);

    NodeT* next = node->next();
    _bounds[0] = next;

    if (next) {
      next->_listNodes[0] = nullptr;
      node->_listNodes[1] = nullptr;
    }
    else {
      _bounds[1] = nullptr;
    }

    return node;
  }

  inline NodeT* pop() noexcept {
    NodeT* node = _bounds[1];
    ASMJIT_ASSERT(node != nullptr);

    NodeT* prev = node->prev();
    _bounds[1] = prev;

    if (prev) {
      prev->_listNodes[1] = nullptr;
      node->_listNodes[0] = nullptr;
    }
    else {
      _bounds[0] = nullptr;
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

  NodeT* _bounds[Globals::kLinkCount];
};

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_ZONELIST_H
