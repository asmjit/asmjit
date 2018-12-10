// [AsmJit]
// Complete x86/x64 JIT and Remote Assembler for C++.
//
// [License]
// ZLIB - See LICENSE.md file in the package.

// ============================================================================
// tablegen-x86.js
//
// The purpose of this script is to fetch all instructions' names into a single
// string and to optimize common patterns that appear in instruction data. It
// prevents relocation of small strings (instruction names) that has to be done
// by a linker to make all pointers the binary application/library uses valid.
// This approach decreases the final size of AsmJit binary and relocation data.
//
// NOTE: This script relies on 'asmdb' package. Either install it by using
// node.js package manager (npm) or by copying/symlinking the whole asmdb
// directory as [asmjit]/tools/asmdb.
// ============================================================================

"use strict";

const core = require("./tablegen.js");
const asmdb = core.asmdb;
const kIndent = core.kIndent;

const MapUtils = core.MapUtils;
const ArrayUtils = core.ArrayUtils;
const StringUtils = core.StringUtils;
const IndexedArray = core.IndexedArray;

const hasOwn = Object.prototype.hasOwnProperty;
const padLeft = StringUtils.padLeft;
const disclaimer = StringUtils.disclaimer;

const FAIL = core.FAIL;
const DEBUG = core.DEBUG;

// ============================================================================
// [tablegen.x86.x86isa]
// ============================================================================

// Create the X86 database and add some special cases recognized by AsmJit.
const x86isa = new asmdb.x86.ISA({
  instructions: [
    // Imul in [reg, imm] form is encoded as [reg, reg, imm].
    ["imul", "r16, ib"    , "RMI"  , "66 6B /r ib"        , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul", "r32, ib"    , "RMI"  , "6B /r ib"           , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul", "r64, ib"    , "RMI"  , "REX.W 6B /r ib"     , "X64 OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul", "r16, iw"    , "RMI"  , "66 69 /r iw"        , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul", "r32, id"    , "RMI"  , "69 /r id"           , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul", "r64, id"    , "RMI"  , "REX.W 69 /r id"     , "X64 OF=W SF=W ZF=U AF=U PF=U CF=W"]
  ]
});

// Remapped instructions contain mapping between instructions that AsmJit expects
// and instructions provided by asmdb. In general, AsmJit uses string instructions
// (like cmps, movs, etc...) without the suffix, so we just remap these and keep
// all others.
const RemappedInsts = {
  __proto__: null,

  "cmpsd": { names: ["cmpsd"]                           , rep: false },
  "movsd": { names: ["movsd"]                           , rep: false },
  "cmps" : { names: ["cmpsb", "cmpsw", "cmpsd", "cmpsq"], rep: true  },
  "movs" : { names: ["movsb", "movsw", "movsd", "movsq"], rep: true  },
  "lods" : { names: ["lodsb", "lodsw", "lodsd", "lodsq"], rep: null  },
  "scas" : { names: ["scasb", "scasw", "scasd", "scasq"], rep: null  },
  "stos" : { names: ["stosb", "stosw", "stosd", "stosq"], rep: null  },
  "ins"  : { names: ["insb" , "insw" , "insd" ]         , rep: null  },
  "outs" : { names: ["outsb", "outsw", "outsd"]         , rep: null  }
};

// Map of instructions that can use fixed registers, but are also encodable
// by using any others. This is to simplify some decisions about instruction
// flags as we don't want to see `FixedReg` in `adc` instruction, for example.
const NotFixedInsts = MapUtils.arrayToMap([
  "adc", "add", "and", "cmp", "mov", "or", "sbb", "sub", "test", "xchg", "xor"
]);

// ============================================================================
// [tablegen.x86.GenUtils]
// ============================================================================

class GenUtils {
  static hasFixedReg(dbInsts) {
    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      if (NotFixedInsts[dbInst.name]) continue;

      const operands = dbInst.operands;
      for (var j = 0; j < operands.length; j++)
        if (operands[j].isFixedReg())
          return true;
    }

