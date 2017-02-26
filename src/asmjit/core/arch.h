// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_ARCH_H
#define _ASMJIT_CORE_ARCH_H

// [Dependencies]
#include "../core/globals.h"
#include "../core/operand.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::ArchInfo]
// ============================================================================

class ArchInfo {
public:
  //! Architecture id.
  enum Id : uint32_t {
    kTypeNone  = 0,                      //!< No/Unknown architecture.

    // X86 architectures.
    kTypeX86   = 1,                      //!< X86 architecture (32-bit).
    kTypeX64   = 2,                      //!< X64 architecture (64-bit) (AMD64).

    // ARM architectures.
    kTypeA32   = 3,                      //!< ARM 32-bit architecture (AArch32/ARM/THUMB).
    kTypeA64   = 4,                      //!< ARM 64-bit architecture (AArch64).

    //! Architecture detected at compile-time (architecture of the host).
    kTypeHost  = ASMJIT_ARCH_X86 == 32 ? kTypeX86 :
                 ASMJIT_ARCH_X86 == 64 ? kTypeX64 :
                 ASMJIT_ARCH_ARM == 32 ? kTypeA32 :
                 ASMJIT_ARCH_ARM == 64 ? kTypeA64 : kTypeNone
  };

  //! Architecture sub-type or execution mode.
  enum SubType : uint32_t {
    kSubTypeNone         = 0,            //!< Default mode (or no specific mode).

    // X86 sub-types.
    kSubTypeX86_AVX      = 1,            //!< Code generation uses AVX         by default (VEC instructions).
    kSubTypeX86_AVX2     = 2,            //!< Code generation uses AVX2        by default (VEC instructions).
    kSubTypeX86_AVX512   = 3,            //!< Code generation uses AVX-512F    by default (+32 vector regs).
    kSubTypeX86_AVX512VL = 4,            //!< Code generation uses AVX-512F-VL by default (+VL extensions).

    // ARM sub-types.
    kSubTypeA32_Thumb    = 8,            //!< THUMB|THUMB2 sub-type (only ARM in 32-bit mode).

    #if   (ASMJIT_ARCH_X86) && defined(__AVX512VL__)
    kSubTypeHost = kSubTypeX86_AVX512VL
    #elif (ASMJIT_ARCH_X86) && defined(__AVX512F__)
    kSubTypeHost = kSubTypeX86_AVX512
    #elif (ASMJIT_ARCH_X86) && defined(__AVX2__)
    kSubTypeHost = kSubTypeX86_AVX2
    #elif (ASMJIT_ARCH_X86) && defined(__AVX__)
    kSubTypeHost = kSubTypeX86_AVX
    #elif (ASMJIT_ARCH_ARM == 32) && (defined(_M_ARMT) || defined(__thumb__) || defined(__thumb2__))
    kSubTypeHost = kSubTypeA32_Thumb
    #else
    kSubTypeHost = 0
    #endif
  };

  // --------------------------------------------------------------------------
  // [Utilities]
  // --------------------------------------------------------------------------

  static inline bool isX86Family(uint32_t archType) noexcept { return archType >= kTypeX86 && archType <= kTypeX64; }
  static inline bool isArmFamily(uint32_t archType) noexcept { return archType >= kTypeA32 && archType <= kTypeA64; }

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline ArchInfo() noexcept : _signature(0) {}
  inline ArchInfo(const ArchInfo& other) noexcept : _signature(other._signature) {}
  explicit inline ArchInfo(uint32_t type, uint32_t subType = kSubTypeNone) noexcept { init(type, subType); }

  inline static ArchInfo host() noexcept { return ArchInfo(kTypeHost, kSubTypeHost); }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline bool isInitialized() const noexcept { return _type != kTypeNone; }

  ASMJIT_API void init(uint32_t type, uint32_t subType = kSubTypeNone) noexcept;
  inline void reset() noexcept { _signature = 0; }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get architecture type, see \ref Id.
  inline uint32_t getType() const noexcept { return _type; }

  //! Get architecture sub-type, see \ref SubType.
  //!
  //! X86 & X64
  //! ---------
  //!
  //! Architecture subtype describe the highest instruction-set level that can
  //! be used.
  //!
  //! A32 & A64
  //! ---------
  //!
  //! Architecture mode means the instruction encoding to be used when generating
  //! machine code, thus mode can be used to force generation of THUMB and THUMB2
  //! encoding or regular ARM encoding.
  inline uint32_t getSubType() const noexcept { return _subType; }

  //! Get whether the architecture is 32-bit.
  inline bool is32Bit() const noexcept { return _gpSize == 4; }
  //! Get whether the architecture is 64-bit.
  inline bool is64Bit() const noexcept { return _gpSize == 8; }

  //! Get whether the architecture is X86, X64, or X32.
  inline bool isX86Family() const noexcept { return isX86Family(_type); }
  //! Get whether the architecture is ARM32 or ARM64.
  inline bool isArmFamily() const noexcept { return isArmFamily(_type); }

  //! Get a size of a general-purpose register.
  inline uint32_t getGpSize() const noexcept { return _gpSize; }
  //! Get number of general-purpose registers.
  inline uint32_t getGpCount() const noexcept { return _gpCount; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline ArchInfo& operator=(const ArchInfo& other) noexcept = default;

  inline bool operator==(const ArchInfo& other) const noexcept { return _signature == other._signature; }
  inline bool operator!=(const ArchInfo& other) const noexcept { return _signature != other._signature; }

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
  //! Count (maximum) of registers per \ref Reg::Type.
  uint8_t regCount[Reg::kRegMax + 1];
  //! Converts RegType to TypeId, see \ref Type::Id.
  uint8_t regTypeToTypeId[Reg::kRegMax + 1];
};

// ============================================================================
// [asmjit::ArchUtils]
// ============================================================================

struct ArchUtils {
  ASMJIT_API static Error typeIdToRegInfo(uint32_t archType, uint32_t& typeIdInOut, RegInfo& regInfo) noexcept;
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_ARCH_H
