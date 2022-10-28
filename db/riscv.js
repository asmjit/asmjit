// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib


(function($scope, $as) {
"use strict";

function FAIL(msg) { throw new Error("[RISC-V] " + msg); }
function ASSERT(cond) { if (!cond) { FAIL(`Assertion failed.`) } }
function ASSERT_EQ(l, r) { if (l !== r) { FAIL(`Assertion failed. Left = '${l}', right = '${r}'`) } }
function ARRAY_CMP(l, r) { return JSON.stringify(l) === JSON.stringify(r); }

// Import
// ======

const base = $scope.base ? $scope.base : require("./base.js");
const riscvdata = $scope.riscvdata ? $scope.riscvdata : require("./riscvdata.js");

const hasOwn = Object.prototype.hasOwnProperty;
const dict = base.dict;
const NONE = base.NONE;
const Parsing = base.Parsing;

const exp = base.exp;

// Export
// ======

const riscv = $scope[$as] = dict();

// asmdb.riscv.Utils
// =================

// RISC-V utilities.
class Utils {
}
riscv.Utils = Utils;

function decomposeOperandSuffix(s) {
  ASSERT(s.length >= 1);

  let number_slice = s.slice(0, 1);

  ASSERT(!isNaN(number_slice));
  
  for (let i = 1; i < s.length; i++) {
    let slice = s.slice(0, i);
    if (!isNaN(slice)) {
      number_slice = slice
    } else {
      break;
    }
  }

  return [ Number(number_slice), s.slice(number_slice.length) ];
}

// asmdb.riscv.Operand
// ===================

// RISC-V operand.
class Operand extends base.Operand {
  constructor(def) {
    super(def);

    if (def === "rd" || def === "rs1" || def === "rs2") {
      this._handleReg(def)
    }
    else if (def.includes("imm")) {
      this._handleImm(def);
    }
    else if (!this._handleSpecial(def)) {
      FAIL(`unhandled operand '${def}'`)
    }
  }

  _handleReg(s) {
    this.type = "reg";
    
    // TODO: [RISC-V]
  }

  _handleImm(s) {
    this.type = "imm";

    let attr = s.split("imm");
    ASSERT_EQ(attr.length, 2);
    let [prefix, suffix] = [attr[0], attr[1]];

    if (prefix !== "") {
      if (prefix === "b") {
        // TODO: [RISC-V]
      }
      else if (prefix === "j") {
        // TODO: [RISC-V]
      }
      else {
        FAIL(`unhandled imm prefix. imm = '${s}', prefix = '${prefix}'`);
      }
    }

    if (suffix !== "") {
      // check if the characters of the suffix are all digits
      if (!isNaN(suffix)) {
        this.immSize = Number(suffix);
      }
      else {
        let [size, remaining] = decomposeOperandSuffix(suffix);

        this.immSize = size;

        if (remaining === "hi") {
          // TODO: [RISC-V]
        }
        else if (remaining === "lo") {
          // TODO: [RISC-V]
        }
        else {
          FAIL(`unhandled imm suffix. imm = '${s}', suffix = '${suffix}'`);
        }
      }
    }
  }

  _handleSpecial(s) {
    // Starts with '-' to indicate that it is a special operand

    if (s === "fm") {
      this.type = "-fm";
      // TODO: [RISC-V] in instruction 'fench', fence mode
    }
    else if (s === "pred") {
      this.type = "-pred";
      // TODO: [RISC-V] in instruction 'fench', predecessor
    }
    else if (s === "succ") {
      this.type = "-succ";
      // TODO: [RISC-V] in instruction 'fench', successor
    }
    else {
      return false;
    }
    return true;
  }

  // TODO: [RISC-V]
}
riscv.Operand = Operand;

// asmdb.riscv.Instruction
// =======================

// RISC-V instruction.
class Instruction extends base.Instruction {
  constructor(db, name, extensions, operands, [mask, match]) {
    super(db);

    this.name = name;
    this.extensions = extensions;
    this.mask = 0;
    this.match = 0;

    this._assignOperands(operands);
    this._assignOpcode(mask, match);
    this._assignEncoding();

    // TODO: [RISC-V] some fields of operands need to be filled, see the impl of this function for details
    // this._updateOperandsInfo();
  }

  _assignOperands(operands) {
    ASSERT(operands);
    this.operands = operands.map(def => new Operand(def));
  }

  _assignOpcode(mask, match) {
    ASSERT(!isNaN(mask) && !isNaN(match));

    this.mask = Number(mask);
    this.match = Number(match);
  }
  
  _assignEncoding() {
    let operands = this.operands.map(op => op.data);

    if (this.mask === 0xfe00707f && ARRAY_CMP(operands, ["rd", "rs1", "rs2"])) {
      this.encoding = "R";
    }
    else if (this.mask === 0x707f && ARRAY_CMP(operands, ["rd", "rs1", "imm12"])) {
      this.encoding = "I";
    }
    else if (this.mask === 0x7f && ARRAY_CMP(operands, ["rd", "imm20"])) {
      this.encoding = "U";
    }
    else if (this.mask === 0x707f && ARRAY_CMP(operands, ["bimm12hi", "rs1", "rs2", "bimm12lo"])) {
      this.encoding = "B";
    }
    else if (this.mask === 0x7f && ARRAY_CMP(operands, ["rd", "jimm20"])) {
      this.encoding = "J";
    }
    else if (this.mask === 0x707f && ARRAY_CMP(operands, ["imm12hi", "rs1", "rs2", "imm12lo"])) {
      this.encoding = "S";
    }
    // special cases ()
    else if ((this.name === "ebreak" || this.name === "ecall") && this.mask === 0xffffffff && ARRAY_CMP(operands, [])) {
      this.encoding = `I-${this.name}`;
    }
    else if (this.name === "fence" && this.mask === 0x707f && ARRAY_CMP(operands, ["fm", "pred", "succ", "rs1", "rd"])) {
      this.encoding = `-${this.name}`
    }
    else {
      FAIL(`unhandled encoding. name = '${this.name}', operand_names = '${operands}'`);
    }
  }
}
riscv.Instruction = Instruction;

// asmdb.riscv.ISA
// ===============

class ISA extends base.ISA {
  constructor(args) {
    super(args);

    if (!args)
      args = NONE;

    if (args.builtins !== false)
      this.addData(riscvdata);

    this.addData(args);
  }

  _addInstructions(instructions) {
    for (let [name, data] of Object.entries(instructions)) {
      const inst = new Instruction(this, name, data.extension, data.variable_fields, [data.mask, data.match]);
      this._addInstruction(inst);
    }

    return this;
  }
}
riscv.ISA = ISA;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "riscv"]);
