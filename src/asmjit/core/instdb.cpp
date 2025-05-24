// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/instdb_p.h"

ASMJIT_BEGIN_NAMESPACE

namespace InstNameUtils {

static constexpr uint32_t kBufferSize = 32;

static ASMJIT_INLINE_CONSTEXPR char decode5BitChar(uint32_t c) noexcept {
  uint32_t base = c <= 26 ? uint32_t('a') - 1u : uint32_t('0') - 27u;
  return char(base + c);
}

static ASMJIT_INLINE size_t decodeToBuffer(char nameOut[kBufferSize], uint32_t nameValue, InstStringifyOptions options, const char* stringTable) noexcept {
  size_t i;

  if (nameValue & 0x80000000u) {
    // Small string of 5-bit characters.
    //
    // NOTE: Small string optimization never provides additional
    // aliases formatting, so we don't have to consider `options`.
    for (i = 0; i < 6; i++, nameValue >>= 5) {
      uint32_t c = nameValue & 0x1F;
      if (c == 0)
        break;
      nameOut[i] = decode5BitChar(c);
    }
    return i;
  }
  else {
    size_t prefixBase = nameValue & 0xFFFu;
    size_t prefixSize = (nameValue >> 12) & 0xFu;

    size_t suffixBase = (nameValue >> 16) & 0xFFFu;
    size_t suffixSize = (nameValue >> 28) & 0x7u;

    if (Support::test(options, InstStringifyOptions::kAliases) && suffixBase == 0xFFFu) {
      // Alias formatting immediately follows the instruction name in string table.
      // The first character specifies the length and then string data follows.
      prefixBase += prefixSize;
      prefixSize = uint8_t(stringTable[prefixBase]);
      ASMJIT_ASSERT(prefixSize <= kBufferSize);

      prefixBase += 1; // Skip the byte that specifies the length of a formatted alias.
    }

    for (i = 0; i < prefixSize; i++) {
      nameOut[i] = stringTable[prefixBase + i];
    }

    char* suffixOut = nameOut + prefixSize;
    for (i = 0; i < suffixSize; i++) {
      suffixOut[i] = stringTable[suffixBase + i];
    }

    return prefixSize + suffixSize;
  }
}

Error decode(uint32_t nameValue, InstStringifyOptions options, const char* stringTable, String& output) noexcept {
  char nameData[kBufferSize];
  size_t nameSize = decodeToBuffer(nameData, nameValue, options, stringTable);

  return output.append(nameData, nameSize);
}

InstId findInstruction(const char* s, size_t len, const uint32_t* nameTable, const char* stringTable, const InstNameIndex& nameIndex) noexcept {
  ASMJIT_ASSERT(s != nullptr);
  ASMJIT_ASSERT(len > 0u);

  uint32_t prefix = uint32_t(s[0]) - uint32_t('a');
  if (ASMJIT_UNLIKELY(prefix > uint32_t('z') - uint32_t('a'))) {
    return BaseInst::kIdNone;
  }

  size_t base = nameIndex.data[prefix].start;
  size_t end = nameIndex.data[prefix].end;

  if (ASMJIT_UNLIKELY(!base)) {
    return BaseInst::kIdNone;
  }

  char nameData[kBufferSize];
  for (size_t lim = end - base; lim != 0; lim >>= 1) {
    size_t instId = base + (lim >> 1);
    size_t nameSize = decodeToBuffer(nameData, nameTable[instId], InstStringifyOptions::kNone, stringTable);

    int result = Support::compareStringViews(s, len, nameData, nameSize);
    if (result < 0) {
      continue;
    }

    if (result > 0) {
      base = instId + 1;
      lim--;
      continue;
    }

    return InstId(instId);
  }

  return BaseInst::kIdNone;
}


uint32_t findAlias(const char* s, size_t len, const uint32_t* nameTable, const char* stringTable, uint32_t aliasNameCount) noexcept {
  ASMJIT_ASSERT(s != nullptr);
  ASMJIT_ASSERT(len > 0u);

  size_t base = 0;
  char nameData[kBufferSize];

  for (size_t lim = size_t(aliasNameCount) - base; lim != 0; lim >>= 1) {
    size_t index = base + (lim >> 1);
    size_t nameSize = decodeToBuffer(nameData, nameTable[index], InstStringifyOptions::kNone, stringTable);

    int result = Support::compareStringViews(s, len, nameData, nameSize);
    if (result < 0) {
      continue;
    }

    if (result > 0) {
      base = index + 1;
      lim--;
      continue;
    }

    return uint32_t(index);
  }

  return Globals::kInvalidId;
}

} // {InstNameUtils}

ASMJIT_END_NAMESPACE
