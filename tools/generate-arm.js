// [Generate-ARM]
//
// NOTE: This script relies on 'asmdb' package. Either install it by using
// node.js package manager (npm) or by copying/symlinking the whole asmdb
// directory as [asmjit]/tools/asmdb.
"use strict";

const base = require("./generate-base.js");
const hasOwn = Object.prototype.hasOwnProperty;
const kIndent = base.kIndent;
const StringUtils = base.StringUtils;

// ----------------------------------------------------------------------------
// [ArmDB]
// ----------------------------------------------------------------------------

// Create the ARM database.
const arm = base.asmdb.arm;
const isa = new arm.ISA();

console.log(
  isa.query({
    filter: function(inst) {
      return !!inst.extensions.ASIMD;
    }
  })
);

//console.log(JSON.stringify(isa.instructionMap, null, 2));

// ----------------------------------------------------------------------------
// [GenUtils]
// ----------------------------------------------------------------------------

class GenUtils {
  // Get a list of instructions based on `name` and optional `mode`.
  static instsOf(name, mode) {
    const insts = isa.query(name);
    return !mode ? insts : insts.filter(function(inst) { return inst.arch === mode; });
  }

  static archOf(group) {
    var t16Arch = false;
    var t32Arch = false;
    var a32Arch = false;
    var a64Arch = false;

    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      if (inst.encoding === "T16") t16Arch = true;
      if (inst.encoding === "T32") t32Arch = true;
      if (inst.encoding === "A32") a32Arch = true;
      if (inst.encoding === "A64") a64Arch = true;
    }

    var s = (t16Arch && !t32Arch) ? "T16" :
            (t32Arch && !t16Arch) ? "T32" :
            (t16Arch &&  t32Arch) ? "Txx" : "---";
    s += " ";
    s += (a32Arch) ? "A32" : "---";
    s += " ";
    s += (a64Arch) ? "A64" : "---";

    return `[${s}]`;
  }

  static featuresOf(group) {
    const exts = Object.create(null);
    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      for (var k in inst.extensions)
        exts[k] = true;
    }
    const arr =  Object.keys(exts);
    arr.sort();
    return arr;
  }
}

// ----------------------------------------------------------------------------
// [ArmGenerator]
// ----------------------------------------------------------------------------

class ArmGenerator extends base.BaseGenerator {
  constructor() {
    super("Arm");

    this.load([
      "src/asmjit/arm/arminst.cpp",
      "src/asmjit/arm/arminst.h"
    ]);
  }

  // --------------------------------------------------------------------------
  // [Parse]
  // --------------------------------------------------------------------------

  parse() {
    this.addInst({
      id: 0,
      name: "",
      enum: "None"
    });

    var names = isa.instructionNames;
    for (var i = 0; i < names.length; i++) {
      const name = names[i];

      const enum_ = StringUtils.upFirst(name);
      const insts = GenUtils.instsOf(names[i]);

      this.addInst({
        id            : 0,             // Instruction id (numeric value).
        name          : name,          // Instruction name.
        enum          : enum_,         // Instruction enum without `kId` prefix.

        familyType    : "kFamilyNone", // Family type.
        familyIndex   : 0,             // Index to a family-specific data.

        nameIndex     : -1,            // Instruction name-index.
        commonIndex   : -1
      });
    }

    console.log("Number of Instructions: " + this.instArray.length);
  }

  // --------------------------------------------------------------------------
  // [Generate]
  // --------------------------------------------------------------------------

  generate() {
    // Order doesn't matter here.
    this.generateIdData();
    this.generateNameData();

    // These must be last, and order matters.
    this.generateCommonData();
    this.generateInstData();

    return this;
  }

  // --------------------------------------------------------------------------
  // [Generate - CommonData]
  // --------------------------------------------------------------------------

  generateCommonData() {
    const table = new base.IndexedArray();
    for (var i = 0; i < this.instArray.length; i++) {
      const inst = this.instArray[i];

      const item = "{ " + StringUtils.padLeft("0", 1) + "}";
      inst.commonIndex = table.addIndexed(item);
    }

    var s = `const ArmInst::CommonData ArmInstDB::commonData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`;
    return this.inject("commonData", StringUtils.disclaimer(s), table.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Generate - InstData]
  // --------------------------------------------------------------------------

  generateInstData() {
    var s = StringUtils.format(this.instArray, "", false, function(inst) {
      return "INST(" +
        StringUtils.padLeft(inst.enum       , 16) + ", " +
        StringUtils.padLeft(inst.encoding   , 23) + ", " +
        StringUtils.padLeft(inst.opcode0    , 26) + ", " +
        StringUtils.padLeft(inst.nameIndex  ,  4) + ", " +
        StringUtils.padLeft(inst.commonIndex,  3) + ")";
    }) + "\n";
    return this.inject("instData", s, this.instArray.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Reimplement]
  // --------------------------------------------------------------------------

  getCommentOf(name) {
    var insts = GenUtils.instsOf(name);
    if (!insts) return "";

    var features = GenUtils.featuresOf(insts);
    var comment = GenUtils.archOf(insts);

    if (features.length)
      comment += " {" + features.join("|") + "}";

    return comment;
  }
}

// ----------------------------------------------------------------------------
// [Main]
// ----------------------------------------------------------------------------

function main() {
  const gen = new ArmGenerator();

  gen.parse();
  gen.generate();
  gen.dumpTableSizes();
  gen.save();
}
main();
