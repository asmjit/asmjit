// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Guard]
#include "../asmjit_build.h"
#if !defined(ASMJIT_DISABLE_LOGGING)

// [Dependencies]
#include "../base/logging.h"
#include "../base/utils.h"
#include <stdarg.h>

// [Api-Begin]
#include "../asmjit_apibegin.h"

namespace asmjit {

// ============================================================================
// [asmjit::LogUtil]
// ============================================================================

Error LogUtil::formatLine(StringBuilder& sb, const uint8_t* binData, size_t binLen, size_t dispLen, size_t imLen, const char* comment) noexcept {
  size_t currentLen = sb.getLength();
  size_t commentLen = comment ? Utils::strLen(comment, kMaxCommentLength) : 0;

  ASMJIT_ASSERT(binLen >= dispLen);

  if ((binLen != 0 && binLen != kInvalidIndex) || commentLen) {
    size_t align = kMaxInstLength;
    char sep = ';';

    for (size_t i = (binLen == kInvalidIndex); i < 2; i++) {
      size_t begin = sb.getLength();

      // Append align.
      if (currentLen < align)
        ASMJIT_PROPAGATE(sb.appendChars(' ', align - currentLen));

      // Append separator.
      if (sep) {
        ASMJIT_PROPAGATE(sb.appendChar(sep));
        ASMJIT_PROPAGATE(sb.appendChar(' '));
      }

      // Append binary data or comment.
      if (i == 0) {
        ASMJIT_PROPAGATE(sb.appendHex(binData, binLen - dispLen - imLen));
        ASMJIT_PROPAGATE(sb.appendChars('.', dispLen * 2));
        ASMJIT_PROPAGATE(sb.appendHex(binData + binLen - imLen, imLen));
        if (commentLen == 0) break;
      }
      else {
        ASMJIT_PROPAGATE(sb.appendString(comment, commentLen));
      }

      currentLen += sb.getLength() - begin;
      align += kMaxBinaryLength;
      sep = '|';
    }
  }

  return sb.appendChar('\n');
}

// ============================================================================
// [asmjit::Formatter - Construction / Destruction]
// ============================================================================

Formatter::Formatter() noexcept
  : _virtRegHandlerFunc(nullptr),
    _virtRegHandlerData(nullptr) {}
Formatter::~Formatter() noexcept {}

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

Error Logger::logf(const char* fmt, ...) noexcept {
  Error err;

  va_list ap;
  va_start(ap, fmt);
  err = logv(fmt, ap);
  va_end(ap);

  return err;
}

Error Logger::logv(const char* fmt, va_list ap) noexcept {
  char buf[1024];
  size_t len = vsnprintf(buf, sizeof(buf), fmt, ap);

  if (len >= sizeof(buf))
    len = sizeof(buf) - 1;
  return log(buf, len);
}

Error Logger::logBinary(const void* data, size_t size) noexcept {
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
    ASMJIT_PROPAGATE(log(buffer, (size_t)(p - buffer)));
  }

  return kErrorOk;
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

Error FileLogger::log(const char* buf, size_t len) noexcept {
  if (!_stream)
    return kErrorOk;

  if (len == kInvalidIndex)
    len = strlen(buf);

  fwrite(buf, 1, len, _stream);
  return kErrorOk;
}

// ============================================================================
// [asmjit::StringLogger - Construction / Destruction]
// ============================================================================

StringLogger::StringLogger() noexcept {}
StringLogger::~StringLogger() noexcept {}

// ============================================================================
// [asmjit::StringLogger - Logging]
// ============================================================================

Error StringLogger::log(const char* buf, size_t len) noexcept {
  return _stringBuilder.appendString(buf, len);
}

} // asmjit namespace

// [Api-End]
#include "../asmjit_apiend.h"

// [Guard]
#endif // !ASMJIT_DISABLE_LOGGING
