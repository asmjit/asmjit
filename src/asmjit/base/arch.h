// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_ARCH_H
#define _ASMJIT_BASE_ARCH_H

// [Dependencies]
#include "../base/globals.h"
#include "../base/operand.h"

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

// ============================================================================
// [asmjit::ArchInfo]
// ============================================================================

class ArchInfo {
public:
  //! Architecture type.
  ASMJIT_ENUM(Type) {
    kTypeNone  = 0,                      //!< No/Unknown architecture.
    kTypeX86   = 1,                      //!< X86 architecture (32-bit).
    kTypeX64   = 2,                      //!< X64 architecture (64-bit) (AMD64).
    kTypeX32   = 3,                      //!< X32 architecture (DEAD-END).
    kTypeArm32 = 4,                      //!< ARM32 architecture (32-bit).
    kTypeArm64 = 5,                      //!< ARM64 architecture (64-bit).

    //! Architecture detected at compile-time (architecture of the host).
    kTypeHost  = ASMJIT_ARCH_X86   ? kTypeX86   :
                 ASMJIT_ARCH_X64   ? kTypeX64   :
                 ASMJIT_ARCH_ARM32 ? kTypeArm32 :
                 ASMJIT_ARCH_ARM64 ? kTypeArm64 : kTypeNone
  };

  //! Architecture sub-type.
  ASMJIT_ENUM(SubType) {
    kSubTypeNone        = 0,             //!< Default mode (or no specific mode).

    kX86SubTypeLegacy   = 0,             //!< Legacy (the most compatible) mode.
    kX86SubTypeAVX      = 1,             //!< AVX mode.
    kX86SubTypeAVX512F  = 2              //!< AVX512F mode.
  };

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  static ASMJIT_INLINE bool isX86Family(uint32_t archType) noexcept { return archType >= kTypeX86 && archType <= kTypeX32; }
  static ASMJIT_INLINE bool isArmFamily(uint32_t archType) noexcept { return archType >= kTypeArm32 && archType <= kTypeArm64; }

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE ArchInfo() noexcept : _signature(0) {}
  ASMJIT_INLINE ArchInfo(const ArchInfo& other) noexcept : _signature(other._signature) {}

  explicit ASMJIT_INLINE ArchInfo(uint32_t type, uint32_t subType = kSubTypeNone) noexcept {
    init(type, subType);
  }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE bool isInitialized() const noexcept { return _type != kTypeNone; }

  ASMJIT_API void init(uint32_t type, uint32_t subType = kSubTypeNone) noexcept;
  ASMJIT_INLINE void reset() noexcept { _signature = 0; }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get if the architecture is 32-bit.
  ASMJIT_INLINE bool is32Bit() const noexcept { return _gpSize == 4; }
  //! Get if the architecture is 64-bit.
  ASMJIT_INLINE bool is64Bit() const noexcept { return _gpSize == 8; }

  //! Get architecture type, see \ref Type.
  ASMJIT_INLINE uint32_t getType() const noexcept { return _type; }

  //! Get architecture sub-type, see \ref SubType.
  //!
  //! X86 & X64
  //! ---------
  //!
  //! Architecture subtype describe the highest instruction-set level that can
  //! be used.
  //!
  //! ARM32
  //! -----
  //!
  //! Architecture mode means the instruction encoding to be used when generating
  //! machine code, thus mode can be used to force generation of THUMB and THUMB2
  //! encoding of regular ARM encoding.
  //!
  //! ARM64
  //! -----
  //!
  //! No meaning yet.
  ASMJIT_INLINE uint32_t getSubType() const noexcept { return _subType; }

  //! Get if the architecture is X86, X64, or X32.
  ASMJIT_INLINE bool isX86Family() const noexcept { return isX86Family(_type); }
  //! Get if the architecture is ARM32 or ARM64.
  ASMJIT_INLINE bool isArmFamily() const noexcept { return isArmFamily(_type); }

  //! Get a size of a general-purpose register.
  ASMJIT_INLINE uint32_t getGpSize() const noexcept { return _gpSize; }
  //! Get number of general-purpose registers.
  ASMJIT_INLINE uint32_t getGpCount() const noexcept { return _gpCount; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  ASMJIT_INLINE const ArchInfo& operator=(const ArchInfo& other) noexcept { _signature = other._signature; return *this; }
  ASMJIT_INLINE bool operator==(const ArchInfo& other) const noexcept { return _signature == other._signature; }
  ASMJIT_INLINE bool operator!=(const ArchInfo& other) const noexcept { return _signature != other._signature; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  union {
    struct {
      uint8_t _type;                     //!< Architecture type.
      uint8_t _subType;                  //!< Architecture sub-type.
      uint8_t _gpSize;                   //!< Default size of a general purpose register.
      uint8_t _gpCount;                  //!< Count of all general purpose registers.
    };
    uint32_t _signature;                 //!< Architecture signature (32-bit int).
  };
};

// ============================================================================
// [asmjit::ArchRegs]
// ============================================================================

//! Information about all architecture registers.
struct ArchRegs {
  //! Register information and signatures indexed by \ref Reg::Type.
  RegInfo regInfo[Reg::kRegMax + 1];
  //! Converts RegType to TypeId, see \ref TypeId::Id.
  uint8_t regTypeToTypeId[Reg::kRegMax + 1];
};

// ============================================================================
// [asmjit::ArchUtils]
// ============================================================================

struct ArchUtils {
  static ASMJIT_API Error typeIdToRegInfo(uint32_t archType, uint32_t& typeIdInOut, RegInfo& regInfo) noexcept;
};

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_ARCH_H
