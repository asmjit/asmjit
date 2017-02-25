// [Generate-Base]
"use strict";

const fs = require("fs");
const hasOwn = Object.prototype.hasOwnProperty;

// ----------------------------------------------------------------------------
// [asmdb]
// ----------------------------------------------------------------------------

const asmdb = (function() {
  try {
    // Prefer a local copy of 'asmdb' package if possible.
    return require("./asmdb");
  }
  catch (ex) {
    // Report a possible problem within a local asmdb.
    if (ex.code !== "MODULE_NOT_FOUND")
      throw ex;

    // Okay, so global then...
    return require("asmdb");
  }
})();
exports.asmdb = asmdb;

// ----------------------------------------------------------------------------
// [Constants]
// ----------------------------------------------------------------------------

const kIndent = "  ";
const kJustify = 79;
const kAsmJitRoot = "..";

exports.kIndent = kIndent;
exports.kJustify = kJustify;
exports.kAsmJitRoot = kAsmJitRoot;

// ----------------------------------------------------------------------------
// [StringUtils]
// ----------------------------------------------------------------------------

function asString(x) { return String(x); }

class StringUtils {
  static trimLeft(s) {
    return s.replace(/^\s+/, "");
  }

  static padLeft(s, n, x) {
    if (!x) x = " ";

    s = String(s);
    if (s.length < n)
      s += x.repeat(n - s.length);

    return s;
  }

  static upFirst(s) {
    if (!s) return "";
    return s[0].toUpperCase() + s.substr(1);
  }

  static decToHex(n, nPad) {
    var hex = Number(n < 0 ? 0x100000000 + n : n).toString(16);
    while (nPad > hex.length)
      hex = "0" + hex;
    return "0x" + hex.toUpperCase();
  }

  static format(array, indent, showIndex, mapFn) {
    if (!mapFn)
      mapFn = asString;

    var s = "";
    const commentSize = showIndex ? String(array.length).length : 0;

    for (var i = 0; i < array.length; i++) {
      const last = i === array.length - 1;
      s += `${indent}${mapFn(array[i])}`;

      if (commentSize)
        s += `${last ? " " : ","} // #${i}`;
      else if (!last)
        s += ",";

      if (!last) s += "\n";
    }

    return s;
  }

  static makeCxxArray(array, code, indent) {
    if (!indent) indent = kIndent;
    return `${code} = {\n${indent}` + array.join(`,\n${indent}`) + `\n};\n`;
  }

  static makeCxxArrayWithComment(array, code, indent) {
    if (!indent) indent = kIndent;
    var s = "";
    for (var i = 0; i < array.length; i++) {
      const last = i === array.length - 1;
      s += indent + array[i].data +
           (last ? "  // " : ", // ") + StringUtils.padLeft(array[i].refs ? "#" + String(i) : "", 5) + array[i].comment + "\n";
    }
    return `${code} = {\n${s}};\n`;
  }

  static disclaimer(s) {
    return "// ------------------- Automatically generated, do not edit -------------------\n" +
           s +
           "// ----------------------------------------------------------------------------\n";
  }

  static indent(s, indentation) {
    var lines = s.split(/\r?\n/g);
    if (indentation) {
      for (var i = 0; i < lines.length; i++) {
        var line = lines[i];
        if (line) lines[i] = indentation + line;
      }
    }

    return lines.join("\n");
  }

  static inject(s, start, end, code) {
    var iStart = s.indexOf(start);
    var iEnd   = s.indexOf(end);

    if (iStart === -1)
      throw new Error(`Utils.inject(): Couldn't locate start mark '${start}'`);

    if (iEnd === -1)
      throw new Error(`Utils.inject(): Couldn't locate end mark '${end}'`);

    var nIndent = 0;
    while (iStart > 0 && s[iStart-1] === " ") {
      iStart--;
      nIndent++;
    }

    if (nIndent) {
      const indentation = " ".repeat(nIndent);
      code = StringUtils.indent(code, indentation) + indentation;
    }

    return s.substr(0, iStart + start.length + nIndent) + code + s.substr(iEnd);
  }
}
exports.StringUtils = StringUtils;

