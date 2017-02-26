// [Generate-X86]
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
"use strict";

const base = require("./generate-base.js");
const hasOwn = Object.prototype.hasOwnProperty;
const kIndent = base.kIndent;
const MapUtils = base.MapUtils;
const StringUtils = base.StringUtils;

const DEBUG = false;

// ----------------------------------------------------------------------------
// [X86DB]
// ----------------------------------------------------------------------------

// Create the X86 database and add some special cases recognized by AsmJit.
const x86 = base.asmdb.x86;
const isa = new x86.ISA({
  instructions: [
    // Imul in [reg, imm] form is encoded as [reg, reg, imm].
    ["imul"  , "r16, ib"    , "RM"   , "66 6B /r ib"        , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul"  , "r32, ib"    , "RM"   , "6B /r ib"           , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul"  , "r64, ib"    , "RM"   , "REX.W 6B /r ib"     , "X64 OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul"  , "r16, iw"    , "RM"   , "66 69 /r iw"        , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul"  , "r32, id"    , "RM"   , "69 /r id"           , "ANY OF=W SF=W ZF=U AF=U PF=U CF=W"],
    ["imul"  , "r64, id"    , "RM"   , "REX.W 69 /r id"     , "X64 OF=W SF=W ZF=U AF=U PF=U CF=W"]
  ]
});

// Remapped instructions contain mapping between instructions that AsmJit expects
// and instructions provided by asmdb. In general, AsmJit uses `cmps` instructions
// without the suffix, so we just remap these and keep all others.
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

// ----------------------------------------------------------------------------
// [GenUtils]
// ----------------------------------------------------------------------------

class GenUtils {
  // Get group of instructions having the same name as understood by AsmJit.
  static groupOf(name) {
    const remapped = RemappedInsts[name];
    if (!remapped) return isa.query(name);

    const insts = isa.query(remapped.names);
    const rep = remapped.rep;
    if (rep === null) return insts;

    return insts.filter(function(inst) {
      return rep === !!(inst.attributes.REP || inst.attributes.REPNZ);
    });
  }

  static hasFixedReg(group) {
    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      if (NotFixedInsts[inst.name]) continue;

      const operands = inst.operands;
      for (var j = 0; j < operands.length; j++)
        if (operands[j].isFixedReg())
          return true;
    }

    return false;
  }

  static hasFixedMem(group) {
    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      if (NotFixedInsts[inst.name]) continue;

      const operands = inst.operands;
      for (var j = 0; j < operands.length; j++)
        if (operands[j].isFixedMem())
          return true;
    }

    return false;
  }

  static cpuArchOf(group) {
    var anyArch = false;
    var x86Arch = false;
    var x64Arch = false;

    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      if (inst.arch === "ANY") anyArch = true;
      if (inst.arch === "X86") x86Arch = true;
      if (inst.arch === "X64") x64Arch = true;
    }

    return anyArch || (x86Arch && x64Arch) ? "[ANY]" : x86Arch ? "[X86]" : "[X64]";
  }

  static cpuFeaturesOf(group) {
    const features = Object.create(null);

    for (var i = 0; i < group.length; i++)
      for (var feature in group[i].extensions)
        features[feature] = true;

    const result = Object.getOwnPropertyNames(features);
    result.sort();
    return result;
  }

  static specialsOf(group) {
    const r = Object.create(null);
    const w = Object.create(null);

    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      const specialRegs = inst.specialRegs;

      // Mov is a special case, moving to/from control regs makes flags undefined,
      // which we don't want to have in `X86InstDB::operationData`. This is, thus,
      // a special case instruction analyzer must deal with.
      if (inst.name === "mov")
        continue;

      for (var specialReg in specialRegs) {
        const group = isa.specialRegs[specialReg].group;
        const op = specialRegs[specialReg];

        switch (op) {
          case "R":
            r[group] = true;
            break;
          case "X":
            r[group] = true;
            // .. fallthrough ...
          case "W":
          case "U":
          case "0":
          case "1":
            w[group] = true;
            break;
        }
      }
    }

    const rArray = Object.getOwnPropertyNames(r);
    const wArray = Object.getOwnPropertyNames(w);

    rArray.sort();
    wArray.sort();

    return [rArray, wArray];
  }

  static flagsOf(group) {
    function getAccess(inst) {
      const operands = inst.operands;
      if (!operands.length) return "";

      if (inst.name === "xchg" || inst.name === "xadd")
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

    var mib = group.length > 0 && /^(?:bndldx|bndstx)$/.test(group[0].name);
    var access = "";
    var ambiguous = false;

    for (i = 0; i < group.length; i++) {
      const inst = group[i];
      const name = inst.name;

      const acc = getAccess(inst);
      if (!access)
        access = acc;
      else if (access !== acc)
        ambiguous = true;
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

    const fixedReg = GenUtils.hasFixedReg(group);
    const fixedMem = GenUtils.hasFixedMem(group);

    if (fixedReg && fixedMem)
      f.FixedRM = true;
    else if (fixedReg)
      f.FixedReg = true;
    else if (fixedMem)
      f.FixedMem = true;

    var mmx = false;
    var vec = false;

    for (i = 0; i < group.length; i++) {
      const inst = group[i];
      const operands = inst.operands;

      if (inst.name === "emms")
        mmx = true;

      if (inst.name === "vzeroall" || inst.name === "vzeroupper")
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

    for (i = 0; i < group.length; i++) {
      const inst = group[i];
      const name = inst.name;
      const operands = inst.operands;

      if (inst.attributes.LOCK    ) f.Lock     = true;
      if (inst.attributes.REP     ) f.Rep      = true;
      if (inst.attributes.REPNZ   ) f.Repnz    = true;
      if (inst.attributes.XACQUIRE) f.XAcquire = true;
      if (inst.attributes.XRELEASE) f.XRelease = true;

      if (inst.fpu) {
        for (var j = 0; j < operands.length; j++) {
          const op = operands[j];
          if (op.memSize === 16) f.FpuM16 = true;
          if (op.memSize === 32) f.FpuM32 = true;
          if (op.memSize === 64) f.FpuM64 = true;
          if (op.memSize === 80) f.FpuM80 = true;
        }
      }

      if (inst.vsibReg)
        f.Vsib = true;

      if (inst.prefix === "VEX" || inst.prefix === "XOP")
        f.Vex = true;

      if (inst.prefix === "EVEX") {
        f.Evex = true;

        if (inst.kmask) f.Avx512K = true;
        if (inst.zmask) f.Avx512Z = true;

        if (inst.er) f.Avx512ER = true;
        if (inst.sae) f.Avx512SAE = true;

        if (inst.broadcast) f["Avx512B" + String(inst.elementSize)] = true;
        if (inst.tupleType === "T1_4X") f.Avx512T4X = true;
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

  static operationFlagsOf(group) {
    const f = Object.create(null);

    for (var i = 0; i < group.length; i++) {
      const inst = group[i];
      const name = inst.name;

      const operands = inst.operands;

      // Special case: MOV undefines flags if moving between GP and CR|DR registers.
      if (name === "mov")
        f.MovCrDr = true;

      // Special case: MOVSS|MOVSD zeroes the remaining part of destination if source operand is memory.
      if ((name === "movss" || name === "movsd") && !inst.attributes.REP)
        f.MovSsSd = true;

      // Hardware prefetch.
      if (name.startsWith("prefetch"))
        f.Prefetch = true;

      // Memory barrier.
      if (/^[lms]fence$/.test(name))
        f.Barrier = true;

      // Instruction is volatile.
      if (inst.attributes.VOLATILE)
        f.Volatile = true;

      // Instruction is privileged.
      if (inst.privilege !== "L3")
        f.Privileged = true;
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
      case "pand"    : case "vpand"    : case "vpandd"  : case "vpandq"   :
      case "andpd"   : case "vandpd"   :
      case "andps"   : case "vandps"   :

      case "or"      :
      case "por"     : case "vpor"     : case "vpord"   : case "vporq"    :
      case "orpd"    : case "vorpd"    :
      case "orps"    : case "vorps"    :

      case "pminsb"  : case "vpminsb"  : case "pmaxsb"  : case "vpmaxsb"  :
      case "pminsw"  : case "vpminsw"  : case "pmaxsw"  : case "vpmaxsw"  :
      case "pminsd"  : case "vpminsd"  : case "pmaxsd"  : case "vpmaxsd"  :
      case "pminub"  : case "vpminub"  : case "pmaxub"  : case "vpmaxub"  :
      case "pminuw"  : case "vpminuw"  : case "pmaxuw"  : case "vpmaxuw"  :
      case "pminud"  : case "vpminud"  : case "pmaxud"  : case "vpmaxud"  :
        return "RO";

      case "pandn"   : case "vpandn"   : case "vpandnd" : case "vpandnq"  :

      case "xor"     :
      case "pxor"    : case "vpxor"    : case "vpxord"  : case "vpxorq"   :
      case "xorpd"   : case "vxorpd"   :
      case "xorps"   : case "vxorps"   :

      case "sub"     :
      case "psubb"   : case "vpsubb"   :
      case "psubw"   : case "vpsubw"   :
      case "psubd"   : case "vpsubd"   :
      case "psubq"   : case "vpsubq"   :
      case "psubsb"  : case "vpsubsb"  : case "psubusb" : case "vpsubusb" :
      case "psubsw"  : case "vpsubsw"  : case "psubusw" : case "vpsubusw" :

      case "vpcmpeqb": case "pcmpeqb"  : case "vpcmpgtb": case "pcmpgtb"  :
      case "vpcmpeqw": case "pcmpeqw"  : case "vpcmpgtw": case "pcmpgtw"  :
      case "vpcmpeqd": case "pcmpeqd"  : case "vpcmpgtd": case "pcmpgtd"  :
      case "vpcmpeqq": case "pcmpeqq"  : case "vpcmpgtq": case "pcmpgtq"  :

      case "vpcmpb"  : case "vpcmpub"  :
      case "vpcmpd"  : case "vpcmpud"  :
      case "vpcmpw"  : case "vpcmpuw"  :
      case "vpcmpq"  : case "vpcmpuq"  :
        return "WO";

      default:
        return "None";
    }
  }

  static jumpType(name) {
    switch (name) {
      case "jo" :
      case "jno":
      case "jb" : case "jnae":
      case "jae": case "jnb" :
      case "je" : case "jz"  :
      case "jne": case "jnz" :
      case "jbe": case "jna" :
      case "js" :
      case "jns":
      case "jp" : case "jpe" :
      case "jnp": case "jpo" :
      case "jl" : case "jnge":
      case "jge": case "jnl" :
      case "jle": case "jng" :
      case "jg" : case "jnle":
      case "jecxz":
      case "loop":
      case "loope":
      case "loopne":
      case "xbegin":
        return "Conditional";

      case "jmp" : return "Direct";
      case "call": return "Call";
      case "ret" : return "Return";

      default:
        return "None";
    }
  }
}

// ----------------------------------------------------------------------------
// [Generate]
// ----------------------------------------------------------------------------

const RegOp = MapUtils.arrayToMap([
  "al", "ah", "ax", "eax", "rax", "cl",
  "r8lo", "r8hi", "r16", "r32", "r64", "fp", "mm", "k", "xmm", "ymm", "zmm", "bnd", "sreg", "creg", "dreg"
]);

const MemOp = MapUtils.arrayToMap([
  "m8", "m16", "m32", "m48", "m64", "m80", "m128", "m256", "m512", "m1024"
]);

const OpSortPriority = {
  "read"    :-9,
  "write"   :-8,
  "rw"      :-7,
  "implicit":-6,

  "r8lo"    : 1,
  "r8hi"    : 2,
  "r16"     : 3,
  "r32"     : 4,
  "r64"     : 5,
  "fp"      : 6,
  "mm"      : 7,
  "k"       : 8,
  "xmm"     : 9,
  "ymm"     : 10,
  "zmm"     : 11,
  "sreg"    : 12,
  "bnd"     : 13,
  "creg"    : 14,
  "dreg"    : 15,

  "mem"     : 30,
  "vm"      : 31,
  "m8"      : 32,
  "m16"     : 33,
  "m32"     : 34,
  "m48"     : 35,
  "m64"     : 36,
  "m80"     : 37,
  "m128"    : 38,
  "m256"    : 39,
  "m512"    : 40,
  "m1024"   : 41,
  "mib"     : 42,
  "vm32x"   : 43,
  "vm32y"   : 44,
  "vm32z"   : 45,
  "vm64x"   : 46,
  "vm64y"   : 47,
  "vm64z"   : 48,
  "memBase" : 49,
  "memES"   : 50,
  "memDS"   : 51,

  "i4"      : 60,
  "u4"      : 61,
  "i8"      : 62,
  "u8"      : 63,
  "i16"     : 64,
  "u16"     : 65,
  "i32"     : 66,
  "u32"     : 67,
  "i64"     : 68,
  "u64"     : 69,

  "rel8"    : 70,
  "rel32"   : 71
};

const OpToAsmJitOp = {
  "read"    : "FLAG(R)",
  "write"   : "FLAG(W)",
  "rw"      : "FLAG(X)",
  "implicit": "FLAG(Implicit)",

  "r8lo"    : "FLAG(GpbLo)",
  "r8hi"    : "FLAG(GpbHi)",
  "r16"     : "FLAG(Gpw)",
  "r32"     : "FLAG(Gpd)",
  "r64"     : "FLAG(Gpq)",
  "fp"      : "FLAG(Fp)",
  "mm"      : "FLAG(Mm)",
  "k"       : "FLAG(K)",
  "xmm"     : "FLAG(Xmm)",
  "ymm"     : "FLAG(Ymm)",
  "zmm"     : "FLAG(Zmm)",
  "bnd"     : "FLAG(Bnd)",
  "sreg"    : "FLAG(Seg)",
  "creg"    : "FLAG(Cr)",
  "dreg"    : "FLAG(Dr)",

  "mem"     : "FLAG(Mem)",
  "vm"      : "FLAG(Vm)",

  "m8"      : "MEM(M8)",
  "m16"     : "MEM(M16)",
  "m32"     : "MEM(M32)",
  "m48"     : "MEM(M48)",
  "m64"     : "MEM(M64)",
  "m80"     : "MEM(M80)",
  "m128"    : "MEM(M128)",
  "m256"    : "MEM(M256)",
  "m512"    : "MEM(M512)",
  "m1024"   : "MEM(M1024)",
  "mib"     : "MEM(Mib)",
  "mAny"    : "MEM(Any)",
  "vm32x"   : "MEM(Vm32x)",
  "vm32y"   : "MEM(Vm32y)",
  "vm32z"   : "MEM(Vm32z)",
  "vm64x"   : "MEM(Vm64x)",
  "vm64y"   : "MEM(Vm64y)",
  "vm64z"   : "MEM(Vm64z)",

  "memBase" : "MEM(BaseOnly)",
  "memDS"   : "MEM(Ds)",
  "memES"   : "MEM(Es)",

  "i4"      : "FLAG(I4)",
  "u4"      : "FLAG(U4)",
  "i8"      : "FLAG(I8)",
  "u8"      : "FLAG(U8)",
  "i16"     : "FLAG(I16)",
  "u16"     : "FLAG(U16)",
  "i32"     : "FLAG(I32)",
  "u32"     : "FLAG(U32)",
  "i64"     : "FLAG(I64)",
  "u64"     : "FLAG(U64)",

  "rel8"    : "FLAG(Rel8)",
  "rel32"   : "FLAG(Rel32)"
};

function OpSortFunc(a, b) {
  return (OpSortPriority[a] || 0) - (OpSortPriority[b] || 0);
}

function SortOpArray(a) {
  a.sort(OpSortFunc);
  return a;
}

function StringifyArray(a, map) {
  var s = "";
  for (var i = 0; i < a.length; i++) {
    const op = a[i];
    if (!hasOwn.call(map, op))
      throw new Error(`UNHANDLED OPERAND '${op}'`);
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
      const index = x86.Utils.regIndexOf(k);
      const kind = x86.Utils.regKindOf(k);

      if (kind)
        hasReg = true;

      if (index !== null && index !== -1)
        indexKind = kind;
    }

    if (hasReg) {
      for (k in bf) {
        const index = x86.Utils.regIndexOf(k);
        if (index !== null && index !== -1) {
          const kind = x86.Utils.regKindOf(k);
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
    var prefix = (flags.read && flags.write) ? "X:" : (flags.write) ? "W:" : "R:";

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

    return prefix + s;
  }

  toAsmJitOpData() {
    var oFlags = this.flags;

    var mFlags = Object.create(null);
    var mMemFlags = Object.create(null);
    var mExtFlags = Object.create(null);
    var sRegMask = 0;

    if (oFlags.read && oFlags.write)
      mFlags.rw = true;
    else if (oFlags.write)
      mFlags.write = true;
    else
      mFlags.read = true;

    for (var k in oFlags) {
      switch (k) {
        case "read"    : break;
        case "write"   : break;

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
        case "fp"      :
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
            case "fp0"   : mFlags.fp   = true; sRegMask |= 1 << 0; break;
            case "xmm0"  : mFlags.xmm  = true; sRegMask |= 1 << 0; break;
            case "ymm0"  : mFlags.ymm  = true; sRegMask |= 1 << 0; break;
            default:
              console.log(`UNKNOWN OPERAND '${k}'`);
          }
        }
      }
    }

    const sFlags    = StringifyArray(SortOpArray(Object.getOwnPropertyNames(mFlags   )), OpToAsmJitOp);
    const sMemFlags = StringifyArray(SortOpArray(Object.getOwnPropertyNames(mMemFlags)), OpToAsmJitOp);
    const sExtFlags = StringifyArray(SortOpArray(Object.getOwnPropertyNames(mExtFlags)), OpToAsmJitOp);

    return `OSIGNATURE(${sFlags || 0}, ${sMemFlags || 0}, ${sExtFlags || 0}, ${StringUtils.decToHex(sRegMask, 2)})`;
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
        if (regOps & (1 << i)) {
          const props = Object.getOwnPropertyNames(MapUtils.and(op.flags, RegOp));
          props.sort();
          s += "{" + props.join("|") + "}";
        }
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
          // Stop if the memory operand is implicit or if there is more than one.
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
      //   A) The memory operand is implicit if `diffSizeSet` is empty. That means
      //      that the instruction only uses one size for all reg combinations.
      //
      //   B) The memory operand is implicit if `diffSizeSet` contains different
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
            diff.forEach(function(diffInst) {
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

      // Patch all instructions to accept implicit memory operand.
      for (bIndex = 0; bIndex < sameSizeSet.length; bIndex++) {
        const bInst = sameSizeSet[bIndex];
        if (implicit) bInst[memPos].flags.mem = true;

        if (DEBUG && !implicit)
          console.log(`${this.name}: Explicit: ${bInst}`);
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

// ----------------------------------------------------------------------------
// [X86Generator]
// ----------------------------------------------------------------------------

class X86Generator extends base.BaseGenerator {
  constructor() {
    super("X86");

    this.opCombinations = Object.create(null);
    this.maxOpRows = 0;

    this.opBlackList = {
      "moff8" : true,
      "moff16": true,
      "moff32": true,
      "moff64": true
    };

    this.load([
      "src/asmjit/x86/x86inst.cpp",
      "src/asmjit/x86/x86inst.h"
    ]);
  }

  signaturesFromInsts(insts) {
    const signatures = new SignatureArray();

    for (var i = 0; i < insts.length; i++) {
      const inst = insts[i];
      const ops = inst.operands;

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
        // and skip all instructions having implicit `imm` operand of `1`
        // (handled fine by asmjit).
        if (this.opBlackList[mem] === true || iop.immValue !== null) {
          row = null;
          break;
        }

        if (reg === "r8") reg = "r8lo";
        if (reg === "seg") reg = "sreg";
        if (reg === "st(i)") reg = "fp";
        if (reg === "st(0)") reg = "fp0";

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

        const op = new OSignature();

        if (iop.read) op.flags.read = true;
        if (iop.write) op.flags.write = true;

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

          // Exception: Allow LEA to contain any memory size.
          if (inst.name === "lea") MapUtils.add(op.flags, MemOp);
        }

        if (imm) {
          if (iop.immSign === "any" || iop.immSign === "signed"  ) op.flags["i" + imm] = true;
          if (iop.immSign === "any" || iop.immSign === "unsigned") op.flags["u" + imm] = true;
        }
        if (rel) op.flags["rel" + rel] = true;

        row.push(op);
      }

      if (row)
        signatures.push(row);
    }

    signatures.calcImplicitMemSize();
    signatures.simplify();
    signatures.compact();

    signatures.simplify();
    signatures.compact();

    return signatures;
  }

  // --------------------------------------------------------------------------
  // [Parse]
  // --------------------------------------------------------------------------

  parse() {
    const data = this.dataOf("src/asmjit/x86/x86inst.cpp");
    const re = new RegExp(
      "INST\\(" +
        "([A-Za-z0-9_]+)\\s*"              + "," + // [01] Instruction.
        "([^,]+)"                          + "," + // [02] Encoding.
        "(.{26}[^,]*)"                     + "," + // [03] Opcode[0].
        "(.{26}[^,]*)"                     + "," + // [04] Opcode[1].
        "([^,]+)"                          + "," + // [05] Write-Index.
        "([^,]+)"                          + "," + // [06] Write-Size.
        // --- autogenerated fields ---
        "([^\\)]+)"                        + "," + // [07] NameIndex.
        "([^\\)]+)"                        + "," + // [08] CommonDataIndex.
        "([^\\)]+)"                        + "\\)",// [09] OperationDataIndex.
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

      const group = GenUtils.groupOf(name);
      if (name && !group.length)
        console.log(`INSTRUCTION '${name}' not found in asmdb`);

      const flags         = GenUtils.flagsOf(group);
      const signatures    = this.signaturesFromInsts(group);
      const singleRegCase = GenUtils.singleRegCase(name);
      const jumpType      = GenUtils.jumpType(name);

      this.addInst({
        id                : 0,             // Instruction id (numeric value).
        name              : name,          // Instruction name.
        enum              : enum_,         // Instruction enum without `kId` prefix.
        encoding          : encoding,      // Instruction encoding.
        opcode0           : opcode0,       // Primary opcode.
        opcode1           : opcode1,       // Secondary opcode.
        flags             : flags,
        writeIndex        : writeIndex,
        writeSize         : writeSize,
        signatures        : signatures,    // Rows containing instruction signatures.
        singleRegCase     : singleRegCase,
        jumpType          : jumpType,

        nameIndex         : -1,            // Instruction name-index.
        altOpCodeIndex    : -1,            // Index to X86InstDB::altOpCodeTable.
        commonDataIndex   : -1,
        operationDataIndex: -1,
        sseToAvxDataIndex : -1,

        signatureIndex    : -1,
        signatureCount    : -1
      });
      this.maxOpRows = Math.max(this.maxOpRows, signatures.length);
    }

    if (this.instArray.length === 0)
      throw new Error("X86Generator.parse(): Invalid parsing regexp (no data parsed)");

    console.log("Number of Instructions: " + this.instArray.length);
  }

  // --------------------------------------------------------------------------
  // [Generate]
  // --------------------------------------------------------------------------

  generate() {
    // Order doesn't matter here.
    this.generateIdData();
    this.generateNameData();
    this.generateOperationData();
    this.generateSseToAvxData();
    this.generateAltOpCodeData();
    this.generateSignatureData();

    // These must be last, and order matters.
    this.generateCommonData();
    this.generateInstData();

    return this;
  }

  // --------------------------------------------------------------------------
  // [Generate - AltOpCodeData]
  // --------------------------------------------------------------------------

  generateAltOpCodeData() {
    const table = new base.IndexedArray();
    for (var i = 0; i < this.instArray.length; i++) {
      const inst = this.instArray[i];
      inst.altOpCodeIndex = table.addIndexed(StringUtils.padLeft(inst.opcode1, 26));
    }

    var s = `const uint32_t X86InstDB::altOpCodeData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`;
    return this.inject("altOpCodeData", StringUtils.disclaimer(s), table.length * 4);
  }

  // --------------------------------------------------------------------------
  // [Generate - OperationData]
  // --------------------------------------------------------------------------

  generateOperationData() {
    const instArray = this.instArray;
    const table = new base.IndexedArray();

    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      const group = GenUtils.groupOf(inst.name);

      var opFlags = GenUtils.operationFlagsOf(group).map(function(f) { return `OP_FLAG(${f})`; });
      if (!opFlags.length) opFlags.push("0");

      var features = GenUtils.cpuFeaturesOf(group).map(function(f) { return `FEATURE(${f})`; });
      if (!features.length) features.push("0");

      var [r, w] = GenUtils.specialsOf(group);
      r = r.map(function(item) { return `SPECIAL(${item.replace(".", "_")})`; });
      w = w.map(function(item) { return `SPECIAL(${item.replace(".", "_")})`; });

      const opFlagsStr = opFlags.join(" | ");
      const featuresStr = features.join(", ");
      const rStr = r.join(" | ") || "0";
      const wStr = w.join(" | ") || "0";

      inst.operationDataIndex = table.addIndexed(`{ ${opFlagsStr}, { ${featuresStr} }, ${rStr}, ${wStr} }`);
    }

    var s = `#define OP_FLAG(F) X86Inst::kOperation##F\n` +
            `#define FEATURE(F) CpuInfo::kX86Feature##F\n` +
            `#define SPECIAL(F) x86::kSpecialReg_##F\n` +
            `const X86Inst::OperationData X86InstDB::operationData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef SPECIAL\n` +
            `#undef FEATURE\n` +
            `#undef OP_FLAG\n` ;
    return this.inject("operationData", StringUtils.disclaimer(s), table.length * 16);
  }

  // --------------------------------------------------------------------------
  // [Generate - SseToAvxData]
  // --------------------------------------------------------------------------

  generateSseToAvxData() {
    const instArray = this.instArray;
    const instMap = this.instMap;
    const table = new base.IndexedArray();

    function getSseToAvxInsts(insts) {
      const combinations = [];
      for (var x = 0; x < insts.length; x++) {
        const inst = insts[x];
        const ops = inst.operands;

        // SSE instruction does never share its name with AVX one.
        if (/^(VEX|XOP|EVEX)$/.test(inst.prefix))
          return null;

        var ok = false;
        for (var y = 0; y < ops.length; y++) {
          if (ops[y].reg === "xmm")
            ok = true;

          // There is no AVX instruction that works with MMX regs.
          if (ops[y].reg === "mm") {
            ok = false;
            break;
          }
        }

        if (ok) combinations.push(inst);
      }

      return combinations.length ? combinations : null;
    }

    function calcSseToAvxData(insts) {
      const out = {
        mode : "None", // No conversion by default.
        delta: 0       // 0 if no conversion is possible.
      };

      const sseInsts = getSseToAvxInsts(insts);
      if (!sseInsts) return out;

      const sseName = sseInsts[0].name;
      const avxName = "v" + sseName;
      const avxInsts = GenUtils.groupOf(avxName);

      if (!avxInsts.length) {
        if (DEBUG)
          console.log(`SseToAvx: Instruction '${sseName}' has no AVX counterpart`);
        return out;
      }

      if (avxName === "vblendvpd" || avxName === "vblendvps" || avxName === "vpblendvb") {
        // Special cases first.
        out.mode = "Blend";
      }
      else {
        // Common case, deduce conversion mode by checking both SSE and AVX instructions.
        const map = Object.create(null);
        for (var sseIndex = 0; sseIndex < sseInsts.length; sseIndex++) {
          const sseInst = sseInsts[sseIndex];
          var match = false;

          for (var avxIndex = 0; avxIndex < avxInsts.length; avxIndex++) {
            const avxInst = avxInsts[avxIndex];

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
            return out;
          }
        }

        out.mode = (map.raw && !map.nds) ? "Move" : (map.raw && map.nds) ? "MoveIfMem" : "Extend";
      }
      out.delta = instMap[avxName].id - instMap[sseName].id;
      return out;
    }

    // This will receive a zero index, which means that no translation is possible.
    table.addIndexed("{ " + StringUtils.padLeft(`X86Inst::kSseToAvxNone`, 27) + ", " + StringUtils.padLeft("0", 4) + " }");

    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];

      // If it's not `-1` it's an AVX instruction that shares the SseToAvx
      // data. So we won't touch it as it already has `sseToAvxDataIndex`.
      if (inst.sseToAvxDataIndex === -1) {
        const data = calcSseToAvxData(GenUtils.groupOf(inst.name));
        inst.sseToAvxDataIndex = table.addIndexed("{ " + StringUtils.padLeft(`X86Inst::kSseToAvx${data.mode}`, 27) + ", " + StringUtils.padLeft(data.delta, 4) + " }");
        if (data.delta !== 0)
          instMap["v" + inst.name].sseToAvxDataIndex = inst.sseToAvxDataIndex;
      }
    }

    var s = `const X86Inst::SseToAvxData X86InstDB::sseToAvxData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n`;
    return this.inject("sseToAvxData", StringUtils.disclaimer(s), table.length * 2);
  }

  // --------------------------------------------------------------------------
  // [Generate - SignatureData]
  // --------------------------------------------------------------------------

  generateSignatureData() {
    const instArray = this.instArray;

    const opMap = Object.create(null);
    const opArr = [];

    const signatureMap = Object.create(null);
    const signatureArr = [];

    const noOperand = "OSIGNATURE(0, 0, 0, 0xFF)";
    opMap[noOperand] = [0];
    opArr.push(noOperand);

    function findSignaturesIndex(rows) {
      const len = rows.length;
      if (!len) return 0;

      const indexes = signatureMap[rows[0].data];
      if (indexes === undefined) return -1;

      for (var i = 0; i < indexes.length; i++) {
        const index = indexes[i];
        if (index + len > signatureArr.length) continue;

        var ok = true;
        for (var j = 0; j < len; j++) {
          if (signatureArr[index + j].data !== rows[j].data) {
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
      const result = signatureArr.length;

      for (var i = 0; i < signatures.length; i++) {
        const signature = signatures[i];
        const idx = signatureArr.length;

        if (!hasOwn.call(signatureMap, signature.data))
          signatureMap[signature.data] = [];

        signatureMap[signature.data].push(idx);
        signatureArr.push(signature);
      }

      return result;
    }

    for (var len = this.maxOpRows; len >= 0; len--) {
      for (var i = 0; i < instArray.length; i++) {
        const inst = instArray[i];
        const signatures = inst.signatures;
        if (signatures.length !== len) continue;

        const signatureEntries = [];
        for (var j = 0; j < len; j++) {
          const signature = signatures[j];

          var signatureEntry = `ISIGNATURE(${signature.length}, ${signature.x86 ? 1 : 0}, ${signature.x64 ? 1 : 0}, ${signature.implicit}`;
          var signatureComment = signature.toString();

          var x = 0;
          while (x < signature.length) {
            const h = signature[x].toAsmJitOpData();
            var index = -1;
            if (!hasOwn.call(opMap, h)) {
              index = opArr.length;
              opMap[h] = index;
              opArr.push(h);
            }
            else {
              index = opMap[h];
            }

            signatureEntry += `, ${StringUtils.padLeft(index, 3)}`;
            x++;
          }

          while (x < 6) {
            signatureEntry += `, ${StringUtils.padLeft(0, 3)}`;
            x++;
          }

          signatureEntry += `)`;
          signatureEntries.push({ data: signatureEntry, comment: signatureComment, refs: 0 });
        }

        var count = signatureEntries.length;
        var index = findSignaturesIndex(signatureEntries);

        if (index === -1)
          index = indexSignatures(signatureEntries);

        signatureArr[index].refs++;
        inst.signatureIndex = index;
        inst.signatureCount = count;
      }
    }

    var s = "#define FLAG(flag) X86Inst::kOp##flag\n" +
            "#define MEM(mem) X86Inst::kMemOp##mem\n" +
            "#define OSIGNATURE(flags, memFlags, extFlags, regId) \\\n" +
            "  { uint32_t(flags), uint16_t(memFlags), uint8_t(extFlags), uint8_t(regId) }\n" +
                StringUtils.makeCxxArray(opArr, "const X86Inst::OSignature X86InstDB::oSignatureData[]") +
            "#undef OSIGNATURE\n" +
            "#undef MEM\n" +
            "#undef FLAG\n" +
            "\n" +
            "#define ISIGNATURE(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5) \\\n" +
            "  { count, (x86 ? uint8_t(X86Inst::kArchMaskX86) : uint8_t(0)) |      \\\n" +
            "           (x64 ? uint8_t(X86Inst::kArchMaskX64) : uint8_t(0)) ,      \\\n" +
            "    implicit,                                                         \\\n" +
            "    0,                                                                \\\n" +
            "    { o0, o1, o2, o3, o4, o5 }                                        \\\n" +
            "  }\n" +
            StringUtils.makeCxxArrayWithComment(signatureArr, "const X86Inst::ISignature X86InstDB::iSignatureData[]") +
            "#undef ISIGNATURE\n";
    return this.inject("signatureData", StringUtils.disclaimer(s), opArr.length * 8 + signatureArr.length * 8);
  }

  // --------------------------------------------------------------------------
  // [Generate - CommonData]
  // --------------------------------------------------------------------------

  generateCommonData() {
    const table = new base.IndexedArray();

    for (var i = 0; i < this.instArray.length; i++) {
      const inst = this.instArray[i];
      const group = GenUtils.groupOf(inst.name);

      const flags         = inst.flags.map(function(flag) { return `F(${flag})`; }).join("|") || "0";
      const singleRegCase = `SINGLE_REG(${inst.singleRegCase})`;
      const jumpType      = `JUMP_TYPE(${inst.jumpType})`;

      const item = "{ " + StringUtils.padLeft(flags                  , 54) + ", " +
                          StringUtils.padLeft(inst.writeIndex        ,  3) + ", " +
                          StringUtils.padLeft(inst.writeSize         ,  3) + ", " +
                          StringUtils.padLeft(inst.altOpCodeIndex    ,  3) + ", " +
                          StringUtils.padLeft(inst.signatureIndex    ,  3) + ", " +
                          StringUtils.padLeft(inst.signatureCount    ,  2) + ", " +
                          StringUtils.padLeft(jumpType               , 22) + ", " +
                          StringUtils.padLeft(singleRegCase          , 16) + ", " + "0 }";
      inst.commonDataIndex = table.addIndexed(item);
    }

    var s = `#define F(VAL) X86Inst::kFlag##VAL\n` +
            `#define JUMP_TYPE(VAL) AnyInst::kJumpType##VAL\n` +
            `#define SINGLE_REG(VAL) X86Inst::kSingleReg##VAL\n` +
            `const X86Inst::CommonData X86InstDB::commonData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef SINGLE_REG\n` +
            `#undef JUMP_TYPE\n` +
            `#undef F\n`;
    return this.inject("commonData", StringUtils.disclaimer(s), table.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Generate - InstData]
  // --------------------------------------------------------------------------

  generateInstData() {
    var s = StringUtils.format(this.instArray, "", false, function(inst) {
      return "INST(" +
        StringUtils.padLeft(inst.enum              , 16) + ", " +
        StringUtils.padLeft(inst.encoding          , 19) + ", " +
        StringUtils.padLeft(inst.opcode0           , 26) + ", " +
        StringUtils.padLeft(inst.opcode1           , 26) + ", " +
        StringUtils.padLeft(inst.writeIndex        ,  1) + ", " +
        StringUtils.padLeft(inst.writeSize         ,  1) + ", " +
        StringUtils.padLeft(inst.nameIndex         ,  4) + ", " +
        StringUtils.padLeft(inst.commonDataIndex   ,  3) + ", " +
        StringUtils.padLeft(inst.operationDataIndex,  3) + ", " +
        StringUtils.padLeft(inst.sseToAvxDataIndex ,  2) + ")";
    }) + "\n";
    return this.inject("instData", s, this.instArray.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Reimplement]
  // --------------------------------------------------------------------------

  getCommentOf(name) {
    var insts = GenUtils.groupOf(name);
    if (!insts.length) return "";

    var features = GenUtils.cpuFeaturesOf(insts);
    var comment = GenUtils.cpuArchOf(insts);

    if (features.length) {
      comment += " {";

      const vl = features.indexOf("AVX512_VL");
      if (vl !== -1) features.splice(vl, 1);
      comment += features.join("|");
      if (vl !== -1) comment += "+VL";

      comment += "}";
    }

    return comment;
  }

  // --------------------------------------------------------------------------
  // [Print]
  // --------------------------------------------------------------------------

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
    isa.instructionNames.forEach(function(name) {
      var insts = isa.query(name);
      if (!this.instMap[name] && ignored[name] !== true) {
        console.log(`MISSING INSTRUCTION '${name}'`);
        var inst = this.newInstFromInsts(insts);
        if (inst) {
          out += "  INST(" +
            StringUtils.padLeft(inst.enum          , 16) + ", " +
            StringUtils.padLeft(inst.encoding      , 23) + ", " +
            StringUtils.padLeft(inst.opcode0       , 26) + ", " +
            StringUtils.padLeft(inst.opcode1       , 26) + ", " +
            StringUtils.padLeft(inst.writeIndex    ,  2) + ", " +
            StringUtils.padLeft(inst.writeSize     ,  2) + ", " +
            StringUtils.padLeft("0", 4) + ", " +
            StringUtils.padLeft("0", 3) + ", " +
            StringUtils.padLeft("0", 3) + ", " +
            StringUtils.padLeft("0", 3) + "),\n";
        }
      }
    }, this);
    console.log(out);
  }

  newInstFromInsts(insts) {
    function composeOpCode(obj) {
      return `${obj.type}(${obj.prefix},${obj.opcode},${obj.o},${obj.l},${obj.w},${obj.ew},${obj.en},${obj.tt})`;
    }

    function GetAccess(inst) {
      var operands = inst.operands;
      if (!operands.length) return "";

      var op = operands[0];
      if (op.read && op.write)
        return "RW";
      else if (op.read)
        return "RO";
      else
        return "WO";
    }

    var inst = insts[0];

    var id       = this.instArray.length;
    var name     = inst.name;
    var enum_    = name[0].toUpperCase() + name.substr(1);

    var opcode   = inst.opcodeHex;
    var rm       = inst.rm;
    var mm       = inst.mm;
    var pp       = inst.pp;
    var encoding = inst.encoding;
    var prefix   = inst.prefix;

    var access   = GetAccess(inst);

    var vexL     = undefined;
    var vexW     = undefined;
    var evexW    = undefined;

    for (var i = 1; i < insts.length; i++) {
      inst = insts[i];

      if (opcode   !== inst.opcode    ) return null;
      if (rm       !== inst.rm        ) return null;
      if (mm       !== inst.mm        ) return null;
      if (pp       !== inst.pp        ) return null;
      if (encoding !== inst.encoding  ) return null;
      if (prefix   !== inst.prefix    ) return null;
      if (access   !== GetAccess(inst)) return null;
    }

    var ppmm = StringUtils.padLeft(pp, 2).replace(/ /g, "0") +
               StringUtils.padLeft(mm, 4).replace(/ /g, "0") ;

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
}

// ----------------------------------------------------------------------------
// [Main]
// ----------------------------------------------------------------------------

function main() {
  const gen = new X86Generator();
  gen.parse();
  gen.generate();
  gen.printMissing();
  gen.dumpTableSizes();
  gen.save();
}
main();
