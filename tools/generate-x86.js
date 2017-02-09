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
const StringUtils = base.StringUtils;

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

// var fs = require("fs");
// fs.writeFileSync("X86.JSON", JSON.stringify(isa.instructionMap, null, 2));

// ----------------------------------------------------------------------------
// [GenUtils]
// ----------------------------------------------------------------------------

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

class GenUtils {
  // Get group of instructions having the same name as understood by AsmJit.
  static groupOf(name) {
    const remapped = RemappedInsts[name];
    if (!remapped) return isa.query(name);

    const insts = isa.query(remapped.names);
    const rep = remapped.rep;
    if (rep === null) return insts;

    return insts.filter(function(inst) {
      return rep === !!(inst.attributes.REP || inst.attributes.REPZ || inst.attributes.REPNZ);
    });
  }

  static archOf(group) {
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

  // Calculate a family of a group of instructions.
  static familyOf(group) {
    var i, j;

    var nSSE = 0;
    var nAVX = 0;

    for (i = 0; i < group.length; i++) {
      const inst = group[i];
      const ops = inst.operands;

      if (/^(VEX|XOP|EVEX)$/.test(inst.prefix)) {
        for (j = 0; j < ops.length; j++) {
          if (/^(xmm|ymm|zmm)$/.test(ops[j].reg)) {
            nAVX++;
            break;
          }
        }
      }
      else {
        for (j = 0; j < ops.length; j++) {
          if (/^(mm|xmm)$/.test(ops[j].reg)) {
            nSSE++;
            break;
          }
        }
      }
    }

    if (nSSE === group.length) return "Sse";
    if (nAVX === group.length) return "Avx";

    return "General";
  }

  static featuresOf(group) {
    const features = Object.create(null);

    for (var i = 0; i < group.length; i++)
      for (var feature in group[i].extensions)
        features[feature] = true;

    const result = Object.getOwnPropertyNames(features);
    result.sort();
    return result;
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

function getEFlagsMask(eflags, passing) {
  var msk = 0x0;
  var bit = 0x1;

  for (var i = 0; i < 8; i++, bit <<= 1)
    if (passing.indexOf(eflags[i]) !== -1)
      msk |= bit;

  return msk;
}

// Compose opcode back to what INST expects.
function composeOpCode(obj) {
  var w = obj.w;
  var ew = obj.ew;

  return `${obj.type}(${obj.prefix},${obj.opcode},${obj.o},${obj.l},${w},${ew},${obj.en})`;
}


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
  "m64"     : 35,
  "m80"     : 36,
  "m128"    : 37,
  "m256"    : 38,
  "m512"    : 39,
  "m1024"   : 40,
  "mib"     : 41,
  "vm32x"   : 42,
  "vm32y"   : 43,
  "vm32z"   : 44,
  "vm64x"   : 45,
  "vm64y"   : 46,
  "vm64z"   : 47,
  "memBase" : 48,
  "memES"   : 49,
  "memDS"   : 50,

  "i4"      : 60,
  "i8"      : 61,
  "i16"     : 62,
  "i32"     : 63,
  "i64"     : 64,

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
  "i8"      : "FLAG(I8)",
  "i16"     : "FLAG(I16)",
  "i32"     : "FLAG(I32)",
  "i64"     : "FLAG(I64)",

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
    const af = this.flags;
    const bf = other.flags;

    for (var k in af) if (!hasOwn.call(bf, k)) return false;
    for (var k in bf) if (!hasOwn.call(af, k)) return false;

    return true;
  }

  xor(other) {
    const result = Object.create(null);

    const af = this.flags;
    const bf = other.flags;

    for (var k in af) if (!hasOwn.call(bf, k)) result[k] = true;
    for (var k in bf) if (!hasOwn.call(af, k)) result[k] = true;

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
    if (flags.al && flags.r8lo) delete flags["al"];
    if (flags.ah && flags.r8hi) delete flags["ah"];
    if (flags.ax && flags.r16) delete flags["ax"];
    if (flags.eax && flags.r32) delete flags["eax"];
    if (flags.rax && flags.r64) delete flags["rax"];

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

        case "m8"      : mFlags.mem = true; mMemFlags.m8    = true; break;
        case "m16"     : mFlags.mem = true; mMemFlags.m16   = true; break;
        case "m32"     : mFlags.mem = true; mMemFlags.m32   = true; break;
        case "m64"     : mFlags.mem = true; mMemFlags.m64   = true; break;
        case "m80"     : mFlags.mem = true; mMemFlags.m80   = true; break;
        case "m128"    : mFlags.mem = true; mMemFlags.m128  = true; break;
        case "m256"    : mFlags.mem = true; mMemFlags.m256  = true; break;
        case "m512"    : mFlags.mem = true; mMemFlags.m512  = true; break;
        case "m1024"   : mFlags.mem = true; mMemFlags.m1024 = true; break;
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
        case "i8"      :
        case "i16"     :
        case "i32"     :
        case "i64"     : mFlags[k] = true; break;

        case "rel8"    :
        case "rel32"   :
          mFlags.i32 = true;
          mFlags.i64 = true;
          mFlags[k] = true;
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
  constructor() {
    super();
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

  simplify() {
    for (var i = 0; i < this.length; i++)
      this[i].simplify();
  }

  toString() {
    return "[" + this.join(", ") + "]";
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

      var row = new ISignature();

      row.x86 = inst.arch === "ANY" || inst.arch === "X86";
      row.x64 = inst.arch === "ANY" || inst.arch === "X64";

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

        if (mem) op.flags[mem] = true;
        if (imm) op.flags["i" + imm] = true;
        if (rel) op.flags["rel" + rel] = true;

        row.push(op);
      }

      if (row)
        signatures.push(row);
    }

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
        "(.{38}[^,]*)"                     + "," + // [05] IFLAGS.
        "\\s*EF\\(([A-Z_]+)\\)\\s*"        + "," + // [06] EFLAGS.
        "([^,]+)"                          + "," + // [07] Write-Index.
        "([^,]+)"                          + "," + // [08] Write-Size.
        // --- autogenerated fields ---
        "([^\\)]+)"                        + "," + // [09] FamilyType.
        "([^\\)]+)"                        + "," + // [10] FamilyIndex.
        "([^\\)]+)"                        + "," + // [11] NameIndex.
        "([^\\)]+)"                        + "\\)",// [12] ExtIndex.
      "g");

    var m;
    while ((m = re.exec(data)) !== null) {
      var enum_       = m[1];
      var name        = enum_ === "None" ? "" : enum_.toLowerCase();
      var encoding    = m[2].trim();
      var opcode0     = m[3].trim();
      var opcode1     = m[4].trim();
      var iflags      = m[5].trim();
      var eflags      = m[6];
      var writeIndex  = StringUtils.trimLeft(m[7]);
      var writeSize   = StringUtils.trimLeft(m[8]);

      const insts = GenUtils.groupOf(name);
      if (!insts.length)
        console.log(`INSTRUCTION '${name}' not found in asmdb`);

      const signatures    = this.signaturesFromInsts(insts);
      const singleRegCase = GenUtils.singleRegCase(name);
      const jumpType      = GenUtils.jumpType(name);

      this.addInst({
        id            : 0,             // Instruction id (numeric value).
        name          : name,          // Instruction name.
        enum          : enum_,         // Instruction enum without `kId` prefix.
        encoding      : encoding,      // Instruction encoding.
        opcode0       : opcode0,       // Primary opcode.
        opcode1       : opcode1,       // Secondary opcode.
        iflags        : iflags,
        eflags        : eflags,
        writeIndex    : writeIndex,
        writeSize     : writeSize,
        signatures    : signatures,    // Rows containing instruction signatures.
        singleRegCase : singleRegCase,
        jumpType      : jumpType,

        familyType    : "kFamilyNone", // Family type.
        familyIndex   : 0,             // Index to a family-specific data.

        nameIndex     : -1,            // Instruction name-index.
        altOpCodeIndex: -1,            // Index to X86InstDB::altOpCodeTable.
        commonIndex   : -1,
        signatureIndex: -1,
        signatureCount: -1
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
    this.generateFpuData();
    this.generateSseData();
    this.generateAvxData();
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
  // [Generate - FpuData]
  // --------------------------------------------------------------------------

  generateFpuData() {
    return this;
  }

  // --------------------------------------------------------------------------
  // [Generate - SseData]
  // --------------------------------------------------------------------------

  generateSseData() {
    const instArray = this.instArray;
    const instMap = this.instMap;

    const prefix = "X86Inst::SseData::";
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

    function calcSseToAvxData(insts, out) {
      const sseInsts = getSseToAvxInsts(insts);
      if (!sseInsts) return 0;

      const sseName = sseInsts[0].name;
      const avxName = "v" + sseName;
      const avxInsts = GenUtils.groupOf(avxName);

      if (!avxInsts.length) {
        console.log(`SseToAvx: Instruction '${sseName}' has no AVX counterpart`);
        return null;
      }

      if (avxName === "vblendvpd" || avxName === "vblendvps" || avxName === "vpblendvb") {
        // Special cases first.
        out.avxConvMode = "Blend";
      }
      else {
        // Common case, deduce conversion mode by checking both SSE and AVX instructions' operands.
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
            return null;
          }
        }

        out.avxConvMode = (map.raw && !map.nds) ? "Move" :
                          (map.raw &&  map.nds) ? "MoveIfMem" : "Extend";
      }
      out.avxConvDelta = instMap[avxName].id - instMap[sseName].id;
    }

    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      const insts = GenUtils.groupOf(inst.name);

      if (!insts.length)
        continue;

      if (GenUtils.familyOf(insts) === "Sse") {
        const data = {
          avxConvMode : "None", // No conversion by default.
          avxConvDelta: 0       // 0 if no conversion is possible.
        };
        calcSseToAvxData(insts, data);

        var features = GenUtils.featuresOf(insts).map(function(f) { return StringUtils.padLeft(`FEATURE(${f})`, 19); }).join(`|\n${kIndent}  `);
        if (!features) features = StringUtils.padLeft("0", 19);

        inst.familyType = "kFamilySse";
        inst.familyIndex = table.addIndexed(
          "{ " + features + ", " +
                 StringUtils.padLeft(`AVX_CONV(${data.avxConvMode})`, 20) + ", " +
                 StringUtils.padLeft(data.avxConvDelta              ,  4) + " }"
        );
      }
    }

    var s = `#define FEATURE(F) ${prefix}kFeature##F\n` +
            `#define AVX_CONV(MODE) ${prefix}kAvxConv##MODE\n` +
            `const X86Inst::SseData X86InstDB::sseData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef AVX_CONV\n` +
            `#undef FEATURE\n`;
    return this.inject("sseData", StringUtils.disclaimer(s), table.length * 4);
  }

  // --------------------------------------------------------------------------
  // [Generate - AvxData]
  // --------------------------------------------------------------------------

  generateAvxData() {
    const instArray = this.instArray;

    const prefix = "X86Inst::AvxData::";
    const table = new base.IndexedArray();

    function fillFlags(insts, out) {
      for (var i = 0; i < insts.length; i++) {
        const inst = insts[i];
        if (inst.prefix === "EVEX") {
          if (inst.kmask) out.Masking = true;
          if (inst.zmask) out.Zeroing = true;
          if (inst.rnd) out.ER = true;
          if (inst.sae) out.SAE = true;
          if (inst.broadcast) out["Broadcast" + String(inst.elementSize)] = true;
        }
      }
    }

    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      const insts = GenUtils.groupOf(inst.name);

      if (!insts.length)
        continue;

      if (GenUtils.familyOf(insts) === "Avx") {
        var features = GenUtils.featuresOf(insts).map(function(f) { return StringUtils.padLeft(`FEATURE(${f})`, 19); }).join(`|\n${kIndent}  `);
        if (!features) features = StringUtils.padLeft("0", 19);

        const flagsMap = {};
        fillFlags(insts, flagsMap);

        const flagsArr = Object.getOwnPropertyNames(flagsMap);
        flagsArr.sort();

        var flags = flagsArr.map(function(flag) { return `FLAG(${flag})`; }).join(" | ");
        if (!flags) flags = "0";

        inst.familyType = "kFamilyAvx";
        inst.familyIndex = table.addIndexed("{ " + features + ", " + flags + " }");
      }
    }

    var s = `#define FEATURE(F) ${prefix}kFeature##F\n` +
            `#define FLAG(F) ${prefix}kFlag##F\n` +
            `const X86Inst::AvxData X86InstDB::avxData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef FLAG\n` +
            `#undef FEATURE\n`;
    return this.inject("avxData", StringUtils.disclaimer(s), table.length * 8);
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

    var s = "#define ISIGNATURE(count, x86, x64, implicit, o0, o1, o2, o3, o4, o5) \\\n" +
            "  { count, (x86 ? uint8_t(X86Inst::kArchMaskX86) : uint8_t(0)) |      \\\n" +
            "           (x64 ? uint8_t(X86Inst::kArchMaskX64) : uint8_t(0)) ,      \\\n" +
            "    implicit,                                                         \\\n" +
            "    0,                                                                \\\n" +
            "    { o0, o1, o2, o3, o4, o5 }                                        \\\n" +
            "  }\n" +
            StringUtils.makeCxxArrayWithComment(signatureArr, "static const X86Inst::ISignature _x86InstISignatureData[]") +
            "#undef ISIGNATURE\n" +
            "\n" +
            "#define FLAG(flag) X86Inst::kOp##flag\n" +
            "#define MEM(mem) X86Inst::kMemOp##mem\n" +
            "#define OSIGNATURE(flags, memFlags, extFlags, regId) \\\n" +
            "  { uint32_t(flags), uint16_t(memFlags), uint8_t(extFlags), uint8_t(regId) }\n" +
                StringUtils.makeCxxArray(opArr, "static const X86Inst::OSignature _x86InstOSignatureData[]") +
            "#undef OSIGNATURE\n" +
            "#undef MEM\n" +
            "#undef FLAG\n";
    return this.inject("signatureData", StringUtils.disclaimer(s), opArr.length * 8 + signatureArr.length * 8);
  }

