// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#if !defined(ASMJIT_NO_RISCV64)

#include "../core/codewriter_p.h"
#include "../core/cpuinfo.h"
#include "../core/emitterutils_p.h"
#include "../core/formatter.h"
#include "../core/logger.h"
#include "../core/misc_p.h"
#include "../core/support.h"
#include "../riscv/riscv64assembler.h"

ASMJIT_BEGIN_SUB_NAMESPACE(riscv64)

struct InstDispatchRecord {
  Encoding encoding;
  uint16_t index;
};

// ${riscv64::Assembler::Dispatch:Begin}
// ------------------- Automatically generated, do not edit -------------------
static const InstDispatchRecord instDispatchTable[] = {
  {EncodingType::R, 0},
  {EncodingType::I, 0},
  {EncodingType::R, 1},
  {EncodingType::I, 1},
  {EncodingType::U, 0},
  {EncodingType::B, 0},
  {EncodingType::B, 1},
  {EncodingType::B, 2},
  {EncodingType::B, 3},
  {EncodingType::B, 4},
  {EncodingType::B, 5},
  {EncodingType::I-ebreak, 0},
  {EncodingType::I-ecall, 0},
  {EncodingType::-fence, 0},
  {EncodingType::J, 0},
  {EncodingType::I, 2},
  {EncodingType::I, 3},
  {EncodingType::I, 4},
  {EncodingType::I, 5},
  {EncodingType::I, 6},
  {EncodingType::U, 1},
  {EncodingType::I, 7},
  {EncodingType::R, 2},
  {EncodingType::I, 8},
  {EncodingType::S, 0},
  {EncodingType::S, 1},
  {EncodingType::R, 3},
  {EncodingType::R, 4},
  {EncodingType::I, 9},
  {EncodingType::I, 10},
  {EncodingType::R, 5},
  {EncodingType::R, 6},
  {EncodingType::R, 7},
  {EncodingType::R, 8},
  {EncodingType::S, 2},
  {EncodingType::R, 9},
  {EncodingType::I, 11}
};
// ----------------------------------------------------------------------------
// ${riscv64::Assembler::Dispatch:End}

// TODO: [RISC-V]

// riscv64::Assembler - Construction & Destruction
// ===========================================

Assembler::Assembler(CodeHolder* code) noexcept : BaseAssembler() {
  // TODO: [RISC-V]
};

Assembler::~Assembler() noexcept {}

// riscv64::Assembler - Emit
// =====================

// TODO: [RISC-V] consider use the public one if any
enum EncodingType {
  R, I, U, B, J, S,
  // special cases
  I_ebreak, I_ecall, _fence
};

struct Encoding {
  uint32_t mask, match;
}