    return false;
  }

  static hasFixedMem(dbInsts) {
    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      if (NotFixedInsts[dbInst.name]) continue;

      const operands = dbInst.operands;
      for (var j = 0; j < operands.length; j++)
        if (operands[j].isFixedMem())
          return true;
    }

    return false;
  }

  static cpuArchOf(dbInsts) {
    var anyArch = false;
    var x86Arch = false;
    var x64Arch = false;

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      if (dbInst.arch === "ANY") anyArch = true;
      if (dbInst.arch === "X86") x86Arch = true;
      if (dbInst.arch === "X64") x64Arch = true;
    }

    return anyArch || (x86Arch && x64Arch) ? "[ANY]" : x86Arch ? "[X86]" : "[X64]";
  }

  static cpuFeaturesOf(dbInsts) {
    return ArrayUtils.sorted(dbInsts.unionCpuFeatures());
  }

  static specialsOf(dbInsts) {
    const r = Object.create(null);
    const w = Object.create(null);

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const specialRegs = dbInst.specialRegs;

      // Mov is a special case, moving to/from control regs makes flags undefined,
      // which we don't want to have in `X86InstDB::operationData`. This is, thus,
      // a special case instruction analyzer must deal with.
      if (dbInst.name === "mov")
        continue;

      for (var specialReg in specialRegs) {
        const group = x86isa.specialRegs[specialReg].group;
        const op = specialRegs[specialReg];

        switch (op) {
          case "R":
            r[group] = true;
            break;
          case "X":
            r[group] = true;
            // ... fallthrough ...
          case "W":
          case "U":
          case "0":
          case "1":
            w[group] = true;
            break;
        }
      }
    }

    return [ArrayUtils.sorted(r), ArrayUtils.sorted(w)];
  }

  static flagsOf(dbInsts) {
    function getAccess(dbInst) {
      const operands = dbInst.operands;
      if (!operands.length) return "";

      if (dbInst.name === "xchg" || dbInst.name === "xadd")
        return "UseXX";

      const op = operands[0];
      if (!op.isRegOrMem())
        return "";
      else if (op.read && op.write)
        return "UseX";
      else
        return op.read ? "UseR" :"UseW";
    }

    function replace(map, a, b, c) {
      if (map[a] && map[b]) {
        delete map[a];
        delete map[b];
        map[c] = true;
      }
    }

    const f = Object.create(null);
    var i, j;

    var mib = dbInsts.length > 0 && /^(?:bndldx|bndstx)$/.test(dbInsts[0].name);
    var access = "";
    var ambiguous = false;

    for (i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const acc = getAccess(dbInst);

      if (!access)
        access = acc;
      else if (access !== acc)
        ambiguous = true;

      if (dbInst.attributes.Volatile) f.Volatile = true;
      if (dbInst.privilege !== "L3") f.Privileged = true;
    }

    // Default to "RO" if there is no access information nor operands.
    if (!access) access = "UseR";
    if (ambiguous) access = "UseA";
    if (access) {
      if (access === "UseXX")
        f.UseX = true;
      f[access] = true;
    }

    if (mib) f.Mib = true;

    const fixedReg = GenUtils.hasFixedReg(dbInsts);
    const fixedMem = GenUtils.hasFixedMem(dbInsts);

    if (fixedReg && fixedMem)
      f.FixedRM = true;
    else if (fixedReg)
      f.FixedReg = true;
    else if (fixedMem)
      f.FixedMem = true;

    var mmx = false;
    var vec = false;

    for (i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const operands = dbInst.operands;

      if (dbInst.name === "emms")
        mmx = true;

      if (dbInst.name === "vzeroall" || dbInst.name === "vzeroupper")
        vec = true;

      for (j = 0; j < operands.length; j++) {
        const op = operands[j];
        if (op.reg === "mm")
          mmx = true;
        else if (/^(k|xmm|ymm|zmm)$/.test(op.reg)) {
          vec = true;
        }
      }
    }

    if (mmx) f.Mmx = true;
    if (vec) f.Vec = true;

    for (i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const operands = dbInst.operands;

      if (dbInst.attributes.Lock      ) f.Lock       = true;
      if (dbInst.attributes.XAcquire  ) f.XAcquire   = true;
      if (dbInst.attributes.XRelease  ) f.XRelease   = true;
      if (dbInst.attributes.REP       ) f.Rep        = true;
      if (dbInst.attributes.REPNE     ) f.Rep        = true;
      if (dbInst.attributes.RepIgnored) f.RepIgnored = true;

      if (dbInst.fpu) {
        for (var j = 0; j < operands.length; j++) {
          const op = operands[j];
          if (op.memSize === 16) f.FpuM16 = true;
          if (op.memSize === 32) f.FpuM32 = true;
          if (op.memSize === 64) f.FpuM64 = true;
          if (op.memSize === 80) f.FpuM80 = true;
        }
      }

      if (dbInst.vsibReg)
        f.Vsib = true;

      if (dbInst.prefix === "VEX" || dbInst.prefix === "XOP")
        f.Vex = true;

      if (dbInst.prefix === "EVEX") {
        f.Evex = true;

        if (dbInst.kmask) f.Avx512K = true;
        if (dbInst.zmask) f.Avx512Z = true;

        if (dbInst.er) f.Avx512ER = true;
        if (dbInst.sae) f.Avx512SAE = true;

        if (dbInst.broadcast) f["Avx512B" + String(dbInst.elementSize)] = true;
        if (dbInst.tupleType === "T1_4X") f.Avx512T4X = true;
      }
    }

    replace(f, "Avx512K"        , "Avx512Z"     , "Avx512KZ");
    replace(f, "Avx512ER"       , "Avx512SAE"   , "Avx512ER_SAE");
    replace(f, "Avx512KZ"       , "Avx512SAE"   , "Avx512KZ_SAE");
    replace(f, "Avx512KZ"       , "Avx512ER_SAE", "Avx512KZ_ER_SAE");
    replace(f, "Avx512K"        , "Avx512B32"   , "Avx512K_B32");
    replace(f, "Avx512K"        , "Avx512B64"   , "Avx512K_B64");
    replace(f, "Avx512KZ"       , "Avx512B32"   , "Avx512KZ_B32");
    replace(f, "Avx512KZ"       , "Avx512B64"   , "Avx512KZ_B64");
    replace(f, "Avx512KZ_SAE"   , "Avx512B32"   , "Avx512KZ_SAE_B32");
    replace(f, "Avx512KZ_SAE"   , "Avx512B64"   , "Avx512KZ_SAE_B64");
    replace(f, "Avx512KZ_ER_SAE", "Avx512B32"   , "Avx512KZ_ER_SAE_B32");
    replace(f, "Avx512KZ_ER_SAE", "Avx512B64"   , "Avx512KZ_ER_SAE_B64");

    return Object.getOwnPropertyNames(f);
  }

  static operationFlagsOf(dbInsts) {
    const f = Object.create(null);

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const name = dbInst.name;
      const operands = dbInst.operands;
    }

    return Object.getOwnPropertyNames(f);
  }

  static specialCasesOf(dbInsts) {
    const f = Object.create(null);

    for (var i = 0; i < dbInsts.length; i++) {
      const dbInst = dbInsts[i];
      const name = dbInst.name;

      // Special case: MOV undefines flags if moving between GP and CR|DR registers.
      if (name === "mov")
        f.MovCrDr = true;

      // Special case: MOVSS|MOVSD zeroes the remaining part of destination if source operand is memory.
      if ((name === "movss" || name === "movsd") && !dbInst.attributes.REP)
        f.MovSsSd = true;
    }

    return Object.getOwnPropertyNames(f);
  }

  static eqOps(aOps, aFrom, bOps, bFrom) {
    var x = 0;
    for (;;) {
      const aIndex = x + aFrom;
      const bIndex = x + bFrom;

      const aOut = aIndex >= aOps.length;
      const bOut = bIndex >= bOps.length;

      if (aOut || bOut)
        return !!(aOut && bOut);

      const aOp = aOps[aIndex];
      const bOp = bOps[bIndex];

      if (aOp.data !== bOp.data)
        return false;

      x++;
    }
  }

  static singleRegCase(name) {
    switch (name) {
      case "xchg"    :

      case "and"     :
      case "pand"    : case "vpand"  : case "vpandd"  : case "vpandq"   :
      case "andpd"   : case "vandpd" :
      case "andps"   : case "vandps" :

      case "or"      :
      case "por"     : case "vpor"   : case "vpord"   : case "vporq"    :
      case "orpd"    : case "vorpd"  :
      case "orps"    : case "vorps"  :

      case "pminsb"  : case "vpminsb": case "pmaxsb"  : case "vpmaxsb"  :
      case "pminsw"  : case "vpminsw": case "pmaxsw"  : case "vpmaxsw"  :
      case "pminsd"  : case "vpminsd": case "pmaxsd"  : case "vpmaxsd"  :
      case "pminub"  : case "vpminub": case "pmaxub"  : case "vpmaxub"  :
      case "pminuw"  : case "vpminuw": case "pmaxuw"  : case "vpmaxuw"  :
      case "pminud"  : case "vpminud": case "pmaxud"  : case "vpmaxud"  :
        return "RO";

      case "pandn"   : case "vpandn" : case "vpandnd" : case "vpandnq"  :

      case "xor"     :
      case "pxor"    : case "vpxor"  : case "vpxord"  : case "vpxorq"   :
      case "xorpd"   : case "vxorpd" :
      case "xorps"   : case "vxorps" :

      case "sub"     :
      case "psubb"   : case "vpsubb" :
      case "psubw"   : case "vpsubw" :
      case "psubd"   : case "vpsubd" :
      case "psubq"   : case "vpsubq" :
      case "psubsb"  : case "vpsubsb": case "psubusb" : case "vpsubusb" :
      case "psubsw"  : case "vpsubsw": case "psubusw" : case "vpsubusw" :

      case "vpcmpeqb": case "pcmpeqb": case "vpcmpgtb": case "pcmpgtb"  :
      case "vpcmpeqw": case "pcmpeqw": case "vpcmpgtw": case "pcmpgtw"  :
      case "vpcmpeqd": case "pcmpeqd": case "vpcmpgtd": case "pcmpgtd"  :
      case "vpcmpeqq": case "pcmpeqq": case "vpcmpgtq": case "pcmpgtq"  :

      case "vpcmpb"  : case "vpcmpub":
      case "vpcmpd"  : case "vpcmpud":
      case "vpcmpw"  : case "vpcmpuw":
      case "vpcmpq"  : case "vpcmpuq":
        return "WO";

      default:
        return "None";
    }
  }

  static controlType(dbInsts) {
    if (dbInsts.checkAttribute("Control", "Jump")) return "Jump";
    if (dbInsts.checkAttribute("Control", "Call")) return "Call";
    if (dbInsts.checkAttribute("Control", "Branch")) return "Branch";
    if (dbInsts.checkAttribute("Control", "Return")) return "Return";
    return "None";
  }
}

// ============================================================================
// [tablegen.x86.X86TableGen]
// ============================================================================

class X86TableGen extends core.TableGen {
  constructor() {
    super("X86");
  }

  // --------------------------------------------------------------------------
  // [Query]
  // --------------------------------------------------------------------------

  // Get instructions (dbInsts) having the same name as understood by AsmJit.
  query(name) {
    const remapped = RemappedInsts[name];
    if (!remapped) return x86isa.query(name);

    const dbInsts = x86isa.query(remapped.names);
    const rep = remapped.rep;
    if (rep === null) return dbInsts;

    return dbInsts.filter((inst) => {
      return rep === !!(inst.attributes.REP || inst.attributes.REPNE);
    });
  }

  // --------------------------------------------------------------------------
  // [Parse / Merge]
  // --------------------------------------------------------------------------

