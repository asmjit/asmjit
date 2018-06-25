// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// ============================================================================
// tablegen-arm.js
// ============================================================================

"use strict";

const commons = require("./tablegen.js");
const hasOwn = Object.prototype.hasOwnProperty;

const asmdb = commons.asmdb;
const kIndent = commons.kIndent;
const IndexedArray = commons.IndexedArray;
const StringUtils = commons.StringUtils;

// ============================================================================
// [ArmDB]
// ============================================================================

// Create the ARM database.
const isa = new asmdb.arm.ISA();

/*
console.log(
  isa.query({
    filter: function(inst) {
      return !!inst.extensions.ASIMD;
    }
  })
);
//console.log(JSON.stringify(isa.instructionMap, null, 2));
*/

// ============================================================================
// [tablegen.arm.GenUtils]
// ============================================================================

class GenUtils {
  // Get a list of instructions based on `name` and optional `mode`.
  static query(name, mode) {
    const insts = isa.query(name);
    return !mode ? insts : insts.filter(function(inst) { return inst.arch === mode; });
  }

  static archOf(records) {
    var t16Arch = false;
    var t32Arch = false;
    var a32Arch = false;
    var a64Arch = false;

    for (var i = 0; i < records.length; i++) {
      const record = records[i];
      if (record.encoding === "T16") t16Arch = true;
      if (record.encoding === "T32") t32Arch = true;
      if (record.encoding === "A32") a32Arch = true;
      if (record.encoding === "A64") a64Arch = true;
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

  static featuresOf(records) {
    const exts = Object.create(null);
    for (var i = 0; i < records.length; i++) {
      const record = records[i];
      for (var k in record.extensions)
        exts[k] = true;
    }
    const arr =  Object.keys(exts);
    arr.sort();
    return arr;
  }
}

// ============================================================================
// [tablegen.arm.ArmTableGen]
// ============================================================================

class ArmTableGen extends commons.TableGen {
  constructor() {
    super("Arm");
  }

  // --------------------------------------------------------------------------
  // [Parse / Merge]
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
      const records = GenUtils.query(names[i]);

      this.addInst({
        id            : 0,             // Instruction id (numeric value).
        name          : name,          // Instruction name.
        enum          : enum_,         // Instruction enum without `kId` prefix.
        records       : records,       // Instruction records (asmdb query).

        familyType    : "kFamilyNone", // Family type.
        familyIndex   : 0,             // Index to a family-specific data.

        nameIndex     : -1,            // Instruction name-index.
        commonIndex   : -1
      });
    }

    console.log("Number of Instructions: " + this.insts.length);
  }

  merge() {
    var s = StringUtils.format(this.insts, "", false, function(inst) {
      return "INST(" +
        StringUtils.padLeft(inst.enum       , 16) + ", " +
        StringUtils.padLeft(inst.encoding   , 23) + ", " +
        StringUtils.padLeft(inst.opcode0    , 26) + ", " +
        StringUtils.padLeft(inst.nameIndex  ,  4) + ", " +
        StringUtils.padLeft(inst.commonIndex,  3) + ")";
    }) + "\n";
    return this.inject("instData", s, this.insts.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Hooks]
  // --------------------------------------------------------------------------

  onBeforeRun() {
    this.load([
      "src/asmjit/arm/armglobals.h"
      "src/asmjit/arm/arminstdb.cpp",
      "src/asmjit/arm/arminstdb.h"
    ]);
    this.parse();
  }

  onAfterRun() {
    this.merge();
    this.save();
    this.dumpTableSizes();
  }
}

// ============================================================================
// [tablegen.arm.ArmIdEnum]
// ============================================================================

class ArmIdEnum extends commons.IdEnum {
  constructor() {
    super("ArmIdEnum");
  }

  comment(inst) {
    var dbInsts = inst.dbInsts;
    if (!dbInsts) return "";

    var text = GenUtils.archOf(dbInsts);
    var features = GenUtils.featuresOf(dbInsts);

    if (features.length)
      text += " {" + features.join("|") + "}";
    return text;
  }
}

// ============================================================================
// [tablegen.arm.ArmNameTable]
// ============================================================================

class ArmNameTable extends commons.NameTable {
  constructor() {
    super("ArmNameTable");
  }
}

// ============================================================================
// [tablegen.arm.ArmCommonTable]
// ============================================================================

class ArmCommonTable extends commons.Task {
  constructor() {
    super("ArmCommonTable", [
      "ArmIdEnum",
      "ArmNameTable"
    ]);
  }

  run() {
    const insts = this.ctx.insts;
    const table = new IndexedArray();

    for (var i = 0; i < insts.length; i++) {
      const inst = insts[i];
      const item = "{ " + StringUtils.padLeft("0", 1) + "}";
      inst.commonIndex = table.addIndexed(item);
    }

    var s = `const ArmInst::CommonData ArmInstDB::commonData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`;
    return this.ctx.inject("commonData", StringUtils.disclaimer(s), table.length * 12);
  }
}

// ============================================================================
// [Main]
// ============================================================================

new ArmTableGen()
  .addTask(new ArmIdEnum())
  .addTask(new ArmNameTable())
  .addTask(new ArmCommonTable())
  .run();
