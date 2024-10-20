// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

"use strict";

const fs = require("fs");
const path = require("path");

const commons = require("./generator-commons.js");
const cxx = require("./generator-cxx.js");
const core = require("./tablegen.js");

const asmdb = core.asmdb;

const kIndent = commons.kIndent;

const FATAL = commons.FATAL;
const ArrayUtils = commons.ArrayUtils;
const IndexedArray = commons.IndexedArray;
const StringUtils = commons.StringUtils;

const Injector = core.Injector;
const Output = core.Output;
const exp = core.exp;

const indent = cxx.Utils.indent;
const toHex = cxx.Utils.toHex;

function readJSON(fileName) {
  const content = fs.readFileSync(fileName);
  return JSON.parse(content);
}

function dict(src) {
  const dst = Object.create(null);
  if (src)
    Object.assign(dst, src);
  return dst;
}

function instNameToEnum(instName) {
  return !instName ? "kIdNone" : "kId" + instName[0].toUpperCase() + instName.substr(1);
}

function encodeValueOps(name, values, cutLsb) {
  if (!cutLsb)
    cutLsb = 0;

  const ops = [];
  for (let value of values) {
    const mask = ((1 << (value.size)) - 1) << (value.from + cutLsb);
    const shift = -cutLsb + value.index - value.from;

    if (shift == 0)
      ops.push(`(${name} & ${toHex(mask)})`);
    else
      ops.push(`((${name} & ${toHex(mask)}) ${shift >= 0 ? "<<" : ">>"} ${Math.abs(shift)})`);
  }

  return ops.join(" | ");
}

function regToOpcode(field, op, regType) {
  const q = regType === "v";

  let key = q ? "Q" + op.substr(1) : "R" + op.substr(1) ;
  let loHi = "";

  let v = "rId" + op.substr(1);
  let vars = {"type": "uint32_t", "name": v, "init": `${op}.as<Reg>().id()`};

  for (let value of field.values) {
    if (value.from === 0 && value.size === 1)
      loHi += `Lo${value.index}`;
    else if (value.from === 4 && value.size === 1)
      loHi += `Hi${value.index}`;
    else
      key += `At${value.index}Of${value.size}`;
  }

  const ops = encodeValueOps(q ? `(${v} << 1u)` : v, field.values);
  return { vars: vars, code: "opcode |= " + ops + ";", key: key + loHi};
}

const RW = dict({
  _: 0,
  R: 1,
  W: 2,
  X: 3
});

function regToField(s) {
  switch (s) {
    case "Sd" : return "Vd";
    case "Dd" : return "Vd";
    case "Qd" : return "Vd";

    case "Sn" : return "Vn";
    case "Dn" : return "Vn";
    case "Qn" : return "Vn";

    case "Sm" : return "Vm";
    case "Dm" : return "Vm";
    case "Qm" : return "Vm";

    case "Ss" : return "Vs";
    case "Ds" : return "Vs";
    case "Qs" : return "Vs";

    case "Sx" : return "Vx";
    case "Dx" : return "Vx";
    case "Qx" : return "Vx";

    case "Sx2": return "Vx2";
    case "Dx2": return "Vx2";
    case "Qx2": return "Vx2";

    default: return s;
  }
}

class OffEncodeContext {
  stringifyFunction(name) { return name; }

  stringifyVariable(name) {
    if (name === "off")
      return "uint32_t(mem->offsetLo32())"
    else
      return name;
  }

  stringifyImmediate(value) {
    return String(value) + "u";
  }
};

class ImmEncodeContext {
  constructor() {
    this.name = "";
    this.imms = dict();
    this.hasImms = false;
  }

  stringifyFunction(name) {
    return name;
  }

  stringifyVariable(name) {
    if (name in this.imms)
      return this.imms[name];
    return name;
  }

  stringifyImmediate(value) {
    return String(value) + "u";
  }
}

const standardImmEncode = {
  "immA": "ImmA(immA)"
};

function lsbFromMul(mul) {
  for (let i = 0; i < 32; i++)
    if ((mul & (1 << i)) != 0)
      return i;
  return 32;
}

function evaluateRegisterExpression(node, field, value) {
  return node.evaluate({
    variable(name) {
      if (name === field)
        return value;

      switch (name) {
        case "FP": return 11;
        case "PC": return 15;
        case "SP": return 13;
        case "LR": return 14;
        case "HI": return 8;
      }
    }
  });
}

function checkRegisterExpression(node, field, regId) {
  if (!node)
    return regId <= 15;
  return evaluateRegisterExpression(node, field, regId) != 0;
}

function compileMemExpression(field, val, node) {
  if (node) {
    return node.toString({
      stringifyVariable(name) {
        return name === field ? val :
               name === "PC" ? "15u" :
               name === "LR" ? "14u" :
               name === "SP" ? "13u" : name;

      }
    });
  }
  else {
    return `${val} <= 15u`;
  }
}

function getMemModeCheck(mem) {
  const mm = mem.memModes;
  if (!mm.offset && !mm.preIndex && !mm.postIndex) {
    return "mem->isFixedOffset()";
  }

  if (mm.offset && mm.preIndex && mm.postIndex) {
    return "";
  }

  if (mm.offset && !mm.preIndex && mm.postIndex) {
    return "!mem->isPreIndex()";
  }

  if (mm.offset && mm.preIndex && !mm.postIndex) {
    return "!mem->isPostIndex()";
  }

  if (mm.offset && !mm.preIndex && !mm.postIndex) {
    return "mem->isFixedOffset()";
  }

  if (!mm.offset && mm.preIndex && mm.postIndex) {
    return "!mem->isFixedOffset()";
  }

  if (!mm.offset && mm.preIndex && !mm.postIndex) {
    return "mem->isPreIndex()";
  }

  if (!mm.offset && !mm.preIndex && mm.postIndex) {
    return "mem->isPostIndex()";
  }

  FATAL("Unsupported offset mode combination");
}

function getOffsetType(field) {
  let s = "";

  for (let v of field.values) {
    if (s)
      s += "_";

    if (v.size > 1)
      s += `${v.from}To${v.from + v.size - 1}At${v.index}`;
    else
      s += `${v.from}At${v.index}`;
  }

  return s;
}

function getOffsetSize(field) {
  let size = 0;
  for (let v of field.values)
    size = Math.max(size, v.from + v.size);
  return size;
}

function getOffsetBitsToDiscard(restrict) {
  if (restrict && restrict.startsWith("*")) {
    const mul = parseInt(restrict.substring(1));
    return lsbFromMul(mul);
  }
  else {
    return 0;
  }
}

