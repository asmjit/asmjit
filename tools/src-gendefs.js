// [GenDefs]
//
// The purpose of this script is to fetch all instructions' names into a single
// string and to optimize common patterns that appear in instruction data. It
// prevents relocation of small strings (instruction names) that has to be done
// by a linker to make all pointers the binary application/library uses valid.
// This approach decreases the final size of AsmJit binary and relocation data.

"use strict";

var fs = require("fs");
var hasOwn = Object.prototype.hasOwnProperty;

// ----------------------------------------------------------------------------
// [Misc]
// ----------------------------------------------------------------------------

const kIndent = "  ";
const kJustify = 79;

const kDisclaimerStart = "// ------------------- Automatically generated, do not edit -------------------\n";
const kDisclaimerEnd   = "// ----------------------------------------------------------------------------\n";

// ----------------------------------------------------------------------------
// [Utils]
// ----------------------------------------------------------------------------

class Utils {
  static upFirst(s) {
    if (!s) return "";
    return s[0].toUpperCase() + s.substr(1);
  }

  static trimLeft(s) {
    return s.replace(/^\s+/, "");
  }

  static padLeft(s, n) {
    while (s.length < n)
      s += " ";
    return s;
  }

  static inject(s, start, end, code) {
    var iStart = s.indexOf(start);
    var iEnd   = s.indexOf(end);

    if (iStart === -1)
      throw new Error(`Couldn't locate start mark.`);

    if (iEnd === -1)
      throw new Error(`Couldn't locate end mark.`);

    return s.substr(0, iStart + start.length) + code + s.substr(iEnd);
  }
}

// ----------------------------------------------------------------------------
// [IndexedString]
// ----------------------------------------------------------------------------

class IndexedString {
  constructor() {
    this.map = Object.create(null);
    this.size = -1;
    this.array = [];
  }

  add(s) {
    this.map[s] = -1;
  }

  index() {
    const map = this.map;
    const array = this.array;
    const partialMap = Object.create(null);

    var k, kp;
    var i, len;

    // Create a map that will contain all keys and partial keys.
    for (k in map) {
      if (!k) {
        partialMap[k] = k;
      }
      else {
        for (i = 0, len = k.length; i < len; i++) {
          kp = k.substr(i);
          if (!hasOwn.call(partialMap, kp) || partialMap[kp].length < len)
            partialMap[kp] = k;
        }
      }
    }

    // Create an array that will only contain keys that are needed.
    for (k in map)
      if (partialMap[k] === k)
        array.push(k);
    array.sort();

    // Create valid offsets to the `array`.
    var offMap = {};
    var offset = 0;

    for (i = 0, len = array.length; i < len; i++) {
      k = array[i];

      offMap[k] = offset;
      offset += k.length + 1;
    }
    this.size = offset;

    // Assign valid offsets to `map`.
    for (kp in map) {
      k = partialMap[kp];
      map[kp] = offMap[k] + k.length - kp.length;
    }
  }

  format(indent, justify) {
    if (this.size === -1)
      throw new Error(`IndexedString - not indexed yet, call index()`);

    const array = this.array;
    if (!justify) justify = 0;

    var i;
    var s = "";
    var line = "";

    for (i = 0; i < array.length; i++) {
      const item = "\"" + array[i] + ((i !== array.length - 1) ? "\\0\"" : "\";");
      const newl = line + (line ? " " : indent) + item;

      if (newl.length <= justify) {
        line = newl;
        continue;
      }
      else {
        s += line + "\n";
        line = indent + item;
      }
    }

    s += line + "\n";
    return s;
  }

  getSize() {
    if (this.size === -1)
      throw new Error(`IndexedString - not indexed yet, call index()`);
    return this.size;
  }

  getIndex(k) {
    if (this.size === -1)
      throw new Error(`IndexedString - not indexed yet, call index()`);

    if (!hasOwn.call(this.map, k))
      throw new Error(`IndexedString - key '${k}' not found.`);

    return this.map[k];
  }
}

// ----------------------------------------------------------------------------
// [Database]
// ----------------------------------------------------------------------------

class Database {
  constructor() {
    this.instMap = Object.create(null);
    this.instArray = [];

    this.instNames = new IndexedString();
    this.instAlpha = new Array(26);

    this.extendedData = [];
    this.extendedMap = {};
  }

  add(name, enum_, id, extendedData) {
    const inst = {
      id            : id,           // Instruction index (number).
      name          : name,         // Instruction name.
      enum          : enum_,        // Instruction enum string.
      nameIndex     : -1,           // Instruction name-index.
      extendedData  : extendedData,
      extendedIndex : ""
    };

    this.instMap[name] = inst;
    this.instArray[id] = inst;

    this.instNames.add(name);
  }

