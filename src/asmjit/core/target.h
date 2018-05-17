// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_TARGET_H
#define _ASMJIT_CORE_TARGET_H

// [Dependencies]
#include "../core/arch.h"
#include "../core/func.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core_api
//! \{

// ============================================================================
// [asmjit::CodeInfo]
// ============================================================================

//! Basic information about a code (or target). It describes its architecture,
//! code generation mode (or optimization level), and base address.
class CodeInfo {
public:
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  inline CodeInfo() noexcept
    : _archInfo(),
      _stackAlignment(0),
      _cdeclCallConv(CallConv::kIdNone),
      _stdCallConv(CallConv::kIdNone),
      _fastCallConv(CallConv::kIdNone),
      _baseAddress(Globals::kNoBaseAddress) {}

  inline explicit CodeInfo(uint32_t archId, uint32_t archMode = 0, uint64_t baseAddress = Globals::kNoBaseAddress) noexcept
    : _archInfo(archId, archMode),
      _stackAlignment(0),
      _cdeclCallConv(CallConv::kIdNone),
      _stdCallConv(CallConv::kIdNone),
      _fastCallConv(CallConv::kIdNone),
      _baseAddress(baseAddress) {}

  inline CodeInfo(const CodeInfo& other) noexcept { init(other); }

  // --------------------------------------------------------------------------
  // [Init / Reset]
  // --------------------------------------------------------------------------

  inline bool isInitialized() const noexcept {
    return _archInfo.archId() != ArchInfo::kIdNone;
  }

  inline void init(const CodeInfo& other) noexcept {
    std::memcpy(this, &other, sizeof(*this));
  }

  inline void init(uint32_t archId, uint32_t archMode = 0, uint64_t baseAddress = Globals::kNoBaseAddress) noexcept {
    _archInfo.init(archId, archMode);
    _stackAlignment = 0;
    _cdeclCallConv = CallConv::kIdNone;
    _stdCallConv = CallConv::kIdNone;
    _fastCallConv = CallConv::kIdNone;
    _baseAddress = baseAddress;
  }

  inline void reset() noexcept {
    _archInfo.reset();
    _stackAlignment = 0;
    _cdeclCallConv = CallConv::kIdNone;
    _stdCallConv = CallConv::kIdNone;
    _fastCallConv = CallConv::kIdNone;
    _baseAddress = Globals::kNoBaseAddress;
  }

  // --------------------------------------------------------------------------
  // [Architecture Information]
  // --------------------------------------------------------------------------

  //! Get architecture information, see `ArchInfo`.
  inline const ArchInfo& archInfo() const noexcept { return _archInfo; }

  //! Get architecture type, see `ArchInfo::Id`.
  inline uint32_t archId() const noexcept { return _archInfo.archId(); }
  //! Get architecture sub-type, see `ArchInfo::SubId`.
  inline uint32_t archSubId() const noexcept { return _archInfo.archSubId(); }
  //! Get a size of a GP register of the architecture the code is using.
  inline uint32_t gpSize() const noexcept { return _archInfo.gpSize(); }
  //! Get number of GP registers available of the architecture the code is using.
  inline uint32_t gpCount() const noexcept { return _archInfo.gpCount(); }

  // --------------------------------------------------------------------------
  // [High-Level Information]
  // --------------------------------------------------------------------------

  //! Get a natural stack alignment that must be honored (or 0 if not known).
  inline uint32_t stackAlignment() const noexcept { return _stackAlignment; }
  //! Set a natural stack alignment that must be honored.
  inline void setStackAlignment(uint32_t sa) noexcept { _stackAlignment = uint8_t(sa); }

  inline uint32_t cdeclCallConv() const noexcept { return _cdeclCallConv; }
  inline void setCdeclCallConv(uint32_t cc) noexcept { _cdeclCallConv = uint8_t(cc); }

  inline uint32_t stdCallConv() const noexcept { return _stdCallConv; }
  inline void setStdCallConv(uint32_t cc) noexcept { _stdCallConv = uint8_t(cc); }

  inline uint32_t fastCallConv() const noexcept { return _fastCallConv; }
  inline void setFastCallConv(uint32_t cc) noexcept { _fastCallConv = uint8_t(cc); }

  // --------------------------------------------------------------------------
  // [Addressing Information]
  // --------------------------------------------------------------------------

  inline bool hasBaseAddress() const noexcept { return _baseAddress != Globals::kNoBaseAddress; }
  inline uint64_t baseAddress() const noexcept { return _baseAddress; }
  inline void setBaseAddress(uint64_t p) noexcept { _baseAddress = p; }
  inline void resetBaseAddress() noexcept { _baseAddress = Globals::kNoBaseAddress; }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  inline CodeInfo& operator=(const CodeInfo& other) noexcept = default;

  inline bool operator==(const CodeInfo& other) const noexcept { return std::memcmp(this, &other, sizeof(*this)) == 0; }
  inline bool operator!=(const CodeInfo& other) const noexcept { return std::memcmp(this, &other, sizeof(*this)) != 0; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  ArchInfo _archInfo;                    //!< Architecture information.
  uint8_t _stackAlignment;               //!< Natural stack alignment (ARCH+OS).
  uint8_t _cdeclCallConv;                //!< Default CDECL calling convention.
  uint8_t _stdCallConv;                  //!< Default STDCALL calling convention.
  uint8_t _fastCallConv;                 //!< Default FASTCALL calling convention.
  uint64_t _baseAddress;                 //!< Base address.
};

// ============================================================================
// [asmjit::Target]
// ============================================================================

//! Target is an abstract class that describes the machine code target.
class ASMJIT_VIRTAPI Target {
public:
  ASMJIT_NONCOPYABLE(Target)

  enum TargetType : uint32_t {
    kTargetNone   = 0,
    kTargetJit    = 1
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `Target` instance.
  ASMJIT_API Target() noexcept;
  //! Destroy the `Target` instance.
  ASMJIT_API virtual ~Target() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get CodeInfo of this runtime.
  //!
  //! CodeInfo can be used to setup a CodeHolder in case you plan to generate a
  //! code compatible and executable by this Runtime.
  inline const CodeInfo& codeInfo() const noexcept { return _codeInfo; }

  //! Get the Runtime's architecture type, see `ArchInfo::Id`.
  inline uint32_t archId() const noexcept { return _codeInfo.archId(); }
  //! Get the Runtime's architecture sub-type, see `ArchInfo::SubId`.
  inline uint32_t archSubId() const noexcept { return _codeInfo.archSubId(); }

  //! Get target type, see `TargetType`.
  inline uint32_t targetType() const noexcept { return _targetType; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t _targetType;                   //!< Tartget type, see `TargetType`.
  uint8_t _reserved[7];                  //!< \internal

  CodeInfo _codeInfo;                    //!< Basic information about the Runtime's code.
};

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_TARGET_H