class InstructionHandler {
  constructor(id) {
    this.id = id;
    this.block = new cxx.Block();
    this.signatures = dict();
    this.commonVars = dict();
    this.condCnt = 0;
  }

  getBlock(checks, memIdx, relIdx, regListIdx) {
    const sgnCheck = checks[0];
    let block = this.signatures[sgnCheck];

    if (!block) {
      const cond = new cxx.If(sgnCheck);
      this.block.addEmptyLine();
      this.block.appendNode(cond);

      block = cond.body;
      this.signatures[sgnCheck] = block;

      if (memIdx !== -1) {
        block.addLine(`mem = &o${memIdx}.as<Mem>();`);
      }

      if (relIdx !== -1) {
        block.addLine(`rel = &o${relIdx};`);
      }

      if (regListIdx !== -1) {
        block.addLine(`const GpList& regList = o${regListIdx}.as<GpList>();`);
      }
    }

    for (let check of checks.slice(1)) {
      let cond = null;
      for (let node of block.nodes) {
        if (node.kind === "if" && node.cond === check) {
          cond = node;
          break;
        }
      }

      if (!cond) {
        cond = new cxx.If(check);
        block.addEmptyLine();
        block.appendNode(cond);
      }

      block = cond.body;
    }

    return block;
  }

  useCommonVar(name) {
    if (name in this.commonVars)
      return;

    switch (name) {
      case "sz":
      case "szM1":
        this.block.addVarDecl("uint32_t", "sz", "szFromDt(dtBits)");
        this.commonVars["sz"] = true;
        break;
    }
  }

  useCommonVars(obj) {
    for (let k in obj)
      this.useCommonVar(k);
  }

  nextCondEncoderName() {
    return "enc" + String(this.condCnt++);
  }
}

class Generator extends Injector {
  constructor(isa) {
    super()

    this.isa = isa;

    this.instructionIdMap = new Map();
    this.instructionIdTable = [];
    this.instructionHandlers = dict();
    this.instructionMetadata = [];

    this.emitHandlers = dict();
    this.emitterMap = dict();
    this.emitterTable = [];

    this.load([
      "src/asmjit/arm/a32assembler.cpp",
      "src/asmjit/arm/a32emitter.h",
      "src/asmjit/arm/a32globals.h",
      "src/asmjit/arm/a32instapi.cpp",
      "src/asmjit/arm/a32instdb.cpp",
      "src/asmjit/arm/a32instdb.h",
      "src/asmjit/arm/a32instdb_p.h"
    ]);
  }

  prepare() {
    let index = 0;
    const instNames = [""].concat(this.isa.instructionNames);

    for (let instName of instNames) {
      const id = this.instructionIdTable.length;
      const ih = new InstructionHandler(id);

      this.instructionIdMap.set(instName, ih);
      this.instructionIdTable.push(instName);
      this.instructionMetadata.push({
        name: instName,
        index: index,
        a32: false,
        t32: false,
        t16: false,
        comment: "",
        arch: "ARMv6",
        ext: dict()
      });

      index++;
    }
  }

  getInstructionHandler(name) {
    return this.instructionIdMap.get(name);
  }

  getEmitHandler(key, handler) {
    if (!this.emitHandlers[key])
      this.emitHandlers[key] = { code: handler, useCount: 1 };
    else
      this.emitHandlers[key].useCount++;
  }

  generateMetadata() {
    for (let im of this.instructionMetadata) {
      const instName = im.name;
      const instructions = this.isa.query(instName);

      for (let inst of instructions) {
        if (inst.encoding === "T16")
          im.t16 = true;

        if (inst.encoding === "T32")
          im.t32 = true;

        if (inst.encoding === "A32")
          im.a32 = true;

        for (let k in inst.ext) {
          if (k === "ARMv7+")
            im.arch = "ARMv7";
          else if (k === "ARMv8+")
            im.arch = "ARMv8";
          else
            im.ext[k] = true;
        }
      }
    }

    for (let im of this.instructionMetadata) {
      let enc = "";
      let ext = ArrayUtils.sorted(im.ext).filter(function(ext) { return ext !== "FP16"; }).join(" & ");

      if (im.name && !(im.a32 && (im.t16 || im.t32))) {
        enc = im.a32 ? " (ARM)" : " (THUMB)";
      }

      if (im.ext.FP16)
        ext += ext ? " ~FP16" : "~FP16";

      if (ext)
        ext = " {" + ext + "}";

      im.comment = `Instruction '${im.name || '<none>'}'${enc}${ext}.`;
    }
  }

  generateEmitCode() {
    for (let inst of this.isa.instructions) {
      if (inst.encoding !== "A32")
        continue;

      let maxRel = 0;
      for (let rel = 0; rel <= maxRel; rel++) {
        for (let maxOpIndex = inst.minimumOperandCount; maxOpIndex <= inst.operandCount; maxOpIndex++) {
          const operands = inst.operands.slice(0, maxOpIndex);
          const dt1Array = inst.dt.length ? inst.dt : ["any"];
          const dt2Array = inst.dt2.length ? inst.dt2 : ["any"];
          const dataTypes = [];

          for (let dt2 of dt2Array) {
            for (let dt1 of dt1Array) {
              if (dt2 !== "any")
                dataTypes.push({modifier: ` | kDT1_${dt1.toUpperCase()} | kDT2_${dt2.toUpperCase()}`, suffix: `_${dt1}_${dt2}`})
              else if (dt1 !== "any")
                dataTypes.push({modifier: ` | kDT1_${dt1.toUpperCase()}`, suffix: `_${dt1}`})
              else
                dataTypes.push({modifier: ``, suffix: ``})
            }
          }

          // Signatures of operators that are used in a C++ emit function of an Assembler instruction.
          const emitOps = operands.map(function(operand) {
            switch (operand.type) {
              case "reg":
                if (operand.regType === "r")
                  return "Gp";
                else if (operand.regType === "s")
                  return "Vec";
                else if (operand.regType === "d")
                  return "Vec";
                else if (operand.regType === "v")
                  return "Vec";
                else
                  return "Unknown";

              case "reg-list":
                if (operand.regType === "r")
                  return "GpList";
                else if (operand.regType === "s")
                  return "VecSList";
                else if (operand.regType === "d")
                  return "VecDList";
                else
                  FATAL(`Instruction '${inst.name}: Unsupported register-list type '${operand.regType}'`);

              case "mem":
                return "Mem";

              case "imm":
                if (operand.isRelative()) {
                  maxRel = 1;
                  return rel === 0 ? "Label" : "Imm";
                }
                else {
                  return "Imm";
                }
              default:
                return "Unknown";
            }
          });

          for (let dt of dataTypes) {
            let emitSignature = emitOps.join(", ");

            const emitSignatureC = `ASMJIT_INST_${operands.length}c(${cxx.Utils.normalizeSymbolName(inst.name)}${dt.suffix}, Inst::${instNameToEnum(inst.name)}${dt.modifier}${operands.length ? ", " + emitSignature : ""})`;
            const emitSignatureX = `ASMJIT_INST_${operands.length}x(${cxx.Utils.normalizeSymbolName(inst.name)}${dt.suffix}, Inst::${instNameToEnum(inst.name)}${dt.modifier}${operands.length ? ", " + emitSignature : ""})`;

            if (inst.fields["cond"]) {
              if (this.emitterMap[emitSignatureX])
                this.emitterTable.splice(this.emitterTable.indexOf(emitSignatureX), 1);
              emitSignature = emitSignatureC;
            }
            else {
              emitSignature = emitSignatureX;
              if (this.emitterMap[emitSignatureC])
                emitSignature = emitSignatureC;
            }

            if (!this.emitterMap[emitSignature]) {
              this.emitterTable.push(emitSignature);
              this.emitterMap[emitSignature] = true;
            }
          }
        }
      }
    }
  }

