// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

"use strict";

const core = require("./tablegen.js");
const isa = new core.asmdb.arm.ISA();

const LLVM_MC = "/home/petr/workspace/3rdparty/llvm-project-build/bin/llvm-mc";

function lsbFromMul(mul) {
  for (let i = 0; i < 32; i++)
    if ((mul & (1 << i)) != 0)
      return i;
  return 32;
}
class TestsGenerator {
  constructor(isa) {
    this.isa = isa;
  }

  generate() {
    const child_process = require("child_process");
    const tests = {
      "gp": [],
      "vec": []
    };

    function flatten(inputArray) {
      const outputArray = [inputArray];

      let i = 0;
      while (i < outputArray.length) {
        let didFlatten = false;
        const items = outputArray[i];

        for (let j = 0; j < items.length; j++) {
          if (Array.isArray(items[j])) {
            const flattenedItems = [];
            for (let k = 0; k < items[j].length; k++) {
              const copy = items.slice();
              copy[j] = items[j][k];
              flattenedItems.push(copy);
            }

            outputArray.splice(i, 1, ...flattenedItems);
            didFlatten = true;
            break;
          }
        }

        i += Number(!didFlatten);
      }

      return outputArray;
    }

    function makeShiftOp(sop, op) {
      if (sop === "sop")
        return ["lsl " + op, "lsr " + op, "asr " + op, "ror " + op];
      else if (sop === "lsl_or_asr")
        return ["lsl " + op, "asr " + op];
      else
        return sop + " " + op;
    }

    function encodingFromText(s) {
      let m = s.match(/@\s*encoding\:\s*\[0x([\w]{2}),0x([\w]{2}),0x([\w]{2}),0x([\w]{2})\]/);
      if (m)
        return ('"' + m[1] + m[2] + m[3] + m[4] + '"').toUpperCase();
      else
        return null;
    }

    function codeFromInstruction(s) {
      s = s.trim();
      var sIdx = s.indexOf(" ");
      var name = s.substr(0, sIdx !== -1 ? sIdx : s.length).replace(/\./g, "_");
      var operands = s.substr(name.length).trim();

      var sops = operands.replace(/(v\d+)\.(\d+)?([a-z]+)\[(\d+)\]/g, "$1.$3$2($4)")
                          .replace(/([d|s|q]\d+)\[(\d+)\]/g, "$1.at($2)")
                          .replace(/\[(\w+),\s*[#]?([-]?\w+)\]!/g, "ptr_pre($1, $2)")
                          .replace(/\[(\w+)\],\s*[#]?([-]?\w+)/g, "ptr_post($1, $2)")
                          .replace(/\[(\w+),\s*[#]?([-]?\w+)\]/g, "ptr($1, $2)")
                          .replace(/\[(\w+)\]/g, "ptr($1)")
                          .replace(/\beq\b/g, "Cond::kEQ")
                          .replace(/#((?:0x)?[-]?[\dA-Fa-f]+)\b/g, "$1")
                          .replace(/(lsl|lsr|asr|ror)\s+([r]?\d+)/g, "$1($2)")
                          .replace(/[{}]/g, "");

      if (name === "and")
        name = "and_";

      return name + "(" + sops + ")";
    }

    for (let inst of isa.instructions) {
      if (inst.encoding !== "A32")
        continue;

      let category = "gp";
      let elements = [""];

      const dt1Array = inst.dt.length ? inst.dt : ["any"];
      const dt2Array = inst.dt2.length ? inst.dt2 : ["any"];
      const dataTypes = [];

      for (let dt2 of dt2Array) {
        for (let dt1 of dt1Array) {
          if (dt2 !== "any")
            dataTypes.push(`.${dt1}.${dt2}`);
          else if (dt1 !== "any")
            dataTypes.push(`.${dt1}`);
          else
            dataTypes.push(``);
        }
      }

      for (let dataType of dataTypes) {
        let instruction = inst.name + dataType;
        let ops = [];
        let proceed = true;

        for (let i = 0; i < inst.operands.length; i++) {
          const operand = inst.operands[i];
          let regId = i + 1;

          if (/^(ldaexd|ldrd|ldrexd|strd)$/.test(inst.name)) {
            regId--;
          }
          else if (/^(stlexd|strexd)$/.test(inst.name)) {
            if (i == 0)
              regId = 7;
            else
              regId = i - 1;
          }

          switch (operand.type) {
            case "reg": {
              if (operand.regType === "r" && operand.shiftOp) {
                ops.push(makeShiftOp(operand.shiftOp, `r${i + 1}`));
              }
              else if (operand.regType === "r") {
                ops.push(`r${regId}`);
              }
              else if (operand.regType === "s") {
                ops.push(`s${regId}`);
                category = "vec";
              }
              else if (operand.regType === "d") {
                ops.push(`d${regId}`);
                category = "vec";
              }
              else if (operand.regType === "v") {
                ops.push(`q${regId}`);
                category = "vec";
              }
              else {
                proceed = false;
              }

              if (operand.element) {
                ops[ops.length - 1] += "[@element@]";
                switch (dataType) {
                  case ".8":
                  case ".s8":
                  case ".u8":
                    elements = [0, 1, 2, 3, 4, 5, 6, 7];
                    break;
                  case ".16":
                  case ".s16":
                  case ".u16":
                  case ".f16":
                    if (inst.name === "vcmla" || inst.name === "vfmsl" || inst.name === "vfmal")
                      elements = [0, 1];
                    else
                      elements = [0, 1, 2, 3];
                    break;
                  case ".32":
                  case ".s32":
                  case ".u32":
                  case ".f32":
                    if (inst.name === "vcmla")
                      elements = [0];
                    else
                      elements = [0, 1];
                    break;
                  case ".64":
                  case ".s64":
                  case ".u64":
                  case ".f64":
                    elements = [0, 1];
                    break;

                  case ".bf16":
                    if (inst.name === "vdot")
                      elements = [0, 1];
                    else
                      elements = [0, 1, 2, 3];
                    break;
                }
              }

              break;
            }

            case "imm": {
              if (inst.name === "vcadd")
                ops.push(["#90", "#270"]);
              else if (inst.name === "vcmla")
                ops.push(["#0", "#90", "#180", "#270"]);
              else if (operand.shiftOp)
                ops.push(makeShiftOp(operand.shiftOp, "#8"));
              else if (operand.imm === "zero")
                ops.push("#0");
              else if (operand.imm === "immA")
                ops.push(["#0xFF", "0xFF00", "0xFF000000", "0xF000000F"]);
              else if (operand.imm === "immZ")
                ops.push("#1");
              else if (operand.imm === "immV" && instruction.endsWith("16"))
                ops.push(["#0x1F", "#0x80", "#0xFF", "0x1F00", "0xFF00"]);
              else if (operand.imm === "immV" && instruction.endsWith("32"))
                ops.push(["#0x1F", "#0x80", "#0xFF", "0x1F00", "0xFF00", "0x1F0000", "0xFF0000", "0xFF000000", "0xFFFF0000", "0x0000FFFF", "0xFFFFFF00", "0x00FFFFFF", "0xC0F00000"]);
              else if (operand.imm === "immV" && instruction.endsWith("64"))
                ops.push(["#00FFFF0000000000", "#0xFF00FF0000000000", "#0x00000000FFFFFFFF"]);
              else if (operand.imm === "lsb")
                ops.push("#3");
              else if (operand.imm === "width")
                ops.push("#5");
              else if (operand.imm === "sat")
                ops.push("#8");
              else if (operand.imm === "imm")
                ops.push("#0");
              else if (operand.imm === "n")
                ops.push("#3");
              else
                proceed = false;
              break;
            }

            case "mem": {
              const combinations = [];
              var off = "4";

              if (operand.offset && operand.offset.exp) {
                const e = operand.offset.exp;
                if (e.op === "==") {
                  off = e.right.toString();
                  if (off.indexOf("sz") !== -1 && dataType) {
                    const sz = dataType.match(/(\d+)/);
                    off = eval(off.replace("sz", parseInt(lsbFromMul(sz[1] / 8))));
                  }
                }
              }

              if (operand.memModes.offset) {
                if (operand.index)
                  combinations.push(`[r${regId}, r${regId + 1}]`);
                else if (operand.offset)
                  combinations.push(`[r${regId}, #${off}]`);
                else
                  combinations.push(`[r${regId}]`);
              }

              if (operand.memModes.preIndex) {
                if (operand.index)
                  combinations.push(`[r${regId}, r${regId + 1}]!`);
                else if (operand.offset)
                  combinations.push(`[r${regId}, #${off}]!`);
              }

              if (operand.memModes.postIndex) {
                if (operand.index)
                  combinations.push(`[r${regId}], r${regId + 1}`);
                else if (operand.offset)
                  combinations.push(`[r${regId}], #${off}`);
              }

              ops.push(combinations);
              break;
            }

            default: {
              proceed = false;
              break;
            }
          }
        }

        if (!proceed) {
          console.log(`IGNORING: ${inst.name}`);
          continue;
        }

        const opsArray = flatten(ops);

        const args = [
          "--arch=arm",
          "-mattr=+v8.5a",
          "-mattr=+aes",
          "-mattr=+bf16",
          "-mattr=+crc",
          "-mattr=+crypto",
          "-mattr=+hwdiv-arm",
          "-mattr=+fp16fml",
          "-mattr=+fullfp16",
          "-mattr=+i8mm",
          "-mattr=+mp",
          "-mattr=+trustzone",
          "-mattr=+virtualization",
          "-show-encoding",
          "-output-asm-variant=0"
        ];

        for (let element of elements) {
          for (let opsRow of opsArray) {
            let line = instruction;
            opsRow = opsRow.map(function(s) { return s.replace("@element@", element) });

            let o = opsRow.slice();
            switch (inst.name) {
              case "pop":
              case "push": {
                o[0] = "{" + o[0];
                o[o.length - 1] += "}";
                break;
              }

              case "vcls": {
                line = line.replace(".u", ".s");
                break;
              }

              case "vtbx":
              case "vtbl": {
                o[1] = "{" + o[1];
                o[o.length - 2] += "}";
                break;
              }

              case "rfe":
              case "rfeda":
              case "rfedb":
              case "rfeib": {
                o[0] = o[0].replace(/[\[\]]/g, "");
                break;
              }

              case "vld1r":
              case "vld2r":
              case "vld3r":
              case "vld4r": {
                line = line.replace("vld1r", "vld1")
                           .replace("vld2r", "vld2")
                           .replace("vld3r", "vld3")
                           .replace("vld4r", "vld4");
                for (let i = 0; i < o.length - 1; i++)
                  o[i] += "[]";
                // fallthrough;
              }

              case "vld1":
              case "vld2":
              case "vld3":
              case "vld4":
              case "vst1":
              case "vst2":
              case "vst3":
              case "vst4": {
                o[o.length - 1] = o[o.length - 1].replace(/,\s*#\d+$/g, "!");
                o[0] = "{" + o[0];
                o[o.length - 2] += "}";
                break;
              }
            }

            try {
              const out = child_process.execFileSync(LLVM_MC, args, {
                input: line + " " + o.join(", "),
                encoding: "utf-8"
              });

              const encoding = encodingFromText(out);
              const code = codeFromInstruction(instruction + " " + opsRow.join(", "));

              tests[category].push(`TEST_INSTRUCTION(${encoding}, ${code});`);
            }
            catch (ex) {
              console.log(`FATALED: ${line} ${opsRow.join(", ")}`);
            }
          }
        }
      }
    }

    console.log("static void ASMJIT_NOINLINE testA32AssemblerGp(AssemblerTester<a32::Assembler>& tester) noexcept {");
    console.log("  using namespace a32;");
    console.log("");
    for (let test of tests.gp) {
      console.log("  " + test);
    }
    console.log("}");

    console.log("");

    console.log("static void ASMJIT_NOINLINE testA32AssemblerVec(AssemblerTester<a32::Assembler>& tester) noexcept {");
    console.log("  using namespace a32;");
    console.log("");
    for (let test of tests.vec)
      console.log("  " + test);
    console.log("}");
  }
}
new TestsGenerator(isa).generate();
