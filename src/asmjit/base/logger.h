// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_LOGGER_H
#define _ASMJIT_BASE_LOGGER_H

// [Dependencies - AsmJit]
#include "../base/defs.h"
#include "../base/string.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

//! @addtogroup asmjit_logging
//! @{

// ============================================================================
// [asmjit::kLoggerOption]
// ============================================================================

//! @brief Logger options.
ASMJIT_ENUM(kLoggerOption) {
  //! @brief Whether to output instructions also in binary form.
  kLoggerOptionBinaryForm = 0,

  //! @brief Whether to output immediates as hexadecimal numbers.
  kLoggerOptionHexImmediate = 1,
  //! @brief Whether to output displacements as hexadecimal numbers.
  kLoggerOptionHexDisplacement = 2,

  //! @brief Count of logger options.
  kLoggerOptionCount = 3
};

// ============================================================================
// [asmjit::kLoggerStyle]
// ============================================================================

ASMJIT_ENUM(kLoggerStyle) {
  kLoggerStyleDefault = 0,
  kLoggerStyleDirective = 1,
  kLoggerStyleLabel = 2,
  kLoggerStyleData = 3,
  kLoggerStyleComment = 4,

  kLoggerStyleCount = 5
};

// ============================================================================
// [asmjit::Logger]
// ============================================================================

//! @brief Abstract logging class.
//!
//! This class can be inherited and reimplemented to fit into your logging
//! subsystem. When reimplementing use @c asmjit::Logger::log() method to
//! log into your stream.
//!
//! This class also contain @c _enabled member that can be used to enable
//! or disable logging.
struct BaseLogger {
  ASMJIT_NO_COPY(BaseLogger)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a @ref BaseLogger instance.
  ASMJIT_API BaseLogger();
  //! @brief Destroy the @ref BaseLogger instance.
  ASMJIT_API virtual ~BaseLogger();

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  //! @brief Abstract method to log output.
  //!
  //! Default implementation that is in @c asmjit::Logger is to do nothing.
  //! It's virtual to fit to your logging system.
  virtual void logString(uint32_t style, const char* buf, size_t len = kInvalidIndex) = 0;

  //! @brief Log formatter message (like sprintf) sending output to @c logString() method.
  ASMJIT_API void logFormat(uint32_t style, const char* fmt, ...);
  //! @brief Log binary data.
  ASMJIT_API void logBinary(uint32_t style, const void* data, size_t size);

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! @brief Get all logger options as a single integer.
  ASMJIT_INLINE uint32_t getOptions() const
  { return _options; }

  //! @brief Get the given logger option.
  ASMJIT_INLINE bool getOption(uint32_t id) const {
    ASMJIT_ASSERT(id < kLoggerOptionCount);
    return static_cast<bool>((_options >> id) & 0x1);
  }

  //! @brief Set the given logger option.
  ASMJIT_API void setOption(uint32_t id, bool value);

  // --------------------------------------------------------------------------
  // [Indentation]
  // --------------------------------------------------------------------------

  //! @brief Get indentation.
  ASMJIT_INLINE const char* getIndentation() const { return _indentation; }
  //! @brief Set indentation.
  ASMJIT_API void setIndentation(const char* indentation);
  //! @brief Reset indentation.
  ASMJIT_INLINE void resetIndentation() { setIndentation(NULL); }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Options, see @ref kLoggerOption.
  uint32_t _options;

  //! @brief Indentation.
  char _indentation[12];
};

// ============================================================================
// [asmjit::FileLogger]
// ============================================================================

//! @brief Logger that can log to standard C @c FILE* stream.
struct FileLogger : public BaseLogger {
  ASMJIT_NO_COPY(FileLogger)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create a new @c FileLogger.
  //! @param stream FILE stream where logging will be sent (can be @c NULL
  //! to disable logging).
  ASMJIT_API FileLogger(FILE* stream = NULL);

  //! @brief Destroy the @ref FileLogger.
  ASMJIT_API virtual ~FileLogger();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get @c FILE* stream.
  //!
  //! @note Return value can be @c NULL.
  ASMJIT_INLINE FILE* getStream() const { return _stream; }

  //! @brief Set @c FILE* stream.
  //!
  //! @param stream @c FILE stream where to log output (can be @c NULL to
  //! disable logging).
  ASMJIT_API void setStream(FILE* stream);

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual void logString(uint32_t style, const char* buf, size_t len = kInvalidIndex);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief C file stream.
  FILE* _stream;
};

// ============================================================================
// [asmjit::StringLogger]
// ============================================================================

//! @brief String logger.
struct StringLogger : public BaseLogger {
  ASMJIT_NO_COPY(StringLogger)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Create new @ref StringLogger.
  ASMJIT_API StringLogger();

  //! @brief Destroy the @ref StringLogger.
  ASMJIT_API virtual ~StringLogger();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get <code>char*</code> pointer which represents the resulting
  //! string.
  //!
  //! The pointer is owned by @ref StringLogger, it can't be modified or freed.
  ASMJIT_INLINE const char* getString() const { return _stringBuilder.getData(); }

  //! @brief Clear the resulting string.
  ASMJIT_INLINE void clearString() { _stringBuilder.clear(); }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual void logString(uint32_t style, const char* buf, size_t len = kInvalidIndex);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Output.
  StringBuilder _stringBuilder;
};

//! @}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_LOGGER_H
