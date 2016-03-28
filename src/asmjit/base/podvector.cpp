// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/podvector.h"
#include "../base/utils.h"

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::PodVectorBase - NullData]
// ============================================================================

const PodVectorBase::Data PodVectorBase::_nullData = { 0, 0 };

static ASMJIT_INLINE bool isDataStatic(PodVectorBase* self, PodVectorBase::Data* d) noexcept {
  return (void*)(self + 1) == (void*)d;
}

// ============================================================================
// [asmjit::PodVectorBase - Reset]
// ============================================================================

//! Clear vector data and free internal buffer.
void PodVectorBase::reset(bool releaseMemory) noexcept {
  Data* d = _d;
  if (d == &_nullData)
    return;

  if (releaseMemory && !isDataStatic(this, d)) {
    ASMJIT_FREE(d);
    _d = const_cast<Data*>(&_nullData);
    return;
  }

  d->length = 0;
}

// ============================================================================
// [asmjit::PodVectorBase - Helpers]
// ============================================================================

Error PodVectorBase::_grow(size_t n, size_t sizeOfT) noexcept {
  Data* d = _d;

  size_t threshold = kMemAllocGrowMax / sizeOfT;
  size_t capacity = d->capacity;
  size_t after = d->length;

  if (IntTraits<size_t>::maxValue() - n < after)
    return kErrorNoHeapMemory;

  after += n;

  if (capacity >= after)
    return kErrorOk;

  // PodVector is used as a linear array for some data structures used by
  // AsmJit code generation. The purpose of this agressive growing schema
  // is to minimize memory reallocations, because AsmJit code generation
  // classes live short life and will be freed or reused soon.
  if (capacity < 32)
    capacity = 32;
  else if (capacity < 128)
    capacity = 128;
  else if (capacity < 512)
    capacity = 512;

  while (capacity < after) {
    if (capacity < threshold)
      capacity *= 2;
    else
      capacity += threshold;
  }

  return _reserve(capacity, sizeOfT);
}

Error PodVectorBase::_reserve(size_t n, size_t sizeOfT) noexcept {
  Data* d = _d;

  if (d->capacity >= n)
    return kErrorOk;

  size_t nBytes = sizeof(Data) + n * sizeOfT;
  if (ASMJIT_UNLIKELY(nBytes < n))
    return kErrorNoHeapMemory;

  if (d == &_nullData) {
    d = static_cast<Data*>(ASMJIT_ALLOC(nBytes));
    if (ASMJIT_UNLIKELY(d == nullptr))
      return kErrorNoHeapMemory;
    d->length = 0;
  }
  else {
    if (isDataStatic(this, d)) {
      Data* oldD = d;

      d = static_cast<Data*>(ASMJIT_ALLOC(nBytes));
      if (ASMJIT_UNLIKELY(d == nullptr))
        return kErrorNoHeapMemory;

      size_t len = oldD->length;
      d->length = len;
      ::memcpy(d->getData(), oldD->getData(), len * sizeOfT);
    }
    else {
      d = static_cast<Data*>(ASMJIT_REALLOC(d, nBytes));
      if (ASMJIT_UNLIKELY(d == nullptr))
        return kErrorNoHeapMemory;
    }
  }

  d->capacity = n;
  _d = d;

  return kErrorOk;
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
