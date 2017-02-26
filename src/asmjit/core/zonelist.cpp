// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/zone.h"
#include "../core/zonelist.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ZoneList - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
class MyListNode : public ZoneListNode<MyListNode> {};

UNIT(core_zone_list) {
  Zone zone(4096);
  ZoneList<MyListNode> list;

  MyListNode* a = zone.newT<MyListNode>();
  MyListNode* b = zone.newT<MyListNode>();
  MyListNode* c = zone.newT<MyListNode>();
  MyListNode* d = zone.newT<MyListNode>();

  INFO("Append / Unlink");

  // []
  EXPECT(list.isEmpty() == true);

  // [A]
  list.append(a);
  EXPECT(list.isEmpty() == false);
  EXPECT(list.getFirst() == a);
  EXPECT(list.getLast() == a);
  EXPECT(a->getPrev() == nullptr);
  EXPECT(a->getNext() == nullptr);

  // [A, B]
  list.append(b);
  EXPECT(list.getFirst() == a);
  EXPECT(list.getLast() == b);
  EXPECT(a->getPrev() == nullptr);
  EXPECT(a->getNext() == b);
  EXPECT(b->getPrev() == a);
  EXPECT(b->getNext() == nullptr);

  // [A, B, C]
  list.append(c);
  EXPECT(list.getFirst() == a);
  EXPECT(list.getLast() == c);
  EXPECT(a->getPrev() == nullptr);
  EXPECT(a->getNext() == b);
  EXPECT(b->getPrev() == a);
  EXPECT(b->getNext() == c);
  EXPECT(c->getPrev() == b);
  EXPECT(c->getNext() == nullptr);

  // [B, C]
  list.unlink(a);
  EXPECT(list.getFirst() == b);
  EXPECT(list.getLast() == c);
  EXPECT(a->getPrev() == nullptr);
  EXPECT(a->getNext() == nullptr);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == c);
  EXPECT(c->getPrev() == b);
  EXPECT(c->getNext() == nullptr);

  // [B]
  list.unlink(c);
  EXPECT(list.getFirst() == b);
  EXPECT(list.getLast() == b);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == nullptr);
  EXPECT(c->getPrev() == nullptr);
  EXPECT(c->getNext() == nullptr);

  // []
  list.unlink(b);
  EXPECT(list.isEmpty() == true);
  EXPECT(list.getFirst() == nullptr);
  EXPECT(list.getLast() == nullptr);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == nullptr);

  INFO("Prepend / Unlink");

  // [A]
  list.prepend(a);
  EXPECT(list.isEmpty() == false);
  EXPECT(list.getFirst() == a);
  EXPECT(list.getLast() == a);
  EXPECT(a->getPrev() == nullptr);
  EXPECT(a->getNext() == nullptr);

  // [B, A]
  list.prepend(b);
  EXPECT(list.getFirst() == b);
  EXPECT(list.getLast() == a);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == a);
  EXPECT(a->getPrev() == b);
  EXPECT(a->getNext() == nullptr);

  INFO("InsertAfter / InsertBefore");

  // [B, A, C]
  list.insertAfter(a, c);
  EXPECT(list.getFirst() == b);
  EXPECT(list.getLast() == c);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == a);
  EXPECT(a->getPrev() == b);
  EXPECT(a->getNext() == c);
  EXPECT(c->getPrev() == a);
  EXPECT(c->getNext() == nullptr);

  // [B, D, A, C]
  list.insertBefore(a, d);
  EXPECT(list.getFirst() == b);
  EXPECT(list.getLast() == c);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == d);
  EXPECT(d->getPrev() == b);
  EXPECT(d->getNext() == a);
  EXPECT(a->getPrev() == d);
  EXPECT(a->getNext() == c);
  EXPECT(c->getPrev() == a);
  EXPECT(c->getNext() == nullptr);

  INFO("PopFirst / Pop");

  // [D, A, C]
  EXPECT(list.popFirst() == b);
  EXPECT(b->getPrev() == nullptr);
  EXPECT(b->getNext() == nullptr);

  EXPECT(list.getFirst() == d);
  EXPECT(list.getLast() == c);
  EXPECT(d->getPrev() == nullptr);
  EXPECT(d->getNext() == a);
  EXPECT(a->getPrev() == d);
  EXPECT(a->getNext() == c);
  EXPECT(c->getPrev() == a);
  EXPECT(c->getNext() == nullptr);

  // [D, A]
  EXPECT(list.pop() == c);
  EXPECT(c->getPrev() == nullptr);
  EXPECT(c->getNext() == nullptr);

  EXPECT(list.getFirst() == d);
  EXPECT(list.getLast() == a);
  EXPECT(d->getPrev() == nullptr);
  EXPECT(d->getNext() == a);
  EXPECT(a->getPrev() == d);
  EXPECT(a->getNext() == nullptr);
}
#endif

ASMJIT_END_NAMESPACE
