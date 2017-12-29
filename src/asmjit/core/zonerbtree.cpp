// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/intutils.h"
#include "../core/zone.h"
#include "../core/zonerbtree.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ZoneRBTree - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
template<typename NODE>
struct ZoneRBUnit {
  typedef ZoneRBTree<NODE> Tree;

  static void verifyTree(Tree& tree) noexcept {
    EXPECT(checkHeight(static_cast<NODE*>(tree._root)) > 0);
  }

  // Check whether the Red-Black tree is valid.
  static int checkHeight(NODE* node) noexcept {
    if (!node) return 1;

    NODE* ln = node->left();
    NODE* rn = node->right();

    // Invalid tree.
    EXPECT(ln == nullptr || *ln < *node);
    EXPECT(rn == nullptr || *rn > *node);

    // Red violation.
    EXPECT(!node->isRed() ||
          (!ZoneRBNode::_isValidRed(ln) && !ZoneRBNode::_isValidRed(rn)));

    // Black violation.
    int lh = checkHeight(ln);
    int rh = checkHeight(rn);
    EXPECT(!lh || !rh || lh == rh);

    // Only count black links.
    return (lh && rh) ? lh + !node->isRed() : 0;
  }
};

class MyRBNode : public ZoneRBNodeT<MyRBNode> {
public:
  ASMJIT_NONCOPYABLE(MyRBNode)

  explicit inline MyRBNode(uint32_t key) noexcept
    : _key(key) {}

  inline bool operator<(const MyRBNode& other) const noexcept { return _key < other._key; }
  inline bool operator>(const MyRBNode& other) const noexcept { return _key > other._key; }

  inline bool operator<(uint32_t queryKey) const noexcept { return _key < queryKey; }
  inline bool operator>(uint32_t queryKey) const noexcept { return _key > queryKey; }

  uint32_t _key;
};

UNIT(core_zone_rbtree) {
  constexpr uint32_t kCount = 10000;

  Zone zone(4096);
  ZoneRBTree<MyRBNode> rbTree;

  uint32_t key;
  INFO("Inserting %u elements to RBTree and validating each operation", unsigned(kCount));
  for (key = 0; key < kCount; key++) {
    rbTree.insert(zone.newT<MyRBNode>(key));
    ZoneRBUnit<MyRBNode>::verifyTree(rbTree);
  }

  uint32_t count = kCount;
  INFO("Removing %u elements from RBTree and validating each operation", unsigned(kCount));
  do {
    MyRBNode* node;

    for (key = 0; key < count; key++) {
      node = rbTree.get(key);
      EXPECT(node != nullptr);
      EXPECT(node->_key == key);
    }

    node = rbTree.get(--count);
    rbTree.remove(node);
    ZoneRBUnit<MyRBNode>::verifyTree(rbTree);
  } while (count);

  EXPECT(rbTree.empty());
}
#endif

ASMJIT_END_NAMESPACE
