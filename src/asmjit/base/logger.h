// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_LOGGER_H
#define _ASMJIT_BASE_LOGGER_H

#include "../build.h"

// [Dependencies - AsmJit]
#include "../base/containers.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

#if !defined(ASMJIT_DISABLE_LOGGER)

// ============================================================================
// [asmjit::LogUtil]
// ============================================================================

// Only used by asmjit internals, not available to consumers.
#if defined(ASMJIT_EXPORTS)
struct LogUtil {
  enum {
    // Has to be big to be able to hold all metadata compiler can assign to a
    // single instruction.
    kMaxCommentLength = 512,
    kMaxInstLength = 40,
    kMaxBinaryLength = 26
  };

  static bool formatLine(
    StringBuilder& sb,
    const uint8_t* binData, size_t binLen, size_t dispLen, size_t imLen, const char* comment) noexcept;
};
#endif // ASMJIT_EXPORTS

// ============================================================================
// [asmjit::Logger]
// ============================================================================

//! Abstract logging class.
//!
//! This class can be inherited and reimplemented to fit into your logging
//! subsystem. When reimplementing use `Logger::log()` method to log into
//! a custom stream.
//!
//! This class also contain `_enabled` member that can be used to enable
//! or disable logging.
struct ASMJIT_VIRTAPI Logger {
  ASMJIT_NO_COPY(Logger)

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Logger options.
  ASMJIT_ENUM(Options) {
    //! Whether to output instructions also in binary form.
    kOptionBinaryForm = 0,

    //! Whether to output immediates as hexadecimal numbers.
    kOptionHexImmediate = 1,
    //! Whether to output displacements as hexadecimal numbers.
    kOptionHexDisplacement = 2,

    //! Count of logger options.
    kOptionCount = 3
  };

  // --------------------------------------------------------------------------
  // [Style]
  // --------------------------------------------------------------------------

  //! Logger style.
  ASMJIT_ENUM(Style) {
    kStyleDefault = 0,
    kStyleDirective = 1,
    kStyleLabel = 2,
    kStyleData = 3,
    kStyleComment = 4,

    kStyleCount = 5
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

  //! Log output.
  virtual void logString(uint32_t style, const char* buf, size_t len = kInvalidIndex) noexcept = 0;

  //! Log formatter message (like sprintf) sending output to `logString()` method.
  ASMJIT_API void logFormat(uint32_t style, const char* fmt, ...) noexcept;
  //! Log binary data.
  ASMJIT_API void logBinary(uint32_t style, const void* data, size_t size) noexcept;

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Get all logger options as a single integer.
  ASMJIT_INLINE uint32_t getOptions() const noexcept { return _options; }

  //! Get the given logger option.
  ASMJIT_INLINE bool hasOption(uint32_t option) const noexcept {
    return (_options & option) != 0;
  }
  ASMJIT_INLINE void addOptions(uint32_t options) noexcept { _options |= options; }
  ASMJIT_INLINE void clearOptions(uint32_t options) noexcept { _options &= ~options; }

  // --------------------------------------------------------------------------
  // [Indentation]
  // --------------------------------------------------------------------------

  //! Get indentation.
  ASMJIT_INLINE const char* getIndentation() const noexcept {
    return _indentation;
  }

  //! Set indentation.
  ASMJIT_API void setIndentation(const char* indentation) noexcept;

  //! Reset indentation.
  ASMJIT_INLINE void resetIndentation() noexcept {
    setIndentation(nullptr);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Options, see \ref LoggerOption.
  uint32_t _options;

  //! Indentation.
  char _indentation[12];
};

// ============================================================================
// [asmjit::FileLogger]
// ============================================================================

//! Logger that can log to standard C `FILE*` stream.
struct ASMJIT_VIRTAPI FileLogger : public Logger {
  ASMJIT_NO_COPY(FileLogger)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Create a new `FileLogger` that logs to a `FILE` stream.
  ASMJIT_API FileLogger(FILE* stream = nullptr) noexcept;

  //! Destroy the `FileLogger`.
  ASMJIT_API virtual ~FileLogger() noexcept;

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! Get `FILE*` stream.
  //!
  //! \note Return value can be `nullptr`.
  ASMJIT_INLINE FILE* getStream() const noexcept {
    return _stream;
  }

  //! Set `FILE*` stream, can be set to `nullptr` to disable logging, although
  //! the `ExternalTool` will still call `logString` even if there is no stream.
  ASMJIT_INLINE void setStream(FILE* stream) noexcept {
    _stream = stream;
  }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual void logString(uint32_t style, const char* buf, size_t len = kInvalidIndex) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! C file stream.
  FILE* _stream;
};

// ============================================================================
// [asmjit::StringLogger]
// ============================================================================

//! String logger.
struct ASMJIT_VIRTAPI StringLogger : public Logger {
  ASMJIT_NO_COPY(StringLogger)

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
  ASMJIT_INLINE const char* getString() const noexcept {
    return _stringBuilder.getData();
  }

  //! Get the length of the string returned by `getString()`.
  ASMJIT_INLINE size_t getLength() const noexcept {
    return _stringBuilder.getLength();
  }

  //! Clear the resulting string.
  ASMJIT_INLINE void clearString() noexcept {
    _stringBuilder.clear();
  }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual void logString(uint32_t style, const char* buf, size_t len = kInvalidIndex) noexcept;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Output.
  StringBuilder _stringBuilder;
};
#else
struct Logger;
#endif // !ASMJIT_DISABLE_LOGGER

//! \}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_LOGGER_H