// ----------------------------------------------------------------------------
// [MapUtils]
// ----------------------------------------------------------------------------

class MapUtils {
  static arrayToMap(arr, value) {
    if (value === undefined)
      value = true;

    const map = Object.create(null);
    for (var i = 0; i < arr.length; i++)
      map[arr[i]] = value;
    return map;
  }

  static equals(a, b) {
    for (var k in a) if (!hasOwn.call(b, k)) return false;
    for (var k in b) if (!hasOwn.call(a, k)) return false;

    return true;
  }

  static firstOf(map, flags) {
    for (var k in flags)
      if (hasOwn.call(map, k))
        return k;
    return undefined;
  }

  static anyOf(map, flags) {
    for (var k in flags)
      if (hasOwn.call(map, k))
        return true;
    return false;
  }

  static add(a, b) {
    for (var k in b)
      a[k] = b[k];
    return a;
  }

  static and(a, b) {
    const out = Object.create(null);
    for (var k in a)
      if (hasOwn.call(b, k))
        out[k] = true;
    return out;
  }

  static xor(a, b) {
    const out = Object.create(null);
    for (var k in a) if (!hasOwn.call(b, k)) out[k] = true;
    for (var k in b) if (!hasOwn.call(a, k)) out[k] = true;
    return out;
  }
};
exports.MapUtils = MapUtils;

// ----------------------------------------------------------------------------
// [IndexedArray]
// ----------------------------------------------------------------------------

class IndexedArray extends Array {
  constructor() {
    super();
    this._index = Object.create(null);
  }

  addIndexed(element) {
    const key = typeof element === "string" ? element : JSON.stringify(element);
    var idx = this._index[key];

    if (idx !== undefined)
      return idx;

    idx = this.length;
    this._index[key] = idx;
    this.push(element);
    return idx;
  }
}
exports.IndexedArray = IndexedArray;

// ----------------------------------------------------------------------------
// [IndexedString]
// ----------------------------------------------------------------------------

class IndexedString {
  constructor() {
    this.map = Object.create(null);
    this.array = [];
    this.size = -1;
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
    var offMap = Object.create(null);
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
      throw new Error(`IndexedString.format(): not indexed yet, call index()`);

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

    return s + line;
  }

  getSize() {
    if (this.size === -1)
      throw new Error(`IndexedString.getSize(): Not indexed yet, call index()`);
    return this.size;
  }

  getIndex(k) {
    if (this.size === -1)
      throw new Error(`IndexedString.getIndex(): Not indexed yet, call index()`);

    if (!hasOwn.call(this.map, k))
      throw new Error(`IndexedString.getIndex(): Key '${k}' not found.`);

    return this.map[k];
  }
}
exports.IndexedString = IndexedString;

// ----------------------------------------------------------------------------
// [BaseGenerator]
// ----------------------------------------------------------------------------

class BaseGenerator {
  constructor(arch) {
    this.arch = arch;

    this.instMap = Object.create(null);
    this.instArray = [];

    this.files = Object.create(null);
    this.tableSizes = Object.create(null);
  }

  // --- File management ---
  load(fileList) {
    for (var i = 0; i < fileList.length; i++) {
      const file = fileList[i];
      const path = kAsmJitRoot + "/" + file;
      const data = fs.readFileSync(path, "utf8").replace(/\r\n/g, "\n");

      this.files[file] = {
        prev: data,
        data: data
      };
    }
    return this;
  }

  save() {
    for (var file in this.files) {
      const obj = this.files[file];
      if (obj.data !== obj.prev) {
        const path = kAsmJitRoot + "/" + file;
        console.log(`MODIFIED '${file}'`);

        fs.writeFileSync(path + ".backup", obj.prev, "utf8");
        fs.writeFileSync(path, obj.data, "utf8");
      }
    }
  }

  dataOf(file) {
    const obj = this.files[file];
    if (!obj)
      throw new Error(`BaseGenerator.getData(): File ${file} not loaded`);
    return obj.data;
  }

