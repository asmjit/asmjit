// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

(function($scope, $as) {
"use strict";

const exp = require("./exp.js");

function FAIL(msg) { throw new Error("[BASE] " + msg); }

// Import.
const hasOwn = Object.prototype.hasOwnProperty;

// Export.
const base = $scope[$as] = Object.create(null);

base.exp = exp;

function dict() { return Object.create(null); }
base.dict = dict;
const NONE = base.NONE = Object.freeze(dict());

// ============================================================================
// [asmdb.base.Symbols]
// ============================================================================

const Symbols = Object.freeze({
  Commutative: '~'
});
base.Symbols = Symbols;

// ============================================================================
// [asmdb.base.Parsing]
// ============================================================================

// Namespace that provides functions related to text parsing.
const Parsing = {
  // Get whether the string `s` representing an operand is <implicit>.
  isImplicit: function(s) { return s.startsWith("<") && s.endsWith(">"); },

  // Clear <implicit> attribute from the given operand string `s`.
  clearImplicit: function(s) { return s.substring(1, s.length - 1); },

  // Get whether the string `s` representing an operand is {optional}.
  isOptional: function(s) { return s.startsWith("{") && s.endsWith("}"); },

  // Clear {optional} attribute from the given operand string `s`.
  clearOptional: function(s) { return s.substring(1, s.length - 1); },

  // Get whether the string `s` representing an operand specifies commutativity.
  isCommutative: function(s) { return s.length > 0 && s.charAt(0) === Symbols.Commutative; },

  // Clear commutative attribute from the given operand string `s`.
  clearCommutative: function(s) { return s.substring(1); },

  // Matches a closing bracket in string `s` starting `from` the given index.
  // It behaves like `s.indexOf()`, but uses a counter and skips all nested
  // matches.
  matchClosingChar: function(s, from) {
    const len = s.length;
    const opening = s.charCodeAt(from);
    const closing = opening === 40  ? 31  :    // ().
                    opening === 60  ? 62  :    // <>.
                    opening === 91  ? 93  :    // [].
                    opening === 123 ? 125 : 0; // {}.

    let i = from;
    let pending = 1;
    do {
      if (++i >= len)
        break;

      const c = s.charCodeAt(i);
      pending += Number(c === opening);
      pending -= Number(c === closing);
    } while (pending);

    return i;
  },

  // Split instruction operands into an array containing each operand as a
  // trimmed string. This function is similar to `s.split(",")`, however,
  // it matches brackets inside the operands and won't just blindly split
  // the string based on "," token. If operand contains metadata or it's
  // an address it would still be split correctly.
  splitOperands: function(s) {
    const result = [];

    s = s.trim();
    if (!s)
      return result;

    let start = 0;
    let i = 0;
    let c = "";

    for (;;) {
      if (i === s.length || (c = s[i]) === ",") {
        const op = s.substring(start, i).trim();
        if (!op)
          FAIL(`Found empty operand in '${s}'`);

        result.push(op);
        if (i === s.length)
          return result;

        start = ++i;
        continue;
      }

      if ((c === "<" || c === ">") && i != start) {
        i++;
        continue;
      }

      if (c === "[" || c === "{" || c === "(" || c === "<")
        i = base.Parsing.matchClosingChar(s, i);
      else
        i++;
    }
  }
}
base.Parsing = Parsing;

// ============================================================================
// [asmdb.base.Operand]
// ============================================================================

const OperandFlags = Object.freeze({
  Optional   : 0x00000001,
  Implicit   : 0x00000002,
  Commutative: 0x00000004,
  ZExt       : 0x00000008,
  ReadAccess : 0x00000010,
  WriteAccess: 0x00000020
});
base.OperandFlags = OperandFlags;

class Operand {
  constructor(data) {
    this.type = "";              // Type of the operand ("reg", "reg-list", "mem", "reg/mem", "imm", "rel").
    this.data = data;            // The operand's data (possibly processed).
    this.flags = 0;

    this.reg = "";               // Register operand's definition.
    this.mem = "";               // Memory operand's definition.
    this.imm = 0;                // Immediate operand's size.
    this.rel = 0;                // Relative displacement operand's size.

    this.restrict = "";          // Operand is restricted (specific register or immediate value).
    this.read = false;           // True if the operand is a read-op from reg/mem.
    this.write = false;          // True if the operand is a write-op to reg/mem.

    this.regType = "";           // Register operand's type.
    this.regIndexRel = 0;        // Register index is relative to the previous register operand index (0 if not).
    this.memSize = -1;           // Memory operand's size.
    this.immSign = "";           // Immediate sign (any / signed / unsigned).
    this.immValue = null;        // Immediate value - `null` or `1` (only used by shift/rotate instructions).

    this.rwxIndex = -1;          // Read/Write (RWX) index.
    this.rwxWidth = -1;          // Read/Write (RWX) width.
  }

  _getFlag(flag) {
    return (this.flags & flag) != 0;
  }

  _setFlag(flag, value) {
    this.flags = (this.flags & ~flag) | (value ? flag : 0);
    return this;
  }

  get optional() { return this._getFlag(OperandFlags.Optional); }
  set optional(value) { this._setFlag(OperandFlags.Optional, value); }

  get implicit() { return this._getFlag(OperandFlags.Implicit); }
  set implicit(value) { this._setFlag(OperandFlags.Implicit, value); }

  get commutative() { return this._getFlag(OperandFlags.Commutative); }
  set commutative(value) { this._setFlag(OperandFlags.Commutative, value); }

  get zext() { return this._getFlag(OperandFlags.ZExt); }
  set zext(value) { this._setFlag(OperandFlags.ZExt, value); }

  toString() { return this.data; }

  isReg() { return !!this.reg; }
  isMem() { return !!this.mem; }
  isImm() { return !!this.imm; }
  isRel() { return !!this.rel; }

  isRegMem() { return this.reg && this.mem; }
  isRegOrMem() { return !!this.reg || !!this.mem; }

  isRegList() { return this.type === "reg-list" }
  isPartialOp() { return false; }
}
base.Operand = Operand;

// ============================================================================
// [asmdb.base.Instruction]
// ============================================================================

// Defines interface and properties that each architecture dependent instruction
// must provide even if that particular architecture doesn't use that feature(s).
class Instruction {
  constructor(db) {
    Object.defineProperty(this, "db", { value: db });

    this.name = "";            // Instruction name.
    this.arch = "ANY";         // Architecture.
    this.encoding = "";        // Encoding type.

    this.implicit = 0;         // Indexes of all implicit operands (registers / memory).
    this.commutative = 0;      // Indexes of all commutative operands.

    this.privilege = "";       // Privilege-level required to execute the instruction.

    this.opcodeString = "";    // Instruction opcode as specified in manual.
    this.opcodeValue = 0;      // Instruction opcode as number (arch dependent).
    this.fields = dict();      // Information about each opcode field (arch dependent).
    this.operations = dict();  // Operations the instruction performs.
    this.extensions = dict();  // Extensions required by the instruction.
    this.attributes = dict();  // Instruction attributes from metadata & opcode.
    this.specialRegs = dict(); // Information about read/write to special registers.

    this.operands = [];        // Instruction operands.
  }

  _assignMetadata(s) {
    // Split into individual attributes (separated by spaces).
    const attributes = s.trim().split(/[ ]+/);
    const shortcuts = this.db.shortcuts;

    for (let i = 0; i < attributes.length; i++) {
      const attr = attributes[i].trim();
      if (!attr) continue;

      const eq = attr.indexOf("=");
      let key = eq === -1 ? attr : attr.substr(0, eq);
      let val = eq === -1 ? "TRUE" : attr.substr(eq + 1);

      // apply shortcut, if defined.
      const shortcut = shortcuts[key];
      if (shortcut)
        key = shortcut.expand;

      // If the key contains "|" it's a definition of multiple attributes.
      if (key.indexOf("|") !== -1) {
        const dot = key.indexOf(".");

        const base = dot === -1 ? "" : key.substr(0, dot + 1);
        const keys = (dot === -1 ? key : key.substr(dot + 1)).split("|");

        for (let j = 0; j < keys.length; j++)
          this._assignAttribute(base + keys[j], val);
      }
      else {
        this._assignAttribute(key, val);
      }
    }
  }

  _assignAttribute(key, value) {
    if (key == "Op") {
      let arr = null;
      if (Array.isArray(value))
        arr = value;
      else if (typeof value === "string")
        arr = value.split("|");
      else
        return this.report(`Unhandled operation ${key}=${value}`);

      for (let i = 0; i < arr.length; i++) {
        const op = arr[i].trim();
        if (op) this.operations[value] = true;
      }

      return;
    }

    if (this._assignSimpleAttribute(key, value))
      return;

    if (this._assignSpecificAttribute(key, value))
      return;

    this.report(`Unhandled flag ${key}=${value}`);
  }

  _assignSimpleAttribute(key, value) {
    const db = this.db;

    const attributeDef = db.attributes[key];
    const specialRegDef = db.specialRegs[key];
    const extensionDef = db.extensions[key];

    if (attributeDef) {
      switch (attributeDef.type) {
        case "flag":
          value = String(value).toUpperCase() === "TRUE" ? true : false;
          break;

        case "string":
          value = String(value);
          break;

        case "string[]":
          value = String(value).split("|");
          break;

        default:
          FAIL(`Unknown attribute type ${attributeDef.type}`);
      }

      this.attributes[key] = value;
      return true;
    }

    // Probe both `specialRegs` and `extension` as they may specify the same thing.
    let result = false;

    if (specialRegDef) {
      if (typeof value !== "string" || !/^[RWXU01]$/.test(value))
        this.report(`Special register must specify 'R|W|X|U|0|1', not ${value}`);

      this.specialRegs[specialRegDef.name] = value;
      result = true;
    }

    if (extensionDef) {
      this.extensions[key] = true;
      result = true;
    }

    return result;
  }

  _assignSpecificAttribute(key, value) {
    return false;
  }

  _updateOperandsInfo() {
    this.implicit = 0;
    this.commutative = 0;

    for (let i = 0; i < this.operands.length; i++) {
      const op = this.operands[i];

      if (op.implicit) this.implicit |= (1 << i);
      if (op.commutative) this.commutative |= (1 << i);
    }
  }

  isAlias() { return !!this.attributes.AliasOf; }
  isCommutative() { return this.commutative !== 0; }

  hasImplicit() { return this.implicit !== 0; }

  hasAttribute(name, matchValue) {
    const value = this.attributes[name];
    if (value === undefined)
      return false;

    if (matchValue === undefined)
      return true;

    return value === matchValue;
  }

  report(msg) {
    console.log(`${this}: ${msg}`);
  }

  toString() {
    return `${this.name} ${this.operands.join(", ")}`;
  }
}
base.Instruction = Instruction;

// ============================================================================
// [asmdb.base.InstructionGroup]
// ============================================================================

// Instruction group is simply array of function that has some additional
// functionality.
class InstructionGroup extends Array {
  constructor() {
    super();

    if (arguments.length === 1) {
      const a = arguments[0];
      if (Array.isArray(a)) {
        for (let i = 0; i < a.length; i++)
          this.push(a[i]);
      }
    }
  }

  unionCpuFeatures(name) {
    const result = dict();
    for (let i = 0; i < this.length; i++) {
      const inst = this[i];
      const features = inst.extensions;
      for (let k in features)
        result[k] = features[k];
    }
    return result;
  }

  checkAttribute(name, value) {
    let n = 0;
    for (let i = 0; i < this.length; i++)
      n += Number(this[i].hasAttribute(name, value));
    return n;
  }
}
base.InstructionGroup = InstructionGroup;

const EmptyInstructionGroup = Object.freeze(new InstructionGroup());

// ============================================================================
// [asmdb.base.ISA]
// ============================================================================

class ISA {
  constructor() {
    this._instructions = null;           // Instruction array (contains all instructions).
    this._instructionNames = null;       // Instruction names (sorted), regenerated when needed.
    this._instructionMap = dict();       // Instruction name to `Instruction[]` mapping.
    this._aliases = dict();              // Instruction aliases.
    this._cpuLevels = dict();            // Architecture versions.
    this._extensions = dict();           // Architecture extensions.
    this._attributes = dict();           // Instruction attributes.
    this._specialRegs = dict();          // Special registers.
    this._shortcuts = dict();            // Shortcuts used by instructions metadata.
    this.stats = {
      insts : 0,                         // Number of all instructions.
      groups: 0                          // Number of grouped instructions (having unique name).
    };
  }

  get instructions() {
    let array = this._instructions;
    if (array === null) {
      array = [];
      const map = this.instructionMap;
      const names = this.instructionNames;
      for (let i = 0; i < names.length; i++)
        array.push.apply(array, map[names[i]]);
      this._instructions = array;
    }
    return array;
  }

  get instructionNames() {
    let names = this._instructionNames;
    if (names === null) {
      names = Object.keys(this._instructionMap);
      names.sort();
      this._instructionNames = names;
    }
    return names;
  }

  get instructionMap() { return this._instructionMap; }
  get aliases() { return this._aliases; }
  get cpuLevels() { return this._cpuLevels; }
  get extensions() { return this._extensions; }
  get attributes() { return this._attributes; }
  get specialRegs() { return this._specialRegs; }
  get shortcuts() { return this._shortcuts; }

  query(args, copy) {
    if (typeof args !== "object" || !args || Array.isArray(args))
      return this._queryByName(args, copy);

    const filter = args.filter;
    if (filter)
      copy = false;

    let result = this._queryByName(args.name, copy);
    if (filter)
      result = result.filter(filter, args.filterThis);

    return result;
  }

  _queryByName(name, copy) {
    let result = EmptyInstructionGroup;
    const map = this._instructionMap;

    if (typeof name === "string") {
      const insts = map[name];
      if (insts) result = insts;
      return copy ? result.slice() : result;
    }

    if (Array.isArray(name)) {
      const names = name;
      for (let i = 0; i < names.length; i++) {
        const insts = map[names[i]];
        if (!insts) continue;

        if (result === EmptyInstructionGroup)
          result = new InstructionGroup();

        for (let j = 0; j < insts.length; j++)
          result.push(insts[j]);
      }
      return result;
    }

    result = this.instructions;
    return copy ? result.slice() : result;
  }

  forEachGroup(cb, thisArg) {
    const map = this._instructionMap;
    const names = this.instructionNames;

    for (let i = 0; i < names.length; i++) {
      const name = names[i];
      cb.call(thisArg, name, map[name]);
    }

    return this;
  }

  addData(data) {
    if (typeof data !== "object" || !data)
      FAIL("Data must be object");

    if (data.cpuLevels) this._addCpuLevels(data.cpuLevels);
    if (data.extensions) this._addExtensions(data.extensions);
    if (data.attributes) this._addAttributes(data.attributes);
    if (data.specialRegs) this._addSpecialRegs(data.specialRegs);
    if (data.shortcuts) this._addShortcuts(data.shortcuts);
    if (data.instructions) this._addInstructions(data.instructions);
  }

  _addCpuLevels(items) {
    if (!Array.isArray(items))
      FAIL("Property 'cpuLevels' must be array");

    for (let i = 0; i < items.length; i++) {
      const item = items[i];
      const name = item.name;

      const obj = {
        name: name
      };

      this._cpuLevels[name] = obj;
    }
  }

  _addExtensions(items) {
    if (!Array.isArray(items))
      FAIL("Property 'extensions' must be array");

    for (let i = 0; i < items.length; i++) {
      const item = items[i];
      const name = item.name;

      const obj = {
        name: name,
        from: item.from || ""
      };

      this._extensions[name] = obj;
    }
  }

  _addAttributes(items) {
    if (!Array.isArray(items))
      FAIL("Property 'attributes' must be array");

    for (let i = 0; i < items.length; i++) {
      const item = items[i];
      const name = item.name;
      const type = item.type;

      if (!/^(?:flag|string|string\[\])$/.test(type))
        FAIL(`Unknown attribute type '${type}'`);

      const obj = {
        name: name,
        type: type,
        doc : item.doc || ""
      };

      this._attributes[name] = obj;
    }
  }

  _addSpecialRegs(items) {
    if (!Array.isArray(items))
      FAIL("Property 'specialRegs' must be array");

    for (let i = 0; i < items.length; i++) {
      const item = items[i];
      const name = item.name;

      const obj = {
        name : name,
        group: item.group || name,
        doc  : item.doc || ""
      };

      this._specialRegs[name] = obj;
    }
  }

  _addShortcuts(items) {
    if (!Array.isArray(items))
      FAIL("Property 'shortcuts' must be array");

    for (let i = 0; i < items.length; i++) {
      const item = items[i];
      const name = item.name;
      const expand = item.expand;

      if (!name || !expand)
        FAIL("Shortcut must contain 'name' and 'expand' properties");

      const obj = {
        name  : name,
        expand: expand,
        doc   : item.doc || ""
      };

      this._shortcuts[name] = obj;
    }
  }

  _addInstructions(instructions) {
    FAIL("ISA._addInstructions() must be reimplemented");
  }

  _addInstruction(inst) {
    let group;

    if (hasOwn.call(this._instructionMap, inst.name)) {
      group = this._instructionMap[inst.name];
    }
    else {
      group = new InstructionGroup();
      this._instructionNames = null;
      this._instructionMap[inst.name] = group;
      this.stats.groups++;
    }

    if (inst.attributes.AliasOf)
      this._aliases[inst.name] = inst.attributes.AliasOf;

    group.push(inst);
    this.stats.insts++;
    this._instructions = null;

    return this;
  }
}
base.ISA = ISA;

}).apply(this, typeof module === "object" && module && module.exports
  ? [module, "exports"] : [this.asmdb || (this.asmdb = {}), "base"]);
