// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_CONSTPOOL_H
#define _ASMJIT_BASE_CONSTPOOL_H

// [Dependencies - AsmJit]
#include "../base/zone.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::ConstPool]
// ============================================================================

//! Constant pool.
struct ConstPool {
  ASMJIT_NO_COPY(ConstPool)

  enum {
    kIndex1 = 0,
    kIndex2 = 1,
    kIndex4 = 2,
    kIndex8 = 3,
    kIndex16 = 4,
    kIndex32 = 5,
    kIndexCount = 6
  };

  // --------------------------------------------------------------------------
  // [Gap]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Zone-allocated const-pool gap.
  struct Gap {
    //! Link to the next gap
    Gap* _next;
    //! Offset of the gap.
    size_t _offset;
    //! Remaining bytes of the gap (basically a gap size).
    size_t _length;
  };

  // --------------------------------------------------------------------------
  // [Node]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Zone-allocated const-pool node.
  struct Node {
    // --------------------------------------------------------------------------
    // [Accessors]
    // --------------------------------------------------------------------------

    ASMJIT_INLINE void* getData() const noexcept {
      return static_cast<void*>(const_cast<ConstPool::Node*>(this) + 1);
    }

    // --------------------------------------------------------------------------
    // [Members]
    // --------------------------------------------------------------------------

    //! Left/Right nodes.
    Node* _link[2];
    //! Horizontal level for balance.
    uint32_t _level : 31;
    //! Whether this constant is shared with another.
    uint32_t _shared : 1;
    //! Data offset from the beginning of the pool.
    uint32_t _offset;
  };

  // --------------------------------------------------------------------------
  // [Tree]
  // --------------------------------------------------------------------------

  //! \internal
  //!
  //! Zone-allocated const-pool tree.
  struct Tree {
    enum {
      //! Maximum tree height == log2(1 << 64).
      kHeightLimit = 64
    };

    // --------------------------------------------------------------------------
    // [Construction / Destruction]
    // --------------------------------------------------------------------------

    ASMJIT_INLINE Tree(size_t dataSize = 0) noexcept
      : _root(nullptr),
        _length(0),
        _dataSize(dataSize) {}
    ASMJIT_INLINE ~Tree() {}

    // --------------------------------------------------------------------------
    // [Reset]
    // --------------------------------------------------------------------------

    ASMJIT_INLINE void reset() noexcept {
      _root = nullptr;
      _length = 0;
    }

    // --------------------------------------------------------------------------
    // [Accessors]
    // --------------------------------------------------------------------------

    ASMJIT_INLINE bool isEmpty() const noexcept { return _length == 0; }
    ASMJIT_INLINE size_t getLength() const noexcept { return _length; }

    ASMJIT_INLINE void setDataSize(size_t dataSize) noexcept {
      ASMJIT_ASSERT(isEmpty());
      _dataSize = dataSize;
    }

    // --------------------------------------------------------------------------
    // [Ops]
    // --------------------------------------------------------------------------

    ASMJIT_API Node* get(const void* data) noexcept;
    ASMJIT_API void put(Node* node) noexcept;

    // --------------------------------------------------------------------------
    // [Iterate]
    // --------------------------------------------------------------------------

    template<typename Visitor>
    ASMJIT_INLINE void iterate(Visitor& visitor) const noexcept {
      Node* node = const_cast<Node*>(_root);
      Node* link;

      Node* stack[kHeightLimit];

      if (node == nullptr)
        return;

      size_t top = 0;

      for (;;) {
        link = node->_link[0];

        if (link != nullptr) {
          ASMJIT_ASSERT(top != kHeightLimit);
          stack[top++] = node;

          node = link;
          continue;
        }

_Visit:
        visitor.visit(node);
        link = node->_link[1];

        if (link != nullptr) {
          node = link;
          continue;
        }

        if (top == 0)
          break;

        node = stack[--top];
        goto _Visit;
      }
    }

    // --------------------------------------------------------------------------
    // [Helpers]
    // --------------------------------------------------------------------------

    static ASMJIT_INLINE Node* _newNode(Zone* zone, const void* data, size_t size, size_t offset, bool shared) noexcept {
      Node* node = zone->allocT<Node>(sizeof(Node) + size);
      if (node == nullptr)
        return nullptr;

      node->_link[0] = nullptr;
      node->_link[1] = nullptr;
      node->_level = 1;
      node->_shared = shared;
      node->_offset = static_cast<uint32_t>(offset);

      ::memcpy(node->getData(), data, size);
      return node;
    }

    // --------------------------------------------------------------------------
    // [Members]
    // --------------------------------------------------------------------------

    //! Root of the tree
    Node* _root;
    //! Length of the tree (count of nodes).
    size_t _length;
    //! Size of the data.
    size_t _dataSize;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_API ConstPool(Zone* zone) noexcept;
  ASMJIT_API ~ConstPool() noexcept;

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  ASMJIT_API void reset() noexcept;

  // --------------------------------------------------------------------------
  // [Ops]
  // --------------------------------------------------------------------------

  //! Get whether the constant-pool is empty.
  ASMJIT_INLINE bool isEmpty() const noexcept { return _size == 0; }
  //! Get the size of the constant-pool in bytes.
  ASMJIT_INLINE size_t getSize() const noexcept { return _size; }
  //! Get minimum alignment.
  ASMJIT_INLINE size_t getAlignment() const noexcept { return _alignment; }

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

  //! Zone allocator.
  Zone* _zone;
  //! Tree per size.
  Tree _tree[kIndexCount];
  //! Gaps per size.
  Gap* _gaps[kIndexCount];
  //! Gaps pool
  Gap* _gapPool;

  //! Size of the pool (in bytes).
  size_t _size;
  //! Alignemnt.
  size_t _alignment;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_CONSTPOOL_H