  // --- Instruction management ---
  addInst(inst) {
    inst.id = this.instArray.length;

    this.instMap[inst.name] = inst;
    this.instArray.push(inst);

    return this;
  }

  // --- Code Injection ---
  inject(key, str, size) {
    const begin = "// ${" + key + ":Begin}\n";
    const end   = "// ${" + key + ":End}\n";

    var done = false;
    for (var file in this.files) {
      const obj = this.files[file];
      const data = obj.data;

      if (data.indexOf(begin) !== -1) {
        obj.data = StringUtils.inject(data, begin, end, str);
        done = true;
        break;
      }
    }

    if (!done)
      throw new Error(`Generator.inject(): Cannot find '${key}'`);

    if (size)
      this.tableSizes[key] = size;

    return this;
  }

  // --- Independent Generators ---
  generateIdData() {
    const instArray = this.instArray;

    var s = "";
    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];

      var line = "kId" + inst.enum + (i ? "" : " = 0") + ",";
      var comment = this.getCommentOf(inst.name);

      if (comment)
        line = StringUtils.padLeft(line, 37) + "// " + comment;

      s += line + "\n";
    }
    s += "_kIdCount\n";

    return this.inject("idData", s);
  }

  generateNameData() {
    const arch = this.arch;
    const none = `${arch}Inst::kIdNone`;

    const instArray = this.instArray;
    const instNames = new IndexedString();

    const instFirst = new Array(26);
    const instLast  = new Array(26);

    var maxLength = 0;
    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      instNames.add(inst.name);
      maxLength = Math.max(maxLength, inst.name.length);
    }
    instNames.index();

    for (var i = 0; i < instArray.length; i++) {
      const inst = instArray[i];
      const name = inst.name;
      const nameIndex = instNames.getIndex(name);

      const index = name.charCodeAt(0) - 'a'.charCodeAt(0);
      if (index < 0 || index >= 26)
        throw new Error(`BaseGenerator.generateNameData(): Invalid lookup character '${name[0]}' of '${name}'`);

      inst.nameIndex = nameIndex;
      if (instFirst[index] === undefined)
        instFirst[index] = `${arch}Inst::kId${inst.enum}`;
      instLast[index] = `${arch}Inst::kId${inst.enum}`;
    }

    var s = "";
    s += `const char ${arch}InstDB::nameData[] =\n${instNames.format(kIndent, kJustify)}\n`;
    s += `\n`;

    s += `enum {\n`;
    s += `  k${arch}InstMaxLength = ${maxLength}\n`;
    s += `};\n`;
    s += `\n`;

    s += `struct InstNameAZ {\n`;
    s += `  uint16_t start;\n`;
    s += `  uint16_t end;\n`;
    s += `};\n`;
    s += `\n`;

    s += `static const InstNameAZ ${arch}InstNameAZ[26] = {\n`;
    for (var i = 0; i < instFirst.length; i++) {
      const firstId = instFirst[i] || none;
      const lastId = instLast[i] || none;

      s += `  { ${StringUtils.padLeft(firstId, 22)}, ${StringUtils.padLeft(lastId , 22)} + 1 }`;
      if (i !== 26 - 1)
        s += `,`;
      s += `\n`;
    }
    s += `};\n`;

    return this.inject("nameData", StringUtils.disclaimer(s), instNames.getSize() + 26 * 4);
  }

  // --- Reimplement ---
  getCommentOf(name) {
    throw new Error("BaseGenerator.getCommentOf(): Must be reimplemented");
  }

  // --- Miscellaneous ---
  dumpTableSizes() {
    const sizes = this.tableSizes;

    var pad = 24;
    var total = 0;

    for (var name in sizes) {
      const size = sizes[name];
      total += size;
      console.log(StringUtils.padLeft('Size of ' + name, pad) + ": " + size);
    }

    console.log(StringUtils.padLeft('Size of all tables', pad) + ": " + total);
  }
}
exports.BaseGenerator = BaseGenerator;
