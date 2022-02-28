// [armdata.js]
// ARM instruction-set data.
//
// [License]
// Public Domain.


// This file can be parsed as pure JSON, locate ${JSON:BEGIN} and ${JSON:END}
// marks and strip everything outside, a sample JS function that would do the job:
//
// function strip(s) {
//   return s.replace(/(^.*\$\{JSON:BEGIN\}\s+)|(\/\/\s*\$\{JSON:END\}\s*.*$)/g, "");
// }


// INSTRUCTION OPERANDS
// --------------------
//
// Instruction operands contain standard operand field(s) as defined by ARM
// instruction reference, and also additional metadata that is defined by
// ARM, but in notes section (instead of instruction format section). Additional
// data include:
//
//   - "R?!=HI" - The register cannot be R8..R15 (most T16 instructions).
//   - "R?!=PC" - The register cannot be R15 (PC).
//   - "R?!=SP" - The register cannot be R13 (SP).
//   - "R?!=XX" - The register cannot be R13 (SP) or R15 (PC).
//   - "??<=07" - The register must be from 0..7  (some ASIMD instructions).
//   - "??<=15" - The register must be from 0..15 (some ASIMD instructions).
//
// Also, all instructions that use T16 layout were normalized into 3 operand
// form to make these compatible with T32 and A32 architecturess. It was designed
// for convenience. These are easily recognizable as they always share the first
// two operands.
//
// Divergence from ARM Manual:
//   - "Rdn" register (used by T16) was renamed to Rx to make the table easier to
//     read when multiple instructions follow (register names have the same length).


// METADATA
// --------
//
// The following metadata is used to describe instructions:
//
//   "ARMv??"
//     - Required ARM version:
//       - '+' sign means it's supported by that version and above.
//       - '-' sign means it's deprecated (and discontinued) by that version.
//
//   "APSR"
//     - The instruction reads/writes APSR register:
//       - [N|Z|C|V] - Which flags are read/written
//       - Since most of ARM instructions provide conditional execution the
//         APSR mostly defines APSR writes, as reads are controlled by IT
//         or condition code {cond}, which is part of each instruction.
//
//   "APSR_IF_NOT_PC"
//     - Instruction writes to APSR register only if the destination register
//       is not R15 (PC). In that case APSR is not modified (ARM specific).
//

// TODO: MISSING/REVIEW:
//   ldc / ldc2
//   ADD 'mov' with shift spec.
//   MRS/MSR <banked_reg>
//   SETPAN: Set Privileged Access Never.
//   SMC
//   STC / SRC2
//   STC
//   STM
//   SUBS PC, LR
//
//   VLDM, VLDMDB, VLDMIA
//   vmrs
//   vmsr
//   vpop
//   vpush
//   vstm

