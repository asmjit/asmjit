// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_SMALLSTRING_H
#define _ASMJIT_CORE_SMALLSTRING_H

#include "../core/globals.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_zone
//! \{

// ============================================================================
// [asmjit::ZoneStringBase]
// ============================================================================

struct ZoneStringBase {
  union {
    struct {
      uint32_t _size;
      char _embedded[sizeof(void*) * 2 - 4];
    };
    struct {
      void* _dummy;
      char* _external;
    };
  };

  inline void reset() noexcept {
    _dummy = nullptr;
    _external = nullptr;
  }

  Error setData(Zone* zone, uint32_t maxEmbeddedSize, const char* str, size_t size) noexcept {
    if (size == SIZE_MAX)
      size = strlen(str);

    if (size <= maxEmbeddedSize) {
      memcpy(_embedded, str, size);
      _embedded[size] = '\0';
    }
    else {
      char* external = static_cast<char*>(zone->dup(str, size, true));
      if (ASMJIT_UNLIKELY(!external))
        return DebugUtils::errored(kErrorOutOfMemory);
      _external = external;
    }

    _size = uint32_t(size);
    return kErrorOk;
  }
};

// ============================================================================
// [asmjit::ZoneString<N>]
// ============================================================================

//! Small string is a template that helps to create strings that can be either
//! statically allocated if they are small, or externally allocated in case
//! their size exceeds the limit. The `N` represents the size of the whole
//! `ZoneString` structure, based on that size the maximum size of the internal
//! buffer is determined.
template<size_t N>
class ZoneString {
public:
  static constexpr uint32_t kWholeSize =
    (N > sizeof(ZoneStringBase)) ? uint32_t(N) : uint32_t(sizeof(ZoneStringBase));
  static constexpr uint32_t kMaxEmbeddedSize = kWholeSize - 5;

  union {
    ZoneStringBase _base;
    char _wholeData[kWholeSize];
  };

  //! \name Construction & Destruction
  //! \{

  inline ZoneString() noexcept { reset(); }
  inline void reset() noexcept { _base.reset(); }

  //! \}

  //! \name Accessors
  //! \{

  inline const char* data() const noexcept { return _base._size <= kMaxEmbeddedSize ? _base._embedded : _base._external; }
  inline bool empty() const noexcept { return _base._size == 0; }
  inline uint32_t size() const noexcept { return _base._size; }

  inline bool isEmbedded() const noexcept { return _base._size <= kMaxEmbeddedSize; }

  inline Error setData(Zone* zone, const char* data, size_t size) noexcept {
    return _base.setData(zone, kMaxEmbeddedSize, data, size);
  }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_SMALLSTRING_H
