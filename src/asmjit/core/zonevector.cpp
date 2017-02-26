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
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::ZoneVectorBase - Helpers]
// ============================================================================

Error ZoneVectorBase::_grow(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept {
  uint32_t threshold = Globals::kAllocThreshold / sizeOfT;
  uint32_t capacity = _capacity;
  uint32_t after = _length;

  if (ASMJIT_UNLIKELY(std::numeric_limits<uint32_t>::max() - n < after))
    return DebugUtils::errored(kErrorNoHeapMemory);

  after += n;
  if (capacity >= after)
    return kErrorOk;

  // ZoneVector is used as an array to hold short-lived data structures used
  // during code generation. The growing strategy is simple - use small capacity
  // at the beginning (very good for ZoneAllocator) and then grow quicker to
  // prevent successive reallocations.
  if (capacity < 4)
    capacity = 4;
  else if (capacity < 8)
    capacity = 8;
  else if (capacity < 16)
    capacity = 16;
  else if (capacity < 64)
    capacity = 64;
  else if (capacity < 256)
    capacity = 256;

  while (capacity < after) {
    if (capacity < threshold)
      capacity *= 2;
    else
      capacity += threshold;
  }

  return _reserve(allocator, sizeOfT, capacity);
}

Error ZoneVectorBase::_reserve(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept {
  uint32_t oldCapacity = _capacity;
  if (oldCapacity >= n) return kErrorOk;

  uint32_t nBytes = n * sizeOfT;
  if (ASMJIT_UNLIKELY(nBytes < n))
    return DebugUtils::errored(kErrorNoHeapMemory);

  size_t allocatedBytes;
  uint8_t* newData = static_cast<uint8_t*>(allocator->alloc(nBytes, allocatedBytes));

  if (ASMJIT_UNLIKELY(!newData))
    return DebugUtils::errored(kErrorNoHeapMemory);

  void* oldData = _data;
  if (_length)
    std::memcpy(newData, oldData, size_t(_length) * sizeOfT);

  if (oldData)
    allocator->release(oldData, size_t(oldCapacity) * sizeOfT);

  _capacity = uint32_t(allocatedBytes / sizeOfT);
  ASMJIT_ASSERT(_capacity >= n);

  _data = newData;
  return kErrorOk;
}

Error ZoneVectorBase::_resize(ZoneAllocator* allocator, uint32_t sizeOfT, uint32_t n) noexcept {
  uint32_t length = _length;

  if (_capacity < n) {
    ASMJIT_PROPAGATE(_grow(allocator, sizeOfT, n - length));
    ASMJIT_ASSERT(_capacity >= n);
  }

  if (length < n)
    std::memset(static_cast<uint8_t*>(_data) + size_t(length) * sizeOfT, 0, size_t(n - length) * sizeOfT);

  _length = n;
  return kErrorOk;
}

// ============================================================================
// [asmjit::ZoneBitVector - Ops]
// ============================================================================

Error ZoneBitVector::copyFrom(ZoneAllocator* allocator, const ZoneBitVector& other) noexcept {
  BitWord* data = _data;
  uint32_t newLength = other.getLength();

  if (!newLength) {
    _length = 0;
    return kErrorOk;
  }

  if (newLength > _capacity) {
    // Realloc needed... Calculate the minimum capacity (in bytes) requied.
    uint32_t minimumCapacityInBits = IntUtils::alignUp<uint32_t>(newLength, kBitWordSize);
    if (ASMJIT_UNLIKELY(minimumCapacityInBits < newLength))
      return DebugUtils::errored(kErrorNoHeapMemory);

    // Normalize to bytes.
    uint32_t minimumCapacity = minimumCapacityInBits / 8;
    size_t allocatedCapacity;

    BitWord* newData = static_cast<BitWord*>(allocator->alloc(minimumCapacity, allocatedCapacity));
    if (ASMJIT_UNLIKELY(!newData))
      return DebugUtils::errored(kErrorNoHeapMemory);

    // `allocatedCapacity` now contains number in bytes, we need bits.
    size_t allocatedCapacityInBits = allocatedCapacity * 8;

    // Arithmetic overflow should normally not happen. If it happens we just
    // change the `allocatedCapacityInBits` to the `minimumCapacityInBits` as
    // this value is still safe to be used to call `_allocator->release(...)`.
    if (ASMJIT_UNLIKELY(allocatedCapacityInBits < allocatedCapacity))
      allocatedCapacityInBits = minimumCapacityInBits;

    if (data)
      allocator->release(data, _capacity / 8);
    data = newData;

    _data = data;
    _capacity = uint32_t(allocatedCapacityInBits);
  }

  _length = newLength;
  _copyBits(data, other.getData(), _wordsPerBits(newLength));

  return kErrorOk;
}

Error ZoneBitVector::_resize(ZoneAllocator* allocator, uint32_t newLength, uint32_t idealCapacity, bool newBitsValue) noexcept {
  ASMJIT_ASSERT(idealCapacity >= newLength);

  if (newLength <= _length) {
    // The size after the resize is lesser than or equal to the current length.
    uint32_t idx = newLength / kBitWordSize;
    uint32_t bit = newLength % kBitWordSize;

    // Just set all bits outside of the new length in the last word to zero.
    // There is a case that there are not bits to set if `bit` is zero. This
    // happens when `newLength` is a multiply of `kBitWordSize` like 64, 128,
    // and so on. In that case don't change anything as that would mean settings
    // bits outside of the `_length`.
    if (bit)
      _data[idx] &= (BitWord(1) << bit) - 1U;

    _length = newLength;
    return kErrorOk;
  }

  uint32_t oldLength = _length;
  BitWord* data = _data;

  if (newLength > _capacity) {
    // Realloc needed, calculate the minimum capacity (in bytes) requied.
    uint32_t minimumCapacityInBits = IntUtils::alignUp<uint32_t>(idealCapacity, kBitWordSize);

    if (ASMJIT_UNLIKELY(minimumCapacityInBits < newLength))
      return DebugUtils::errored(kErrorNoHeapMemory);

    // Normalize to bytes.
    uint32_t minimumCapacity = minimumCapacityInBits / 8;
    size_t allocatedCapacity;

    BitWord* newData = static_cast<BitWord*>(allocator->alloc(minimumCapacity, allocatedCapacity));
    if (ASMJIT_UNLIKELY(!newData))
      return DebugUtils::errored(kErrorNoHeapMemory);

    // `allocatedCapacity` now contains number in bytes, we need bits.
    size_t allocatedCapacityInBits = allocatedCapacity * 8;

    // Arithmetic overflow should normally not happen. If it happens we just
    // change the `allocatedCapacityInBits` to the `minimumCapacityInBits` as
    // this value is still safe to be used to call `_allocator->release(...)`.
    if (ASMJIT_UNLIKELY(allocatedCapacityInBits < allocatedCapacity))
      allocatedCapacityInBits = minimumCapacityInBits;

    _copyBits(newData, data, _wordsPerBits(oldLength));

    if (data)
      allocator->release(data, _capacity / 8);
    data = newData;

    _data = data;
    _capacity = uint32_t(allocatedCapacityInBits);
  }

  // Start (of the old length) and end (of the new length) bits
  uint32_t idx = oldLength / kBitWordSize;
  uint32_t startBit = oldLength % kBitWordSize;
  uint32_t endBit = newLength % kBitWordSize;

  // Set new bits to either 0 or 1. The `pattern` is used to set multiple
  // bits per bit-word and contains either all zeros or all ones.
  BitWord pattern = IntUtils::maskFromBool<BitWord>(newBitsValue);

  // First initialize the last bit-word of the old length.
  if (startBit) {
    uint32_t nBits = 0;

    if (idx == (newLength / kBitWordSize)) {
      // The number of bit-words is the same after the resize. In that case
      // we need to set only bits necessary in the current last bit-word.
      ASMJIT_ASSERT(startBit < endBit);
      nBits = endBit - startBit;
    }
    else {
      // There is be more bit-words after the resize. In that case we don't
      // have to be extra careful about the last bit-word of the old length.
      nBits = kBitWordSize - startBit;
    }

    data[idx++] |= pattern << nBits;
  }

  // Initialize all bit-words after the last bit-word of the old length.
  uint32_t endIdx = _wordsPerBits(newLength);
  while (idx < endIdx) data[idx++] = pattern;

  // Clear unused bits of the last bit-word.
  if (endBit)
    data[endIdx - 1] = pattern & ((BitWord(1) << endBit) - 1);

  _length = newLength;
  return kErrorOk;
}

Error ZoneBitVector::_append(ZoneAllocator* allocator, bool value) noexcept {
  uint32_t kThreshold = Globals::kAllocThreshold * 8;
  uint32_t newLength = _length + 1;
  uint32_t idealCapacity = _capacity;

  if (idealCapacity < 128)
    idealCapacity = 128;
  else if (idealCapacity <= kThreshold)
    idealCapacity *= 2;
  else
    idealCapacity += kThreshold;

  if (ASMJIT_UNLIKELY(idealCapacity < _capacity)) {
    if (ASMJIT_UNLIKELY(_length == std::numeric_limits<uint32_t>::max()))
      return DebugUtils::errored(kErrorNoHeapMemory);
    idealCapacity = newLength;
  }

  return _resize(allocator, newLength, idealCapacity, value);
}

// ============================================================================
// [asmjit::ZoneVector / ZoneBitVector - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
template<typename T>
static void test_zone_vector(ZoneAllocator* allocator, const char* typeName) {
  int i;
  int kMax = 100000;

  ZoneVector<T> vec;

  INFO("ZoneVector<%s> basic tests", typeName);
  EXPECT(vec.append(allocator, 0) == kErrorOk);
  EXPECT(vec.isEmpty() == false);
  EXPECT(vec.getLength() == 1);
  EXPECT(vec.getCapacity() >= 1);
  EXPECT(vec.indexOf(0) == 0);
  EXPECT(vec.indexOf(-11) == Globals::kNotFound);

  vec.clear();
  EXPECT(vec.isEmpty());
  EXPECT(vec.getLength() == 0);
  EXPECT(vec.indexOf(0) == Globals::kNotFound);

  for (i = 0; i < kMax; i++) {
    EXPECT(vec.append(allocator, T(i)) == kErrorOk);
  }
  EXPECT(vec.isEmpty() == false);
  EXPECT(vec.getLength() == uint32_t(kMax));
  EXPECT(vec.indexOf(T(kMax - 1)) == uint32_t(kMax - 1));

  vec.release(allocator);
}

static void test_zone_bitvector(ZoneAllocator* allocator) {
  Zone zone(8096 - Zone::kZoneOverhead);

  uint32_t i, count;
  uint32_t kMaxCount = 100;

  ZoneBitVector vec;
  EXPECT(vec.isEmpty());
  EXPECT(vec.getLength() == 0);

  INFO("ZoneBitVector::resize()");
  for (count = 1; count < kMaxCount; count++) {
    vec.clear();
    EXPECT(vec.resize(allocator, count, false) == kErrorOk);
    EXPECT(vec.getLength() == count);

    for (i = 0; i < count; i++)
      EXPECT(vec.getBit(i) == false);

    vec.clear();
    EXPECT(vec.resize(allocator, count, true) == kErrorOk);
    EXPECT(vec.getLength() == count);

    for (i = 0; i < count; i++)
      EXPECT(vec.getBit(i) == true);
  }

  INFO("ZoneBitVector::fillBits() / clearBits()");
  for (count = 1; count < kMaxCount; count += 2) {
    vec.clear();
    EXPECT(vec.resize(allocator, count) == kErrorOk);
    EXPECT(vec.getLength() == count);

    for (i = 0; i < (count + 1) / 2; i++) {
      bool value = bool(i & 1);
      if (value)
        vec.fillBits(i, count - i * 2);
      else
        vec.clearBits(i, count - i * 2);
    }

    for (i = 0; i < count; i++) {
      EXPECT(vec.getBit(i) == bool(i & 1));
    }
  }
}

UNIT(core_zone_vector) {
  Zone zone(8096 - Zone::kZoneOverhead);
  ZoneAllocator allocator(&zone);

  test_zone_vector<int>(&allocator, "int");
  test_zone_vector<int64_t>(&allocator, "int64_t");
  test_zone_bitvector(&allocator);
}
#endif

ASMJIT_END_NAMESPACE