  generateAssembler() {
    for (let inst of this.isa.instructions) {
      if (inst.encoding !== "A32")
        continue;

      // console.log(`PROCESSING ${inst.name}`)

      // We have to generate code for each instruction signature. If an operand is optional, we would generate a
      // signature check for the instruction without that operand, and with that operand provided. So in general
      // we end up with two distinct instruction signatures.
      for (let maxOpIndex = inst.minimumOperandCount; maxOpIndex <= inst.operandCount; maxOpIndex++) {
        const allOperands = inst.operands;
        const operands = allOperands.slice(0, maxOpIndex);

        let memTypeMax = 0;

        // Multiple Operand Signatures
        // ---------------------------
        //
        // Some instructions require handling of multiple signatures for the same instruction. For example branch
        // instructions can use both Label and Imm as an operand (where Label is relative and Imm absolute target).
        //
        // Instead of adding ugly logic to the code, we just process such instructions multiple times - it's
        // perfectly okay as each time we process it we just insert more code to the handlers.

        for (let memType = 0; memType <= memTypeMax; memType++) {
          // Instruction handler.
          const ih = this.getInstructionHandler(inst.name);

          // Instruction fields.
          let fields = inst.fields;
          let fieldsDone = dict();

          // Memory operand info.
          let mem = null;
          let memIdx = -1;
          let memRelNoOffset = false;

          // Label/Imm target info.
          let rel = null;
          let relIdx = -1;

          let regList = null;
          let regListIdx = -1;

          // Operand Signatures Analysis
          // ---------------------------
          //
          // Does the initial analysis and populate opSignatures array - this contains the signature this handler
          // matches.

          const opSignatures = [];

          for (let i = 0; i < operands.length; i++) {
            const operand = operands[i];
            switch (operand.type) {
              case "reg":
                if (operand.reg.substr(0, 1).toUpperCase() === "V")
                  opSignatures.push("kOpRegQ");
                else
                  opSignatures.push("kOpReg" + (operand.reg.substr(0, 1)).toUpperCase());
                break;

              case "reg-list":
                if (regListIdx !== -1)
                  FATAL(`Instruction ${inst.name}: More than 1 register-list operand is not supported`);

                regList = operand;
                regListIdx = i;

                if (regList.regType === "r")
                  opSignatures.push("kOpRegListR");
                else if (regList.regType === "s")
                  opSignatures.push("kOpRegListS");
                else if (regList.regType === "d")
                  opSignatures.push("kOpRegListD");
                else
                  FATAL(`Instruction ${inst.name}: Unhandled register type '${regList.regType}' in register-list`);
                break;

              case "mem":
                if (memIdx !== -1)
                  FATAL(`Instruction ${inst.name}: More than 1 memory operand is not supported`);

                mem = operand;
                memIdx = i;

                // If this is a memory operand that supports relative addressing (literal), then there are two cases:
                //   - it only supports only PC register => `memType = 1` (we will emit a single handler).
                //   - it supports both base register and PC register >= `memTypeMax = 1` (we will emit two handlers).
                if (memType === 0 && memTypeMax === 0) {
                  memTypeMax = checkRegisterExpression(mem.base.exp, mem.base.field, 15) ? 1 : 0;

                  if (memTypeMax > 0 && !checkRegisterExpression(mem.base.exp, mem.base.field, 0))
                    memType = 1;
                }

                memRelNoOffset = memType > 0 && !mem.offset;
                opSignatures.push(memType === 0 || memRelNoOffset ? "kOpMemB" : "kOpMemAny");
                break;

              case "imm":
                if (operand.isRelative()) {
                  if (relIdx !== -1)
                    FATAL(`Instruction ${inst.name}: More more than 1 branch target makes no sense`);

                  rel = operand;
                  relIdx = i;

                  opSignatures.push("kOpLabel");
                }
                else {
                  opSignatures.push("kOpImmI");
                }
                break;

              default:
                opSignatures.push("kOpUnknown");
                break;
            }
          }

          // Signature is the first check - every instruction has one or more signatures to be handled.
          let opSignatureCheck = opSignatures.length ? `sgn.test<${opSignatures.join(", ")}>()` : `sgn.empty()`;

          // Support float immediate value that can be used with VMOV instruction.
          if (inst.name === "vmov" && operands[operands.length - 1].imm === "immVFP")
            opSignatureCheck = opSignatureCheck + " || " + opSignatureCheck.replace("ImmI", "ImmF");

          // Support both Label and Imm that can be used with branch instructions.
          if (rel)
            opSignatureCheck = opSignatureCheck + " || " + opSignatureCheck.replace("kOpLabel", "kOpImmI");

          // Instruction Handler Checks
          // --------------------------

          const checks = [opSignatureCheck];

          // Emit handler key and code.
          //
          // There are two handlers generated for each instruction:
          //
          //   - Instruction handler - this handler is emitted into the assembler's big instruction switch block,
          //     and it's the code that is handled separately for each instruction group.
          //
          //   - Emit handler - this is additional code that has its own label in Assembler's emit() implementation.
          //     The idea is that emit handlers are shared by more instruction handlers, so we generate a bit smaller
          //     code when we split the instruction handler into two.
          let ehKey = "";
          let ehBlock = new cxx.Block();

          // Emit handler next - this is the label where we jump from emit handler. In our case, this is a label
          // that has a C++ implementation, which is not generated. It's responsible for adding the opcode into
          // the destination buffer and for encoding relative displacements.
          const ehNext = rel ? "Emit_Op32_Rel" : memType !== 0 && !memRelNoOffset ? "Emit_Op32_MemPC" : "Emit_Op32";

          // Emit memory check early as many instructions that access memory share the same check even if different
          // signatures exist - these signatures usually influence how registers are used / encoded, not memory.
          if (mem) {
            const mc = getMemModeCheck(mem);

            if (memType == 0) {
              checks.push(compileMemExpression(mem.base.field, "mem->baseId()", mem.base.exp));
              if (mem.index) {
                checks.push("mem->indexType() == RegType::kGp32", "!mem->offsetLo32()");
                checks.push(compileMemExpression(mem.index.field, "mem->indexId()", mem.index.exp));
              }
              else {
                checks.push("!mem->hasIndex()");
                if (mem.offset) {
                  const field = fields[mem.offset.field];
                  if (mem.offset.exp) {
                    checks.push(mem.offset.exp.toString(new OffEncodeContext()));
                  }
                  else if (mem.offset.u === "1") {
                    checks.push(`checkUOffset(mem->offsetLo32(), ${field.bits}, ${lsbFromMul(mem.offset.mul)})`);
                  }
                  else {
                    checks.push(`checkSOffset(mem->offsetLo32(), ${field.bits}, ${lsbFromMul(mem.offset.mul)})`);
                  }
                }
                else {
                  checks.push("!mem->offsetLo32()");
                }
              }
            }
            else if (memRelNoOffset) {
              checks.push("mem->baseId() == 15u && !mem->offsetLo32()");
            }
            else {
              checks.push("isMemPCRel(*mem)");
            }

            if (mc)
              checks.push(mc);
          }

          // DataType Check
          // --------------
          //
          // DataType is only used by SIMD instructions, so we either use it or not. If there is a DataType it means that
          // there could be more handlers depending on a DataType combination. This basically adds a condition into the
          // sgnBlock.
          let dtCheck = "";

          if (inst.dt.length || inst.dt2.length) {
            function mapDts(dts) {
              const out = [];
              for (let dt of dts) {
                if (dt === "8")
                  out.push("DT::kS8", "DT::kU8");
                else if (dt === "16")
                  out.push("DT::kS16", "DT::kU16", "DT::kF16", "DT::kBF16");
                else if (dt === "32")
                  out.push("DT::kS32", "DT::kU32", "DT::kF32");
                else if (dt === "64")
                  out.push("DT::kS64", "DT::kU64", "DT::kF64");
                else
                  out.push("DT::k" + dt.toUpperCase());
              }
              return out;
            }

            const dts = mapDts(inst.dt);
            const dt2s = mapDts(inst.dt2);

            if (dts.length && !(dts.length === 1 && dts[0] === "DT::kANY")) {
              if (dts.length === 1 && dt2s.length === 1)
                dtCheck = `isDtAndDt2Single(dtBits, ${dts[0]}, ${dt2s[0]})`;
              else if (dts.length === 1)
                dtCheck = `isDtSingle(dtBits, ${dts[0]})`;
              else if (dt2s.length === 0)
                dtCheck = `isDtMultiple(dtBits, makeDtBits(${dts.join(", ")}))`;
              else if (dt2s.length !== 0)
                FATAL(`Instruction '${inst.name}': Cannot generate data-type check for dt(${dts}) and dt2(${dt2s})`);
            }
          }

          // Operand Checks
          // --------------
          //
          // Operands check follows data-type check - check for register type, vector element type, etc...

          let opChecks = [];
          let opPureVec = [];
          let opElementVec = [];

          let opConsecutiveRegs = [];
          let opConsecutiveRegInc = 0;

          for (let i = 0; i < operands.length; i++) {
            const operand = operands[i];

            if (operand.type === "reg") {
              if (/^(s|d|v)$/.test(operand.regType)) {
                if (!operand.element)
                  opPureVec.push(`o${i}.as<Vec>()`);
                else
                  opElementVec.push(`o${i}.as<Vec>()`);
              }

              if (operand.consecutive) {
                if (opConsecutiveRegs.length === 0) {
                  opConsecutiveRegs.push(`o${i - 1}.as<Reg>()`);
                  opConsecutiveRegInc = operand.consecutive;
                }
                else if (opConsecutiveRegInc !== operand.consecutive) {
                  FATAL(`Instruction '${inst.name}': Consecutive registers: Increment ${operand.consecutive} doesn't match ${opConsecutiveRegInc}`);
                }

                opConsecutiveRegs.push(`o${i}.as<Reg>()`);
              }
            }
          }

          if (opPureVec.length)
            opChecks.push(`isPureVec(${opPureVec.join(", ")})`);

          if (opElementVec.length)
            opChecks.push(`isElementVec(${opElementVec.join(", ")})`);

          if (opChecks.length)
            checks.push(opChecks.join(" && "));

          if (opConsecutiveRegs.length)
            checks.push(`isConsecutive(${opConsecutiveRegInc}, ${opConsecutiveRegs.join(", ")})`);

          if (dtCheck)
            checks.push(dtCheck);

          // Block where we generate the code to handle this instruction, its data-type combination(s), and other constraints.
          let ihBlock = ih.getBlock(checks, memIdx, relIdx, regListIdx);

          let sopImmIndex = -1;
          let zeroImmIndex = -1;

          let imm = inst.imm;
          let immCtx = new ImmEncodeContext();
          let immConds = [];
          const imms = dict();

          if (opElementVec.length) {
            imms["i"] = "i";

            ihBlock.addVarDecl("uint32_t", "i", `${opElementVec[0]}.as<Vec>().elementIndex()`);
            if ("i" in fields) {
              const cond = new cxx.If(`i <= ${toHex((1 << fields.i.bits) - 1)}`);
              ihBlock.appendNode(cond);
              ihBlock = cond.body;
            }
            else {
              const cond = new cxx.If(`i == 0u`);
              ihBlock.appendNode(cond);
              ihBlock = cond.body;
            }
          }

          for (let i = 0; i < operands.length; i++) {
            const operand = operands[i];
            if (operand.type === "reg") {
              if (operand.regType === "r") {
                if (operand.shiftOp) {
                  const cond = new cxx.If(`shiftOp <= 3u`);
                  ihBlock.addVarDecl("uint32_t", "shiftOp", `o${i}.as<Gp>().predicate()`);
                  ihBlock.appendNode(cond);
                  ihBlock = cond.body;
                  imms["sop"] = "shiftOp";
                }
              }
            }
          }

          // Label & Imm Displacement
          // ------------------------

          if (rel) {
            const field = fields[rel.name];

            if (!field)
              FATAL(`Instruction '${inst.name}': Relative displacement ${rel.name} doesn't have a field in opcode`);

            fieldsDone[rel.name] = true;

            let offsetType = "";
            let immBitShift = field.values[0].index;
            let immBitCount = getOffsetSize(field);
            let immDiscardLSB = getOffsetBitsToDiscard(rel.restrict);

            if ((inst.name === "b" || inst.name === "bl" || inst.name === "blx") && inst.encoding === "T32") {
              fieldsDone.ja = true;
              fieldsDone.jb = true;
              offsetType = inst.name === "blx" ? "kThumb32_BLX" : fields.cond ? "kThumb32_BCond" : "kThumb32_B";
            }
            else if (inst.name === "adr") {
              fieldsDone.N = true;
              fieldsDone.P = true;
              offsetType = inst.encoding === "A32" ? "kAArch32_ADR" : "kThumb32_ADR";
            }
            else if (field.values.length === 1) {
              offsetType = "kSignedOffset";
            }
            else {
              offsetType = `kAArch32_${getOffsetType(field)}`;
            }

            ihBlock.addLine(`offsetFormat.resetToImmValue(OffsetType::${offsetType}, 4, ${immBitShift}, ${immBitCount}, ${immDiscardLSB});`);
          }

          // Immediate Values
          // ----------------

          // First, check whether we can encode the given immediate.
          for (let i = 0; i < operands.length; i++) {
            const operand = operands[i];

            // Skip branch targets, handled separately.
            if (operand === rel)
              continue;

            if (operand.type === "imm") {
              if (operand.imm === "zero") {
                zeroImmIndex = i;
                continue;
              }

              immCtx.imms[operand.imm] = `o${i}.as<Imm>()`;
              immCtx.hasImms = true;

              if (operand.shiftOp) {
                sopImmIndex = i;
                continue;
              }

              if (!imm) {
                const field = fields[operand.imm];
                if (field) {
                  immConds.push(`o${i}.as<Imm>().valueAs<uint64_t>() <= ${toHex((1 << field.bits) - 1)}u`);
                  imms[operand.imm] = `o${i}.as<Imm>().valueAs<uint32_t>()`;
                }
              }
            }
            else if (operand.type === "mem" && operand.shiftOp) {
              sopImmIndex = i;
            }
          }

          for (let i = operands.length; i < allOperands.length; i++) {
            const operand = allOperands[i];
            if (operand.type === "imm") {
              imms[operand.imm] = "0u";
              immCtx.imms[operand.imm] = "0u";
              immCtx.hasImms = true;
              fieldsDone["sop"] = true;

              if (operand.shiftOp === "sop") {
                imms["sop"] = "0u";
              }
            }
          }

          if (!imm) {
            if (immCtx.imms["immA"])
              imm = exp.parse(standardImmEncode.immA);
          }

          if (zeroImmIndex >= 0) {
            const cond = new cxx.If(`o${zeroImmIndex}.as<Imm>().value() == 0u`);
            ihBlock.appendNode(cond);
            ihBlock = cond.body;
          }

          if (sopImmIndex >= 0) {
            const operand = operands[sopImmIndex];

            if (operand.type === "imm") {
              if (operand.shiftOp === "sop") {
                imms["sop"] = "shiftOp";
                ihBlock.addVarDecl("uint32_t", "shiftOp", `o${sopImmIndex}.as<Imm>().predicate()`);
                immConds.push(`shiftOp <= 3`);
              }
              else if (operand.shiftOp === "lsl_or_asr") {
                imms["sop"] = "shiftOp";
                ihBlock.addVarDecl("uint32_t", "shiftOp", `o${sopImmIndex}.as<Imm>().predicate()`);
                immConds.push(`(shiftOp == uint32_t(ShiftOp::kLSL) || shiftOp == uint32_t(ShiftOp::kASR))`);
              }
              else {
                immConds.push(`o${sopImmIndex}.as<Imm>().predicate() == uint32_t(ShiftOp::k${operand.shiftOp.toUpperCase()})`);
              }

              if (!imm) {
                ihBlock.addVarDecl("uint64_t", "shiftImm", `o${sopImmIndex}.as<Imm>().valueAs<uint64_t>()`);
                immConds.push(`shiftImm <= 31u`);
                imms[operand.imm] = "uint32_t(shiftImm)";
              }
            }
            else {
              if (operand.offset) {
                if (operand.shiftOp === "sop") {
                  imms["sop"] = "shiftOp";
                  ihBlock.addVarDecl("uint32_t", "shiftOp", `mem->predicate()`);
                  immConds.push(`shiftOp <= 3`);
                }
                else if (operand.shiftOp === "lsl_or_asr") {
                  imms["sop"] = "shiftOp";
                  ihBlock.addVarDecl("uint32_t", "shiftOp", `mem->predicate()`);
                  immConds.push(`(shiftOp == uint32_t(ShiftOp::kLSL) || shiftOp == uint32_t(ShiftOp::kASR))`);
                }
                else {
                  immConds.push(`mem->predicate() == uint32_t(ShiftOp::k${operand.shiftOp.toUpperCase()})`);
                }

                if (!imm) {
                  ihBlock.addVarDecl("uint32_t", "shiftImm", `mem->shift()`);
                  immConds.push(`shiftImm <= ${(1 << fields[operand.shiftImm].bits) - 1}u`);
                  imms[operand.shiftImm] = "shiftImm";
                }
              }
            }
          }

          if (immConds.length) {
            const cond = new cxx.If(immConds.join(" && "));
            ihBlock.appendNode(cond);
            ihBlock = cond.body;
          }

          if (imm) {
            for (let arg of imm.args)
              if (arg.type === "var")
                ih.useCommonVar(arg.name);

            immCtx.name = ih.nextCondEncoderName();
            immCtx.hasImms = true;

            const args = imm.args.map(function(arg) {
              if (arg.type === "var" && arg.name.endsWith("List"))
                return `regList`;
              else
                return arg.toString(immCtx);
            });
            const cond = new cxx.If(`${immCtx.name}.init(${args.join(", ")})`);

            ihBlock.addVarDecl(imm.name + "Encode", immCtx.name);
            ihBlock.appendNode(cond);
            ihBlock = cond.body;
          }

          ihBlock.addLine(`opcode = ${toHex(inst.opcodeValue, 8)}u;`);

          for (let i = 0; i < operands.length; i++) {
            const operand = operands[i];
            const opVar = `o${i}`;

            if (operand.type === "reg") {
              const fieldName = regToField(operand.reg);
              const field = fields[fieldName];

              if (field) {
                const op = regToOpcode(field, opVar, operand.regType);
                ehKey += "_" + op.key;
                ehBlock.addVarDecl(op.vars.type, op.vars.name, op.vars.init);
                ehBlock.addLine(op.code);
                fieldsDone[fieldName] = true;
              }
            }
            else if (operand.type === "reg-list" || operand.type === "mem" || operand.type === "imm") {
              // nothing here...
            }
            else {
              console.log(`Unhandled operand type: ${operand.type}`);
            }
          }

          if (mem) {
            if (memType === 0 || memRelNoOffset) {
              // Memory operand - base or PC without offset.
              const P = fields.P;
              const W = fields.W;

              if (mem.base) {
                const field = fields[mem.base.field];
                if (field) {
                  fieldsDone[mem.base.field] = true;
                  ehKey += `_MemBaseAt${field.index}`;
                  ehBlock.addLine(`opcode |= mem->baseId() << ${field.index};`);
                }
              }

              if (mem.index) {
                const field = fields[mem.index.field];
                if (field) {
                  fieldsDone[mem.index.field] = true;

                  if (mem.index.u === "U") {
                    ehKey += `_MemSIndexAt${field.index}`;
                    ehBlock.addLine(`opcode |= mem->indexId() << ${field.index};`);
                    // TODO: Support both positive and negative indexing.
                    ehBlock.addLine(`opcode |= 1u << ${fields.U.index};`);
                  }
                  else {
                    ehKey += `_MemUIndexAt${field.index}`;
                    ehBlock.addLine(`opcode |= mem->indexId() << ${field.index};`);
                  }

                  if (mem.shiftOp) {
                    const SOP = fields.sop;
                    const N = fields.n;

                    if (!SOP && mem.shiftOp === "sop")
                      FATAL(`Instruction '${inst.name}': Memory index operation is missing SOP field in the opcode`);

                    if (!N)
                      FATAL(`Instruction '${inst.name}': Memory index operation is missing N field in the opcode`);

                    if (SOP)
                      ehKey += `_${mem.shiftOp.toUpperCase()}At${SOP.index}_N${N.bits}At${N.index}`;
                    else
                      ehKey += `_N${N.bits}At${N.index}`;

                    ehBlock.addLine("uint32_t n = mem->shift();");
                    ehBlock.addLine("uint32_t sop = uint32_t(mem->shiftOp());");
                    ehBlock.addEmptyLine();

                    ehBlock.addLine("if (n > 31u)");
                    ehBlock.addLine("  goto InvalidAddressScale;");
                    ehBlock.addEmptyLine();

                    ehBlock.addLine("if (sop > 3u)");
                    ehBlock.addLine("  goto InvalidAddress;");
                    ehBlock.addEmptyLine();

                    if (SOP)
                      ehBlock.addLine(`opcode |= sop << ${SOP.index};`);
                    ehBlock.addLine(`opcode |= n << ${N.index};`);

                    fieldsDone.n = true;
                    fieldsDone.sop = true;
                  }
                }
              }

              if (mem.offset) {
                const field = fields[mem.offset.field];
                if (field) {
                  fieldsDone[mem.offset.field] = true;
                  if (mem.offset.u === "U") {
                    ehBlock.addLine(`SOffsetEncode off(*mem);`);

                    for (let value of field.values)
                      ehKey += `_SOffAt${value.index}Of${value.size}`;

                    if (mem.offset.mul > 1)
                      ehKey += `Mul${mem.offset.mul}`;

                    ehBlock.addLine(`opcode |= ${encodeValueOps("off.imm()", field.values, lsbFromMul(mem.offset.mul))};`);
                    ehBlock.addLine(`opcode |= off.u() << ${fields.U.index}u;`);
                  }
                  else {
                    ehBlock.addLine(`uint32_t off = uint32_t(mem->offsetLo32());`);

                    for (let value of field.values)
                      ehKey += `_UOffAt${value.index}Of${value.size}`;

                    ehBlock.addLine(`opcode |= ${encodeValueOps("off", field.values, lsbFromMul(mem.offset.mul))};`);
                  }
                }
              }

              if (P && W) {
                ehKey += `_P${P.index}W${W.index}`;
                ehBlock.addLine(`opcode |= uint32_t(mem->isPreIndex()) << ${W.index};`);
                ehBlock.addLine(`opcode |= uint32_t(!mem->isPostIndex()) << ${P.index};`);
                fieldsDone.P = true;
                fieldsDone.W = true;
              }
              else if (W) {
                ehKey += `W${W.index}`;
                ehBlock.addLine(`opcode |= uint32_t(mem->isPreIndex()) << ${W.index};`);
                fieldsDone.W = true;
              }
            }
            else {
              ehKey += "_MemPCRel";

              // Memory operand - Either PC base, or label/absolute.
              //
              // This is a different strategy as we want to use a unified handler to handle this case (not generated).
              // So, what we have to do is to fill an `OffsetFormat` so the assembler would now how to encode the
              // instruction, and more importantly, how to patch it later, as patching could be done afterwards by
              // CodeHolder.
              if (mem.base) {
                const field = fields[mem.base.field];
                if (field) {
                  ihBlock.addLine(`opcode |= 0xFu << ${field.index};`);
                  fieldsDone[mem.base.field] = true;
                }
              }

              if (mem.offset) {
                const field = fields[mem.offset.field];
                const U = fields.U;

                if (field) {
                  fieldsDone[mem.offset.field] = true;
                  fieldsDone.U = true;

                  let offsetType = "";
                  let immBitShift = field.values[0].index;
                  let immBitCount = getOffsetSize(field);
                  let immDiscardLSB = 0;

                  const uVal = U ? "_U" + U.index : "";

                  if (field.values.length === 1) {
                    offsetType = `kAArch32${uVal}_SignedOffset`;
                  }
                  else {
                    offsetType = `kAArch32${uVal}_${getOffsetType(field)}`;
                  }

                  ihBlock.addLine(`offsetFormat.resetToImmValue(OffsetType::${offsetType}, 4, ${immBitShift}, ${immBitCount}, ${immDiscardLSB});`);
                }
              }
            }
          }

          if (rel)
            ehKey += "_Rel";

          if (fields.cond) {
            ehKey += "_Cond";
            ehBlock.addLine(`opcode |= encodeCond(cc) << ${fields.cond.index}u;`);
            fieldsDone["cond"] = true;
          }
          else {
            ehKey += "_NoCond";
          }

          ih.useCommonVars(fields);

          for (let fieldName in fields) {
            const field = fields[fieldName];
            if (fieldName in fieldsDone)
              continue;

            if (fieldName === "cond")
              continue;

            if (fieldName in imms) {
              if (imms[fieldName] !== "0u") {
                if (field.values.length > 1) {
                  for (let immValue of field.values) {
                    let mask = ((1 << immValue.size) - 1) << immValue.from;
                    let shift = -immValue.from + immValue.index;
                    ihBlock.addLine(`opcode |= (${imms[fieldName]} & ${toHex(mask)}u) ${shift >= 0 ? "<<" : ">>"} ${Math.abs(shift)}u;`);
                  }
                }
                else {
                  ihBlock.addLine(`opcode |= ${imms[fieldName]} << ${field.index}u;`);
                }
              }
              continue;
            }

            if (fieldName === "sz") {
              ihBlock.addLine(`opcode |= sz << ${field.index}u;`);
              continue;
            }

            if (fieldName === "szM1") {
              ihBlock.addLine(`opcode |= (sz - 1) << ${field.index}u;`);
              continue;
            }

            if (fieldName === "F" && inst.dt.length !== 0) {
              ihBlock.addLine(`opcode |= fBitFromDt(dtBits) << ${field.index}u;`);
              continue;
            }

            if (fieldName === "P" && inst.dt.length !== 0) {
              ihBlock.addLine(`opcode |= pBitFromDt(dtBits) << ${field.index}u;`);
              continue;
            }

            if (fieldName === "U" && inst.dt.length !== 0) {
              ihBlock.addLine(`opcode |= uBitFromDt(dtBits) << ${field.index}u;`);
              continue;
            }

            if (immCtx.hasImms) {
              if (field.values.length > 1) {
                for (let immValue of field.values) {
                  let mask = ((1 << immValue.size) - 1) << immValue.from;
                  let shift = -immValue.from + immValue.index;
                  ihBlock.addLine(`opcode |= (${immCtx.name}.${fieldName}() & ${toHex(mask)}u) ${shift >= 0 ? "<<" : ">>"} ${Math.abs(shift)}u;`);
                }
              }
              else {
                ihBlock.addLine(`opcode |= ${immCtx.name}.${fieldName}() << ${field.index}u;`);
              }
              continue;
            }

            console.log(`Instruction '${inst.name}': Missing ${fieldName}`);
          }

          ehKey = "Emit" + ehKey;
          ihBlock.addLine(`goto ${ehKey};`);
          ehBlock.addLine(`goto ${ehNext};`);

          this.getEmitHandler(ehKey, String(ehBlock));
        }
      }
    }
  }

