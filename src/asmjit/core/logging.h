// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_CORE_LOGGING_H
#define _ASMJIT_CORE_LOGGING_H

// [Dependencies]
#include "../core/inst.h"
#include "../core/stringbuilder.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

#ifndef ASMJIT_DISABLE_LOGGING

// ============================================================================
// [Forward Declarations]
// ============================================================================

class CodeEmitter;
class Logger;
class Reg;
struct Operand_;

#ifndef ASMJIT_DISABLE_BUILDER
class CodeBuilder;
class CBNode;
#endif

// ============================================================================
// [asmjit::Logger]
// ============================================================================

//! Abstract logging interface and helpers.
//!
//! This class can be inherited and reimplemented to fit into your logging
//! subsystem. When reimplementing use `Logger::_log()` method to log into
//! a custom stream.
//!
//! There are two \ref Logger implementations offered by AsmJit:
//!   - \ref FileLogger - allows to log into `std::FILE*`.
//!   - \ref StringLogger - logs into a `StringBuilder`.
class ASMJIT_VIRTAPI Logger {
public:
  ASMJIT_NONCOPYABLE(Logger)

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Logger options.
  enum Options : uint32_t {
    kOptionBinaryForm      = 0x00000001, //!< Show also binary form of each logged instruction.
    kOptionExplainConsts   = 0x00000002, //!< Show a text explanation of some constants.
    kOptionHexConsts       = 0x00000004, //!< Use hexadecimal notation to output constants.
    kOptionHexOffsets      = 0x00000008, //!< Use hexadecimal notation to output offsets.
    kOptionAnnotate        = 0x00000010, //!< Annotate nodes that are lowered by CodeCompiler passes.
    kOptionRegCasts        = 0x00000020, //!< Show casts of virtual registers.
    kOptionNodePosition    = 0x00000040, //!< Show a node position of CodeBuilder/CodeCompiler instructions.
    kOptionDebugPasses     = 0x00000080, //!< Show an additional output from passes.
    kOptionDebugRA         = 0x00000100  //!< Show an additional output from RA.
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a `Logger` instance.
  ASMJIT_API Logger() noexcept;
  //! Destroy the `Logger` instance.
  ASMJIT_API virtual ~Logger() noexcept;

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  //! Log `str` - must be reimplemented.
  virtual Error _log(const char* str, size_t len) noexcept = 0;

  //! Log a string `str`, which is either null terminated or having `len` length.
  inline Error log(const char* str, size_t len = Globals::kNullTerminated) noexcept { return _log(str, len); }
  //! Log a content of a `StringBuilder` `str`.
  inline Error log(const StringBuilder& str) noexcept { return _log(str.getData(), str.getLength()); }

  //! Format the message by using `std::snprintf()` and then send to `log()`.
  ASMJIT_API Error logf(const char* fmt, ...) noexcept;
  //! Format the message by using `std::vsnprintf()` and then send to `log()`.
  ASMJIT_API Error logv(const char* fmt, std::va_list ap) noexcept;
  //! Log binary data.
  ASMJIT_API Error logBinary(const void* data, size_t size) noexcept;

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Get all logger options as a single integer.
  inline uint32_t getOptions() const noexcept { return _options; }
  //! Get the given logger option.
  inline bool hasOption(uint32_t option) const noexcept { return (_options & option) != 0; }
  inline void addOptions(uint32_t options) noexcept { _options |= options; }
  inline void clearOptions(uint32_t options) noexcept { _options &= ~options; }

  // --------------------------------------------------------------------------
  // [Indentation]
  // --------------------------------------------------------------------------

  //! Get indentation.
  inline const char* getIndentation() const noexcept { return _indentation; }
  //! Set indentation.
  ASMJIT_API void setIndentation(const char* indentation) noexcept;
  //! Reset indentation.
  inline void resetIndentation() noexcept { setIndentation(nullptr); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Options, see \ref Options.
  uint32_t _options;

  //! Indentation.
  char _indentation[12];
};

// ============================================================================
// [asmjit::FileLogger]
// ============================================================================

//! Logger that can log to a `std::FILE*`.
class ASMJIT_VIRTAPI FileLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(FileLogger)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `FileLogger` that logs to `std::FILE*`.
  ASMJIT_API FileLogger(std::FILE* stream = nullptr) noexcept;
  //! Destroy the `FileLogger`.
  ASMJIT_API virtual ~FileLogger() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get the logging out put stream or null.
  inline std::FILE* getStream() const noexcept { return _stream; }

  //! Set the logging output stream to `stream` or null.
  //!
  //! NOTE: If the `stream` is null it will disable logging, but it won't
  //! stop calling `log()` unless the logger is detached from the
  //! \ref Assembler.
  inline void setStream(std::FILE* stream) noexcept { _stream = stream; }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _log(const char* buf, size_t len = Globals::kNullTerminated) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! `std::FILE*` stream.
  std::FILE* _stream;
};

// ============================================================================
// [asmjit::StringLogger]
// ============================================================================

//! Logger that stores everything in an internal string buffer.
class ASMJIT_VIRTAPI StringLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(StringLogger)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create new `StringLogger`.
  ASMJIT_API StringLogger() noexcept;
  //! Destroy the `StringLogger`.
  ASMJIT_API virtual ~StringLogger() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get `char*` pointer which represents the resulting string.
  //!
  //! The pointer is owned by `StringLogger`, it can't be modified or freed.
  inline const char* getString() const noexcept { return _stringBuilder.getData(); }
  //! Clear the resulting string.
  inline void clearString() noexcept { _stringBuilder.clear(); }

  //! Get the length of the string returned by `getString()`.
  inline size_t getLength() const noexcept { return _stringBuilder.getLength(); }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API Error _log(const char* buf, size_t len = Globals::kNullTerminated) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Output string.
  StringBuilder _stringBuilder;
};

// ============================================================================
// [asmjit::Logging]
// ============================================================================

struct Logging {
  ASMJIT_API static Error formatRegister(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t archType,
    uint32_t regType,
    uint32_t regId) noexcept;

  ASMJIT_API static Error formatLabel(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t labelId) noexcept;

  ASMJIT_API static Error formatOperand(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t archType,
    const Operand_& op) noexcept;

  ASMJIT_API static Error formatInstruction(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeEmitter* emitter,
    uint32_t archType,
    const Inst::Detail& detail, const Operand_* operands, uint32_t count) noexcept;

  ASMJIT_API static Error formatTypeId(
    StringBuilder& sb,
    uint32_t typeId) noexcept;

  #ifndef ASMJIT_DISABLE_BUILDER
  ASMJIT_API static Error formatNode(
    StringBuilder& sb,
    uint32_t logOptions,
    const CodeBuilder* cb,
    const CBNode* node_) noexcept;
  #endif

  // Only used by AsmJit internals, not available to users.
  #if defined(ASMJIT_EXPORTS)
  enum {
    // Has to be big to be able to hold all metadata compiler can assign to a
    // single instruction.
    kMaxCommentLength = 512,
    kMaxInstLength = 44,
    kMaxBinaryLength = 26
  };

  static Error formatLine(
    StringBuilder& sb,
    const uint8_t* binData, size_t binLen, size_t dispLen, size_t imLen, const char* comment) noexcept;
  #endif
};
#endif

//! \}

ASMJIT_END_NAMESPACE

// [Guard]
#endif // _ASMJIT_CORE_LOGGER_H