(function($export, $as) {
"use strict";

$export[$as] =
// ${JSON:BEGIN}
{
  "architectures": [
    "T16",
    "T32",
    "A32"
  ],

  "cpuLevels": [
    {"name": "ARMv4"   },
    {"name": "ARMv4T"  },
    {"name": "ARMv5T"  },
    {"name": "ARMv5TE" },
    {"name": "ARMv5TEJ"},
    {"name": "ARMv6"   },
    {"name": "ARMv6K"  },
    {"name": "ARMv6T2" },
    {"name": "ARMv7"   },
    {"name": "ARMv8"   },
    {"name": "ARMv8_1" },
    {"name": "ARMv8_2" },
    {"name": "ARMv8_3" },
    {"name": "ARMv8_4" },
    {"name": "ARMv8_5" },
    {"name": "ARMv8_6" }
  ],

  "extensions": [
    {"name": "VFPv2"            , "from": "ARMv8"   },
    {"name": "VFPv3"            , "from": "ARMv8"   },
    {"name": "VFPv4"            , "from": "ARMv8"   },
    {"name": "AA32BF16"         , "from": ""        },
    {"name": "AA32I8MM"         , "from": ""        },
    {"name": "AES"              , "from": ""        },
    {"name": "ASIMD"            , "from": ""        },
    {"name": "CRC32"            , "from": "ARMv8_1+"},
    {"name": "FCMA"             , "from": ""        },
    {"name": "FP16CONV"         , "from": "ARMv8"   },
    {"name": "FP16FML"          , "from": "ARMv8_4+"},
    {"name": "FP16FULL"         , "from": ""        },
    {"name": "IDIVA"            , "from": "ARMv8+"  },
    {"name": "IDIVT"            , "from": "ARMv7+"  },
    {"name": "JSCVT"            , "from": "ARMv8_3+"},
    {"name": "SECURITY"         , "from": ""        },
    {"name": "SHA1"             , "from": ""        },
    {"name": "SHA256"           , "from": ""        }
  ],

  "attributes": [
    {"name": "T16_LDM"          , "type": "flag"        , "doc": "Writeback is enabled if Rn is specified also in RdList."},
    {"name": "T32_LDM"          , "type": "flag"        , "doc": "RdList can contain one of R15|R14 and requires at least 2 registers."},
    {"name": "LSL_3_IF_SP"      , "type": "flag"        , "doc": "Restricts the shift operation to lsl by a maximum amount of 3 bit if the destination register is SP."},
    {"name": "ARMv6T2_IF_LOW"   , "type": "flag"        , "doc": "ARMv6T2+ required if both registers are low (R0..R7)."},
    {"name": "UNPRED_COMPLEX"   , "type": "flag"        , "doc": "Unpredictable based on complex rules."},
    {"name": "UNPRED_IF_ALL_LOW", "type": "flag"        , "doc": "Unpredictable if both registers are low (R0..R7)."},
  ],

  "specialRegs": [
    {"name": "APSR.N"           , "group": "APSR.N"     , "doc": "Negative flag."},
    {"name": "APSR.Z"           , "group": "APSR.Z"     , "doc": "Zero flag."},
    {"name": "APSR.C"           , "group": "APSR.C"     , "doc": "Carry or unsigned overflow flag."},
    {"name": "APSR.V"           , "group": "APSR.V"     , "doc": "Signed overflow flag."},
    {"name": "APSR.Q"           , "group": "APSR.Q"     , "doc": "Sticky saturation flag."},
    {"name": "APSR.GE"          , "group": "APSR.GE"    , "doc": "Greater than or equal flag."},

    {"name": "CPSR.IT"          , "group": "CPSR.IT"    , "doc": "If-then bits."},
    {"name": "CPSR.J"           , "group": "CPSR.J"     , "doc": "Jazelle bit."},
    {"name": "CPSR.E"           , "group": "CPSR.E"     , "doc": "Endianness bit."},
    {"name": "CPSR.A"           , "group": "CPSR.A"     , "doc": "Imprecise abort disable bit."},
    {"name": "CPSR.I"           , "group": "CPSR.I"     , "doc": "IRQ disable bit."},
    {"name": "CPSR.F"           , "group": "CPSR.F"     , "doc": "FIQ disable bit."},
    {"name": "CPSR.T"           , "group": "CPSR.T"     , "doc": "Thumb mode bit."},
    {"name": "CPSR.M"           , "group": "CPSR.M"     , "doc": "Current processor mode."},

    {"name": "IPSR.N"           , "group": "IPSR.N"     , "doc": "ISR number."},

    {"name": "FPCSR.N"          , "group": "FPCSR.N"    , "doc": "Less than flag."},
    {"name": "FPCSR.Z"          , "group": "FPCSR.Z"    , "doc": "Equal flag."},
    {"name": "FPCSR.C"          , "group": "FPCSR.C"    , "doc": "Equal, greater than, or unordered flag."},
    {"name": "FPCSR.V"          , "group": "FPCSR.V"    , "doc": "Unordered flag."},
    {"name": "FPCSR.Q"          , "group": "FPCSR.Q"    , "doc": "Sticky saturation flag."},
    {"name": "FPCSR.AHP"        , "group": "FPCSR.MODE" , "doc": "Alternative half-precision control bit."},
    {"name": "FPCSR.DN"         , "group": "FPCSR.MODE" , "doc": "Default NaN mode enable bit."},
    {"name": "FPCSR.FZ"         , "group": "FPCSR.MODE" , "doc": "Flush-to-zero mode enable bit."},
    {"name": "FPCSR.RMode"      , "group": "FPCSR.MODE" , "doc": "Rounding mode control field."},
    {"name": "FPCSR.Stride"     , "group": "FPCSR.VEC"  , "doc": "Vector stride."},
    {"name": "FPCSR.Length"     , "group": "FPCSR.VEC"  , "doc": "Vector length."},
    {"name": "FPCSR.IDE"        , "group": "FPCSR.EXC"  , "doc": "Input subnormal exception enable bit."},
    {"name": "FPCSR.IXE"        , "group": "FPCSR.EXC"  , "doc": "Inexact exception enable bit."},
    {"name": "FPCSR.UFE"        , "group": "FPCSR.EXC"  , "doc": "Underflow exception enable bit."},
    {"name": "FPCSR.OFE"        , "group": "FPCSR.EXC"  , "doc": "Overflow exception enable bit."},
    {"name": "FPCSR.DZE"        , "group": "FPCSR.EXC"  , "doc": "Division by zero exception enable bit."},
    {"name": "FPCSR.IOE"        , "group": "FPCSR.EXC"  , "doc": "Invalid operation exception enable bit."},
    {"name": "FPCSR.IDC"        , "group": "FPCSR.CUM"  , "doc": "Input subnormal cumulative flag."},
    {"name": "FPCSR.IXC"        , "group": "FPCSR.CUM"  , "doc": "Inexact cumulative flag."},
    {"name": "FPCSR.UFC"        , "group": "FPCSR.CUM"  , "doc": "Underflow cumulative flag."},
    {"name": "FPCSR.OFC"        , "group": "FPCSR.CUM"  , "doc": "Overflow cumulative flag."},
    {"name": "FPCSR.DZC"        , "group": "FPCSR.CUM"  , "doc": "Division by zero cumulative flag."},
    {"name": "FPCSR.IOC"        , "group": "FPCSR.CUM"  , "doc": "Invalid operation cumulative flag."}
  ],

  "shortcuts": [
    {"name": "APSR.NZ"          , "expand": "APSR.N|Z"    },
    {"name": "APSR.NZC"         , "expand": "APSR.N|Z|C"  },
    {"name": "APSR.NZCV"        , "expand": "APSR.N|Z|C|V"}
  ],

  "registers": {
    "r": {"kind": "gp" , "any": "r", "names": ["r0-15"]},
    "s": {"kind": "vec", "any": "s", "names": ["s0-31"]},
    "d": {"kind": "vec", "any": "d", "names": ["d0-31"]},
    "v": {"kind": "vec", "any": "v", "names": ["v0-15"]}
  },

  "instructions": [
    {"inst": "adc Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|101|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "adc Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1010|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "adc Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|1010|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "adc Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0101|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "adc Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0101|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "adc Rd, Rn, #immA"                                       , "a32": "cond|001|0101|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "adcS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|101|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W", "mod": "APSR_IF_NOT_PC"},
    {"inst": "adcS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1010|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W", "mod": "APSR_IF_NOT_PC"},
    {"inst": "adcS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|1010|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W", "mod": "APSR_IF_NOT_PC"},
    {"inst": "adcS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0101|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "adcS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0101|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "adcS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0101|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "add Rd!=HI, Rn!=HI, Rm!=HI"                              , "t16": "0001|100|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "in"},
    {"inst": "add Rx!=XX, Rx!=XX, Rm!=XX"                              , "t16": "0100|010|0|Rx:1|Rm:4|Rx:3"                              , "ext": "ARMv4T+ ARMv6T2_IF_LOW", "it": "in"},
    {"inst": "add Rx, Rx, Rm==SP"                                      , "t16": "0100|010|0|Rx:1|Rm:4|Rx:3"                              , "ext": "ARMv4T+", "it": "any"},
    {"inst": "add Rx==SP, Rx==SP, Rm"                                  , "t16": "0100|010|0|Rx:1|Rm:4|Rx:3"                              , "ext": "ARMv4T+", "it": "any"},
    {"inst": "add Rx!=HI, Rx!=HI, #immZ"                               , "t16": "0011|0|Rx:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "in"},
    {"inst": "add Rd!=HI, Rn!=HI, #immZ"                               , "t16": "0001|110|immZ:3|Rn:3|Rd:3"                              , "ext": "ARMv4T+", "it": "in"},
    {"inst": "add Rx==SP, Rx==SP, #immZ*4"                             , "t16": "1011|00000|immZ:7"                                      , "ext": "ARMv4T+", "it": "any"},
    {"inst": "add Rd!=SP, Rn==SP, #immZ*4"                             , "t16": "1010|1|Rd:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "any"},
    {"inst": "add Rd!=XX, Rn!=PC, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1000|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "add Rd!=PC, Rn==SP, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1000|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "mod": "LSL_3_IF_SP"},
    {"inst": "add Rd!=XX, Rn!=PC, #immZ"                               , "t32": "1111|0|immZ:1|1|0000|0|Rn|0|immZ:3|Rd|immZ:8"           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "add Rd!=XX, Rn!=PC, #immA"                               , "t32": "1111|0|imm:1|0|1000|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "add Rd!=PC, Rn==SP, #immZ"                               , "t32": "1111|0|immZ:1|1|0000|0|Rn|0|immZ:3|Rd|immZ:8"           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "add Rd!=PC, Rn==SP, #immA"                               , "t32": "1111|0|imm:1|0|1000|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "add Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0100|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "add Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0100|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "add Rd, Rn!=PC, #immA"                                   , "a32": "cond|001|0100|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "addS Rx!=HI, Rx!=HI, #immZ"                              , "t16": "0011|0|Rx:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=HI, Rn!=HI, #immZ"                              , "t16": "0001|110|immZ:3|Rn:3|Rd:3"                              , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=HI, Rn!=HI, Rm!=HI"                             , "t16": "0001|100|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=XX, Rn!=PC, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1000|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "addS Rd!=PC, Rn==SP, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1000|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "mod": "LSL_3_IF_SP"},
    {"inst": "addS Rd!=XX, Rn!=PC, #immA"                              , "t32": "1111|0|imm:1|0|1000|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=PC, Rn==SP, #immA"                              , "t32": "1111|0|imm:1|0|1000|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0100|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0100|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "addS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0100|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "adr Rd!=HI, #relZ*4"                                     , "t16": "1010|0|Rd:3|relZ:8"                                     , "ext": "ARMv4T+", "it": "any", "mode": "add"},
    {"inst": "adr Rd!=XX, #relZ"                                       , "t32": "1111|0|relZ:1|10000|0|1111|0|relZ:3|Rd|relZ:8"          , "ext": "ARMv6T2+", "it": "any", "mode": "add"},
    {"inst": "adr Rd!=XX, #relZ"                                       , "t32": "1111|0|relZ:1|10101|0|1111|0|relZ:3|Rd|relZ:8"          , "ext": "ARMv6T2+", "it": "any", "mode": "sub"},
    {"inst": "adr Rd, #RelA"                                           , "a32": "cond|001|0100|0|1111|Rd|RelA:12"                        , "ext": "ARMv4+", "mode": "add"},
    {"inst": "adr Rd, #RelA"                                           , "a32": "cond|001|0010|0|1111|Rd|RelA:12"                        , "ext": "ARMv4+", "mode": "sub"},

    {"inst": "aesd.x8 Vx, Vm"                                          , "t32": "1111|11111|Vx'|11|00|00|Vx|0|0110|1|Vm'|0|Vm"           , "ext": "AES"},
    {"inst": "aesd.x8 Vx, Vm"                                          , "a32": "1111|00111|Vx'|11|00|00|Vx|0|0110|1|Vm'|0|Vm"           , "ext": "AES"},

    {"inst": "aese.x8 Vx, Vm"                                          , "t32": "1111|11111|Vx'|11|00|00|Vx|0|0110|0|Vm'|0|Vm"           , "ext": "AES"},
    {"inst": "aese.x8 Vx, Vm"                                          , "a32": "1111|00111|Vx'|11|00|00|Vx|0|0110|0|Vm'|0|Vm"           , "ext": "AES"},

    {"inst": "aesimc.x8 Vd, Vm"                                        , "t32": "1111|11111|Vd'|11|00|00|Vd|0|0111|1|Vm'|0|Vm"           , "ext": "AES"},
    {"inst": "aesimc.x8 Vd, Vm"                                        , "a32": "1111|00111|Vd'|11|00|00|Vd|0|0111|1|Vm'|0|Vm"           , "ext": "AES"},

    {"inst": "aesmc.x8 Vd, Vm"                                         , "t32": "1111|11111|Vd'|11|00|00|Vd|0|0111|0|Vm'|0|Vm"           , "ext": "AES"},
    {"inst": "aesmc.x8 Vd, Vm"                                         , "a32": "1111|00111|Vd'|11|00|00|Vd|0|0111|0|Vm'|0|Vm"           , "ext": "AES"},

    {"inst": "and Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|000|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "and Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|0000|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "and Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|1010|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "and Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0000|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "and Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0000|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "and Rd, Rn, #immA"                                       , "a32": "cond|001|0000|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "andS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|000|Rm:3|Rx:3"                                 , "ext": "ARMv4T+ IT=OUT", "apsr": "NZ=W"},
    {"inst": "andS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|0000|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "andS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|1010|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W", "mod": "APSR_IF_NOT_PC"},
    {"inst": "andS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0000|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "andS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0000|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "andS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0000|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "asr Rd!=HI, Rn!=HI, #n"                                  , "t16": "0001|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "in"},
    {"inst": "asr Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|100|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "asr Rd!=XX, Rn!=XX, #n"                                  , "t32": "1110|101|0010|0|1111|0|n:3|Rd|n:2|10|Rn"                , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "asr Rd!=XX, Rn!=XX, Rm!=XX"                              , "t32": "1111|101|0010|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "asr Rd!=PC, Rn!=PC, Rm!=PC"                              , "a32": "cond|000|1101|0|0000|Rd|Rm|0101|Rn"                     , "ext": "ARMv4+"},
    {"inst": "asr Rd, Rn, #n"                                          , "a32": "cond|000|1101|0|0000|Rd|n:5|100|Rn"                     , "ext": "ARMv4+"},

    {"inst": "asrS Rd!=HI, Rn!=HI, #n"                                 , "t16": "0001|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "asrS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|100|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "asrS Rd!=XX, Rn!=XX, #n"                                 , "t32": "1110|101|0010|1|1111|0|n:3|Rd|n:2|10|Rn"                , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "asrS Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0010|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "asrS Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|1101|1|0000|Rd|Rm|0101|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "asrS Rd, Rn, #n"                                         , "a32": "cond|000|1101|1|0000|Rd|n:5|100|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "b #relS*2"                                               , "t16": "1101|cond|relS:8"                                       , "ext": "ARMv4T+", "it": "out"},
    {"inst": "b #relS*2"                                               , "t16": "1110|0|relS:11"                                         , "ext": "ARMv4T+", "it": "out|last"},
    {"inst": "b #relS*2"                                               , "t32": "1111|0|relS[19]|cond|relS[16:11]|10|ja|0|jb|relS[10:0]" , "ext": "ARMv6T2+", "calc": {"ja": "relS[19] ^ relS[18] ^ 1", "jb": "relS[19] ^ relS[17] ^ 1"}, "it": "out"},
    {"inst": "b #relS*2"                                               , "t32": "1111|0|relS[23]|     relS[20:11]|10|ja|1|jb|relS[10:0]" , "ext": "ARMv6T2+", "calc": {"ja": "relS[23] ^ relS[22] ^ 1", "jb": "relS[23] ^ relS[21] ^ 1"}, "it": "out|last"},
    {"inst": "b #relS*4"                                               , "a32": "cond|101|0|relS:24"                                     , "ext": "ARMv4+"},

    {"inst": "bfc Rd!=XX, #lsb, #width"                                , "t32": "1111|001|1011|0|1111|0|lsb:3|Rd|lsb:2|0|msb:5"          , "ext": "ARMv6T2+", "imm": "BfcBfiImm(lsb, width)", "it": "any"},
    {"inst": "bfc Rd!=PC, #lsb, #width"                                , "a32": "cond|011|1110|msb:5|Rd|lsb:5|001|1111"                  , "ext": "ARMv6T2+", "imm": "BfcBfiImm(lsb, width)", },

    {"inst": "bfi Rd!=XX, Rn!=XX, #lsb, #width"                        , "t32": "1111|001|1011|0|Rn|0|lsb:3|Rd|lsb:2|0|msb:5"            , "ext": "ARMv6T2+", "imm": "BfcBfiImm(lsb, width)", "it": "any"},
    {"inst": "bfi Rd!=PC, Rn!=PC, #lsb, #width"                        , "a32": "cond|011|1110|msb:5|Rd|lsb:5|001|Rn"                    , "ext": "ARMv6T2+", "imm": "BfcBfiImm(lsb, width)", },

    {"inst": "bic Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|001|110|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "bic Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|0001|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "bic Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|0001|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "bic Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|1110|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "bic Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|1110|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "bic Rd, Rn, #immA"                                       , "a32": "cond|001|1110|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "bicS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|001|110|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "bicS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|0001|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "bicS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|0001|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "bicS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|1110|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "bicS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|1110|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "bicS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|1110|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "bkpt #immZ"                                              , "t16": "1011|111|0|immZ:8"                                      , "ext": "ARMv5T+", "it": "uncond"},
    {"inst": "bkpt #immZ"                                              , "a32": "cond|000|1001|0|immZ:12|0111|immZ:4"                    , "ext": "ARMv6+ ARMv5T"},

    {"inst": "bl #relS*2"                                              , "t32": "1111|0|relS[23]|relS[20:11]|11|ja|1|jb|relS[10:0]"      , "ext": "ARMv4T+", "it": "out|last"},
    {"inst": "bl #relS*4"                                              , "a32": "cond|101|1|relS:24"                                     , "ext": "ARMv4+"},

    {"inst": "blx Rm!=PC"                                              , "t16": "0100|011|11|Rm:4|000"                                   , "ext": "ARMv5T+", "it": "out|last"},
    {"inst": "blx #relS*4"                                             , "t32": "1111|0|relS[22]|relS[19:10]|11|ja|0|jb|relS[9:0]|0"     , "ext": "ARMv4T+", "it": "out|last"},
    {"inst": "blx Rm!=PC"                                              , "a32": "cond|000|1001|0|1111|1111|1111|0011|Rm"                 , "ext": "ARMv5T+"},
    {"inst": "blx #relS*2"                                             , "a32": "1111|101|relS[0]|relS[24:1]"                            , "ext": "ARMv4+"},

    {"inst": "bx Rm"                                                   , "t16": "0100|011|10|Rm:4|000"                                   , "ext": "ARMv4T+", "it": "out|last"},
    {"inst": "bx Rm"                                                   , "a32": "cond|000|1001|0|1111|1111|1111|0001|Rm"                 , "ext": "ARMv4T+"},

    {"inst": "bxj Rm!=XX"                                              , "t32": "1111|001|1110|0|Rm|1000|1111|00000000"                  , "ext": "ARMv6T2+", "it": "out|last"},
    {"inst": "bxj Rm!=PC"                                              , "a32": "cond|000|1001|0|1111|1111|1111|0010|Rm"                 , "ext": "ARMv5TEJ+"},

    {"inst": "cbz Rn!=HI, #relZ*2"                                     , "t16": "1011|00|relZ:1|1|relZ:5|Rn:3"                           , "ext": "ARMv6T2+", "it": "out"},
    {"inst": "cbnz Rn!=HI, #relZ*2"                                    , "t16": "1011|10|relZ:1|1|relZ:5|Rn:3"                           , "ext": "ARMv6T2+", "it": "out"},

    {"inst": "clrex"                                                   , "t32": "1111|001|1101|1|1111|1000|1111|0010|1111"               , "ext": "ARMv7+", "it": "any"},
    {"inst": "clrex"                                                   , "a32": "1111|010|1011|1|1111|1111|0000|0001|1111"               , "ext": "ARMv7+ ARMv6K"},

    {"inst": "clz Rd!=XX, Rn!=XX"                                      , "t32": "1111|101|0101|1|Rm|1111|Rd|1000|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "clz Rd!=PC, Rn!=PC"                                      , "a32": "cond|000|1011|0|1111|Rd|1111|0001|Rn"                   , "ext": "ARMv6+ ARMv5T"},

    {"inst": "cmn Rn!=HI, Rm!=HI"                                      , "t16": "0100|001|011|Rm:3|Rn:3"                                 , "ext": "ARMv4T+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmn Rn!=PC, Rm!=XX, {sop #n}"                            , "t32": "1110|101|1000|1|Rn|0|n:3|1111|n:2|sop:2|Rm"             , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmn Rn!=PC, #immA"                                       , "t32": "1111|0|imm:1|0|1000|1|Rn|0|imm:3|1111|imm:8"            , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmn Rn!=PC, Rm!=PC, sop Rs!=PC"                          , "a32": "cond|000|1011|1|Rn|0000|Rs|0|sop:2|1|Rm"                , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "cmn Rn, Rm, {sop #n}"                                    , "a32": "cond|000|1011|1|Rn|0000|n:5|sop:2|0|Rm"                 , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "cmn Rn, #immA"                                           , "a32": "cond|001|1011|1|Rn|0000|imm:12"                         , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "cmp Rn!=HI, #immZ"                                       , "t16": "0010|1|Rn:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmp Rn!=HI, Rm!=HI"                                      , "t16": "0100|001|010|Rm:3|Rn:3"                                 , "ext": "ARMv4T+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmp Rn!=PC, Rm!=PC"                                      , "t16": "0100|010|1|Rn:1|Rm:4|Rn:3"                              , "ext": "ARMv4T+", "it": "any", "apsr": "NZCV=W", "mod": "UNPRED_IF_ALL_LOW"},
    {"inst": "cmp Rn!=PC, Rm!=XX, {sop #n}"                            , "t32": "1110|101|1101|1|Rn|0|n:3|1111|n:2|sop:2|Rm"             , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmp Rn!=PC, #immA"                                       , "t32": "1111|0|imm:1|0|1101|1|Rn|0|imm:3|1111|imm:8"            , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "cmp Rn!=PC, Rm!=PC, sop Rs!=PC"                          , "a32": "cond|000|1010|1|Rn|0000|Rs|0|sop:2|1|Rm"                , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "cmp Rn, Rm, {sop #n}"                                    , "a32": "cond|000|1010|1|Rn|0000|n:5|sop:2|0|Rm"                 , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "cmp Rn, #immA"                                           , "a32": "cond|001|1010|1|Rn|0000|imm:12"                         , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "cps #immZ"                                               , "a32": "1111|000|1000|0|0010|0000|000|000|0|immZ:5"             , "?": "?"},

    {"inst": "cpsid #aif"                                              , "a32": "1111|000|1000|0|1100|0000|000|aif:3|0|00000"            , "?": "?"},
    {"inst": "cpsid #aif, #immZ"                                       , "a32": "1111|000|1000|0|1110|0000|000|aif:3|0|immZ:5"           , "?": "?"},

    {"inst": "cpsie #aif"                                              , "a32": "1111|000|1000|0|1000|0000|000|aif:3|0|00000"            , "?": "?"},
    {"inst": "cpsie #aif, #immZ"                                       , "a32": "1111|000|1000|0|1010|0000|000|aif:3|0|immZ:5"           , "?": "?"},

    {"inst": "crc32b Rd!=PC, Rn!=PC, Rm!=PC"                           , "t32": "1111|101|0110|0|Rn|1111|Rd|10|00|Rm"                    , "ext": "CRC32", "it": "out"},
    {"inst": "crc32b Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1000|0|Rn|Rd|0000|0100|Rm"                     , "ext": "CRC32"},

    {"inst": "crc32h Rd!=PC, Rn!=PC, Rm!=PC"                           , "t32": "1111|101|0110|0|Rn|1111|Rd|10|01|Rm"                    , "ext": "CRC32", "it": "out"},
    {"inst": "crc32h Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1001|0|Rn|Rd|0000|0100|Rm"                     , "ext": "CRC32"},

    {"inst": "crc32w Rd!=PC, Rn!=PC, Rm!=PC"                           , "t32": "1111|101|0110|0|Rn|1111|Rd|10|10|Rm"                    , "ext": "CRC32", "it": "out"},
    {"inst": "crc32w Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1010|0|Rn|Rd|0000|0100|Rm"                     , "ext": "CRC32"},

    {"inst": "crc32cb Rd!=PC, Rn!=PC, Rm!=PC"                          , "t32": "1111|101|0110|1|Rn|1111|Rd|10|00|Rm"                    , "ext": "CRC32", "it": "out"},
    {"inst": "crc32cb Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|000|1000|0|Rn|Rd|0010|0100|Rm"                     , "ext": "CRC32"},

    {"inst": "crc32ch Rd!=PC, Rn!=PC, Rm!=PC"                          , "t32": "1111|101|0110|1|Rn|1111|Rd|10|01|Rm"                    , "ext": "CRC32", "it": "out"},
    {"inst": "crc32ch Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|000|1001|0|Rn|Rd|0010|0100|Rm"                     , "ext": "CRC32"},

    {"inst": "crc32cw Rd!=PC, Rn!=PC, Rm!=PC"                          , "t32": "1111|101|0110|1|Rn|1111|Rd|10|10|Rm"                    , "ext": "CRC32", "it": "out"},
    {"inst": "crc32cw Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|000|1010|0|Rn|Rd|0010|0100|Rm"                     , "ext": "CRC32"},

    {"inst": "dbg #immZ"                                               , "t32": "1111|001|1101|0|1111|1000|0000|1111|immZ:4"             , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "dbg #immZ"                                               , "a32": "cond|001|1001|0|0000|1111|0000|1111|immZ:4"             , "ext": "ARMv7+ ARMv8-"},

    {"inst": "dmb #immZ"                                               , "t32": "1111|001|1101|1|1111|1000|1111|0101|immZ:4"             , "ext": "ARMv7+", "it": "any"},
    {"inst": "dmb #immZ"                                               , "a32": "1111|010|1011|1|1111|1111|0000|0101|immZ:4"             , "ext": "ARMv7+"},

    {"inst": "dsb #immZ"                                               , "t32": "1111|001|1101|1|1111|1000|1111|0100|immZ:4"             , "ext": "ARMv7+", "it": "any"},
    {"inst": "dsb #immZ"                                               , "a32": "1111|010|1011|1|1111|1111|0000|0100|immZ:4"             , "ext": "ARMv7+"},

    {"inst": "eor Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|001|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "eor Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|0100|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "eor Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|0100|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "eor Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0001|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "eor Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0001|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "eor Rd, Rn, #immA"                                       , "a32": "cond|001|0001|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "eorS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|001|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "eorS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|0100|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "eorS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|0100|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "eorS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0001|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "eorS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0001|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "eorS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0001|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "eret"                                                    , "a32": "cond|000|1011|0|0000|0000|0000|0110|1110"               , "?": "?"},

    {"inst": "hlt #immZ"                                               , "a32": "cond|000|1000|0|immZ:12|0111|immZ:4"                    , "?": "?"},
    {"inst": "hvc #immZ"                                               , "a32": "cond|000|1010|0|immZ:12|0111|immZ:4"                    , "?": "?"},

    {"inst": "isb #immZ"                                               , "a32": "1111|010|1011|1|1111|1111|0000|0110|immZ:4"             , "ext": "ARMv7+"},

    {"inst": "it #FirstCond!=15"                                       , "t16": "1011|111|1|FirstCond[3:0]|1000"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "ite #FirstCond!=15"                                      , "t16": "1011|111|1|FirstCond[3:0]|X100"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itee #FirstCond!=15"                                     , "t16": "1011|111|1|FirstCond[3:0]|XY10"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "iteee #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "iteet #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itet #FirstCond!=15"                                     , "t16": "1011|111|1|FirstCond[3:0]|XY10"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itete #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itett #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itt #FirstCond!=15"                                      , "t16": "1011|111|1|FirstCond[3:0]|X100"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itte #FirstCond!=15"                                     , "t16": "1011|111|1|FirstCond[3:0]|XY10"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "ittee #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "ittet #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "ittt #FirstCond!=15"                                     , "t16": "1011|111|1|FirstCond[3:0]|XY10"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "ittte #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},
    {"inst": "itttt #FirstCond!=15"                                    , "t16": "1011|111|1|FirstCond[3:0]|XYZ1"                         , "ext": "ARMv6T2+", "it": "out|def"},

    {"inst": "lda Rd!=PC, [Rn!=PC]"                                    , "t32": "1110|100|0110|1|Rn|Rd|1111|1010|1111"                   , "ext": "ARMv8+", "it": "any"},
    {"inst": "lda Rd!=PC, [Rn!=PC]"                                    , "a32": "cond|000|1100|1|Rn|Rd|1100|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldab Rd!=PC, [Rn!=PC]"                                   , "t32": "1110|100|0110|1|Rn|Rd|1111|1000|1111"                   , "ext": "ARMv8+", "it": "any"},
    {"inst": "ldab Rd!=PC, [Rn!=PC]"                                   , "a32": "cond|000|1110|1|Rn|Rd|1100|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldaex Rd!=PC, [Rn!=PC]"                                  , "t32": "1110|100|0110|1|Rn|Rd|1111|1110|1111"                   , "ext": "ARMv8+", "it": "any"},
    {"inst": "ldaex Rd!=PC, [Rn!=PC]"                                  , "a32": "cond|000|1100|1|Rn|Rd|1110|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldaexb Rd!=PC, [Rn!=PC]"                                 , "t32": "1110|100|0110|1|Rn|Rd|1111|1100|1111"                   , "ext": "ARMv8+", "it": "any"},
    {"inst": "ldaexb Rd!=PC, [Rn!=PC]"                                 , "a32": "cond|000|1110|1|Rn|Rd|1110|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldaexd Rd!=PC, Rd2!=PC, [Rn!=PC]"                        , "t32": "1110|100|0110|1|Rn|Rd|Rd2|1111|1111"                    , "ext": "ARMv8+", "it": "any"},
    {"inst": "ldaexd Rd<=13, Rd2+, [Rn!=PC]"                           , "a32": "cond|000|1101|1|Rn|Rd|1110|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldaexh Rd!=PC, [Rn!=PC]"                                 , "t32": "1110|100|0110|1|Rn|Rd|1111|1101|1111"                   , "ext": "ARMv8+", "it": "any"},
    {"inst": "ldaexh Rd!=PC, [Rn!=PC]"                                 , "a32": "cond|000|1111|1|Rn|Rd|1110|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldah Rd!=PC, [Rn!=PC]"                                   , "t32": "1110|100|0110|1|Rn|Rd|1111|1001|1111"                   , "ext": "ARMv8+", "it": "any"},
    {"inst": "ldah Rd!=PC, [Rn!=PC]"                                   , "a32": "cond|000|1111|1|Rn|Rd|1100|1001|1111"                   , "ext": "ARMv8+"},

    {"inst": "ldm [Rn!=HI]{!}, RdList"                                 , "t16": "1100|1|Rn:3|RdList:8"                                   , "ext": "ARMv4T+", "it": "any", "mod": "T16_LDM"},
    {"inst": "ldm [Rn!=PC]{!}, RdList"                                 , "t32": "1110|100|010W|1|Rn|RdList[15:14]|0|RdList[12:0]"        , "ext": "ARMv6T2+", "it": "any", "mod": "T32_LDM"},
    {"inst": "ldm [Rn!=PC]{!}, RdList"                                 , "a32": "cond|100|010W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},
    {"inst": "ldm [Rn!=PC]{!}, RdList"                                 , "a32": "cond|100|011W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},

    {"inst": "ldmda [Rn!=PC]{!}, RdList"                               , "a32": "cond|100|000W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},
    {"inst": "ldmda [Rn!=PC]{!}, RdList"                               , "a32": "cond|100|001W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},

    {"inst": "ldmdb [Rn!=PC]{!}, RdList"                               , "t32": "1110|100|100W|1|Rn|RdList[15:14]|0|RdList[12:0]"        , "ext": "ARMv6T2+", "it": "any", "mod": "T32_LDM"},
    {"inst": "ldmdb [Rn!=PC]{!}, RdList"                               , "a32": "cond|100|100W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},
    {"inst": "ldmdb [Rn!=PC]{!}, RdList"                               , "a32": "cond|100|101W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},

    {"inst": "ldmib [Rn!=PC]{!}, RdList"                               , "a32": "cond|100|110W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},
    {"inst": "ldmib [Rn!=PC]{!}, RdList"                               , "a32": "cond|100|111W|1|Rn|RdList:16"                           , "ext": "ARMv4+"},

    {"inst": "ldr Rd!=HI, [Rn!=HI, Rm!=HI]"                            , "t16": "0101|100|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldr Rd, [Rn!=PC, Rm!=XX, {lsl #n}]"                      , "t32": "1111|100|0010|1|Rn|Rd|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldr Rd!=HI, [Rn!=HI, #off*4]"                            , "t16": "0110|1|off:5|Rn:3|Rd:3"                                 , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldr Rd!=HI, [Rn==SP, #off*4]"                            , "t16": "1001|1|Rd:3|off:8"                                      , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldr Rd!=HI, [Rn==PC, #off*4]"                            , "t16": "0100|1|Rd:3|off:8"                                      , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldr Rd, [Rn!=PC, #off]"                                  , "t32": "1111|100|0110|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldr Rd, [Rn!=PC, #+/-off]{@}{!}"                         , "t32": "1111|100|0010|1|Rn|Rd|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldr Rd, [Rn==PC, #+/-off]"                               , "t32": "1111|100|0U10|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldr Rd, [Rn, +/-Rm!=PC, {sop #n}]{@}{!}"                 , "a32": "cond|011|PU0W|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "ldr Rd, [Rn, #+/-off]{@}{!}"                             , "a32": "cond|010|PU0W|1|Rn|Rd|off:12"                           , "ext": "ARMv4+"},

    {"inst": "ldrb Rd!=HI, [Rn!=HI, Rm!=HI]"                           , "t16": "0101|110|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldrb Rd!=XX, [Rn!=PC, Rm!=XX, {lsl #n}]"                 , "t32": "1111|100|0000|1|Rn|Rd|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrb Rd!=HI, [Rn!=HI, #off*4]"                           , "t16": "0111|1|off:5|Rn:3|Rd:3"                                 , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldrb Rd!=XX, [Rn!=PC, #off]"                             , "t32": "1111|100|0100|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrb Rd!=XX, [Rn!=PC, #+/-off]{!}"                       , "t32": "1111|100|0000|1|Rn|Rd|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrb Rd!=XX, [Rn==PC, #+/-off]"                          , "t32": "1111|100|0U00|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrb Rd!=PC, [Rn, +/-Rm!=PC, {sop #n}]{@}{!}"            , "a32": "cond|011|PU1W|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "ldrb Rd!=PC, [Rn, #+/-off]{@}{!}"                        , "a32": "cond|010|PU1W|1|Rn|Rd|off:12"                           , "ext": "ARMv4+"},

    {"inst": "ldrbt Rd!=XX, [Rn!=PC, #off]"                            , "t32": "1111|100|0000|1|Rn|Rd|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrbt Rd!=PC, [Rn!=PC, +/-Rm!=PC, {sop #n}]@"            , "a32": "cond|011|0U11|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "ldrbt Rd!=PC, [Rn!=PC, #+/-off]@"                        , "a32": "cond|010|0U11|1|Rn|Rd|off:12"                           , "ext": "ARMv4+"},

    {"inst": "ldrd Rd!=XX, Rd2!=XX, [Rn==PC, #+/-off*4]"               , "t32": "1110|100|PU10|1|Rn|Rd|Rd2|off:8"                        , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrd Rd!=XX, Rd2!=XX, [Rn!=PC, #+/-off*4]{@}{!}"         , "t32": "1110|100|PU1W|1|Rn|Rd|Rd2|off:8"                        , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrd Rd<=13, Rd2+, [Rn, +/-Rm]{@}{!}"                    , "a32": "cond|000|PU0W|0|Rn|Rd|0000|1101|Rm"                     , "ext": "ARMv5TE+"},
    {"inst": "ldrd Rd<=13, Rd2+, [Rn, #+/-off]{!}"                     , "a32": "cond|000|PU1W|0|Rn|Rd|off:4|1101|off:4"                 , "ext": "ARMv5TE+"},

    {"inst": "ldrex Rd!=XX, [Rn!=PC, #off*4]"                          , "t32": "1110|100|0010|1|Rn|Rd|1111|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrex Rd!=PC, [Rn!=PC]"                                  , "a32": "cond|000|1100|1|Rn|Rd|1111|1001|1111"                   , "ext": "ARMv6+"},

    {"inst": "ldrexb Rd!=XX, [Rn!=PC]"                                 , "t32": "1110|100|0110|1|Rn|Rd|1111|0100|1111"                   , "ext": "ARMv7+", "it": "any"},
    {"inst": "ldrexb Rd!=PC, [Rn!=PC]"                                 , "a32": "cond|000|1110|1|Rn|Rd|1111|1001|1111"                   , "ext": "ARMv6K+"},

    {"inst": "ldrexd Rd!=XX, Rd2!=XX, [Rn!=PC]"                        , "t32": "1110|100|0110|1|Rn|Rd|Rd2 |0111|1111"                   , "ext": "ARMv7+", "it": "any"},
    {"inst": "ldrexd Rd<=13, Rd2+, [Rn!=PC]"                           , "a32": "cond|000|1101|1|Rn|Rd|1111|1001|1111"                   , "ext": "ARMv6K+"},

    {"inst": "ldrexh Rd!=XX, [Rn!=PC]"                                 , "t32": "1110|100|0110|1|Rn|Rd|1111|0101|1111"                   , "ext": "ARMv7+", "it": "any"},
    {"inst": "ldrexh Rd!=PC, [Rn!=PC]"                                 , "a32": "cond|000|1111|1|Rn|Rd|1111|1001|1111"                   , "ext": "ARMv6K+"},

    {"inst": "ldrh Rd!=HI, [Rn!=HI, Rm!=HI]"                           , "t16": "0101|101|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldrh Rd!=XX, [Rn!=PC, Rm!=XX, {lsl #n}]"                 , "t32": "1111|100|0001|1|Rn|Rd|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrh Rd!=HI, [Rn!=HI, #off*4]"                           , "t16": "1000|1|off:5|Rn:3|Rd:3"                                 , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldrh Rd!=XX, [Rn!=PC, #off]"                             , "t32": "1111|100|0101|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrh Rd!=XX, [Rn!=PC, #+/-off]{@}{!}"                    , "t32": "1111|100|0001|1|Rn|Rd|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrh Rd!=XX, [Rn==PC, #+/-off]"                          , "t32": "1111|100|0U01|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrh Rd!=PC, [Rn, +/-Rm!=PC]{@}{!}"                      , "a32": "cond|000|PU0W|1|Rn|Rd|0000|1011|Rm"                     , "ext": "ARMv4+"},
    {"inst": "ldrh Rd!=PC, [Rn, #+/-off]{@}{!}"                        , "a32": "cond|000|PU1W|1|Rn|Rd|off:4|1011|off:4"                 , "ext": "ARMv4+"},

    {"inst": "ldrht Rd!=XX, [Rn!=PC, #off]"                            , "t32": "1111|100|0001|1|Rn|Rd|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrht Rd!=PC, [Rn!=PC, +/-Rm!=PC]@"                      , "a32": "cond|000|0U01|1|Rn|Rd|0000|1011|Rm"                     , "ext": "ARMv6T2+"},
    {"inst": "ldrht Rd!=PC, [Rn!=PC, #+/-off]@"                        , "a32": "cond|000|0U11|1|Rn|Rd|off:4|1011|off:4"                 , "ext": "ARMv6T2+"},

    {"inst": "ldrsb Rd!=HI, [Rn!=HI, Rm!=HI]"                          , "t16": "0101|011|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldrsb Rd!=XX, [Rn!=PC, Rm!=XX, {lsl #n}]"                , "t32": "1111|100|1000|1|Rn|Rd|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsb Rd!=XX, [Rn!=PC, #off]"                            , "t32": "1111|100|1100|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsb Rd!=XX, [Rn!=PC, #+/-off]{@}{!}"                   , "t32": "1111|100|1000|1|Rn|Rd|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsb Rd!=XX, [Rn==PC, #+/-off]"                         , "t32": "1111|100|1U00|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsb Rd!=PC, [Rn!=PC, +/-Rm!=PC]{@}{!}"                 , "a32": "cond|000|PU0W|1|Rn|Rd|0000|1101|Rm"                     , "ext": "ARMv4+"},
    {"inst": "ldrsb Rd!=PC, [Rn, #+/-off]{@}{!}"                       , "a32": "cond|000|PU1W|1|Rn|Rd|off:4|1101|off:4"                 , "ext": "ARMv4+"},

    {"inst": "ldrsbt Rd!=XX, [Rn!=PC, #off]"                           , "t32": "1111|100|1000|1|Rn|Rd|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsbt Rd!=PC, [Rn!=PC, +/-Rm]@"                         , "a32": "cond|000|0U01|1|Rn|Rd|0000|1101|Rm"                     , "ext": "ARMv6T2+"},
    {"inst": "ldrsbt Rd!=PC, [Rn!=PC, #+/-off]@"                       , "a32": "cond|000|0U11|1|Rn|Rd|off:4|1101|off:4"                 , "ext": "ARMv6T2+"},

    {"inst": "ldrsh Rd!=HI, [Rn!=HI, Rm!=HI]"                          , "t16": "0101|111|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "ldrsh Rd!=XX, [Rn!=PC, Rm!=XX, {lsl #n}]"                , "t32": "1111|100|1001|1|Rn|Rd|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsh Rd!=XX, [Rn!=PC, #off]"                            , "t32": "1111|100|1101|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsh Rd!=XX, [Rn!=PC, #+/-off]{@}{!}"                   , "t32": "1111|100|1001|1|Rn|Rd|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsh Rd!=XX, [Rn==PC, #+/-off]"                         , "t32": "1111|100|1U01|1|Rn|Rd|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsh Rd!=PC, [Rn, +/-Rm!=PC]{@}{!}"                     , "a32": "cond|000|PU0W|1|Rn|Rd|0000|1111|Rm"                     , "ext": "ARMv4+"},
    {"inst": "ldrsh Rd!=PC, [Rn, #+/-off]{@}{!}"                       , "a32": "cond|000|PU1W|1|Rn|Rd|off:4|1111|off:4"                 , "ext": "ARMv4+"},

    {"inst": "ldrsht Rd!=XX, [Rn!=PC, #off]"                           , "t32": "1111|100|1001|1|Rn|Rd|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrsht Rd!=PC, [Rn!=PC, #+/-off]@"                       , "a32": "cond|000|0U11|1|Rn|Rd|off:4|1111|off:4"                 , "ext": "ARMv6T2+"},
    {"inst": "ldrsht Rd!=PC, [Rn!=PC, +/-Rm!=PC]@"                     , "a32": "cond|000|0U01|1|Rn|Rd|0000|1111|Rm"                     , "ext": "ARMv6T2+"},

    {"inst": "ldrt Rd!=XX, [Rn!=PC, #off]"                             , "t32": "1111|100|0010|1|Rn|Rd|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ldrt Rd!=PC, [Rn!=PC, +/-Rm!=PC, {sop #n}]@"             , "a32": "cond|011|0U01|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "ldrt Rd!=PC, [Rn!=PC, #+/-off]@"                         , "a32": "cond|010|0U01|1|Rn|Rd|off:12"                           , "ext": "ARMv4+"},

    {"inst": "lsl Rd!=HI, Rn!=HI, #n"                                  , "t16": "0000|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "in"},
    {"inst": "lsl Rd!=XX, Rn!=XX, #n"                                  , "t32": "1110|101|0010|0|1111|0|n:3|Rd|n:2|00|Rn"                , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "lsl Rd, Rn, #n"                                          , "a32": "cond|000|1101|0|0000|Rd|n:5|000|Rn"                     , "ext": "ARMv4+"},
    {"inst": "lsl Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|010|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "lsl Rd!=XX, Rn!=XX, Rm!=XX"                              , "t32": "1111|101|0000|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "lsl Rd!=PC, Rn!=PC, Rm!=PC"                              , "a32": "cond|000|1101|0|0000|Rd|Rm|0001|Rn"                     , "ext": "ARMv4+"},

    {"inst": "lslS Rd!=HI, Rn!=HI, #n"                                 , "t16": "0000|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "lslS Rd!=XX, Rn!=XX, #n"                                 , "t32": "1110|101|0010|1|1111|0|n:3|Rd|n:2|00|Rn"                , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "lslS Rd, Rn, #n"                                         , "a32": "cond|000|1101|1|0000|Rd|n:5|000|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "lslS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|010|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "lslS Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1110|101|0000|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "lslS Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|1101|1|0000|Rd|Rm|0001|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "lsr Rd!=HI, Rn!=HI, #n"                                  , "t16": "0000|1|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "in"},
    {"inst": "lsr Rd!=XX, Rn!=XX, #n"                                  , "t32": "1110|101|0010|0|1111|0|n:3|Rd|n:2|01|Rn"                , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "lsr Rd, Rn, #n"                                          , "a32": "cond|000|1101|0|0000|Rd|n:5|010|Rn"                     , "ext": "ARMv4+"},
    {"inst": "lsr Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|011|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "lsr Rd!=XX, Rn!=XX, Rm!=XX"                              , "t32": "1111|101|0001|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "lsr Rd!=PC, Rn!=PC, Rm!=PC"                              , "a32": "cond|000|1101|0|0000|Rd|Rm|0011|Rn"                     , "ext": "ARMv4+"},

    {"inst": "lsrS Rd!=HI, Rn!=HI, #n"                                 , "t16": "0000|1|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "lsrS Rd!=XX, Rn!=XX, #n"                                 , "t32": "1110|101|0010|1|1111|0|n:3|Rd|n:2|01|Rn"                , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "lsrS Rd, Rn, #n"                                         , "a32": "cond|000|1101|1|0000|Rd|n:5|010|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "lsrS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|011|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "lsrS Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1110|101|0001|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "lsrS Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|1101|1|0000|Rd|Rm|0011|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "isb #immZ"                                               , "t32": "1111|001|1101|1|1111|1000|1111|0110|immZ:4"             , "ext": "ARMv7+", "it": "any"},

    {"inst": "mcr #CP, #Op1, Rs!=XX, Cn, Cm, {#Op2}"                   , "t32": "1110|111|0|Op1:3|0|Cn:4|Rs|CP:4|Op2:3|1|Cm:4"           , "ext": "ARMv6T2+"},
    {"inst": "mcr #CP, #Op1, Rs!=PC, Cn, Cm, {#Op2}"                   , "a32": "cond|111|0|Op1:3|0|Cn:4|Rs|CP:4|Op2:3|1|Cm:4"           , "ext": "ARMv4+"},

    {"inst": "mcr2 #CP, #Op1, Rs!=XX, Cn, Cm, {#Op2}"                  , "t32": "1111|111|0|Op1:3|0|Cn:4|Rs|CP:4|Op2:3|1|Cm:4"           , "ext": "ARMv6T2+"},
    {"inst": "mcr2 #CP, #Op1, Rs!=PC, Cn, Cm, {#Op2}"                  , "a32": "1111|111|0|Op1:3|0|Cn:4|Rs|CP:4|Op2:3|1|Cm:4"           , "ext": "ARMv4+"},

    {"inst": "mcrr #CP, #Op1, Rs!=XX, Rs2!=XX, Cm"                     , "t32": "1110|110|0010|0|Rs2|Rs|CP:4|Op1:4|Cm:4"                 , "ext": "ARMv6T2+"},
    {"inst": "mcrr #CP, #Op1, Rs!=PC, Rs2!=PC, Cm"                     , "a32": "cond|110|0010|0|Rs2|Rs|CP:4|Op1:4|Cm:4"                 , "ext": "ARMv5TE+"},

    {"inst": "mcrr2 #CP, #Op1, Rs!=XX, Rs2!=XX, Cm"                    , "t32": "1111|110|0010|0|Rs2|Rs|CP:4|Op1:4|Cm:4"                 , "ext": "ARMv6T2+"},
    {"inst": "mcrr2 #CP, #Op1, Rs!=PC, Rs2!=PC, Cm"                    , "a32": "1111|110|0010|0|Rs2|Rs|CP:4|Op1:4|Cm:4"                 , "ext": "ARMv5TE+"},

    {"inst": "mla Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                      , "t32": "1111|101|1000|0|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mla Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                      , "a32": "cond|000|0001|0|Rd|Ra|Rm|1001|Rn"                       , "ext": "ARMv4+"},
    {"inst": "mlaS Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                     , "a32": "cond|000|0001|1|Rd|Ra|Rm|1001|Rn"                       , "ext": "ARMv4+", "apsr": "NZ=W"},

    {"inst": "mls Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                      , "t32": "1111|101|1000|0|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mls Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                      , "a32": "cond|000|0011|0|Rd|Ra|Rm|1001|Rn"                       , "ext": "ARMv6T2+"},

    {"inst": "mov Rd, Rn"                                              , "t16": "0100|0110|Rd:1|Rn:4|Rd:3"                               , "ext": "ARMv4T+", "it": "in", "mod": "ARMv6T2_IF_LOW"},
    {"inst": "mov Rx!=HI, Rx!=HI, lsl Rm!=HI"                          , "t16": "0100|000|010|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rx!=HI, Rx!=HI, lsr Rm!=HI"                          , "t16": "0100|000|011|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rx!=HI, Rx!=HI, asr Rm!=HI"                          , "t16": "0100|000|100|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rx!=HI, Rx!=HI, ror Rm!=HI"                          , "t16": "0100|000|111|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rd!=HI, Rn!=HI, lsl #n"                              , "t16": "0000|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rd!=HI, Rn!=HI, lsr #n"                              , "t16": "0000|1|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rd!=HI, Rn!=HI, asr #n"                              , "t16": "0001|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rd!=HI, #immZ"                                       , "t16": "0010|0|Rd:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mov Rd!=PC, Rn"                                          , "t32": "1110|101|0010|0|1111|0000|Rd|0000|Rn"                   , "ext": "ARMv6T2+", "it": "any", "mod": "UNPRED_COMPLEX"},
    {"inst": "mov Rd!=XX, Rn!=XX, sop #n"                              , "t32": "1110|101|0010|0|1111|0|n:3|Rd|n:2|sop:2|Rn"             , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mov Rd!=XX, Rn!=XX, sop Rm!=XX"                          , "t32": "1111|101|00|sop:2|0|Rn|1111|Rd|0000|Rm"                 , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mov Rd!=XX, #immA"                                       , "t32": "1111|0|imm:1|0|0010|0|1111|0|imm:3|Rd|imm:8"            , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mov Rd, Rn, sop Rs"                                      , "a32": "cond|000|1101|0|0000|Rd|Rs|0|sop:2|1|Rn"                , "ext": "ARMv4+"},
    {"inst": "mov Rd, Rn, {sop #n}"                                    , "a32": "cond|000|1101|0|0000|Rd|n:5|sop:2|0|Rn"                 , "ext": "ARMv4+"},
    {"inst": "mov Rd, #immA"                                           , "a32": "cond|001|1101|0|0000|Rd|imm:12"                         , "ext": "ARMv4+"},

    {"inst": "movS Rd!=HI, Rn!=HI"                                     , "t16": "0000|0000|00|Rn:3|Rd:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rx!=HI, Rx!=HI, lsl Rm!=HI"                         , "t16": "0100|000|010|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rx!=HI, Rx!=HI, lsr Rm!=HI"                         , "t16": "0100|000|011|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rx!=HI, Rx!=HI, asr Rm!=HI"                         , "t16": "0100|000|100|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rx!=HI, Rx!=HI, ror Rm!=HI"                         , "t16": "0100|000|111|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rd!=HI, Rn!=HI, lsl #n"                             , "t16": "0000|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rd!=HI, Rn!=HI, lsr #n"                             , "t16": "0000|1|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rd!=HI, Rn!=HI, asr #n"                             , "t16": "0001|0|n:5|Rn:3|Rd:3"                                   , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rd!=HI, #immZ"                                      , "t16": "0010|0|Rd:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "movS Rd!=XX, Rn"                                         , "t32": "1110|101|0010|1|1111|0000|Rd|0000|Rn"                   , "ext": "ARMv6T2+", "it": "any", "mod": "UNPRED_COMPLEX"},
    {"inst": "movS Rd!=XX, Rn!=XX, sop Rm!=XX"                         , "t32": "1110|101|00|sop:2|1|Rn|1111|Rd|0000|Rm"                 , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "movS Rd!=XX, Rn!=XX, sop #n"                             , "t32": "1110|101|0010|1|1111|0|n:3|Rd|n:2|sop:2|Rn"             , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "movS Rd!=XX, #immA"                                      , "t32": "1111|0|imm:1|0|0010|1|1111|0|imm:3|Rd|imm:8"            , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "movS Rd!=PC, Rn, sop Rs"                                 , "a32": "cond|000|1101|1|0000|Rd|Rs|0|sop:2|1|Rn"                , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "movS Rd!=PC, Rn, {sop #n}"                               , "a32": "cond|000|1101|1|0000|Rd|n:5|sop:2|0|Rn"                 , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "movS Rd!=PC, #immA"                                      , "a32": "cond|001|1101|1|0000|Rd|imm:12"                         , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "movt Rx!=XX, #immZ"                                      , "t32": "1111|0|immZ:1|1|0110|0|immZ:4|0|immZ:3|Rx|immZ:8"       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "movt Rx!=PC, #immZ"                                      , "a32": "cond|001|1010|0|immZ:4|Rx|immZ:12"                      , "ext": "ARMv6T2+"},

    {"inst": "movw Rd!=XX, #immZ"                                      , "t32": "1111|0|immZ:1|1|0010|0|immZ:4|0|immZ:3|Rd|immZ:8"       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "movw Rd!=PC, #immZ"                                      , "a32": "cond|001|1000|0|immZ:4|Rd|immZ:12"                      , "ext": "ARMv6T2+"},

    {"inst": "mrc #CP, #op1, Rd!=XX, Cn, Cm, {#op2}"                   , "t32": "1110|111|0|op1:3|1|Cn:4|Rd|CP:4|op2:3|1|Cm:4"           , "ext": "ARMv4+"},
    {"inst": "mrc #CP, #op1, Rd!=PC, Cn, Cm, {#op2}"                   , "a32": "cond|111|0|op1:3|1|Cn:4|Rd|CP:4|op2:3|1|Cm:4"           , "ext": "ARMv4+"},

    {"inst": "mrc2 #CP, #op1, Rd!=XX, Cn, Cm, {#op2}"                  , "t32": "1111|111|0|op1:3|1|Cn:4|Rd|CP:4|op2:3|1|Cm:4"           , "ext": "ARMv4+"},
    {"inst": "mrc2 #CP, #op1, Rd!=PC, Cn, Cm, {#op2}"                  , "a32": "1111|111|0|op1:3|1|Cn:4|Rd|CP:4|op2:3|1|Cm:4"           , "ext": "ARMv4+"},

    {"inst": "mrrc #CP, #op1, Rd!=XX, Rd2!=Rd, Cm"                     , "t32": "1110|110|0010|1|Rd2|Rd|CP:4|op1:4|Cm:4"                 , "ext": "ARMv5TE+"},
    {"inst": "mrrc #CP, #op1, Rd!=PC, Rd2!=Rd, Cm"                     , "a32": "cond|110|0010|1|Rd2|Rd|CP:4|op1:4|Cm:4"                 , "ext": "ARMv5TE+"},

    {"inst": "mrrc2 #CP, #op1, Rd!=XX, Rd2!=Rd, Cm"                    , "t32": "1111|110|0010|1|Rd2|Rd|CP:4|op1:4|Cm:4"                 , "ext": "ARMv5TE+"},
    {"inst": "mrrc2 #CP, #op1, Rd!=PC, Rd2!=Rd, Cm"                    , "a32": "1111|110|0010|1|Rd2|Rd|CP:4|op1:4|Cm:4"                 , "ext": "ARMv5TE+"},

    {"inst": "mrs Rd!=XX, #APSR"                                       , "t32": "11110011111|APSR:1|1111|1000|Rd|0000|0000"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mrs Rd!=PC, #APSR"                                       , "a32": "cond|000|10|APSR:1|0|0|1111|Rd|0000|0000|0000"          , "ext": "ARMv4+"},

    {"inst": "msr #APSR, Rn!=PC"                                       , "t32": "1111|001|1100|0|Rn|1100|APSR:2|00|0000|0000"            , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "msr #APSR, Rn!=PC"                                       , "a32": "cond|000|1001|0|APSR:2|00|1111|0000|0000|Rn"            , "ext": "ARMv4+"},
    {"inst": "msr #APSR, #immA"                                        , "a32": "cond|001|1001|0|APSR:2|00|1111|imm:12"                  , "ext": "ARMv4+"},

    {"inst": "mul Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|001|101|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mulS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|001|101|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "mul Rd!=XX, Rn!=XX, Rm!=XX"                              , "t32": "1111|101|1000|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mul Rd!=PC, Rn!=PC, Rm!=PC"                              , "a32": "cond|000|0000|0|Rd|0000|Rm|1001|Rn"                     , "ext": "ARMv4+"},
    {"inst": "mulS Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|0000|1|Rd|0000|Rm|1001|Rn"                     , "ext": "ARMv4+"},

    {"inst": "mvn Rd!=HI, Rn!=HI"                                      , "t16": "0100|001|111|Rn:3|Rd:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "mvn Rd!=XX, Rn!=XX, {sop #n}"                            , "t32": "1110|101|0011|0|1111|0|n:3|Rd|n:2|sop:2|Rn"             , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mvn Rd!=XX, #immA"                                       , "t32": "1111|0|imm:1|0|0011|0|1111|0|imm:3|Rd|imm:8"            , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "mvn Rd!=PC, Rn!=PC, sop Rs!=PC"                          , "a32": "cond|000|1111|0|0000|Rd|Rs|0|sop:2|1|Rn"                , "ext": "ARMv4+"},
    {"inst": "mvn Rd, Rn, {sop #n}"                                    , "a32": "cond|000|1111|0|0000|Rd|n:5|sop:2|0|Rn"                 , "ext": "ARMv4+"},
    {"inst": "mvn Rd, #immA"                                           , "a32": "cond|001|1111|0|0000|Rd|imm:12"                         , "ext": "ARMv4+"},

    {"inst": "mvnS Rd!=HI, Rn!=HI"                                     , "t16": "0100|001|111|Rn:3|Rd:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "mvnS Rd!=XX, Rn!=XX, {sop #n}"                           , "t32": "1110|101|0011|1|1111|0|n:3|Rd|n:2|sop:2|Rn"             , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "mvnS Rd!=XX, #immA"                                      , "t32": "1111|0|imm:1|0|0011|1|1111|0|imm:3|Rd|imm:8"            , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "mvnS Rd!=PC, Rn!=PC, sop Rs!=PC"                         , "a32": "cond|000|1111|1|0000|Rd|Rs|0|sop:2|1|Rn"                , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "mvnS Rd!=PC, Rn, {sop #n}"                               , "a32": "cond|000|1111|1|0000|Rd|n:5|sop:2|0|Rn"                 , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "mvnS Rd!=PC, #immA"                                      , "a32": "cond|001|1111|1|0000|Rd|imm:12"                         , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "nop"                                                     , "t16": "1011|111|1000|0|0000"                                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "nop"                                                     , "t32": "1111|001|1101|0|1111|1000|0000|0000|0000"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "nop"                                                     , "a32": "cond|001|1001|0|0000|1111|0000|0000|0000"               , "ext": "ARMv6K+"},

    {"inst": "orn Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|0011|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "orn Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|0011|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},

    {"inst": "ornS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|0011|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "ornS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|0011|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},

    {"inst": "orr Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|001|100|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "orr Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|0010|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "orr Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|0010|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "orr Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|1100|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "orr Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|1100|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "orr Rd, Rn, #immA"                                       , "a32": "cond|001|1100|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "orrS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|001|100|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "orrS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|0010|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "orrS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|0010|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "orrS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|1100|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "orrS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|1100|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "orrS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|1100|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "pkhbt Rd!=XX, Rn!=XX, Rm!=XX, {lsl #n}"                  , "t32": "1110|101|0110|0|Rn|0|n:3|Rd|n:2|00|Rm"                  , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pkhbt Rd!=PC, Rn!=PC, Rm!=PC, {lsl #n}"                  , "a32": "cond|011|0100|0|Rn|Rd|n:5|0|01|Rm"                      , "ext": "ARMv6+"},

    {"inst": "pkhtb Rd!=XX, Rn!=XX, Rm!=XX, {asr #n}"                  , "t32": "1110|101|0110|0|Rn|0|n:3|Rd|n:2|10|Rm"                  , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pkhtb Rd!=PC, Rn!=PC, Rm!=PC, {asr #n}"                  , "a32": "cond|011|0100|0|Rn|Rd|n:5|1|01|Rm"                      , "ext": "ARMv6+"},

    {"inst": "pld [Rn!=PC, Rm!=XX, {lsl #n}]"                          , "t32": "1111|100|0000|1|Rn|1111|0000|00|n:2|Rm"                 , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pld [Rn!=PC, #off]"                                      , "t32": "1111|100|0100|1|Rn|1111|off:12"                         , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pld [Rn!=PC, #-off]"                                     , "t32": "1111|100|0000|1|Rn|1111|1100|off:8"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pld [Rn==PC, #+/-off]"                                   , "t32": "1111|100|0U00|1|Rn|1111|off:12"                         , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pld [Rn, +/-Rm!=PC, {sop #n}]"                           , "a32": "1111|011|1U10|1|Rn|1111|n:5|sop:2|0|Rm"                 , "ext": "ARMv5TE+"},
    {"inst": "pld [Rn, #+/-off]"                                       , "a32": "1111|010|1U10|1|Rn|1111|off:12"                         , "ext": "ARMv5TE+"},

    {"inst": "pldw [Rn!=PC, Rm!=XX, {lsl #n}]"                         , "t32": "1111|100|0001|1|Rn|1111|0000|00|n:2|Rm"                 , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pldw [Rn!=PC, #off]"                                     , "t32": "1111|100|0101|1|Rn|1111|off:12"                         , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pldw [Rn!=PC, #-off]"                                    , "t32": "1111|100|0001|1|Rn|1111|1100|off:8"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pldw [Rn, +/-Rm!=PC, {sop #n}]"                          , "a32": "1111|011|1U00|1|Rn|1111|n:5|sop:2|0|Rm"                 , "ext": "ARMv7+ MP"},
    {"inst": "pldw [Rn, #+/-off]"                                      , "a32": "1111|010|1U00|1|Rn|1111|off:12"                         , "ext": "ARMv7+ MP"},

    {"inst": "pli [Rn!=PC, Rm!=XX, {lsl #n}]"                          , "t32": "1111|100|1000|1|Rn|1111|0000|00|n:2|Rm"                 , "ext": "ARMv7+", "it": "any"},
    {"inst": "pli [Rn!=PC, #off]"                                      , "t32": "1111|100|1100|1|Rn|1111|off:12"                         , "ext": "ARMv7+", "it": "any"},
    {"inst": "pli [Rn!=PC, #-off]"                                     , "t32": "1111|100|1000|1|Rn|1111|1100|off:8"                     , "ext": "ARMv7+", "it": "any"},
    {"inst": "pli [Rn==PC, #+/-off]"                                   , "t32": "1111|100|1U00|1|Rn|1111|off:12"                         , "ext": "ARMv7+", "it": "any"},
    {"inst": "pli [Rn, +/-Rm!=PC, {sop #n}]"                           , "a32": "1111|011|0U10|1|Rn|1111|n:5|sop:2|0|Rm"                 , "ext": "ARMv7+"},
    {"inst": "pli [Rn, #+/-off]"                                       , "a32": "1111|010|0U10|1|Rn|1111|off:12"                         , "ext": "ARMv7+"},

    {"inst": "pop Rd!=SP"                                              , "t32": "1111|100|0010|1|1101|Rd|1011|00000100"                  , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pop Rd!=SP"                                              , "a32": "cond|010|0100|1|1101|Rd|0000|0000|0100"                 , "ext": "ARMv4+"},
    {"inst": "pop RdList"                                              , "t16": "1011|110|RdList[15]|RdList[7:0]"                        , "ext": "ARMv4T+", "it": "any"},
    {"inst": "pop RdList"                                              , "t32": "1110|100|0101|1|1101|RdList[15:14]|0|RdList[12:0]"      , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "pop RdList"                                              , "a32": "cond|100|0101|1|1101|RdList:16"                         , "ext": "ARMv4+"},

    {"inst": "push Rs!=XX"                                             , "t32": "1111|100|0010|0|1101|Rs|1101|0000|0100"                 , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "push Rs!=SP"                                             , "a32": "cond|010|1001|0|1101|Rs|0000|0000|0100"                 , "ext": "ARMv4+"},
    {"inst": "push RsList"                                             , "t16": "1011|010|RsList[14]|RsList[7:0]"                        , "ext": "ARMv4T+", "it": "any"},
    {"inst": "push RsList"                                             , "t32": "1110|100|0101|0|1101|0|RsList[14]|0|RsList[12:0]"       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "push RsList"                                             , "a32": "cond|100|1001|0|1101|RsList:16"                         , "ext": "ARMv4+"},

    {"inst": "qadd Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0100|0|Rm|1111|Rd|1000|Rn"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "qadd Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|1000|0|Rm|Rd|0000|0101|Rn"                     , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "qadd16 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0100|1|Rm|1111|Rd|0001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "qadd16 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0001|0|Rn|Rd|1111|0001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "qadd8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0100|0|Rm|1111|Rd|0001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "qadd8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0001|0|Rn|Rd|1111|1001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "qasx Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0101|0|Rm|1111|Rd|0001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "qasx Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|0001|0|Rn|Rd|1111|0011|Rm"                     , "ext": "ARMv6+"},

    {"inst": "qdadd Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0100|0|Rm|1111|Rd|1001|Rn"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "qdadd Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|000|1010|0|Rm|Rd|0000|0101|Rn"                     , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "qdsub Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0100|0|Rm|1111|Rd|1011|Rn"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "qdsub Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|000|1011|0|Rm|Rd|0000|0101|Rn"                     , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "qsax Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0111|0|Rm|1111|Rd|0001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "qsax Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|0001|0|Rn|Rd|1111|0101|Rm"                     , "ext": "ARMv6+"},

    {"inst": "qsub Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0100|0|Rm|1111|Rd|1010|Rn"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "qsub Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|1001|0|Rm|Rd|0000|0101|Rn"                     , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "qsub16 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0110|1|Rm|1111|Rd|0001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "qsub16 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0001|0|Rn|Rd|1111|0111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "qsub8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0110|0|Rm|1111|Rd|0001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "qsub8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0001|0|Rn|Rd|1111|1111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "rbit Rd!=XX, Rn!=XX"                                     , "t32": "1111|101|0100|1|Rm|1111|Rd|1010|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "rbit Rd!=PC, Rn!=PC"                                     , "a32": "cond|011|0111|1|1111|Rd|1111|0011|Rn"                   , "ext": "ARMv6T2+"},

    {"inst": "rev Rd!=HI, Rn!=HI"                                      , "t16": "1011|101|000|Rn:3|Rd:3"                                 , "ext": "ARMv6T+", "it": "any"},
    {"inst": "rev Rd!=XX, Rn!=XX"                                      , "t32": "1111|101|0100|1|Rm|1111|Rd|1000|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "rev Rd!=PC, Rn!=PC"                                      , "a32": "cond|011|0101|1|1111|Rd|1111|0011|Rn"                   , "ext": "ARMv6+"},

    {"inst": "rev16 Rd!=HI, Rn!=HI"                                    , "t16": "1011|101|001|Rn:3|Rd:3"                                 , "ext": "ARMv6T+", "it": "any"},
    {"inst": "rev16 Rd!=XX, Rn!=XX"                                    , "t32": "1111|101|0100|1|Rm|1111|Rd|1001|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "rev16 Rd!=PC, Rn!=PC"                                    , "a32": "cond|011|0101|1|1111|Rd|1111|1011|Rn"                   , "ext": "ARMv6+"},

    {"inst": "revsh Rd!=HI, Rn!=HI"                                    , "t16": "1011|101|011|Rn:3|Rd:3"                                 , "ext": "ARMv6T+", "it": "any"},
    {"inst": "revsh Rd!=XX, Rn!=XX"                                    , "t32": "1111|101|0100|1|Rm|1111|Rd|1011|Rn"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "revsh Rd!=PC, Rn!=PC"                                    , "a32": "cond|011|0111|1|1111|Rd|1111|1011|Rn"                   , "ext": "ARMv6+"},

    {"inst": "rfe [Rn!=PC]{!}"                                         , "a32": "1111|100|010|W|1|Rn|0000|1010|0000|0000"                , "ext": "ARMv6+"},
    {"inst": "rfeda [Rn!=PC]{!}"                                       , "a32": "1111|100|000|W|1|Rn|0000|1010|0000|0000"                , "ext": "ARMv6+"},
    {"inst": "rfedb [Rn!=PC]{!}"                                       , "a32": "1111|100|100|W|1|Rn|0000|1010|0000|0000"                , "ext": "ARMv6+"},
    {"inst": "rfeib [Rn!=PC]{!}"                                       , "a32": "1111|100|110|W|1|Rn|0000|1010|0000|0000"                , "ext": "ARMv6+"},

    {"inst": "ror Rd!=XX, Rn!=XX, #n"                                  , "t32": "1110|101|0010|0|1111|0|n:3|Rd|n:2|11|Rn"                , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ror Rd, Rn, #n"                                          , "a32": "cond|000|1101|0|0000|Rd|n:5|110|Rn"                     , "ext": "ARMv4+"},
    {"inst": "ror Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|111|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "ror Rd!=XX, Rn!=XX, Rm!=XX"                              , "t32": "1111|101|0011|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "ror Rd!=PC, Rn!=PC, Rm!=PC"                              , "a32": "cond|000|1101|0|0000|Rd|Rm|0111|Rn"                     , "ext": "ARMv4+"},

    {"inst": "rorS Rd!=XX, Rn!=XX, #n"                                 , "t32": "1110|101|0010|1|1111|0|n:3|Rd|n:2|11|Rn"                , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "rorS Rd, Rn, #n"                                         , "a32": "cond|000|1101|1|0000|Rd|n:5|110|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "rorS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|111|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZC=W"},
    {"inst": "rorS Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0011|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "rorS Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|000|1101|1|0000|Rd|Rm|0111|Rn"                     , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "rsb Rd!=HI, Rn!=HI, #zero"                               , "t16": "0100|001001|Rn:3|Rd:3"                                  , "ext": "ARMv4T+", "it": "in"},
    {"inst": "rsb Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1110|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "rsb Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|1110|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "rsb Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0011|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "rsb Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0011|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "rsb Rd, Rn, #immA"                                       , "a32": "cond|001|0011|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "rsbS Rd!=HI, Rn!=HI, #zero"                              , "t16": "0100|001001|Rn:3|Rd:3"                                  , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "rsbS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1110|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "rsbS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|1110|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "rsbS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0011|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "rsbS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0011|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "rsbS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0011|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "rsc Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0111|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "rsc Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0111|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "rsc Rd, Rn, #immA"                                       , "a32": "cond|001|0111|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "rscS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0111|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "rscS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0111|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "rscS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0111|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "rrx Rd!=XX, Rn!=XX"                                      , "t32": "1110|101|0010|0|1111|0000|Rd|0011|Rn"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "rrx Rd, Rn"                                              , "a32": "cond|000|1101|0|0000|Rd|00000|110|Rn"                   , "ext": "ARMv4+"},

    {"inst": "rrxS Rd!=XX, Rn!=XX"                                     , "t32": "1110|101|0010|1|1111|0000|Rd|0011|Rn"                   , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "rrxS Rd, Rn"                                             , "a32": "cond|000|1101|1|0000|Rd|00000|110|Rn"                   , "ext": "ARMv4+", "apsr": "NZ=W", "apsr": "C=X"},

    {"inst": "sadd16 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0100|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "sadd16 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0000|1|Rn|Rd|1111|0001|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "sadd8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0100|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "sadd8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0000|1|Rn|Rd|1111|1001|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "sasx Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0101|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "sasx Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|0000|1|Rn|Rd|1111|0011|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "sbc Rx!=HI, Rx!=HI, Rm!=HI"                              , "t16": "0100|000|110|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "in"},
    {"inst": "sbc Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1011|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sbc Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|1011|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sbc Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                  , "a32": "cond|000|0110|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "sbc Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0110|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "sbc Rd, Rn, #immA"                                       , "a32": "cond|001|0110|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "sbcS Rx!=HI, Rx!=HI, Rm!=HI"                             , "t16": "0100|000|110|Rm:3|Rx:3"                                 , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "sbcS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1011|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sbcS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|1011|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "sbcS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0110|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "sbcS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0110|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "sbcS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0110|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "sbfx Rd!=XX, Rn!=XX, #lsb, #width"                       , "t32": "1111|001|1010|0|Rn|0|lsb:3|Rd|lsb:2|0|widthM1:5"        , "ext": "ARMv6T2+", "imm": "SbfxUbfxImm(lsb, width)", "it": "any"},
    {"inst": "sbfx Rd!=PC, Rn!=PC, #lsb, #width"                       , "a32": "cond|011|1101|widthM1:5|Rd|lsb:5|101|Rn"                , "ext": "ARMv6T2+", "imm": "SbfxUbfxImm(lsb, width)"},

    {"inst": "sdiv Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|1100|1|Rn|1111|Rd|1111|Rm"                     , "ext": "IDIVT", "it": "any"},
    {"inst": "sdiv Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|1000|1|Rd|1111|Rm|0001|Rn"                     , "ext": "IDIVA"},

    {"inst": "sel Rd!=XX, Rn!=XX, Rm!=XX"                              , "t32": "1111|101|0101|0|Rn|1111|Rd|1000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sel Rd!=PC, Rn!=PC, Rm!=PC"                              , "a32": "cond|011|0100|0|Rn|Rd|1111|1011|Rm"                     , "ext": "ARMv6+", "apsr": "GE=R"},

    {"inst": "setend #immZ"                                            , "t16": "1011|0110|010|1|immZ:1|000"                             , "ext": "ARMv6+", "it": "out"},
    {"inst": "setend #immZ"                                            , "a32": "1111|000|1000|0|0001|0000|00|immZ:1|0|0000|0000"        , "ext": "ARMv6+ ARMv8-"},

    {"inst": "sev"                                                     , "t16": "1011|1111|0100|0000"                                    , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sev"                                                     , "t32": "1111|001|1101|0|1111|1000|0000|0000|0000"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sev"                                                     , "a32": "cond|001|1001|0|0000|1111|0000|0000|0100"               , "ext": "ARMv7+ ARMv6K"},

    {"inst": "sevl"                                                    , "a32": "cond|001|1001|0|0000|1111|0000|0000|0101"               , "?": "?"},

    {"inst": "sha1c.32 Vx, Vn, Vm"                                     , "t32": "1110|11110|Vx'|00|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},
    {"inst": "sha1c.32 Vx, Vn, Vm"                                     , "a32": "1111|00100|Vx'|00|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},

    {"inst": "sha1h.32 Vd, Vn"                                         , "t32": "1111|11111|Vd'|11|1001|Vd|0|0101|1|Vn'|0|Vn"            , "ext": "SHA1"},
    {"inst": "sha1h.32 Vd, Vn"                                         , "a32": "1111|00111|Vd'|11|1001|Vd|0|0101|1|Vn'|0|Vn"            , "ext": "SHA1"},

    {"inst": "sha1m.32 Vx, Vn, Vm"                                     , "t32": "1110|11110|Vx'|10|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},
    {"inst": "sha1m.32 Vx, Vn, Vm"                                     , "a32": "1111|00100|Vx'|10|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},

    {"inst": "sha1p.32 Vx, Vn, Vm"                                     , "t32": "1110|11110|Vx'|01|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},
    {"inst": "sha1p.32 Vx, Vn, Vm"                                     , "a32": "1111|00100|Vx'|01|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},

    {"inst": "sha1su0.32 Vx, Vn, Vm"                                   , "t32": "1110|11110|Vx'|11|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},
    {"inst": "sha1su0.32 Vx, Vn, Vm"                                   , "a32": "1111|00100|Vx'|11|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA1"},

    {"inst": "sha1su1.32 Vx, Vn"                                       , "t32": "1111|11111|Vx'|11|1010|Vx|0|0111|0|Vn'|0|Vn"            , "ext": "SHA1"},
    {"inst": "sha1su1.32 Vx, Vn"                                       , "a32": "1111|00111|Vx'|11|1010|Vx|0|0111|0|Vn'|0|Vn"            , "ext": "SHA1"},

    {"inst": "sha256h.32 Vx, Vn, Vm"                                   , "t32": "1111|11110|Vx'|00|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA256"},
    {"inst": "sha256h.32 Vx, Vn, Vm"                                   , "a32": "1111|00110|Vx'|00|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA256"},

    {"inst": "sha256h2.32 Vx, Vn, Vm"                                  , "t32": "1111|11110|Vx'|01|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA256"},
    {"inst": "sha256h2.32 Vx, Vn, Vm"                                  , "a32": "1111|00110|Vx'|01|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA256"},

    {"inst": "sha256su0.32 Vx, Vn"                                     , "t32": "1111|11111|Vx'|11|1010|Vx|0|0111|1|Vn'|0|Vn"            , "ext": "SHA256"},
    {"inst": "sha256su0.32 Vx, Vn"                                     , "a32": "1111|00111|Vx'|11|1010|Vx|0|0111|1|Vn'|0|Vn"            , "ext": "SHA256"},

    {"inst": "sha256su1.32 Vx, Vn, Vm"                                 , "t32": "1111|11110|Vx'|10|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA256"},
    {"inst": "sha256su1.32 Vx, Vn, Vm"                                 , "a32": "1111|00110|Vx'|10|Vn|Vx|1100|Vn'|1|Vm'|0|Vm"            , "ext": "SHA256"},

    {"inst": "shadd16 Rd!=XX, Rn!=XX, Rm!=XX"                          , "t32": "1111|101|0100|1|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "shadd16 Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|011|0001|1|Rn|Rd|1111|0001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "shadd8 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0100|0|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "shadd8 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0001|1|Rn|Rd|1111|1001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "shasx Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0101|0|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "shasx Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0001|1|Rn|Rd|1111|0011|Rm"                     , "ext": "ARMv6+"},

    {"inst": "shsax Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0111|0|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "shsax Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0001|1|Rn|Rd|1111|0101|Rm"                     , "ext": "ARMv6+"},

    {"inst": "shsub16 Rd!=XX, Rn!=XX, Rm!=XX"                          , "t32": "1111|101|0110|1|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "shsub16 Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|011|0001|1|Rn|Rd|1111|0111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "shsub8 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0110|0|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "shsub8 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0001|1|Rn|Rd|1111|1111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "smc #immZ"                                               , "t32": "1111|011|1111|1|immZ:4|1000|000000000000"               , "ext": "SECURITY"},
    {"inst": "smc #immZ"                                               , "a32": "cond|000|1011|0|000000000000|0111|immZ:4"               , "ext": "SECURITY"},

    {"inst": "smlabb Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|1|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlabb Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|000|1000|0|Rd|Ra|Rm|1000|Rn"                       , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "smlabt Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|1|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlabt Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|000|1000|0|Rd|Ra|Rm|1100|Rn"                       , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "smlatb Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|1|Rn|Ra|Rd|0010|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlatb Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|000|1000|0|Rd|Ra|Rm|1010|Rn"                       , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "smlatt Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|1|Rn|Ra|Rd|0011|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlatt Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|000|1000|0|Rd|Ra|Rm|1110|Rn"                       , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "smlad Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                    , "t32": "1111|101|1001|0|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlad Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                    , "a32": "cond|011|1000|0|Rd|Ra|Rm|0001|Rn"                       , "ext": "ARMv6+", "apsr": "Q=X"},

    {"inst": "smladx Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|0|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smladx Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|011|1000|0|Rd|Ra|Rm|0011|Rn"                       , "ext": "ARMv6+", "apsr": "Q=X"},

    {"inst": "smlal RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"                , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|0000|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlal RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"                , "a32": "cond|000|0111|0|RxHi|RxLo|Rm|1001|Rn"                   , "ext": "ARMv4+"},
    {"inst": "smlalS RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"               , "a32": "cond|000|0111|1|RxHi|RxLo|Rm|1001|Rn"                   , "ext": "ARMv4+", "apsr": "NZ=W"},

    {"inst": "smlalbb RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"              , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|1000|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlalbb RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"              , "a32": "cond|000|1010|0|RxHi|RxLo|Rm|1000|Rn"                   , "ext": "ARMv5TE+"},

    {"inst": "smlalbt RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"              , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|1001|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlalbt RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"              , "a32": "cond|000|1010|0|RxHi|RxLo|Rm|1100|Rn"                   , "ext": "ARMv5TE+"},

    {"inst": "smlaltb RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"              , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|1010|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlaltb RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"              , "a32": "cond|000|1010|0|RxHi|RxLo|Rm|1010|Rn"                   , "ext": "ARMv5TE+"},

    {"inst": "smlaltt RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"              , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|1011|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlaltt RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"              , "a32": "cond|000|1010|0|RxHi|RxLo|Rm|1110|Rn"                   , "ext": "ARMv5TE+"},

    {"inst": "smlald RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"               , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|1100|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlald RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"               , "a32": "cond|011|1010|0|RxHi|RxLo|Rm|0001|Rn"                   , "ext": "ARMv6+"},
    {"inst": "smlaldx RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"              , "t32": "1111|101|1110|0|Rn|RxLo|RxHi|1101|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smlaldx RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"              , "a32": "cond|011|1010|0|RxHi|RxLo|Rm|0011|Rn"                   , "ext": "ARMv6+"},

    {"inst": "smlawb Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|1|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlawb Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|000|1001|0|Rd|Ra|Rm|1000|Rn"                       , "ext": "ARMv5TE+", "apsr": "Q=X"},
    {"inst": "smlawt Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1001|1|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlawt Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|000|1001|0|Rd|Ra|Rm|1100|Rn"                       , "ext": "ARMv5TE+", "apsr": "Q=X"},

    {"inst": "smlsd Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                    , "t32": "1111|101|1010|1|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlsd Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                    , "a32": "cond|011|1000|0|Rd|Ra|Rm|0101|Rn"                       , "ext": "ARMv6+", "apsr": "Q=X"},

    {"inst": "smlsdx Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1010|1|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlsdx Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|011|1000|0|Rd|Ra|Rm|0111|Rn"                       , "ext": "ARMv6+", "apsr": "Q=X"},

    {"inst": "smlsld RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"               , "t32": "1111|101|1110|1|Rn|RxLo|RxHi|1100|Rm"                   , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlsld RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"               , "a32": "cond|011|1010|0|RxHi|RxLo|Rm|0101|Rn"                   , "ext": "ARMv6+"},

    {"inst": "smlsldx RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"              , "t32": "1111|101|1110|1|Rn|RxLo|RxHi|1101|Rm"                   , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smlsldx RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"              , "a32": "cond|011|1010|0|RxHi|RxLo|Rm|0111|Rn"                   , "ext": "ARMv6+"},

    {"inst": "smmla Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                    , "t32": "1111|101|1010|1|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smmla Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                    , "a32": "cond|011|1010|1|Rd|Ra|Rm|0001|Rn"                       , "ext": "ARMv6+"},

    {"inst": "smmlar Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1010|1|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smmlar Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|011|1010|1|Rd|Ra|Rm|0011|Rn"                       , "ext": "ARMv6+"},

    {"inst": "smmls Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                    , "t32": "1111|101|1011|0|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smmls Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                    , "a32": "cond|011|1010|1|Rd|Ra|Rm|1101|Rn"                       , "ext": "ARMv6+"},

    {"inst": "smmlsr Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1011|0|Rn|Ra|Rd|0001|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smmlsr Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|011|1010|1|Rd|Ra|Rm|1111|Rn"                       , "ext": "ARMv6+"},

    {"inst": "smmul Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|1010|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smmul Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|1010|1|Rd|1111|Rm|0001|Rn"                     , "ext": "ARMv6+"},

    {"inst": "smmulr Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1010|1|Rn|1111|Rd|0001|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smmulr Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|1010|1|Rd|1111|Rm|0011|Rn"                     , "ext": "ARMv6+"},

    {"inst": "smuad Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|1001|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smuad Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|1000|0|Rd|1111|Rm|0001|Rn"                     , "ext": "ARMv6+", "apsr": "Q=X"},

    {"inst": "smuadx Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1001|0|Rn|1111|Rd|0001|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "smuadx Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|1000|0|Rd|1111|Rm|0011|Rn"                     , "ext": "ARMv6+", "apsr": "Q=X"},

    {"inst": "smulbb Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1000|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smulbb Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1011|0|Rd|0000|Rm|1000|Rn"                     , "ext": "ARMv5TE+"},

    {"inst": "smulbt Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1000|1|Rn|1111|Rd|0001|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smulbt Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1011|0|Rd|0000|Rm|1100|Rn"                     , "ext": "ARMv5TE+"},

    {"inst": "smultb Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1000|1|Rn|1111|Rd|0010|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smultb Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1011|0|Rd|0000|Rm|1010|Rn"                     , "ext": "ARMv5TE+"},

    {"inst": "smultt Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1000|1|Rn|1111|Rd|0011|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smultt Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1011|0|Rd|0000|Rm|1110|Rn"                     , "ext": "ARMv5TE+"},

    {"inst": "smull RdLo!=XX, RdHi!=XX, Rn!=XX, Rm!=XX"                , "t32": "1111|101|1100|0|Rn|RdLo|RdHi|0000|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smull RdLo!=PC, RdHi!=PC, Rn!=PC, Rm!=PC"                , "a32": "cond|000|0110|0|RdHi|RdLo|Rm|1001|Rn"                   , "ext": "ARMv4+" },
    {"inst": "smullS RdLo!=PC, RdHi!=PC, Rn!=PC, Rm!=PC"               , "a32": "cond|000|0110|1|RdHi|RdLo|Rm|1001|Rn"                   , "ext": "ARMv4+", "apsr": "NZ=W"},

    {"inst": "smulwb Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1001|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smulwb Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1001|0|Rd|0000|Rm|1010|Rn"                     , "ext": "ARMv5TE+"},

    {"inst": "smulwt Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1001|1|Rn|1111|Rd|0001|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smulwt Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|000|1001|0|Rd|0000|Rm|1110|Rn"                     , "ext": "ARMv5TE+"},

    {"inst": "smusd Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|1010|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smusd Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|1000|0|Rd|1111|Rm|0101|Rn"                     , "ext": "ARMv6+"},

    {"inst": "smusdx Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|1010|0|Rn|1111|Rd|0001|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "smusdx Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|1000|0|Rd|1111|Rm|0111|Rn"                     , "ext": "ARMv6+"},

    {"inst": "srs [Rn==SP]{!}, #mode"                                  , "a32": "1111|100|011|W|0|1101|0000|0101|000|mode:5"             , "ext": "ARMv6+"},
    {"inst": "srsda [Rn==SP]{!}, #mode"                                , "a32": "1111|100|001|W|0|1101|0000|0101|000|mode:5"             , "ext": "ARMv6+"},
    {"inst": "srsdb [Rn==SP]{!}, #mode"                                , "a32": "1111|100|101|W|0|1101|0000|0101|000|mode:5"             , "ext": "ARMv6+"},
    {"inst": "srsib [Rn==SP]{!}, #mode"                                , "a32": "1111|100|111|W|0|1101|0000|0101|000|mode:5"             , "ext": "ARMv6+"},

    {"inst": "ssat Rd!=XX, #sat, Rn!=XX"                               , "t32": "1111|001|100|00|Rn|0|000|Rd|00|0|imm:5"                 , "ext": "ARMv6T2+", "imm": "SsatImm(sat)", "apsr": "Q=X", "it": "any"},
    {"inst": "ssat Rd!=PC, #sat, Rn!=PC"                               , "a32": "cond|011|0101|imm:5|Rd|00000|00|1|Rn"                   , "ext": "ARMv6+"  , "imm": "SsatImm(sat)", "apsr": "Q=X"},

    {"inst": "ssat Rd!=XX, #sat, Rn!=XX, lsl_or_asr #n"                , "t32": "1111|001|100|sop:2|Rn|0|n:3|Rd|n:2|0|imm:5"             , "ext": "ARMv6T2+", "imm": "SsatImm(sat, n)", "apsr": "Q=X", "it": "any"},
    {"inst": "ssat Rd!=PC, #sat, Rn!=PC, lsl_or_asr #n"                , "a32": "cond|011|0101|imm:5|Rd|n:5|sop:2|1|Rn"                  , "ext": "ARMv6+"  , "imm": "SsatImm(sat, n)", "apsr": "Q=X"},

    {"inst": "ssat16 Rd!=XX, #sat, Rn!=XX"                             , "t32": "1111|001|1001|0|Rn|0000|Rd|0000|imm:4"                  , "ext": "ARMv6T2+", "imm": "Ssat16Imm(sat)", "apsr": "Q=X", "it": "any"},
    {"inst": "ssat16 Rd!=PC, #sat, Rn!=PC"                             , "a32": "cond|011|0101|0|imm:4|Rd|1111|0011|Rn"                  , "ext": "ARMv6+"  , "imm": "Ssat16Imm(sat)", "apsr": "Q=X"},

    {"inst": "ssax Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0111|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "apsr": "GE=W", "it": "any"},
    {"inst": "ssax Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|0000|1|Rn|Rd|1111|0101|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "ssub16 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0110|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "apsr": "GE=W", "it": "any"},
    {"inst": "ssub16 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0000|1|Rn|Rd|1111|0111|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "ssub8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0110|0|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "apsr": "GE=W", "it": "any"},
    {"inst": "ssub8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0000|1|Rn|Rd|1111|1111|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "stl Rs!=PC, [Rn!=PC]"                                    , "t32": "1110|100|0110|0|Rn|Rs|1111|1010|1111"                   , "ext": "ARMv8+"},
    {"inst": "stl Rs!=PC, [Rn!=PC]"                                    , "a32": "cond|000|1100|0|Rn|1111|1100|1001|Rs"                   , "ext": "ARMv8+"},

    {"inst": "stlb Rs!=PC, [Rn!=PC]"                                   , "t32": "1110|100|0110|0|Rn|Rs|1111|1000|1111"                   , "ext": "ARMv8+"},
    {"inst": "stlb Rs!=PC, [Rn!=PC]"                                   , "a32": "cond|000|1110|0|Rn|1111|1100|1001|Rs"                   , "ext": "ARMv8+"},

    {"inst": "stlex Rd!=PC, Rs!=PC, [Rn!=PC]"                          , "t32": "1110|100|0110|0|Rn|Rs|1111|1110|Rd"                     , "ext": "ARMv8+"},
    {"inst": "stlex Rd!=PC, Rs!=PC, [Rn!=PC]"                          , "a32": "cond|000|1100|0|Rn|Rd|1110|1001|Rs"                     , "ext": "ARMv8+"},

    {"inst": "stlexb Rd!=PC, Rs!=PC, [Rn!=PC]"                         , "t32": "1110|100|0110|0|Rn|Rs|1111|1100|Rd"                     , "ext": "ARMv8+"},
    {"inst": "stlexb Rd!=PC, Rs!=PC, [Rn!=PC]"                         , "a32": "cond|000|1110|0|Rn|Rd|1110|1001|Rs"                     , "ext": "ARMv8+"},

    {"inst": "stlexd Rd!=PC, Rs!=PC, Rs2!=PC, [Rn!=PC]"                , "t32": "1110|100|0110|0|Rn|Rs|Rs2|1111|Rd"                      , "ext": "ARMv8+"},
    {"inst": "stlexd Rd!=PC, Rs!=PC, Rs2+, [Rn!=PC]"                   , "a32": "cond|000|1101|0|Rn|Rd|1110|1001|Rs"                     , "ext": "ARMv8+"},

    {"inst": "stlexh Rd!=PC, Rs!=PC, [Rn!=PC]"                         , "t32": "1110|100|0110|0|Rn|Rs|1111|1101|Rd"                     , "ext": "ARMv8+"},
    {"inst": "stlexh Rd!=PC, Rs!=PC, [Rn!=PC]"                         , "a32": "cond|000|1111|0|Rn|Rd|1110|1001|Rs"                     , "ext": "ARMv8+"},

    {"inst": "stlh Rs!=PC, [Rn!=PC]"                                   , "t32": "1110|100|0110|0|Rn|Rs|1111|1001|1111"                   , "ext": "ARMv8+"},
    {"inst": "stlh Rs!=PC, [Rn!=PC]"                                   , "a32": "cond|000|1111|0|Rn|1111|1100|1001|Rs"                   , "ext": "ARMv8+"},

    {"inst": "stm [Rn!=HI]!, RsList"                                   , "t16": "1100|0|Rn:3|RsList:8"                                   , "ext": "ARMv4T+", "it": "any"},
    {"inst": "stm [Rn!=PC]{!}, RsList"                                 , "t32": "1110|100|010W|0|Rn|0|RsList[14]|0|RsList[12:0]"         , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "stm [Rn!=PC]{!}, RsList"                                 , "a32": "cond|100|010W|0|Rn|RsList:16"                           , "ext": "ARMv4+"},
    {"inst": "stm [Rn!=PC], RsList"                                    , "a32": "cond|100|0110|0|Rn|RsList:16"                           , "ext": "ARMv4+"},

    {"inst": "stmda [Rn!=PC]{!}, RsList"                               , "a32": "cond|100|000W|0|Rn|RsList:16"                           , "ext": "ARMv4+"},
    {"inst": "stmda [Rn!=PC], RsList"                                  , "a32": "cond|100|0010|0|Rn|RsList:16"                           , "ext": "ARMv4+"},

    {"inst": "stmdb [Rn!=PC]{!}, RsList"                               , "t32": "1110|100|100W|0|Rn|0|RsList[14]|0|RsList[12:0]"         , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "stmdb [Rn!=PC]{!}, RsList"                               , "a32": "cond|100|100W|0|Rn|RsList:16"                           , "ext": "ARMv4+"},
    {"inst": "stmdb [Rn!=PC], RsList"                                  , "a32": "cond|100|1010|0|Rn|RsList:16"                           , "ext": "ARMv4+"},

    {"inst": "stmib [Rn!=PC]{!}, RsList"                               , "a32": "cond|100|110W|0|Rn|RsList:16"                           , "ext": "ARMv4+"},
    {"inst": "stmib [Rn!=PC], RsList"                                  , "a32": "cond|100|1110|0|Rn|RsList:16"                           , "ext": "ARMv4+"},

    {"inst": "str Rs!=HI, [Rn!=HI, Rm!=HI]"                            , "t16": "0101|000|Rm:3|Rn:3|Rs:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "str Rs!=PC, [Rn!=PC, Rm!=XX, {lsl #n}]"                  , "t32": "1111|100|0010|0|Rn|Rs|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "str Rs!=HI, [Rn==SP, #off*4]"                            , "t16": "1001|0|Rs:3|off:8"                                      , "ext": "ARMv4T+", "it": "any"},
    {"inst": "str Rs!=HI, [Rn!=HI, #off*4]"                            , "t16": "0110|0|off:5|Rn:3|Rs:3"                                 , "ext": "ARMv4T+", "it": "any"},
    {"inst": "str Rs!=PC, [Rn!=PC, #off]"                              , "t32": "1111|100|0110|0|Rn|Rs|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "str Rs!=PC, [Rn!=PC, #+/-off]{@}{!}"                     , "t32": "1111|100|0010|0|Rn|Rs|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "str Rs, [Rn, +/-Rm!=PC, {sop #n}]{@}{!}"                 , "a32": "cond|011|PU0W|0|Rn|Rs|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "str Rs, [Rn, #+/-off]{@}{!}"                             , "a32": "cond|010|PU0W|0|Rn|Rs|off:12"                           , "ext": "ARMv4+"},

    {"inst": "strb Rs!=HI, [Rn!=HI, Rm!=HI]"                           , "t16": "0101|010|Rm:3|Rn:3|Rs:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "strb Rs!=XX, [Rn!=PC, Rm!=XX, {lsl #n}]"                 , "t32": "1111|100|0000|0|Rn|Rs|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strb Rs!=HI, [Rn!=HI, #off*4]"                           , "t16": "0111|0|off:5|Rn:3|Rs:3"                                 , "ext": "ARMv4T+", "it": "any"},
    {"inst": "strb Rs!=XX, [Rn!=PC, #off]"                             , "t32": "1111|100|0100|0|Rn|Rs|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strb Rs!=XX, [Rn!=PC, #+/-off]{@}{!}"                    , "t32": "1111|100|0000|0|Rn|Rs|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strb Rs!=PC, [Rn, +/-Rm!=PC, {sop #n}]{@}{!}"            , "a32": "cond|011|PU1W|0|Rn|Rs|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "strb Rs!=PC, [Rn, #+/-off]{@}{!}"                        , "a32": "cond|010|PU1W|0|Rn|Rs|off:12"                           , "ext": "ARMv4+"},

    {"inst": "strbt Rs!=XX, [Rn!=PC, #off]"                            , "t32": "1111|100|0000|0|Rn|Rs|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strbt Rs!=PC, [Rn!=PC, +/-Rm!=PC, {sop #n}]@"            , "a32": "cond|011|0U11|0|Rn|Rs|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "strbt Rs!=PC, [Rn, #+/-off]@"                            , "a32": "cond|010|0U11|0|Rn|Rs|off:12"                           , "ext": "ARMv4+"},

    {"inst": "strd Rs!=XX, Rs2!=XX, [Rn!=PC, #off]{@}{!}"              , "t32": "1110|100|PU1W|0|Rn|Rs|Rs2|off:8"                        , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strd Rs<=13, Rs2+, [Rn, +/-Rm!=PC]{@}{!}"                , "a32": "cond|000|PU0W|0|Rn|Rs|0000|1111|Rm"                     , "ext": "ARMv4+"},
    {"inst": "strd Rs<=13, Rs2+, [Rn, #+/-off]{@}{!}"                  , "a32": "cond|000|PU1W|0|Rn|Rs|off:4|1111|off:4"                 , "ext": "ARMv4+"},

    {"inst": "strex Rd!=XX, Rs!=XX, [Rn!=PC, #off]"                    , "t32": "1110|100|0010|0|Rn|Rs|Rd|off:8"                         , "ext": "ARMv6T2+", "it": "any", "mod": "UNPRED_COMPLEX"},
    {"inst": "strex Rd!=PC, Rs!=PC, [Rn!=PC]"                          , "a32": "cond|000|1100|0|Rn|Rd|1111|1001|Rs"                     , "ext": "ARMv6+"},

    {"inst": "strexb Rd!=XX, Rs!=XX, [Rn!=PC]"                         , "t32": "1110|100|0110|0|Rn|Rs|1111|0100|Rd"                     , "ext": "ARMv6T2+", "it": "any", "mod": "UNPRED_COMPLEX"},
    {"inst": "strexb Rd!=PC, Rs!=PC, [Rn!=PC]"                         , "a32": "cond|000|1110|0|Rn|Rd|1111|1001|Rs"                     , "ext": "ARMv6K+"},

    {"inst": "strexd Rd!=XX, Rs!=XX, Rs2!=XX, [Rn!=PC]"                , "t32": "1110|100|0110|0|Rn|Rs|Rs2|0111|Rd"                      , "ext": "ARMv6T2+", "it": "any", "mod": "UNPRED_COMPLEX"},
    {"inst": "strexd Rd!=PC, Rs!=PC, Rs2+, [Rn!=PC]"                   , "a32": "cond|000|1101|0|Rn|Rd|1111|1001|Rs"                     , "ext": "ARMv6K+"},

    {"inst": "strexh Rd!=XX, Rs!=XX, [Rn!=PC]"                         , "t32": "1110|100|0110|0|Rn|Rs|1111|0101|Rd"                     , "ext": "ARMv6T2+", "it": "any", "mod": "UNPRED_COMPLEX"},
    {"inst": "strexh Rd!=PC, Rs!=PC, [Rn!=PC]"                         , "a32": "cond|000|1111|0|Rn|Rd|1111|1001|Rs"                     , "ext": "ARMv6K+"},

    {"inst": "strh Rs!=HI, [Rn!=HI, Rm!=HI]"                           , "t16": "0101|001|Rm:3|Rn:3|Rs:3"                                , "ext": "ARMv4T+", "it": "any"},
    {"inst": "strh Rs!=XX, [Rn!=PC, Rm!=XX, {lsl #n}]"                 , "t32": "1111|100|0001|0|Rn|Rs|0|00000|n:2|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strh Rs!=HI, [Rn!=HI, #off*4]"                           , "t16": "1000|0|off:5|Rn:3|Rs:3"                                 , "ext": "ARMv4T+", "it": "any"},
    {"inst": "strh Rs!=XX, [Rn!=PC, #off]"                             , "t32": "1111|100|0101|0|Rn|Rs|off:12"                           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strh Rs!=XX, [Rn!=PC, #+/-off]{@}{!}"                    , "t32": "1111|100|0001|0|Rn|Rs|1PUW|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strh Rs!=PC, [Rn, +/-Rm!=PC]{@}{!}"                      , "a32": "cond|000|PU0W|0|Rn|Rs|0000|1011|Rm"                     , "ext": "ARMv4+"},
    {"inst": "strh Rs!=PC, [Rn, #+/-off]{@}{!}"                        , "a32": "cond|000|PU1W|0|Rn|Rs|off:4|1011|off:4"                 , "ext": "ARMv4+"},

    {"inst": "strht Rs!=XX, [Rn!=PC, #off]"                            , "t32": "1111|100|0001|0|Rn|Rs|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strht Rs!=PC, [Rn!=PC, +/-Rm!=PC]@"                      , "a32": "cond|000|0U01|0|Rn|Rs|0000|1011|Rm"                     , "ext": "ARMv6T2+"},
    {"inst": "strht Rs!=PC, [Rn!=PC, #+/-off]@"                        , "a32": "cond|000|0U11|0|Rn|Rs|off:4|1011|off:4"                 , "ext": "ARMv6T2+"},

    {"inst": "strt Rs!=XX, [Rn!=PC, #off]"                             , "t32": "1111|100|0010|0|Rn|Rs|1110|off:8"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "strt Rs, [Rn!=PC, +/-Rm!=PC, {sop #n}]@"                 , "a32": "cond|011|0U01|0|Rn|Rs|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "strt Rs, [Rn!=PC, #+/-off]@"                             , "a32": "cond|010|0U01|0|Rn|Rs|off:12"                           , "ext": "ARMv4+"},

    {"inst": "sub Rd!=HI, Rn!=HI, Rm!=HI"                              , "t16": "0001|101|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "in"},
    {"inst": "sub Rd!=HI, Rn!=HI, #immZ"                               , "t16": "0001|111|immZ:3|Rn:3|Rd:3"                              , "ext": "ARMv4T+", "it": "in"},
    {"inst": "sub Rx!=HI, Rx!=HI, #immZ"                               , "t16": "0011|1|Rx:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "in"},
    {"inst": "sub Rx==SP, Rx==SP, #immZ*4"                             , "t16": "1011|00001|immZ:7"                                      , "ext": "ARMv4T+", "it": "any"},
    {"inst": "sub Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1101|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sub Rd!=PC, Rn==SP, Rm!=XX, {sop #n}"                    , "t32": "1110|101|1101|0|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sub Rd!=XX, Rn!=XX, #immA"                               , "t32": "1111|0|imm:1|0|1101|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sub Rd!=PC, Rn==SP, #immA"                               , "t32": "1111|0|imm:1|0|1101|0|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sub Rd!=XX, Rn!=XX, #immZ"                               , "t32": "1111|0|immZ:1|1|0101|0|Rn|0|immZ:3|Rd|immZ:8"           , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "sub Rd, Rn!=PC, Rm!=PC, sop Rs!=PC"                      , "a32": "cond|000|0010|0|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+"},
    {"inst": "sub Rd, Rn, Rm, {sop #n}"                                , "a32": "cond|000|0010|0|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+"},
    {"inst": "sub Rd, Rn, #immA"                                       , "a32": "cond|001|0010|0|Rn|Rd|imm:12"                           , "ext": "ARMv4+"},

    {"inst": "sub Rd!=HI, Rn!=HI, Rm!=HI"                              , "t16": "0001|101|Rm:3|Rn:3|Rd:3"                                , "ext": "ARMv4T+", "it": "in"},
    {"inst": "subS Rd!=HI, Rn!=HI, #immZ"                              , "t16": "0001|111|immZ:3|Rn:3|Rd:3"                              , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "subS Rx!=HI, Rx!=HI, #immZ"                              , "t16": "0001|1|Rx:3|immZ:8"                                     , "ext": "ARMv4T+", "it": "out", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=XX, Rn!=XX, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1101|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=PC, Rn==SP, Rm!=XX, {sop #n}"                   , "t32": "1110|101|1101|1|Rn|0|n:3|Rd|n:2|sop:2|Rm"               , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=XX, Rn!=XX, #immA"                              , "t32": "1111|0|imm:1|0|1101|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=PC, Rn==SP, #immA"                              , "t32": "1111|0|imm:1|0|1101|1|Rn|0|imm:3|Rd|imm:8"              , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=XX, Rn!=XX, #immZ"                              , "t32": "1111|0|immZ:1|1|0101|1|Rn|0|immZ:3|Rd|immZ:8"           , "ext": "ARMv6T2+", "it": "any", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=PC, Rn!=PC, Rm!=PC, sop Rs!=PC"                 , "a32": "cond|000|0010|1|Rn|Rd|Rs|0|sop:2|1|Rm"                  , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=PC, Rn, Rm, {sop #n}"                           , "a32": "cond|000|0010|1|Rn|Rd|n:5|sop:2|0|Rm"                   , "ext": "ARMv4+", "apsr": "NZCV=W"},
    {"inst": "subS Rd!=PC, Rn, #immA"                                  , "a32": "cond|001|0010|1|Rn|Rd|imm:12"                           , "ext": "ARMv4+", "apsr": "NZCV=W"},

    {"inst": "svc #immZ"                                               , "t16": "1101|1111|immZ:8"                                       , "ext": "ARMv4T+", "it": "any"},
    {"inst": "svc #immZ"                                               , "a32": "cond|1111|immZ:24"                                      , "ext": "ARMv4+"},

    {"inst": "sxtab Rd!=XX, Rn!=XX, Rm!=XX, {ror #n}"                  , "t32": "1111|101|0010|0|Rn|1111|Rd|1|0|imm:2|Rm"                , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "sxtab Rd!=PC, Rn!=PC, Rm!=PC, {ror #n}"                  , "a32": "cond|011|0101|0|Rn|Rd|imm:2|00|0111|Rm"                 , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "sxtab16 Rd!=XX, Rn!=XX, Rm!=XX, {ror #n}"                , "t32": "1111|101|0001|0|Rn|1111|Rd|1|0|imm:2|Rm"                , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "sxtab16 Rd!=PC, Rn!=PC, Rm!=PC, {ror #n}"                , "a32": "cond|011|0100|0|Rn|Rd|imm:2|00|0111|Rm"                 , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "sxtah Rd!=XX, Rn!=XX, Rm!=XX, {ror #n}"                  , "t32": "1111|101|0000|0|Rn|1111|Rd|1|0|imm:2|Rm"                , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "sxtah Rd!=PC, Rn!=PC, Rm!=PC, {ror #n}"                  , "a32": "cond|011|0101|1|Rn|Rd|imm:2|00|0111|Rm"                 , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "sxtb Rd!=HI, Rn!=HI, {ror #zero}"                        , "t16": "1011|001|001|Rn:3|Rd:3"                                 , "ext": "ARMv6+"  , "it": "any"},
    {"inst": "sxtb Rd!=XX, Rn!=XX, {ror #n}"                           , "t32": "1111|101|0010|0|1111|1111|Rd|10|imm:2|Rn"               , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "sxtb Rd!=PC, Rn!=PC, {ror #n}"                           , "a32": "cond|011|0101|0|1111|Rd|imm:2|00|0111|Rn"               , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "sxtb16 Rd!=XX, Rn!=XX, {ror #n}"                         , "t32": "1111|101|0001|0|1111|1111|Rd|10|imm:2|Rn"               , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "sxtb16 Rd!=PC, Rn!=PC, {ror #n}"                         , "a32": "cond|011|0100|0|1111|Rd|imm:2|00|0111|Rn"               , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "sxth Rd!=HI, Rn!=HI, {ror #zero}"                        , "t16": "1011|001|000|Rn:3|Rd:3"                                 , "ext": "ARMv6+"  , "it": "any"},
    {"inst": "sxth Rd!=XX, Rn!=XX, {ror #n}"                           , "t32": "1111|101|0000|0|1111|1111|Rd|10|imm:2|Rn"               , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "sxth Rd!=PC, Rn!=PC, {ror #n}"                           , "a32": "cond|011|0101|1|1111|Rd|imm:2|00|0111|Rn"               , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "tbb Rn!=SP, Rn!=XX"                                      , "t32": "1110|100|0110|1|Rn|1111|0000|0000|Rn"                   , "ext": "ARMv6T2+", "it": "out|last"},
    {"inst": "tbh Rn!=SP, Rn!=XX"                                      , "t32": "1110|100|0110|1|Rn|1111|0000|0001|Rn"                   , "ext": "ARMv6T2+", "it": "out|last"},

    {"inst": "teq Rn!=XX, Rn!=XX, {sop #n}"                            , "t32": "1110|101|0100|1|Rn|0|n:3|1111|n:2|sop:2|Rn"             , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "teq Rn!=XX, #immA"                                       , "t32": "1111|0|imm:1|0|0100|1|Rn|0|imm:3|1111|imm:8"            , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "teq Rn!=PC, Rm!=PC, sop Rs!=PC"                          , "a32": "cond|000|1001|1|Rn|0000|Rs|0|sop:2|1|Rm"                , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "teq Rn, Rm, {sop #n}"                                    , "a32": "cond|000|1001|1|Rn|0000|n:5|sop:2|0|Rm"                 , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "teq Rn, #immA"                                           , "a32": "cond|001|1001|1|Rn|0000|imm:12"                         , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "tst Rn!=HI, Rn!=HI"                                      , "t16": "0100|00|1000|Rn:3|Rn:3"                                 , "ext": "ARMv4T+", "it": "any", "apsr": "NZC=W"},
    {"inst": "tst Rn!=XX, Rn!=XX, {sop #n}"                            , "t32": "1110|101|0000|1|Rn|0|n:3|1111|n:2|sop:2|Rn"             , "ext": "ARMv6T2+", "it": "any", "apsr": "NZC=W"},
    {"inst": "tst Rn!=XX, #immA"                                       , "t32": "1111|0|imm:1|0|0000|1|Rn|0|imm:3|1111|imm:8"            , "ext": "ARMv4T+", "it": "any", "apsr": "NZC=W"},
    {"inst": "tst Rn!=PC, Rm!=PC, sop Rs!=PC"                          , "a32": "cond|000|1000|1|Rn|0000|Rs|0|sop:2|1|Rm"                , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "tst Rn, Rm, {sop #n}"                                    , "a32": "cond|000|1000|1|Rn|0000|n:5|sop:2|0|Rm"                 , "ext": "ARMv4+", "apsr": "NZC=W"},
    {"inst": "tst Rn, #immA"                                           , "a32": "cond|001|1000|1|Rn|0000|imm:12"                         , "ext": "ARMv4+", "apsr": "NZC=W"},

    {"inst": "uadd16 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0100|1|Rn|1111|Rd|0100|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "uadd16 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0010|1|Rn|Rd|1111|0001|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "uadd8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0100|0|Rn|1111|Rd|0100|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "uadd8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0010|1|Rn|Rd|1111|1001|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "uasx Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0101|0|Rn|1111|Rd|0100|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "uasx Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|0010|1|Rn|Rd|1111|0011|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "ubfx Rd!=XX, Rn!=XX, #lsb, #width"                       , "t32": "1111|001|1110|0|Rn|0|lsb:3|Rd|lsb:2|0|widthM1:5"        , "ext": "ARMv6T2+", "imm": "SbfxUbfxImm(lsb, width)", "it": "any"},
    {"inst": "ubfx Rd!=PC, Rn!=PC, #lsb, #width"                       , "a32": "cond|011|1111|widthM1:5|Rd|lsb:5|101|Rn"                , "ext": "ARMv6T2+", "imm": "SbfxUbfxImm(lsb, width)"},

    {"inst": "udiv Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|1101|1|Rn|1111|Rd|1111|Rm"                     , "ext": "IDIVT", "it": "any"},
    {"inst": "udiv Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|1001|1|Rd|1111|Rm|0001|Rn"                     , "ext": "IDIVA"},

    {"inst": "udf #immZ"                                               , "a32": "1110|011|1111|1|immZ:12|1111|immZ:4"                    , "?": "?"},

    {"inst": "uhadd8 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0100|0|Rn|1111|Rd|0110|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uhadd8 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0011|1|Rn|Rd|1111|1001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uhadd16 Rd!=XX, Rn!=XX, Rm!=XX"                          , "t32": "1111|101|0100|1|Rn|1111|Rd|0110|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uhadd16 Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|011|0011|1|Rn|Rd|1111|0001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uhasx Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0101|0|Rn|1111|Rd|0110|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uhasx Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0011|1|Rn|Rd|1111|0011|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uhsax Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0111|0|Rn|1111|Rd|0110|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uhsax Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0011|1|Rn|Rd|1111|0101|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uhsub16 Rd!=XX, Rn!=XX, Rm!=XX"                          , "t32": "1111|101|0110|1|Rn|1111|Rd|0110|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uhsub16 Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|011|0011|1|Rn|Rd|1111|0111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uhsub8 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0110|0|Rn|1111|Rd|0110|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uhsub8 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0011|1|Rn|Rd|1111|1111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "umaal RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"                , "t32": "1111|101|1111|0|Rn|RxLo|RxHi|0110|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "umaal RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"                , "a32": "cond|000|0010|0|RxHi|RxLo|Rm|1001|Rn"                   , "ext": "ARMv6+"},

    {"inst": "umlal RxLo!=XX, RxHi!=XX, Rn!=XX, Rm!=XX"                , "t32": "1111|101|1111|0|Rn|RxLo|RxHi|0000|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "umlal RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"                , "a32": "cond|000|0101|0|RxHi|RxLo|Rm|1001|Rn"                   , "ext": "ARMv4+"},
    {"inst": "umlalS RxLo!=PC, RxHi!=PC, Rn!=PC, Rm!=PC"               , "a32": "cond|000|0101|1|RxHi|RxLo|Rm|1001|Rn"                   , "ext": "ARMv4+", "apsr": "NZ=W"},

    {"inst": "umull RdLo!=XX, RdHi!=XX, Rn!=XX, Rm!=XX"                , "t32": "1111|101|1101|0|Rn|RdLo|RdHi|0000|Rm"                   , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "umull RdLo!=PC, RdHi!=PC, Rn!=PC, Rm!=PC"                , "a32": "cond|000|0100|0|RdHi|RdLo|Rm|1001|Rn"                   , "ext": "ARMv4+"},
    {"inst": "umullS RdLo!=PC, RdHi!=PC, Rn!=PC, Rm!=PC"               , "a32": "cond|000|0100|1|RdHi|RdLo|Rm|1001|Rn"                   , "ext": "ARMv4+", "apsr": "NZ=W"},

    {"inst": "uqadd16 Rd!=XX, Rn!=XX, Rm!=XX"                          , "t32": "1111|101|0100|1|Rn|1111|Rd|0101|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uqadd16 Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|011|0011|0|Rn|Rd|1111|0001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uqadd8 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0100|0|Rn|1111|Rd|0101|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uqadd8 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0011|0|Rn|Rd|1111|1001|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uqasx Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0101|0|Rn|1111|Rd|0101|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uqasx Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0011|0|Rn|Rd|1111|0011|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uqsax Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0111|0|Rn|1111|Rd|0101|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uqsax Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0011|0|Rn|Rd|1111|0101|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uqsub16 Rd!=XX, Rn!=XX, Rm!=XX"                          , "t32": "1111|101|0110|1|Rn|1111|Rd|0101|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uqsub16 Rd!=PC, Rn!=PC, Rm!=PC"                          , "a32": "cond|011|0011|0|Rn|Rd|1111|0111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "uqsub8 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0110|0|Rn|1111|Rd|0101|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uqsub8 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0011|0|Rn|Rd|1111|1111|Rm"                     , "ext": "ARMv6+"},

    {"inst": "usad8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|1011|1|Rn|1111|Rd|0000|Rm"                     , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "usad8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|1100|0|Rd|1111|Rm|0001|Rn"                     , "ext": "ARMv6+"},

    {"inst": "usada8 Rd!=XX, Rn!=XX, Rm!=XX, Ra!=XX"                   , "t32": "1111|101|1011|1|Rn|Ra|Rd|0000|Rm"                       , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "usada8 Rd!=PC, Rn!=PC, Rm!=PC, Ra!=PC"                   , "a32": "cond|011|1100|0|Rd|Ra|Rm|0001|Rn"                       , "ext": "ARMv6+"},

    {"inst": "usat Rd!=XX, #sat, Rn!=XX, {lsl_or_asr #n}"              , "t32": "1111|001|110|sop:2|Rn|0|n:3|Rd|n:2|0|sat:5"             , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "usat Rd!=PC, #sat, Rn!=PC, {lsl_or_asr #n}"              , "a32": "cond|011|0111|sat:5|Rd|n:5|sop:2|1|Rn"                  , "ext": "ARMv6+"},

    {"inst": "usat16 Rd!=XX, #sat, Rn!=XX"                             , "t32": "1111|001|1101|0|Rn|0000|Rd|0000|sat:4"                  , "ext": "ARMv6T2+", "it": "any", "apsr": "Q=X"},
    {"inst": "usat16 Rd!=PC, #sat, Rn!=PC"                             , "a32": "cond|011|0111|0|sat:4|Rd|1111|0011|Rn"                  , "ext": "ARMv6+"},

    {"inst": "usax Rd!=XX, Rn!=XX, Rm!=XX"                             , "t32": "1111|101|0111|0|Rn|1111|Rd|0100|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "usax Rd!=PC, Rn!=PC, Rm!=PC"                             , "a32": "cond|011|0010|1|Rn|Rd|1111|0101|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "usub16 Rd!=XX, Rn!=XX, Rm!=XX"                           , "t32": "1111|101|0110|1|Rn|1111|Rd|0100|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "usub16 Rd!=PC, Rn!=PC, Rm!=PC"                           , "a32": "cond|011|0010|1|Rn|Rd|1111|0111|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "usub8 Rd!=XX, Rn!=XX, Rm!=XX"                            , "t32": "1111|101|0110|0|Rn|1111|Rd|0100|Rm"                     , "ext": "ARMv6T2+", "it": "any", "apsr": "GE=W"},
    {"inst": "usub8 Rd!=PC, Rn!=PC, Rm!=PC"                            , "a32": "cond|011|0010|1|Rn|Rd|1111|1111|Rm"                     , "ext": "ARMv6+", "apsr": "GE=W"},

    {"inst": "uxtab Rd!=XX, Rn!=XX, Rm!=XX, {ror #n}"                  , "t32": "1111|101|0010|1|Rn|1111|Rd|10|imm:2|Rm"                 , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "uxtab Rd!=PC, Rn!=PC, Rm!=PC, {ror #n}"                  , "a32": "cond|011|0111|0|Rn|Rd|imm:2|00|0111|Rm"                 , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "uxtab16 Rd!=XX, Rn!=XX, Rm!=XX, {ror #n}"                , "t32": "1111|101|0001|1|Rn|1111|Rd|10|imm:2|Rm"                 , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "uxtab16 Rd!=PC, Rn!=PC, Rm!=PC, {ror #n}"                , "a32": "cond|011|0110|0|Rn|Rd|imm:2|00|0111|Rm"                 , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "uxtah Rd!=XX, Rn!=XX, Rm!=XX, {ror #n}"                  , "t32": "1111|101|0000|1|Rn|1111|Rd|10|imm:2|Rm"                 , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "uxtah Rd!=PC, Rn!=PC, Rm!=PC, {ror #n}"                  , "a32": "cond|011|0111|1|Rn|Rd|imm:2|00|0111|Rm"                 , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "uxtb Rd!=HI, Rn!=HI, {ror #zero}"                        , "t16": "1011|001|011|Rn:3|Rd:3"                                 , "ext": "ARMv6+"  , "it": "any"},
    {"inst": "uxtb Rd!=XX, Rn!=XX, {ror #n}"                           , "t32": "1111|101|0010|1|1111|1111|Rd|10|imm:2|Rn"               , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "uxtb Rd!=PC, Rn!=PC, {ror #n}"                           , "a32": "cond|011|0111|0|1111|Rd|imm:2|00|0111|Rn"               , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "uxtb16 Rd!=XX, Rn!=XX, {ror #n}"                         , "t32": "1111|101|0001|1|1111|1111|Rd|10|imm:2|Rn"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "uxtb16 Rd!=PC, Rn!=PC, {ror #n}"                         , "a32": "cond|011|0110|0|1111|Rd|imm:2|00|0111|Rn"               , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "uxth Rd!=HI, Rn!=HI, {ror #zero}"                        , "t16": "1011|001|010|Rn:3|Rd:3"                                 , "ext": "ARMv6+"  , "it": "any"},
    {"inst": "uxth Rd!=XX, Rn!=XX, {ror #n}"                           , "t32": "1111|101|0000|1|1111|1111|Rd|10|imm:2|Rn"               , "ext": "ARMv6T2+", "imm": "Ror8Imm(n)", "it": "any"},
    {"inst": "uxth Rd!=PC, Rn!=PC, {ror #n}"                           , "a32": "cond|011|0111|1|1111|Rd|imm:2|00|0111|Rn"               , "ext": "ARMv6+"  , "imm": "Ror8Imm(n)"},

    {"inst": "vaba.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|111U0|Vd'|sz|Vn|Vd|0111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vaba.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|111U0|Vd'|sz|Vn|Vd|0111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vaba.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vaba.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vabal.x8-32 Vd, Dn, Dm"                                  , "t32": "111U|11111|Vd'|sz|Vn|Vd|0101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vabal.x8-32 Vd, Dn, Dm"                                  , "a32": "1111|001U1|Vd'|sz|Vn|Vd|0101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vabd.f16-32 Dd, Dn, Dm"                                  , "t32": "1111|11110|Vd'|1H|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vabd.f16-32 Vd, Vn, Vm"                                  , "t32": "1111|11110|Vd'|1H|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vabd.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00110|Vd'|1H|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vabd.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00110|Vd'|1H|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vabd.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vabd.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0111|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vabd.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vabd.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0111|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vabdl.x8-32 Vd, Dn, Dm"                                  , "t32": "111U|11111|Vd'|sz|Vn|Vd|0111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vabdl.x8-32 Vd, Dn, Dm"                                  , "a32": "1111|001U1|Vd'|sz|Vn|Vd|0111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vabs.f16-32 Sd, Sn"                                      , "t32": "1110|11101|'Vd|11|00|00|Vd|10|sz|1|1|'Vn|0|Vn"          , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vabs.f16-32 Sd, Sn"                                      , "a32": "cond|11101|'Vd|11|00|00|Vd|10|sz|1|1|'Vn|0|Vn"          , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vabs.f64 Dd, Dn"                                         , "t32": "1110|11101|Vd'|11|00|00|Vd|10|11|1|1|Vn'|0|Vn"          , "ext": "VFPv2"},
    {"inst": "vabs.f64 Dd, Dn"                                         , "a32": "cond|11101|Vd'|11|00|00|Vd|10|11|1|1|Vn'|0|Vn"          , "ext": "VFPv2"},

    {"inst": "vabs.f16-32 Dd, Dn"                                      , "t32": "1111|11111|Vd'|11|sz|01|Vd|01|11|0|0|Vn'|0|Vn"          , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vabs.f16-32 Vd, Vn"                                      , "t32": "1111|11111|Vd'|11|sz|01|Vd|01|11|0|1|Vn'|0|Vn"          , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vabs.f16-32 Dd, Dn"                                      , "a32": "1111|00111|Vd'|11|sz|01|Vd|01|11|0|0|Vn'|0|Vn"          , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vabs.f16-32 Vd, Vn"                                      , "a32": "1111|00111|Vd'|11|sz|01|Vd|01|11|0|1|Vn'|0|Vn"          , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vabs.s8-32 Dd, Dn"                                       , "t32": "1111|11111|Vd'|11|sz|01|Vd|00|11|0|0|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vabs.s8-32 Vd, Vn"                                       , "t32": "1111|11111|Vd'|11|sz|01|Vd|00|11|0|1|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vabs.s8-32 Dd, Dn"                                       , "a32": "1111|00111|Vd'|11|sz|01|Vd|00|11|0|0|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vabs.s8-32 Vd, Vn"                                       , "a32": "1111|00111|Vd'|11|sz|01|Vd|00|11|0|1|Vn'|0|Vn"          , "ext": "ASIMD"},

    {"inst": "vacge.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|0H|Vn|Vd|1110|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacge.f16-32 Vd, Vn, Vm"                                 , "t32": "1111|11110|Vd'|0H|Vn|Vd|1110|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacge.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|0H|Vn|Vd|1110|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacge.f16-32 Vd, Vn, Vm"                                 , "a32": "1111|00110|Vd'|0H|Vn|Vd|1110|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vacgt.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|1H|Vn|Vd|1110|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacgt.f16-32 Vd, Vn, Vm"                                 , "t32": "1111|11110|Vd'|1H|Vn|Vd|1110|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacgt.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|1H|Vn|Vd|1110|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacgt.f16-32 Vd, Vn, Vm"                                 , "a32": "1111|00110|Vd'|1H|Vn|Vd|1110|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vacle.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|0H|Vm|Vd|1110|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacle.f16-32 Vd, Vn, Vm"                                 , "t32": "1111|11110|Vd'|0H|Vm|Vd|1110|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacle.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|0H|Vm|Vd|1110|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vacle.f16-32 Vd, Vn, Vm"                                 , "a32": "1111|00110|Vd'|0H|Vm|Vd|1110|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vaclt.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|1H|Vm|Vd|1110|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vaclt.f16-32 Vd, Vn, Vm"                                 , "t32": "1111|11110|Vd'|1H|Vm|Vd|1110|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vaclt.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|1H|Vm|Vd|1110|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vaclt.f16-32 Vd, Vn, Vm"                                 , "a32": "1111|00110|Vd'|1H|Vm|Vd|1110|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vadd.f16-32 Sd, Sn, Sm"                                  , "t32": "1110|11100|'Vd|11|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vadd.f16-32 Sd, Sn, Sm"                                  , "a32": "cond|11100|'Vd|11|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vadd.f64 Dd, Dn, Dm"                                     , "t32": "1110|11100|Vd'|11|Vn|Vd|1011|Vn'|0|Vm'|0|Vm"            , "ext": "VFPv2"},
    {"inst": "vadd.f64 Dd, Dn, Dm"                                     , "a32": "cond|11100|Vd'|11|Vn|Vd|1011|Vn'|0|Vm'|0|Vm"            , "ext": "VFPv2"},

    {"inst": "vadd.f32 Dd, Dn, Dm"                                     , "t32": "1110|11110|Vd'|00|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vadd.f32 Vd, Vn, Vm"                                     , "t32": "1110|11110|Vd'|00|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vadd.f32 Dd, Dn, Dm"                                     , "a32": "1111|00100|Vd'|00|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vadd.f32 Vd, Vn, Vm"                                     , "a32": "1111|00100|Vd'|00|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vadd.x8-64 Dd, Dn, Dm"                                   , "t32": "1110|11110|Vd'|sz|Vn|Vd|1000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vadd.x8-64 Vd, Vn, Vm"                                   , "t32": "1110|11110|Vd'|sz|Vn|Vd|1000|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vadd.x8-64 Dd, Dn, Dm"                                   , "a32": "1111|00100|Vd'|sz|Vn|Vd|1000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vadd.x8-64 Vd, Vn, Vm"                                   , "a32": "1111|00100|Vd'|sz|Vn|Vd|1000|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vaddhn.x16-64 Dd, Vn, Vm"                                , "t32": "1110|11111|Vd'|szM1:2|Vn|Vd|0100|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},
    {"inst": "vaddhn.x16-64 Dd, Vn, Vm"                                , "a32": "1111|00101|Vd'|szM1:2|Vn|Vd|0100|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},

    {"inst": "vaddl.x8-32 Vd, Dn, Dm"                                  , "t32": "111U|11111|Vd'|sz|Vn|Vd|0000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vaddl.x8-32 Vd, Dn, Dm"                                  , "a32": "1111|001U1|Vd'|sz|Vn|Vd|0000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vaddw.x8-32 Vd, Vn, Dm"                                  , "t32": "111U|11111|Vd'|sz|Vn|Vd|0001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vaddw.x8-32 Vd, Vn, Dm"                                  , "a32": "1111|001U1|Vd'|sz|Vn|Vd|0001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vand.any Dd, Dn, Dm"                                     , "t32": "1110|11110|Vd'|00|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vand.any Vd, Vn, Vm"                                     , "t32": "1110|11110|Vd'|00|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vand.any Dd, Dn, Dm"                                     , "a32": "1111|00100|Vd'|00|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vand.any Vd, Vn, Vm"                                     , "a32": "1111|00100|Vd'|00|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vand.x16-64 Dx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0011|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vbic"},
    {"inst": "vand.x16-64 Vx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0111|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vbic"},
    {"inst": "vand.x16-64 Dx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0011|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vbic"},
    {"inst": "vand.x16-64 Vx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0111|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vbic"},

    {"inst": "vbic.any Dd, Dn, Dm"                                     , "t32": "1110|11110|Vd'|01|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbic.any Vd, Vn, Vm"                                     , "t32": "1110|11110|Vd'|01|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbic.any Dd, Dn, Dm"                                     , "a32": "1111|00100|Vd'|01|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbic.any Vd, Vn, Vm"                                     , "a32": "1111|00100|Vd'|01|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vbic.x16-64 Dx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0011|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},
    {"inst": "vbic.x16-64 Vx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0111|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},
    {"inst": "vbic.x16-64 Dx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0011|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},
    {"inst": "vbic.x16-64 Vx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0111|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},

    {"inst": "vbif.any Dd, Dn, Dm"                                     , "t32": "1111|11110|Vd'|11|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbif.any Vd, Vn, Vm"                                     , "t32": "1111|11110|Vd'|11|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbif.any Dd, Dn, Dm"                                     , "a32": "1111|00110|Vd'|11|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbif.any Vd, Vn, Vm"                                     , "a32": "1111|00110|Vd'|11|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vbit.any Dd, Dn, Dm"                                     , "t32": "1111|11110|Vd'|10|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbit.any Vd, Vn, Vm"                                     , "t32": "1111|11110|Vd'|10|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbit.any Dd, Dn, Dm"                                     , "a32": "1111|00110|Vd'|10|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbit.any Vd, Vn, Vm"                                     , "a32": "1111|00110|Vd'|10|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vbsl.any Dd, Dn, Dm"                                     , "t32": "1111|11110|Vd'|01|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbsl.any Vd, Vn, Vm"                                     , "t32": "1111|11110|Vd'|01|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbsl.any Dd, Dn, Dm"                                     , "a32": "1111|00110|Vd'|01|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vbsl.any Vd, Vn, Vm"                                     , "a32": "1111|00110|Vd'|01|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vcadd.f16-32 Dd, Dn, Dm, #rot1"                          , "t32": "1111|110|imm:1|1|Vd'|0|isFp32|Vn|Vd|1000|Vn'|0|Vm'|0|Vm", "ext": "FCMA ~FP16FULL", "imm": "VecRot1Imm(rot1)"},
    {"inst": "vcadd.f16-32 Vd, Vn, Vm, #rot1"                          , "t32": "1111|110|imm:1|1|Vd'|0|isFp32|Vn|Vd|1000|Vn'|1|Vm'|0|Vm", "ext": "FCMA ~FP16FULL", "imm": "VecRot1Imm(rot1)"},
    {"inst": "vcadd.f16-32 Dd, Dn, Dm, #rot1"                          , "a32": "1111|110|imm:1|1|Vd'|0|isFp32|Vn|Vd|1000|Vn'|0|Vm'|0|Vm", "ext": "FCMA ~FP16FULL", "imm": "VecRot1Imm(rot1)"},
    {"inst": "vcadd.f16-32 Vd, Vn, Vm, #rot1"                          , "a32": "1111|110|imm:1|1|Vd'|0|isFp32|Vn|Vd|1000|Vn'|1|Vm'|0|Vm", "ext": "FCMA ~FP16FULL", "imm": "VecRot1Imm(rot1)"},

    {"inst": "vceq.f16-32 Dd, Dn, Dm"                                  , "t32": "1110|11110|Vd'|0H|Vn|Vd|1110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vceq.f16-32 Vd, Vn, Vm"                                  , "t32": "1110|11110|Vd'|0H|Vn|Vd|1110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vceq.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00100|Vd'|0H|Vn|Vd|1110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vceq.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00100|Vd'|0H|Vn|Vd|1110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vceq.f16-32 Dd, Dn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01010|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vceq.f16-32 Vd, Vn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01010|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vceq.f16-32 Dd, Dn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01010|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vceq.f16-32 Vd, Vn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01010|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vceq.x8-32 Dd, Dn, Dm"                                   , "t32": "1111|11110|Vd'|sz|Vn|Vd|1000|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vceq.x8-32 Vd, Vn, Vm"                                   , "t32": "1111|11110|Vd'|sz|Vn|Vd|1000|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vceq.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|00110|Vd'|sz|Vn|Vd|1000|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vceq.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|00110|Vd'|sz|Vn|Vd|1000|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vceq.x8-32 Dd, Dn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00010|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vceq.x8-32 Vd, Vn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00010|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vceq.x8-32 Dd, Dn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00010|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vceq.x8-32 Vd, Vn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00010|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcge.f16-32 Dd, Dn, Dm"                                  , "t32": "1111|11110|Vd'|0H|Vn|Vd|1110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcge.f16-32 Vd, Vn, Vm"                                  , "t32": "1111|11110|Vd'|0H|Vn|Vd|1110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcge.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00110|Vd'|0H|Vn|Vd|1110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcge.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00110|Vd'|0H|Vn|Vd|1110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vcge.f16-32 Dd, Dn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01001|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcge.f16-32 Vd, Vn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01001|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcge.f16-32 Dd, Dn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01001|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcge.f16-32 Vd, Vn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01001|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vcge.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0011|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vcge.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0011|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vcge.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0011|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vcge.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0011|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vcge.s8-32 Dd, Dn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00001|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcge.s8-32 Vd, Vn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00001|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcge.s8-32 Dd, Dn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00001|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcge.s8-32 Vd, Vn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00001|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcgt.f16-32 Dd, Dn, Dm"                                  , "t32": "1111|11110|Vd'|1H|Vn|Vd|1110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcgt.f16-32 Vd, Vn, Vm"                                  , "t32": "1111|11110|Vd'|1H|Vn|Vd|1110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcgt.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00110|Vd'|1H|Vn|Vd|1110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcgt.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00110|Vd'|1H|Vn|Vd|1110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vcgt.f16-32 Dd, Dn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01000|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcgt.f16-32 Vd, Vn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01000|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcgt.f16-32 Dd, Dn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01000|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcgt.f16-32 Vd, Vn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01000|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vcgt.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vcgt.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0011|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vcgt.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vcgt.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0011|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vcgt.s8-32 Dd, Dn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00000|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcgt.s8-32 Vd, Vn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00000|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcgt.s8-32 Dd, Dn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00000|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcgt.s8-32 Vd, Vn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00000|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcle.f16-32 Dd, Dn, Dm"                                  , "t32": "1111|11110|Vd'|0H|Vm|Vd|1110|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcle.f16-32 Vd, Vn, Vm"                                  , "t32": "1111|11110|Vd'|0H|Vm|Vd|1110|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcle.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00110|Vd'|0H|Vm|Vd|1110|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcle.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00110|Vd'|0H|Vm|Vd|1110|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vcle.f16-32 Dd, Dn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01011|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcle.f16-32 Vd, Vn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01011|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcle.f16-32 Dd, Dn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01011|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vcle.f16-32 Vd, Vn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01011|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vcle.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vm|Vd|0011|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD"},
    {"inst": "vcle.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vm|Vd|0011|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD"},
    {"inst": "vcle.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0011|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD"},
    {"inst": "vcle.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0011|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD"},

    {"inst": "vcle.s8-32 Dd, Dn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00011|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcle.s8-32 Vd, Vn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00011|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcle.s8-32 Dd, Dn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00011|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcle.s8-32 Vd, Vn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00011|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcls.x8-32 Dd, Dn"                                       , "t32": "1111|11111|Vd'|11|sz|00|Vd|01000|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcls.x8-32 Vd, Vn"                                       , "t32": "1111|11111|Vd'|11|sz|00|Vd|01000|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcls.x8-32 Dd, Dn"                                       , "a32": "1111|00111|Vd'|11|sz|00|Vd|01000|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcls.x8-32 Vd, Vn"                                       , "a32": "1111|00111|Vd'|11|sz|00|Vd|01000|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vclt.f16-32 Dd, Dn, Dm"                                  , "t32": "1111|11110|Vd'|1H|Vm|Vd|1110|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vclt.f16-32 Vd, Vn, Vm"                                  , "t32": "1111|11110|Vd'|1H|Vm|Vd|1110|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vclt.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00110|Vd'|1H|Vm|Vd|1110|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vclt.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00110|Vd'|1H|Vm|Vd|1110|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vclt.f16-32 Dd, Dn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01100|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vclt.f16-32 Vd, Vn, #zero"                               , "t32": "1111|11111|Vd'|11|sz|01|Vd|01100|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vclt.f16-32 Dd, Dn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01100|0|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vclt.f16-32 Vd, Vn, #zero"                               , "a32": "1111|00111|Vd'|11|sz|01|Vd|01100|1|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vclt.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vm|Vd|0011|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclt.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vm|Vd|0011|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclt.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0011|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclt.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0011|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vclt.s8-32 Dd, Dn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00100|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclt.s8-32 Vd, Vn, #zero"                                , "t32": "1111|11111|Vd'|11|sz|01|Vd|00100|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclt.s8-32 Dd, Dn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00100|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclt.s8-32 Vd, Vn, #zero"                                , "a32": "1111|00111|Vd'|11|sz|01|Vd|00100|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vclz.x8-32 Dd, Dn"                                       , "t32": "1111|11111|Vd'|11|sz|00|Vd|01001|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclz.x8-32 Vd, Vn"                                       , "t32": "1111|11111|Vd'|11|sz|00|Vd|01001|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclz.x8-32 Dd, Dn"                                       , "a32": "1111|00111|Vd'|11|sz|00|Vd|01001|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vclz.x8-32 Vd, Vn"                                       , "a32": "1111|00111|Vd'|11|sz|00|Vd|01001|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcmla.f16-32 Dx, Dn, Dm, #rot2"                          , "t32": "1111|110|imm:2|Vx'|1|isFp32|Vn|Vx|1000|Vn'|0|Vm'|0|Vm"  , "ext": "FCMA ~FP16FULL", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f16-32 Vx, Vn, Vm, #rot2"                          , "t32": "1111|110|imm:2|Vx'|1|isFp32|Vn|Vx|1000|Vn'|1|Vm'|0|Vm"  , "ext": "FCMA ~FP16FULL", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f16-32 Dx, Dn, Dm, #rot2"                          , "a32": "1111|110|imm:2|Vx'|1|isFp32|Vn|Vx|1000|Vn'|0|Vm'|0|Vm"  , "ext": "FCMA ~FP16FULL", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f16-32 Vx, Vn, Vm, #rot2"                          , "a32": "1111|110|imm:2|Vx'|1|isFp32|Vn|Vx|1000|Vn'|1|Vm'|0|Vm"  , "ext": "FCMA ~FP16FULL", "imm": "VecRot2Imm(rot2)"},

    {"inst": "vcmla.f16 Dx, Dn, Dm<=15[#i], #rot2"                     , "t32": "1111|1110|0|Vx'|imm:2|Vn|Vx|1000|Vn'|0|i:1|0|Vm"        , "ext": "FCMA FP16FULL", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f16 Vx, Vn, Dm<=15[#i], #rot2"                     , "t32": "1111|1110|0|Vx'|imm:2|Vn|Vx|1000|Vn'|1|i:1|0|Vm"        , "ext": "FCMA FP16FULL", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f16 Dx, Dn, Dm<=15[#i], #rot2"                     , "a32": "1111|1110|0|Vx'|imm:2|Vn|Vx|1000|Vn'|0|i:1|0|Vm"        , "ext": "FCMA FP16FULL", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f16 Vx, Vn, Dm<=15[#i], #rot2"                     , "a32": "1111|1110|0|Vx'|imm:2|Vn|Vx|1000|Vn'|1|i:1|0|Vm"        , "ext": "FCMA FP16FULL", "imm": "VecRot2Imm(rot2)"},

    {"inst": "vcmla.f32 Dx, Dn, Dm[#i], #rot2"                         , "t32": "1111|1110|1|Vx'|imm:2|Vn|Vx|1000|Vn'|0|Vm'|0|Vm"        , "ext": "FCMA", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f32 Vx, Vn, Dm[#i], #rot2"                         , "t32": "1111|1110|1|Vx'|imm:2|Vn|Vx|1000|Vn'|1|Vm'|0|Vm"        , "ext": "FCMA", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f32 Dx, Dn, Dm[#i], #rot2"                         , "a32": "1111|1110|1|Vx'|imm:2|Vn|Vx|1000|Vn'|0|Vm'|0|Vm"        , "ext": "FCMA", "imm": "VecRot2Imm(rot2)"},
    {"inst": "vcmla.f32 Vx, Vn, Dm[#i], #rot2"                         , "a32": "1111|1110|1|Vx'|imm:2|Vn|Vx|1000|Vn'|1|Vm'|0|Vm"        , "ext": "FCMA", "imm": "VecRot2Imm(rot2)"},

    {"inst": "vcmp.f16-32 Sn, Sm"                                      , "t32": "1110|11101|'Vn|11|01|00|Vn|10|sz|01|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vcmp.f16-32 Sn, Sm"                                      , "a32": "cond|11101|'Vn|11|01|00|Vn|10|sz|01|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vcmp.f16-32 Sn, #zero"                                   , "t32": "1110|11101|'Vn|11|01|01|Vn|10|sz|01|00|0000"            , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vcmp.f16-32 Sn, #zero"                                   , "a32": "cond|11101|'Vn|11|01|01|Vn|10|sz|01|00|0000"            , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vcmp.f64 Dn, Dm"                                         , "t32": "1110|11101|Vn'|11|01|00|Vn|10|11|01|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vcmp.f64 Dn, Dm"                                         , "a32": "cond|11101|Vn'|11|01|00|Vn|10|11|01|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vcmp.f64 Dn, #zero"                                      , "t32": "1110|11101|Vn'|11|01|01|Vn|10|11|01|00|0000"            , "ext": "VFPv2"},
    {"inst": "vcmp.f64 Dn, #zero"                                      , "a32": "cond|11101|Vn'|11|01|01|Vn|10|11|01|00|0000"            , "ext": "VFPv2"},

    {"inst": "vcmpe.f16-32 Sn, Sm"                                     , "t32": "1110|11101|'Vn|11|01|00|Vn|10|sz|11|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vcmpe.f16-32 Sn, Sm"                                     , "a32": "cond|11101|'Vn|11|01|00|Vn|10|sz|11|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vcmpe.f16-32 Sn, #zero"                                  , "t32": "1110|11101|'Vn|11|01|01|Vn|10|sz|11|00|0000"            , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vcmpe.f16-32 Sn, #zero"                                  , "a32": "cond|11101|'Vn|11|01|01|Vn|10|sz|11|00|0000"            , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vcmpe.f64 Dn, Dm"                                        , "t32": "1110|11101|Vn'|11|01|00|Vn|10|11|11|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vcmpe.f64 Dn, Dm"                                        , "a32": "cond|11101|Vn'|11|01|00|Vn|10|11|11|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vcmpe.f64 Dn, #zero"                                     , "t32": "1110|11101|Vn'|11|01|01|Vn|10|11|11|00|0000"            , "ext": "VFPv2"},
    {"inst": "vcmpe.f64 Dn, #zero"                                     , "a32": "cond|11101|Vn'|11|01|01|Vn|10|11|11|00|0000"            , "ext": "VFPv2"},

    {"inst": "vcnt.x8 Dd, Dn"                                          , "t32": "1111|11111|Vd'|11|00|00|Vd|01010|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcnt.x8 Vd, Vn"                                          , "t32": "1111|11111|Vd'|11|00|00|Vd|01010|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcnt.x8 Dd, Dn"                                          , "a32": "1111|00111|Vd'|11|00|00|Vd|01010|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcnt.x8 Vd, Vn"                                          , "a32": "1111|00111|Vd'|11|00|00|Vd|01010|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcvt.f32.f64 Sd, Dn"                                     , "t32": "1110|11101|'Vd|11|01|11|Vd|10111|1|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f64.f32 Dd, Sn"                                     , "t32": "1110|11101|Vd'|11|01|11|Vd|10101|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f32.f64 Sd, Dn"                                     , "a32": "cond|11101|'Vd|11|01|11|Vd|10111|1|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f64.f32 Dd, Sn"                                     , "a32": "cond|11101|Vd'|11|01|11|Vd|10101|1|'Vn|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vcvt.f32.s32 Sd, Sn"                                     , "t32": "1110|11101|'Vd|11|10|00|Vd|10101|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f32.u32 Sd, Sn"                                     , "t32": "1110|11101|'Vd|11|10|00|Vd|10100|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f32.s32 Sd, Sn"                                     , "a32": "cond|11101|'Vd|11|10|00|Vd|10101|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f32.u32 Sd, Sn"                                     , "a32": "cond|11101|'Vd|11|10|00|Vd|10100|1|'Vn|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vcvt.f64.s32 Dd, Sn"                                     , "t32": "1110|11101|Vd'|11|10|00|Vd|10111|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f64.u32 Dd, Sn"                                     , "t32": "1110|11101|Vd'|11|10|00|Vd|10110|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f64.s32 Dd, Sn"                                     , "a32": "cond|11101|Vd'|11|10|00|Vd|10111|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvt.f64.u32 Dd, Sn"                                     , "a32": "cond|11101|Vd'|11|10|00|Vd|10110|1|'Vn|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vcvt.f32.s16 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|10|10|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.s16.f32 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|11|10|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.f32.s16 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|10|10|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.s16.f32 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|11|10|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},

    {"inst": "vcvt.f32.u16 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|10|11|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.u16.f32 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|11|11|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.f32.u16 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|10|11|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.u16.f32 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|11|11|Vx|10100|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},

    {"inst": "vcvt.f32.s32 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|10|10|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.s32.f32 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|11|10|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.f32.s32 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|10|10|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.s32.f32 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|11|10|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},

    {"inst": "vcvt.f32.u32 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|10|11|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.u32.f32 Sx, Sx, #fbits"                             , "t32": "1110|11101|'Vx|11|11|11|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.f32.u32 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|10|11|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.u32.f32 Sx, Sx, #fbits"                             , "a32": "cond|11101|'Vx|11|11|11|Vx|10101|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},

    {"inst": "vcvt.f64.s16 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|10|10|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.s16.f64 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|11|10|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.f64.s16 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|10|10|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.s16.f64 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|11|10|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},

    {"inst": "vcvt.f64.u16 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|10|11|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.u16.f64 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|11|11|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.f64.u16 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|10|11|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},
    {"inst": "vcvt.u16.f64 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|11|11|Vx|10110|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(16, fbits)"},

    {"inst": "vcvt.f64.s32 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|10|10|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.s32.f64 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|11|10|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.f64.s32 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|10|10|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.s32.f64 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|11|10|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},

    {"inst": "vcvt.f64.u32 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|10|11|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.u32.f64 Dx, Dx, #fbits"                             , "t32": "1110|11101|Vx'|11|11|11|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.f64.u32 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|10|11|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},
    {"inst": "vcvt.u32.f64 Dx, Dx, #fbits"                             , "a32": "cond|11101|Vx'|11|11|11|Vx|10111|1|imm[0]|0|imm[4:1]"   , "ext": "VFPv3", "imm": "VecFBitsVFP(32, fbits)"},

    {"inst": "vcvt.f16.f32 Dd, Vn"                                     , "t32": "1111|11111|Vd'|11|01|10|Vd|01100|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvt.f32.f16 Vd, Dn"                                     , "t32": "1111|11111|Vd'|11|01|10|Vd|01110|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvt.f16.f32 Dd, Vn"                                     , "a32": "1111|00111|Vd'|11|01|10|Vd|01100|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvt.f32.f16 Vd, Dn"                                     , "a32": "1111|00111|Vd'|11|01|10|Vd|01110|0|Vn'|0|Vn"            , "ext": "FP16CONV"},

    {"inst": "vcvt.s32.f32 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01110|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.s32.f32 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01110|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.s32.f32 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01110|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.s32.f32 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01110|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcvt.u32.f32 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01111|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.u32.f32 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01111|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.u32.f32 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01111|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.u32.f32 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01111|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcvt.f32.s32 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01100|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.f32.s32 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01100|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.f32.s32 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01100|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.f32.s32 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01100|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcvt.f32.u32 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01101|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.f32.u32 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|10|11|Vd|01101|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.f32.u32 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01101|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vcvt.f32.u32 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|10|11|Vd|01101|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vcvt.f16.s16 Dd, Dn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11000|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f16.s16 Vd, Vn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11000|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s16.f16 Dd, Dn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11010|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s16.f16 Vd, Vn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11010|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f16.s16 Dd, Dn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11000|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f16.s16 Vd, Vn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11000|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s16.f16 Dd, Dn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11010|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s16.f16 Vd, Vn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11010|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},

    {"inst": "vcvt.f16.u16 Dd, Dn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11000|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f16.u16 Vd, Vn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11000|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u16.f16 Dd, Dn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11010|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u16.f16 Vd, Vn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11010|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f16.u16 Dd, Dn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11000|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f16.u16 Vd, Vn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11000|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u16.f16 Dd, Dn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11010|0|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u16.f16 Vd, Vn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11010|1|Vn'|1|Vn"               , "ext": "FP16CONV", "imm": "VecFBitsASIMD(fbits)"},

    {"inst": "vcvt.f32.s32 Dd, Dn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11100|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f32.s32 Vd, Vn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11100|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s32.f32 Dd, Dn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11110|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s32.f32 Vd, Vn, #fbits"                             , "t32": "1110|11111|Vd'|imm:6|Vd|11110|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f32.s32 Dd, Dn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11100|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f32.s32 Vd, Vn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11100|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s32.f32 Dd, Dn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11110|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.s32.f32 Vd, Vn, #fbits"                             , "a32": "1111|00101|Vd'|imm:6|Vd|11110|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},

    {"inst": "vcvt.f32.u32 Dd, Dn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11100|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f32.u32 Vd, Vn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11100|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u32.f32 Dd, Dn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11110|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u32.f32 Vd, Vn, #fbits"                             , "t32": "1111|11111|Vd'|imm:6|Vd|11110|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f32.u32 Dd, Dn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11100|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.f32.u32 Vd, Vn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11100|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u32.f32 Dd, Dn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11110|0|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},
    {"inst": "vcvt.u32.f32 Vd, Vn, #fbits"                             , "a32": "1111|00111|Vd'|imm:6|Vd|11110|1|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecFBitsASIMD(fbits)"},

    {"inst": "vcvta.s32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|00|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvta.s32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|00|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvta.s32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|00|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvta.s32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|00|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvta.s32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|00|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvta.s32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|00|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvta.u32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|00|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvta.u32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|00|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvta.u32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|00|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvta.u32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|00|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvta.u32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|00|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvta.u32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|00|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtb.f32.f16 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|00|10|Vd|10100|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtb.f16.f32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|00|11|Vd|10100|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtb.f32.f16 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|00|10|Vd|10100|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtb.f16.f32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|00|11|Vd|10100|1|'Vn|0|Vn"            , "ext": "FP16CONV"},

    {"inst": "vcvtb.f64.f16 Dd, Sn"                                    , "t32": "1110|11101|Vd'|11|00|10|Vd|10110|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtb.f16.f64 Sd, Dn"                                    , "t32": "1110|11101|'Vd|11|00|11|Vd|10110|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtb.f64.f16 Dd, Sn"                                    , "a32": "cond|11101|Vd'|11|00|10|Vd|10110|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtb.f16.f64 Sd, Dn"                                    , "a32": "cond|11101|'Vd|11|00|11|Vd|10110|1|Vn'|0|Vn"            , "ext": "FP16CONV"},

    {"inst": "vcvtb.bf16.f32 Sd, Dn"                                   , "t32": "1110|11101|'Vd|11|00|11|Vd|10010|1|'Vn|0|Vn"            , "ext": "AA32BF16"},
    {"inst": "vcvtb.bf16.f32 Sd, Dn"                                   , "a32": "cond|11101|'Vd|11|00|11|Vd|10010|1|'Vn|0|Vn"            , "ext": "AA32BF16"},

    {"inst": "vcvtm.s16.f16 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00110|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.s16.f16 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00110|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.u16.f16 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00111|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.u16.f16 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00111|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.s32.f32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00110|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.s32.f32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00111|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.s16.f16 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00110|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.s16.f16 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00110|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.u16.f16 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00111|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.u16.f16 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00111|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.s32.f32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00110|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.s32.f32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00111|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtm.s32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|11|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.s32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|11|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.s32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|11|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|11|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.u32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|11|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|11|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.s32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|11|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.s32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|11|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.s32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|11|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|11|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtm.u32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|11|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtm.u32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|11|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtn.s16.f16 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00010|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.s16.f16 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00010|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.u16.f16 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00011|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.u16.f16 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00011|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.s32.f32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00010|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.s32.f32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00010|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00011|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00011|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.s16.f16 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00010|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.s16.f16 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00010|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.u16.f16 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00011|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.u16.f16 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00011|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.s32.f32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00010|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.s32.f32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00010|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00011|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00011|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtn.s32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|01|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.s32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|01|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.s32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|01|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|01|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.u32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|01|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|01|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.s32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|01|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.s32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|01|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.s32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|01|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|01|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtn.u32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|01|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtn.u32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|01|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtp.s16.f16 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00100|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.s16.f16 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00100|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.u16.f16 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00101|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.u16.f16 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|01|11|Vd|00101|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.s32.f32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00100|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.s32.f32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00100|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00101|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|10|11|Vd|00101|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.s16.f16 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00100|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.s16.f16 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00100|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.u16.f16 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00101|0|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.u16.f16 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|01|11|Vd|00101|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.s32.f32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00100|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.s32.f32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00100|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00101|0|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|10|11|Vd|00101|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtp.s32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|10|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.s32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|10|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.s32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|10|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f16 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|10|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.u32.f32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|11|10|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f64 Sd, Dn"                                    , "t32": "1111|11101|'Vd|11|11|10|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.s32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|10|Vd|10011|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.s32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|10|Vd|10101|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.s32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|10|Vd|10111|1|Vn'|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f16 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|10|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtp.u32.f32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|11|10|Vd|10100|1|'Vn|0|Vn"            , "ext": "ARMv8+"},
    {"inst": "vcvtp.u32.f64 Sd, Dn"                                    , "a32": "1111|11101|'Vd|11|11|10|Vd|10110|1|Vn'|0|Vn"            , "ext": "ARMv8+"},

    {"inst": "vcvtr.s32.f16 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|11|01|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtr.s32.f32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|11|01|Vd|10100|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvtr.s32.f64 Sd, Dn"                                    , "t32": "1110|11101|'Vd|11|11|01|Vd|10110|1|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvtr.s32.f16 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|11|01|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtr.s32.f32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|11|01|Vd|10100|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvtr.s32.f64 Sd, Dn"                                    , "a32": "cond|11101|'Vd|11|11|01|Vd|10110|1|Vn'|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vcvtr.u32.f16 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|11|00|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtr.u32.f32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|11|00|Vd|10100|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvtr.u32.f64 Sd, Dn"                                    , "t32": "1110|11101|'Vd|11|11|00|Vd|10110|1|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvtr.u32.f16 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|11|00|Vd|10010|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtr.u32.f32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|11|00|Vd|10100|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vcvtr.u32.f64 Sd, Dn"                                    , "a32": "cond|11101|'Vd|11|11|00|Vd|10110|1|Vn'|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vcvtt.f32.f16 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|00|10|Vd|10101|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtt.f16.f32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|00|11|Vd|10101|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtt.f32.f16 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|00|10|Vd|10101|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtt.f16.f32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|00|11|Vd|10101|1|'Vn|0|Vn"            , "ext": "FP16CONV"},

    {"inst": "vcvtt.f64.f16 Dd, Sn"                                    , "t32": "1110|11101|Vd'|11|00|10|Vd|10111|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtt.f16.f64 Sd, Dn"                                    , "t32": "1110|11101|'Vd|11|00|11|Vd|10111|1|Vn'|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtt.f64.f16 Dd, Sn"                                    , "a32": "cond|11101|Vd'|11|00|10|Vd|10111|1|'Vn|0|Vn"            , "ext": "FP16CONV"},
    {"inst": "vcvtt.f16.f64 Sd, Dn"                                    , "a32": "cond|11101|'Vd|11|00|11|Vd|10111|1|Vn'|0|Vn"            , "ext": "FP16CONV"},

    {"inst": "vcvtt.bf16.f32 Sd, Dn"                                   , "t32": "1110|11101|'Vd|11|00|11|Vd|10011|1|'Vn|0|Vn"            , "ext": "AA32BF16"},
    {"inst": "vcvtt.bf16.f32 Sd, Dn"                                   , "a32": "cond|11101|'Vd|11|00|11|Vd|10011|1|'Vn|0|Vn"            , "ext": "AA32BF16"},

    {"inst": "vdiv.f16-32 Sd, Sn, Sm"                                  , "t32": "1110|11101|'Vd|00|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vdiv.f16-32 Sd, Sn, Sm"                                  , "a32": "cond|11101|'Vd|00|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vdiv.f64 Dd, Dn, Dm"                                     , "t32": "1110|11101|Vd'|00|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vdiv.f64 Dd, Dn, Dm"                                     , "a32": "cond|11101|Vd'|00|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vdot.bf16 Dd, Dn, Dm"                                    , "t32": "1110|11000|Vd'|00|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "AA32BF16"},
    {"inst": "vdot.bf16 Vd, Vn, Vm"                                    , "t32": "1110|11000|Vd'|00|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "AA32BF16"},
    {"inst": "vdot.bf16 Dd, Dn, Dm"                                    , "a32": "1111|11000|Vd'|00|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "AA32BF16"},
    {"inst": "vdot.bf16 Vd, Vn, Vm"                                    , "a32": "1111|11000|Vd'|00|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "AA32BF16"},

    {"inst": "vdot.bf16 Dd, Dn, Dm<=15[#i]"                            , "t32": "1110|11100|Vd'|00|Vn|Vd|1101|Vn'|0|i:1|0|Vm"            , "ext": "AA32BF16"},
    {"inst": "vdot.bf16 Vd, Vn, Dm<=15[#i]"                            , "t32": "1110|11100|Vd'|00|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "AA32BF16"},
    {"inst": "vdot.bf16 Dd, Dn, Dm<=15[#i]"                            , "a32": "1111|11100|Vd'|00|Vn|Vd|1101|Vn'|0|i:1|0|Vm"            , "ext": "AA32BF16"},
    {"inst": "vdot.bf16 Vd, Vn, Dm<=15[#i]"                            , "a32": "1111|11100|Vd'|00|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "AA32BF16"},

    {"inst": "vdup.8 Dd, Rn!=PC"                                       , "t32": "1110|11101|1|00|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.8 Vd, Rn!=PC"                                       , "t32": "1110|11101|1|10|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.16 Dd, Rn!=PC"                                      , "t32": "1110|11101|0|00|Vd|Rn|1011|Vd'|0|1|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.16 Vd, Rn!=PC"                                      , "t32": "1110|11101|0|10|Vd|Rn|1011|Vd'|0|1|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.32 Dd, Rn!=PC"                                      , "t32": "1110|11101|0|00|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.32 Vd, Rn!=PC"                                      , "t32": "1110|11101|0|10|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.8 Dd, Rn!=PC"                                       , "a32": "cond|11101|1|00|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.8 Vd, Rn!=PC"                                       , "a32": "cond|11101|1|10|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.16 Dd, Rn!=PC"                                      , "a32": "cond|11101|0|00|Vd|Rn|1011|Vd'|0|1|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.16 Vd, Rn!=PC"                                      , "a32": "cond|11101|0|10|Vd|Rn|1011|Vd'|0|1|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.32 Dd, Rn!=PC"                                      , "a32": "cond|11101|0|00|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},
    {"inst": "vdup.32 Vd, Rn!=PC"                                      , "a32": "cond|11101|0|10|Vd|Rn|1011|Vd'|0|0|1|0000"              , "ext": "ASIMD"},

    {"inst": "vdup.8 Dd, Dn[#i]"                                       , "t32": "1111|11111|Vd'|11|i:3|1|Vd|1100|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vdup.8 Vd, Dn[#i]"                                       , "t32": "1111|11111|Vd'|11|i:3|1|Vd|1100|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vdup.16 Dd, Dn[#i]"                                      , "t32": "1111|11111|Vd'|11|i:2|10|Vd|1100|0|0|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vdup.16 Vd, Dn[#i]"                                      , "t32": "1111|11111|Vd'|11|i:2|10|Vd|1100|0|1|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vdup.32 Dd, Dn[#i]"                                      , "t32": "1111|11111|Vd'|11|i:1|100|Vd|1100|0|0|Vn'|0|Vn"         , "ext": "ASIMD"},
    {"inst": "vdup.32 Vd, Dn[#i]"                                      , "t32": "1111|11111|Vd'|11|i:1|100|Vd|1100|0|1|Vn'|0|Vn"         , "ext": "ASIMD"},
    {"inst": "vdup.8 Dd, Dn[#i]"                                       , "a32": "1111|00111|Vd'|11|i:3|1|Vd|1100|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vdup.8 Vd, Dn[#i]"                                       , "a32": "1111|00111|Vd'|11|i:3|1|Vd|1100|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vdup.16 Dd, Dn[#i]"                                      , "a32": "1111|00111|Vd'|11|i:2|10|Vd|1100|0|0|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vdup.16 Vd, Dn[#i]"                                      , "a32": "1111|00111|Vd'|11|i:2|10|Vd|1100|0|1|Vn'|0|Vn"          , "ext": "ASIMD"},
    {"inst": "vdup.32 Dd, Dn[#i]"                                      , "a32": "1111|00111|Vd'|11|i:1|100|Vd|1100|0|0|Vn'|0|Vn"         , "ext": "ASIMD"},
    {"inst": "vdup.32 Vd, Dn[#i]"                                      , "a32": "1111|00111|Vd'|11|i:1|100|Vd|1100|0|1|Vn'|0|Vn"         , "ext": "ASIMD"},

    {"inst": "veor.any Dd, Dn, Dm"                                     , "t32": "1111|11110|Vd'|00|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "veor.any Vd, Vn, Vm"                                     , "t32": "1111|11110|Vd'|00|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "veor.any Dd, Dn, Dm"                                     , "a32": "1111|00110|Vd'|00|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "veor.any Vd, Vn, Vm"                                     , "a32": "1111|00110|Vd'|00|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vext.8 Dd, Dn, Dm, #immZ<=07"                            , "t32": "1110|11111|Vd'|11|Vn|Vd|immZ:4|Vn'|0|Vm'|0|Vm"          , "ext": "ASIMD"},
    {"inst": "vext.8 Vd, Vn, Vm, #immZ<=15"                            , "t32": "1110|11111|Vd'|11|Vn|Vd|immZ:4|Vn'|1|Vm'|0|Vm"          , "ext": "ASIMD"},
    {"inst": "vext.8 Dd, Dn, Dm, #immZ<=07"                            , "a32": "1111|00101|Vd'|11|Vn|Vd|immZ:4|Vn'|0|Vm'|0|Vm"          , "ext": "ASIMD"},
    {"inst": "vext.8 Vd, Vn, Vm, #immZ<=15"                            , "a32": "1111|00101|Vd'|11|Vn|Vd|immZ:4|Vn'|1|Vm'|0|Vm"          , "ext": "ASIMD"},

    {"inst": "vfma.f16-32 Sx, Sn, Sm"                                  , "t32": "1110|11101|'Vx|10|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},
    {"inst": "vfma.f16-32 Sx, Sn, Sm"                                  , "a32": "cond|11101|'Vx|10|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},

    {"inst": "vfma.f64 Dx, Dn, Dm"                                     , "t32": "1110|11101|Vx'|10|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv4"},
    {"inst": "vfma.f64 Dx, Dn, Dm"                                     , "a32": "cond|11101|Vx'|10|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv4"},

    {"inst": "vfma.f16-32 Dx, Dn, Dm"                                  , "t32": "1110|11110|Vx'|0H|Vn|Vx|1100|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vfma.f16-32 Vx, Vn, Vm"                                  , "t32": "1110|11110|Vx'|0H|Vn|Vx|1100|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vfma.f16-32 Dx, Dn, Dm"                                  , "a32": "1111|00100|Vx'|0H|Vn|Vx|1100|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vfma.f16-32 Vx, Vn, Vm"                                  , "a32": "1111|00100|Vx'|0H|Vn|Vx|1100|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vfmab.bf16 Vd, Vn, Vm"                                   , "t32": "1111|11000|Vd'|11|Vn|Vd|1000|Vn'|0|Vm'|1|Vm"            , "ext": "AA32BF16", "it": "out"},
    {"inst": "vfmab.bf16 Vd, Vn, Vm"                                   , "a32": "1111|11000|Vd'|11|Vn|Vd|1000|Vn'|0|Vm'|1|Vm"            , "ext": "AA32BF16"},

    {"inst": "vfmab.bf16 Vd, Vn, Dm<=07[#i]"                           , "t32": "1111|11100|Vd'|11|Vn|Vd|1000|Vn'|0|i:1|1|i:1|Vm[2:0]"   , "ext": "AA32BF16", "it": "out"},
    {"inst": "vfmab.bf16 Vd, Vn, Dm<=07[#i]"                           , "a32": "1111|11100|Vd'|11|Vn|Vd|1000|Vn'|0|i:1|1|i:1|Vm[2:0]"   , "ext": "AA32BF16"},

    {"inst": "vfmal.f16 Dx, Sn, Sm"                                    , "t32": "1111|11000|Vx'|10|Vn|Vx|1000|'Vn|0|'Vm|1|Vm"            , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmal.f16 Vx, Dn, Dm"                                    , "t32": "1111|11000|Vx'|10|Vn|Vx|1000|Vn'|1|Vm'|1|Vm"            , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmal.f16 Dx, Sn, Sm"                                    , "a32": "1111|11000|Vx'|10|Vn|Vx|1000|'Vn|0|'Vm|1|Vm"            , "ext": "FP16FML"},
    {"inst": "vfmal.f16 Vx, Dn, Dm"                                    , "a32": "1111|11000|Vx'|10|Vn|Vx|1000|Vn'|1|Vm'|1|Vm"            , "ext": "FP16FML"},

    {"inst": "vfmal.f16 Dx, Sn, Sm<=15[#i]"                            , "t32": "1111|11100|Vx'|00|Vn|Vx|1000|'Vn|0|Vm[0]|1|i:1|Vm[3:1]" , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmal.f16 Dx, Sn, Sm<=15[#i]"                            , "a32": "1111|11100|Vx'|00|Vn|Vx|1000|'Vn|0|Vm[0]|1|i:1|Vm[3:1]" , "ext": "FP16FML"},

    {"inst": "vfmal.f16 Vx, Dn, Dm<=15[#i]"                            , "t32": "1111|11100|Vx'|00|Vn|Vx|1000|Vn'|1|i:1|1|i:1|Vm:3"      , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmal.f16 Vx, Dn, Dm<=15[#i]"                            , "a32": "1111|11100|Vx'|00|Vn|Vx|1000|Vn'|1|i:1|1|i:1|Vm:3"      , "ext": "FP16FML"},

    {"inst": "vfmat.bf16 Vd, Vn, Vm"                                   , "t32": "1111|11000|Vd'|11|Vn|Vd|1000|Vn'|1|Vm'|1|Vm"            , "ext": "AA32BF16", "it": "out"},
    {"inst": "vfmat.bf16 Vd, Vn, Vm"                                   , "a32": "1111|11000|Vd'|11|Vn|Vd|1000|Vn'|1|Vm'|1|Vm"            , "ext": "AA32BF16"},

    {"inst": "vfmat.bf16 Vd, Vn, Dm<=07[#i]"                           , "t32": "1111|11100|Vd'|11|Vn|Vd|1000|Vn'|1|i:1|1|i:1|Vm[2:0]"   , "ext": "AA32BF16", "it": "out"},
    {"inst": "vfmat.bf16 Vd, Vn, Dm<=07[#i]"                           , "a32": "1111|11100|Vd'|11|Vn|Vd|1000|Vn'|1|i:1|1|i:1|Vm[2:0]"   , "ext": "AA32BF16"},

    {"inst": "vfms.f16-32 Sx, Sn, Sm"                                  , "t32": "1110|11101|'Vx|10|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},
    {"inst": "vfms.f16-32 Sx, Sn, Sm"                                  , "a32": "cond|11101|'Vx|10|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},

    {"inst": "vfms.f64 Dx, Dn, Dm"                                     , "t32": "1110|11101|Vx'|10|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv4"},
    {"inst": "vfms.f64 Dx, Dn, Dm"                                     , "a32": "cond|11101|Vx'|10|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv4"},

    {"inst": "vfms.f16-32 Dx, Dn, Dm"                                  , "t32": "1110|11110|Vx'|1H|Vn|Vx|1100|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vfms.f16-32 Vx, Vn, Vm"                                  , "t32": "1110|11110|Vx'|1H|Vn|Vx|1100|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vfms.f16-32 Dx, Dn, Dm"                                  , "a32": "1111|00100|Vx'|1H|Vn|Vx|1100|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vfms.f16-32 Vx, Vn, Vm"                                  , "a32": "1111|00100|Vx'|1H|Vn|Vx|1100|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vfmsl.f16 Dx, Sn, Sm"                                    , "t32": "1111|11001|Vx'|10|Vn|Vx|1000|'Vn|0|'Vm|1|Vm"            , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmsl.f16 Vx, Dn, Dm"                                    , "t32": "1111|11001|Vx'|10|Vn|Vx|1000|Vn'|1|Vm'|1|Vm"            , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmsl.f16 Dx, Sn, Sm"                                    , "a32": "1111|11001|Vx'|10|Vn|Vx|1000|'Vn|0|'Vm|1|Vm"            , "ext": "FP16FML"},
    {"inst": "vfmsl.f16 Vx, Dn, Dm"                                    , "a32": "1111|11001|Vx'|10|Vn|Vx|1000|Vn'|1|Vm'|1|Vm"            , "ext": "FP16FML"},

    {"inst": "vfmsl.f16 Dx, Sn, Sm<=15[#i]"                            , "t32": "1111|11100|Vx'|01|Vn|Vx|1000|'Vn|0|Vm[0]|1|i:1|Vm[3:1]" , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmsl.f16 Dx, Sn, Sm<=15[#i]"                            , "a32": "1111|11100|Vx'|01|Vn|Vx|1000|'Vn|0|Vm[0]|1|i:1|Vm[3:1]" , "ext": "FP16FML"},

    {"inst": "vfmsl.f16 Vx, Dn, Dm<=07[#i]"                            , "t32": "1111|11100|Vx'|01|Vn|Vx|1000|Vn'|1|i:1|1|i:1|Vm:3"      , "ext": "FP16FML", "it": "out"},
    {"inst": "vfmsl.f16 Vx, Dn, Dm<=07[#i]"                            , "a32": "1111|11100|Vx'|01|Vn|Vx|1000|Vn'|1|i:1|1|i:1|Vm:3"      , "ext": "FP16FML"},

    {"inst": "vfnma.f16-32 Sx, Sn, Sm"                                 , "t32": "1110|11101|'Vx|01|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},
    {"inst": "vfnma.f16-32 Sx, Sn, Sm"                                 , "a32": "cond|11101|'Vx|01|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},

    {"inst": "vfnma.f64 Dx, Dn, Dm"                                    , "t32": "1110|11101|Vx'|01|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv4"},
    {"inst": "vfnma.f64 Dx, Dn, Dm"                                    , "a32": "cond|11101|Vx'|01|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv4"},

    {"inst": "vfnms.f16-32 Sx, Sn, Sm"                                 , "t32": "1110|11101|'Vx|01|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},
    {"inst": "vfnms.f16-32 Sx, Sn, Sm"                                 , "a32": "cond|11101|'Vx|01|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv4 ~FP16FULL"},

    {"inst": "vfnms.f64 Dx, Dn, Dm"                                    , "t32": "1110|11101|Vx'|01|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv4"},
    {"inst": "vfnms.f64 Dx, Dn, Dm"                                    , "a32": "cond|11101|Vx'|01|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv4"},

    {"inst": "vhadd.x8-32 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vhadd.x8-32 Vd, Vn, Vm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0000|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vhadd.x8-32 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vhadd.x8-32 Vd, Vn, Vm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0000|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vhsub.x8-32 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vhsub.x8-32 Vd, Vn, Vm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0010|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vhsub.x8-32 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vhsub.x8-32 Vd, Vn, Vm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0010|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vins.f16 Sd, Sn"                                         , "t32": "1111|11101|'Vd|11|0000|Vd|1010|11|'Vn|0|Vn"             , "ext": "FP16FULL"},
    {"inst": "vins.f16 Sd, Sn"                                         , "a32": "1111|11101|'Vd|11|0000|Vd|1010|11|'Vn|0|Vn"             , "ext": "FP16FULL"},

    {"inst": "vjcvt.s32.f64 Sd, Dn"                                    , "t32": "1110|11101|'Vd|11|1001|Vd|1011|11|Vn'|0|Vn"             , "ext": "JSCVT"},
    {"inst": "vjcvt.s32.f64 Sd, Dn"                                    , "a32": "cond|11101|'Vd|11|1001|Vd|1011|11|Vn'|0|Vn"             , "ext": "JSCVT"},

    {"inst": "vld1.8-64 Dd, [Rn!=PC]"                                  , "t32": "1111|10010|Vd'|10|Rn|Vd|0111|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 Dd, [Rn!=PC, Rm!=XX]@"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|0111|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 Dd, [Rn!=PC, #off==8]@"                        , "t32": "1111|10010|Vd'|10|Rn|Vd|0111|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 2x{Dd<=30}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|1010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|1010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 2x{Dd<=30}+, [Rn!=PC, #off==16]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|1010|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 3x{Dd<=29}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|0110|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 3x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|0110|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 3x{Dd<=29}+, [Rn!=PC, #off==24]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|0110|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 4x{Dd<=28}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|0010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 4x{Dd<=28}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|0010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 4x{Dd<=28}+, [Rn!=PC, #off==32]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|0010|sz|0|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld1.8-64 Dd, [Rn!=PC]"                                  , "a32": "1111|01000|Vd'|10|Rn|Vd|0111|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 Dd, [Rn!=PC, Rm!=XX]@"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|0111|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 Dd, [Rn!=PC, #off==8]@"                        , "a32": "1111|01000|Vd'|10|Rn|Vd|0111|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 2x{Dd<=30}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|1010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|1010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 2x{Dd<=30}+, [Rn!=PC, #off==16]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|1010|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 3x{Dd<=29}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|0110|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 3x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|0110|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 3x{Dd<=29}+, [Rn!=PC, #off==24]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|0110|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 4x{Dd<=28}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|0010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1.8-64 4x{Dd<=28}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|0010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1.8-64 4x{Dd<=28}+, [Rn!=PC, #off==32]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|0010|sz|0|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld1.8 Dd[#i], [Rn!=PC]"                                 , "t32": "1111|10011|Vd'|10|Rn|Vd|0000|i:3|align:1|1111"          , "ext": "ASIMD"},
    {"inst": "vld1.8 Dd[#i], [Rn!=PC, Rm!=XX]@"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|0000|i:3|align:1|Rm"            , "ext": "ASIMD"},
    {"inst": "vld1.8 Dd[#i], [Rn!=PC, #off==1]@"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|0000|i:3|align:1|1101"          , "ext": "ASIMD"},
    {"inst": "vld1.16 Dd[#i], [Rn!=PC]"                                , "t32": "1111|10011|Vd'|10|Rn|Vd|0100|i:2|0|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld1.16 Dd[#i], [Rn!=PC, Rm!=XX]@"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|0100|i:2|0|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld1.16 Dd[#i], [Rn!=PC, #off==2]@"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|0100|i:2|0|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld1.32 Dd[#i], [Rn!=PC]"                                , "t32": "1111|10011|Vd'|10|Rn|Vd|1000|i:1|0|align:2|1111"        , "ext": "ASIMD"},
    {"inst": "vld1.32 Dd[#i], [Rn!=PC, Rm!=XX]@"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1000|i:1|0|align:2|Rm"          , "ext": "ASIMD"},
    {"inst": "vld1.32 Dd[#i], [Rn!=PC, #off==4]@"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|1000|i:1|0|align:2|1101"        , "ext": "ASIMD"},

    {"inst": "vld1.8 Dd[#i], [Rn!=PC]"                                 , "a32": "1111|01001|Vd'|10|Rn|Vd|0000|i:3|align:1|1111"          , "ext": "ASIMD"},
    {"inst": "vld1.8 Dd[#i], [Rn!=PC, Rm!=XX]@"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|0000|i:3|align:1|Rm"            , "ext": "ASIMD"},
    {"inst": "vld1.8 Dd[#i], [Rn!=PC, #off==1]@"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|0000|i:3|align:1|1101"          , "ext": "ASIMD"},
    {"inst": "vld1.16 Dd[#i], [Rn!=PC]"                                , "a32": "1111|01001|Vd'|10|Rn|Vd|0100|i:2|0|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld1.16 Dd[#i], [Rn!=PC, Rm!=XX]@"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|0100|i:2|0|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld1.16 Dd[#i], [Rn!=PC, #off==2]@"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|0100|i:2|0|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld1.32 Dd[#i], [Rn!=PC]"                                , "a32": "1111|01001|Vd'|10|Rn|Vd|1000|i:1|0|align:2|1111"        , "ext": "ASIMD"},
    {"inst": "vld1.32 Dd[#i], [Rn!=PC, Rm!=XX]@"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1000|i:1|0|align:2|Rm"          , "ext": "ASIMD"},
    {"inst": "vld1.32 Dd[#i], [Rn!=PC, #off==4]@"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|1000|i:1|0|align:2|1101"        , "ext": "ASIMD"},

    {"inst": "vld1r.8-32 Dd, [Rn!=PC]"                                 , "t32": "1111|10011|Vd'|10|Rn|Vd|1100|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 Dd, [Rn!=PC, Rm!=XX]@"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|1100|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 Dd, [Rn!=PC, #off==1<<sz]@"                   , "t32": "1111|10011|Vd'|10|Rn|Vd|1100|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 2x{Dd<=30}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|1100|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|1100|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 2x{Dd<=30}+, [Rn!=PC, #off==1<<sz]@"          , "t32": "1111|10011|Vd'|10|Rn|Vd|1100|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld1r.8-32 Dd, [Rn!=PC]"                                 , "a32": "1111|01001|Vd'|10|Rn|Vd|1100|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 Dd, [Rn!=PC, Rm!=XX]@"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|1100|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 Dd, [Rn!=PC, #off==1<<sz]@"                   , "a32": "1111|01001|Vd'|10|Rn|Vd|1100|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 2x{Dd<=30}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|1100|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|1100|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld1r.8-32 2x{Dd<=30}+, [Rn!=PC, #off==1<<sz]@"          , "a32": "1111|01001|Vd'|10|Rn|Vd|1100|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld2.8-32 2x{Dd<=30}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|1000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|1000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=30}+, [Rn!=PC, #off==16]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|1000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=29}++, [Rn!=PC]"                        , "t32": "1111|10010|Vd'|10|Rn|Vd|1001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=29}++, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10010|Vd'|10|Rn|Vd|1001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=29}++, [Rn!=PC, #off==16]@"             , "t32": "1111|10010|Vd'|10|Rn|Vd|1001|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 4x{Dd<=28}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|0011|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 4x{Dd<=28}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|0011|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld2.8-32 4x{Dd<=28}+, [Rn!=PC, #off==32]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|0011|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vld2.8-32 2x{Dd<=30}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|1000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|1000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=30}+, [Rn!=PC, #off==16]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|1000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=29}++, [Rn!=PC]"                        , "a32": "1111|01000|Vd'|10|Rn|Vd|1001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=29}++, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01000|Vd'|10|Rn|Vd|1001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld2.8-32 2x{Dd<=29}++, [Rn!=PC, #off==16]@"             , "a32": "1111|01000|Vd'|10|Rn|Vd|1001|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 4x{Dd<=28}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|0011|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld2.8-32 4x{Dd<=28}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|0011|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld2.8-32 4x{Dd<=28}+, [Rn!=PC, #off==32]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|0011|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vld2.8 2x{Dd<=30[#i]}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|0001|i:3|align:1|1111"          , "ext": "ASIMD"},
    {"inst": "vld2.8 2x{Dd<=30[#i]}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|0001|i:3|align:1|Rm"            , "ext": "ASIMD"},
    {"inst": "vld2.8 2x{Dd<=30[#i]}+, [Rn!=PC, #off==2]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|0001|i:3|align:1|1101"          , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=30[#i]}+, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|0101|i:2|0|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=30[#i]}+, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|0101|i:2|0|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=30[#i]}+, [Rn!=PC, #off==4]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|0101|i:2|0|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=29[#i]}++, [Rn!=PC]"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|0101|i:2|1|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=29[#i]}++, [Rn!=PC, Rm!=XX]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|0101|i:2|1|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=29[#i]}++, [Rn!=PC, #off==4]@"            , "t32": "1111|10011|Vd'|10|Rn|Vd|0101|i:2|1|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=30[#i]}+, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1001|i:1|0|0|align:1|1111"      , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=30[#i]}+, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|1001|i:1|0|0|align:1|Rm"        , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=30[#i]}+, [Rn!=PC, #off==8]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|1001|i:1|0|0|align:1|1101"      , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=29[#i]}++, [Rn!=PC]"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|1001|i:1|1|0|align:1|1111"      , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=29[#i]}++, [Rn!=PC, Rm!=XX]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|1001|i:1|1|0|align:1|Rm"        , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=29[#i]}++, [Rn!=PC, #off==8]@"            , "t32": "1111|10011|Vd'|10|Rn|Vd|1001|i:1|1|0|align:1|1101"      , "ext": "ASIMD"},

    {"inst": "vld2.8 2x{Dd<=30[#i]}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|0001|i:3|align:1|1111"          , "ext": "ASIMD"},
    {"inst": "vld2.8 2x{Dd<=30[#i]}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|0001|i:3|align:1|Rm"            , "ext": "ASIMD"},
    {"inst": "vld2.8 2x{Dd<=30[#i]}+, [Rn!=PC, #off==2]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|0001|i:3|align:1|1101"          , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=30[#i]}+, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|0101|i:2|0|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=30[#i]}+, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|0101|i:2|0|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=30[#i]}+, [Rn!=PC, #off==4]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|0101|i:2|0|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=29[#i]}++, [Rn!=PC]"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|0101|i:2|1|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=29[#i]}++, [Rn!=PC, Rm!=XX]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|0101|i:2|1|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld2.16 2x{Dd<=29[#i]}++, [Rn!=PC, #off==4]@"            , "a32": "1111|01001|Vd'|10|Rn|Vd|0101|i:2|1|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=30[#i]}+, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1001|i:1|0|0|align:1|1111"      , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=30[#i]}+, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|1001|i:1|0|0|align:1|Rm"        , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=30[#i]}+, [Rn!=PC, #off==8]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|1001|i:1|0|0|align:1|1101"      , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=29[#i]}++, [Rn!=PC]"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|1001|i:1|1|0|align:1|1111"      , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=29[#i]}++, [Rn!=PC, Rm!=XX]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|1001|i:1|1|0|align:1|Rm"        , "ext": "ASIMD"},
    {"inst": "vld2.32 2x{Dd<=29[#i]}++, [Rn!=PC, #off==8]@"            , "a32": "1111|01001|Vd'|10|Rn|Vd|1001|i:1|1|0|align:1|1101"      , "ext": "ASIMD"},

    {"inst": "vld2r.8-32 2x{Dd<=30}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|1101|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|1101|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=30}+, [Rn!=PC, #off==2<<sz]@"          , "t32": "1111|10011|Vd'|10|Rn|Vd|1101|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=29}++, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1101|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=29}++, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|1101|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=29}++, [Rn!=PC, #off==2<<sz]@"         , "t32": "1111|10011|Vd'|10|Rn|Vd|1101|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld2r.8-32 2x{Dd<=30}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|1101|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=30}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|1101|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=30}+, [Rn!=PC, #off==2<<sz]@"          , "a32": "1111|01001|Vd'|10|Rn|Vd|1101|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=29}++, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1101|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=29}++, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|1101|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld2r.8-32 2x{Dd<=29}++, [Rn!=PC, #off==2<<sz]@"         , "a32": "1111|01001|Vd'|10|Rn|Vd|1101|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld3.8-32 3x{Dd<=29}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|0100|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|0100|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=29}+, [Rn!=PC, #off==24]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|0100|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=27}++, [Rn!=PC]"                        , "t32": "1111|10010|Vd'|10|Rn|Vd|0101|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=27}++, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10010|Vd'|10|Rn|Vd|0101|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=27}++, [Rn!=PC, #off==24]@"             , "t32": "1111|10010|Vd'|10|Rn|Vd|0101|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld3.8-32 3x{Dd<=29}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|0100|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|0100|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=29}+, [Rn!=PC, #off==24]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|0100|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=27}++, [Rn!=PC]"                        , "a32": "1111|01000|Vd'|10|Rn|Vd|0101|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=27}++, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01000|Vd'|10|Rn|Vd|0101|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld3.8-32 3x{Dd<=27}++, [Rn!=PC, #off==24]@"             , "a32": "1111|01000|Vd'|10|Rn|Vd|0101|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld3.8 3x{Dd<=29[#i]}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|0010|i:3|0|1111"                , "ext": "ASIMD"},
    {"inst": "vld3.8 3x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|0010|i:3|0|Rm"                  , "ext": "ASIMD"},
    {"inst": "vld3.8 3x{Dd<=29[#i]}+, [Rn!=PC, #off==3]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|0010|i:3|0|1101"                , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=29[#i]}+, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|0110|i:2|00|1111"               , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|0110|i:2|00|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=29[#i]}+, [Rn!=PC, #off==6]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|0110|i:2|00|1101"               , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=27[#i]}++, [Rn!=PC]"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|0110|i:2|10|1111"               , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|0110|i:2|10|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=27[#i]}++, [Rn!=PC, #off==6]@"            , "t32": "1111|10011|Vd'|10|Rn|Vd|0110|i:2|10|1101"               , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=29[#i]}+, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1010|i:1|000|1111"              , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|1010|i:1|000|Rm"                , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=29[#i]}+, [Rn!=PC, #off==12]@"            , "t32": "1111|10011|Vd'|10|Rn|Vd|1010|i:1|000|1101"              , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=27[#i]}++, [Rn!=PC]"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|1010|i:1|100|1111"              , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|1010|i:1|100|Rm"                , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=27[#i]}++, [Rn!=PC, #off==12]@"           , "t32": "1111|10011|Vd'|10|Rn|Vd|1010|i:1|100|1101"              , "ext": "ASIMD"},

    {"inst": "vld3.8 3x{Dd<=29[#i]}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|0010|i:3|0|1111"                , "ext": "ASIMD"},
    {"inst": "vld3.8 3x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|0010|i:3|0|Rm"                  , "ext": "ASIMD"},
    {"inst": "vld3.8 3x{Dd<=29[#i]}+, [Rn!=PC, #off==3]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|0010|i:3|0|1101"                , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=29[#i]}+, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|0110|i:2|00|1111"               , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|0110|i:2|00|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=29[#i]}+, [Rn!=PC, #off==6]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|0110|i:2|00|1101"               , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=27[#i]}++, [Rn!=PC]"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|0110|i:2|10|1111"               , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|0110|i:2|10|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3.16 3x{Dd<=27[#i]}++, [Rn!=PC, #off==6]@"            , "a32": "1111|01001|Vd'|10|Rn|Vd|0110|i:2|10|1101"               , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=29[#i]}+, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1010|i:1|000|1111"              , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|1010|i:1|000|Rm"                , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=29[#i]}+, [Rn!=PC, #off==12]@"            , "a32": "1111|01001|Vd'|10|Rn|Vd|1010|i:1|000|1101"              , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=27[#i]}++, [Rn!=PC]"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|1010|i:1|100|1111"              , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|1010|i:1|100|Rm"                , "ext": "ASIMD"},
    {"inst": "vld3.32 3x{Dd<=27[#i]}++, [Rn!=PC, #off==12]@"           , "a32": "1111|01001|Vd'|10|Rn|Vd|1010|i:1|100|1101"              , "ext": "ASIMD"},

    {"inst": "vld3r.8-32 3x{Dd<=29}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|1110|sz|0|0|1111"               , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|1110|sz|0|0|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=29}+, [Rn!=PC, #off==3<<sz]@"          , "t32": "1111|10011|Vd'|10|Rn|Vd|1110|sz|0|0|1101"               , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=27}++, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1110|sz|1|0|1111"               , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=27}++, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|1110|sz|1|0|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=27}++, [Rn!=PC, #off==3<<sz]@"         , "t32": "1111|10011|Vd'|10|Rn|Vd|1110|sz|1|0|1101"               , "ext": "ASIMD"},

    {"inst": "vld3r.8-32 3x{Dd<=29}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|1110|sz|0|0|1111"               , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|1110|sz|0|0|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=29}+, [Rn!=PC, #off==3<<sz]@"          , "a32": "1111|01001|Vd'|10|Rn|Vd|1110|sz|0|0|1101"               , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=27}++, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1110|sz|1|0|1111"               , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=27}++, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|1110|sz|1|0|Rm"                 , "ext": "ASIMD"},
    {"inst": "vld3r.8-32 3x{Dd<=27}++, [Rn!=PC, #off==3<<sz]@"         , "a32": "1111|01001|Vd'|10|Rn|Vd|1110|sz|1|0|1101"               , "ext": "ASIMD"},

    {"inst": "vld4.8-32 4x{Dd<=29}+, [Rn!=PC]"                         , "t32": "1111|10010|Vd'|10|Rn|Vd|0000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vd'|10|Rn|Vd|0000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=29}+, [Rn!=PC, #off==32]@"              , "t32": "1111|10010|Vd'|10|Rn|Vd|0000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=27}++, [Rn!=PC]"                        , "t32": "1111|10010|Vd'|10|Rn|Vd|0001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=27}++, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10010|Vd'|10|Rn|Vd|0001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=27}++, [Rn!=PC, #off==32]@"             , "t32": "1111|10010|Vd'|10|Rn|Vd|0001|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vld4.8-32 4x{Dd<=29}+, [Rn!=PC]"                         , "a32": "1111|01000|Vd'|10|Rn|Vd|0000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=29}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vd'|10|Rn|Vd|0000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=29}+, [Rn!=PC, #off==32]@"              , "a32": "1111|01000|Vd'|10|Rn|Vd|0000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=27}++, [Rn!=PC]"                        , "a32": "1111|01000|Vd'|10|Rn|Vd|0001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=27}++, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01000|Vd'|10|Rn|Vd|0001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vld4.8-32 4x{Dd<=27}++, [Rn!=PC, #off==32]@"             , "a32": "1111|01000|Vd'|10|Rn|Vd|0001|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vld4.8 4x{Dd<=29[#i]}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|0011|i:3|align:1|1111"          , "ext": "ASIMD"},
    {"inst": "vld4.8 4x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|0011|i:3|align:1|Rm"            , "ext": "ASIMD"},
    {"inst": "vld4.8 4x{Dd<=29[#i]}+, [Rn!=PC, #off==4]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|0011|i:3|align:1|1101"          , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=29[#i]}+, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|0111|i:2|0|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|0111|i:2|0|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=29[#i]}+, [Rn!=PC, #off==8]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|0111|i:2|0|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=27[#i]}++, [Rn!=PC]"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|0111|i:2|1|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|0111|i:2|1|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=27[#i]}++, [Rn!=PC, #off==8]@"            , "t32": "1111|10011|Vd'|10|Rn|Vd|0111|i:2|1|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=29[#i]}+, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1011|i:1|0|align:2|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|1011|i:1|0|align:2|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=29[#i]}+, [Rn!=PC, #off==16]@"            , "t32": "1111|10011|Vd'|10|Rn|Vd|1011|i:1|0|align:2|1101"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=27[#i]}++, [Rn!=PC]"                      , "t32": "1111|10011|Vd'|10|Rn|Vd|1011|i:1|1|align:2|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "t32": "1111|10011|Vd'|10|Rn|Vd|1011|i:1|1|align:2|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=27[#i]}++, [Rn!=PC, #off==16]@"           , "t32": "1111|10011|Vd'|10|Rn|Vd|1011|i:1|1|align:2|1101"        , "ext": "ASIMD"},

    {"inst": "vld4.8 4x{Dd<=29[#i]}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|0011|i:3|align:1|1111"          , "ext": "ASIMD"},
    {"inst": "vld4.8 4x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|0011|i:3|align:1|Rm"            , "ext": "ASIMD"},
    {"inst": "vld4.8 4x{Dd<=29[#i]}+, [Rn!=PC, #off==4]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|0011|i:3|align:1|1101"          , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=29[#i]}+, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|0111|i:2|0|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|0111|i:2|0|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=29[#i]}+, [Rn!=PC, #off==8]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|0111|i:2|0|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=27[#i]}++, [Rn!=PC]"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|0111|i:2|1|align:1|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|0111|i:2|1|align:1|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.16 4x{Dd<=27[#i]}++, [Rn!=PC, #off==8]@"            , "a32": "1111|01001|Vd'|10|Rn|Vd|0111|i:2|1|align:1|1101"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=29[#i]}+, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1011|i:1|0|align:2|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=29[#i]}+, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|1011|i:1|0|align:2|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=29[#i]}+, [Rn!=PC, #off==16]@"            , "a32": "1111|01001|Vd'|10|Rn|Vd|1011|i:1|0|align:2|1101"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=27[#i]}++, [Rn!=PC]"                      , "a32": "1111|01001|Vd'|10|Rn|Vd|1011|i:1|1|align:2|1111"        , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=27[#i]}++, [Rn!=PC, Rm!=XX]@"             , "a32": "1111|01001|Vd'|10|Rn|Vd|1011|i:1|1|align:2|Rm"          , "ext": "ASIMD"},
    {"inst": "vld4.32 4x{Dd<=27[#i]}++, [Rn!=PC, #off==16]@"           , "a32": "1111|01001|Vd'|10|Rn|Vd|1011|i:1|1|align:2|1101"        , "ext": "ASIMD"},

    {"inst": "vld4r.8-32 4x{Dd<=28}+, [Rn!=PC]"                        , "t32": "1111|10011|Vd'|10|Rn|Vd|1111|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=28}+, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10011|Vd'|10|Rn|Vd|1111|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=28}+, [Rn!=PC, #off==4<<sz]@"          , "t32": "1111|10011|Vd'|10|Rn|Vd|1111|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=25}++, [Rn!=PC]"                       , "t32": "1111|10011|Vd'|10|Rn|Vd|1111|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=25}++, [Rn!=PC, Rm!=XX]@"              , "t32": "1111|10011|Vd'|10|Rn|Vd|1111|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=25}++, [Rn!=PC, #off==4<<sz]@"         , "t32": "1111|10011|Vd'|10|Rn|Vd|1111|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vld4r.8-32 4x{Dd<=28}+, [Rn!=PC]"                        , "a32": "1111|01001|Vd'|10|Rn|Vd|1111|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=28}+, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01001|Vd'|10|Rn|Vd|1111|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=28}+, [Rn!=PC, #off==4<<sz]@"          , "a32": "1111|01001|Vd'|10|Rn|Vd|1111|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=25}++, [Rn!=PC]"                       , "a32": "1111|01001|Vd'|10|Rn|Vd|1111|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=25}++, [Rn!=PC, Rm!=XX]@"              , "a32": "1111|01001|Vd'|10|Rn|Vd|1111|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vld4r.8-32 4x{Dd<=25}++, [Rn!=PC, #off==4<<sz]@"         , "a32": "1111|01001|Vd'|10|Rn|Vd|1111|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vldr.16 Sd, [Rn, #+/-off*2]"                             , "t32": "1110|1101U|'Vd|01|Rn|Vd|1001|off:8"                     , "ext": "FP16FULL" , "it": "out"},
    {"inst": "vldr.32 Sd, [Rn, #+/-off*4]"                             , "t32": "1110|1101U|'Vd|01|Rn|Vd|1010|off:8"                     , "ext": "VFPv3"},
    {"inst": "vldr.64 Dd, [Rn, #+/-off*4]"                             , "t32": "1110|1101U|Vd'|01|Rn|Vd|1011|off:8"                     , "ext": "VFPv3"},
    {"inst": "vldr.16 Sd, [Rn, #+/-off*2]"                             , "a32": "cond|1101U|'Vd|01|Rn|Vd|1001|off:8"                     , "ext": "FP16FULL" },
    {"inst": "vldr.32 Sd, [Rn, #+/-off*4]"                             , "a32": "cond|1101U|'Vd|01|Rn|Vd|1010|off:8"                     , "ext": "VFPv3"},
    {"inst": "vldr.64 Dd, [Rn, #+/-off*4]"                             , "a32": "cond|1101U|Vd'|01|Rn|Vd|1011|off:8"                     , "ext": "VFPv3"},

    {"inst": "vmax.f16-32 Dd, Dn, Dm"                                  , "t32": "1110|11110|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vmax.f16-32 Vd, Vn, Vm"                                  , "t32": "1110|11110|Vd'|0H|Vn|Vd|1111|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vmax.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00100|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmax.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00100|Vd'|0H|Vn|Vd|1111|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmax.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmax.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmax.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0110|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmax.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0110|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmaxnm.f16-32 Sd, Sn, Sm"                                , "t32": "1111|11101|'Vd|00|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vmaxnm.f16-32 Sd, Sn, Sm"                                , "a32": "1111|11101|'Vd|00|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vmaxnm.f64 Dd, Dn, Dm"                                   , "t32": "1111|11101|Vd'|00|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+", "it": "out"},
    {"inst": "vmaxnm.f64 Dd, Dn, Dm"                                   , "a32": "1111|11101|Vd'|00|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+"},

    {"inst": "vmaxnm.f16-32 Dd, Dn, Dm"                                , "t32": "1111|11110|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vmaxnm.f16-32 Vd, Vn, Vm"                                , "t32": "1111|11110|Vd'|0H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vmaxnm.f16-32 Dd, Dn, Dm"                                , "a32": "1111|00110|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmaxnm.f16-32 Vd, Vn, Vm"                                , "a32": "1111|00110|Vd'|0H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmin.f16-32 Dd, Dn, Dm"                                  , "t32": "1110|11110|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vmin.f16-32 Vd, Vn, Vm"                                  , "t32": "1110|11110|Vd'|1H|Vn|Vd|1111|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vmin.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00100|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmin.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00100|Vd'|1H|Vn|Vd|1111|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmin.x8-32 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0110|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vmin.x8-32 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vn|Vd|0110|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vmin.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0110|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vmin.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0110|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vminnm.f16-32 Sd, Sn, Sm"                                , "t32": "1111|11101|'Vd|00|Vn|Vd|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vminnm.f16-32 Sd, Sn, Sm"                                , "a32": "1111|11101|'Vd|00|Vn|Vd|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vminnm.f64 Dd, Dn, Dm"                                   , "t32": "1111|11101|Vd'|00|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "ARMv8+", "it": "out"},
    {"inst": "vminnm.f64 Dd, Dn, Dm"                                   , "a32": "1111|11101|Vd'|00|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "ARMv8+"},

    {"inst": "vminnm.f16-32 Dd, Dn, Dm"                                , "t32": "1111|11110|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vminnm.f16-32 Vd, Vn, Vm"                                , "t32": "1111|11110|Vd'|1H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vminnm.f16-32 Dd, Dn, Dm"                                , "a32": "1111|00110|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vminnm.f16-32 Vd, Vn, Vm"                                , "a32": "1111|00110|Vd'|1H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmla.f16-32 Sx, Sn, Sm"                                  , "t32": "1110|11100|'Vx|00|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vmla.f16-32 Sx, Sn, Sm"                                  , "a32": "cond|11100|'Vx|00|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vmla.f64 Dx, Dn, Dm"                                     , "t32": "1110|11100|Vx'|00|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vmla.f64 Dx, Dn, Dm"                                     , "a32": "cond|11100|Vx'|00|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vmla.f16-32 Dx, Dn, Dm"                                  , "t32": "1110|11110|Vx'|0H|Vn|Vx|1101|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmla.f16-32 Vx, Vn, Vm"                                  , "t32": "1110|11110|Vx'|0H|Vn|Vx|1101|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmla.f16-32 Dx, Dn, Dm"                                  , "a32": "1111|00100|Vx'|0H|Vn|Vx|1101|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmla.f16-32 Vx, Vn, Vm"                                  , "a32": "1111|00100|Vx'|0H|Vn|Vx|1101|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmla.x8-32 Dx, Dn, Dm"                                   , "t32": "1110|11110|Vx'|sz|Vn|Vx|1001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.x8-32 Vx, Vn, Vm"                                   , "t32": "1110|11110|Vx'|sz|Vn|Vx|1001|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.x8-32 Dx, Dn, Dm"                                   , "a32": "1111|00100|Vx'|sz|Vn|Vx|1001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.x8-32 Vx, Vn, Vm"                                   , "a32": "1111|00100|Vx'|sz|Vn|Vx|1001|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmla.f16 Dx, Dn, Dm<=07[#i]"                             , "t32": "1110|11111|Vx'|01|Vn|Vx|0001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},
    {"inst": "vmla.f16 Vx, Vn, Dm<=07[#i]"                             , "t32": "1111|11111|Vx'|01|Vn|Vx|0001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},
    {"inst": "vmla.f16 Dx, Dn, Dm<=07[#i]"                             , "a32": "1111|00101|Vx'|01|Vn|Vx|0001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},
    {"inst": "vmla.f16 Vx, Vn, Dm<=07[#i]"                             , "a32": "1111|00111|Vx'|01|Vn|Vx|0001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},

    {"inst": "vmla.f32 Dx, Dn, Dm<=15[#i]"                             , "t32": "1110|11111|Vx'|10|Vn|Vx|0001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.f32 Vx, Vn, Dm<=15[#i]"                             , "t32": "1111|11111|Vx'|10|Vn|Vx|0001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.f32 Dx, Dn, Dm<=15[#i]"                             , "a32": "1111|00101|Vx'|10|Vn|Vx|0001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.f32 Vx, Vn, Dm<=15[#i]"                             , "a32": "1111|00111|Vx'|10|Vn|Vx|0001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmla.x16 Dx, Dn, Dm<=07[#i]"                             , "t32": "1110|11111|Vx'|01|Vn|Vx|0000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmla.x16 Vx, Vn, Dm<=07[#i]"                             , "t32": "1111|11111|Vx'|01|Vn|Vx|0000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmla.x16 Dx, Dn, Dm<=07[#i]"                             , "a32": "1111|00101|Vx'|01|Vn|Vx|0000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmla.x16 Vx, Vn, Dm<=07[#i]"                             , "a32": "1111|00111|Vx'|01|Vn|Vx|0000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},

    {"inst": "vmla.x32 Dx, Dn, Dm<=15[#i]"                             , "t32": "1110|11111|Vx'|10|Vn|Vx|0000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.x32 Vx, Vn, Dm<=15[#i]"                             , "t32": "1111|11111|Vx'|10|Vn|Vx|0000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.x32 Dx, Dn, Dm<=15[#i]"                             , "a32": "1111|00101|Vx'|10|Vn|Vx|0000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmla.x32 Vx, Vn, Dm<=15[#i]"                             , "a32": "1111|00111|Vx'|10|Vn|Vx|0000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmlal.x8-32 Vx, Dn, Dm"                                  , "t32": "111U|11111|Vx'|sz|Vn|Vx|1000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmlal.x8-32 Vx, Dn, Dm"                                  , "a32": "1111|001U1|Vx'|sz|Vn|Vx|1000|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmlal.x16 Vx, Dn, Dm<=07[#i]"                            , "t32": "111U|11111|Vx'|01|Vn|Vx|0010|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmlal.x16 Vx, Dn, Dm<=07[#i]"                            , "a32": "1111|001U1|Vx'|01|Vn|Vx|0010|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmlal.x32 Vx, Dn, Dm<=15[#i]"                            , "t32": "111U|11111|Vx'|10|Vn|Vx|0010|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmlal.x32 Vx, Dn, Dm<=15[#i]"                            , "a32": "1111|001U1|Vx'|10|Vn|Vx|0010|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmls.f16-32 Sx, Sn, Sm"                                  , "t32": "1110|11100|'Vx|00|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vmls.f16-32 Sx, Sn, Sm"                                  , "a32": "cond|11100|'Vx|00|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vmls.f64 Dx, Dn, Dm"                                     , "t32": "1110|11100|Vx'|00|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vmls.f64 Dx, Dn, Dm"                                     , "a32": "cond|11100|Vx'|00|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vmls.f16-32 Dx, Dn, Dm"                                  , "t32": "1110|11110|Vx'|1H|Vn|Vx|1101|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmls.f16-32 Vx, Vn, Vm"                                  , "t32": "1110|11110|Vx'|1H|Vn|Vx|1101|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmls.f16-32 Dx, Dn, Dm"                                  , "a32": "1111|00100|Vx'|1H|Vn|Vx|1101|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmls.f16-32 Vx, Vn, Vm"                                  , "a32": "1111|00100|Vx'|1H|Vn|Vx|1101|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmls.x8-32 Dx, Dn, Dm"                                   , "t32": "1111|11110|Vx'|sz|Vn|Vx|1001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.x8-32 Vx, Vn, Vm"                                   , "t32": "1111|11110|Vx'|sz|Vn|Vx|1001|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.x8-32 Dx, Dn, Dm"                                   , "a32": "1111|00110|Vx'|sz|Vn|Vx|1001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.x8-32 Vx, Vn, Vm"                                   , "a32": "1111|00110|Vx'|sz|Vn|Vx|1001|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmls.f16 Dx, Dn, Dm<=07[#i]"                             , "t32": "1110|11111|Vx'|01|Vn|Vx|0101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmls.f16 Vx, Vn, Dm<=07[#i]"                             , "t32": "1111|11111|Vx'|01|Vn|Vx|0101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmls.f16 Dx, Dn, Dm<=07[#i]"                             , "a32": "1111|00101|Vx'|01|Vn|Vx|0101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmls.f16 Vx, Vn, Dm<=07[#i]"                             , "a32": "1111|00111|Vx'|01|Vn|Vx|0101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},

    {"inst": "vmls.f32 Dx, Dn, Dm<=15[#i]"                             , "t32": "1110|11111|Vx'|10|Vn|Vx|0101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.f32 Vx, Vn, Dm<=15[#i]"                             , "t32": "1111|11111|Vx'|10|Vn|Vx|0101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.f32 Dx, Dn, Dm<=15[#i]"                             , "a32": "1111|00101|Vx'|10|Vn|Vx|0101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.f32 Vx, Vn, Dm<=15[#i]"                             , "a32": "1111|00111|Vx'|10|Vn|Vx|0101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmls.x16 Dx, Dn, Dm<=07[#i]"                             , "t32": "1110|11111|Vx'|01|Vn|Vx|0100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmls.x16 Vx, Vn, Dm<=07[#i]"                             , "t32": "1111|11111|Vx'|01|Vn|Vx|0100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmls.x16 Dx, Dn, Dm<=07[#i]"                             , "a32": "1111|00101|Vx'|01|Vn|Vx|0100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmls.x16 Vx, Vn, Dm<=07[#i]"                             , "a32": "1111|00111|Vx'|01|Vn|Vx|0100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},

    {"inst": "vmls.x32 Dx, Dn, Dm<=15[#i]"                             , "t32": "1110|11111|Vx'|10|Vn|Vx|0100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.x32 Vx, Vn, Dm<=15[#i]"                             , "t32": "1111|11111|Vx'|10|Vn|Vx|0100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.x32 Dx, Dn, Dm<=15[#i]"                             , "a32": "1111|00101|Vx'|10|Vn|Vx|0100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmls.x32 Vx, Vn, Dm<=15[#i]"                             , "a32": "1111|00111|Vx'|10|Vn|Vx|0100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmlsl.x8-32 Vx, Dn, Dm"                                  , "t32": "111U|11111|Vx'|sz|Vn|Vx|1010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmlsl.x8-32 Vx, Dn, Dm"                                  , "a32": "1111|001U1|Vx'|sz|Vn|Vx|1010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmlsl.x16 Vx, Dn, Dm<=07[#i]"                            , "t32": "111U|11111|Vx'|01|Vn|Vx|0110|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmlsl.x16 Vx, Dn, Dm<=07[#i]"                            , "a32": "1111|001U1|Vx'|01|Vn|Vx|0110|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmlsl.x32 Vx, Dn, Dm<=15[#i]"                            , "t32": "111U|11111|Vx'|10|Vn|Vx|0110|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmlsl.x32 Vx, Dn, Dm<=15[#i]"                            , "a32": "1111|001U1|Vx'|10|Vn|Vx|0110|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmmla.bf16 Vd, Vn, Vm"                                   , "t32": "1111|11000|Vd'|00|Vn|Vd|1100|Vn'|1|Vm'|0|Vm"            , "ext": "AA32BF16", "it": "out"},
    {"inst": "vmmla.bf16 Vd, Vn, Vm"                                   , "a32": "1111|11000|Vd'|00|Vn|Vd|1100|Vn'|1|Vm'|0|Vm"            , "ext": "AA32BF16"},

    {"inst": "vmov.f16 Rd!=PC, Sn"                                     , "t32": "1110|11100|0|01|Vn|Rd|1001|'Vn|00|1|0000"               , "ext": "FP16FULL"},
    {"inst": "vmov.f16 Sd, Rn!=PC"                                     , "t32": "1110|11100|0|00|Vd|Rn|1001|'Vd|00|1|0000"               , "ext": "FP16FULL"},
    {"inst": "vmov.f16 Rd!=PC, Sn"                                     , "a32": "1110|11100|0|01|Vn|Rd|1001|'Vn|00|1|0000"               , "ext": "FP16FULL"},
    {"inst": "vmov.f16 Sd, Rn!=PC"                                     , "a32": "1110|11100|0|00|Vd|Rn|1001|'Vd|00|1|0000"               , "ext": "FP16FULL"},

    {"inst": "vmov Rd!=PC, Sn"                                         , "t32": "1110|11100|0|01|Vn|Rd|1010|'Vn|00|1|0000"               , "?": "?"},
    {"inst": "vmov Sd, Rn!=PC"                                         , "t32": "1110|11100|0|00|Vd|Rn|1010|'Vd|00|1|0000"               , "?": "?"},
    {"inst": "vmov Rd!=PC, Sn"                                         , "a32": "cond|11100|0|01|Vn|Rd|1010|'Vn|00|1|0000"               , "?": "?"},
    {"inst": "vmov Sd, Rn!=PC"                                         , "a32": "cond|11100|0|00|Vd|Rn|1010|'Vd|00|1|0000"               , "?": "?"},

    {"inst": "vmov Rd!=PC, Rd2!=PC, Ss!=31, Ss2+"                      , "t32": "1110|11000|1|01|Rd2|Rd|101|000|'Vs|1|Vs"                , "?": "?"},
    {"inst": "vmov Sd!=31, Sd2+, Rs!=PC, Rs2!=PC"                      , "t32": "1110|11000|1|00|Rs2|Rs|101|000|'Vd|1|Vd"                , "?": "?"},
    {"inst": "vmov Rd!=PC, Rd2!=PC, Ss!=31, Ss2+"                      , "a32": "cond|11000|1|01|Rd2|Rd|101|000|'Vs|1|Vs"                , "?": "?"},
    {"inst": "vmov Sd!=31, Sd2+, Rs!=PC, Rs2!=PC"                      , "a32": "cond|11000|1|00|Rs2|Rs|101|000|'Vd|1|Vd"                , "?": "?"},

    {"inst": "vmov Rd!=PC, Rd2!=PC, Ds"                                , "t32": "1110|11000|1|01|Rd2|Rd|1011|0|0|Vs'|1|Vs"               , "?": "?"},
    {"inst": "vmov Dd, Rs!=PC, Rs2!=PC"                                , "t32": "1110|11000|1|00|Rs2|Rs|1011|0|0|Vd'|1|Vd"               , "?": "?"},
    {"inst": "vmov Rd!=PC, Rd2!=PC, Ds"                                , "a32": "cond|11000|1|01|Rd2|Rd|1011|0|0|Vs'|1|Vs"               , "?": "?"},
    {"inst": "vmov Dd, Rs!=PC, Rs2!=PC"                                , "a32": "cond|11000|1|00|Rs2|Rs|1011|0|0|Vd'|1|Vd"               , "?": "?"},

    {"inst": "vmov.f32 Sd, Sn"                                         , "t32": "1110|11101|'Vd|11|0000|Vd|1010|0|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vmov.f64 Dd, Dn"                                         , "t32": "1110|11101|Vd'|11|0000|Vd|1011|0|1|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vmov.f32 Sd, Sn"                                         , "a32": "cond|11101|'Vd|11|0000|Vd|1010|0|1|'Vn|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vmov.f64 Dd, Dn"                                         , "a32": "cond|11101|Vd'|11|0000|Vd|1011|0|1|Vn'|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vmov.x8 Dd[#i], Rn!=PC"                                  , "t32": "1110|11100|1|i:1|0|Vd|Rn|1011|Vd'|i:2|1|0000"           , "?": "?"},
    {"inst": "vmov.x16 Dd[#i], Rn!=PC"                                 , "t32": "1110|11100|0|i:1|0|Vd|Rn|1011|Vd'|i:1|110000"           , "?": "?"},
    {"inst": "vmov.x32 Dd[#i], Rn!=PC"                                 , "t32": "1110|11100|0|i:1|0|Vd|Rn|1011|Vd'|001|0000"             , "?": "?"},
    {"inst": "vmov.x8 Dd[#i], Rn!=PC"                                  , "a32": "cond|11100|1|i:1|0|Vd|Rn|1011|Vd'|i:2|1|0000"           , "?": "?"},
    {"inst": "vmov.x16 Dd[#i], Rn!=PC"                                 , "a32": "cond|11100|0|i:1|0|Vd|Rn|1011|Vd'|i:1|110000"           , "?": "?"},
    {"inst": "vmov.x32 Dd[#i], Rn!=PC"                                 , "a32": "cond|11100|0|i:1|0|Vd|Rn|1011|Vd'|001|0000"             , "?": "?"},

    {"inst": "vmov.x8 Rd!=PC, Dn[#i]"                                  , "t32": "1110|1110U|1|i:1|1|Vn|Rd|1011|Vn'|i:2|1|0000"           , "?": "?"},
    {"inst": "vmov.x16 Rd!=PC, Dn[#i]"                                 , "t32": "1110|1110U|0|i:1|1|Vn|Rd|1011|Vn'|i:1|110000"           , "?": "?"},
    {"inst": "vmov.x32 Rd!=PC, Dn[#i]"                                 , "t32": "1110|11100|0|i:1|1|Vn|Rd|1011|Vn'|001|0000"             , "?": "?"},
    {"inst": "vmov.x8 Rd!=PC, Dn[#i]"                                  , "a32": "cond|1110U|1|i:1|1|Vn|Rd|1011|Vn'|i:2|1|0000"           , "?": "?"},
    {"inst": "vmov.x16 Rd!=PC, Dn[#i]"                                 , "a32": "cond|1110U|0|i:1|1|Vn|Rd|1011|Vn'|i:1|110000"           , "?": "?"},
    {"inst": "vmov.x32 Rd!=PC, Dn[#i]"                                 , "a32": "cond|11100|0|i:1|1|Vn|Rd|1011|Vn'|001|0000"             , "?": "?"},

    {"inst": "vmov.any Dd, Dn"                                         , "t32": "1110|11110|Vd'|10|Vn[3:0]|Vd|0001|Vn[4]|0|Vn[4]|1|Vn[3:0]", "ext": "ASIMD", "aliasOf": "vorr"},
    {"inst": "vmov.any Vd, Vn"                                         , "t32": "1110|11110|Vd'|10|Vn[3:0]|Vd|0001|Vn[4]|1|Vn[4]|1|Vn[3:0]", "ext": "ASIMD", "aliasOf": "vorr"},
    {"inst": "vmov.any Dd, Dn"                                         , "a32": "1111|00100|Vd'|10|Vn[3:0]|Vd|0001|Vn[4]|0|Vn[4]|1|Vn[3:0]", "ext": "ASIMD", "aliasOf": "vorr"},
    {"inst": "vmov.any Vd, Vn"                                         , "a32": "1111|00100|Vd'|10|Vn[3:0]|Vd|0001|Vn[4]|1|Vn[4]|1|Vn[3:0]", "ext": "ASIMD", "aliasOf": "vorr"},

    {"inst": "vmov.f16 Sd, #immVFP"                                    , "t32": "1110|11101|'Vd|11|imm:4|Vd|1001|0000|imm:4"             , "ext": "FP16FULL" , "imm": "VecVFPImm(immVFP)", "it": "out"},
    {"inst": "vmov.f32 Sd, #immVFP"                                    , "t32": "1110|11101|'Vd|11|imm:4|Vd|1010|0000|imm:4"             , "ext": "VFPv3"    , "imm": "VecVFPImm(immVFP)"},
    {"inst": "vmov.f64 Dd, #immVFP"                                    , "t32": "1110|11101|Vd'|11|imm:4|Vd|1011|0000|imm:4"             , "ext": "VFPv3"    , "imm": "VecVFPImm(immVFP)"},
    {"inst": "vmov.f16 Sd, #immVFP"                                    , "a32": "cond|11101|'Vd|11|imm:4|Vd|1001|0000|imm:4"             , "ext": "FP16FULL" , "imm": "VecVFPImm(immVFP)"},
    {"inst": "vmov.f32 Sd, #immVFP"                                    , "a32": "cond|11101|'Vd|11|imm:4|Vd|1010|0000|imm:4"             , "ext": "VFPv3"    , "imm": "VecVFPImm(immVFP)"},
    {"inst": "vmov.f64 Dd, #immVFP"                                    , "a32": "cond|11101|Vd'|11|imm:4|Vd|1011|0000|imm:4"             , "ext": "VFPv3"    , "imm": "VecVFPImm(immVFP)"},

    {"inst": "vmov.x8-64 Dd, #immV"                                    , "t32": "111|imm:1|11111|Vd'|000|imm:3|Vd|cmode:4|00|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 0, immV)"},
    {"inst": "vmov.x8-64 Vd, #immV"                                    , "t32": "111|imm:1|11111|Vd'|000|imm:3|Vd|cmode:4|01|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 0, immV)"},
    {"inst": "vmov.x8-64 Dd, #immV"                                    , "a32": "1111001|imm:1|1|Vd'|000|imm:3|Vd|cmode:4|00|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 0, immV)"},
    {"inst": "vmov.x8-64 Vd, #immV"                                    , "a32": "1111001|imm:1|1|Vd'|000|imm:3|Vd|cmode:4|01|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 0, immV)"},

    {"inst": "vmovl.x8 Vd, Dn"                                         , "t32": "111U|11111|Vd'|00|1000|Vd|1010|0|0|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmovl.x16 Vd, Dn"                                        , "t32": "111U|11111|Vd'|01|0000|Vd|1010|0|0|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmovl.x32 Vd, Dn"                                        , "t32": "111U|11111|Vd'|10|0000|Vd|1010|0|0|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmovl.x8 Vd, Dn"                                         , "a32": "1111|001U1|Vd'|00|1000|Vd|1010|0|0|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmovl.x16 Vd, Dn"                                        , "a32": "1111|001U1|Vd'|01|0000|Vd|1010|0|0|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmovl.x32 Vd, Dn"                                        , "a32": "1111|001U1|Vd'|10|0000|Vd|1010|0|0|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmovn.x16-64 Dd, Vn"                                     , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|00|Vn'|0|Vn"        , "ext": "ASIMD", "vecOp": "narrow"},
    {"inst": "vmovn.x16-64 Dd, Vn"                                     , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|00|Vn'|0|Vn"        , "ext": "ASIMD", "vecOp": "narrow"},

    {"inst": "vmovx.f16 Sd, Sn"                                        , "t32": "1111|11101|'Vd|11|00|00|Vd|1010|01|'Vn|0|Vn"            , "ext": "FP16FULL", "it": "out"},
    {"inst": "vmovx.f16 Sd, Sn"                                        , "a32": "1111|11101|'Vd|11|00|00|Vd|1010|01|'Vn|0|Vn"            , "ext": "FP16FULL"},

    {"inst": "vmul.f16-32 Sd, Sn, Sm"                                  , "t32": "1110|11100|'Vd|10|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vmul.f16-32 Sd, Sn, Sm"                                  , "a32": "cond|11100|'Vd|10|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},
    {"inst": "vmul.f64 Dd, Dn, Dm"                                     , "t32": "1110|11100|Vd'|10|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vmul.f64 Dd, Dn, Dm"                                     , "a32": "cond|11100|Vd'|10|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vmul.f16-32 Dd, Dn, Dm"                                  , "t32": "1111|11110|Vd'|0H|Vn|Vd|1101|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmul.f16-32 Vd, Vn, Vm"                                  , "t32": "1111|11110|Vd'|0H|Vn|Vd|1101|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmul.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00110|Vd'|0H|Vn|Vd|1101|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vmul.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00110|Vd'|0H|Vn|Vd|1101|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vmul.x8-32|p8 Dd, Dn, Dm"                                , "t32": "111P|11110|Vd'|sz|Vn|Vd|1001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.x8-32|p8 Vd, Vn, Vm"                                , "t32": "111P|11110|Vd'|sz|Vn|Vd|1001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.x8-32|p8 Dd, Dn, Dm"                                , "a32": "1111|001P0|Vd'|sz|Vn|Vd|1001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.x8-32|p8 Vd, Vn, Vm"                                , "a32": "1111|001P0|Vd'|sz|Vn|Vd|1001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vmul.f16 Dd, Dn, Dm<=07[#i]"                             , "t32": "1110|11111|Vd'|01|Vn|Vd|1001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},
    {"inst": "vmul.f16 Vd, Vn, Dm<=07[#i]"                             , "t32": "1111|11111|Vd'|01|Vn|Vd|1001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},
    {"inst": "vmul.f16 Dd, Dn, Dm<=07[#i]"                             , "a32": "1111|00101|Vd'|01|Vn|Vd|1001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},
    {"inst": "vmul.f16 Vd, Vn, Dm<=07[#i]"                             , "a32": "1111|00111|Vd'|01|Vn|Vd|1001|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "FP16FULL"},

    {"inst": "vmul.f32 Dd, Dn, Dm<=15[#i]"                             , "t32": "1110|11111|Vd'|10|Vn|Vd|1001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.f32 Vd, Vn, Dm<=15[#i]"                             , "t32": "1111|11111|Vd'|10|Vn|Vd|1001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.f32 Dd, Dn, Dm<=15[#i]"                             , "a32": "1111|00101|Vd'|10|Vn|Vd|1001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.f32 Vd, Vn, Dm<=15[#i]"                             , "a32": "1111|00111|Vd'|10|Vn|Vd|1001|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmul.x16 Dd, Dn, Dm<=07[#i]"                             , "t32": "1110|11111|Vd'|01|Vn|Vd|1000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmul.x16 Vd, Vn, Dm<=07[#i]"                             , "t32": "1111|11111|Vd'|01|Vn|Vd|1000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmul.x16 Dd, Dn, Dm<=07[#i]"                             , "a32": "1111|00101|Vd'|01|Vn|Vd|1000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},
    {"inst": "vmul.x16 Vd, Vn, Dm<=07[#i]"                             , "a32": "1111|00111|Vd'|01|Vn|Vd|1000|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD"},

    {"inst": "vmul.x32 Dd, Dn, Dm<=15[#i]"                             , "t32": "1110|11111|Vd'|10|Vn|Vd|1000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.x32 Vd, Vn, Dm<=15[#i]"                             , "t32": "1111|11111|Vd'|10|Vn|Vd|1000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.x32 Dd, Dn, Dm<=15[#i]"                             , "a32": "1111|00101|Vd'|10|Vn|Vd|1000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vmul.x32 Vd, Vn, Dm<=15[#i]"                             , "a32": "1111|00111|Vd'|10|Vn|Vd|1000|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vmull.x8-32|p8 Vd, Dn, Dm"                               , "t32": "111U|11111|Vd'|sz|Vn|Vd|11P0|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmull.x8-32|p8 Vd, Dn, Dm"                               , "a32": "1111|001U1|Vd'|sz|Vn|Vd|11P0|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmull.p64 Vd, Dn, Dm"                                    , "t32": "1110|11111|Vd'|10|Vn|Vd|11P0|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmull.p64 Vd, Dn, Dm"                                    , "a32": "1111|00101|Vd'|10|Vn|Vd|11P0|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmull.x16 Vd, Dn, Dm<=07[#i]"                            , "t32": "111U|11111|Vd'|01|Vn|Vd|1010|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmull.x16 Vd, Dn, Dm<=07[#i]"                            , "a32": "1111|001U1|Vd'|01|Vn|Vd|1010|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmull.x32 Vd, Dn, Dm<=15[#i]"                            , "t32": "111U|11111|Vd'|10|Vn|Vd|1010|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vmull.x32 Vd, Dn, Dm<=15[#i]"                            , "a32": "1111|001U1|Vd'|10|Vn|Vd|1010|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vmvn.any Dd, #immV"                                      , "t32": "111|imm:1|11111|Vd'|000|imm:3|Vd|cmode:4|00|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 1, immV)"},
    {"inst": "vmvn.any Vd, #immV"                                      , "t32": "111|imm:1|11111|Vd'|000|imm:3|Vd|cmode:4|01|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 1, immV)"},
    {"inst": "vmvn.any Dd, #immV"                                      , "a32": "1111001|imm:1|1|Vd'|000|imm:3|Vd|cmode:4|00|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 1, immV)"},
    {"inst": "vmvn.any Vd, #immV"                                      , "a32": "1111001|imm:1|1|Vd'|000|imm:3|Vd|cmode:4|01|op|1|imm:4" , "ext": "ASIMD", "imm": "VecMovImm(sz, 1, immV)"},

    {"inst": "vmvn.any Dd, Dn"                                         , "t32": "1111|11111|Vd'|11|0000|Vd|0101|10|Vn'|0|Vn"             , "ext": "ASIMD"},
    {"inst": "vmvn.any Vd, Vn"                                         , "t32": "1111|11111|Vd'|11|0000|Vd|0101|11|Vn'|0|Vn"             , "ext": "ASIMD"},
    {"inst": "vmvn.any Dd, Dn"                                         , "a32": "1111|00111|Vd'|11|0000|Vd|0101|10|Vn'|0|Vn"             , "ext": "ASIMD"},
    {"inst": "vmvn.any Vd, Vn"                                         , "a32": "1111|00111|Vd'|11|0000|Vd|0101|11|Vn'|0|Vn"             , "ext": "ASIMD"},

    {"inst": "vneg.f16-32 Sd, Sn"                                      , "t32": "1110|11101|'Vd|11|0001|Vd|10|sz|01|'Vn|0|Vn"            , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vneg.f16-32 Sd, Sn"                                      , "a32": "1110|11101|'Vd|11|0001|Vd|10|sz|01|'Vn|0|Vn"            , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vneg.f64 Dd, Dn"                                         , "t32": "1110|11101|Vd'|11|0001|Vd|10|11|01|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vneg.f64 Dd, Dn"                                         , "a32": "cond|11101|Vd'|11|0001|Vd|10|11|01|Vn'|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vneg.f16-32 Dd, Dn"                                      , "t32": "1111|11111|Vd'|11|sz|01|Vd|0111|10|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vneg.f16-32 Vd, Vn"                                      , "t32": "1111|11111|Vd'|11|sz|01|Vd|0111|11|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vneg.f16-32 Dd, Dn"                                      , "a32": "1111|00111|Vd'|11|sz|01|Vd|0111|10|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vneg.f16-32 Vd, Vn"                                      , "a32": "1111|00111|Vd'|11|sz|01|Vd|0111|11|Vn'|0|Vn"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vneg.s8-32 Dd, Dn"                                       , "t32": "1111|11111|Vd'|11|sz|01|Vd|0011|10|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vneg.s8-32 Vd, Vn"                                       , "t32": "1111|11111|Vd'|11|sz|01|Vd|0011|11|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vneg.s8-32 Dd, Dn"                                       , "a32": "1111|00111|Vd'|11|sz|01|Vd|0011|10|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vneg.s8-32 Vd, Vn"                                       , "a32": "1111|00111|Vd'|11|sz|01|Vd|0011|11|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vnmla.f16-32 Sx, Sn, Sm"                                 , "t32": "1110|11100|'Vx|01|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vnmla.f16-32 Sx, Sn, Sm"                                 , "a32": "cond|11100|'Vx|01|Vn|Vx|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vnmla.f64 Dx, Dn, Dm"                                    , "t32": "1110|11100|Vx'|01|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vnmla.f64 Dx, Dn, Dm"                                    , "a32": "cond|11100|Vx'|01|Vn|Vx|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vnmls.f16-32 Sx, Sn, Sm"                                 , "t32": "1110|11100|'Vx|01|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vnmls.f16-32 Sx, Sn, Sm"                                 , "a32": "cond|11100|'Vx|01|Vn|Vx|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vnmls.f64 Dx, Dn, Dm"                                    , "t32": "1110|11100|Vx'|01|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vnmls.f64 Dx, Dn, Dm"                                    , "a32": "cond|11100|Vx'|01|Vn|Vx|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vnmul.f16-32 Sd, Sn, Sm"                                 , "t32": "1110|11100|'Vd|10|Vn|Vd|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vnmul.f16-32 Sd, Sn, Sm"                                 , "a32": "cond|11100|'Vd|10|Vn|Vd|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vnmul.f64 Dd, Dn, Dm"                                    , "t32": "1110|11100|Vd'|10|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vnmul.f64 Dd, Dn, Dm"                                    , "a32": "cond|11100|Vd'|10|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vorn.x16-64 Dx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0001|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vorr"},
    {"inst": "vorn.x16-64 Vx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0101|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vorr"},
    {"inst": "vorn.x16-64 Dx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0001|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vorr"},
    {"inst": "vorn.x16-64 Vx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0101|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 1, immV)", "pseudoOf": "vorr"},

    {"inst": "vorn.any Dd, Dn, Dm"                                     , "t32": "1110|11110|Vd'|11|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vorn.any Vd, Vn, Vm"                                     , "t32": "1110|11110|Vd'|11|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vorn.any Dd, Dn, Dm"                                     , "a32": "1111|00100|Vd'|11|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vorn.any Vd, Vn, Vm"                                     , "a32": "1111|00100|Vd'|11|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vorr.x16-64 Dx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0001|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},
    {"inst": "vorr.x16-64 Vx, #immV"                                   , "t32": "111|imm:1|11111|Vx'|000|imm:3|Vx|cmode:4|0101|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},
    {"inst": "vorr.x16-64 Dx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0001|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},
    {"inst": "vorr.x16-64 Vx, #immV"                                   , "a32": "1111001|imm:1|1|Vx'|000|imm:3|Vx|cmode:4|0101|imm:4"    , "ext": "ASIMD", "imm": "VecBicOrrImm(sz, 0, immV)"},

    {"inst": "vorr.any Dd, Dn, Dm"                                     , "t32": "1110|11110|Vd'|10|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vorr.any Vd, Vn, Vm"                                     , "t32": "1110|11110|Vd'|10|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vorr.any Dd, Dn, Dm"                                     , "a32": "1111|00100|Vd'|10|Vn|Vd|0001|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vorr.any Vd, Vn, Vm"                                     , "a32": "1111|00100|Vd'|10|Vn|Vd|0001|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vpadal.x8-32 Dx, Dn"                                     , "t32": "1111|11111|Vx'|11|sz|00|Vx|0110|U|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vpadal.x8-32 Vx, Vn"                                     , "t32": "1111|11111|Vx'|11|sz|00|Vx|0110|U|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vpadal.x8-32 Dx, Dn"                                     , "a32": "1111|00111|Vx'|11|sz|00|Vx|0110|U|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vpadal.x8-32 Vx, Vn"                                     , "a32": "1111|00111|Vx'|11|sz|00|Vx|0110|U|1|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vpadd.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|0H|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vpadd.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|0H|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vpaddl.x8-32 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|sz|00|Vd|0010|U|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vpaddl.x8-32 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|sz|00|Vd|0010|U|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vpaddl.x8-32 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|sz|00|Vd|0010|U|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vpaddl.x8-32 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|sz|00|Vd|0010|U|1|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vpmax.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vpmax.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vpmax.x8-32 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|1010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vpmax.x8-32 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|1010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vpmin.f16-32 Dd, Dn, Dm"                                 , "t32": "1111|11110|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vpmin.f16-32 Dd, Dn, Dm"                                 , "a32": "1111|00110|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vpmin.x8-32 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|1010|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vpmin.x8-32 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|1010|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vqabs.s8-32 Dd, Dn"                                      , "t32": "1111|11111|Vd'|11|sz|00|Vd|0111|0|0|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqabs.s8-32 Vd, Vn"                                      , "t32": "1111|11111|Vd'|11|sz|00|Vd|0111|0|1|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqabs.s8-32 Dd, Dn"                                      , "a32": "1111|00111|Vd'|11|sz|00|Vd|0111|0|0|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqabs.s8-32 Vd, Vn"                                      , "a32": "1111|00111|Vd'|11|sz|00|Vd|0111|0|1|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqadd.x8-64 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0000|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqadd.x8-64 Vd, Vn, Vm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0000|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqadd.x8-64 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0000|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqadd.x8-64 Vd, Vn, Vm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0000|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqdmlal.s16-32 Vx, Dn, Dm"                               , "t32": "1110|11111|Vx'|sz|Vn|Vx|1001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlal.s16-32 Vx, Dn, Dm"                               , "a32": "1111|00101|Vx'|sz|Vn|Vx|1001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqdmlal.s16 Vx, Dn, Dm<=07[#i]"                          , "t32": "1110|11111|Vx'|01|Vn|Vx|0011|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlal.s32 Vx, Dn, Dm<=15[#i]"                          , "t32": "1110|11111|Vx'|10|Vn|Vx|0011|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlal.s16 Vx, Dn, Dm<=07[#i]"                          , "a32": "1111|00101|Vx'|01|Vn|Vx|0011|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlal.s32 Vx, Dn, Dm<=15[#i]"                          , "a32": "1111|00101|Vx'|10|Vn|Vx|0011|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqdmlsl.s16-32 Vx, Dn, Dm"                               , "t32": "1110|11111|Vx'|sz|Vn|Vx|1011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlsl.s16-32 Vx, Dn, Dm"                               , "a32": "1111|00101|Vx'|sz|Vn|Vx|1011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqdmlsl.s16 Vx, Dn, Dm<=07[#i]"                          , "t32": "1110|11111|Vx'|01|Vn|Vx|0111|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlsl.s32 Vx, Dn, Dm<=15[#i]"                          , "t32": "1110|11111|Vx'|10|Vn|Vx|0111|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlsl.s16 Vx, Dn, Dm<=07[#i]"                          , "a32": "1111|00101|Vx'|01|Vn|Vx|0111|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmlsl.s32 Vx, Dn, Dm<=15[#i]"                          , "a32": "1111|00101|Vx'|10|Vn|Vx|0111|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqdmulh.s16-32 Dd, Dn, Dm"                               , "t32": "1110|11110|Vd'|sz|Vn|Vd|1011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s16-32 Vd, Vn, Vm"                               , "t32": "1110|11110|Vd'|sz|Vn|Vd|1011|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s16-32 Dd, Dn, Dm"                               , "a32": "1111|00100|Vd'|sz|Vn|Vd|1011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s16-32 Vd, Vn, Vm"                               , "a32": "1111|00100|Vd'|sz|Vn|Vd|1011|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqdmulh.s16 Dd, Dn, Dm<=07[#i]"                          , "t32": "1110|11111|Vd'|01|Vn|Vd|1100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s16 Dd, Dn, Dm<=07[#i]"                          , "a32": "1111|00101|Vd'|01|Vn|Vd|1100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s16 Vd, Vn, Dm<=07[#i]"                          , "t32": "1111|11111|Vd'|01|Vn|Vd|1100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s16 Vd, Vn, Dm<=07[#i]"                          , "a32": "1111|00111|Vd'|01|Vn|Vd|1100|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqdmulh.s32 Dd, Dn, Dm<=15[#i]"                          , "t32": "1110|11111|Vd'|10|Vn|Vd|1100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s32 Vd, Vn, Dm<=15[#i]"                          , "t32": "1111|11111|Vd'|10|Vn|Vd|1100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s32 Dd, Dn, Dm<=15[#i]"                          , "a32": "1111|00101|Vd'|10|Vn|Vd|1100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqdmulh.s32 Vd, Vn, Dm<=15[#i]"                          , "a32": "1111|00111|Vd'|10|Vn|Vd|1100|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqdmull.s16-32 Vd, Dn, Dm"                               , "t32": "1110|11111|Vd'|sz|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmull.s16-32 Vd, Dn, Dm"                               , "a32": "1111|00101|Vd'|sz|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqdmull.s16 Vd, Dn, Dm<=07[#i]"                          , "t32": "1110|11111|Vd'|01|Vn|Vd|1011|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmull.s16 Vd, Dn, Dm<=07[#i]"                          , "a32": "1111|00101|Vd'|01|Vn|Vd|1011|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqdmull.s32 Vd, Dn, Dm<=15[#i]"                          , "t32": "1110|11111|Vd'|10|Vn|Vd|1011|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},
    {"inst": "vqdmull.s32 Vd, Dn, Dm<=15[#i]"                          , "a32": "1111|00101|Vd'|10|Vn|Vd|1011|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "widen"},

    {"inst": "vqmovn.x16-64 Dd, Vn"                                    , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|1|U|Vn'|0|Vn"       , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},
    {"inst": "vqmovn.x16-64 Dd, Vn"                                    , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|1|U|Vn'|0|Vn"       , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},

    {"inst": "vqmovun.s16-64 Dd, Vn"                                   , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|0|1|Vn'|0|Vn"       , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},
    {"inst": "vqmovun.s16-64 Dd, Vn"                                   , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|0|1|Vn'|0|Vn"       , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},

    {"inst": "vqneg.s8-32 Dd, Dn"                                      , "t32": "1111|11111|Vd'|11|sz|00|Vd|0111|1|0|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqneg.s8-32 Vd, Vn"                                      , "t32": "1111|11111|Vd'|11|sz|00|Vd|0111|1|1|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqneg.s8-32 Dd, Dn"                                      , "a32": "1111|00111|Vd'|11|sz|00|Vd|0111|1|0|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqneg.s8-32 Vd, Vn"                                      , "a32": "1111|00111|Vd'|11|sz|00|Vd|0111|1|1|Vn'|0|Vn"           , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqrdmlah.s16-32 Dd, Dn, Dm"                              , "t32": "1111|11110|Vd'|sz|Vn|Vd|1011|Vn'|0|Vm'|1|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s16-32 Vd, Vn, Vm"                              , "t32": "1111|11110|Vd'|sz|Vn|Vd|1011|Vn'|1|Vm'|1|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s16-32 Dd, Dn, Dm"                              , "a32": "1111|00110|Vd'|sz|Vn|Vd|1011|Vn'|0|Vm'|1|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s16-32 Vd, Vn, Vm"                              , "a32": "1111|00110|Vd'|sz|Vn|Vd|1011|Vn'|1|Vm'|1|Vm"            , "ext": "QRDMLAH"},

    {"inst": "vqrdmlah.s16 Dd, Dn, Dm<=07[#i]"                         , "t32": "1110|11111|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s16 Vd, Vn, Dm<=07[#i]"                         , "t32": "1111|11111|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s16 Dd, Dn, Dm<=07[#i]"                         , "a32": "1111|00101|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s16 Vd, Vn, Dm<=07[#i]"                         , "a32": "1111|00111|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},

    {"inst": "vqrdmlah.s32 Dd, Dn, Dm<=15[#i]"                         , "t32": "1110|11111|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s32 Vd, Vn, Dm<=15[#i]"                         , "t32": "1111|11111|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s32 Dd, Dn, Dm<=15[#i]"                         , "a32": "1111|00101|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlah.s32 Vd, Vn, Dm<=15[#i]"                         , "a32": "1111|00111|Vd'|sz|Vn|Vd|1110|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},

    {"inst": "vqrdmlsh.s16-32 Dd, Dn, Dm"                              , "t32": "1111|11110|Vd'|sz|Vn|Vd|1100|Vn'|0|Vm'|1|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s16-32 Vd, Vn, Vm"                              , "t32": "1111|11110|Vd'|sz|Vn|Vd|1100|Vn'|1|Vm'|1|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s16-32 Dd, Dn, Dm"                              , "a32": "1111|00110|Vd'|sz|Vn|Vd|1100|Vn'|0|Vm'|1|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s16-32 Vd, Vn, Vm"                              , "a32": "1111|00110|Vd'|sz|Vn|Vd|1100|Vn'|1|Vm'|1|Vm"            , "ext": "QRDMLAH"},

    {"inst": "vqrdmlsh.s16 Dd, Dn, Dm<=07[#i]"                         , "t32": "1110|11111|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s16 Vd, Vn, Dm<=07[#i]"                         , "t32": "1111|11111|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s16 Dd, Dn, Dm<=07[#i]"                         , "a32": "1111|00101|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s16 Vd, Vn, Dm<=07[#i]"                         , "a32": "1111|00111|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "QRDMLAH"},

    {"inst": "vqrdmlsh.s32 Dd, Dn, Dm<=15[#i]"                         , "t32": "1110|11111|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s32 Vd, Vn, Dm<=15[#i]"                         , "t32": "1111|11111|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s32 Dd, Dn, Dm<=15[#i]"                         , "a32": "1111|00101|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},
    {"inst": "vqrdmlsh.s32 Vd, Vn, Dm<=15[#i]"                         , "a32": "1111|00111|Vd'|sz|Vn|Vd|1111|Vn'|1|i:1|0|Vm"            , "ext": "QRDMLAH"},

    {"inst": "vqrdmulh.s16-32 Dd, Dn, Dm"                              , "t32": "1111|11110|Vd'|sz|Vn|Vd|1011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s16-32 Vd, Vn, Vm"                              , "t32": "1111|11110|Vd'|sz|Vn|Vd|1011|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s16-32 Dd, Dn, Dm"                              , "a32": "1111|00110|Vd'|sz|Vn|Vd|1011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s16-32 Vd, Vn, Vm"                              , "a32": "1111|00110|Vd'|sz|Vn|Vd|1011|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqrdmulh.s16 Dd, Dn, Dm<=07[#i]"                         , "t32": "1110|11111|Vd'|01|Vn|Vd|1101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s16 Vd, Vn, Dm<=07[#i]"                         , "t32": "1111|11111|Vd'|01|Vn|Vd|1101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s16 Dd, Dn, Dm<=07[#i]"                         , "a32": "1111|00101|Vd'|01|Vn|Vd|1101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s16 Vd, Vn, Dm<=07[#i]"                         , "a32": "1111|00111|Vd'|01|Vn|Vd|1101|Vn'|1|i:1|0|i:1|Vm:3"      , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqrdmulh.s32 Dd, Dn, Dm<=15[#i]"                         , "t32": "1110|11111|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s32 Vd, Vn, Dm<=15[#i]"                         , "t32": "1111|11111|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s32 Dd, Dn, Dm<=15[#i]"                         , "a32": "1111|00101|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrdmulh.s32 Vd, Vn, Dm<=15[#i]"                         , "a32": "1111|00111|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqrshl.x8-64 Dd, Dn, Dm"                                 , "t32": "111U|11110|Vd'|sz|Vm|Vd|0101|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrshl.x8-64 Vd, Vn, Vm"                                 , "t32": "111U|11110|Vd'|sz|Vm|Vd|0101|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrshl.x8-64 Dd, Dn, Dm"                                 , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0101|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqrshl.x8-64 Vd, Vn, Vm"                                 , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0101|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqrshrn.x16-64 Dd, Vn, #zero"                            , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|1U|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},
    {"inst": "vqrshrn.x16-64 Dd, Vn, #zero"                            , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|1U|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},

    {"inst": "vqrshrn.x16-64 Dd, Vn, #n"                               , "t32": "111U|11111|Vd'|imm:6|Vd|1001|01|Vn'|1|Vn"               , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},
    {"inst": "vqrshrn.x16-64 Dd, Vn, #n"                               , "a32": "1111|001U1|Vd'|imm:6|Vd|1001|01|Vn'|1|Vn"               , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},

    {"inst": "vqrshrun.s16-64 Dd, Vn, #zero"                           , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|01|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},
    {"inst": "vqrshrun.s16-64 Dd, Vn, #zero"                           , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|01|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},

    {"inst": "vqrshrun.s16-64 Dd, Vn, #n"                              , "t32": "1111|11111|Vd'|imm:6|Vd|1000|01|Vn'|1|Vn"               , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},
    {"inst": "vqrshrun.s16-64 Dd, Vn, #n"                              , "a32": "1111|00111|Vd'|imm:6|Vd|1000|01|Vn'|1|Vn"               , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},

    {"inst": "vqshl.x8-64 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vm|Vd|0100|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqshl.x8-64 Vd, Vn, Vm"                                  , "t32": "111U|11110|Vd'|sz|Vm|Vd|0100|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqshl.x8-64 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0100|Vm'|0|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqshl.x8-64 Vd, Vn, Vm"                                  , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0100|Vm'|1|Vn'|1|Vn"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vqshl.x8-64 Dd, Dn, #n"                                  , "t32": "111U|11111|Vd'|imm[5:0]|Vd|0111|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vqshl.x8-64 Vd, Vn, #n"                                  , "t32": "111U|11111|Vd'|imm[5:0]|Vd|0111|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vqshl.x8-64 Dd, Dn, #n"                                  , "a32": "1111|001U1|Vd'|imm[5:0]|Vd|0111|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vqshl.x8-64 Vd, Vn, #n"                                  , "a32": "1111|001U1|Vd'|imm[5:0]|Vd|0111|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},

    {"inst": "vqshlu.s8-64 Dd, Dn, #n"                                 , "t32": "1111|11111|Vd'|imm[5:0]|Vd|0110|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vqshlu.s8-64 Vd, Vn, #n"                                 , "t32": "1111|11111|Vd'|imm[5:0]|Vd|0110|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vqshlu.s8-64 Dd, Dn, #n"                                 , "a32": "1111|00111|Vd'|imm[5:0]|Vd|0110|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vqshlu.s8-64 Vd, Vn, #n"                                 , "a32": "1111|00111|Vd'|imm[5:0]|Vd|0110|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftPImm(sz, n)"},

    {"inst": "vqshrn.x16-64 Dd, Vn, #zero"                             , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|1U|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},
    {"inst": "vqshrn.x16-64 Dd, Vn, #zero"                             , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|1U|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},

    {"inst": "vqshrn.x16-64 Dd, Vn, #n"                                , "t32": "111U|11111|Vd'|imm:6|Vd|1001|0|0|Vn'|1|Vn"              , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},
    {"inst": "vqshrn.x16-64 Dd, Vn, #n"                                , "a32": "1111|001U1|Vd'|imm:6|Vd|1001|0|0|Vn'|1|Vn"              , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},

    {"inst": "vqshrun.s16-64 Dd, Vn, #zero"                            , "t32": "1111|11111|Vd'|11|szM1:2|10|Vd|0010|01|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},
    {"inst": "vqshrun.s16-64 Dd, Vn, #zero"                            , "a32": "1111|00111|Vd'|11|szM1:2|10|Vd|0010|01|Vn'|0|Vn"        , "ext": "ASIMD", "fpcsr": "Q=X", "vecOp": "narrow"},

    {"inst": "vqshrun.s16-64 Dd, Vn, #n"                               , "t32": "1111|11111|Vd'|imm:6|Vd|1000|0|0|Vn'|1|Vn"              , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},
    {"inst": "vqshrun.s16-64 Dd, Vn, #n"                               , "a32": "1111|00111|Vd'|imm:6|Vd|1000|0|0|Vn'|1|Vn"              , "ext": "ASIMD", "fpcsr": "Q=X", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},

    {"inst": "vqsub.x8-64 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0010|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqsub.x8-64 Vd, Vn, Vm"                                  , "t32": "111U|11110|Vd'|sz|Vn|Vd|0010|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqsub.x8-64 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0010|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},
    {"inst": "vqsub.x8-64 Vd, Vn, Vm"                                  , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0010|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD", "fpcsr": "Q=X"},

    {"inst": "vraddhn.x16-64 Dd, Vn, Vm"                               , "t32": "1111|11111|Vd'|szM1:2|Vn|Vd|0100|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},
    {"inst": "vraddhn.x16-64 Dd, Vn, Vm"                               , "a32": "1111|00111|Vd'|szM1:2|Vn|Vd|0100|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},

    {"inst": "vrecpe.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|11|Vd|0101|0|0|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrecpe.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|11|Vd|0101|0|1|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrecpe.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|11|Vd|0101|0|0|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrecpe.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|11|Vd|0101|0|1|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vrecpe.u32 Dd, Dn"                                       , "t32": "1111|11111|Vd'|11|10|11|Vd|0100|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrecpe.u32 Vd, Vn"                                       , "t32": "1111|11111|Vd'|11|10|11|Vd|0100|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrecpe.u32 Dd, Dn"                                       , "a32": "1111|00111|Vd'|11|10|11|Vd|0100|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrecpe.u32 Vd, Vn"                                       , "a32": "1111|00111|Vd'|11|10|11|Vd|0100|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vrecps.f16-32 Dd, Dn, Dm"                                , "t32": "1110|11110|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrecps.f16-32 Vd, Vn, Vm"                                , "t32": "1110|11110|Vd'|0H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrecps.f16-32 Dd, Dn, Dm"                                , "a32": "1111|00100|Vd'|0H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrecps.f16-32 Vd, Vn, Vm"                                , "a32": "1111|00100|Vd'|0H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vrev16.x8 Dd, Dn"                                        , "t32": "1111|11111|Vd'|11|00|00|Vd|0001|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev16.x8 Vd, Vn"                                        , "t32": "1111|11111|Vd'|11|00|00|Vd|0001|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev16.x8 Dd, Dn"                                        , "a32": "1111|00111|Vd'|11|00|00|Vd|0001|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev16.x8 Vd, Vn"                                        , "a32": "1111|00111|Vd'|11|00|00|Vd|0001|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vrev32.x8-16 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|sz|00|Vd|0000|1|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev32.x8-16 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|sz|00|Vd|0000|1|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev32.x8-16 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|sz|00|Vd|0000|1|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev32.x8-16 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|sz|00|Vd|0000|1|1|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vrev64.x8-32 Dd, Dn"                                     , "t32": "1111|11111|Vd'|11|sz|00|Vd|0000|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev64.x8-32 Vd, Vn"                                     , "t32": "1111|11111|Vd'|11|sz|00|Vd|0000|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev64.x8-32 Dd, Dn"                                     , "a32": "1111|00111|Vd'|11|sz|00|Vd|0000|0|0|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrev64.x8-32 Vd, Vn"                                     , "a32": "1111|00111|Vd'|11|sz|00|Vd|0000|0|1|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vrhadd.x8-32 Dd, Dn, Dm"                                 , "t32": "111U|11110|Vd'|sz|Vn|Vd|0001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vrhadd.x8-32 Vd, Vn, Vm"                                 , "t32": "111U|11110|Vd'|sz|Vn|Vd|0001|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vrhadd.x8-32 Dd, Dn, Dm"                                 , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0001|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD"},
    {"inst": "vrhadd.x8-32 Vd, Vn, Vm"                                 , "a32": "1111|001U0|Vd'|sz|Vn|Vd|0001|Vn'|1|Vm'|0|Vm"            , "ext": "ASIMD"},

    {"inst": "vrinta.f16-32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|10|00|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrinta.f16-32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|10|00|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrinta.f64 Dd, Dn"                                       , "t32": "1111|11101|Vd'|11|10|00|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrinta.f64 Dd, Dn"                                       , "a32": "1111|11101|Vd'|11|10|00|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrinta.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|01|00|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrinta.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|01|01|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrinta.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|01|00|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrinta.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|01|01|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintm.f16-32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|10|11|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintm.f16-32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|10|11|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintm.f64 Dd, Dn"                                       , "t32": "1111|11101|Vd'|11|10|11|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrintm.f64 Dd, Dn"                                       , "a32": "1111|11101|Vd'|11|10|11|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrintm.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|10|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintm.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|10|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintm.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|10|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrintm.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|10|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintn.f16-32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|10|01|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintn.f16-32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|10|01|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintn.f64 Dd, Dn"                                       , "t32": "1111|11101|Vd'|11|10|01|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrintn.f64 Dd, Dn"                                       , "a32": "1111|11101|Vd'|11|10|01|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrintn.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|00|00|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintn.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|00|01|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintn.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|00|00|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrintn.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|00|01|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintp.f16-32 Sd, Sn"                                    , "t32": "1111|11101|'Vd|11|10|10|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintp.f16-32 Sd, Sn"                                    , "a32": "1111|11101|'Vd|11|10|10|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintp.f64 Dd, Dn"                                       , "t32": "1111|11101|Vd'|11|10|10|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrintp.f64 Dd, Dn"                                       , "a32": "1111|11101|Vd'|11|10|10|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrintp.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|11|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintp.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|11|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintp.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|11|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrintp.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|11|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintr.f16-32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|01|10|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintr.f16-32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|01|10|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintr.f64 Dd, Dn"                                       , "t32": "1110|11101|Vd'|11|01|10|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrintr.f64 Dd, Dn"                                       , "a32": "cond|11101|Vd'|11|01|10|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrintx.f16-32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|01|11|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintx.f16-32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|01|11|Vd|10|sz|01|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintx.f64 Dd, Dn"                                       , "t32": "1110|11101|Vd'|11|01|11|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrintx.f64 Dd, Dn"                                       , "a32": "cond|11101|Vd'|11|01|11|Vd|10|11|01|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrintx.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|00|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintx.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|00|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintx.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|00|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrintx.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|00|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintz.f16-32 Sd, Sn"                                    , "t32": "1110|11101|'Vd|11|01|10|Vd|10|sz|11|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrintz.f16-32 Sd, Sn"                                    , "a32": "cond|11101|'Vd|11|01|10|Vd|10|sz|11|'Vn|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrintz.f64 Dd, Dn"                                       , "t32": "1110|11101|Vd'|11|01|10|Vd|10|11|11|Vn'|0|Vn"           , "ext": "ARMv8+"},
    {"inst": "vrintz.f64 Dd, Dn"                                       , "a32": "cond|11101|Vd'|11|01|10|Vd|10|11|11|Vn'|0|Vn"           , "ext": "ARMv8+"},

    {"inst": "vrintz.f16-32 Dd, Dn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|01|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintz.f16-32 Vd, Vn"                                    , "t32": "1111|11111|Vd'|11|sz|10|Vd|01|01|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vrintz.f16-32 Dd, Dn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|01|10|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},
    {"inst": "vrintz.f16-32 Vd, Vn"                                    , "a32": "1111|00111|Vd'|11|sz|10|Vd|01|01|11|Vn'|0|Vn"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vrshl.x8-64 Dd, Dn, Dm"                                  , "t32": "111U|11110|Vd'|sz|Vm|Vd|0101|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vrshl.x8-64 Vd, Vn, Vm"                                  , "t32": "111U|11110|Vd'|sz|Vm|Vd|0101|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vrshl.x8-64 Dd, Dn, Dm"                                  , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0101|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vrshl.x8-64 Vd, Vn, Vm"                                  , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0101|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vrshr.x8-32 Dd, Dn, #zero"                               , "t32": "1110|11110|Vd'|10|Vd|Vd|0001|Vn'|0|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},
    {"inst": "vrshr.x8-32 Vd, Vn, #zero"                               , "t32": "1110|11110|Vd'|10|Vd|Vd|0001|Vn'|1|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},
    {"inst": "vrshr.x8-32 Dd, Dn, #zero"                               , "a32": "1111|00100|Vd'|10|Vd|Vd|0001|Vn'|0|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},
    {"inst": "vrshr.x8-32 Vd, Vn, #zero"                               , "a32": "1111|00100|Vd'|10|Vd|Vd|0001|Vn'|1|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},

    {"inst": "vrshr.x8-32 Dd, Dn, #n"                                  , "t32": "111U|11111|Vd'|imm:6|Vd|0010|0|0|Vn'|1|Vn"              , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vrshr.x8-32 Vd, Vn, #n"                                  , "t32": "111U|11111|Vd'|imm:6|Vd|0010|0|1|Vn'|1|Vn"              , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vrshr.x8-32 Dd, Dn, #n"                                  , "a32": "1111|001U1|Vd'|imm:6|Vd|0010|0|0|Vn'|1|Vn"              , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vrshr.x8-32 Vd, Vn, #n"                                  , "a32": "1111|001U1|Vd'|imm:6|Vd|0010|0|1|Vn'|1|Vn"              , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},

    {"inst": "vrshrn.x16-64 Dd, Vn, #zero"                             , "t32": "1111|11111|Vd'|11|sz|10|Vd|0010|00|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "narrow", "pseudoOf": "vmovn"},
    {"inst": "vrshrn.x16-64 Dd, Vn, #zero"                             , "a32": "1111|00111|Vd'|11|sz|10|Vd|0010|00|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "narrow", "pseudoOf": "vmovn"},

    {"inst": "vrshrn.x16-64 Dd, Vn, #n"                                , "t32": "1110|11111|Vd'|imm:6|Vd|1000|0|1|Vn'|1|Vn"              , "ext": "ASIMD", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},
    {"inst": "vrshrn.x16-64 Dd, Vn, #n"                                , "a32": "1111|00101|Vd'|imm:6|Vd|1000|0|1|Vn'|1|Vn"              , "ext": "ASIMD", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},

    {"inst": "vrsqrte.f16-32 Dd, Dn"                                   , "t32": "1111|11111|Vd'|11|sz|11|Vd|01|01|10|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vrsqrte.f16-32 Vd, Vn"                                   , "t32": "1111|11111|Vd'|11|sz|11|Vd|01|01|11|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vrsqrte.f16-32 Dd, Dn"                                   , "a32": "1111|00111|Vd'|11|sz|11|Vd|01|01|10|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrsqrte.f16-32 Vd, Vn"                                   , "a32": "1111|00111|Vd'|11|sz|11|Vd|01|01|11|Vn'|0|Vn"           , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vrsqrte.u32 Dd, Dn"                                      , "t32": "1111|11111|Vd'|11|10|11|Vd|01|00|10|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrsqrte.u32 Vd, Vn"                                      , "t32": "1111|11111|Vd'|11|10|11|Vd|01|00|11|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrsqrte.u32 Dd, Dn"                                      , "a32": "1111|00111|Vd'|11|10|11|Vd|01|00|10|Vn'|0|Vn"           , "ext": "ASIMD"},
    {"inst": "vrsqrte.u32 Vd, Vn"                                      , "a32": "1111|00111|Vd'|11|10|11|Vd|01|00|11|Vn'|0|Vn"           , "ext": "ASIMD"},

    {"inst": "vrsqrts.f16-32 Dd, Dn, Dm"                               , "t32": "1110|11110|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vrsqrts.f16-32 Vd, Vn, Vm"                               , "t32": "1110|11110|Vd'|1H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL", "it": "out"},
    {"inst": "vrsqrts.f16-32 Dd, Dn, Dm"                               , "a32": "1111|00100|Vd'|1H|Vn|Vd|1111|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vrsqrts.f16-32 Vd, Vn, Vm"                               , "a32": "1111|00100|Vd'|1H|Vn|Vd|1111|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vrsra.x8-64 Dx, Dn, #n"                                  , "t32": "111U|11111|Vx'|imm[5:0]|Vx|0011|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vrsra.x8-64 Vx, Vn, #n"                                  , "t32": "111U|11111|Vx'|imm[5:0]|Vx|0011|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vrsra.x8-64 Dx, Dn, #n"                                  , "a32": "1111|001U1|Vx'|imm[5:0]|Vx|0011|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vrsra.x8-64 Vx, Vn, #n"                                  , "a32": "1111|001U1|Vx'|imm[5:0]|Vx|0011|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},

    {"inst": "vrsubhn.x16-64 Dd, Vn, Vm"                               , "t32": "1111|11111|Vd'|szM1:2|Vn|Vd|0110|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},
    {"inst": "vrsubhn.x16-64 Dd, Vn, Vm"                               , "a32": "1111|00111|Vd'|szM1:2|Vn|Vd|0110|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},

    {"inst": "vsdot.s8 Dd, Dn, Dm"                                     , "t32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "DOTPROD"},
    {"inst": "vsdot.s8 Vd, Vn, Vm"                                     , "t32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "DOTPROD"},
    {"inst": "vsdot.s8 Dd, Dn, Dm"                                     , "a32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "DOTPROD"},
    {"inst": "vsdot.s8 Vd, Vn, Vm"                                     , "a32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "DOTPROD"},

    {"inst": "vsdot.s8 Dd, Dn, Dm<=15[#i]"                             , "t32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|0|i:1|0|Vm"            , "ext": "DOTPROD"},
    {"inst": "vsdot.s8 Vd, Vn, Dm<=15[#i]"                             , "t32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "DOTPROD"},
    {"inst": "vsdot.s8 Dd, Dn, Dm<=15[#i]"                             , "a32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|0|i:1|0|Vm"            , "ext": "DOTPROD"},
    {"inst": "vsdot.s8 Vd, Vn, Dm<=15[#i]"                             , "a32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "DOTPROD"},

    {"inst": "vseleq.f16-32 Sd, Sn, Sm"                                , "t32": "1111|11100|'Vd|00|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vseleq.f16-32 Sd, Sn, Sm"                                , "a32": "1111|11100|'Vd|00|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vseleq.f64 Dd, Dn, Dm"                                   , "t32": "1111|11100|Vd'|00|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+", "it": "out"},
    {"inst": "vseleq.f64 Dd, Dn, Dm"                                   , "a32": "1111|11100|Vd'|00|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+"},

    {"inst": "vselge.f16-32 Sd, Sn, Sm"                                , "t32": "1111|11100|'Vd|10|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vselge.f16-32 Sd, Sn, Sm"                                , "a32": "1111|11100|'Vd|10|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vselge.f64 Dd, Dn, Dm"                                   , "t32": "1111|11100|Vd'|10|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+", "it": "out"},
    {"inst": "vselge.f64 Dd, Dn, Dm"                                   , "a32": "1111|11100|Vd'|10|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+"},

    {"inst": "vselgt.f16-32 Sd, Sn, Sm"                                , "t32": "1111|11100|'Vd|11|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vselgt.f16-32 Sd, Sn, Sm"                                , "a32": "1111|11100|'Vd|11|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vselgt.f64 Dd, Dn, Dm"                                   , "t32": "1111|11100|Vd'|11|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+", "it": "out"},
    {"inst": "vselgt.f64 Dd, Dn, Dm"                                   , "a32": "1111|11100|Vd'|11|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+"},

    {"inst": "vselvs.f16-32 Sd, Sn, Sm"                                , "t32": "1111|11100|'Vd|01|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL", "it": "out"},
    {"inst": "vselvs.f16-32 Sd, Sn, Sm"                                , "a32": "1111|11100|'Vd|01|Vn|Vd|10|sz|'Vn|0|'Vm|0|Vm"           , "ext": "ARMv8+ ~FP16FULL"},

    {"inst": "vselvs.f64 Dd, Dn, Dm"                                   , "t32": "1111|11100|Vd'|01|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+", "it": "out"},
    {"inst": "vselvs.f64 Dd, Dn, Dm"                                   , "a32": "1111|11100|Vd'|01|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ARMv8+"},

    {"inst": "vshl.x8-64 Dd, Dn, Dm"                                   , "t32": "111U|11110|Vd'|sz|Vm|Vd|0100|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vshl.x8-64 Vd, Vn, Vm"                                   , "t32": "111U|11110|Vd'|sz|Vm|Vd|0100|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vshl.x8-64 Dd, Dn, Dm"                                   , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0100|Vm'|0|Vn'|0|Vn"            , "ext": "ASIMD"},
    {"inst": "vshl.x8-64 Vd, Vn, Vm"                                   , "a32": "1111|001U0|Vd'|sz|Vm|Vd|0100|Vm'|1|Vn'|0|Vn"            , "ext": "ASIMD"},

    {"inst": "vshl.x8-64 Dd, Dn, #n"                                   , "t32": "1110|11111|Vd'|imm[5:0]|Vd|0101|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vshl.x8-64 Vd, Vn, #n"                                   , "t32": "1110|11111|Vd'|imm[5:0]|Vd|0101|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vshl.x8-64 Dd, Dn, #n"                                   , "a32": "1111|00101|Vd'|imm[5:0]|Vd|0101|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vshl.x8-64 Vd, Vn, #n"                                   , "a32": "1111|00101|Vd'|imm[5:0]|Vd|0101|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},

    {"inst": "vshll.x8-32 Vd, Dn, #n"                                  , "t32": "111U|11111|Vd'|imm[5:0]|Vd|1010|0|0|Vn'|1|Vn"           , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)", "vecOp": "widen"},
    {"inst": "vshll.x8-32 Vd, Dn, #n"                                  , "a32": "1111|001U1|Vd'|imm[5:0]|Vd|1010|0|0|Vn'|1|Vn"           , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)", "vecOp": "widen"},

    {"inst": "vshr.x8-64 Dd, Dn, #zero"                                , "t32": "1110|11110|Vd'|10|Vn|Vd|0001|Vn'|0|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},
    {"inst": "vshr.x8-64 Vd, Vn, #zero"                                , "t32": "1110|11110|Vd'|10|Vn|Vd|0001|Vn'|1|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},
    {"inst": "vshr.x8-64 Dd, Dn, #zero"                                , "a32": "1111|00100|Vd'|10|Vn|Vd|0001|Vn'|0|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},
    {"inst": "vshr.x8-64 Vd, Vn, #zero"                                , "a32": "1111|00100|Vd'|10|Vn|Vd|0001|Vn'|1|Vn'|1|Vn"            , "ext": "ASIMD", "pseudoOf": "vorr"},

    {"inst": "vshr.x8-64 Dd, Dn, #n"                                   , "t32": "111U|11111|Vd'|imm[5:0]|Vd|0000|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vshr.x8-64 Vd, Vn, #n"                                   , "t32": "111U|11111|Vd'|imm[5:0]|Vd|0000|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vshr.x8-64 Dd, Dn, #n"                                   , "a32": "1111|001U1|Vd'|imm[5:0]|Vd|0000|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vshr.x8-64 Vd, Vn, #n"                                   , "a32": "1111|001U1|Vd'|imm[5:0]|Vd|0000|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},

    {"inst": "vshrn.x16-64 Dd, Vn, #zero"                              , "t32": "1111|11111|Vd'|11|sz|10|Vd|0010|00|Vn'|0|Vn"            , "ext": "ASIMD", "vecOp": "narrow"},
    {"inst": "vshrn.x16-64 Dd, Vn, #zero"                              , "a32": "1111|11111|Vd'|11|sz|10|Vd|0010|00|Vn'|1|Vn"            , "ext": "ASIMD", "vecOp": "narrow"},

    {"inst": "vshrn.x16-64 Dd, Vn, #n"                                 , "t32": "1110|11111|Vd'|imm:6|Vd|1000|00|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},
    {"inst": "vshrn.x16-64 Dd, Vn, #n"                                 , "a32": "1111|00101|Vd'|imm:6|Vd|1000|00|Vn'|1|Vn"               , "ext": "ASIMD", "imm": "VecShiftNarrowImm(sz, n)", "vecOp": "narrow"},

    {"inst": "vsli.x8-64 Dx, Dn, #n"                                   , "t32": "1111|11111|Vx'|imm[5:0]|Vx|0101|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vsli.x8-64 Vx, Vn, #n"                                   , "t32": "1111|11111|Vx'|imm[5:0]|Vx|0101|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vsli.x8-64 Dx, Dn, #n"                                   , "a32": "1111|00111|Vx'|imm[5:0]|Vx|0101|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},
    {"inst": "vsli.x8-64 Vx, Vn, #n"                                   , "a32": "1111|00111|Vx'|imm[5:0]|Vx|0101|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftPImm(sz, n)"},

    {"inst": "vsmmla.s8 Vd, Vn, Vm"                                    , "t32": "1111|11000|Vd'|10|Vn|Vd|1100|Vn'|1|Vm'|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vsmmla.s8 Vd, Vn, Vm"                                    , "a32": "1111|11000|Vd'|10|Vn|Vd|1100|Vn'|1|Vm'|0|Vm"            , "ext": "AA32I8MM"},

    {"inst": "vsqrt.f16-32 Sd, Sn"                                     , "t32": "1110|11101|'Vd|11|0001|Vd|10|sz|11|'Vn|0|Vn"            , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vsqrt.f16-32 Sd, Sn"                                     , "a32": "cond|11101|'Vd|11|0001|Vd|10|sz|11|'Vn|0|Vn"            , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vsqrt.f64 Dd, Dn"                                        , "t32": "1110|11101|Vd'|11|0001|Vd|1011|1|1|Vn'|0|Vn"            , "ext": "VFPv2"},
    {"inst": "vsqrt.f64 Dd, Dn"                                        , "a32": "cond|11101|Vd'|11|0001|Vd|1011|1|1|Vn'|0|Vn"            , "ext": "VFPv2"},

    {"inst": "vsra.x8-64 Dx, Dn, #n"                                   , "t32": "111U|11111|Vx'|imm[5:0]|Vx|0001|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vsra.x8-64 Vx, Vn, #n"                                   , "t32": "111U|11111|Vx'|imm[5:0]|Vx|0001|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vsra.x8-64 Dx, Dn, #n"                                   , "a32": "1111|001U1|Vx'|imm[5:0]|Vx|0001|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vsra.x8-64 Vx, Vn, #n"                                   , "a32": "1111|001U1|Vx'|imm[5:0]|Vx|0001|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},

    {"inst": "vsri.x8-64 Dx, Dn, #n"                                   , "t32": "1111|11111|Vx'|imm[5:0]|Vx|0100|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vsri.x8-64 Vx, Vn, #n"                                   , "t32": "1111|11111|Vx'|imm[5:0]|Vx|0100|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vsri.x8-64 Dx, Dn, #n"                                   , "a32": "1111|00111|Vx'|imm[5:0]|Vx|0100|imm[6]|0|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},
    {"inst": "vsri.x8-64 Vx, Vn, #n"                                   , "a32": "1111|00111|Vx'|imm[5:0]|Vx|0100|imm[6]|1|Vn'|1|Vn"      , "ext": "ASIMD", "imm": "VecShiftNImm(sz, n)"},

    {"inst": "vst1.8-64 Ds, [Rn!=PC]"                                  , "t32": "1111|10010|Vs'|00|Rn|Vs|0111|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 Ds, [Rn!=PC, Rm!=XX]@"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|0111|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 Ds, [Rn!=PC, #off==8]@"                        , "t32": "1111|10010|Vs'|00|Rn|Vs|0111|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 2x{Ds<=30}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|1010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 2x{Ds<=30}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|1010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 2x{Ds<=30}+, [Rn!=PC, #off==16]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|1010|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 3x{Ds<=29}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|0110|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 3x{Ds<=29}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|0110|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 3x{Ds<=29}+, [Rn!=PC, #off==24]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|0110|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 4x{Ds<=28}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|0010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 4x{Ds<=28}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|0010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 4x{Ds<=28}+, [Rn!=PC, #off==32]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|0010|sz|0|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vst1.8-64 Ds, [Rn!=PC]"                                  , "a32": "1111|01000|Vs'|00|Rn|Vs|0111|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 Ds, [Rn!=PC, Rm!=XX]@"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|0111|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 Ds, [Rn!=PC, #off==8]@"                        , "a32": "1111|01000|Vs'|00|Rn|Vs|0111|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 2x{Ds<=30}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|1010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 2x{Ds<=30}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|1010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 2x{Ds<=30}+, [Rn!=PC, #off==16]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|1010|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 3x{Ds<=29}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|0110|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 3x{Ds<=29}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|0110|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 3x{Ds<=29}+, [Rn!=PC, #off==24]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|0110|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 4x{Ds<=28}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|0010|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst1.8-64 4x{Ds<=28}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|0010|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst1.8-64 4x{Ds<=28}+, [Rn!=PC, #off==32]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|0010|sz|0|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vst2.8-32 2x{Ds<=30}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|1000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=30}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|1000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=30}+, [Rn!=PC, #off=16]@"               , "t32": "1111|10010|Vs'|00|Rn|Vs|1000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=29}++, [Rn!=PC]"                        , "t32": "1111|10010|Vs'|00|Rn|Vs|1001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=29}++, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10010|Vs'|00|Rn|Vs|1001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=29}++, [Rn!=PC, #off=16]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|1001|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 4x{Ds<=28}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|0011|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 4x{Ds<=28}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|0011|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst2.8-32 4x{Ds<=28}+, [Rn!=PC, #off==32]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|0011|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vst2.8-32 2x{Ds<=30}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|1000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=30}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|1000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=30}+, [Rn!=PC, #off==16]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|1000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=29}++, [Rn!=PC]"                        , "a32": "1111|01000|Vs'|00|Rn|Vs|1001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=29}++, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01000|Vs'|00|Rn|Vs|1001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst2.8-32 2x{Ds<=29}++, [Rn!=PC, #off==16]@"             , "a32": "1111|01000|Vs'|00|Rn|Vs|1001|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 4x{Ds<=28}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|0011|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst2.8-32 4x{Ds<=28}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|0011|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst2.8-32 4x{Ds<=28}+, [Rn!=PC, #off==32]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|0011|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vst3.8-32 3x{Ds<=29}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|0100|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=29}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|0100|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=29}+, [Rn!=PC, #off==24]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|0100|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=27}++, [Rn!=PC]"                        , "t32": "1111|10010|Vs'|00|Rn|Vs|0101|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=27}++, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10010|Vs'|00|Rn|Vs|0101|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=27}++, [Rn!=PC, #off==24]@"             , "t32": "1111|10010|Vs'|00|Rn|Vs|0101|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vst3.8-32 3x{Ds<=29}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|0100|sz|0|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=29}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|0100|sz|0|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=29}+, [Rn!=PC, #off==24]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|0100|sz|0|align:1|1101"         , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=27}++, [Rn!=PC]"                        , "a32": "1111|01000|Vs'|00|Rn|Vs|0101|sz|1|align:1|1111"         , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=27}++, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01000|Vs'|00|Rn|Vs|0101|sz|1|align:1|Rm"           , "ext": "ASIMD"},
    {"inst": "vst3.8-32 3x{Ds<=27}++, [Rn!=PC, #off==24]@"             , "a32": "1111|01000|Vs'|00|Rn|Vs|0101|sz|1|align:1|1101"         , "ext": "ASIMD"},

    {"inst": "vst4.8-32 4x{Ds<=29}+, [Rn!=PC]"                         , "t32": "1111|10010|Vs'|00|Rn|Vs|0000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=29}+, [Rn!=PC, Rm!=XX]@"                , "t32": "1111|10010|Vs'|00|Rn|Vs|0000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=29}+, [Rn!=PC, #off==32]@"              , "t32": "1111|10010|Vs'|00|Rn|Vs|0000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=27}++, [Rn!=PC]"                        , "t32": "1111|10010|Vs'|00|Rn|Vs|0001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=27}++, [Rn!=PC, Rm!=XX]@"               , "t32": "1111|10010|Vs'|00|Rn|Vs|0001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=27}++, [Rn!=PC, #off==32]@"             , "t32": "1111|10010|Vs'|00|Rn|Vs|0001|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vst4.8-32 4x{Ds<=29}+, [Rn!=PC]"                         , "a32": "1111|01000|Vs'|00|Rn|Vs|0000|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=29}+, [Rn!=PC, Rm!=XX]@"                , "a32": "1111|01000|Vs'|00|Rn|Vs|0000|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=29}+, [Rn!=PC, #off==32]@"              , "a32": "1111|01000|Vs'|00|Rn|Vs|0000|sz|align:2|1101"           , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=27}++, [Rn!=PC]"                        , "a32": "1111|01000|Vs'|00|Rn|Vs|0001|sz|align:2|1111"           , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=27}++, [Rn!=PC, Rm!=XX]@"               , "a32": "1111|01000|Vs'|00|Rn|Vs|0001|sz|align:2|Rm"             , "ext": "ASIMD"},
    {"inst": "vst4.8-32 4x{Ds<=27}++, [Rn!=PC, #off==32]@"             , "a32": "1111|01000|Vs'|00|Rn|Vs|0001|sz|align:2|1101"           , "ext": "ASIMD"},

    {"inst": "vstr.16 Ss, [Rn, #+/-off*2]"                             , "t32": "1110|1101U|'Vs|00|Rn|Vs|1001|off:8"                     , "ext": "FP16FULL" , "it": "out"},
    {"inst": "vstr.32 Ss, [Rn, #+/-off*4]"                             , "t32": "1110|1101U|'Vs|00|Rn|Vs|1010|off:8"                     , "ext": "VFPv3"},
    {"inst": "vstr.64 Ds, [Rn, #+/-off*4]"                             , "t32": "1110|1101U|Vs'|00|Rn|Vs|1011|off:8"                     , "ext": "VFPv3"},
    {"inst": "vstr.16 Ss, [Rn, #+/-off*2]"                             , "a32": "cond|1101U|'Vs|00|Rn|Vs|1001|off:8"                     , "ext": "FP16FULL" },
    {"inst": "vstr.32 Ss, [Rn, #+/-off*4]"                             , "a32": "cond|1101U|'Vs|00|Rn|Vs|1010|off:8"                     , "ext": "VFPv3"},
    {"inst": "vstr.64 Ds, [Rn, #+/-off*4]"                             , "a32": "cond|1101U|Vs'|00|Rn|Vs|1011|off:8"                     , "ext": "VFPv3"},

    {"inst": "vsub.f16-32 Sd, Sn, Sm"                                  , "t32": "1110|11100|'Vd|11|Vn|Vd|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL", "it": "out"},
    {"inst": "vsub.f16-32 Sd, Sn, Sm"                                  , "a32": "cond|11100|'Vd|11|Vn|Vd|10|sz|'Vn|1|'Vm|0|Vm"           , "ext": "VFPv2 ~FP16FULL"},

    {"inst": "vsub.f64 Dd, Dn, Dm"                                     , "t32": "1110|11100|Vd'|11|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},
    {"inst": "vsub.f64 Dd, Dn, Dm"                                     , "a32": "cond|11100|Vd'|11|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "VFPv2"},

    {"inst": "vsub.f16-32 Dd, Dn, Dm"                                  , "t32": "1110|11110|Vd'|1H|Vn|Vd|11|01|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vsub.f16-32 Vd, Vn, Vm"                                  , "t32": "1110|11110|Vd'|1H|Vn|Vd|11|01|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vsub.f16-32 Dd, Dn, Dm"                                  , "a32": "1111|00100|Vd'|1H|Vn|Vd|11|01|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD ~FP16FULL"},
    {"inst": "vsub.f16-32 Vd, Vn, Vm"                                  , "a32": "1111|00100|Vd'|1H|Vn|Vd|11|01|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD ~FP16FULL"},

    {"inst": "vsub.x8-64 Dd, Dn, Dm"                                   , "t32": "1111|11110|Vd'|sz|Vn|Vd|10|00|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vsub.x8-64 Vd, Vn, Vm"                                   , "t32": "1111|11110|Vd'|sz|Vn|Vd|10|00|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vsub.x8-64 Dd, Dn, Dm"                                   , "a32": "1111|00110|Vd'|sz|Vn|Vd|10|00|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vsub.x8-64 Vd, Vn, Vm"                                   , "a32": "1111|00110|Vd'|sz|Vn|Vd|10|00|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},

    {"inst": "vsubhn.x16-64 Dd, Vn, Vm"                                , "t32": "1110|11111|Vd'|szM1:2|Vn|Vd|0110|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},
    {"inst": "vsubhn.x16-64 Dd, Vn, Vm"                                , "a32": "1111|00101|Vd'|szM1:2|Vn|Vd|0110|Vn'|0|Vm'|0|Vm"        , "ext": "ASIMD", "vecOp": "narrow"},

    {"inst": "vsubl.x8-32 Vd, Dn, Dm"                                  , "t32": "111U|11111|Vd'|sz|Vn|Vd|0010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vsubl.x8-32 Vd, Dn, Dm"                                  , "a32": "1111|001U1|Vd'|sz|Vn|Vd|0010|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vsubw.x8-32 Vd, Vn, Dm"                                  , "t32": "111U|11111|Vd'|sz|Vn|Vd|0011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},
    {"inst": "vsubw.x8-32 Vd, Vn, Dm"                                  , "a32": "1111|001U1|Vd'|sz|Vn|Vd|0011|Vn'|0|Vm'|0|Vm"            , "ext": "ASIMD", "vecOp": "widen"},

    {"inst": "vsudot.u8 Dd, Dn, Dm<=15[#i]"                            , "t32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|0|i:1|1|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vsudot.u8 Vd, Vn, Dm<=15[#i]"                            , "t32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|1|i:1|1|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vsudot.u8 Dd, Dn, Dm<=15[#i]"                            , "a32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|0|i:1|1|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vsudot.u8 Vd, Vn, Dm<=15[#i]"                            , "a32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|1|i:1|1|Vm"            , "ext": "AA32I8MM"},

    {"inst": "vswp.any Dx, Dx2!=Dx"                                    , "t32": "1111|11111|Vx'|11|00|10|Vx|0000|0|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vswp.any Vx, Vx2!=Vx"                                    , "t32": "1111|11111|Vx'|11|00|10|Vx|0000|0|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vswp.any Dx, Dx2!=Dx"                                    , "a32": "1111|00111|Vx'|11|00|10|Vx|0000|0|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vswp.any Vx, Vx2!=Vx"                                    , "a32": "1111|00111|Vx'|11|00|10|Vx|0000|0|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},

    {"inst": "vtbl.x8 Dd, Dn, Dm"                                      , "t32": "1111|11111|Vd'|11|Vn|Vd|10|00|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dn2+, Dm"                                , "t32": "1111|11111|Vd'|11|Vn|Vd|10|01|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dn2+, Dn3+, Dm"                          , "t32": "1111|11111|Vd'|11|Vn|Vd|10|10|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dn2+, Dn3+, Dn4+, Dm"                    , "t32": "1111|11111|Vd'|11|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dm"                                      , "a32": "1111|00111|Vd'|11|Vn|Vd|10|00|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dn2+, Dm"                                , "a32": "1111|00111|Vd'|11|Vn|Vd|10|01|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dn2+, Dn3+, Dm"                          , "a32": "1111|00111|Vd'|11|Vn|Vd|10|10|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbl.x8 Dd, Dn, Dn2+, Dn3+, Dn4+, Dm"                    , "a32": "1111|00111|Vd'|11|Vn|Vd|10|11|Vn'|0|Vm'|0|Vm"           , "ext": "ASIMD"},

    {"inst": "vtbx.x8 Dd, Dn, Dm"                                      , "t32": "1111|11111|Vd'|11|Vn|Vd|10|00|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dn2+, Dm"                                , "t32": "1111|11111|Vd'|11|Vn|Vd|10|01|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dn2+, Dn3+, Dm"                          , "t32": "1111|11111|Vd'|11|Vn|Vd|10|10|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dn2+, Dn3+, Dn4+, Dm"                    , "t32": "1111|11111|Vd'|11|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dm"                                      , "a32": "1111|00111|Vd'|11|Vn|Vd|10|00|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dn2+, Dm"                                , "a32": "1111|00111|Vd'|11|Vn|Vd|10|01|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dn2+, Dn3+, Dm"                          , "a32": "1111|00111|Vd'|11|Vn|Vd|10|10|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},
    {"inst": "vtbx.x8 Dd, Dn, Dn2+, Dn3+, Dn4+, Dm"                    , "a32": "1111|00111|Vd'|11|Vn|Vd|10|11|Vn'|1|Vm'|0|Vm"           , "ext": "ASIMD"},

    {"inst": "vtrn.x8-32 Dx, Dx2"                                      , "t32": "1111|11111|Vx'|11|sz|10|Vx|0000|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vtrn.x8-32 Vx, Vx2"                                      , "t32": "1111|11111|Vx'|11|sz|10|Vx|0000|1|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vtrn.x8-32 Dx, Dx2"                                      , "a32": "1111|00111|Vx'|11|sz|10|Vx|0000|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vtrn.x8-32 Vx, Vx2"                                      , "a32": "1111|00111|Vx'|11|sz|10|Vx|0000|1|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},

    {"inst": "vtst.x8-32 Dd, Dn, Dm"                                   , "t32": "1110|11110|Vd'|sz|Vn|Vd|1000|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vtst.x8-32 Vd, Vn, Vm"                                   , "t32": "1110|11110|Vd'|sz|Vn|Vd|1000|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vtst.x8-32 Dd, Dn, Dm"                                   , "a32": "1111|00100|Vd'|sz|Vn|Vd|1000|Vn'|0|Vm'|1|Vm"            , "ext": "ASIMD"},
    {"inst": "vtst.x8-32 Vd, Vn, Vm"                                   , "a32": "1111|00100|Vd'|sz|Vn|Vd|1000|Vn'|1|Vm'|1|Vm"            , "ext": "ASIMD"},

    {"inst": "vudot.u8 Dd, Dn, Dm"                                     , "t32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|0|Vm'|1|Vm"            , "ext": "DOTPROD"},
    {"inst": "vudot.u8 Vd, Vn, Vm"                                     , "t32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|1|Vm'|1|Vm"            , "ext": "DOTPROD"},
    {"inst": "vudot.u8 Dd, Dn, Dm"                                     , "a32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|0|Vm'|1|Vm"            , "ext": "DOTPROD"},
    {"inst": "vudot.u8 Vd, Vn, Vm"                                     , "a32": "1111|11000|Vd'|10|Vn|Vd|1101|Vn'|1|Vm'|1|Vm"            , "ext": "DOTPROD"},

    {"inst": "vudot.u8 Dd, Dn, Dm<=15[#i]"                             , "t32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|0|i:1|1|Vm"            , "ext": "DOTPROD"},
    {"inst": "vudot.u8 Vd, Vn, Dm<=15[#i]"                             , "t32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|1|Vm"            , "ext": "DOTPROD"},
    {"inst": "vudot.u8 Dd, Dn, Dm<=15[#i]"                             , "a32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|0|i:1|1|Vm"            , "ext": "DOTPROD"},
    {"inst": "vudot.u8 Vd, Vn, Dm<=15[#i]"                             , "a32": "1111|11100|Vd'|10|Vn|Vd|1101|Vn'|1|i:1|1|Vm"            , "ext": "DOTPROD"},

    {"inst": "vummla.u8 Vd, Vn, Vm"                                    , "t32": "1111|11000|Vd'|10|Vn|Vd|1100|Vn'|1|Vm'|1|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vummla.u8 Vd, Vn, Vm"                                    , "a32": "1111|11000|Vd'|10|Vn|Vd|1100|Vn'|1|Vm'|1|Vm"            , "ext": "AA32I8MM"},

    {"inst": "vusdot.s8 Dd, Dn, Dm"                                    , "t32": "1111|11001|Vd'|10|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusdot.s8 Vd, Vn, Vm"                                    , "t32": "1111|11001|Vd'|10|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusdot.s8 Dd, Dn, Dm"                                    , "a32": "1111|11001|Vd'|10|Vn|Vd|1101|Vn'|0|Vm'|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusdot.s8 Vd, Vn, Vm"                                    , "a32": "1111|11001|Vd'|10|Vn|Vd|1101|Vn'|1|Vm'|0|Vm"            , "ext": "AA32I8MM"},

    {"inst": "vusdot.s8 Dd, Dn, Dm<=15[#i]"                            , "t32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|0|i:1|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusdot.s8 Vd, Vn, Dm<=15[#i]"                            , "t32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusdot.s8 Dd, Dn, Dm<=15[#i]"                            , "a32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|0|i:1|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusdot.s8 Vd, Vn, Dm<=15[#i]"                            , "a32": "1111|11101|Vd'|00|Vn|Vd|1101|Vn'|1|i:1|0|Vm"            , "ext": "AA32I8MM"},

    {"inst": "vusmmla.s8 Vd, Vn, Vm"                                   , "t32": "1111|11001|Vd'|10|Vn|Vd|1100|Vn'|1|Vm'|0|Vm"            , "ext": "AA32I8MM"},
    {"inst": "vusmmla.s8 Vd, Vn, Vm"                                   , "a32": "1111|11001|Vd'|10|Vn|Vd|1100|Vn'|1|Vm'|0|Vm"            , "ext": "AA32I8MM"},

    {"inst": "vuzp.x8-16 Dx, Dx2"                                      , "t32": "1111|11111|Vx'|11|sz|10|Vx|0001|0|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vuzp.x32 Dx, Dx2"                                        , "t32": "1111|11111|Vx'|11|sz|10|Vx|0000|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD", "aliasOf": "vtrn"},
    {"inst": "vuzp.x8-32 Vx, Vx2"                                      , "t32": "1111|11111|Vx'|11|sz|10|Vx|0001|0|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vuzp.x8-16 Dx, Dx2"                                      , "a32": "1111|00111|Vx'|11|sz|10|Vx|0001|0|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vuzp.x32 Dx, Dx2"                                        , "a32": "1111|00111|Vx'|11|sz|10|Vx|0000|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD", "aliasOf": "vtrn"},
    {"inst": "vuzp.x8-32 Vx, Vx2"                                      , "a32": "1111|00111|Vx'|11|sz|10|Vx|0001|0|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},

    {"inst": "vzip.x8-16 Dx, Dx2"                                      , "t32": "1111|11111|Vx'|11|sz|10|Vx|0001|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vzip.x32 Dx, Dx2"                                        , "t32": "1111|11111|Vx'|11|sz|10|Vx|0000|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD", "aliasOf": "vtrn"},
    {"inst": "vzip.x8-32 Vx, Vx2"                                      , "t32": "1111|11111|Vx'|11|sz|10|Vx|0001|1|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vzip.x8-16 Dx, Dx2"                                      , "a32": "1111|00111|Vx'|11|sz|10|Vx|0001|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD"},
    {"inst": "vzip.x32 Dx, Dx2"                                        , "a32": "1111|00111|Vx'|11|sz|10|Vx|0000|1|0|Vx2'|0|Vx2"         , "ext": "ASIMD", "aliasOf": "vtrn"},
    {"inst": "vzip.x8-32 Vx, Vx2"                                      , "a32": "1111|00111|Vx'|11|sz|10|Vx|0001|1|1|Vx2'|0|Vx2"         , "ext": "ASIMD"},

    {"inst": "wfe"                                                     , "t16": "1011|1111|0010|0000"                                    , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "wfe"                                                     , "t32": "1111|001|1101|0|1111|1000|0000|0000|0010"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "wfe"                                                     , "a32": "cond|001|1001|0|0000|1111|0000|0000|0010"               , "ext": "ARMv6K+"},

    {"inst": "wfi"                                                     , "t16": "1011|1111|0011|0000"                                    , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "wfi"                                                     , "t32": "1111|001|1101|0|1111|1000|0000|0000|0011"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "wfi"                                                     , "a32": "cond|001|1001|0|0000|1111|0000|0000|0011"               , "ext": "ARMv6K+"},

    {"inst": "yield"                                                   , "t16": "1011|1111|0001|0000"                                    , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "yield"                                                   , "t32": "1111|001|1101|0|1111|1000|0000|0000|0001"               , "ext": "ARMv6T2+", "it": "any"},
    {"inst": "yield"                                                   , "a32": "cond|001|1001|0|0000|1111|0000|0000|0001"               , "ext": "ARMv6K+"}
  ]
}
// ${JSON:END}
;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "armdata"]);
