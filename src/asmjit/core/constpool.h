// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_CONSTPOOL_H_INCLUDED
#define ASMJIT_CORE_CONSTPOOL_H_INCLUDED

#include "../core/support.h"
#include "../core/zone.h"
#include "../core/zonetree.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_utilities
//! \{

//! Constant pool scope.
enum class ConstPoolScope : uint32_t {
  //! Local constant, always embedded right after the current function.
  kLocal = 0,
  //! Global constant, embedded at the end of the currently compiled code.
  kGlobal = 1,

  //! Maximum value of `ConstPoolScope`.
  kMaxValue = kGlobal
};

//! Constant pool.
//!
//! Constant pool is designed to hold 1, 2, 4, 8, 16, 32, and 64 byte constants. It's not designed to hold constants
//! having arbitrary length like strings and arrays.
class ConstPool {
public:
  ASMJIT_NONCOPYABLE(ConstPool)

  //! \cond INTERNAL

  //! Index of a given size in const-pool table.
  enum Index : uint32_t {
    kIndex1 = 0,
    kIndex2 = 1,
    kIndex4 = 2,
    kIndex8 = 3,
    kIndex16 = 4,
    kIndex32 = 5,
    kIndex64 = 6,
    kIndexCount = 7
  };

  //! Zone-allocated const-pool gap created by two differently aligned constants.
  struct Gap {
    //! Pointer to the next gap
    Gap* _next;
    //! Offset of the gap.
    size_t _offset;
    //! Remaining bytes of the gap (basically a gap size).
    size_t _size;
  };

  //! Zone-allocated const-pool node.
  class Node : public ZoneTreeNodeT<Node> {
  public:
    ASMJIT_NONCOPYABLE(Node)

    //! If this constant is shared with another.
    uint32_t _shared : 1;
    //! Data offset from the beginning of the pool.
    uint32_t _offset;

    ASMJIT_INLINE_NODEBUG Node(size_t offset, bool shared) noexcept
      : ZoneTreeNodeT<Node>(),
        _shared(shared),
        _offset(uint32_t(offset)) {}

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG void* data() noexcept { return Support::offsetPtr<void>(this, sizeof(*this)); }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG const void* data() const noexcept { return Support::offsetPtr<void>(this, sizeof(*this)); }
  };

  //! Data comparer used internally.
  class Compare {
  public:
    size_t _dataSize;

    ASMJIT_INLINE_NODEBUG Compare(size_t dataSize) noexcept
      : _dataSize(dataSize) {}

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG int operator()(const Node& a, const Node& b) const noexcept {
      return ::memcmp(a.data(), b.data(), _dataSize);
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG int operator()(const Node& a, const void* data) const noexcept {
      return ::memcmp(a.data(), data, _dataSize);
    }
  };

  //! Zone-allocated const-pool tree.
  struct Tree {
    //! RB tree.
    ZoneTree<Node> _tree;
    //! Size of the tree (number of nodes).
    size_t _size;
    //! Size of the data.
    size_t _dataSize;

    ASMJIT_INLINE_NODEBUG explicit Tree(size_t dataSize = 0) noexcept
      : _tree(),
        _size(0),
        _dataSize(dataSize) {}

    ASMJIT_INLINE_NODEBUG void reset() noexcept {
      _tree.reset();
      _size = 0;
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG bool empty() const noexcept { return _size == 0; }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

    inline void setDataSize(size_t dataSize) noexcept {
      ASMJIT_ASSERT(empty());
      _dataSize = dataSize;
    }

    [[nodiscard]]
    ASMJIT_INLINE_NODEBUG Node* get(const void* data) noexcept {
      Compare cmp(_dataSize);
      return _tree.get(data, cmp);
    }

    ASMJIT_INLINE_NODEBUG void insert(Node* node) noexcept {
      Compare cmp(_dataSize);
      _tree.insert(node, cmp);
      _size++;
    }

    template<typename Visitor>
    inline void forEach(Visitor& visitor) const noexcept {
      Node* node = _tree.root();
      if (!node) return;

      Node* stack[Globals::kMaxTreeHeight];
      size_t top = 0;

      for (;;) {
        Node* left = node->left();
        if (left != nullptr) {
          ASMJIT_ASSERT(top != Globals::kMaxTreeHeight);
          stack[top++] = node;

          node = left;
          continue;
        }

        for (;;) {
          visitor(node);
          node = node->right();

          if (node != nullptr)
            break;

          if (top == 0)
            return;

          node = stack[--top];
        }
      }
    }

    [[nodiscard]]
    static inline Node* _newNode(Zone* zone, const void* data, size_t size, size_t offset, bool shared) noexcept {
      size_t nodeSize = Support::alignUp(sizeof(Node) + size, Globals::kZoneAlignment);
      Node* node = zone->alloc<Node>(nodeSize);

      if (ASMJIT_UNLIKELY(!node)) {
        return nullptr;
      }

      node = new(Support::PlacementNew{node}) Node(offset, shared);
      memcpy(node->data(), data, size);
      return node;
    }
  };

  //! \endcond

  //! \name Members
  //! \{

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
  //! Required pool alignment.
  size_t _alignment;
  //! Minimum item size in the pool.
  size_t _minItemSize;

  //! \}

  //! \name Construction & Destruction
  //! \{

  //! Creates a new constant pool that would use `zone` as a memory allocator.
  ASMJIT_API explicit ConstPool(Zone* zone) noexcept;
  //! Destroys this constant pool.
  ASMJIT_API ~ConstPool() noexcept;

  //! \}

  //! \name Reset
  //! \{

  //! Resets this constant pool and its allocator to `zone`.
  ASMJIT_API void reset(Zone* zone) noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Tests whether the constant-pool is empty.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG bool empty() const noexcept { return _size == 0; }

  //! Returns the size of the constant-pool in bytes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t size() const noexcept { return _size; }

  //! Returns minimum alignment.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t alignment() const noexcept { return _alignment; }

  //! Returns the minimum size of all items added to the constant pool.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG size_t minItemSize() const noexcept { return _minItemSize; }

  //! \}

  //! \name Utilities
  //! \{

  //! Adds a constant to the constant pool.
  //!
  //! The constant must have known size, which is 1, 2, 4, 8, 16 or 32 bytes. The constant is added to the pool only
  //! if it doesn't not exist, otherwise cached value is returned.
  //!
  //! AsmJit is able to subdivide added constants, so for example if you add 8-byte constant 0x1122334455667788 it
  //! will create the following slots:
  //!
  //!   8-byte: 0x1122334455667788
  //!   4-byte: 0x11223344, 0x55667788
  //!
  //! The reason is that when combining MMX/SSE/AVX code some patterns are used frequently. However, AsmJit is not
  //! able to reallocate a constant that has been already added. For example if you try to add 4-byte constant and
  //! then 8-byte constant having the same 4-byte pattern as the previous one, two independent slots will be used.
  ASMJIT_API Error add(const void* data, size_t size, size_t& dstOffset) noexcept;

  //! Fills the destination with the content of this constant pool.
  ASMJIT_API void fill(void* dst) const noexcept;
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CONSTPOOL_H_INCLUDED
