// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_CONSTPOOL_H
#define _ASMJIT_CORE_CONSTPOOL_H

// [Dependencies]
#include "../core/zone.h"
#include "../core/zonerbtree.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::ConstPool]
// ============================================================================

//! Constant pool.
class ConstPool {
public:
  ASMJIT_NONCOPYABLE(ConstPool)

  enum {
    kIndex1 = 0,
    kIndex2 = 1,
    kIndex4 = 2,
    kIndex8 = 3,
    kIndex16 = 4,
    kIndex32 = 5,
    kIndexCount = 6
  };

  //! Zone-allocated const-pool gap.
  struct Gap {
    Gap* _next;                          //!< Pointer to the next gap
    size_t _offset;                      //!< Offset of the gap.
    size_t _length;                      //!< Remaining bytes of the gap (basically a gap size).
  };

  //! Zone-allocated const-pool node.
  class Node : public ZoneRBNodeT<Node> {
  public:
    ASMJIT_NONCOPYABLE(Node)

    inline Node(size_t offset, bool shared) noexcept
      : ZoneRBNodeT<Node>(),
        _shared(shared),
        _offset(uint32_t(offset)) {}

    inline void* getData() const noexcept {
      return static_cast<void*>(const_cast<ConstPool::Node*>(this) + 1);
    }

    uint32_t _shared : 1;                //!< If this constant is shared with another.
    uint32_t _offset;                    //!< Data offset from the beginning of the pool.
  };

  class Compare {
  public:
    inline Compare(size_t dataSize) noexcept
      : _dataSize(dataSize) {}

    inline int operator()(const Node& a, const Node& b) const noexcept {
      return std::memcmp(a.getData(), b.getData(), _dataSize);
    }

    inline int operator()(const Node& a, const void* data) const noexcept {
      return std::memcmp(a.getData(), data, _dataSize);
    }

    size_t _dataSize;
  };

  // --------------------------------------------------------------------------
  // [Tree]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Zone-allocated const-pool tree.
  struct Tree {
    // --------------------------------------------------------------------------
    // [Construction / Destruction]
    // --------------------------------------------------------------------------

    explicit inline Tree(size_t dataSize = 0) noexcept
      : _tree(),
        _length(0),
        _dataSize(dataSize) {}

    // --------------------------------------------------------------------------
    // [Reset]
    // --------------------------------------------------------------------------

    inline void reset() noexcept {
      _tree.reset();
      _length = 0;
    }

    // --------------------------------------------------------------------------
    // [Accessors]
    // --------------------------------------------------------------------------

    inline bool isEmpty() const noexcept { return _length == 0; }
    inline size_t getLength() const noexcept { return _length; }

    inline void setDataSize(size_t dataSize) noexcept {
      ASMJIT_ASSERT(isEmpty());
      _dataSize = dataSize;
    }

    // --------------------------------------------------------------------------
    // [Ops]
    // --------------------------------------------------------------------------

    inline Node* get(const void* data) noexcept {
      Compare cmp(_dataSize);
      return _tree.get(data, cmp);
    }

    inline void insert(Node* node) noexcept {
      Compare cmp(_dataSize);
      _tree.insert(node, cmp);
      _length++;
    }

    // --------------------------------------------------------------------------
    // [Iterate]
    // --------------------------------------------------------------------------

    template<typename Visitor>
    inline void iterate(Visitor& visitor) const noexcept {
      Node* node = _tree.getRoot();
      if (!node) return;

      static constexpr uint32_t kHeightLimit = 64;
      Node* stack[kHeightLimit];
      size_t top = 0;

      for (;;) {
        Node* left = node->getLeft();
        if (left != nullptr) {
          ASMJIT_ASSERT(top != kHeightLimit);
          stack[top++] = node;

          node = left;
          continue;
        }

Visit:
        visitor.visit(node);
        node = node->getRight();
        if (node != nullptr)
          continue;

        if (top == 0)
          return;

        node = stack[--top];
        goto Visit;
      }
    }

    // --------------------------------------------------------------------------
    // [Helpers]
    // --------------------------------------------------------------------------

    static inline Node* _newNode(Zone* zone, const void* data, size_t size, size_t offset, bool shared) noexcept {
      Node* node = zone->allocAlignedT<Node>(sizeof(Node) + size, sizeof(intptr_t));
      if (ASMJIT_UNLIKELY(!node)) return nullptr;

      node = new(node) Node(offset, shared);
      std::memcpy(node->getData(), data, size);
      return node;
    }

    // --------------------------------------------------------------------------
    // [Members]
    // --------------------------------------------------------------------------

    ZoneRBTree<Node> _tree;              //!< RB tree.
    size_t _length;                      //!< Length of the tree (count of nodes).
    size_t _dataSize;                    //!< Size of the data.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API ConstPool(Zone* zone) noexcept;
  ASMJIT_API ~ConstPool() noexcept;

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_API void reset(Zone* zone) noexcept;

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Get whether the constant-pool is empty.
  inline bool isEmpty() const noexcept { return _size == 0; }
  //! Get the size of the constant-pool in bytes.
  inline size_t getSize() const noexcept { return _size; }
  //! Get minimum alignment.
  inline size_t getAlignment() const noexcept { return _alignment; }

  //! Add a constant to the constant pool.
  //!
  //! The constant must have known size, which is 1, 2, 4, 8, 16 or 32 bytes.
  //! The constant is added to the pool only if it doesn't not exist, otherwise
  //! cached value is returned.
  //!
  //! AsmJit is able to subdivide added constants, so for example if you add
  //! 8-byte constant 0x1122334455667788 it will create the following slots:
  //!
  //!   8-byte: 0x1122334455667788
  //!   4-byte: 0x11223344, 0x55667788
  //!
  //! The reason is that when combining MMX/SSE/AVX code some patterns are used
  //! frequently. However, AsmJit is not able to reallocate a constant that has
  //! been already added. For example if you try to add 4-byte constant and then
  //! 8-byte constant having the same 4-byte pattern as the previous one, two
  //! independent slots will be generated by the pool.
  ASMJIT_API Error add(const void* data, size_t size, size_t& dstOffset) noexcept;

  // --------------------------------------------------------------------------
  // [Fill]
  // --------------------------------------------------------------------------

  //! Fill the destination with the constants from the pool.
  ASMJIT_API void fill(void* dst) const noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Zone* _zone;                           //!< Zone allocator.
  Tree _tree[kIndexCount];               //!< Tree per size.
  Gap* _gaps[kIndexCount];               //!< Gaps per size.
  Gap* _gapPool;                         //!< Gaps pool

  size_t _size;                          //!< Size of the pool (in bytes).
  size_t _alignment;                     //!< Required pool alignment.
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_CONSTPOOL_H