  index() {
    const instMap = this.instMap;
    const instNames = this.instNames;
    const instAlpha = this.instAlpha;

    var extendedData = this.extendedData;
    var extendedMap = this.extendedMap;

    instNames.index();

    for (var name in instMap) {
      var inst = instMap[name];

      var nameIndex = instNames.getIndex(name);
      var extendedIndex = extendedMap[inst.extendedData];

      if (typeof extendedIndex !== "number") {
        extendedIndex = extendedData.length;
        extendedMap[inst.extendedData] = extendedIndex;
        extendedData.push(inst.extendedData);
      }

      inst.nameIndex = nameIndex;
      inst.extendedIndex = extendedIndex;

      var aIndex = name.charCodeAt(0) - 'a'.charCodeAt(0);
      if (aIndex < 0 || aIndex >= 26)
        throw new Error("Alphabetical index error");

      if (instAlpha[aIndex] === undefined)
        instAlpha[aIndex] = inst.enum;
    }
  }

  formatInstNameIndex(indent, justify) {
    const instArray = this.instArray;
    if (!justify) justify = 0;

    for (var i = 0; i < instArray; i++) {
      const inst = instArray[i];
      if (inst === undefined)
        throw new Error(`Database - no instruction #${i}`);
    }

    var i;
    var s = "";
    var line = "";

    for (i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      if (inst === undefined)
        throw new Error(`Database - no instruction #${i}`);

      const item = String(inst.nameIndex) + ((i !== instArray.length - 1) ? "," : "");
      const newl = line + (line ? " " : indent) + item;

      if (newl.length <= justify) {
        line = newl;
        continue;
      }
      else {
        s += line + "\n";
        line = indent + item;
      }
    }

    s += line + "\n";
    return s;
  }
}

// ----------------------------------------------------------------------------
// [Generate]
// ----------------------------------------------------------------------------

function decToHex(n, nPad) {
  var hex = Number(n < 0 ? 0x100000000 + n : n).toString(16);
  while (nPad > hex.length)
    hex = "0" + hex;
  return "0x" + hex.toUpperCase();
}

function getEFlagsMask(eflags, passing) {
  var msk = 0x0;
  var bit = 0x1;

  for (var i = 0; i < 8; i++, bit <<= 1)
    if (passing.indexOf(eflags[i]) !== -1)
      msk |= bit;

  return msk;
}

