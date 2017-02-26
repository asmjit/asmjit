// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../core/build.h"
#ifndef ASMJIT_DISABLE_COMPILER

// [Dependencies]
#include "../core/algorithm.h"
#include "../core/intutils.h"
#include "../core/rastack_p.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::RAStackAllocator - Slots]
// ============================================================================

RAStackSlot* RAStackAllocator::newSlot(uint32_t baseRegId, uint32_t size, uint32_t alignment, uint32_t flags) noexcept {
  if (ASMJIT_UNLIKELY(_slots.willGrow(getAllocator(), 1) != kErrorOk))
    return nullptr;

  RAStackSlot* slot = getAllocator()->allocT<RAStackSlot>();
  if (ASMJIT_UNLIKELY(!slot))
    return nullptr;

  slot->_baseRegId = uint8_t(baseRegId);
  slot->_alignment = uint8_t(std::max<uint32_t>(alignment, 1));
  slot->_reserved[0] = 0;
  slot->_reserved[1] = 0;
  slot->_useCount = 0;
  slot->_size = size;
  slot->_flags = flags;

  slot->_weight = 0;
  slot->_offset = 0;

  _alignment = std::max<uint32_t>(_alignment, alignment);
  _slots.appendUnsafe(slot);
  return slot;
}

// ============================================================================
// [asmjit::RAStackAllocator - Utilities]
// ============================================================================

struct RAStackGap {
  inline RAStackGap() noexcept
    : offset(0),
      size(0) {}

  inline RAStackGap(uint32_t offset, uint32_t size) noexcept
    : offset(offset),
      size(size) {}

  inline RAStackGap(const RAStackGap& other) noexcept
    : offset(other.offset),
      size(other.size) {}

  uint32_t offset;
  uint32_t size;
};

Error RAStackAllocator::calculateStackFrame() noexcept {
  // Base weight added to all registers regardless of their size and alignment.
  uint32_t kBaseRegWeight = 16;

  uint32_t i;
  RAStackSlots& slots = getSlots();
  uint32_t numSlots = slots.getLength();

  // STEP 1:
  //
  // Update usage based on the size of the slot. We boost smaller slots in a way
  // that 32-bit register has higher priority than a 128-bit register, however,
  // if one 128-bit register is used 4 times more than some other 32-bit register
  // it will overweight it.
  for (i = 0; i < numSlots; i++) {
    RAStackSlot* slot = _slots[i];

    uint32_t alignment = slot->getAlignment();
    ASMJIT_ASSERT(alignment > 0);

    uint32_t power = IntUtils::ctz(alignment);
    uint64_t weight;

    if (slot->isRegHome())
      weight = kBaseRegWeight + (uint64_t(slot->getUseCount()) * (7 - power));
    else
      weight = power;

    // If overflown, which has less chance of winning a lottery, just use max
    // possible weight. In such case it probably doesn't matter at all.
    if (weight > 0xFFFFFFFFU)
      weight = 0xFFFFFFFFU;

    slot->setWeight(uint32_t(weight));
  }

  // STEP 2:
  //
  // Sort stack slots based on their newly calculated weight (in descending order).
  slots.sort([](const RAStackSlot* a, const RAStackSlot* b) noexcept {
    return a->getWeight() >  b->getWeight() ? 1 :
           a->getWeight() == b->getWeight() ? 0 : -1;
  });

  // STEP 3:
  //
  // Calculate offset of each slot. We start from the slot that has the highest
  // weight and advance to slots with lower weight. It could look that offsets
  // start from the first slot in our list and then simply increase, but it's
  // not always the case as we also try to fill all gaps introduced by the fact
  // that slots are sorted by weight and not by size & alignment, so when we need
  // to align some slot we distribute the gap caused by the alignment to `gaps`.
  uint32_t offset = 0;

  ZoneAllocator* allocator = getAllocator();
  ZoneVector<RAStackGap> gaps[kSizeCount - 1];

  for (i = 0; i < numSlots; i++) {
    RAStackSlot* slot = _slots[i];
    if (slot->isStackArg()) continue;

    uint32_t slotAlignment = slot->getAlignment();
    uint32_t alignedOffset = IntUtils::alignUp(offset, slotAlignment);

    // Try to find a slot within gaps first, before advancing the `offset`.
    bool foundGap = false;

    uint32_t gapSize = 0;
    uint32_t gapOffset = 0;

    // Try to find a slot within gaps first.
    {
      uint32_t slotSize = slot->getSize();
      if (slotSize < (1U << uint32_t(ASMJIT_ARRAY_SIZE(gaps)))) {
        // Iterate from the lowest to the highest possible.
        uint32_t index = IntUtils::ctz(slotSize);
        do {
          if (!gaps[index].isEmpty()) {
            RAStackGap gap = gaps[index].pop();

            ASMJIT_ASSERT(IntUtils::isAligned(gap.offset, slotAlignment));
            slot->setOffset(int32_t(gap.offset));

            gapSize = gap.size - slotSize;
            gapOffset = gap.offset - slotSize;

            foundGap = true;
            break;
          }
        } while (++index < uint32_t(ASMJIT_ARRAY_SIZE(gaps)));
      }
    }

    // No gap found, we may create a new one(s) if the current offset is not aligned.
    if (!foundGap && offset != alignedOffset) {
      gapSize = alignedOffset - offset;
      gapOffset = alignedOffset;

      offset = alignedOffset;
    }

    // True if we have found a gap and not filled all of it or we aligned the current offset.
    if (gapSize) {
      uint32_t slotSize = IntUtils::alignUpPowerOf2(gapSize + 1) / 2;
      do {
        if (gapSize >= slotSize) {
          gapSize -= slotSize;
          gapOffset -= slotSize;

          uint32_t index = IntUtils::ctz(slotSize);
          ASMJIT_PROPAGATE(gaps[index].append(allocator, RAStackGap(gapOffset, slotSize)));
        }
        slotSize >>= 1;
      } while (gapSize);
    }

    if (!foundGap) {
      ASMJIT_ASSERT(IntUtils::isAligned(offset, slotAlignment));
      slot->setOffset(int32_t(offset));
      offset += slot->getSize();
    }
  }

  _stackSize = IntUtils::alignUp(offset, _alignment);
  return kErrorOk;
}

Error RAStackAllocator::adjustSlotOffsets(int32_t offset) noexcept {
  uint32_t count = getSlotCount();
  for (uint32_t i = 0; i < count; i++) {
    RAStackSlot* slot = _slots[i];
    if (!slot->isStackArg())
      slot->_offset += offset;
  }
  return kErrorOk;
}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // !ASMJIT_DISABLE_COMPILER