  generateFunctions(output) {
    let code = new cxx.Block();
    code.addLine("idr = instDispatchTable[instId];");
    let switch_ = new cxx.Switch("idr.group");

    const map = dict();
    const dispatchTable = [];

    let groupId = 0;
    const groups = [];

    for (let name of this.instructionIdTable) {
      const ih = this.instructionIdMap.get(name);

      if (ih) {
        let caseCode = String(new cxx.Block([ih.block, new cxx.Line(""), new cxx.Line("break;")])).replace(/\n$/, "");
        let values = [];

        function r(match, p1) {
          values.push(p1);
          return `opcode = opcodeTablePtr[${values.length - 1}]`;
        }

        let template = caseCode.replace(/opcode = (0x[A-F0-9]+u)/g, r);
        const block = new cxx.Block([new cxx.Line(template)]);
        const dispatchRecord = {name: name, block: block, values: values};

        if (template in map) {
          dispatchRecord.group = map[template][0].group;
          dispatchRecord.index = map[template].length;
          map[template].push(dispatchRecord);
        }
        else {
          dispatchRecord.group = groupId++;
          dispatchRecord.index = 0;
          map[template] = [dispatchRecord];
          groups.push(map[template]);
        }
        dispatchTable.push(dispatchRecord);
      }
    }

    let table =
      "static const InstDispatchRecord instDispatchTable[] = {\n" +
        StringUtils.formatRecords(dispatchTable, kIndent, (record) => { return `{${record.group}, ${record.index}}`; }, 10) +
      "\n};\n";

    for (let records of groups) {
      const groupId = records[0].group;
      const block = new cxx.Block();

      if (records.length === 1) {
        const record = records[0];
        function x(match, p1) { return `opcode = ${record.values[parseInt(p1)]}`; }

        let code = `// Instruction '${record.name}'.\n` +
                   record.block.nodes[0].code.replace(/opcode = opcodeTablePtr\[(\d+)\]/g, x);
        block.addLine(code);
      }
      else if (records[0].values.length > 0) {
        let code = "";
        for (let record of records) {
          code += record.values.join(", ");
          code += record !== records[records.length - 1] ? ", " : "  ";
          code += `// Instruction '${record.name}'.`;
          code += "\n";
        }

        code = "static const uint32_t opcodeTable[] = {\n" + indent(code, 2) + "};\n\n" +
               "const uint32_t* opcodeTablePtr = opcodeTable + uint32_t(idr.index) * " + records[0].values.length + "u;\n\n" +
               records[0].block.nodes[0].code;
        block.addLine(code);
      }
      else {
        // nothing, no code generated for this case.
        block.addLine("break;");
      }

      switch_.addCase(`${groupId}`, block);
    }

    switch_.addCase("default", new cxx.Block([new cxx.Line("break;")]));

    code.appendNode(switch_);
    code.addEmptyLine();
    code.addLine("goto InvalidInstruction;");
    code.addEmptyLine();
    code.addLine("// Emit - Bits");
    code.addLine("// -----------");

    for (let k in this.emitHandlers) {
      code.addEmptyLine();
      code.addLine(k + ": {");
      code.addLine(indent(this.emitHandlers[k].code.trim(), 2));
      code.addLine("}");
    }

    output.add("dispatchTable", table, dispatchTable.length * 2);
    output.add("assemblerImpl", String(code), 0);
  }