function generate(fileName, arch) {
  const Arch = Utils.upFirst(arch);
  const oldData = fs.readFileSync(fileName, "utf8").replace(/\r\n/g, "\n");

  var data = oldData;
  var code = "";

  var instCount = 0;
  var sizeof_X86InstInfo = 8;
  var sizeof_X86InstExtendedInfo = 24;

  var kX86InstPrefix = "kX86InstId";

  // Create database.
  var db = new Database();
  var re = new RegExp(
    "INST\\(([A-Za-z0-9_]+)\\s*," +       // [01] Id.
    "\\s*\\\"([A-Za-z0-9_ ]*)\\\"\\s*," + // [02] Name.
    "([^,]+)," +                          // [03] Encoding.
    "(.{20}[^,]*)," +                     // [04] Opcode[0].
    "(.{20}[^,]*)," +                     // [05] Opcode[1].
    "([^,]+)," +                          // [06] IFLAGS.
    "\\s*EF\\(([A-Z_]+)\\)\\s*," +        // [07] EFLAGS.
    "([^,]+)," +                          // [08] Write-Index.
    "([^,]+)," +                          // [09] Write-Size.
    "([^,]+)," +                          // [10] Operand-Flags[0].
    "([^,]+)," +                          // [11] Operand-Flags[1].
    "([^,]+)," +                          // [12] Operand-Flags[2].
    "([^,]+)," +                          // [13] Operand-Flags[3].
    "([^\\)]+)\\)",                       // [14] Operand-Flags[4].
    "g");

  var i, k, m;
  var opCombinations = {};

  while (m = re.exec(data)) {
    // Extract instruction ID and Name.
    const enum_ = kX86InstPrefix + m[1];
    const name = m[2];

    // Extract data that goes to the secondary table (X86InstExtendedInfo).
    const encoding   = Utils.trimLeft(m[3]);
    const opcode0    = Utils.trimLeft(m[4]);
    const opcode1    = Utils.trimLeft(m[5]);
    const iflags     = Utils.trimLeft(m[6]);
    const eflags     = m[7];
    const writeIndex = Utils.trimLeft(m[8]);
    const writeSize  = Utils.trimLeft(m[9]);
    const oflags0    = Utils.trimLeft(m[10]);
    const oflags1    = Utils.trimLeft(m[11]);
    const oflags2    = Utils.trimLeft(m[12]);
    const oflags3    = Utils.trimLeft(m[13]);
    const oflags4    = Utils.trimLeft(m[14]);

    // Generate EFlags-In and EFlags-Out.
    var eflagsIn   = decToHex(getEFlagsMask(eflags, "RX" ), 2);
    var eflagsOut  = decToHex(getEFlagsMask(eflags, "WXU"), 2);

    var ops = `{ ${oflags0}, ${oflags1}, ${oflags2}, ${oflags3}, ${oflags4} }`;
    var extData = `${encoding}, ${writeIndex}, ${writeSize}, ${eflagsIn}, ${eflagsOut}, 0, ${ops}, ${iflags}, ${opcode1}`;

    if (!opCombinations[ops]) opCombinations[ops] = 0;
    opCombinations[ops]++;

    db.add(name, enum_, instCount, extData);
    instCount++;
  }

  db.index();

  var instDataSize = instCount * sizeof_X86InstInfo + db.extendedData.length * sizeof_X86InstExtendedInfo;

  console.log("Number of Instructions  : " + instCount);
  console.log("Number of ExtInfo Rows  : " + db.extendedData.length);
  console.log("Instructions' Data  Size: " + instDataSize);
  console.log("Instructions' Names Size: " + db.instNames.getSize());

  // Generate _x86InstNameData[].
  code += kDisclaimerStart;
  code += `#if !defined(ASMJIT_DISABLE_NAMES)\n`;

  code += `static const char _${arch}InstNameData[] =\n`;
  code += db.instNames.format(kIndent, kJustify);
  code += kDisclaimerEnd;
  code += `\n`;

  // Generate _x86InstNameIndex[].
  code += `static const uint16_t _${arch}InstNameIndex[] = {\n`;
  code += db.formatInstNameIndex(kIndent, kJustify);
  code += `};\n`;
  code += `\n`;

  // Generate AlphaIndex.
  code += `enum ${Arch}InstAlphaIndex {\n`;
  code += `  k${Arch}InstAlphaIndexFirst = 'a',\n`;
  code += `  k${Arch}InstAlphaIndexLast = 'z',\n`;
  code += `  k${Arch}InstAlphaIndexInvalid = 0xFFFF\n`;
  code += `};\n`;
  code += `\n`;

  code += `static const uint16_t _${arch}InstAlphaIndex[26] = {\n`;
  for (i = 0; i < db.instAlpha.length; i++) {
    var id = db.instAlpha[i];
    code += kIndent + (id === undefined ? "0xFFFF" : id);
    if (i !== db.instAlpha.length - 1)
      code += `,`;
    code += `\n`;
  }
  code += `};\n`;

  code += `#endif // !ASMJIT_DISABLE_NAMES\n`;
  code += kDisclaimerEnd;
  code += `\n`;

  // Generate ExtendedInfo.
  code += kDisclaimerStart;
  code += `const ${Arch}InstExtendedInfo _${arch}InstExtendedInfo[] = {\n`;
  for (i = 0; i < db.extendedData.length; i++) {
    code += `${kIndent}{ ${db.extendedData[i]} }`;
    if (i !== db.extendedData.length - 1) code += `,`;
    code += `\n`;
  }
  code += `};\n`;
  code += kDisclaimerEnd;
  code += `\n`;

  code += kDisclaimerStart;
  code += `enum ${Arch}InstData_ExtendedIndex {\n`;
  for (k in db.instMap) {
    var inst = db.instMap[k];
    code += `${kIndent}  ${inst.enum}_ExtendedIndex = ${inst.extendedIndex},\n`;
  }
  code = code.substr(0, code.length - 2) + "\n};\n";
  code += kDisclaimerEnd;

  // Inject.
  data = Utils.inject(data,
    "// ${" + Arch + "InstData:Begin}\n",
    "// ${" + Arch + "InstData:End}\n", code);

  // Save only if modified.
  if (data !== oldData)
    fs.writeFileSync(fileName, data, "utf8");
}

// ----------------------------------------------------------------------------
// [Main]
// ----------------------------------------------------------------------------

function main(files) {
  files.forEach(function(file) {
    generate(file.name, file.arch);
  });
}

main([{
  name: "../src/asmjit/x86/x86inst.cpp",
  arch: "x86"
}]);
