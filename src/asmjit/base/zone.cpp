// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// Zlib - See LICENSE.md file in the package.

// [Export]
#define ASMJIT_EXPORTS

// [Dependencies - AsmJit]
#include "../base/intutil.h"
#include "../base/zone.h"

// [Dependencies - C]
#include <stdarg.h>

// [Api-Begin]
#include "../apibegin.h"

namespace asmjit {

//! Zero width chunk used when Zone doesn't have any memory allocated.
static const Zone::Chunk Zone_zeroChunk = {
  NULL, 0, 0, { 0 }
};

// ============================================================================
// [asmjit::Zone - Construction / Destruction]
// ============================================================================

Zone::Zone(size_t chunkSize) {
  _chunks = const_cast<Zone::Chunk*>(&Zone_zeroChunk);
  _chunkSize = chunkSize;
}

Zone::~Zone() {
  reset();
}

// ============================================================================
// [asmjit::Zone - Clear / Reset]
// ============================================================================

void Zone::clear() {
  Chunk* cur = _chunks;

  if (cur == &Zone_zeroChunk)
    return;

  cur = cur->prev;
  while (cur != NULL) {
    Chunk* prev = cur->prev;
    ::free(cur);
    cur = prev;
  }

  _chunks->pos = 0;
  _chunks->prev = NULL;
}

void Zone::reset() {
  Chunk* cur = _chunks;

  if (cur == &Zone_zeroChunk)
    return;

  while (cur != NULL) {
    Chunk* prev = cur->prev;
    ::free(cur);
    cur = prev;
  }

  _chunks = const_cast<Zone::Chunk*>(&Zone_zeroChunk);
}

// ============================================================================
// [asmjit::Zone - Alloc]
// ============================================================================

void* Zone::_alloc(size_t size) {
  Chunk* cur = _chunks;
  ASMJIT_ASSERT(cur == &Zone_zeroChunk || cur->getRemainingSize() < size);

  size_t chunkSize = _chunkSize;
  if (chunkSize < size)
    chunkSize = size;

  cur = static_cast<Chunk*>(::malloc(sizeof(Chunk) - sizeof(void*) + chunkSize));
  if (cur == NULL)
    return NULL;

  cur->prev = NULL;
  cur->pos = 0;
  cur->size = chunkSize;

  if (_chunks != &Zone_zeroChunk)
    cur->prev = _chunks;
  _chunks = cur;

  uint8_t* p = cur->data + cur->pos;
  cur->pos += size;

  ASMJIT_ASSERT(cur->pos <= cur->size);
  return (void*)p;
}

void* Zone::_calloc(size_t size) {
  void* p = _alloc(size);

  if (p != NULL)
    ::memset(p, 0, size);

  return p;
}

void* Zone::dup(const void* data, size_t size) {
  if (data == NULL)
    return NULL;

  if (size == 0)
    return NULL;

  void* m = alloc(size);
  if (m == NULL)
    return NULL;

  ::memcpy(m, data, size);
  return m;
}

char* Zone::sdup(const char* str) {
  if (str == NULL)
    return NULL;

  size_t len = strlen(str);
  if (len == 0)
    return NULL;

  // Include NULL terminator and limit string length.
  if (++len > 256)
    len = 256;

  char* m = static_cast<char*>(alloc(len));
  if (m == NULL)
    return NULL;

  ::memcpy(m, str, len);
  m[len - 1] = '\0';
  return m;
}

char* Zone::sformat(const char* fmt, ...) {
  if (fmt == NULL)
    return NULL;

  char buf[256];
  size_t len;

  va_list ap;
  va_start(ap, fmt);
  len = vsnprintf(buf, 256, fmt, ap);
  va_end(ap);

  len = IntUtil::iMin<size_t>(len, 255);
  buf[len++] = 0;

  return static_cast<char*>(dup(buf, len));
}

} // asmjit namespace

// [Api-End]
#include "../apiend.h"