Error Assembler::_emit(InstId instId, const Operand_& o0, const Operand_& o1, const Operand_& o2, const Operand_* opExt) {
  constexpr uint32_t kOpRegR = uint32_t(OperandType::kReg) | (uint32_t(RegType::kGp32) << 3);
  constexpr uint32_t kOpImmI = uint32_t(OperandType::kImm) | (uint32_t(ImmType::kInt ) << 3);
  
  // TODO: [RISC-V]

  Error err;
  CodeWriter writer(this);

  // TODO: [RISC-V]
  
  // TODO: [RISC-V]

  InstDispatchRecord idr;

  // TODO: [RISC-V]

  // ${riscv64::Assembler::Impl:Begin}
  // ------------------- Automatically generated, do not edit -------------------
  idr = instDispatchTable[instId];
  switch (idr.encodingType) {
    case EncodingType::R: {
      // Group of 'add|and|or|sll|slt|sltu|sra|srl|sub|xor'.
      static const Encoding encodingTable[] = {
        0x00000033u, // Instruction 'add'.
        0x00007033u, // Instruction 'and'.
        0x00006033u, // Instruction 'or'.
        0x00001033u, // Instruction 'sll'.
        0x00002033u, // Instruction 'slt'.
        0x00003033u, // Instruction 'sltu'.
        0x40005033u, // Instruction 'sra'.
        0x00005033u, // Instruction 'srl'.
        0x40000033u, // Instruction 'sub'.
        0x00004033u  // Instruction 'xor'.
      };
      const uint32_t mask = 0x0000707Fu;

      const Encoding &encoding = encodingTable[idr.index];

      if (sgn.test<kOpRegR, kOpRegR, kOpRegR>()) {
        
        
      }

      break;
    }

    case EncodingType::I: {
      // Group of 'addi|andi|jalr|lb|lbu|lh|lhu|lw|ori|slti|sltiu|xori'.
      static const Encoding encodingTable[] = {
        0x00000013u, // Instruction 'addi'.
        0x00007013u, // Instruction 'andi'.
        0x00000067u, // Instruction 'jalr'.
        0x00000003u, // Instruction 'lb'.
        0x00004003u, // Instruction 'lbu'.
        0x00001003u, // Instruction 'lh'.
        0x00005003u, // Instruction 'lhu'.
        0x00002003u, // Instruction 'lw'.
        0x00006013u, // Instruction 'ori'.
        0x00002013u, // Instruction 'slti'.
        0x00003013u, // Instruction 'sltiu'.
        0x00004013u  // Instruction 'xori'.
      };
      const uint32_t mask = 0x0000707Fu;

      const Encoding &encoding = encodingTable[idr.index];

      if (sgn.test<kOpRegR, kOpRegR, KOpImmI>()) {
        
        
      }

      break;
    }

    case EncodingType::U: {
      // Group of 'auipc|lui'.
      static const Encoding encodingTable[] = {
        0x00000017u, // Instruction 'auipc'.
        0x00000037u  // Instruction 'lui'.
      };
      const uint32_t mask = 0x0000707Fu;

      const Encoding &encoding = encodingTable[idr.index];

      if (sgn.test<kOpRegR, KOpImmI>()) {
        
        
      }

      break;
    }

    case EncodingType::B: {
      // Group of 'beq|bge|bgeu|blt|bltu|bne'.
      static const Encoding encodingTable[] = {
        0x00000063u, // Instruction 'beq'.
        0x00005063u, // Instruction 'bge'.
        0x00007063u, // Instruction 'bgeu'.
        0x00004063u, // Instruction 'blt'.
        0x00006063u, // Instruction 'bltu'.
        0x00001063u  // Instruction 'bne'.
      };
      const uint32_t mask = 0x0000707Fu;

      const Encoding &encoding = encodingTable[idr.index];

      if (sgn.test<KOpImmI, kOpRegR, kOpRegR, KOpImmI>()) {
        
        
      }

      break;
    }

    case EncodingType::I-ebreak: {
      // Group of 'ebreak'.
      if (sgn.empty()) {
        
        
      }

      break;
    }

    case EncodingType::I-ecall: {
      // Group of 'ecall'.
      if (sgn.empty()) {
        
        
      }

      break;
    }

    case EncodingType::-fence: {
      // Group of 'fence'.
      if (sgn.test<KOpImmI, KOpImmI, KOpImmI, kOpRegR, kOpRegR>()) {
        
        
      }

      break;
    }

    case EncodingType::J: {
      // Group of 'jal'.
      if (sgn.test<kOpRegR, KOpImmI>()) {
        
        
      }

      break;
    }

    case EncodingType::S: {
      // Group of 'sb|sh|sw'.
      static const Encoding encodingTable[] = {
        0x00000023u, // Instruction 'sb'.
        0x00001023u, // Instruction 'sh'.
        0x00002023u  // Instruction 'sw'.
      };
      const uint32_t mask = 0x0000707Fu;

      const Encoding &encoding = encodingTable[idr.index];

      if (sgn.test<KOpImmI, kOpRegR, kOpRegR, KOpImmI>()) {
        
        
      }

      break;
    }

    default: {
      break;
    }
  }

  goto InvalidInstruction;

  : {
    goto Emit_Op32;
  }
  // ----------------------------------------------------------------------------
  // ${riscv64::Assembler::Impl:End}

  // TODO: [RISC-V]
}

// riscv64::Assembler - Align
// ======================

Error Assembler::align(AlignMode alignMode, uint32_t alignment) {
  // TODO: [RISC-V]
}

// riscv64::Assembler - Events
// =======================

Error Assembler::onAttach(CodeHolder* code) noexcept {
  return Base::onAttach(code);
}

Error Assembler::onDetach(CodeHolder* code) noexcept {
  return Base::onDetach(code);
}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_RISCV64
