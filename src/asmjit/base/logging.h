// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Guard]
#ifndef _ASMJIT_BASE_LOGGING_H
#define _ASMJIT_BASE_LOGGING_H

#include "../asmjit_build.h"

// [Dependencies]
#include "../base/string.h"
#include <stdarg.h>

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

//! \addtogroup asmjit_base
//! \{

#if !defined(ASMJIT_DISABLE_LOGGING)

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Operand_;
class Reg;

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

  static Error formatLine(
    StringBuilder& sb,
    const uint8_t* binData, size_t binLen, size_t dispLen, size_t imLen, const char* comment) noexcept;
};
#endif // ASMJIT_EXPORTS

// ============================================================================
// [asmjit::Formatter]
// ============================================================================

class ASMJIT_VIRTAPI Formatter {
public:
  typedef Error (ASMJIT_CDECL* VirtRegHandlerFunc)(
    StringBuilder& out, uint32_t logOptions, const Reg& r, void* handlerData);

  ASMJIT_API Formatter() noexcept;
  ASMJIT_API virtual ~Formatter() noexcept;

  virtual Error formatRegister(
    StringBuilder& out,
    uint32_t logOptions,
    uint32_t regType,
    uint32_t regId) const noexcept = 0;

  virtual Error formatOperand(
    StringBuilder& out,
    uint32_t logOptions,
    const Operand_& op) const noexcept = 0;

  virtual Error formatInstruction(
    StringBuilder& out,
    uint32_t logOptions,
    uint32_t instId,
    uint32_t options,
    const Operand_& opExtra,
    const Operand_* opArray, uint32_t opCount) const noexcept = 0;

  ASMJIT_INLINE Error formatVirtReg(StringBuilder& out, uint32_t logOptions, const Reg& r) const {
    ASMJIT_ASSERT(hasVirtRegHandler());
    return _virtRegHandlerFunc(out, logOptions, r, _virtRegHandlerData);
  }

  ASMJIT_INLINE bool hasVirtRegHandler() const noexcept { return _virtRegHandlerFunc != nullptr; }

  ASMJIT_INLINE void setVirtRegHandler(VirtRegHandlerFunc func, void* data) noexcept {
    _virtRegHandlerFunc = func;
    _virtRegHandlerData = data;
  }
  ASMJIT_INLINE void resetVirtRegHandler() { setVirtRegHandler(nullptr, nullptr); }

  VirtRegHandlerFunc _virtRegHandlerFunc;
  void* _virtRegHandlerData;
};

// ============================================================================
// [asmjit::Logger]
// ============================================================================

//! Abstract logging class.
//!
//! This class can be inherited and reimplemented to fit into your logging
//! subsystem. When reimplementing use `Logger::log()` method to log into
//! a custom stream.
class ASMJIT_VIRTAPI Logger {
public:
  ASMJIT_NONCOPYABLE(Logger)

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Logger options.
  ASMJIT_ENUM(Options) {
    kOptionBinaryForm      = 0x00000001, //! Output instructions also in binary form.
    kOptionHexImmediate    = 0x00000002, //! Output constants in hexadecimal form.
    kOptionHexDisplacement = 0x00000004  //! Output displacements in hexadecimal form.
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
  virtual Error log(const char* buf, size_t len = kInvalidIndex) noexcept = 0;

  //! Format the message by using "sprintf()" and then send to `log()`.
  ASMJIT_API Error logf(const char* fmt, ...) noexcept;
  //! Format the message by using "vsprintf()" and then send to `log()`.
  ASMJIT_API Error logv(const char* fmt, va_list ap) noexcept;
  //! Log binary data.
  ASMJIT_API Error logBinary(const void* data, size_t size) noexcept;

  // --------------------------------------------------------------------------
  // [Options]
  // --------------------------------------------------------------------------

  //! Get all logger options as a single integer.
  ASMJIT_INLINE uint32_t getOptions() const noexcept { return _options; }
  //! Get the given logger option.
  ASMJIT_INLINE bool hasOption(uint32_t option) const noexcept { return (_options & option) != 0; }
  ASMJIT_INLINE void addOptions(uint32_t options) noexcept { _options |= options; }
  ASMJIT_INLINE void clearOptions(uint32_t options) noexcept { _options &= ~options; }

  // --------------------------------------------------------------------------
  // [Indentation]
  // --------------------------------------------------------------------------

  //! Get indentation.
  ASMJIT_INLINE const char* getIndentation() const noexcept { return _indentation; }
  //! Set indentation.
  ASMJIT_API void setIndentation(const char* indentation) noexcept;
  //! Reset indentation.
  ASMJIT_INLINE void resetIndentation() noexcept { setIndentation(nullptr); }

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

//! Logger that can log to a `FILE*` stream.
class ASMJIT_VIRTAPI FileLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(FileLogger)

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

  //! Get the logging out put stream or null.
  ASMJIT_INLINE FILE* getStream() const noexcept { return _stream; }

  //! Set the logging output stream to `stream` or null.
  //!
  //! NOTE: If the `stream` is null it will disable logging, but it won't
  //! stop calling `log()` unless the logger is detached from the
  //! \ref Assembler.
  ASMJIT_INLINE void setStream(FILE* stream) noexcept { _stream = stream; }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error log(const char* buf, size_t len = kInvalidIndex) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! C file stream.
  FILE* _stream;
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
  ASMJIT_INLINE const char* getString() const noexcept { return _stringBuilder.getData(); }
  //! Clear the resulting string.
  ASMJIT_INLINE void clearString() noexcept { _stringBuilder.clear(); }

  //! Get the length of the string returned by `getString()`.
  ASMJIT_INLINE size_t getLength() const noexcept { return _stringBuilder.getLength(); }

  // --------------------------------------------------------------------------
  // [Logging]
  // --------------------------------------------------------------------------

  ASMJIT_API virtual Error log(const char* buf, size_t len = kInvalidIndex) noexcept override;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! Output string.
  StringBuilder _stringBuilder;
};
#else
class Formatter;
class Logger;
#endif // !ASMJIT_DISABLE_LOGGING

//! \}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // _ASMJIT_BASE_LOGGER_H
