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
const isa = new asmdb.riscv.ISA();

const indent = cxx.Utils.indent;
const toHex = cxx.Utils.toHex;

function dict() { return Object.create(null); }



function instNameToEnum(instName) {
  return !instName ? "kIdNone" : "kId" + instName[0].toUpperCase() + instName.substr(1);
}

class ImmEncodeContext {
  constructor(operands) {
    this.name = "";
    this.imms = dict();
    this.hasImms = false;

    for (let [i, operand] of operands.entries()) {
      if (operand.type === "imm") {
        this.imms[operand.name] = `o${i}.as<Imm>()`;
        this.hasImms = true;
      }
    }
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

class InstructionHandler {
  constructor(id) {
    this.id = id;
    this.block = new cxx.Block();
    this.signatures = dict();
    this.commonVars = dict();
    this.condCnt = 0;
  }

  getBlock(checks) {
    const sgnCheck = checks[0];
    let block = this.signatures[sgnCheck];

    if (!block) {
      const cond = new cxx.If(sgnCheck);
      this.block.addEmptyLine();
      this.block.appendNode(cond);

      block = cond.body;
      this.signatures[sgnCheck] = block;
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

  // TODO: [RISC-V]
}

class Generator {
  constructor(isa) {
    this.isa = isa;

    this.instructionIdMap = new Map();
    this.instructionIdTable = [];

    this.emitHandlers = dict();
    this.emitterMap = dict();
    this.emitterTable = [];
  }

  registerEmitHandler(key, handler) {
    if (!this.emitHandlers[key]) {
      this.emitHandlers[key] = { code: handler, useCount: 1 };
    }
    else {
      this.emitHandlers[key].useCount++;
    }
  }

  registerInstructionHandler(inst) {
    let key = { name: inst.name, encodingType: inst.encoding };

    if (this.instructionIdMap.has(key)) {
      return this.instructionIdMap.get(key);
    }

    const id = this.instructionIdTable.length;
    const ih = new InstructionHandler(id);

    this.instructionIdMap.set(key, ih);
    this.instructionIdTable.push(key);

    return ih;
  }

  generateEmitCode() {
    for (let inst of isa.instructions) {
      let emitOps = inst.operands.map(operand => {
        switch (operand.type) {
          case "reg":
            return "Gp";
          case "imm":
            return "Imm";
          case "-fm":
          case "-pred":
          case "-succ":
            return "Imm";
          default:
            FATAL(`unhandled operand type '${operand.type}'`);
        }
      }).join(", ");

      let emitSignature = `ASMJIT_INST_${inst.operands.length}x(${inst.name}, Inst::${instNameToEnum(inst.name)}${inst.operands.length != 0 ? ", " + emitOps : ""})`;
      
      this.emitterTable.push(emitSignature);
    }
  }

  generateAssembler() {
    for (let inst of isa.instructions) {
      const ih = this.registerInstructionHandler(inst);

      switch (inst.encoding) {
        case "R": {
          break;
        }
        case "I": {
          // TODO: [RISC-V]
          break;
        }
        case "U": {
          // TODO: [RISC-V]
          break;
        }
        case "B": {
          // TODO: [RISC-V]
          break;
        }
        case "J": {
          // TODO: [RISC-V]
          break;
        }
        case "S": {
          // TODO: [RISC-V]
          break;
        }
        // special cases
        case "I-ebreak":
        case "I-ecall":
        case "-fence":
          // TODO: [RISC-V]
          break;
      }

      // TODO: [RISC-V] cleanup there probably dead code

      let fields = inst.fields;

      // Emit handler key and code.
      let ehKey = "";
      let ehBlock = new cxx.Block();
      let checks = [];

      // Signature check, instruction has one or more signatures.
      const sgnOps = inst.operands.map(operand => {
        switch (operand.type) {
          case "reg": return "kOpRegR";
          case "imm": return "KOpImmI";
          case "-fm":
          case "-pred":
          case "-succ": return "KOpImmI";
          default: FATAL(`unhandled operand type ${operand.type}`);
        }
      });

      let sgnCheck = !sgnOps.length ? `sgn.empty()` : `sgn.test<${sgnOps.join(", ")}>()`;
      checks.push(sgnCheck);

      // TODO: [RISC-V] consider adding more necessary checks

      // Block where we generate the code to handle this instruction, its data-type combination(s), and other constaints.
      let ihBlock = ih.getBlock(checks);

      let imm = inst.imm;
      let immCtx = new ImmEncodeContext(inst.operands);
      let immConds = [];
      const imms = dict();

      // TODO: [RISC-V] add conditions to `immConds`

      if (immConds.length) {
        const cond = new cxx.If(immConds.join(" && "));
        ihBlock.appendNode(cond);
        ihBlock = cond.body;
      }

      ihBlock.addLine(`mask = ${toHex(inst.mask, 8)}u;`);
      ihBlock.addLine(`opcode = ${toHex(inst.match, 8)}u;`);

      for (let operand of inst.operands) {
        if (operand.type === "imm") {

        }
      }

      // TODO: [RISC-V]

      this.registerEmitHandler(ehKey, String(ehBlock));
    }
  }

  genFunc() {
    let code = new cxx.Block();
    code.addLine("idr = instDispatchTable[instId];");
    let switch_ = new cxx.Switch("idr.encodingType");

    const map = dict();
    const dispatchTable = [];
    const groups = [];

    let mask = null;

    for (let key of this.instructionIdTable) {
      let [name, encodingType] = [key.name, key.encodingType];
      const ih = this.instructionIdMap.get(key);

      if (ih) {
        let opcode = null;

        let caseCodeOrig = String(new cxx.Block([ih.block, new cxx.Line(""), new cxx.Line("break;")])).replace(/\n$/, "");
        let caseCode = caseCodeOrig
          .replace(/mask = (0x[A-F0-9]+u);/g, (_, p1) => {
            mask = p1;
            return "";
          })
          .replace(/opcode = (0x[A-F0-9]+u);/g, (_, p1) => {
            opcode = p1;
            return "";
          });

        const block = new cxx.Block([new cxx.Line(caseCode)]);
        const dispatchRecord = {name: name, block: block, opcode: opcode};

        if (encodingType in map) {
          dispatchRecord.group = map[encodingType][0].group;
          dispatchRecord.index = map[encodingType].length;
          map[encodingType].push(dispatchRecord);
        }
        else {
          dispatchRecord.group = `EncodingType::${encodingType}`;
          dispatchRecord.index = 0;
          map[encodingType] = [dispatchRecord];
          groups.push(map[encodingType]);
        }
        dispatchTable.push(dispatchRecord);
      }
    }

    let table = "";
    for (let dispatchRecord of dispatchTable) {
      if (table) {
        table += ",\n";
      }
      table += `{${dispatchRecord.group}, ${dispatchRecord.index}}`;
    }
    table = "static const InstDispatchRecord instDispatchTable[] = {\n" + indent(table, 2) + "\n};\n";

    for (let records of groups) {
      const groupId = records[0].group;
      const block = new cxx.Block();

      if (records.length === 1) { // TODO: [RISC-V] FIXME
        const record = records[0];
        function x(match, p1) { return `opcode = ${record.opcode}`; }

        let code = `// Group of '${record.name}'.\n` +
                   record.block.nodes[0].code.replace(/opcode = opcodeTablePtr\[(\d+)\]/g, x);
        block.addLine(code);
      }
      else {
        let code = "";
        for (let record of records) {
          code += record.opcode;
          code += record !== records[records.length - 1] ? ", " : "  ";
          code += `// Instruction '${record.name}'.`;
          code += "\n";
        }

        code = `// Group of '${records.map(r => { return r.name; }).join("|")}'.\n` + "static const Encoding encodingTable[] = {\n" + indent(code, 2) + "};\n";
        code += `const uint32_t mask = ${mask};\n\n`;
        code += "const Encoding &encoding = encodingTable[idr.index];\n\n";
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

    for (let { name: instructionName } of this.instructionIdTable) {
      if (instructionName === "") {
        instructionEnum += "kIdNone = 0,\n";
      }
      else {
        instructionEnum += instNameToEnum(instructionName) + ",\n";
      }
    }
    instructionEnum += "_kIdCount\n";

    const tg = new core.TableGen();
    tg.load([
      "src/asmjit/riscv/riscv64assembler.cpp",
      "src/asmjit/riscv/riscv64emitter.h",
      "src/asmjit/riscv/riscv64globals.h",
    ]);
    tg.inject("riscv64::Assembler::Dispatch", StringUtils.disclaimer(generated.dispatchTable), 0);
    tg.inject("riscv64::Assembler::Impl", StringUtils.disclaimer(generated.assemblerImpl), 0);
    tg.inject("riscv64::EmitterExplicit", StringUtils.disclaimer(this.emitterTable.join("\n") + "\n"), 0);
    tg.inject("riscv64::InstId", StringUtils.disclaimer(instructionEnum), 0);
    tg.save();
  }
}

const g = new Generator();
g.generate();

