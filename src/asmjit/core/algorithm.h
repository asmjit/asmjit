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

namespace Algorithm {

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::Algorithm::Order]
// ============================================================================

enum Order : uint32_t {
  kOrderAscending  = 0,
  kOrderDescending = 1
};

// ============================================================================
// [asmjit::Algorithm::Compare]
// ============================================================================

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
static inline void iSort(T* base, size_t len, const CMP& cmp = CMP()) noexcept {
  for (T* pm = base + 1; pm < base + len; pm++)
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
  static void sort(T* base, size_t len, const CMP& cmp) noexcept {
    T* end = base + len;
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

//! Quick sort.
template<typename T, class CMP = Compare<kOrderAscending>>
static inline void qSort(T* base, size_t len, const CMP& cmp = CMP()) noexcept {
  QSortImpl<T, CMP>::sort(base, len, cmp);
}

//! \}

} // Algorithm namespace

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ALGORITHM_H
