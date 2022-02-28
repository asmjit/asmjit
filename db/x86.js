// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

(function($scope, $as) {
"use strict";

function FAIL(msg) { throw new Error("[X86] " + msg); }

// Import.
const base = $scope.base ? $scope.base : require("./base.js");
const x86data = $scope.x86data ? $scope.x86data : require("./x86data.js");

const hasOwn = Object.prototype.hasOwnProperty;
const dict = base.dict;
const NONE = base.NONE;
const Parsing = base.Parsing;

// Indexes used by instruction-data.
const kIndexName     = 0;
const kIndexOperands = 1;
const kIndexEncoding = 2;
const kIndexOpcode   = 3;
const kIndexMetadata = 4;

// Export.
const x86 = $scope[$as] = {};

// ============================================================================
// [CpuRegs]
// ============================================================================

// Build an object containing CPU registers as keys mapping them to type, kind, and index.
function buildCpuRegs(defs) {
  const map = dict();

  for (var type in defs) {
    const def = defs[type];
    const kind = def.kind;
    const names = def.names;

    if (def.any)
      map[def.any] = { type: type, kind: kind, index: -1 };

    for (var i = 0; i < names.length; i++) {
      var name = names[i];
      var m = /^([A-Za-z\(\)]+)(\d+)-(\d+)([A-Za-z\(\)]*)$/.exec(name);

      if (m) {
        var a = parseInt(m[2], 10);
        var b = parseInt(m[3], 10);

        for (var n = a; n <= b; n++) {
          const index = m[1] + n + m[4];
          map[index] = { type: type, kind: kind, index: index };
        }
      }
      else {
        map[name] = { type: type, kind: kind, index: i };
      }
    }
  }

  // HACK: In instruction manuals `r8` denotes low 8-bit register, however,
  // that collides with `r8`, which is a 64-bit register. Since the result
  // of this function is only used internally we patch it to be compatible
  // with what Intel specifies.
  map.r8.type = "r8";

  return map;
}

const kCpuRegisters = buildCpuRegs(x86data.registers);

// ============================================================================
// [asmdb.x86.Utils]
// ============================================================================

const RegSize = Object.freeze({
  "r8"  : 8,
  "r8hi": 8,
  "r16" : 16,
  "r32" : 32,
  "r64" : 64,
  "mm"  : 64,
  "xmm" : 128,
  "ymm" : 256,
  "zmm" : 512,
  "tmm" : 512, // Maximum size (64 bytes).
  "bnd" : 128,
  "k"   : 64,
  "st"  : 80
});

// X86/X64 utilities.
class Utils {
  // Split the operand(s) string into individual operands as defined by the
  // instruction database.
  //
  // NOTE: X86/X64 doesn't require anything else than separating the commas,
  // this function is here for compatibility with other instruction sets.
  static splitOperands(s) {
    const array = s.split(",");
    for (var i = 0; i < array.length; i++)
      array[i] = array[i].trim();
    return array;
  }

  // Get whether the string `s` describes a register operand.
  static isRegOp(s) { return s && hasOwn.call(kCpuRegisters, s); }
  // Get whether the string `s` describes a memory operand.
  static isMemOp(s) { return s && /^(?:mem|mib|tmem|(?:m(?:off)?\d+(?:dec|bcd|fp|int)?)|(?:m16_\d+)|(?:vm\d+(?:x|y|z)))$/.test(s); }
  // Get whether the string `s` describes an immediate operand.
  static isImmOp(s) { return s && /^(?:1|i4|u4|ib|ub|iw|uw|id|ud|if|iq|uq|p16_16|p16_32)$/.test(s); }
  // Get whether the string `s` describes a relative displacement (label).
  static isRelOp(s) { return s && /^rel\d+$/.test(s); }

  // Get a register type of a `s`, returns `null` if the register is unknown.
  static regTypeOf(s) { return hasOwn.call(kCpuRegisters, s) ? kCpuRegisters[s].type : null; }
  // Get a register kind of a `s`, returns `null` if the register is unknown.
  static regKindOf(s) { return hasOwn.call(kCpuRegisters, s) ? kCpuRegisters[s].kind : null; }
  // Get a register type of a `s`, returns `null` if the register is unknown and `-1`
  // if the given string does only represent a register type, but not a specific reg.
  static regIndexOf(s) { return hasOwn.call(kCpuRegisters, s) ? kCpuRegisters[s].index : null; }

  static regSize(s) {
    if (s in RegSize)
      return RegSize[s];

    const reg = kCpuRegisters[s];
    if (reg && reg.type in RegSize)
      return RegSize[reg.type];

    return -1;
  }

  // Get size of an immediate `s` [in bits].
  //
  // Handles "ib", "iw", "id", "if", "iq", and also "/is4".
  static immSize(s) {
    switch (s) {
      case "1"     : return 8;
      case "i4"    :
      case "u4"    :
      case "/is4"  : return 4;
      case "ib"    :
      case "ub"    : return 8;
      case "iw"    :
      case "uw"    : return 16;
      case "id"    :
      case "ud"    : return 32;
      case "iq"    :
      case "uq"    : return 64;
      case "p16_16": return 32;
      case "if"    :
      case "p16_32": return 48;
      default      : return -1;
    }
  }

  // Get size of a relative displacement [in bits].
  static relSize(s) {
    switch (s) {
      case "rel8"  : return 8;
      case "rel16" : return 16;
      case "rel32" : return 32;
      default      : return -1;
    }
  }
}
x86.Utils = Utils;

// ============================================================================
// [asmdb.x86.Operand]
// ============================================================================

// X86/X64 operand.
class Operand extends base.Operand {
  constructor(data, defaultAccess) {
    super(data);

    this.memSeg = "";          // Segment specified with register that is used to perform a memory IO.
    this.memOff = false;       // Memory operand is an absolute offset (only a specific version of MOV).
    this.memFar = false;       // Memory is a far pointer (includes segment in first two bytes).
    this.vsibReg = "";         // AVX VSIB register type (xmm/ymm/zmm).
    this.vsibSize = -1;        // AVX VSIB register size (32/64).
    this.bcstSize = -1;        // AVX-512 broadcast size.

    const type = [];
    var s = data;

    // Handle RWX decorators prefix "[RWwXx]:".
    const mAccess = /^([RWwXx])\:/.exec(s);
    if (mAccess) {
      this.setAccess(mAccess[1]);
      s = s.substr(mAccess[0].length);
    }

    // Handle commutativity attribute.
    if (Parsing.isCommutative(s)) {
      this.commutative = true;
      s = Parsing.clearCommutative(s);
    }

    // Handle AVX-512 broadcast possibility specified as "/bN" suffix.
    const mBcst = /\/b(\d+)/.exec(s);
    if (mBcst) {
      this.bcstSize = parseInt(mBcst[1], 10);

      // Remove the broadcast attribute from the definition; it's not needed anymore.
      s = s.substr(0, mBcst.index) + s.substr(mBcst.index + mBcst[0].length);
    }

    // Handle <implicit> attribute.
    if (Parsing.isImplicit(s)) {
      this.implicit = true;
      s = Parsing.clearImplicit(s);
    }

    // Support multiple operands separated by "/" (only used by r/m and i/u).
    var ops = s.split("/");
    var oArr = [];

    for (var i = 0; i < ops.length; i++) {
      var origOp = ops[i].trim();
      var op = origOp;

      // Handle range suffix [A] or [A:B]:
      const mRange = /\[(\d+)\s*(?:\:\s*(\d+)\s*)?\]$/.exec(op);
      if (mRange) {
        var a = parseInt(mRange[1], 10);
        var b = parseInt(mRange[2] || String(a), 10);

        if (a < b)
          FAIL(`Operand '${origOp}' contains invalid range '[${a}:${b}]'`)

        this.rwxIndex = b;
        this.rwxWidth = a - b + 1;

        op = op.substr(0, op.length - mRange[0].length);
      }

      oArr.push(op);

      // Handle a segment specification if this is an implicit register performing
      // memory access.
      if (/^(?:ds|es)\:/.test(op)) {
        this.memSeg = op.substr(0, 2);
        op = op.substr(3);
      }

      var regIndexRel = 0;
      if (op.endsWith("+1") || op.endsWith("+2") || op.endsWith("+3")) {
        regIndexRel = parseInt(op.substr(op.length - 1, 1));
        op = op.substring(0, op.length - 2);
      }

      if (Utils.isRegOp(op)) {
        this.reg = op;
        this.regType = Utils.regTypeOf(op);
        this.regIndexRel = regIndexRel;

        type.push("reg");
        continue;
      }

      if (Utils.isMemOp(op)) {
        this.mem = op;

        // Handle memory size.
        const mOff = /^m(?:off)?(\d+)/.exec(op);
        this.memSize = mOff ? parseInt(mOff[1], 10) : 0;
        this.memOff = op.indexOf("moff") === 0;

        const mSeg = /^m16_(\d+)/.exec(op);
        if (mSeg) {
          this.memFar = true;
          this.memSize = parseInt(mSeg[1], 10) + 16;
        }

        // Handle vector addressing mode and size "vmXXr".
        const mVM = /^vm(\d+)(x|y|z)$/.exec(op);
        if (mVM) {
          this.vsibReg = mVM[2] + "mm";
          this.vsibSize = parseInt(mVM[1], 10);
        }

        type.push("mem");
        continue;
      }

      if (Utils.isImmOp(op)) {
        const size = Utils.immSize(op);
        if (!this.imm)
          this.imm = size;
        else if (this.imm !== size)
          FAIL(`Immediate size mismatch: ${this.imm} != ${size}`);

        // Sign-extend / zero-extend.
        const sign = op.startsWith("i") ? "signed" : "unsigned";

        if (!this.immSign)
          this.immSign = sign;
        else if (this.immType !== sign)
          this.immSign = "any";

        if (op === "1") {
          this.immValue = 1;
          this.implicit = true;
        }

        if (type.indexOf("imm") !== -1)
          type.push("imm");
        continue;
      }

      if (Utils.isRelOp(op)) {
        this.rel = Utils.relSize(op);

        type.push("rel");
        continue;
      }

      FAIL(`Operand '${origOp}' unhandled`);
    }

    // In case the data has been modified it's always better to use the stripped off
    // version as we have already processed and stored all the possible decorators.
    this.data = oArr.join("/");
    this.type = type.join("/");

    if (this.rwxIndex === -1) {
      const opSize = this.isReg() ? this.regSize :
                     this.isMem() ? this.memSize : -1;
      if (opSize !== -1) {
        this.rwxIndex = 0;
        this.rwxWidth = opSize;
      }
    }

    if (!mAccess && this.isRegOrMem())
      this.setAccess(defaultAccess);
  }

  get regSize() {
    return Utils.regSize(this.reg);
  }

  setAccess(x) {
    const u = x.toUpperCase();
    this.zext  = x === "W" || x === "X";
    this.read  = u === "R" || u === "X";
    this.write = u === "W" || u === "X";
    return this;
  }


  isFixedReg() { return this.reg && this.reg !== this.regType && this.reg !== "st(i)"; }
  isFixedMem() { return this.memSeg && this.isFixedReg(); }

  isPartialOp() {
    const maybePartial = this.regType === "r8"   ||
                         this.regType === "r8hi" ||
                         this.regType === "r16"  ||
                         this.regType === "xmm";
    return maybePartial && !this.zext;
  }

  toRegMem() {
    if (this.reg && this.mem)
      return this.reg + "/m";
    else if (this.mem && (this.vsibReg || /fp$|int$/.test(this.mem)))
      return this.mem;
    else if (this.mem)
      return "m";
    else
      return this.toString();
  }

  toString() { return this.data; }
}
x86.Operand = Operand;

// ============================================================================
// [asmdb.x86.Instruction]
// ============================================================================

// X86/X64 instruction.
class Instruction extends base.Instruction {
  constructor(db, name, operands, encoding, opcode, metadata) {
    super(db);

    this.name = name;
    this.privilege = "L3";     // Privilege level required to execute the instruction.
    this.prefix = "";          // Prefix - "", "3DNOW", "EVEX", "VEX", "XOP".
    this.opcodeHex = "";       // A single opcode byte as hexadecimal string "00-FF".

    this.l = "";               // Opcode L field (nothing, 128, 256, 512).
    this.w = "";               // Opcode W field.
    this.pp = "";              // Opcode PP part.
    this.mm = "";              // Opcode MM[MMM] part.
    this._67h = false;         // Instruction requires a size override prefix.

    this.modR = "";            // Instruction specific payload in ModRM byte (R part), specified as "/0..7".
    this.modRM = "";           // Instruction specific payload in ModRM byte (RM part), specified as another opcode byte.
    this.ri = false;           // Instruction opcode is combined with register, "XX+r" or "XX+i".
    this.rel = 0;              // Displacement ("cb", "cw", and "cd" parts).

    this.fpu = false;          // If the instruction is an FPU instruction.
    this.fpuTop = 0;           // FPU top index manipulation [-1, 0, 1, 2].

    this.vsibReg = "";         // AVX VSIB register type (xmm/ymm/zmm).
    this.vsibSize = -1;        // AVX VSIB register size (32/64).

    this.broadcast = false;    // AVX-512 broadcast support.
    this.bcstSize = -1;        // AVX-512 broadcast size.

    this.kmask = false;        // AVX-512 merging {k}.
    this.zmask = false;        // AVX-512 zeroing {kz}, implies {k}.
    this.er = false;           // AVX-512 embedded rounding {er}, implies {sae}.
    this.sae = false;          // AVX-512 suppress all exceptions {sae} support.

    this.tupleType = "";       // AVX-512 tuple-type.
    this.elementSize = -1;     // Instruction's element size.

    this.consecutiveLead = 0;  // Consecutive register leading N other registers.

    this._assignOperands(operands);
    this._assignEncoding(encoding);
    this._assignOpcode(opcode);
    this._assignMetadata(metadata);

    this._updateOperandsInfo();
    this._postProcess();
  }

  _assignOperands(s) {
    if (!s) return;

    // First remove all flags specified as {...}. We put them into `flags`
    // map and mix with others. This seems to be the best we can do here.
    for (;;) {
      var a = s.indexOf("{");
      var b = s.indexOf("}");

      if (a === -1 || b === -1)
        break;

      // Get the `flag` and remove it from `s`.
      this._assignAttribute(s.substring(a + 1, b), true);
      s = s.substr(0, a) + s.substr(b + 1);
    }

    // Split into individual operands and push them to `operands`.
    const arr = Utils.splitOperands(s);
    for (var i = 0; i < arr.length; i++) {
      const operand = new Operand(arr[i].trim(), i === 0 ? "X" : "R");
      if (operand.mem == "tmem")
        this.attributes.Tsib = true;
      this.operands.push(operand);
    }
  }

  _assignEncoding(s) {
    // Parse 'TUPLE-TYPE' as defined by AVX-512.
    var i = s.indexOf("-");
    if (i !== -1) {
      this.tupleType = s.substr(i + 1);
      s = s.substr(0, i);
    }

    this.encoding = s;
  }

  _assignOpcode(s) {
    this.opcodeString = s;

    var parts = s.split(" ");
    var prefix, comp;
    var i;

    if (/^(EVEX|VEX|XOP)\./.test(s)) {
      // Parse VEX and EVEX encoded instruction.
      prefix = parts[0].split(".");

      for (i = 0; i < prefix.length; i++) {
        comp = prefix[i];

        // Ignore NP, it's just a placeholder.
        if (comp == "NP")
          continue;

        // Process "EVEX", "VEX", and "XOP" prefixes.
        if (/^(?:EVEX|VEX|XOP)$/.test(comp)) { this.prefix = comp; continue; }

        // Process `L` field.
        if (/^LIG$/      .test(comp)) { this.l = "LIG"; continue; }
        if (/^128|L0|LZ$/.test(comp)) { this.l = "128"; continue; }
        if (/^256|L1$/   .test(comp)) { this.l = "256"; continue; }
        if (/^512$/      .test(comp)) { this.l = "512"; continue; }

        // Process `PP` field - 66/F2/F3.
        if (comp === "P0") { /* ignored, `P` is zero... */ continue; }
        if (/^(?:66|F2|F3)$/.test(comp)) { this.pp = comp; continue; }

        // Process `MM` field - 0F/0F3A/0F38/MAP5/MAP6/M8/M9.
        if (/^(?:0F|0F3A|0F38|MAP5|MAP6|M08|M09|M0A)$/.test(comp)) { this.mm = comp; continue; }

        // Process `W` field.
        if (/^WIG|W0|W1$/.test(comp)) { this.w = comp; continue; }

        // ERROR.
        this.report(`'${this.opcodeString}' Unhandled component: ${comp}`);
      }

      for (i = 1; i < parts.length; i++) {
        comp = parts[i];

        // Parse opcode.
        if (/^[0-9A-Fa-f]{2}$/.test(comp)) {
          this.opcodeHex = comp.toUpperCase();
          continue;
        }

        // Parse "/r" or "/0-7".
        if (/^\/[r0-7]$/.test(comp)) {
          this.modR = comp.charAt(1);
          continue;
        }

        // Parse immediate byte, word, dword, or qword.
        if (/^(?:ib|iw|id|iq|\/is4)$/.test(comp)) {
          this.imm += Utils.immSize(comp);
          continue;
        }

        this.report(`'${this.opcodeString}' Unhandled opcode component: ${comp}`);
      }
    }
    else {
      // Parse X86/X64 instruction (including legacy MMX/SSE/3DNOW instructions).
      for (i = 0; i < parts.length; i++) {
        comp = parts[i];

        // Parse REX.W prefix.
        if (comp === "REX.W") {
          this.w = "W1";
          continue;
        }

        // Parse `PP` prefixes.
        if ((this.mm === "" && ((this.pp === ""   && /^(?:66|F2|F3)$/.test(comp)) ||
                                (this.pp === "66" && /^(?:F2|F3)$/   .test(comp))))) {
          this.pp += comp;
          continue;
        }

        // Parse `MM` prefixes.
        if ((this.mm === ""   && comp === "0F") ||
            (this.mm === "0F" && /^(?:01|3A|38)$/.test(comp))) {
          this.mm += comp;
          continue;
        }

        // Recognize "0F 0F /r XX" encoding.
        if (this.mm === "0F" && comp === "0F") {
          this.prefix = "3DNOW";
          continue;
        }

        // Parse opcode byte.
        if (/^[0-9A-F]{2}(?:\+[ri])?$/.test(comp)) {
          // Parse "+r" or "+i" suffix.
          if (comp.length > 2) {
            this.ri = true;
            comp = comp.substr(0, 2);
          }

          // FPU instructions are encoded as "PREFIX XX", where prefix is not the same
          // as MM prefixes used everywhere else. AsmJit internally extends MM field in
          // instruction tables to allow storing this prefix together with other "MM"
          // prefixes, currently the unused indexes are used, but if X86 moves forward
          // and starts using these we can simply use more bits in the opcode DWORD.
          if (!this.pp && this.opcodeHex === "9B") {
            this.pp = this.opcodeHex;
            this.opcodeHex = comp;
            continue;
          }

          if (!this.mm && (/^(?:D8|D9|DA|DB|DC|DD|DE|DF)$/.test(this.opcodeHex))) {
            this.mm = this.opcodeHex;
            this.opcodeHex = comp;
            continue;
          }

          if (this.opcodeHex) {
            if (this.opcodeHex === "67") {
              this._67h = true;
            }
            else {
              if (!this.modR && !this.modRM) {
                const value = parseInt(comp, 16);
                if ((value & 0xC0) == 0xC0) {
                  this.modR = String((value >> 3) & 0x7);
                  this.modRM = String((value >> 0) & 0x7);
                }
                else {
                  this.report(`'${this.opcodeString}' Unsupported secondary opcode (MOD/RM) '${comp}' value`);
                }
              }
              else {
                this.report(`'${this.opcodeString}' Multiple opcodes, have ${this.opcodeHex}, found ${comp}`);
              }
            }
          }

          this.opcodeHex = comp;
          continue;
        }

        // Parse "/r" or "/0-7".
        if (/^\/[r0-7]$/.test(comp) && !this.modR) {
          this.modR = comp.charAt(1);
          continue;
        }

        // Parse immediate byte, word, dword, fword, or qword.
        if (/^(?:ib|iw|id|iq|if)$/.test(comp)) {
          this.imm += Utils.immSize(comp);
          continue;
        }

        // Parse displacement.
        if (/^(?:cb|cw|cd)$/.test(comp) && !this.rel) {
          this.rel = comp === "cb" ? 1 :
                     comp === "cw" ? 2 :
                     comp === "cd" ? 4 : -1;
          continue;
        }

        // ERROR.
        this.report(`'${this.opcodeString}' Unhandled opcode component ${comp}`);
      }
    }

    // HACK: Fix instructions having opcode "01".
    if (this.opcodeHex === "" && this.mm.indexOf("0F01") === this.mm.length - 4) {
      this.opcodeHex = "01";
      this.mm = this.mm.substr(0, this.mm.length - 2);
    }

    if (this.opcodeHex)
      this.opcodeValue = parseInt(this.opcodeHex, 16);

    if (!this.opcodeHex)
      this.report(`Couldn't parse instruction's opcode '${this.opcodeString}'`);
  }

  _assignSpecificAttribute(name, value) {
    const db = this.db;

    // Basics.
    if (name == "X86" || name === "X64" || name === "ANY") {
      this.arch = name;
      return true;
    }

    // AVX-512 flag followed by "-VL" suffix is a combination of two extensions.
    if (/^AVX512\w+-VL$/.test(name) && db.extensions[name.substr(0, name.length - 3)]) {
      const ext = name.substr(0, name.length - 3);
      this.extensions[ext] = true;
      this.extensions.AVX512_VL = true;
      return true;
    }

    switch (name) {
      case "FPU":
        this.fpu = true;
        return true;

      case "kz":
        this.zmask = true;
        // fall: {kz} implies {k}.
      case "k":
        this.kmask = true;
        return true;

      case "er":
        this.er = true;
        // fall: {er} implies {sae}.
      case "sae":
        this.sae = true;
        return true;

      case "PRIVILEGE":
        if (!/^L[0123]$/.test(value))
          this.report(`${this.name}: Invalid privilege level '${value}'`);

        this.privilege = value;
        return true;

      case "broadcast":
        this.broadcast = true;
        this.elementSize = value;
        return true;

      case "FPU_PUSH" :
        this.fpu = true;
        this.fpuTop = -1;
        return true;

      case "FPU_POP":
        this.fpu = true;
        this.fpuTop = Number(value);
        return true;

      case "FPU_TOP":
        this.fpu = true;
        if (value === "-1") { this.fpuTop =-1; return true; }
        if (value === "+1") { this.fpuTop = 1; return true; }
        break;
    }

    return false;
  }

  _updateOperandsInfo() {
    super._updateOperandsInfo();

    var consecutiveLead = null;
    var consecutiveLastIndex = 0;

    for (var i = 0; i < this.operands.length; i++) {
      const op = this.operands[i];

      // Propagate broadcast.
      if (op.bcstSize > 0)
        this._assignAttribute("broadcast", op.bcstSize);

      // Propagate VSIB.
      if (op.vsibReg) {
        if (this.vsibReg) {
          this.report("Only one operand can be a vector memory address (vmNNx)");
        }

        this.vsibReg = op.vsibReg;
        this.vsibSize = op.vsibSize;
      }

      if (op.regIndexRel) {
        if (i - op.regIndexRel < 0) {
          this.report(`The consecutive register information is invalid, index of the lead (${i - op.regIndexRel}) is out of range`);
        }
        else {
          const lead = this.operands[i - op.regIndexRel];
          if (consecutiveLead && consecutiveLead != lead) {
            this.report(`The consecutive register chain is invalid`);
          }
          else {
            consecutiveLead = lead;
            consecutiveLastIndex = Math.max(consecutiveLastIndex, op.regIndexRel);
          }
        }
      }
    }

    if (consecutiveLead) {
      consecutiveLead.consecutiveLeadCount = consecutiveLastIndex + 1;
    }
  }

  // Validate the instruction's definition. Common mistakes can be checked and
  // reported easily, however, if the mistake is just an invalid opcode or
  // something else it's impossible to detect.
  _postProcess() {
    var isValid = true;
    var immCount = this.immCount;

    var m;

    // Verify that the immediate operand/operands are specified in instruction
    // encoding and opcode field. Basically if there is an "ix" in operands,
    // the encoding should contain "I".
    if (immCount > 0) {
      if (immCount === 1 && this.operands[this.operands.length - 1].data === "1") {
        // This must be one of rcl|rcr|rol|ror|sar|sal|shr. We won't validate
        // these as these have "1" as implicit (encoded within opcode, not after).
      }
      else {
        var immEncoding = "I".repeat(immCount);

        // "I" or "II" should be part of the encoding.
        if (this.encoding.indexOf(immEncoding) === -1) {
          isValid = false;
          this.report(`Immediate(s) [${immCount}] missing in encoding: ${this.encoding}`);
        }

        // Every immediate should have its imm byte ("ib", "iw", "id", or "iq") in the opcode data.
        m = this.opcodeString.match(/(?:^|\s+)(ib|iw|id|if|iq|\/is4)/g);
        if (!m || m.length !== immCount) {
          isValid = false;
          this.report(`Immediate(s) [${immCount}] not found in opcode: ${this.opcodeString}`);
        }
      }
    }

    // Verify that AVX/XOP or AVX-512 instruction always specifies L and W fields.
    // FIXME: Not passing, because Intel Manual sometimes doesn't specify W.
    /*
    if (this.isAVX() && (this.l === "" || this.w === "")) {
      this.report(`AVX instruction should specify L and W fields: L=${this.l} W=${this.w}`);
    }
    */

    // Verify that if the instruction uses the "VVVV" part of VEX/EVEX prefix,
    // that it has "NDS/NDD/DDS" part of the "VVVV" definition specified, and
    // that the definition matches the opcode encoding.
  }

  isAVX() { return this.isVEX() || this.isEVEX(); }
  isVEX() { return this.prefix === "VEX" || this.prefix === "XOP"; }
  isEVEX() { return this.prefix === "EVEX" }

  getWValue() {
    switch (this.w) {
      case "W0": return 0;
      case "W1": return 1;
    }
    return -1;
  }

  // Get signature of the instruction as "ARCH PREFIX ENCODING[:operands]" form.
  get signature() {
    var operands = this.operands;
    var sign = this.arch;

    if (this.prefix) {
      sign += " " + this.prefix;
      if (this.prefix !== "3DNOW") {
        if (this.l === "L1")
          sign += ".256";
        else if (this.l === "256" || this.l === "512")
          sign += `.${this.l}`;
        else
          sign += ".128";

        if (this.w === "W1")
          sign += ".W";
      }
    }
    else if (this.w === "W1") {
      sign += " REX.W";
    }

    sign += " " + this.encoding;

    for (var i = 0; i < operands.length; i++) {
      sign += (i === 0) ? ":" : ",";

      var operand = operands[i];
      if (operand.implicit)
        sign += `[${operand.reg}]`;
      else
        sign += operand.toRegMem();
    }

    return sign;
  }

  get immCount() {
    var ops = this.operands;
    var n = 0;
    for (var i = 0; i < ops.length; i++)
      if (ops[i].isImm())
        n++;
    return n;
  }

  get modRValue() {
    if (/^[0-7]$/.test(this.modR))
      return parseInt(this.modR, 10);
    else
      return 0;
  }

  get modRMValue() {
    if (/^[0-7]$/.test(this.modRM))
      return parseInt(this.modRM, 10);
    else
      return 0;
  }
}
x86.Instruction = Instruction;

// ============================================================================
// [asmdb.x86.ISA]
// ============================================================================

// X86/X64 instruction database - stores Instruction instances in a map and
// aggregates all instructions with the same name.
class ISA extends base.ISA {
  constructor(args) {
    super(args);

    if (!args)
      args = NONE;

    if (args.builtins !== false)
      this.addData(x86data);

    this.addData(args);
  }

  _addInstructions(instructions) {
    for (var i = 0; i < instructions.length; i++) {
      // Legacy tables (X86).
      const tuple = instructions[i];
      const names = tuple[kIndexName].split("/");

      for (var j = 0; j < names.length; j++) {
        const inst = new Instruction(this, names[j], tuple[kIndexOperands], tuple[kIndexEncoding], tuple[kIndexOpcode], tuple[kIndexMetadata]);
        if (j > 0)
          inst.attributes.AliasOf = names[0];
        this._addInstruction(inst);
      }
    }

    return this;
  }
}
x86.ISA = ISA;

// ============================================================================
// [asmdb.x86.X86DataCheck]
// ============================================================================

class X86DataCheck {
  static checkVexEvex(db) {
    const map = db.instructionMap;
    for (var name in map) {
      const insts = map[name];
      for (var i = 0; i < insts.length; i++) {
        const instA = insts[i];
        for (var j = i + 1; j < insts.length; j++) {
          const instB = insts[j];
          if (instA.operands.join("_") === instB.operands.join("_")) {
            const vex  = instA.prefix === "VEX"  ? instA : instB.prefix === "VEX"  ? instB : null;
            const evex = instA.prefix === "EVEX" ? instA : instB.prefix === "EVEX" ? instB : null;

            if (vex && evex && vex.opcodeHex === evex.opcodeHex) {
              // NOTE: There are some false positives, they will be printed as well.
              var ok = vex.w === evex.w && vex.l === evex.l;

              if (!ok) {
                console.log(`Instruction ${name} differs:`);
                console.log(`  ${vex.operands.join(" ")}: ${vex.opcodeString}`);
                console.log(`  ${evex.operands.join(" ")}: ${evex.opcodeString}`);
              }
            }
          }
        }
      }
    }
  }
}
x86.X86DataCheck = X86DataCheck;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "x86"]);
