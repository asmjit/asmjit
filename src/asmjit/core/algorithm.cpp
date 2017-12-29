// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies]
#include "../core/algorithm.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::Algorithm - Unit]
// ============================================================================

#if defined(ASMJIT_BUILD_TEST)
template<typename T>
static void testArrays(const T* a, const T* b, size_t size) noexcept {
  for (size_t i = 0; i < size; i++)
    EXPECT(a[i] == b[i], "Mismatch at %u", unsigned(i));
}

UNIT(core_algorithm) {
  INFO("Testing qsort and isort of predefined arrays");
  {
    constexpr size_t kArraySize = 11;

    int ref_[kArraySize] = { -4, -2, -1, 0, 1, 9, 12, 13, 14, 19, 22 };
    int arr1[kArraySize] = { 0, 1, -1, 19, 22, 14, -4, 9, 12, 13, -2 };
    int arr2[kArraySize];

    std::memcpy(arr2, arr1, kArraySize * sizeof(int));

    Algorithm::iSort(arr1, kArraySize);
    Algorithm::qSort(arr2, kArraySize);
    testArrays(arr1, ref_, kArraySize);
    testArrays(arr2, ref_, kArraySize);
  }

  INFO("Testing qsort and isort of artificial arrays");
  {
    constexpr size_t kArraySize = 200;

    int arr1[kArraySize];
    int arr2[kArraySize];
    int ref_[kArraySize];

    for (size_t size = 2; size < kArraySize; size++) {
      for (size_t i = 0; i < size; i++) {
        arr1[i] = int(size - 1 - i);
        arr2[i] = int(size - 1 - i);
        ref_[i] = int(i);
      }

      Algorithm::iSort(arr1, size);
      Algorithm::qSort(arr2, size);
      testArrays(arr1, ref_, size);
      testArrays(arr2, ref_, size);
    }
  }

  INFO("Testing qsort and isort having unstable compare function");
  {
    constexpr size_t kArraySize = 5;

    float arr1[kArraySize] = { 1.0f, 0.0f, 3.0f, -1.0f, std::numeric_limits<float>::quiet_NaN() };
    float arr2[kArraySize] = { };

    std::memcpy(arr2, arr1, kArraySize * sizeof(float));

    // We don't test as it's undefined where the NaN would be.
    Algorithm::iSort(arr1, kArraySize);
    Algorithm::qSort(arr2, kArraySize);
  }
}

#endif

ASMJIT_END_NAMESPACE