  parse() {
    const data = this.dataOfFile("src/asmjit/x86/x86instdb.cpp");
    const re = new RegExp(
      "INST\\(" +
        "([A-Za-z0-9_]+)\\s*"              + "," +  // [01] Instruction.
        "([^,]+)"                          + "," +  // [02] Encoding.
        "(.{26}[^,]*)"                     + "," +  // [03] Opcode[0].
        "(.{26}[^,]*)"                     + "," +  // [04] Opcode[1].
        "([^,]+)"                          + "," +  // [05] Write-Index.
        "([^,]+)"                          + "," +  // [06] Write-Size.
        // --- autogenerated fields ---
        "([^\\)]+)"                        + "," +  // [07] NameIndex.
        "([^\\)]+)"                        + "," +  // [08] CommonDataIndex.
        "([^\\)]+)"                        + "\\)", // [09] OperationDataIndex.
      "g");

    var m;
    while ((m = re.exec(data)) !== null) {
      var enum_       = m[1];
      var name        = enum_ === "None" ? "" : enum_.toLowerCase();
      var encoding    = m[2].trim();
      var opcode0     = m[3].trim();
      var opcode1     = m[4].trim();
      var writeIndex  = StringUtils.trimLeft(m[5]);
      var writeSize   = StringUtils.trimLeft(m[6]);

      const dbInsts = this.query(name);
      if (name && !dbInsts.length)
        FAIL(`Instruction '${name}' not found in asmdb`);

      const flags         = GenUtils.flagsOf(dbInsts);
      const controlType   = GenUtils.controlType(dbInsts);
      const singleRegCase = GenUtils.singleRegCase(name);
      const specialCases  = GenUtils.specialCasesOf(dbInsts);

      this.addInst({
        id                : 0,             // Instruction id (numeric value).
        name              : name,          // Instruction name.
        enum              : enum_,         // Instruction enum without `kId` prefix.
        dbInsts           : dbInsts,       // All dbInsts returned from asmdb query.
        encoding          : encoding,      // Instruction encoding.
        opcode0           : opcode0,       // Primary opcode.
        opcode1           : opcode1,       // Secondary opcode.
        flags             : flags,
        writeIndex        : writeIndex,
        writeSize         : writeSize,
        signatures        : null,          // Instruction signatures.
        controlType       : controlType,
        singleRegCase     : singleRegCase,
        specialCases      : specialCases,

        nameIndex         : -1,            // Instruction name-index.
        altOpCodeIndex    : -1,            // Index to X86InstDB::altOpCodeTable.
        commonDataIndex   : -1,
        operationDataIndex: -1,

        rwInfoIndex       : -1,
        rwInfoCount       : -1,

        signatureIndex    : -1,
        signatureCount    : -1
      });
    }

    if (this.insts.length === 0)
      FAIL("X86TableGen.parse(): Invalid parsing regexp (no data parsed)");

    console.log("Number of Instructions: " + this.insts.length);
  }

  merge() {
    var s = StringUtils.format(this.insts, "", true, function(inst) {
      return "INST(" +
        padLeft(inst.enum              , 16) + ", " +
        padLeft(inst.encoding          , 19) + ", " +
        padLeft(inst.opcode0           , 26) + ", " +
        padLeft(inst.opcode1           , 26) + ", " +
        padLeft(inst.writeIndex        ,  1) + ", " +
        padLeft(inst.writeSize         ,  1) + ", " +
        padLeft(inst.nameIndex         ,  4) + ", " +
        padLeft(inst.commonDataIndex   ,  3) + ", " +
        padLeft(inst.operationDataIndex,  3) + ")";
    }) + "\n";
    this.inject("InstInfo", s, this.insts.length * 4);
  }

  // --------------------------------------------------------------------------
  // [Other]
  // --------------------------------------------------------------------------

  /*
  printMissing() {
    const ignored = MapUtils.arrayToMap([
      "cmpsb", "cmpsw", "cmpsd", "cmpsq",
      "lodsb", "lodsw", "lodsd", "lodsq",
      "movsb", "movsw", "movsd", "movsq",
      "scasb", "scasw", "scasd", "scasq",
      "stosb", "stosw", "stosd", "stosq",
      "insb" , "insw" , "insd" ,
      "outsb", "outsw", "outsd",
      "wait" // Maps to `fwait`, which AsmJit uses instead.
    ]);

    var out = "";
    x86isa.instructionNames.forEach(function(name) {
      var dbInsts = x86isa.query(name);
      if (!this.instMap[name] && ignored[name] !== true) {
        console.log(`MISSING INSTRUCTION '${name}'`);
        var inst = this.newInstFromGroup(dbInsts);
        if (inst) {
          out += "  INST(" +
            padLeft(inst.enum      , 16) + ", " +
            padLeft(inst.encoding  , 23) + ", " +
            padLeft(inst.opcode0   , 26) + ", " +
            padLeft(inst.opcode1   , 26) + ", " +
            padLeft(inst.writeIndex,  2) + ", " +
            padLeft(inst.writeSize ,  2) + ", " +
            padLeft("0"            ,  4) + ", " +
            padLeft("0"            ,  3) + ", " +
            padLeft("0"            ,  3) + ", " +
            padLeft("0"            ,  3) + "),\n";
        }
      }
    }, this);
    console.log(out);
  }

  newInstFromGroup(dbInsts) {
    function composeOpCode(obj) {
      return `${obj.type}(${obj.prefix},${obj.opcode},${obj.o},${obj.l},${obj.w},${obj.ew},${obj.en},${obj.tt})`;
    }

    function GetAccess(dbInst) {
      var operands = dbInst.operands;
      if (!operands.length) return "";

      var op = operands[0];
      if (op.read && op.write)
        return "RW";
      else if (op.read)
        return "RO";
      else
        return "WO";
    }

    var dbInst = dbInsts[0];

    var id       = this.insts.length;
    var name     = dbInst.name;
    var enum_    = name[0].toUpperCase() + name.substr(1);

    var opcode   = dbInst.opcodeHex;
    var rm       = dbInst.rm;
    var mm       = dbInst.mm;
    var pp       = dbInst.pp;
    var encoding = dbInst.encoding;
    var prefix   = dbInst.prefix;

    var access   = GetAccess(dbInst);

    var vexL     = undefined;
    var vexW     = undefined;
    var evexW    = undefined;

    for (var i = 1; i < dbInsts.length; i++) {
      dbInst = dbInsts[i];

      if (opcode   !== dbInst.opcode    ) return null;
      if (rm       !== dbInst.rm        ) return null;
      if (mm       !== dbInst.mm        ) return null;
      if (pp       !== dbInst.pp        ) return null;
      if (encoding !== dbInst.encoding  ) return null;
      if (prefix   !== dbInst.prefix    ) return null;
      if (access   !== GetAccess(dbInst)) return null;
    }

    var ppmm = padLeft(pp, 2).replace(/ /g, "0") +
               padLeft(mm, 4).replace(/ /g, "0") ;

    var composed = composeOpCode({
      type  : prefix === "VEX" || prefix === "EVEX" ? "V" : "O",
      prefix: ppmm,
      opcode: opcode,
      o     : rm === "r" ? "_" : (rm ? rm : "_"),
      l     : vexL !== undefined ? vexL : "_",
      w     : vexW !== undefined ? vexW : "_",
      ew    : evexW !== undefined ? vexEW : "_",
      en    : "_",
      tt    : "_  "
    });

    return {
      id                : id,
      name              : name,
      enum              : enum_,
      encoding          : encoding,
      opcode0           : composed,
      opcode1           : "0",
      writeIndex        : "0",
      writeSize         : "0",
      nameIndex         : -1,
      commonDataIndex   : -1,
      operationDataIndex: -1
    };
  }
  */

  // --------------------------------------------------------------------------
  // [Hooks]
  // --------------------------------------------------------------------------

