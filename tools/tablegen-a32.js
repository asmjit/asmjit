// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

"use strict";

const commons = require("./gencommons.js");
const core = require("./tablegen.js");
const cxx = require("./gencxx.js");

const asmdb = core.asmdb;
const StringUtils = core.StringUtils;

const FATAL = commons.FATAL;

const exp = core.exp;
const isa = new asmdb.arm.ISA();

const indent = cxx.Utils.indent;
const toHex = cxx.Utils.toHex;

function dict() { return Object.create(null); }

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

  let v = "r" + op.substr(1);
  let vars = {"type": "uint32_t", "name": v, "init": `${op}.as<Reg>().id()`};

  for (let value of field.values) {
    const mask = ((1 << (value.size)) - 1) << value.from;
    const shift = value.index - value.from;

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

class InstructionHandler {
  constructor(id) {
    this.id = id;
    this.block = new cxx.Block();
    this.signatures = dict();
    this.commonVars = dict();
    this.condCnt = 0;
  }

  getBlock(checks, memOpIndex) {
    const sgnCheck = checks[0];
    let block = this.signatures[sgnCheck];

    if (!block) {
      const cond = new cxx.If(sgnCheck);
      this.block.addEmptyLine();
      this.block.appendNode(cond);

      block = cond.body;
      this.signatures[sgnCheck] = block;

      if (memOpIndex !== -1) {
        block.addLine(`mem = &o${memOpIndex}.as<Mem>();`);
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

class Generator {
  constructor(isa) {
    this.isa = isa;

    this.instructionIdMap = new Map();
    this.instructionIdTable = [""];
    this.instructionHandlers = dict();

    this.emitHandlers = dict();
    this.emitterMap = dict();
    this.emitterTable = [];
  }

  registerEmitHandler(key, handler) {
    if (!this.emitHandlers[key])
      this.emitHandlers[key] = { code: handler, useCount: 1 };
    else
      this.emitHandlers[key].useCount++;
  }

  registerInstructionHandler(name) {
    if (this.instructionIdMap.has(name))
      return this.instructionIdMap.get(name);

    const id = this.instructionIdTable.length;
    const ih = new InstructionHandler(id);

    this.instructionIdMap.set(name, ih);
    this.instructionIdTable.push(name);

    return ih;
  }

  generateEmitCode() {
    for (let inst of isa.instructions) {
      if (inst.encoding !== "A32")
        continue;

      let maxOpIndex = inst.operands.length;
      for (let i = 0; i < inst.operands.length; i++) {
        if (inst.operands[i].optional) {
          maxOpIndex = i;
          break;
        }
      }

      do {
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
            case "mem": return "Mem";
            case "imm": return "Imm";
            default: return "Unknown";
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
      } while (maxOpIndex++ <= inst.operands.length);
    }
  }

  generateAssembler() {
    for (let inst of isa.instructions) {
      if (inst.encoding !== "A32")
        continue;

      // Instruction handler.
      const ih = this.registerInstructionHandler(inst.name);

      let fields = inst.fields;
      let fieldsDone = dict();

      // Emit handler key and code.
      let ehKey = "";
      let ehBlock = new cxx.Block();

      // Signature is the first check - every instruction has one or more signatures.
      const sgnOps = inst.operands.map(function(operand) {
        switch (operand.type) {
          case "reg":
            if (operand.reg.substr(0, 1).toUpperCase() === "V")
              return "kOpRegQ";
            else
              return "kOpReg" + (operand.reg.substr(0, 1)).toUpperCase();
          case "mem": return "kOpMemB";
          case "imm": return "kOpImmI";
          default: return "kOpUnknown";
        }
      });

      let sgnCheck = !sgnOps.length ? `sgn.empty()` : `sgn.test<${sgnOps.join(", ")}>()`;

      // DataType is the second check.
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
          else if (dt2s.length !== 0)
            dtCheck = `isDtAndDt2Multiple(dtBits, makeDtBits(${dts.join(", ")}), makeDtBits(${dt2s.join(", ")}))`;
          else if (dts.length === 1)
            dtCheck = `isDtSingle(dtBits, ${dts[0]})`;
          else
            dtCheck = `isDtMultiple(dtBits, makeDtBits(${dts.join(", ")}))`;
        }
      }

      if (inst.name === "vmov" && inst.operands[inst.operands.length - 1].imm === "immVFP")
        sgnCheck = sgnCheck + " || " + sgnCheck.replace("ImmI", "ImmF");

      // Operands check follows data-type check.
      let opChecks = [];
      let opPureVec = [];
      let opElementVec = [];

      let opConsecutiveRegs = [];
      let opConsecutiveRegInc = 0;

      for (let i = 0; i < inst.operands.length; i++) {
        const operand = inst.operands[i];
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
              FATAL(`Consecutive registers: Increment ${operand.consecutive} doesn't match ${opConsecutiveRegInc}`);
            }

            opConsecutiveRegs.push(`o${i}.as<Reg>()`);
          }
        }
      }

      if (opPureVec.length)
        opChecks.push(`isPureVec(${opPureVec.join(", ")})`);

      if (opElementVec.length)
        opChecks.push(`isElementVec(${opElementVec.join(", ")})`);

      // Memory operand index.
      let memOpIndex = -1;
      for (let i = 0; i < inst.operands.length; i++) {
        const operand = inst.operands[i];
        if (operand.type === "mem") {
          if (memOpIndex !== -1)
            FATAL(`Instruction ${inst.name} has more than 1 memory operand`);
          memOpIndex = i;
        }
      }

      const memOp = memOpIndex === -1 ? null : inst.operands[memOpIndex];
      const checks = [sgnCheck];

      if (opChecks.length)
        checks.push(opChecks.join(" && "));

      if (opConsecutiveRegs.length)
        checks.push(`isConsecutive(${opConsecutiveRegInc}, ${opConsecutiveRegs.join(", ")})`);

      if (dtCheck)
        checks.push(dtCheck);

      function modeCheck(memOp) {
        const mm = memOp.memModes;
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

      if (memOp) {
        const mc = modeCheck(memOp);
        if (memOp.index) {
          checks.push("mem->hasIndex()", "!mem->offsetLo32()");
        }
        else {
          checks.push("!mem->hasIndex()");
          if (memOp.offset) {
            const field = fields[memOp.offset.field];
            if (memOp.offset.exp) {
              checks.push(memOp.offset.exp.toString(new OffEncodeContext()));
            }
            else if (memOp.offset.u === "1") {
              checks.push(`checkUOffset(mem->offsetLo32(), ${field.bits}, ${lsbFromMul(memOp.offset.mul)})`);
            }
            else {
              checks.push(`checkSOffset(mem->offsetLo32(), ${field.bits}, ${lsbFromMul(memOp.offset.mul)})`);
            }
          }
          else {
            checks.push("!mem->offsetLo32()");
          }
        }

        if (mc)
          checks.push(mc);
      }

      // Block where we generate the code to handle this instruction, its data-type combination(s), and other constaints.
      let ihBlock = ih.getBlock(checks, memOpIndex);

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

      for (let i = 0; i < inst.operands.length; i++) {
        const operand = inst.operands[i];
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

      // First, check whether we can encode the given immediate.
      for (let i = 0; i < inst.operands.length; i++) {
        const operand = inst.operands[i];
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
        const operand = inst.operands[sopImmIndex];

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

        const args = imm.args.map(function(arg) { return arg.toString(immCtx); });
        const cond = new cxx.If(`${immCtx.name}.init(${args.join(", ")})`);

        ihBlock.addVarDecl(imm.name + "Encode", immCtx.name);
        ihBlock.appendNode(cond);
        ihBlock = cond.body;
      }

      ihBlock.addLine(`opcode = ${toHex(inst.opcodeValue, 8)}u;`);

      for (let i = 0; i < inst.operands.length; i++) {
        const operand = inst.operands[i];
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
        else if (operand.type === "mem" || operand.type === "imm") {
          // nothing...
        }
        else {
          console.log(`Unhandled operand type: ${operand.type}`);
        }
      }

      if (memOp) {
        const P = fields.P;
        const W = fields.W;

        if (memOp.base) {
          const field = fields[memOp.base.field];
          if (field) {
            fieldsDone[memOp.base.field] = true;
            ehKey += `_MemBaseAt${field.index}`;
            ehBlock.addLine(`opcode |= mem->baseId() << ${field.index};`);
          }
        }

        if (memOp.index) {
          const field = fields[memOp.index.field];
          if (field) {
            fieldsDone[memOp.index.field] = true;

            if (memOp.index.u === "U") {
              ehKey += `_MemSIndexAt${field.index}`;
              ehBlock.addLine(`opcode |= mem->indexId() << ${field.index};`);
              ehBlock.addLine(`opcode |= 1u << ${fields.U.index};`);
            }
            else {
              ehKey += `_MemUIndexAt${field.index}`;
              ehBlock.addLine(`opcode |= mem->indexId() << ${field.index};`);
            }
          }
        }

        if (memOp.offset) {
          const field = fields[memOp.offset.field];
          if (field) {
            fieldsDone[memOp.offset.field] = true;
            if (memOp.offset.u === "U") {
              ehBlock.addLine(`SOffsetEncode off(*mem);`);

              for (let value of field.values)
                ehKey += `_SOffAt${value.index}Of${value.size}`;

              if (memOp.offset.mul > 1)
                ehKey += `Mul${memOp.offset.mul}`;

              ehBlock.addLine(`opcode |= ${encodeValueOps("off.imm()", field.values, lsbFromMul(memOp.offset.mul))};`);
              ehBlock.addLine(`opcode |= off.u() << ${fields.U.index}u;`);
            }
            else {
              ehBlock.addLine(`uint32_t off = uint32_t(mem->offsetLo32());`);

              for (let value of field.values)
                ehKey += `_UOffAt${value.index}Of${value.size}`;

              ehBlock.addLine(`opcode |= ${encodeValueOps("off", field.values, lsbFromMul(memOp.offset.mul))};`);
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

        if (fieldName === "H" && inst.dt.length !== 0) {
          ihBlock.addLine(`opcode |= isFp16Dt(dtBits) << ${field.index}u;`);
          continue;
        }

        if (fieldName === "isFp32" && inst.dt.length !== 0) {
          ihBlock.addLine(`opcode |= isFp32Dt(dtBits) << ${field.index}u;`);
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

      this.registerEmitHandler(ehKey, String(ehBlock));
    }
  }

  genFunc() {
    let code = new cxx.Block();
    code.addLine("idr = instDispatchTable[instId];");
    let switch_ = new cxx.Switch("idr.group");

    const map = dict();
    const dispatchTable = [{
      name: "<none>",
      group: 0,
      index: 0,
      block: new cxx.Block([new cxx.Line("break;")]),
      values: []
    }];

    let groupId = 1;
    const groups = [[dispatchTable[0]]];

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

    let table = "";
    for (let dispatchRecord of dispatchTable) {
      if (table)
        table += ",\n";
      table += `{${dispatchRecord.group}, ${dispatchRecord.index}}`;
    }
    table = "static const InstDispatchRecord instDispatchTable[] = {\n" + indent(table, 2) + "\n};\n";

    for (let records of groups) {
      const groupId = records[0].group;
      const block = new cxx.Block();

      if (records.length === 1) {
        const record = records[0];
        function x(match, p1) { return `opcode = ${record.values[parseInt(p1)]}`; }

        let code = `// Group of '${record.name}'.\n` +
                   record.block.nodes[0].code.replace(/opcode = opcodeTablePtr\[(\d+)\]/g, x);
        block.addLine(code);
      }
      else {
        let code = "";
        for (let record of records) {
          code += record.values.join(", ");
          code += record !== records[records.length - 1] ? ", " : "  ";
          code += `// Instruction '${record.name}'.`;
          code += "\n";
        }

        code = `// Group of '${records.map(function(r) { return r.name}).join("|")}'.\n` + "static const uint32_t opcodeTable[] = {\n" + indent(code, 2) + "};\n\n";
        code += "const uint32_t* opcodeTablePtr = opcodeTable + uint32_t(idr.index) * " + records[0].values.length + "u;\n\n";
        code += records[0].block.nodes[0].code;
        block.addLine(code);
      }

      switch_.addCase(`${groupId}`, block);
    }

    switch_.addCase("default", new cxx.Block([new cxx.Line("break;")]));

    code.appendNode(switch_);
    code.addEmptyLine();
    code.addLine("goto InvalidInstruction;");

    for (let k in this.emitHandlers) {
      code.addEmptyLine();
      code.addLine(k + ": {");
      code.addLine(indent(this.emitHandlers[k].code + "goto Emit_Op32;", 2));
      code.addLine("}");
    }

    return {
      dispatchTable: table,
      assemblerImpl: String(code)
    };
  }

  generate() {
    this.generateEmitCode();
    this.generateAssembler();

    let generated = this.genFunc();
    let instructionEnum = "";

    for (let instructionName of this.instructionIdTable) {
      if (instructionName === "")
        instructionEnum += "kIdNone = 0,\n";
      else
        instructionEnum += instNameToEnum(instructionName) + ",\n";
    }
    instructionEnum += "_kIdCount\n";

    const tg = new core.TableGen();
    tg.load([
      "src/asmjit/arm/a32assembler.cpp",
      "src/asmjit/arm/a32emitter.h",
      "src/asmjit/arm/a32globals.h"
    ]);
    tg.inject("a32::Assembler::Dispatch", StringUtils.disclaimer(generated.dispatchTable), 0)
    tg.inject("a32::Assembler::Impl", StringUtils.disclaimer(generated.assemblerImpl), 0)
    tg.inject("a32::EmitterExplicit", StringUtils.disclaimer(this.emitterTable.join("\n") + "\n"), 0)
    tg.inject("a32::InstId", StringUtils.disclaimer(instructionEnum), 0)
    tg.save();
  }
}

const g = new Generator(isa);
g.generate();
