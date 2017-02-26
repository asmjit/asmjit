// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_SMALLSTRING_H
#define _ASMJIT_CORE_SMALLSTRING_H

// [Dependencies]
#include "../core/globals.h"
#include "../core/zone.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::SmallStringBase]
// ============================================================================

struct SmallStringBase {
  inline void reset() noexcept {
    _dummy = nullptr;
    _external = nullptr;
  }

  Error setData(Zone* zone, uint32_t maxEmbeddedLength, const char* str, size_t len) noexcept {
    if (len == Globals::kNullTerminated)
      len = std::strlen(str);

    if (len <= maxEmbeddedLength) {
      std::memcpy(_embedded, str, len);
      _embedded[len] = '\0';
    }
    else {
      char* external = static_cast<char*>(zone->dup(str, len, true));
      if (ASMJIT_UNLIKELY(!external))
        return DebugUtils::errored(kErrorNoHeapMemory);
      _external = external;
    }

    _length = uint32_t(len);
    return kErrorOk;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      uint32_t _length;
      char _embedded[sizeof(void*) * 2 - 4];
    };
    struct {
      void* _dummy;
      char* _external;
    };
  };
};

// ============================================================================
// [asmjit::SmallString<N>]
// ============================================================================

//! Small string is a template that helps to create strings that can be either
//! statically allocated if they are small, or externally allocated in case
//! their length exceeds the limit. The `N` represents the size of the whole
//! `SmallString` structure, based on that size the maximum size of the internal
//! buffer is determined.
template<size_t N>
class SmallString {
public:
  static constexpr uint32_t kWholeSize = 
    (N > sizeof(SmallStringBase)) ? uint32_t(N)
                                  : uint32_t(sizeof(SmallStringBase));
  static constexpr uint32_t kMaxEmbeddedLength = kWholeSize - 5;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline SmallString() noexcept { reset(); }
  inline void reset() noexcept { _base.reset(); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  inline bool isEmpty() const noexcept { return _base._length == 0; }
  inline bool isEmbedded() const noexcept { return _base._length <= kMaxEmbeddedLength; }

  inline uint32_t getLength() const noexcept { return _base._length; }
  inline const char* getData() const noexcept { return _base._length <= kMaxEmbeddedLength ? _base._embedded : _base._external; }

  inline Error setData(Zone* zone, const char* data, size_t len) noexcept {
    return _base.setData(zone, kMaxEmbeddedLength, data, len);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    SmallStringBase _base;
    char _wholeData[kWholeSize];
  };
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_SMALLSTRING_H