  // --------------------------------------------------------------------------
  // [Generate - CommonData]
  // --------------------------------------------------------------------------

  generateCommonData() {
    const table = new base.IndexedArray();
    for (var i = 0; i < this.instArray.length; i++) {
      const inst = this.instArray[i];

      const eflagsIn      = StringUtils.decToHex(getEFlagsMask(inst.eflags, "RX" ), 2);
      const eflagsOut     = StringUtils.decToHex(getEFlagsMask(inst.eflags, "WXU"), 2);
      const singleRegCase = `SINGLE_REG(${inst.singleRegCase})`;
      const jumpType      = `JUMP_TYPE(${inst.jumpType})`;

      const item = "{ " + StringUtils.padLeft(inst.iflags        , 38) + ", " +
                          StringUtils.padLeft(inst.writeIndex    ,  3) + ", " +
                          StringUtils.padLeft(inst.writeSize     ,  3) + ", " +
                          eflagsIn                                     + ", " +
                          eflagsOut                                    + ", " +
                          StringUtils.padLeft(inst.altOpCodeIndex,  3) + ", " +
                          StringUtils.padLeft(inst.signatureIndex,  3) + ", " +
                          StringUtils.padLeft(inst.signatureCount,  2) + ", " +
                          StringUtils.padLeft(jumpType           , 22) + ", " +
                          StringUtils.padLeft(singleRegCase      , 16) + ", " + "0 }";
      inst.commonIndex = table.addIndexed(item);
    }

    var s = `#define JUMP_TYPE(VAL) AnyInst::kJumpType##VAL\n` +
            `#define SINGLE_REG(VAL) X86Inst::kSingleReg##VAL\n` +
            `const X86Inst::CommonData X86InstDB::commonData[] = {\n${StringUtils.format(table, kIndent, true)}\n};\n` +
            `#undef SINGLE_REG\n` +
            `#undef JUMP_TYPE\n`;
    return this.inject("commonData", StringUtils.disclaimer(s), table.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Generate - InstData]
  // --------------------------------------------------------------------------

  generateInstData() {
    var s = StringUtils.format(this.instArray, "", false, function(inst) {
      return "INST(" +
        StringUtils.padLeft(inst.enum       , 16) + ", " +
        StringUtils.padLeft(inst.encoding   , 19) + ", " +
        StringUtils.padLeft(inst.opcode0    , 26) + ", " +
        StringUtils.padLeft(inst.opcode1    , 26) + ", " +
        StringUtils.padLeft(inst.iflags     , 38) + ", " +
        "EF(" + inst.eflags + "), " +
        StringUtils.padLeft(inst.writeIndex ,  1) + ", " +
        StringUtils.padLeft(inst.writeSize  ,  1) + ", " +
        StringUtils.padLeft(inst.familyType , 11) + ", " +
        StringUtils.padLeft(inst.familyIndex,  3) + ", " +
        StringUtils.padLeft(inst.nameIndex  ,  4) + ", " +
        StringUtils.padLeft(inst.commonIndex,  3) + ")";
    }) + "\n";
    return this.inject("instData", s, this.instArray.length * 12);
  }

  // --------------------------------------------------------------------------
  // [Reimplement]
  // --------------------------------------------------------------------------

  getCommentOf(name) {
    var insts = GenUtils.groupOf(name);
    if (!insts.length) return "";

    var features = GenUtils.featuresOf(insts);
    var comment = GenUtils.archOf(insts);

    if (features.length) {
      comment += " {";

      const vl = features.indexOf("AVX512_VL");
      if (vl !== -1) features.splice(vl, 1);
      comment += features.join("|");
      if (vl !== -1) comment += " (VL)";

      comment += "}";
    }

    return comment;
  }

  // --------------------------------------------------------------------------
  // [Print]
  // --------------------------------------------------------------------------

  printMissing() {
    var out = "";

    function CPUFlags(insts) {
      var flags = {};
      for (var i = 0; i < insts.length; i++) {
        var inst = insts[i];
        for (var k in inst.extensions)
          flags[k] = true;
      }
      return Object.getOwnPropertyNames(flags).join("|");
    }

    isa.instructionNames.forEach(function(name) {
      var insts = isa.query(name);
      if (!this.instMap[name]) {
        console.log(`MISSING INSTRUCTION '${name}'`);
        var inst = this.newInstFromInsts(insts);
        if (inst) {
          out += "  INST(" +
            StringUtils.padLeft(inst.enum          , 16) + ", " +
            StringUtils.padLeft(inst.encoding      , 23) + ", " +
            StringUtils.padLeft(inst.opcode0       , 26) + ", " +
            StringUtils.padLeft(inst.opcode1       , 26) + ", " +
            StringUtils.padLeft(inst.iflags        , 38) + ", " +
            "EF(" + inst.eflags + "), " +
            StringUtils.padLeft(inst.writeIndex    , 2) + ", " +
            StringUtils.padLeft(inst.writeSize     , 2) + ", " +
            StringUtils.padLeft(inst.signatureIndex, 3) + ", " +
            StringUtils.padLeft(inst.signatureCount, 2) + ", " +
            StringUtils.padLeft("0", 3) + ", " +
            StringUtils.padLeft("0", 3) + ", " +
            StringUtils.padLeft("0", 3) + "),\n";
        }
      }
    }, this);
    console.log(out);
  }
}

// ----------------------------------------------------------------------------
// [Main]
// ----------------------------------------------------------------------------

function main() {
  const gen = new X86Generator();

  gen.parse();
  gen.generate();
  // gen.printMissing();
  gen.dumpTableSizes();
  gen.save();
}
main();

























/*
  newInstFromInsts(insts) {
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
    var enum_    = kX86InstId + name[0].toUpperCase() + name.substr(1);

    var opcode   = inst.opcode;
    var rm       = inst.rm;
    var mm       = inst.mm;
    var pp       = inst.pp;
    var encoding = inst.encoding;
    var prefix   = inst.prefix;

    var access   = GetAccess(inst);

    var eflags = ["_", "_", "_", "_", "_", "_", "_", "_"];

    for (var flag in inst.eflags) {
      var fOp = inst.eflags[flag];
      var fIndex = flag === "OF" ? 0 :
                   flag === "SF" ? 1 :
                   flag === "ZF" ? 2 :
                   flag === "AF" ? 3 :
                   flag === "PF" ? 4 :
                   flag === "CF" ? 5 :
                   flag === "DF" ? 6 : 7;
      var fChar = fOp === "W" ? "W" :
                  fOp === "R" ? "R" :
                  fOp === "X" ? "X" :
                  fOp === "0" ? "W" :
                  fOp === "1" ? "W" :
                  fOp === "U" ? "U" : "#";
      eflags[fIndex] = fChar;
    }

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

    var obj = AVX512Flags(insts);
    if (obj) {
      vexL = obj.vexL;
      vexW = obj.vexW;
      evexW = obj.evexW;
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
      en    : "_"
    });

    var iflags = [];

    if (access)
      iflags.push("F(" + access + ")");

    if (insts[0].prefix === "VEX")
      iflags.push("F(Vex)");

    return {
      id            : id,
      name          : name,
      enum          : enum_,
      encoding      : encoding,
      opcode0       : composed,
      opcode1       : "0",
      iflags        : iflags.join(""),
      eflags        : eflags.join(""),
      writeIndex    : "0",
      writeSize     : "0",
      signatures    : this.signaturesFromInsts(insts),
      nameIndex     : -1,
      commonIndex   : -1
    };
  }

  function genAPI() {
    var asm = fs.readFileSync("../src/asmjit/x86/x86assembler.h", "utf8");
    var list = ["AVX512_F", "AVX512_DQ", "AVX512_BW", "AVX512_CD", "AVX512_ER", "AVX512_PF", "AVX512_IFMA", "AVX512_VBMI"];

    function getAVX512Flag(inst) {
      for (var cpu in inst.extensions) {
        if (list.indexOf(cpu) !== -1) {
          return inst.extensions["AVX512_VL"] ? cpu + "-VL" : cpu;
        }
      }
      return "";
    }

    var out = "";
    var signatures = Object.create(null);
    var signNames = [];

    for (var i = 0; i < list.length; i++) {
      var cpu = list[i];

      const EncodeReg = {
        "xmm" : "X86Xmm",
        "ymm" : "X86Ymm",
        "zmm" : "X86Zmm",
        "r32" : "X86Gp",
        "r64" : "X86Gp",
        "k"   : "X86KReg",
        "eax" : "EAX",
        "edx" : "EDX",
        "ecx" : "ECX",
        "zdi" : "ZDI",
        "xmm0": "XMM0"
      }

      isa.forEach(function(name, inst) {
        if (inst.extensions.AVX || inst.extensions.AVX2 || getAVX512Flag(inst)) {
          var operands = inst.operands;
          var iops = [];
          var hasImm = -1;
          var hasMemReg = -1;
          var memReg = null;

          var enum_ = name[0].toUpperCase() + name.substr(1);
          iops.push(name, enum_);

          var flags = "";
          var avx512Flags = [];

          if (inst.zmask)
            avx512Flags.push("kz");
          else if (inst.kmask)
            avx512Flags.push("k");

          if (inst.rnd)
            avx512Flags.push("er");
          else if (inst.sae)
            avx512Flags.push("sae");

          if (inst.broadcast)
            avx512Flags.push(`b${inst.elementSize}`);

          var flags = getAVX512Flag(inst);
          if (flags) {
            if (flags.indexOf("-VL") !== -1)
              flags = flags.substr(0, flags.length - 3) + (avx512Flags.length ? "{" + avx512Flags.join("|") + "}" : "") + "-VL";
            else
              flags = flags + (avx512Flags.length ? "{" + avx512Flags.join("|") + "}" : "");
          }
          else if (inst.extensions.AVX2) {
            flags = "AVX2";
          }
          else if (inst.extensions.AVX) {
            flags = "AVX";
          }
          else {
            flags = "";
          }
          console.log(`${inst.name}: ${flags}`);

          for (var j = 0; j < operands.length; j++) {
            var operand = operands[j];

            if (operand.reg && operand.mem) {
              if (!EncodeReg[operand.reg])
                console.log(`UNHANDLED REG ${operand.reg}`);

              hasMemReg = iops.length;
              memReg = [EncodeReg[operand.reg], "X86Mem"];
              iops.push("?");
            }
            else if (operand.reg) {
              if (!EncodeReg[operand.reg])
                console.log(`UNHANDLED REG ${operand.reg}`);
              iops.push(EncodeReg[operand.reg]);
            }
            else if (operand.mem) {
              iops.push("X86Mem");
            }
            else if (operand.imm) {
              hasImm = iops.length;
              iops.push("Imm");
            }
            else {
              console.log(`UNHANDLED OPERAND (instruction ${inst.name}`);
            }
          }

          var prefix = "INST_" + (operands.length) + ((hasImm === -1) ? "x" : "i");
          var str = `${prefix}(${iops.join(", ")})`;
          var insts = [];

          if (hasMemReg !== -1) {
            insts.push(str.replace("?", memReg[0]));
            insts.push(str.replace("?", memReg[1]));
          }
          else {
            insts.push(str);
          }

          for (var j = 0; j < insts.length; j++) {
            str = insts[j];
            if (!signatures[str]) {
              signNames.push(str);
              signatures[str] = {};
            }
            signatures[str][flags] = true;
            console.log(`ADDED ${str} <- ${flags}`);
          }
        }
      });
    }

    for (var i = 0; i < signNames.length; i++) {
      var signature = signNames[i];
      var flags = Object.getOwnPropertyNames(signatures[signature]);
      flags.sort();

      var line = signature;
      if (flags.length) {
        var fstr = "";

        if (flags[0] === "AVX") {
          fstr = "AVX1"; flags.splice(0, 1);
        }
        else if (flags[0] === "AVX2") {
          fstr = "AVX2"; flags.splice(0, 1);
        }

        if (flags.length) {
          if (!fstr) fstr = "   ";
          fstr = StringUtils.padLeft(fstr, 5);
          fstr += flags.join(" ");
        }

        line = StringUtils.padLeft(line, 72);
        line += "// " + fstr;
      }

      out += line + "\n";
    }


    console.log(out);
  }

  function genOpcodeH() {
    var asm = fs.readFileSync("../src/asmjit/x86/x86assembler.h", "utf8");
    var list = ["AVX512_F", "AVX512_DQ", "AVX512_BW", "AVX512_CD", "AVX512_ER", "AVX512_PF", "AVX512_IFMA", "AVX512_VBMI"];

    function getAVX512Flag(inst) {
      for (var cpu in inst.extensions) {
        if (list.indexOf(cpu) !== -1) {
          return inst.extensions["AVX512_VL"] ? cpu + "-VL" : cpu;
        }
      }
      return "";
    }

    var out = "";
    var signatures = Object.create(null);
    var signNames = [];

    const Encode = {
      ""      : "",
      "xmm0"  : "xmmA",
      "xmm1"  : "xmmB",
      "xmm2"  : "xmmC",
      "xmm3"  : "xmmD",
      "ymm0"  : "ymmA",
      "ymm1"  : "ymmB",
      "ymm2"  : "ymmC",
      "ymm3"  : "ymmD",
      "zmm0"  : "zmmA",
      "zmm1"  : "zmmB",
      "zmm2"  : "zmmC",
      "zmm3"  : "zmmD",
      "r320"  : "gpdA",
      "r321"  : "gpdB",
      "r322"  : "gpdC",
      "r323"  : "gpdD",
      "r640"  : "gpzA",
      "r641"  : "gpzB",
      "r642"  : "gpzC",
      "r643"  : "gpzD",
      "k0"    : "kA",
      "k1"    : "kB",
      "k2"    : "kC",
      "k3"    : "kD",
      "m0"    : "anyptr_gpA",
      "m1"    : "anyptr_gpB",
      "m2"    : "anyptr_gpC",
      "m3"    : "anyptr_gpD",
      "vm32x0": "vx_ptr",
      "vm32x1": "vx_ptr",
      "vm32x2": "vx_ptr",
      "vm32x3": "vx_ptr",
      "vm32y0": "vy_ptr",
      "vm32y1": "vy_ptr",
      "vm32y2": "vy_ptr",
      "vm32y3": "vy_ptr",
      "vm32z0": "vz_ptr",
      "vm32z1": "vz_ptr",
      "vm32z2": "vz_ptr",
      "vm32z3": "vz_ptr"
    };

    isa.forEach(function(name, inst) {
      if (getAVX512Flag(inst)) {
        var operands = inst.operands;
        var iops = [];
        var hasImm = -1;
        var hasMemReg = -1;
        var memReg = null;

        var flags = "";
        var avx512Flags = [];

        if (inst.zmask)
          avx512Flags.push("kz");
        else if (inst.kmask)
          avx512Flags.push("k");

        if (inst.rnd)
          avx512Flags.push("er");
        else if (inst.sae)
          avx512Flags.push("sae");

        if (inst.broadcast)
          avx512Flags.push(`b${inst.elementSize}`);

        var flags = getAVX512Flag(inst);
        if (flags) {
          if (flags.indexOf("-VL") !== -1)
            flags = flags.substr(0, flags.length - 3) + (avx512Flags.length ? "{" + avx512Flags.join("|") + "}" : "") + "-VL";
          else
            flags = flags + (avx512Flags.length ? "{" + avx512Flags.join("|") + "}" : "");
        }
        else if (inst.extensions.AVX2) {
          flags = "AVX2";
        }
        else if (inst.extensions.AVX) {
          flags = "AVX";
        }
        else {
          flags = "";
        }

        for (var j = 0; j < operands.length; j++) {
          var operand = operands[j];

          var reg = operand.reg ? operand.reg + j : "";
          var mem = operand.mem ? operand.mem + j : "";

          if (reg && !Encode[reg]) console.log(`UNHANDLED REG ${reg}`);
          reg = Encode[reg];

          if (mem) mem = Encode[mem] ? Encode[mem] : Encode["m" + j];

          if (reg && mem) {
            hasMemReg = iops.length;
            memReg = [reg, mem];
            iops.push("?");
          }
          else if (reg) {
            iops.push(reg);
          }
          else if (mem) {
            iops.push(mem);
          }
          else if (operand.imm) {
            hasImm = iops.length;
            iops.push("0");
          }
          else {
            console.log(`UNHANDLED OPERAND (instruction ${inst.name}`);
          }
        }

        var str = `  a.${name}(${iops.join(", ")});`;
        var insts = [];

        if (hasMemReg !== -1) {
          insts.push(str.replace("?", memReg[0]));
          insts.push(str.replace("?", memReg[1]));
        }
        else {
          insts.push(str);
        }

        for (var j = 0; j < insts.length; j++) {
          str = insts[j];
          out += str + "\n";
        }
      }
    });

    console.log(out);
  }
*/
