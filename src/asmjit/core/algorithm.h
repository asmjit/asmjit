// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ALGORITHM_H
#define _ASMJIT_CORE_ALGORITHM_H

// [Dependencies]
#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_support
//! \{

//! Implements some algorithms and provides iterators for asmjit containers.
namespace Algorithm {

// ============================================================================
// [asmjit::Algorithm::Order]
// ============================================================================

//! Order of comparison.
enum Order : uint32_t {
  kOrderAscending  = 0, //!< Ascending.
  kOrderDescending = 1  //!< Descending.
};

// ============================================================================
// [asmjit::Algorithm::Iterator]
// ============================================================================

template<typename T>
class Iterator {
public:
  constexpr Iterator(T* p) noexcept : _p(p) {}
  constexpr Iterator(const Iterator& other) noexcept = default;

  inline Iterator& operator=(const Iterator& other) noexcept = default;

  inline Iterator operator+(size_t n) const noexcept { return Iterator(_p + n); }
  inline Iterator operator-(size_t n) const noexcept { return Iterator(_p - n); }

  inline Iterator& operator+=(size_t n) noexcept { _p += n; return *this; }
  inline Iterator& operator-=(size_t n) noexcept { _p -= n; return *this; }

  inline Iterator& operator++() noexcept { return operator+=(1); }
  inline Iterator& operator--() noexcept { return operator-=(1); }

  inline Iterator operator++(int) noexcept { T* prev = _p; operator+=(1); return Iterator(prev); }
  inline Iterator operator--(int) noexcept { T* prev = _p; operator-=(1); return Iterator(prev); }

  inline bool operator==(const Iterator& other) noexcept { return _p == other._p; }
  inline bool operator!=(const Iterator& other) noexcept { return _p != other._p; }

  inline T& operator*() const noexcept { return _p[0]; }

  T* _p;
};

// ============================================================================
// [asmjit::Algorithm::ReverseIterator]
// ============================================================================

template<typename T>
class ReverseIterator {
public:
  constexpr ReverseIterator(T* p) noexcept : _p(p) {}
  constexpr ReverseIterator(const ReverseIterator& other) noexcept = default;

  inline ReverseIterator& operator=(const ReverseIterator& other) noexcept = default;

  inline ReverseIterator operator+(size_t n) const noexcept { return ReverseIterator(_p + n); }
  inline ReverseIterator operator-(size_t n) const noexcept { return ReverseIterator(_p - n); }

  inline ReverseIterator& operator+=(size_t n) noexcept { _p -= n; return *this; }
  inline ReverseIterator& operator-=(size_t n) noexcept { _p += n; return *this; }

  inline ReverseIterator& operator++() noexcept { return operator+=(1); }
  inline ReverseIterator& operator--() noexcept { return operator-=(1); }

  inline ReverseIterator operator++(int) noexcept { T* prev = _p; operator+=(1); return ReverseIterator(prev); }
  inline ReverseIterator operator--(int) noexcept { T* prev = _p; operator-=(1); return ReverseIterator(prev); }

  inline bool operator==(const ReverseIterator& other) noexcept { return _p == other._p; }
  inline bool operator!=(const ReverseIterator& other) noexcept { return _p != other._p; }

  inline T& operator*() const noexcept { return _p[-1]; }

  T* _p;
};

// ============================================================================
// [asmjit::Algorithm::Compare]
// ============================================================================

//! \internal
//!
//! A helper class that provides comparison of any user-defined type that
//! implements `<` and `>` operators (primitive types are supported as well).
template<uint32_t ORDER = kOrderAscending>
struct Compare {
  template<typename A, typename B>
  inline int operator()(const A& a, const B& b) const noexcept {
    return (ORDER == kOrderAscending) ? (a < b ? -1 : a > b ?  1 : 0)
                                      : (a < b ?  1 : a > b ? -1 : 0);
  }
};

// ============================================================================
// [asmjit::Algorithm::ISort]
// ============================================================================

//! Insertion sort.
template<typename T, typename CMP = Compare<kOrderAscending>>
static inline void iSort(T* base, size_t size, const CMP& cmp = CMP()) noexcept {
  for (T* pm = base + 1; pm < base + size; pm++)
    for (T* pl = pm; pl > base && cmp(pl[-1], pl[0]) > 0; pl--)
      std::swap(pl[-1], pl[0]);
}

// ============================================================================
// [asmjit::Algorithm::QSort]
// ============================================================================

//! \internal
//!
//! Quick-sort implementation.
template<typename T, class CMP>
struct QSortImpl {
  static constexpr size_t kStackSize = 64 * 2;
  static constexpr size_t kISortThreshold = 7;

  // Based on "PDCLib - Public Domain C Library" and rewritten to C++.
  static void sort(T* base, size_t size, const CMP& cmp) noexcept {
    T* end = base + size;
    T* stack[kStackSize];
    T** stackptr = stack;

    for (;;) {
      if ((size_t)(end - base) > kISortThreshold) {
        // We work from second to last - first will be pivot element.
        T* pi = base + 1;
        T* pj = end - 1;
        std::swap(base[(size_t)(end - base) / 2], base[0]);

        if (cmp(*pi  , *pj  ) > 0) std::swap(*pi  , *pj  );
        if (cmp(*base, *pj  ) > 0) std::swap(*base, *pj  );
        if (cmp(*pi  , *base) > 0) std::swap(*pi  , *base);

        // Now we have the median for pivot element, entering main loop.
        for (;;) {
          while (pi < pj   && cmp(*++pi, *base) < 0) continue; // Move `i` right until `*i >= pivot`.
          while (pj > base && cmp(*--pj, *base) > 0) continue; // Move `j` left  until `*j <= pivot`.

          if (pi > pj) break;
          std::swap(*pi, *pj);
        }

        // Move pivot into correct place.
        std::swap(*base, *pj);

        // Larger subfile base / end to stack, sort smaller.
        if (pj - base > end - pi) {
          // Left is larger.
          *stackptr++ = base;
          *stackptr++ = pj;
          base = pi;
        }
        else {
          // Right is larger.
          *stackptr++ = pi;
          *stackptr++ = end;
          end = pj;
        }
        ASMJIT_ASSERT(stackptr <= stack + kStackSize);
      }
      else {
        iSort(base, (size_t)(end - base), cmp);
        if (stackptr == stack)
          break;
        end = *--stackptr;
        base = *--stackptr;
      }
    }
  }
};

//! Quick sort implementation.
//!
//! The main reason to provide a custom qsort implementation is that we needed
//! something that will never throw `bad_alloc` exception. This implementation
//! doesn't use dynamic memory allocation.
template<typename T, class CMP = Compare<kOrderAscending>>
static inline void qSort(T* base, size_t size, const CMP& cmp = CMP()) noexcept {
  QSortImpl<T, CMP>::sort(base, size, cmp);
}

} // Algorithm namespace

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ALGORITHM_H
