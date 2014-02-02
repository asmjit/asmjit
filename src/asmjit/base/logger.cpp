// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/intutil.h"
#include "../base/logger.h"
#include "../base/string.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../base/apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::BaseLogger - Construction / Destruction]
// ============================================================================

BaseLogger::BaseLogger() {
  _options = 0;
  ::memset(_indentation, 0, ASMJIT_ARRAY_SIZE(_indentation));
}

BaseLogger::~BaseLogger() {}

// ============================================================================
// [asmjit::BaseLogger - Logging]
// ============================================================================

void BaseLogger::logFormat(uint32_t style, const char* fmt, ...) {
  char buf[1024];
  size_t len;

  va_list ap;
  va_start(ap, fmt);
  len = vsnprintf(buf, 1023, fmt, ap);
  va_end(ap);

  logString(style, buf, len);
}

void BaseLogger::logBinary(uint32_t style, const void* data, size_t size) {
  static const char prefix[] = ".data ";
  static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

  const uint8_t* s = static_cast<const uint8_t*>(data);
  size_t i = size;

  char buffer[128];
  ::memcpy(buffer, prefix, ASMJIT_ARRAY_SIZE(prefix) - 1);

  while (i) {
    uint32_t n = static_cast<uint32_t>(IntUtil::iMax<size_t>(i, 16));
    char* p = buffer + ASMJIT_ARRAY_SIZE(prefix) - 1;

    i -= n;
    do {
      uint32_t c = s[0];

      p[0] = hex[c >> 4];
      p[1] = hex[c & 15];

      p += 2;
      s += 1;
    } while (--n);

    *p++ = '\n';
    logString(style, buffer, (size_t)(p - buffer));
  }
}

// ============================================================================
// [asmjit::BaseLogger - LogBinary]
// ============================================================================

void BaseLogger::setOption(uint32_t id, bool value) {
  if (id >= kLoggerOptionCount)
    return;

  uint32_t mask = 1 << id;

  if (value)
    _options |= mask;
  else
    _options &= ~mask;
}

// ============================================================================
// [asmjit::BaseLogger - Indentation]
// ============================================================================

void BaseLogger::setIndentation(const char* indentation) {
  ::memset(_indentation, 0, ASMJIT_ARRAY_SIZE(_indentation));
  if (!indentation)
    return;

  size_t length = StringUtil::nlen(indentation, ASMJIT_ARRAY_SIZE(_indentation) - 1);
  ::memcpy(_indentation, indentation, length);
}

// ============================================================================
// [asmjit::FileLogger - Construction / Destruction]
// ============================================================================

FileLogger::FileLogger(FILE* stream) : _stream(NULL) {
  setStream(stream);
}

FileLogger::~FileLogger() {}

// ============================================================================
// [asmjit::FileLogger - Accessors]
// ============================================================================

//! @brief Set file stream.
void FileLogger::setStream(FILE* stream) {
  _stream = stream;
}

// ============================================================================
// [asmjit::FileLogger - Logging]
// ============================================================================

void FileLogger::logString(uint32_t style, const char* buf, size_t len) {
  if (!_stream)
    return;

  if (len == kInvalidIndex)
    len = strlen(buf);

  fwrite(buf, 1, len, _stream);
}

// ============================================================================
// [asmjit::StringLogger - Construction / Destruction]
// ============================================================================

StringLogger::StringLogger() {}
StringLogger::~StringLogger() {}

// ============================================================================
// [asmjit::StringLogger - Logging]
// ============================================================================

void StringLogger::logString(uint32_t style, const char* buf, size_t len) {
  _stringBuilder.appendString(buf, len);
}

} // asmjit namespace

// [Api-End]
#include "../base/apiend.h"