  generateNameData(output) {
    const a = [];
    let ids = "";

    let nameMaxLength = 0;
    for (let im of this.instructionMetadata) {
      nameMaxLength = Math.max(nameMaxLength, im.name.length);
    }

    for (let im of this.instructionMetadata) {
      const id = !im.name ? "kIdNone" : instNameToEnum(im.name);
      const prefix = !im.name ? id + " = 0," : id + ",";
      if (!im.comment) {
        ids += prefix;
      }
      else {
        ids += prefix.padEnd(nameMaxLength + 5, ' ') + "//!< " + im.comment;
      }
      ids += "\n";
      a.push({displayName: im.name, enum: id.substring(3)});
    }

    ids += "_kIdCount\n";

    output.add("InstId", StringUtils.disclaimer(ids));
    core.generateNameData(output, a);
  }

  consistentWrites(instructions) {
    if (instructions.length === 0)
      return [RW.R, RW.R, RW.R, RW.R, RW.R, RW.R];

    // These are consistent - the reason that the algorithm would otherwise consider them
    // inconsistent is THUMB encoding that only encodes two registers, but the instruction
    // is still defined as 3 operand instruction.
    const name = instructions[0].name;
    if (/^(adc|adcs|add|adds|and|ands|asr|asrs|bic|bics|eor|eors|lsl|lsls|lsr|lsrs|mul|muls|orr|orrs|ror|rors|sbc|sbcs|sub|subs|)$/.test(name))
      return [RW.W, RW.R, RW.R, RW.R, RW.R, RW.R];

    // If the instructions has multiple forms, but each form has write operand(s) at
    // the same index, then just generate a single record. This is the most common.
    let readCount = Array(6).fill(0);
    let writeCount = Array(6).fill(0);

    for (let inst of instructions) {
      for (let i = 0; i < inst.operands.length; i++) {
        const op = inst.operands[i];
        if (op.isReg()) {
          if (op.read) {
            readCount[i]++;
          }
          if (op.write) {
            writeCount[i]++;
          }
        }
      }
    }

    for (let i = 0; i < writeCount.length; i++) {
      if (writeCount[i] != 0) {
        if (writeCount[i] !== instructions.length)
          return null;

        if (readCount[i] !== 0 && readCount[i] !== writeCount[i])
          return null;
      }
    }

    const rw = [];
    for (let i = 0; i < writeCount.length; i++)
      rw.push((readCount[i] && writeCount[i]) ? RW.X : writeCount[i] ? RW.W : RW.R);
    return rw;
  }