  onBeforeRun() {
    this.load([
      "src/asmjit/x86/x86globals.h",
      "src/asmjit/x86/x86instdb.cpp",
      "src/asmjit/x86/x86instdb.h",
      "src/asmjit/x86/x86ssetoavx.cpp"
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
// [tablegen.x86.IdEnum]
// ============================================================================

class IdEnum extends core.IdEnum {
  constructor() {
    super("IdEnum");
  }

  comment(inst) {
    var dbInsts = inst.dbInsts;
    if (!dbInsts.length) return "";

    var text = GenUtils.cpuArchOf(dbInsts);
    var features = GenUtils.cpuFeaturesOf(dbInsts);

    if (features.length) {
      text += " {";

      const vl = features.indexOf("AVX512_VL");
      if (vl !== -1) features.splice(vl, 1);
      text += features.join("|");
      if (vl !== -1) text += "+VL";

      text += "}";
    }

    return text;
  }
}

// ============================================================================
// [tablegen.x86.NameTable]
// ============================================================================

class NameTable extends core.NameTable {
  constructor() {
    super("NameTable");
  }
}

// ============================================================================
// [tablegen.x86.EncodingTable]
// ============================================================================

class EncodingTable extends core.Task {
  constructor() {
    super("EncodingTable");
  }

  run() {
    const insts = this.ctx.insts;
    const table = insts.map((inst) => { return inst.encoding; });

    function map(encoding) { return `E(${encoding})`; }

    this.inject("EncodingTable",
                disclaimer(`#define E(VAL) InstDB::kEncoding##VAL\n` +
                           `const uint8_t InstDB::_encodingTable[] = {\n${StringUtils.format(table, kIndent, -1, map)}\n};\n` +
                           `#undef E\n`),
                table.length * 1);
  }
}

// ============================================================================
// [tablegen.x86.MainOpcodeTable]
// ============================================================================

class MainOpcodeTable extends core.Task {
  constructor() {
    super("MainOpcodeTable");
  }

  run() {
    const insts = this.ctx.insts;
    const table = insts.map((inst) => { return padLeft(inst.opcode0, 26) });

    this.inject("MainOpcodeTable",
                disclaimer(`const uint32_t InstDB::_mainOpcodeTable[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`),
                table.length * 4);
  }
}

// ============================================================================
// [tablegen.x86.AltOpcodeTable]
// ============================================================================

class AltOpcodeTable extends core.Task {
  constructor() {
    super("AltOpcodeTable");
  }

  run() {
    const insts = this.ctx.insts;
    const table = new IndexedArray();
    const index = insts.map((inst) => { return table.addIndexed(padLeft(inst.opcode1, 26)); });

    this.inject("AltOpcodeIndex",
                disclaimer(`const uint8_t InstDB::_altOpcodeIndex[] = {\n${StringUtils.format(index, kIndent, -1)}\n};\n`),
                index.length * 1);

    this.inject("AltOpcodeTable",
                disclaimer(`const uint32_t InstDB::_altOpcodeTable[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`),
                table.length * 4);
  }
}

// ============================================================================
// [tablegen.x86.SseToAvxTable]
// ============================================================================

class InstSseToAvxTable extends core.Task {
  constructor() {
    super("InstSseToAvxTable", ["IdEnum"]);
  }

  run() {
    const insts = this.ctx.insts;

    const dataTable = new IndexedArray();
    const indexTable = [];

    function add(data) {
      return dataTable.addIndexed("{ " + padLeft(`SseToAvxData::kMode${data.mode}`, 28) + ", " + padLeft(String(data.delta), 4) + " }");
    }

    // This will receive a zero index, which means that no SseToAvx or AvxToSSe translation is possible.
    const kInvalidIndex = add({ mode: "None", delta: 0 });
    insts.forEach((inst) => { indexTable.push(kInvalidIndex); });

    insts.forEach((inst) => {
      // If it's not `kInvalidIndex` it's an AVX instruction that shares the
      // SseToAvx data. We won't touch it as it already has the index assigned.
      if (indexTable[inst.id] === kInvalidIndex) {
        const data = this.calcSseToAvxData(inst.dbInsts);
        const index = add(data);

        indexTable[inst.id] = index;
        if (data.delta !== 0)
          indexTable[this.ctx.instMap["v" + inst.name].id] = index;
      }
    });

    this.inject("SseToAvxIndex",
                disclaimer(`static const uint8_t sseToAvxIndex[] = {\n${StringUtils.format(indexTable, kIndent, -1)}\n};\n`),
                indexTable.length * 1);

    this.inject("SseToAvxTable",
                disclaimer(`static const SseToAvxData sseToAvxData[] = {\n${StringUtils.format(dataTable, kIndent, true)}\n};\n`),
                dataTable.length * 2);
  }

  filterSseToAvx(dbInsts) {
    const filtered = [];
    for (var x = 0; x < dbInsts.length; x++) {
      const dbInst = dbInsts[x];
      const ops = dbInst.operands;

      // SSE instruction does never share its name with AVX one.
      if (/^(VEX|XOP|EVEX)$/.test(dbInst.prefix))
        return [];

      var ok = false;
      for (var y = 0; y < ops.length; y++) {
        // There is no AVX instruction that works with MMX regs.
        if (ops[y].reg === "mm") { ok = false; break; }
        if (ops[y].reg === "xmm") { ok = true; }
      }

      if (ok)
        filtered.push(dbInst);
    }

    return filtered;
  }

  calcSseToAvxData(dbInsts) {
    const data = {
      mode : "None", // No conversion by default.
      delta: 0       // 0 if no conversion is possible.
    };

    const dbSseInsts = this.filterSseToAvx(dbInsts);
    if (!dbSseInsts.length)
      return data;

    const sseName = dbSseInsts[0].name;
    const avxName = "v" + sseName;

    const dbAvxInsts = this.ctx.query(avxName);
    if (!dbAvxInsts.length) {
      DEBUG(`SseToAvx: Instruction '${sseName}' has no AVX counterpart`);
      return data;
    }

    if (avxName === "vblendvpd" || avxName === "vblendvps" || avxName === "vpblendvb") {
      // Special cases first.
      data.mode = "Blend";
    }
    else {
      // Common case, deduce conversion mode by checking both SSE and AVX instructions.
      const map = Object.create(null);
      for (var sseIndex = 0; sseIndex < dbSseInsts.length; sseIndex++) {
        const sseInst = dbSseInsts[sseIndex];
        var match = false;

        for (var avxIndex = 0; avxIndex < dbAvxInsts.length; avxIndex++) {
          const avxInst = dbAvxInsts[avxIndex];

          // Select only VEX instructions.
          if (avxInst.prefix !== "VEX") continue;

          // Check if the AVX version is the same.
          if (GenUtils.eqOps(avxInst.operands, 0, sseInst.operands, 0)) {
            map.raw = true;
            match = true;
          }
          else if (avxInst.operands[0].data === "xmm" && GenUtils.eqOps(avxInst.operands, 1, sseInst.operands, 0)) {
            map.nds = true;
            match = true;
          }
        }

        if (!match) {
          const signature = sseInst.operands.map(function(op) { return op.data; }).join(", ");
          console.log(`SseToAvx: Instruction '${sseName}(${signature})' has no AVX counterpart`);
          return data;
        }
      }

      data.mode = (map.raw && !map.nds) ? "Move" : (map.raw && map.nds) ? "MoveIfMem" : "Extend";
    }
    data.delta = this.ctx.instMap[avxName].id - this.ctx.instMap[sseName].id;
    return data;
  }
}

// ============================================================================
// [tablegen.x86.InstSignatureTable]
// ============================================================================

const RegOp = MapUtils.arrayToMap(["al", "ah", "ax", "eax", "rax", "cl", "r8lo", "r8hi", "r16", "r32", "r64", "xmm", "ymm", "zmm", "mm", "k", "sreg", "creg", "dreg", "st", "bnd"]);
const MemOp = MapUtils.arrayToMap(["m8", "m16", "m32", "m48", "m64", "m80", "m128", "m256", "m512", "m1024"]);

const cmpOp = StringUtils.makePriorityCompare([
  "implicit",
  "r8lo", "r8hi", "r16", "r32", "r64", "xmm", "ymm", "zmm", "mm", "k", "sreg", "creg", "dreg", "st", "bnd",
  "mem", "vm", "m8", "m16", "m32", "m48", "m64", "m80", "m128", "m256", "m512", "m1024",
  "mib",
  "vm32x", "vm32y", "vm32z", "vm64x", "vm64y", "vm64z",
  "memBase", "memES", "memDS",
  "i4", "u4", "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64",
  "rel8", "rel32"
]);

const OpToAsmJitOp = {
  "implicit": "F(Implicit)",

  "r8lo"    : "F(GpbLo)",
  "r8hi"    : "F(GpbHi)",
  "r16"     : "F(Gpw)",
  "r32"     : "F(Gpd)",
  "r64"     : "F(Gpq)",
  "xmm"     : "F(Xmm)",
  "ymm"     : "F(Ymm)",
  "zmm"     : "F(Zmm)",
  "mm"      : "F(Mm)",
  "k"       : "F(KReg)",
  "sreg"    : "F(SReg)",
  "creg"    : "F(CReg)",
  "dreg"    : "F(DReg)",
  "st"      : "F(St)",
  "bnd"     : "F(Bnd)",

  "mem"     : "F(Mem)",
  "vm"      : "F(Vm)",

  "i4"      : "F(I4)",
  "u4"      : "F(U4)",
  "i8"      : "F(I8)",
  "u8"      : "F(U8)",
  "i16"     : "F(I16)",
  "u16"     : "F(U16)",
  "i32"     : "F(I32)",
  "u32"     : "F(U32)",
  "i64"     : "F(I64)",
  "u64"     : "F(U64)",

  "rel8"    : "F(Rel8)",
  "rel32"   : "F(Rel32)",

  "m8"      : "M(M8)",
  "m16"     : "M(M16)",
  "m32"     : "M(M32)",
  "m48"     : "M(M48)",
  "m64"     : "M(M64)",
  "m80"     : "M(M80)",
  "m128"    : "M(M128)",
  "m256"    : "M(M256)",
  "m512"    : "M(M512)",
  "m1024"   : "M(M1024)",
  "mib"     : "M(Mib)",
  "mAny"    : "M(Any)",
  "vm32x"   : "M(Vm32x)",
  "vm32y"   : "M(Vm32y)",
  "vm32z"   : "M(Vm32z)",
  "vm64x"   : "M(Vm64x)",
  "vm64y"   : "M(Vm64y)",
  "vm64z"   : "M(Vm64z)",

  "memBase" : "M(BaseOnly)",
  "memDS"   : "M(Ds)",
  "memES"   : "M(Es)"
};

function StringifyArray(a, map) {
  var s = "";
  for (var i = 0; i < a.length; i++) {
    const op = a[i];
    if (!hasOwn.call(map, op))
      FAIL(`UNHANDLED OPERAND '${op}'`);
    s += (s ? " | " : "") + map[op];
  }
  return s ? s : "0";
}

class OSignature {
  constructor() {
    this.flags = Object.create(null);
  }

  equals(other) {
    return MapUtils.equals(this.flags, other.flags);
  }

  xor(other) {
    const result = MapUtils.xor(this.flags, other.flags);
    return Object.getOwnPropertyNames(result).length === 0 ? null : result;
  }

  mergeWith(other) {
    const af = this.flags;
    const bf = other.flags;

    var k;
    var indexKind = "";
    var hasReg = false;

    for (k in af) {
      const index = asmdb.x86.Utils.regIndexOf(k);
      const kind = asmdb.x86.Utils.regKindOf(k);

      if (kind)
        hasReg = true;

      if (index !== null && index !== -1)
        indexKind = kind;
    }

    if (hasReg) {
      for (k in bf) {
        const index = asmdb.x86.Utils.regIndexOf(k);
        if (index !== null && index !== -1) {
          const kind = asmdb.x86.Utils.regKindOf(k);
          if (indexKind !== kind) return false;
        }
      }
    }

    // Can merge...
    for (k in bf) af[k] = true;
    return true;
  }

  simplify() {
    const flags = this.flags;

    // Implicit register when also any other register can be specified.
    if (flags.al  && flags.r8lo) delete flags["al"];
    if (flags.ah  && flags.r8hi) delete flags["ah"];
    if (flags.ax  && flags.r16 ) delete flags["ax"];
    if (flags.eax && flags.r32 ) delete flags["eax"];
    if (flags.rax && flags.r64 ) delete flags["rax"];

    // 32-bit register or 16-bit memory implies also 16-bit reg.
    if (flags.r32 && flags.m16) {
      flags.r16 = true;
    }

    // 32-bit register or 8-bit memory implies also 16-bit and 8-bit reg.
    if (flags.r32 && flags.m8) {
      flags.r8lo = true;
      flags.r8hi = true;
      flags.r16 = true;
    }
  }

  toString() {
    var s = "";
    var flags = this.flags;

    for (var k in flags) {
      if (k === "read" || k === "write" || k === "implicit" || k === "memDS" || k === "memES")
        continue;

      var x = k;
      if (x === "memZAX") x = "zax";
      if (x === "memZDI") x = "zdi";
      if (x === "memZSI") x = "zsi";
      s += (s ? "|" : "") + x;
    }

    if (flags.memDS) s = "ds:[" + s + "]";
    if (flags.memES) s = "es:[" + s + "]";

    if (flags.implicit)
      s = "<" + s + ">";

    return s;
  }

  toAsmJitOpData() {
    var oFlags = this.flags;

    var mFlags = Object.create(null);
    var mMemFlags = Object.create(null);
    var mExtFlags = Object.create(null);
    var sRegMask = 0;

    for (var k in oFlags) {
      switch (k) {
        case "implicit":
        case "r8lo"    :
        case "r8hi"    :
        case "r16"     :
        case "r32"     :
        case "r64"     :
        case "creg"    :
        case "dreg"    :
        case "sreg"    :
        case "bnd"     :
        case "st"      :
        case "k"       :
        case "mm"      :
        case "xmm"     :
        case "ymm"     :
        case "zmm"     : mFlags[k] = true; break;

        case "m8"      :
        case "m16"     :
        case "m32"     :
        case "m48"     :
        case "m64"     :
        case "m80"     :
        case "m128"    :
        case "m256"    :
        case "m512"    :
        case "m1024"   : mFlags.mem = true; mMemFlags[k] = true; break;
        case "mib"     : mFlags.mem = true; mMemFlags.mib   = true; break;
        case "mem"     : mFlags.mem = true; mMemFlags.mAny  = true; break;

        case "memDS"   : mFlags.mem = true; mMemFlags.memDS = true; break;
        case "memES"   : mFlags.mem = true; mMemFlags.memES = true; break;
        case "memZAX"  : mFlags.mem = true; sRegMask |= 1 << 0; mMemFlags.memBase = true; break;
        case "memZSI"  : mFlags.mem = true; sRegMask |= 1 << 6; mMemFlags.memBase = true; break;
        case "memZDI"  : mFlags.mem = true; sRegMask |= 1 << 7; mMemFlags.memBase = true; break;

        case "vm32x"   : mFlags.vm = true; mMemFlags.vm32x = true; break;
        case "vm32y"   : mFlags.vm = true; mMemFlags.vm32y = true; break;
        case "vm32z"   : mFlags.vm = true; mMemFlags.vm32z = true; break;
        case "vm64x"   : mFlags.vm = true; mMemFlags.vm64x = true; break;
        case "vm64y"   : mFlags.vm = true; mMemFlags.vm64y = true; break;
        case "vm64z"   : mFlags.vm = true; mMemFlags.vm64z = true; break;

        case "i4"      :
        case "u4"      :
        case "i8"      :
        case "u8"      :
        case "i16"     :
        case "u16"     :
        case "i32"     :
        case "u32"     :
        case "i64"     :
        case "u64"     : mFlags[k] = true; break;

        case "rel8"    :
        case "rel32"   :
          mFlags.i32 = true;
          mFlags.i64 = true;
          mFlags[k] = true;
          break;

        case "rel16"   :
          mFlags.i32 = true;
          mFlags.i64 = true;
          mFlags.rel32 = true;
          break;

        default: {
          switch (k) {
            case "es"    : mFlags.sreg = true; sRegMask |= 1 << 1; break;
            case "cs"    : mFlags.sreg = true; sRegMask |= 1 << 2; break;
            case "ss"    : mFlags.sreg = true; sRegMask |= 1 << 3; break;
            case "ds"    : mFlags.sreg = true; sRegMask |= 1 << 4; break;
            case "fs"    : mFlags.sreg = true; sRegMask |= 1 << 5; break;
            case "gs"    : mFlags.sreg = true; sRegMask |= 1 << 6; break;
            case "al"    : mFlags.r8lo = true; sRegMask |= 1 << 0; break;
            case "ah"    : mFlags.r8hi = true; sRegMask |= 1 << 0; break;
            case "ax"    : mFlags.r16  = true; sRegMask |= 1 << 0; break;
            case "eax"   : mFlags.r32  = true; sRegMask |= 1 << 0; break;
            case "rax"   : mFlags.r64  = true; sRegMask |= 1 << 0; break;
            case "bl"    : mFlags.r8lo = true; sRegMask |= 1 << 3; break;
            case "bh"    : mFlags.r8hi = true; sRegMask |= 1 << 3; break;
            case "bx"    : mFlags.r16  = true; sRegMask |= 1 << 3; break;
            case "ebx"   : mFlags.r32  = true; sRegMask |= 1 << 3; break;
            case "rbx"   : mFlags.r64  = true; sRegMask |= 1 << 3; break;
            case "cl"    : mFlags.r8lo = true; sRegMask |= 1 << 1; break;
            case "ch"    : mFlags.r8hi = true; sRegMask |= 1 << 1; break;
            case "cx"    : mFlags.r16  = true; sRegMask |= 1 << 1; break;
            case "ecx"   : mFlags.r32  = true; sRegMask |= 1 << 1; break;
            case "rcx"   : mFlags.r64  = true; sRegMask |= 1 << 1; break;
            case "dl"    : mFlags.r8lo = true; sRegMask |= 1 << 2; break;
            case "dh"    : mFlags.r8hi = true; sRegMask |= 1 << 2; break;
            case "dx"    : mFlags.r16  = true; sRegMask |= 1 << 2; break;
            case "edx"   : mFlags.r32  = true; sRegMask |= 1 << 2; break;
            case "rdx"   : mFlags.r64  = true; sRegMask |= 1 << 2; break;
            case "si"    : mFlags.r16  = true; sRegMask |= 1 << 6; break;
            case "esi"   : mFlags.r32  = true; sRegMask |= 1 << 6; break;
            case "rsi"   : mFlags.r64  = true; sRegMask |= 1 << 6; break;
            case "di"    : mFlags.r16  = true; sRegMask |= 1 << 7; break;
            case "edi"   : mFlags.r32  = true; sRegMask |= 1 << 7; break;
            case "rdi"   : mFlags.r64  = true; sRegMask |= 1 << 7; break;
            case "st0"   : mFlags.st   = true; sRegMask |= 1 << 0; break;
            case "xmm0"  : mFlags.xmm  = true; sRegMask |= 1 << 0; break;
            case "ymm0"  : mFlags.ymm  = true; sRegMask |= 1 << 0; break;
            default:
              console.log(`UNKNOWN OPERAND '${k}'`);
          }
        }
      }
    }

    const sFlags    = StringifyArray(ArrayUtils.sorted(mFlags   , cmpOp), OpToAsmJitOp);
    const sMemFlags = StringifyArray(ArrayUtils.sorted(mMemFlags, cmpOp), OpToAsmJitOp);
    const sExtFlags = StringifyArray(ArrayUtils.sorted(mExtFlags, cmpOp), OpToAsmJitOp);

    return `ROW(${sFlags || 0}, ${sMemFlags || 0}, ${sExtFlags || 0}, ${StringUtils.decToHex(sRegMask, 2)})`;
  }
}

class ISignature extends Array {
  constructor(name) {
    super();
    this.name = name;
    this.x86 = false;
    this.x64 = false;
    this.implicit = 0; // Number of implicit operands.
  }

  simplify() {
    for (var i = 0; i < this.length; i++)
      this[i].simplify();
  }

  opEquals(other) {
    const len = this.length;
    if (len !== other.length) return false;

    for (var i = 0; i < len; i++)
      if (!this[i].equals(other[i]))
        return false;

    return true;
  }

  mergeWith(other) {
    // If both architectures are the same, it's fine to merge.
    var ok = this.x86 === other.x86 && this.x64 === other.x64;

    // If the first arch is [X86|X64] and the second [X64] it's also fine.
    if (!ok && this.x86 && this.x64 && !other.x86 && other.x64)
      ok = true;

    // It's not ok if both signatures have different number of implicit operands.
    if (!ok || this.implicit !== other.implicit)
      return false;

    // It's not ok if both signatures have different number of operands.
    const len = this.length;
    if (len !== other.length)
      return false;

    var xorIndex = -1;
    for (var i = 0; i < len; i++) {
      const xor = this[i].xor(other[i]);
      if (xor === null) continue;

      if (xorIndex === -1)
        xorIndex = i;
      else
        return false;
    }

    // Bail if mergeWidth at operand-level failed.
    if (xorIndex !== -1 && !this[xorIndex].mergeWith(other[xorIndex]))
      return false;

    this.x86 = this.x86 || other.x86;
    this.x64 = this.x64 || other.x64;

    return true;
  }

  toString() {
    return "{" + this.join(", ") + "}";
  }
}

class SignatureArray extends Array {
  // Iterate over all signatures and check which operands don't need explicit memory size.
  calcImplicitMemSize() {
    // Calculates a hash-value (aka key) of all register operands specified by `regOps` in `inst`.
    function keyOf(inst, regOps) {
      var s = "";
      for (var i = 0; i < inst.length; i++) {
        const op = inst[i];
        if (regOps & (1 << i))
          s += "{" + ArrayUtils.sorted(MapUtils.and(op.flags, RegOp)).join("|") + "}";
      }
      return s || "?";
    }

    var i;
    var aIndex, bIndex;

    for (aIndex = 0; aIndex < this.length; aIndex++) {
      const aInst = this[aIndex];
      const len = aInst.length;

      var memOp = "";
      var memPos = -1;
      var regOps = 0;

      // Check if this instruction signature has a memory operand of explicit size.
      for (i = 0; i < len; i++) {
        const aOp = aInst[i];
        const mem = MapUtils.firstOf(aOp.flags, MemOp);

        if (mem) {
          // Stop if the memory operand has implicit-size or if there is more than one.
          if (aOp.flags.mem || memPos >= 0) {
            memPos = -1;
            break;
          }
          else {
            memOp = mem;
            memPos = i;
          }
        }
        else if (MapUtils.anyOf(aOp.flags, RegOp)) {
          // Doesn't consider 'r/m' as we already checked 'm'.
          regOps |= (1 << i);
        }
      }

      if (memPos < 0)
        continue;

      // Create a `sameSizeSet` set of all instructions having the exact
      // explicit memory operand at the same position and registers at
      // positions matching `regOps` bits and `diffSizeSet` having memory
      // operand of different size, but registers at the same positions.
      const sameSizeSet = [aInst];
      const diffSizeSet = [];
      const diffSizeHash = Object.create(null);

      for (bIndex = 0; bIndex < this.length; bIndex++) {
        const bInst = this[bIndex];
        if (aIndex === bIndex || len !== bInst.length) continue;

        var hasMatch = 1;
        for (i = 0; i < len; i++) {
          if (i === memPos) continue;

          const reg = MapUtils.anyOf(bInst[i].flags, RegOp);
          if (regOps & (1 << i))
            hasMatch &= reg;
          else if (reg)
            hasMatch = 0;
        }

        if (hasMatch) {
          const bOp = bInst[memPos];
          if (bOp.flags.mem) continue;

          const mem = MapUtils.firstOf(bOp.flags, MemOp);
          if (mem === memOp) {
            sameSizeSet.push(bInst);
          }
          else if (mem) {
            const key = keyOf(bInst, regOps);
            diffSizeSet.push(bInst);
            if (!diffSizeHash[key])
              diffSizeHash[key] = [bInst];
            else
              diffSizeHash[key].push(bInst);
          }
        }
      }

      // Two cases.
      //   A) The memory operand has implicit-size if `diffSizeSet` is empty. That
      //      means that the instruction only uses one size for all reg combinations.
      //
      //   B) The memory operand has implicit-size if `diffSizeSet` contains different
      //      register signatures than `sameSizeSet`.
      var implicit = true;

      if (!diffSizeSet.length) {
        // Case A:
      }
      else {
        // Case B: Find collisions in `sameSizeSet` and `diffSizeSet`.
        for (bIndex = 0; bIndex < sameSizeSet.length; bIndex++) {
          const bInst = sameSizeSet[bIndex];
          const key = keyOf(bInst, regOps);

          const diff = diffSizeHash[key];
          if (diff) {
            diff.forEach((diffInst) => {
              if ((bInst.x86 && !diffInst.x86) || (!bInst.x86 && diffInst.x86)) {
                // If this is X86|ANY instruction and the other is X64, or vice-versa,
                // then keep this implicit as it won't do any harm. These instructions
                // cannot be mixed and it will make implicit the 32-bit one in cases
                // where X64 introduced 64-bit ones like `cvtsi2ss`.
              }
              else {
                implicit = false;
              }
            });
          }
        }
      }

      // Patch all instructions to accept implicit-size memory operand.
      for (bIndex = 0; bIndex < sameSizeSet.length; bIndex++) {
        const bInst = sameSizeSet[bIndex];
        if (implicit) bInst[memPos].flags.mem = true;

        if (!implicit)
          DEBUG(`${this.name}: Explicit: ${bInst}`);
      }
    }
  }

  simplify() {
    for (var i = 0; i < this.length; i++)
      this[i].simplify();
  }

  compact() {
    for (var i = 0; i < this.length; i++) {
      var row = this[i];
      var j = i + 1;
      while (j < this.length) {
        if (row.mergeWith(this[j])) {
          this.splice(j, 1);
          continue;
        }
        j++;
      }
    }
  }

  toString() {
    return `[${this.join(", ")}]`;
  }
}

class InstSignatureTable extends core.Task {
  constructor() {
    super("InstSignatureTable");

    this.maxOpRows = 0;
    this.opBlackList = {
      "moff8" : true,
      "moff16": true,
      "moff32": true,
      "moff64": true
    };
  }

  run() {
    const insts = this.ctx.insts;

    insts.forEach((inst) => {
      inst.signatures = this.makeSignatures(inst.dbInsts);
      this.maxOpRows = Math.max(this.maxOpRows, inst.signatures.length);
    });

    const iSignatureMap = Object.create(null);
    const iSignatureArr = [];

    const oSignatureMap = Object.create(null);
    const oSignatureArr = [];

    // Must be first to be assigned to zero.
    const oSignatureNone = "ROW(0, 0, 0, 0xFF)";
    oSignatureMap[oSignatureNone] = [0];
    oSignatureArr.push(oSignatureNone);

    function findSignaturesIndex(rows) {
      const len = rows.length;
      if (!len) return 0;

      const indexes = iSignatureMap[rows[0].data];
      if (indexes === undefined) return -1;

      for (var i = 0; i < indexes.length; i++) {
        const index = indexes[i];
        if (index + len > iSignatureArr.length) continue;

        var ok = true;
        for (var j = 0; j < len; j++) {
          if (iSignatureArr[index + j].data !== rows[j].data) {
            ok = false;
            break;
          }
        }

        if (ok)
          return index;
      }

      return -1;
    }

    function indexSignatures(signatures) {
      const result = iSignatureArr.length;

      for (var i = 0; i < signatures.length; i++) {
        const signature = signatures[i];
        const idx = iSignatureArr.length;

        if (!hasOwn.call(iSignatureMap, signature.data))
          iSignatureMap[signature.data] = [];

        iSignatureMap[signature.data].push(idx);
        iSignatureArr.push(signature);
      }

      return result;
    }

    for (var len = this.maxOpRows; len >= 0; len--) {
      insts.forEach((inst) => {
        const signatures = inst.signatures;
        if (signatures.length === len) {
          const signatureEntries = [];
          for (var j = 0; j < len; j++) {
            const signature = signatures[j];

            var signatureEntry = `ROW(${signature.length}, ${signature.x86 ? 1 : 0}, ${signature.x64 ? 1 : 0}, ${signature.implicit}`;
            var signatureComment = signature.toString();

            var x = 0;
            while (x < signature.length) {
              const h = signature[x].toAsmJitOpData();
              var index = -1;
              if (!hasOwn.call(oSignatureMap, h)) {
                index = oSignatureArr.length;
                oSignatureMap[h] = index;
                oSignatureArr.push(h);
              }
              else {
                index = oSignatureMap[h];
              }

              signatureEntry += `, ${padLeft(index, 3)}`;
              x++;
            }

            while (x < 6) {
              signatureEntry += `, ${padLeft(0, 3)}`;
              x++;
            }

            signatureEntry += `)`;
            signatureEntries.push({ data: signatureEntry, comment: signatureComment, refs: 0 });
          }

          var count = signatureEntries.length;
          var index = findSignaturesIndex(signatureEntries);

          if (index === -1)
            index = indexSignatures(signatureEntries);

          iSignatureArr[index].refs++;
          inst.signatureIndex = index;
          inst.signatureCount = count;
        }
      });
    }

    var s = `#define ROW(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5)  \\\n` +
            `  { count, (x86 ? uint8_t(InstDB::kModeX86) : uint8_t(0)) |     \\\n` +
            `           (x64 ? uint8_t(InstDB::kModeX64) : uint8_t(0)) ,     \\\n` +
            `    implicit,                                                   \\\n` +
            `    0,                                                          \\\n` +
            `    { o0, o1, o2, o3, o4, o5 }                                  \\\n` +
            `  }\n` +
            StringUtils.makeCxxArrayWithComment(iSignatureArr, "const InstDB::InstSignature InstDB::_instSignatureTable[]") +
            `#undef ROW\n` +
            `\n` +
            `#define ROW(flags, mFlags, extFlags, regId) { uint32_t(flags), uint16_t(mFlags), uint8_t(extFlags), uint8_t(regId) }\n` +
            `#define F(VAL) InstDB::kOp##VAL\n` +
            `#define M(VAL) InstDB::kMemOp##VAL\n` +
            StringUtils.makeCxxArray(oSignatureArr, "const InstDB::OpSignature InstDB::_opSignatureTable[]") +
            `#undef M\n` +
            `#undef F\n` +
            `#undef ROW\n`;
    this.inject("InstSignatureTable", disclaimer(s), oSignatureArr.length * 8 + iSignatureArr.length * 8);
  }

  makeSignatures(dbInsts) {
    const blackList = this.opBlackList;
    const signatures = new SignatureArray();

    for (var i = 0; i < dbInsts.length; i++) {
      const inst = dbInsts[i];
      const ops = inst.operands;

      // NOTE: This changed from having reg|mem merged into creating two signatures
      // instead. Imagine two instructions in one `dbInsts` array:
      //
      //   1. mov reg, reg/mem
      //   2. mov reg/mem, reg
      //
      // If we merge them and then unmerge, we will have 4 signatures, when iterated:
      //
      //   1a. mov reg, reg
      //   1b. mov reg, mem
      //   2a. mov reg, reg
      //   2b. mov mem, reg
      //
      // So, instead of merging them here, we insert separated signatures and let
      // the tool merge them in a way that can be easily unmerged at runtime into:
      //
      //   1a. mov reg, reg
      //   1b. mov reg, mem
      //   2b. mov mem, reg
      var modrmCount = 1;
      for (var modrm = 0; modrm < modrmCount; modrm++) {
        var row = new ISignature(inst.name);
        row.x86 = (inst.arch === "ANY" || inst.arch === "X86");
        row.x64 = (inst.arch === "ANY" || inst.arch === "X64");

        for (var j = 0; j < ops.length; j++) {
          var iop = ops[j];

          var reg = iop.reg;
          var mem = iop.mem;
          var imm = iop.imm;
          var rel = iop.rel;

          // Terminate if this operand is something asmjit doesn't support
          // and skip all instructions having implicit `imm` operand of `1`,
          // which are handled fine by asmjit.
          if (this.opBlackList[mem] === true || iop.immValue !== null)
            break;

          if (reg === "r8") reg = "r8lo";
          if (reg === "seg") reg = "sreg";
          if (reg === "st(i)") reg = "st";
          if (reg === "st(0)") reg = "st0";

          if (mem === "m32fp") mem = "m32";
          if (mem === "m64fp") mem = "m64";
          if (mem === "m80fp") mem = "m80";
          if (mem === "m80bcd") mem = "m80";
          if (mem === "m80dec") mem = "m80";
          if (mem === "m16int") mem = "m16";
          if (mem === "m32int") mem = "m32";
          if (mem === "m64int") mem = "m64";

          if (mem === "m16_16") mem = "m32";
          if (mem === "m16_32") mem = "m48";
          if (mem === "m16_64") mem = "m80";

          if (reg && mem) {
            if (modrmCount === 1) {
              mem = null;
              modrmCount++;
            }
            else {
              reg = null;
            }
          }

          const op = new OSignature();
          if (iop.implicit) {
            row.implicit++;
            op.flags.implicit = true;
          }

          if (iop.memSeg) {
            if (iop.memSeg === "ds") op.flags.memDS = true;
            if (iop.memSeg === "es") op.flags.memES = true;
            if (reg === "zax") op.flags.memZAX = true;
            if (reg === "zsi") op.flags.memZSI = true;
            if (reg === "zdi") op.flags.memZDI = true;
          }
          else if (reg) {
            op.flags[reg] = true;
            if (reg === "r8lo") op.flags.r8hi = true;
          }
          if (mem) {
            op.flags[mem] = true;
            // Exception: Allow LEA to use any memory size.
            if (inst.name === "lea") MapUtils.add(op.flags, MemOp);
          }
          if (imm) {
            if (iop.immSign === "any" || iop.immSign === "signed"  ) op.flags["i" + imm] = true;
            if (iop.immSign === "any" || iop.immSign === "unsigned") op.flags["u" + imm] = true;
          }
          if (rel) op.flags["rel" + rel] = true;

          row.push(op);
        }

        // Not equal if we terminated the loop.
        if (j === ops.length)
          signatures.push(row);
      }
    }

    signatures.calcImplicitMemSize();
    signatures.simplify();
    signatures.compact();

    signatures.simplify();
    signatures.compact();

    return signatures;
  }
}

// ============================================================================
// [tablegen.x86.InstExecutionTable]
// ============================================================================

class InstExecutionTable extends core.Task {
  constructor() {
    super("InstExecutionTable");
  }

  run() {
    const insts = this.ctx.insts;
    const table = new IndexedArray();

    insts.forEach((inst) => {
      const dbInsts = inst.dbInsts;

      var opFlags = GenUtils.operationFlagsOf(dbInsts).map(function(f) { return `F(${f})`; });
      if (!opFlags.length) opFlags.push("0");

      var features = GenUtils.cpuFeaturesOf(dbInsts).map(function(f) { return `FEATURE(${f})`; });
      if (!features.length) features.push("0");

      var [r, w] = GenUtils.specialsOf(dbInsts);
      r = r.map(function(item) { return `SPECIAL(${item.replace(".", "_")})`; });
      w = w.map(function(item) { return `SPECIAL(${item.replace(".", "_")})`; });

      const opFlagsStr = opFlags.join(" | ");
      const featuresStr = features.join(", ");
      const rStr = r.join(" | ") || "0";
      const wStr = w.join(" | ") || "0";

      inst.operationDataIndex = table.addIndexed(`{ ${opFlagsStr}, { ${featuresStr} }, ${rStr}, ${wStr} }`);
    });

    var s = `#define OP_FLAG(VAL) uint32_t(InstInfo::kOperation##VAL)\n` +
            `#define FEATURE(VAL) uint32_t(Features::k##VAL)\n` +
            `#define SPECIAL(VAL) uint32_t(kSpecialReg_##VAL)\n` +
            `const InstDB::ExecutionInfo InstDB::_executionInfoTable[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef SPECIAL\n` +
            `#undef FEATURE\n` +
            `#undef OP_FLAG\n` ;
    this.inject("InstExecutionTable", disclaimer(s), table.length * 16);
  }
}

// ============================================================================
// [tablegen.x86.InstRWInfoTable]
// ============================================================================

class InstRWInfoTable extends core.Task {
  constructor() {
    super("InstRWInfoTable");
  }

  run() {
    const insts = this.ctx.insts;

    const iRWInfoMap = Object.create(null);
    const iRWInfoArr = [];

    const MAX_OPS = 6;

    // Doesn't have to match AsmJit constants as they will be stringified.
    const kRead            = 0x00000001;
    const kWrite           = 0x00000002;
    const kRW              = 0x00000003;
    const kZExt            = 0x00000010;
    const kNoZExtGpbGpwXmm = 0x00000020;

    function newOInfo() {
      return {
        flags: kRead,
        physId: 0xFF,
        index: 0,
        width: 0
      };
    }

    function OFlagsToString(flags) {
      var s = "";

      s += (flags & kRead) && (flags & kWrite) ? "X" : (flags & kWrite) ? "W" : "R";
      //if (flags & kZExt) s += "|ZExt";
      if (flags & kNoZExtGpbGpwXmm) s += "|NoZExtGpbGpwXmm";

      return s ? s : "0";
    }

    function ORWInfoToString(oRWInfo) {
      return `{ ${OFlagsToString(oRWInfo.flags)}, ${oRWInfo.physId}, ${oRWInfo.index}, ${oRWInfo.width} }`;
    }

    function IRWInfoToString(iRWInfo) {
      var s = "";

      s += "{ ";
      s += `${iRWInfo.flags}`;
      s += ", " + ORWInfoToString(iRWInfo.extraReg);
      s += ", { ";
      const operands = iRWInfo.operands;
      for (var i = 0; i < MAX_OPS; i++) {
        if (i !== 0) s += ", ";
        s += ORWInfoToString(operands[i]);
      }
      s += "} ";
      s += "}";

      return s;
    }

    function canMergeORWInfo(a, b) {
      if (a.physId !== b.physId || a.index !== b.index || a.width !== b.width)
        return false;

      if (a.flags === b.flags)
        return true;

      function mergeable(af, bf) {
        if ((af | kNoZExtGpbGpwXmm) === bf)
          return true;
        else
          return false;
      }

      return mergeable(a.flags, b.flags) || mergeable(b.flags, a.flags);
    }

    function canMergeIRWInfo(a, b) {
      if (a.flags !== b.flags || !canMergeORWInfo(a.extraReg, b.extraReg))
        return false;

      for (var i = 0; i < MAX_OPS; i++)
        if (!canMergeORWInfo(a.operands[i], b.operands[i]))
          return false;

      return true;
    }

    function mergeItem(arr, item) {
      function mergeORWInfo(a, b) {
        a.flags |= b.flags;
      }

      for (var i = 0; i < arr.length; i++) {
        if (canMergeIRWInfo(arr[i], item)) {
          const a = arr[i];
          const b = item;

          mergeORWInfo(a.extraReg, b.extraReg);
          for (var j = 0; j < MAX_OPS; j++)
            mergeORWInfo(a.operands[j], b.operands[j]);

          return true;
        }
      }
      return false;
    }

    insts.forEach((inst) => {
      const dbInsts = inst.dbInsts;
      if (dbInsts.length) {
        const rwArr = [];
        dbInsts.forEach((inst) => {
          var rwInfo = {
            flags: 0,
            extraReg: newOInfo(),
            operands: [newOInfo(), newOInfo(), newOInfo(), newOInfo(), newOInfo(), newOInfo()]
          };

          inst.operands.forEach((op, opIndex) => {
            if (op.isRegOrMem()) {
              const oRWInfo = rwInfo.operands[opIndex];
              if (op.read && op.write) {
                oRWInfo.flags |= kRW;
              }
              else if (op.write) {
                oRWInfo.flags &=~kRead;
                oRWInfo.flags |= kWrite;
              }
              else {
              }

              //if (op.zext) {
              //  oRWInfo.flags |= kZExt;
              // }
              if (op.write && op.isPartialOp()) {
                oRWInfo.flags |= kNoZExtGpbGpwXmm;
              }

              const rwxIndex = Math.max(op.rwxIndex, 0);
              const rwxWidth = Math.max(op.rwxWidth, 0);

              oRWInfo.index = rwxIndex;
              oRWInfo.width = rwxWidth;
            }
          });

          if (!mergeItem(rwArr, rwInfo))
            rwArr.push(rwInfo);
        });

        if (rwArr.length > 1) {
          console.log(inst.name + ": ");
          rwArr.forEach(function(item) { console.log("  " + IRWInfoToString(item)); });
        }

        //iRWInfoArr[index].refs++;
        //inst.rwInfoIndex = index;
        //inst.rwInfoCount = count;
      }
    });

    //var s = StringUtils.makeCxxArrayWithComment(iSignatureArr, "const Inst::IRWInfo X86InstDB::iRWInfoData[]");
    //this.inject("rwInfoData", disclaimer(s), 0);
  }
}

// ============================================================================
// [tablegen.x86.InstCommonTable]
// ============================================================================

class InstCommonTable extends core.Task {
  constructor() {
    super("InstCommonTable", [
      "IdEnum",
      "NameTable",
      "InstSignatureTable",
      "InstExecutionTable",
      "InstRWInfoTable"
    ]);
  }

  run() {
    const insts = this.ctx.insts;
    const table = new IndexedArray();

    insts.forEach((inst) => {
      const flags         = inst.flags.map(function(flag) { return `F(${flag})`; }).join("|") || "0";
      const singleRegCase = `SINGLE_REG(${inst.singleRegCase})`;
      const controlType   = `CONTROL(${inst.controlType})`;
      const specialCases  = inst.specialCases.map(function(flag) { return `SPECIAL_CASE(${flag})`; }).join("|") || "0";

      const row = "{ " +
        padLeft(flags              , 54) + ", " +
        padLeft(inst.writeIndex    ,  3) + ", " +
        padLeft(inst.writeSize     ,  3) + ", " +
        padLeft(inst.signatureIndex,  3) + ", " +
        padLeft(inst.signatureCount,  2) + ", " +
        padLeft(controlType        , 16) + ", " +
        padLeft(singleRegCase      , 16) + ", " +
        padLeft(specialCases       , 20) + ", " + "0 }";
      inst.commonDataIndex = table.addIndexed(row);
    });

    var s = `#define F(VAL) InstDB::kFlag##VAL\n` +
            `#define CONTROL(VAL) Inst::kControl##VAL\n` +
            `#define SINGLE_REG(VAL) InstDB::kSingleReg##VAL\n` +
            `#define SPECIAL_CASE(VAL) InstDB::kSpecialCase##VAL\n` +
            `const InstDB::CommonInfo InstDB::_commonInfoTable[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef SPECIAL_CASE\n` +
            `#undef SINGLE_REG\n` +
            `#undef CONTROL\n` +
            `#undef F\n`;
    this.inject("InstCommonTable", disclaimer(s), table.length * 12);
  }
}

// ============================================================================
// [Main]
// ============================================================================

new X86TableGen()
  .addTask(new IdEnum())
  .addTask(new NameTable())
  .addTask(new EncodingTable())
  .addTask(new MainOpcodeTable())
  .addTask(new AltOpcodeTable())
  .addTask(new InstSseToAvxTable())
  .addTask(new InstSignatureTable())
  .addTask(new InstExecutionTable())
  .addTask(new InstRWInfoTable())
  .addTask(new InstCommonTable())
  .run();
