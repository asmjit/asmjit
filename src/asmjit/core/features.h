// [AsmJit]
// Machine Code Generation for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

#ifndef _ASMJIT_CORE_FEATURES_H
#define _ASMJIT_CORE_FEATURES_H

#include "../core/globals.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::BaseFeatures]
// ============================================================================

class BaseFeatures {
public:
  typedef Support::BitWord BitWord;

  enum : uint32_t {
    kMaxFeatures = 128,
    kNumBitWords = kMaxFeatures / Support::kBitWordSizeInBits
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline BaseFeatures() noexcept { reset(); }
  inline BaseFeatures(const BaseFeatures& other) noexcept = default;
  inline explicit BaseFeatures(Globals::NoInit_) noexcept {}

  // --------------------------------------------------------------------------
  // [Reset]
  // --------------------------------------------------------------------------

  inline void reset() noexcept {
    for (size_t i = 0; i < kNumBitWords; i++)
      _bits[i] = 0;
  }

  // --------------------------------------------------------------------------
  // [Cast]
  // --------------------------------------------------------------------------

  template<typename T>
  inline T& as() noexcept { return static_cast<T&>(*this); }

  template<typename T>
  inline const T& as() const noexcept { return static_cast<const T&>(*this); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Gets all features as `BitWord` array.
  inline BitWord* bits() noexcept { return _bits; }
  //! Gets all features as `BitWord` array (const).
  inline const BitWord* bits() const noexcept { return _bits; }

  //! Gets whether feature `featureId` is present.
  inline bool has(uint32_t featureId) const noexcept {
    ASMJIT_ASSERT(featureId < kMaxFeatures);

    uint32_t idx = featureId / Support::kBitWordSizeInBits;
    uint32_t bit = featureId % Support::kBitWordSizeInBits;

    return bool((_bits[idx] >> bit) & 0x1);
  }

  //! Gets whether all features as defined by `other` are present.
  inline bool hasAll(const BaseFeatures& other) const noexcept {
    for (uint32_t i = 0; i < kNumBitWords; i++)
      if ((_bits[i] & other._bits[i]) != other._bits[i])
        return false;
    return true;
  }

  // --------------------------------------------------------------------------
  // [Operations]
  // --------------------------------------------------------------------------

  //! Adds the given CPU `featureId` to the list of features.
  inline void add(uint32_t featureId) noexcept {
    ASMJIT_ASSERT(featureId < kMaxFeatures);

    uint32_t idx = featureId / Support::kBitWordSizeInBits;
    uint32_t bit = featureId % Support::kBitWordSizeInBits;

    _bits[idx] |= BitWord(1) << bit;
  }

  template<typename... ArgsT>
  inline void add(uint32_t featureId, ArgsT... otherIds) noexcept {
    add(featureId);
    add(otherIds...);
  }

  //! Removes the given CPU `featureId` from the list of features.
  inline void remove(uint32_t featureId) noexcept {
    ASMJIT_ASSERT(featureId < kMaxFeatures);

    uint32_t idx = featureId / Support::kBitWordSizeInBits;
    uint32_t bit = featureId % Support::kBitWordSizeInBits;

    _bits[idx] &= ~(BitWord(1) << bit);
  }

  template<typename... ArgsT>
  inline void remove(uint32_t featureId, ArgsT... otherIds) noexcept {
    remove(featureId);
    remove(otherIds...);
  }

  inline bool eq(const BaseFeatures& other) const noexcept {
    for (size_t i = 0; i < kNumBitWords; i++)
      if (_bits[i] != other._bits[i])
        return false;
    return true;
  }


  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline BaseFeatures& operator=(const BaseFeatures& other) noexcept = default;

  inline bool operator==(const BaseFeatures& other) noexcept { return  eq(other); }
  inline bool operator!=(const BaseFeatures& other) noexcept { return !eq(other); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  BitWord _bits[kNumBitWords];
};

//! \}

ASMJIT_END_NAMESPACE

#endif // _ASMJIT_CORE_FEATURES_H