  getRWInfo(instructions) {
    const rw = dict();

    for (let inst of instructions) {
      Object.assign(rw, inst.io);
    }

    return rw;
  }

  generateRWData() {
    const rwInfoTable = new IndexedArray();
    const rwFlagsTable = new IndexedArray();
    const index = [];

    for (let im of this.instructionMetadata) {
      const instructions = this.isa.query(im.name);
      const consistentRW = this.consistentWrites(instructions);

      const rwFlagsInfo = this.getRWInfo(instructions);
      let rFlags = {};
      let wFlags = {};

      for (let flag of ArrayUtils.sorted(rwFlagsInfo)) {
        if (rwFlagsInfo[flag] === "R" || rwFlagsInfo[flag] === "X") rFlags[flag] = true;
        if (rwFlagsInfo[flag] === "W" || rwFlagsInfo[flag] === "X") wFlags[flag] = true;
      }

      const rwFlagsRecord = StringUtils.formatCppStruct(
        StringUtils.formatCppFlags(rFlags, function(flag) { return `CpuRWFlags::kARM_${flag}`; }, "CpuRWFlags::kNone"),
        StringUtils.formatCppFlags(wFlags, function(flag) { return `CpuRWFlags::kARM_${flag}`; }, "CpuRWFlags::kNone")
      );

      if (consistentRW) {
        let rwInfoRecord = "";

        rwInfoRecord += "{";

        rwInfoRecord += StringUtils.formatCppStruct(...consistentRW.map(function(access) {
          if ((access & RW.X) === RW.X)
            return "X";
          else if (access & RW.W)
            return "W";
          else
            return "R";
        }));

        rwInfoRecord += ", " + rwFlagsTable.addIndexed(rwFlagsRecord);

        rwInfoRecord += "}";
        index.push(String(rwInfoTable.addIndexed(rwInfoRecord)));
      }
      else {
        console.log(`${im.name}: INCONSISTENT`);
        index.push("0xFFu");
      }
    }

    let s = "" +
      "static const constexpr InstRWInfoRecord instRWInfoData[] = {\n" +
      "  #define R uint8_t(OpRWFlags::kRead)\n" +
      "  #define W uint8_t(OpRWFlags::kWrite)\n" +
      "  #define X uint8_t(OpRWFlags::kRW)\n" +
      "\n" +
      `${StringUtils.format(rwInfoTable, kIndent, true)}\n` +
      "\n" +
      "  #undef R\n" +
      "  #undef W\n" +
      "  #undef X\n" +
      "};\n" +
      "\n" +
      "static const constexpr InstRWFlagsRecord instRWFlagsData[] = {\n" +
        `${StringUtils.format(rwFlagsTable, kIndent, true)}\n` +
      "};\n" +
      "\n" +
      "static const constexpr uint8_t instRWInfoIndex[] {\n" +
        `${StringUtils.formatRecords(index, kIndent)}\n` +
      "};\n";

    this.inject("a32::RWInfo", StringUtils.disclaimer(s), rwInfoTable.length * 7 + rwFlagsTable * 8 + index.length * 1);
  }

  generate() {
    const output = new Output();

    this.prepare();
    this.generateMetadata();
    this.generateEmitCode();
    this.generateAssembler();

    this.generateFunctions(output);
    this.generateNameData(output);
    this.generateRWData();

    this.inject("a32::DispatchData", StringUtils.disclaimer(output.content["dispatchTable"]), output.tableSize["dispatchTable"]);
    this.inject("a32::Assembler::Impl", StringUtils.disclaimer(output.content["assemblerImpl"]), 0);
    this.inject("a32::EmitterExplicit", StringUtils.disclaimer(this.emitterTable.join("\n") + "\n"), 0);
    this.inject("a32::InstId", output.content["InstId"], output.tableSize["InstId"]);
    this.inject("a32::NameData", output.content["NameData"], output.tableSize["NameData"]);
  }
}

function main() {
  const armdata = readJSON(path.join(__dirname, "..", "db", asmdb.aarch32.dbName));
  const armisa = new asmdb.aarch32.ISA(armdata);

  const generator = new Generator(armisa);

  generator.generate();
  generator.save();
  generator.dumpTableSizes();
}

main();
