// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ZONERBTREE_H
#define _ASMJIT_CORE_ZONERBTREE_H

// [Dependencies]
#include "../core/algorithm.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::ZoneRBNode]
// ============================================================================

//! RB-Tree node.
//!
//! The color is stored as a least significant bit in the `left` node.
//!
//! NOTE: Always use accessors to access left and right children.
class ZoneRBNode {
public:
  ASMJIT_NONCOPYABLE(ZoneRBNode)

  static constexpr uintptr_t kRedMask = 0x1;
  static constexpr uintptr_t kPtrMask = ~kRedMask;

  inline ZoneRBNode() noexcept
    : _rbNodeData { 0, 0 } {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool hasChild(size_t i) const noexcept { return _rbNodeData[i] > kRedMask; }
  inline bool hasLeft() const noexcept { return _rbNodeData[0] > kRedMask; }
  inline bool hasRight() const noexcept { return _rbNodeData[1] != 0; }

  inline ZoneRBNode* _getChild(size_t i) const noexcept { return (ZoneRBNode*)(_rbNodeData[i] & kPtrMask); }
  inline ZoneRBNode* _getLeft() const noexcept { return (ZoneRBNode*)(_rbNodeData[0] & kPtrMask); }
  inline ZoneRBNode* _getRight() const noexcept { return (ZoneRBNode*)(_rbNodeData[1]); }

  inline void _setChild(size_t i, ZoneRBNode* node) noexcept { _rbNodeData[i] = (_rbNodeData[i] & kRedMask) | (uintptr_t)node; }
  inline void _setLeft(ZoneRBNode* node) noexcept { _rbNodeData[0] = (_rbNodeData[0] & kRedMask) | (uintptr_t)node; }
  inline void _setRight(ZoneRBNode* node) noexcept { _rbNodeData[1] = (uintptr_t)node; }

  template<typename T = ZoneRBNode>
  inline T* getChild(size_t i) const noexcept { return static_cast<T*>(_getChild(i)); }
  template<typename T = ZoneRBNode>
  inline T* getLeft() const noexcept { return static_cast<T*>(_getLeft()); }
  template<typename T = ZoneRBNode>
  inline T* getRight() const noexcept { return static_cast<T*>(_getRight()); }

  inline bool isRed() const noexcept { return static_cast<bool>(_rbNodeData[0] & kRedMask); }
  inline void _makeRed() noexcept { _rbNodeData[0] |= kRedMask; }
  inline void _makeBlack() noexcept { _rbNodeData[0] &= kPtrMask; }

  //! Get whether the node is RED (RED node must be non-null and must have RED flag set).
  static inline bool _isValidRed(ZoneRBNode* node) noexcept { return node && node->isRed(); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uintptr_t _rbNodeData[Globals::kLinkCount];
};

template<typename NODE_T>
class ZoneRBNodeT : public ZoneRBNode {
public:
  ASMJIT_NONCOPYABLE(ZoneRBNodeT)

  inline ZoneRBNodeT() noexcept
    : ZoneRBNode() {}

  inline NODE_T* getChild(size_t i) const noexcept { return static_cast<NODE_T*>(_getChild(i)); }
  inline NODE_T* getLeft() const noexcept { return static_cast<NODE_T*>(_getLeft()); }
  inline NODE_T* getRight() const noexcept { return static_cast<NODE_T*>(_getRight()); }
};

// ============================================================================
// [asmjit::ZoneRBTree]
// ============================================================================

template<typename NODE_T>
class ZoneRBTree {
public:
  ASMJIT_NONCOPYABLE(ZoneRBTree)

  typedef NODE_T Node;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ZoneRBTree() noexcept
    : _root(nullptr) {}

  inline ZoneRBTree(ZoneRBTree&& other) noexcept
    : _root(other._root) {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool isEmpty() const noexcept { return _root == nullptr; }
  inline NODE_T* getRoot() const noexcept { return static_cast<NODE_T*>(_root); }

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  inline void reset() noexcept { _root = nullptr; }

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  template<typename CMP = Algorithm::Compare<Algorithm::kOrderAscending>>
  void insert(NODE_T* node, const CMP& cmp = CMP()) noexcept {
    // Node to insert must not contain garbage.
    ASMJIT_ASSERT(!node->hasLeft());
    ASMJIT_ASSERT(!node->hasRight());
    ASMJIT_ASSERT(!node->isRed());

    if (!_root) {
      _root = node;
      return;
    }

    ZoneRBNode head;           // False root node,
    head._setRight(_root);     // having root on the right.

    ZoneRBNode* g = nullptr;   // Grandparent.
    ZoneRBNode* p = nullptr;   // Parent.
    ZoneRBNode* t = &head;     // Iterator.
    ZoneRBNode* q = _root;     // Query.

    size_t dir = 0;            // Direction for accessing child nodes.
    size_t last = 0;           // Not needed to initialize, but makes some tools happy.

    node->_makeRed();          // New nodes are always red and violations fixed appropriately.

    // Search down the tree.
    for (;;) {
      if (!q) {
        // Insert new node at the bottom.
        q = node;
        p->_setChild(dir, node);
      }
      else if (_isValidRed(q->_getLeft()) && _isValidRed(q->_getRight())) {
        // Color flip.
        q->_makeRed();
        q->_getLeft()->_makeBlack();
        q->_getRight()->_makeBlack();
      }

      // Fix red violation.
      if (_isValidRed(q) && _isValidRed(p))
        t->_setChild(t->_getRight() == g,
                     q == p->_getChild(last) ? _singleRotate(g, !last) : _doubleRotate(g, !last));

      // Stop if found.
      if (q == node)
        break;

      last = dir;
      dir = cmp(*static_cast<NODE_T*>(q), *static_cast<NODE_T*>(node)) < 0;

      // Update helpers.
      if (g) t = g;

      g = p;
      p = q;
      q = q->_getChild(dir);
    }

    // Update root and make it black.
    _root = static_cast<NODE_T*>(head._getRight());
    _root->_makeBlack();
  }

  //! Remove node from RBTree.
  template<typename CMP = Algorithm::Compare<Algorithm::kOrderAscending>>
  void remove(ZoneRBNode* node, const CMP& cmp = CMP()) noexcept {
    ZoneRBNode head;           // False root node,
    head._setRight(_root);     // having root on the right.

    ZoneRBNode* g = nullptr;   // Grandparent.
    ZoneRBNode* p = nullptr;   // Parent.
    ZoneRBNode* q = &head;     // Query.

    ZoneRBNode* f  = nullptr;  // Found item.
    ZoneRBNode* gf = nullptr;  // Found grandparent.
    size_t dir = 1;            // Direction (0 or 1).

    // Search and push a red down.
    while (q->hasChild(dir)) {
      size_t last = dir;

      // Update helpers.
      g = p;
      p = q;
      q = q->_getChild(dir);
      dir = cmp(*static_cast<NODE_T*>(q), *static_cast<NODE_T*>(node)) < 0;

      // Save found node.
      if (q == node) {
        f = q;
        gf = g;
      }

      // Push the red node down.
      if (!_isValidRed(q) && !_isValidRed(q->_getChild(dir))) {
        if (_isValidRed(q->_getChild(!dir))) {
          ZoneRBNode* child = _singleRotate(q, dir);
          p->_setChild(last, child);
          p = child;
        }
        else if (!_isValidRed(q->_getChild(!dir)) && p->_getChild(!last)) {
          ZoneRBNode* s = p->_getChild(!last);
          if (!_isValidRed(s->_getChild(!last)) && !_isValidRed(s->_getChild(last))) {
            // Color flip.
            p->_makeBlack();
            s->_makeRed();
            q->_makeRed();
          }
          else {
            size_t dir2 = g->_getRight() == p;
            ZoneRBNode* child = g->_getChild(dir2);

            if (_isValidRed(s->_getChild(last))) {
              child = _doubleRotate(p, last);
              g->_setChild(dir2, child);
            }
            else if (_isValidRed(s->_getChild(!last))) {
              child = _singleRotate(p, last);
              g->_setChild(dir2, child);
            }

            // Ensure correct coloring.
            q->_makeRed();
            child->_makeRed();
            child->_getLeft()->_makeBlack();
            child->_getRight()->_makeBlack();
          }
        }
      }
    }

    // Replace and remove.
    ASMJIT_ASSERT(f != nullptr);
    ASMJIT_ASSERT(f != &head);
    ASMJIT_ASSERT(q != &head);

    p->_setChild(p->_getRight() == q,
                 q->_getChild(q->_getLeft() == nullptr));

    // NOTE: The original algorithm used a trick to just copy 'key/value' to
    // `f` and mark `q` for deletion. But this is unacceptable here as we
    // really want to destroy the passed `node`. So, we really have to make
    // sure that we really removed `f` and not `q` from the tree.
    if (f != q) {
      ASMJIT_ASSERT(f != &head);
      ASMJIT_ASSERT(f != gf);

      ZoneRBNode* n = gf ? gf : &head;
      dir = (n == &head) ? 1  : cmp(*static_cast<NODE_T*>(n), *static_cast<NODE_T*>(node)) < 0;

      for (;;) {
        if (n->_getChild(dir) == f) {
          n->_setChild(dir, q);
          // RAW copy, including the color.
          q->_rbNodeData[0] = f->_rbNodeData[0];
          q->_rbNodeData[1] = f->_rbNodeData[1];
          break;
        }

        n = n->_getChild(dir);

        // Cannot be true as we know that it must reach `f` in few iterations.
        ASMJIT_ASSERT(n != nullptr);
        dir = cmp(*static_cast<NODE_T*>(n), *static_cast<NODE_T*>(node)) < 0;
      }
    }

    // Update root and make it black.
    _root = static_cast<NODE_T*>(head._getRight());
    if (_root) _root->_makeBlack();
  }

  template<typename KEY, typename CMP = Algorithm::Compare<Algorithm::kOrderAscending>>
  ASMJIT_FORCEINLINE NODE_T* get(const KEY& key, const CMP& cmp = CMP()) const noexcept {
    ZoneRBNode* node = _root;
    while (node) {
      auto result = cmp(*static_cast<const NODE_T*>(node), key);
      if (result == 0) break;

      // Go left or right depending on the `result`.
      node = node->_getChild(result < 0);
    }
    return static_cast<NODE_T*>(node);
  }

  // --------------------------------------------------------------------------
  // [Swap]
  // --------------------------------------------------------------------------

  inline void swapWith(ZoneRBTree& other) noexcept {
    std::swap(_root, other._root);
  }

  // --------------------------------------------------------------------------
  // [Internal]
  // --------------------------------------------------------------------------

  static inline bool _isValidRed(ZoneRBNode* node) noexcept { return ZoneRBNode::_isValidRed(node); }

  //! Single rotation.
  static ASMJIT_FORCEINLINE ZoneRBNode* _singleRotate(ZoneRBNode* root, size_t dir) noexcept {
    ZoneRBNode* save = root->_getChild(!dir);
    root->_setChild(!dir, save->_getChild(dir));
    save->_setChild( dir, root);
    root->_makeRed();
    save->_makeBlack();
    return save;
  }

  //! Double rotation.
  static ASMJIT_FORCEINLINE ZoneRBNode* _doubleRotate(ZoneRBNode* root, size_t dir) noexcept {
    root->_setChild(!dir, _singleRotate(root->_getChild(!dir), !dir));
    return _singleRotate(root, dir);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  NODE_T* _root;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ZONERBTREE_H
