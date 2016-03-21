// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../build.h"
#if !defined(ASMJIT_DISABLE_LOGGER)

// [Dependencies - AsmJit]
#include "../base/containers.h"
#include "../base/logger.h"
#include "../base/utils.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::LogUtil]
// ============================================================================

bool LogUtil::formatLine(StringBuilder& sb, const uint8_t* binData, size_t binLen, size_t dispLen, size_t imLen, const char* comment) noexcept {
  size_t currentLen = sb.getLength();
  size_t commentLen = comment ? Utils::strLen(comment, kMaxCommentLength) : 0;

  ASMJIT_ASSERT(binLen >= dispLen);

  if ((binLen != 0 && binLen != kInvalidIndex) || commentLen) {
    size_t align = kMaxInstLength;
    char sep = ';';

    for (size_t i = (binLen == kInvalidIndex); i < 2; i++) {
      size_t begin = sb.getLength();

      // Append align.
      if (currentLen < align) {
        if (!sb.appendChars(' ', align - currentLen))
          return false;
      }

      // Append separator.
      if (sep) {
        if (!(sb.appendChar(sep) & sb.appendChar(' ')))
          return false;
      }

      // Append binary data or comment.
      if (i == 0) {
        if (!sb.appendHex(binData, binLen - dispLen - imLen))
          return false;
        if (!sb.appendChars('.', dispLen * 2))
          return false;
        if (!sb.appendHex(binData + binLen - imLen, imLen))
          return false;
        if (commentLen == 0)
          break;
      }
      else {
        if (!sb.appendString(comment, commentLen))
          return false;
      }

      currentLen += sb.getLength() - begin;
      align += kMaxBinaryLength;
      sep = '|';
    }
  }

  return sb.appendChar('\n');
}

// ============================================================================
// [asmjit::Logger - Construction / Destruction]
// ============================================================================

Logger::Logger() noexcept {
  _options = 0;
  ::memset(_indentation, 0, ASMJIT_ARRAY_SIZE(_indentation));
}

Logger::~Logger() noexcept {}

// ============================================================================
// [asmjit::Logger - Logging]
// ============================================================================

void Logger::logFormat(uint32_t style, const char* fmt, ...) noexcept {
  char buf[1024];
  size_t len;

  va_list ap;
  va_start(ap, fmt);
  len = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if (len >= sizeof(buf))
    len = sizeof(buf) - 1;

  logString(style, buf, len);
}

void Logger::logBinary(uint32_t style, const void* data, size_t size) noexcept {
  static const char prefix[] = ".data ";
  static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

  const uint8_t* s = static_cast<const uint8_t*>(data);
  size_t i = size;

  char buffer[128];
  ::memcpy(buffer, prefix, ASMJIT_ARRAY_SIZE(prefix) - 1);

  while (i) {
    uint32_t n = static_cast<uint32_t>(Utils::iMin<size_t>(i, 16));
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
// [asmjit::Logger - Indentation]
// ============================================================================

void Logger::setIndentation(const char* indentation) noexcept {
  ::memset(_indentation, 0, ASMJIT_ARRAY_SIZE(_indentation));
  if (!indentation)
    return;

  size_t length = Utils::strLen(indentation, ASMJIT_ARRAY_SIZE(_indentation) - 1);
  ::memcpy(_indentation, indentation, length);
}

// ============================================================================
// [asmjit::FileLogger - Construction / Destruction]
// ============================================================================

FileLogger::FileLogger(FILE* stream) noexcept : _stream(nullptr) { setStream(stream); }
FileLogger::~FileLogger() noexcept {}

// ============================================================================
// [asmjit::FileLogger - Logging]
// ============================================================================

void FileLogger::logString(uint32_t style, const char* buf, size_t len) noexcept {
  if (!_stream)
    return;

  if (len == kInvalidIndex)
    len = strlen(buf);

  fwrite(buf, 1, len, _stream);
}

// ============================================================================
// [asmjit::StringLogger - Construction / Destruction]
// ============================================================================

StringLogger::StringLogger() noexcept {}
StringLogger::~StringLogger() noexcept {}

// ============================================================================
// [asmjit::StringLogger - Logging]
// ============================================================================

void StringLogger::logString(uint32_t style, const char* buf, size_t len) noexcept {
  _stringBuilder.appendString(buf, len);
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGER
