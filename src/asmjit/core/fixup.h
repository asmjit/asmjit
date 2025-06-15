// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_CORE_FIXUP_H_INCLUDED
#define ASMJIT_CORE_FIXUP_H_INCLUDED

#include "../core/globals.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

//! Offset format type, used by \ref OffsetFormat.
enum class OffsetType : uint8_t {
  // Common Offset Formats
  // ---------------------

  //! A value having `_immBitCount` bits and shifted by `_immBitShift`.
  //!
  //! This offset type is sufficient for many targets that store offset as a continuous set bits within an
  //! instruction word / sequence of bytes.
  kSignedOffset,

  //! An unsigned value having `_immBitCount` bits and shifted by `_immBitShift`.
  kUnsignedOffset,

  // AArch64 Specific Offset Formats
  // -------------------------------

  //! AArch64 ADR format of `[.|immlo:2|.....|immhi:19|.....]`.
  kAArch64_ADR,

  //! AArch64 ADRP format of `[.|immlo:2|.....|immhi:19|.....]` (4kB pages).
  kAArch64_ADRP,

  // AArch32 Specific Offset Formats (T16 & T32)
  // -------------------------------------------

  //! AArch32 THUMBv2 immediate encoding of 'ADR' instruction (12-bit payload and sign bit):
  //!
  //!   `|.....|imm:1|..N.N|......|imm:3|....|imm:8|`
  //!
  //! Where `N` is one if the offset is negative. The immediate is encoded as absolute value of the offset if negative.
  kThumb32_ADR,

  //! AArch32 THUMBv2 immediate encoding of 'BLX' instruction (23-bit immediate payload, multiplied by 4):
  //!
  //!   `|.....|imm[22]|imm[19:10]|..|ja|1|jb|imm[9:0]|0`
  //!
  //! Where:
  //!
  //!   - `ja` is calculated as imm[22] ^ imm[21] ^ 1.
  //!   - `jb` is calculated as imm[22] ^ imm[20] ^ 1.
  kThumb32_BLX,

  //! AArch32 THUMBv2 immediate encoding of 'B' instruction without `<cond>` (24-bit immediate payload, multiplied by 2):
  //!
  //!   `|.....|imm[23]|imm[20:11]|..|ja|1|jb|imm[10:0]`
  //!
  //! Where:
  //!
  //!   - `ja` is calculated as imm[23] ^ imm[22] ^ 1.
  //!   - `jb` is calculated as imm[23] ^ imm[21] ^ 1.
  kThumb32_B,

  //! AArch32 THUMBv2 immediate encoding of 'B' instruction with `<cond>` (20-bit immediate payload, multiplied by 2).
  //!
  //!   `|.....|imm[19]|....|imm[16:11]|..|ja|1|jb|imm[10:0]`
  //!
  //! Where:
  //!
  //!   - `ja` is calculated as imm[19] ^ imm[18] ^ 1.
  //!   - `jb` is calculated as imm[19] ^ imm[17] ^ 1.
  kThumb32_BCond,

  // AArch32 Specific Offset Formats (A32)
  // -------------------------------------

  //! AArch32 ADR instruction, which uses a standard 12-bit immediate encoding that is used by other ARM instructions.
  kAArch32_ADR,

  //! AArch32 signed offset that is similar to `kSignedOffset`, however it uses absolute value of the offset and its
  //! sign is encoded in 23rd bit of the opcode.
  //!
  //!   `|........|U.......|........|........|`
  //!
  kAArch32_U23_SignedOffset,

  //! AArch32 offset format that encodes 8-bit offset as:
  //!
  //!   `|........|U.......|....|imm[7:4]|....|imm[3:0]|`
  //!
  //! in a 32-bit word, where U is a sign of the displacement and the displacement itself is encoded as its absolute
  //! value.
  kAArch32_U23_0To3At0_4To7At8,

  //! AArch32 offset format that encodes a signed 25-bit offset as:
  //!
  //!   `|.......|imm[0]|imm[24:1]|`
  //!
  //! in a 32-bit word.
  kAArch32_1To24At0_0At24,

  //! Maximum value of `OffsetFormatType`.
  kMaxValue = kAArch32_1To24At0_0At24
};

//! Provides information about formatting offsets, absolute addresses, or their parts. Offset format is used by both
//! \ref RelocEntry and \ref Fixup. The illustration below describes the relation of region size and offset size.
//! Region size is the size of the whole unit whereas offset size is the size of the unit that will be patched.
//!
//! ```
//! +-> Code buffer |   The subject of the relocation (region)  |
//! |               | (Word-Offset)  (Word-Size)                |
//! |xxxxxxxxxxxxxxx|................|*PATCHED*|................|xxxxxxxxxxxx->
//!                                  |         |
//!     [Word Offset points here]----+         +--- [WordOffset + WordSize]
//! ```
//!
//! Once the offset word has been located it can be patched like this:
//!
//! ```
//!                               |ImmDiscardLSB (discard LSB bits).
//!                               |..
//! [0000000000000iiiiiiiiiiiiiiiiiDD] - Offset value (32-bit)
//! [000000000000000iiiiiiiiiiiiiiiii] - Offset value after discard LSB.
//! [00000000000iiiiiiiiiiiiiiiii0000] - Offset value shifted by ImmBitShift.
//! [xxxxxxxxxxxiiiiiiiiiiiiiiiiixxxx] - Patched word (32-bit)
//!             |...............|
//!               (ImmBitCount) +- ImmBitShift
//! ```
struct OffsetFormat {
  //! \name Members
  //! \{

  //! Type of the offset.
  OffsetType _type;
  //! Encoding flags.
  uint8_t _flags;
  //! Size of the region (in bytes) containing the offset value, if the offset value is part of an instruction,
  //! otherwise it would be the same as `_valueSize`.
  uint8_t _regionSize;
  //! Size of the offset value, in bytes (1, 2, 4, or 8).
  uint8_t _valueSize;
  //! Offset of the offset value, in bytes, relative to the start of the region or data. Value offset would be
  //! zero if both region size and value size are equal.
  uint8_t _valueOffset;
  //! Size of the offset immediate value in bits.
  uint8_t _immBitCount;
  //! Shift of the offset immediate value in bits in the target word.
  uint8_t _immBitShift;
  //! Number of least significant bits to discard before writing the immediate to the destination. All discarded
  //! bits must be zero otherwise the value is invalid.
  uint8_t _immDiscardLsb;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the type of the offset.
  ASMJIT_INLINE_NODEBUG OffsetType type() const noexcept { return _type; }

  //! Returns whether the offset is encoded as an absolute value of the offset with additional field(s) that represent
  //! the sign (AArch32 U/N fields in the opcode).
  //!
  //! If true, the offset itself is always positive and a separate U/N field is used to indicate the sign of the offset
  //! (usually `U==1` means ADD, but sometimes `N==1` means negative offset, which implies SUB).
  ASMJIT_INLINE_NODEBUG bool hasSignBit() const noexcept {
    return _type == OffsetType::kThumb32_ADR ||
           _type == OffsetType::kAArch32_ADR ||
           _type == OffsetType::kAArch32_U23_SignedOffset ||
           _type == OffsetType::kAArch32_U23_0To3At0_4To7At8;
  }

  //! Returns flags.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t flags() const noexcept { return _flags; }

  //! Returns the size of the region/instruction where the offset is encoded.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t regionSize() const noexcept { return _regionSize; }

  //! Returns the offset of the word relative to the start of the region where the offset is.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t valueOffset() const noexcept { return _valueOffset; }

  //! Returns the size of the data-type (word) that contains the offset, in bytes.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t valueSize() const noexcept { return _valueSize; }

  //! Returns the count of bits of the offset value in the data it's stored in.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t immBitCount() const noexcept { return _immBitCount; }

  //! Returns the bit-shift of the offset value in the data it's stored in.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t immBitShift() const noexcept { return _immBitShift; }

  //! Returns the number of least significant bits of the offset value, that must be zero and that are not part of
  //! the encoded data.
  [[nodiscard]]
  ASMJIT_INLINE_NODEBUG uint32_t immDiscardLsb() const noexcept { return _immDiscardLsb; }

  //! Resets this offset format to a simple data value of `dataSize` bytes.
  //!
  //! The region will be the same size as data and immediate bits would correspond to `dataSize * 8`. There will be
  //! no immediate bit shift or discarded bits.
  inline void resetToSimpleValue(OffsetType type, size_t valueSize) noexcept {
    ASMJIT_ASSERT(valueSize <= 8u);

    _type = type;
    _flags = uint8_t(0);
    _regionSize = uint8_t(valueSize);
    _valueSize = uint8_t(valueSize);
    _valueOffset = uint8_t(0);
    _immBitCount = uint8_t(valueSize * 8u);
    _immBitShift = uint8_t(0);
    _immDiscardLsb = uint8_t(0);
  }

  inline void resetToImmValue(OffsetType type, size_t valueSize, uint32_t immBitShift, uint32_t immBitCount, uint32_t immDiscardLsb) noexcept {
    ASMJIT_ASSERT(valueSize <= 8u);
    ASMJIT_ASSERT(immBitShift < valueSize * 8u);
    ASMJIT_ASSERT(immBitCount <= 64u);
    ASMJIT_ASSERT(immDiscardLsb <= 64u);

    _type = type;
    _flags = uint8_t(0);
    _regionSize = uint8_t(valueSize);
    _valueSize = uint8_t(valueSize);
    _valueOffset = uint8_t(0);
    _immBitCount = uint8_t(immBitCount);
    _immBitShift = uint8_t(immBitShift);
    _immDiscardLsb = uint8_t(immDiscardLsb);
  }

  inline void setRegion(size_t regionSize, size_t valueOffset) noexcept {
    _regionSize = uint8_t(regionSize);
    _valueOffset = uint8_t(valueOffset);
  }

  inline void setLeadingAndTrailingSize(size_t leadingSize, size_t trailingSize) noexcept {
    _regionSize = uint8_t(leadingSize + trailingSize + _valueSize);
    _valueOffset = uint8_t(leadingSize);
  }

  //! \}
};

//! Data structure used to mark where a fixup in code or data is necessary.
//!
//! Fixups are generally resolved during machine code generation. For example if a branch instruction is used to
//! jump to a label, which hasn't been bound yet, a fixup is created. However, when such label is bound, the fixup
//! is processed and removed from a list of fixups.
struct Fixup {
  //! Next fixup in a single-linked list.
  Fixup* next;
  //! Section where the fixup comes from.
  uint32_t sectionId;
  //! Label id, relocation id, or \ref Globals::kInvalidId.
  //!
  //! \note Fixup that is used with a LabelEntry always uses relocation id here, however, when a fixup is turned
  //! into unresolved and generally detached from LabelEntry, this field becomes a label identifier as unresolved
  //! fixups won't reference a relocation. This is just a space optimization.
  uint32_t labelOrRelocId;
  //! Label offset relative to the start of the section where the unresolved link comes from.
  size_t offset;
  //! Inlined rel8/rel32.
  intptr_t rel;
  //! Offset format information.
  OffsetFormat format;
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FIXUP_H_INCLUDED
