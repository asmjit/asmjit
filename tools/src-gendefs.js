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
// [Utilities]
// ----------------------------------------------------------------------------

function upFirst(s) {
  if (!s) return "";
  return s[0].toUpperCase() + s.substr(1);
}

function trimLeft(s) {
  return s.replace(/^\s+/, "");
}

function padLeft(s, n) {
  while (s.length < n)
    s += " ";
  return s;
}

function inject(s, start, end, code) {
  var iStart = s.indexOf(start);
  var iEnd   = s.indexOf(end);

  if (iStart === -1)
    throw new Error("Couldn't locate start mark.");

  if (iEnd === -1)
    throw new Error("Couldn't locate end mark.");

  return s.substr(0, iStart + start.length) + code + s.substr(iEnd);
}

// ----------------------------------------------------------------------------
// [IndexedString]
// ----------------------------------------------------------------------------

var IndexedString = function() {
  this.map = {};
  this.size = -1;
  this.array = [];
};

IndexedString.prototype.add = function(s) {
  this.map[s] = -1;
};

IndexedString.prototype.index = function() {
  var map = this.map;
  var array = this.array;

  var partialMap = {};
  var k, kp;
  var i, len;

  // Create a map that will contain all keys and partial keys.
  for (k in map) {
    if (!k) {
      partialMap[k] = k;
    }
    else {
      for (i = 0, len = k.length; i < len; i++) {
        var kp = k.substr(i);
        if (!hasOwn.call(partialMap, kp) || partialMap[kp].length < len)
          partialMap[kp] = k;
      }
    }
  }

  // Create an array that will only contain keys that are needed.
  for (k in map) {
    if (partialMap[k] === k)
      array.push(k);
  }
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

  /*
  (function() {
    // Testing code to experiment with eliminating suffixes from instruction names.
    var suffixList = [
      "ss", "ps", "sd", "pd",
      "bw", "bd", "bq",
      "ww", "wd", "wq",
      "dq", "b", "w", "d", "q"
    ];
    var reducedMap = {};
    var reducedSize = 0;

    var xMap = {};
    var xArr = [];
    
    for (i = 0, len = array.length; i < len; i++) {
      k = array[i];

      var suffix = null;
      var after = k;

      for (var j = 0; j < suffixList.length; j++) {
        suffix = suffixList[j];
        if (k.lastIndexOf(suffix) === k.length - suffix.length) {
          after = k.substr(0, k.length - suffix.length);
          break;
        }
      }

      reducedMap[after] = true;
    }

    // Testing code to get which suffixes are the most used.
    for (k in map) {
      for (i = 1; i < k.length; i++) {
        var xKey = k.substr(i);
        if (hasOwn.call(xMap, xKey)) {
          xMap[xKey]++;
        }
        else {
          xMap[xKey] = 1;
          xArr.push(xKey);
        }
      }
    }

    xArr.sort(function(a, b) {
      return xMap[a] - xMap[b];
    });
    for (i = 0; i < xArr.length; i++) {
      console.log(xArr[i] + " " + xMap[xArr[i]]);
    }

    for (k in reducedMap)
      reducedSize += k.length + 1;
    console.log("ReducedSize=" + reducedSize);
  })();
  */
};

IndexedString.prototype.format = function(indent) {
  if (this.size === -1)
    throw new Error("IndexedString - not indexed yet, call index()");

  var s = "";
  var array = this.array;

  for (var i = 0; i < array.length; i++) {
    s += indent + "\"" + array[i];
    s += (i !== array.length - 1) ? "\\0\"" : "\";";
    s += "\n";
  }

  return s;
};

IndexedString.prototype.getSize = function() {
  if (this.size === -1)
    throw new Error("IndexedString - not indexed yet, call index()");
  return this.size;
};

IndexedString.prototype.getIndex = function(k) {
  if (this.size === -1)
    throw new Error("IndexedString - not indexed yet, call index()");

  if (!hasOwn.call(this.map, k))
    throw new Error("IndexedString - key '" + k + "' not found.");

  return this.map[k];
};

// ----------------------------------------------------------------------------
// [Database]
// ----------------------------------------------------------------------------

var Database = function() {
  this.instMap = {};
  this.instNames = new IndexedString();
  this.instAlpha = new Array(26);

  this.extendedData = [];
  this.extendedMap = {};
};

Database.prototype.add = function(name, id, extendedData) {
  this.instMap[name] = {
    id            : id, // Instruction ID.
    nameIndex     : -1, // Instruction name-index.
    extendedData  : extendedData,
    extendedIndex : ""
  };
  this.instNames.add(name);
};

Database.prototype.index = function() {
  var instMap = this.instMap;
  var instNames = this.instNames;
  var instAlpha = this.instAlpha;

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
      instAlpha[aIndex] = inst.id;
  }
};

// ----------------------------------------------------------------------------
// [Generate]
// ----------------------------------------------------------------------------

var decToHex = function(n, nPad) {
  var hex = Number(n < 0 ? 0x100000000 + n : n).toString(16);
  while (nPad > hex.length)
    hex = "0" + hex;
  return "0x" + hex.toUpperCase();
};

var getEFlagsMask = function(eflags, passing) {
  var msk = 0x0;
  var bit = 0x1;

  for (var i = 0; i < 8; i++, bit <<= 1) {
    if (passing.indexOf(eflags[i]) !== -1)
      msk |= bit;
  }

  return msk;
};

var generate = function(fileName, arch) {
  var Arch = upFirst(arch);
  var oldData = fs.readFileSync(fileName, "utf8").replace(/\r\n/g, "\n");

  var data = oldData;
  var code = "";
  var disclaimer = "// Automatically generated, do not edit.\n";

  var instCount = 0;
  var sizeof_X86InstInfo = 8;
  var sizeof_X86InstExtendedInfo = 24;

  // Create database.
  var db = new Database();
  var re = new RegExp(
    "INST\\(([A-Za-z0-9_]+)\\s*," +       // [01] Id.
    "\\s*\\\"([A-Za-z0-9_ ]*)\\\"\\s*," + // [02] Name.
    "(.{20}[^,]*)," +                     // [03] Opcode[0].
    "(.{20}[^,]*)," +                     // [04] Opcode[1].
    "([^,]+)," +                          // [05] Encoding.
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
  var srcForm = "";

  while (m = re.exec(data)) {
    // Extract instruction ID and Name.
    var id = m[1];
    var name = m[2];

    // Extract data that goes to the secondary table (X86InstExtendedInfo).
    var opcode0    = trimLeft(m[3]);
    var opcode1    = trimLeft(m[4]);
    var encoding   = trimLeft(m[5]);
    var iflags     = trimLeft(m[6]);
    var eflags     = m[7];
    var writeIndex = trimLeft(m[8]);
    var writeSize  = trimLeft(m[9]);
    var oflags0    = trimLeft(m[10]);
    var oflags1    = trimLeft(m[11]);
    var oflags2    = trimLeft(m[12]);
    var oflags3    = trimLeft(m[13]);
    var oflags4    = trimLeft(m[14]);

    // Generate EFlags-In and EFlags-Out.
    var eflagsIn   = decToHex(getEFlagsMask(eflags, "RX" ), 2);
    var eflagsOut  = decToHex(getEFlagsMask(eflags, "WXU"), 2);

    var extData =
      encoding   + ", " +
      writeIndex + ", " +
      writeSize  + ", " +
      eflagsIn   + ", " +
      eflagsOut  + ", " +
      "0"        + ", " +
      "{ "       + oflags0 + ", " + oflags1 + ", " + oflags2 + ", " + oflags3 + ", " + oflags4 + " }, " +
      iflags     + ", " +
      opcode1;

    srcForm += "  INST(" +
      padLeft(id, 27) + ", " +
      padLeft('"' + name + '"', 19) + ", " +
      opcode0    + ", " +
      opcode1    + ", " +
      encoding   + ", " +
      iflags     + ", " +
      "EF(" + eflags + "), " +
      writeIndex + ", " +
      writeSize  + ", " +
      oflags0    + ", " +
      oflags1    + ", " +
      oflags2    + ", " +
      oflags3    + ", " +
      oflags4    + "),\n";

    db.add(name, id, extData);
    instCount++;
  }
  // fs.writeFileSync("srcform.cpp", srcForm, "utf8");
  db.index();

  var instDataSize = instCount * sizeof_X86InstInfo + db.extendedData.length * sizeof_X86InstExtendedInfo;

  console.log("Number of Instructions  : " + instCount);
  console.log("Number of ExtInfo Rows  : " + db.extendedData.length);
  console.log("Instructions' Data  Size: " + instDataSize);
  console.log("Instructions' Names Size: " + db.instNames.getSize());

  // Generate InstName[] string.
  code += disclaimer;
  code += "#if !defined(ASMJIT_DISABLE_NAMES)\n";
  code += "const char _" + arch + "InstName[] =\n";
  code += db.instNames.format("  ") + "\n";

  // Generate AlphaIndex.
  code += disclaimer;
  code += "enum " + Arch + "InstAlphaIndex {\n";
  code += "  k" + Arch + "InstAlphaIndexFirst = 'a',\n";
  code += "  k" + Arch + "InstAlphaIndexLast = 'z',\n";
  code += "  k" + Arch + "InstAlphaIndexInvalid = 0xFFFF\n";
  code += "};\n";
  code += "\n";

  code += disclaimer;
  code += "static const uint16_t _" + arch + "InstAlphaIndex[26] = {\n";
  for (i = 0; i < db.instAlpha.length; i++) {
    var id = db.instAlpha[i];
    code += "  " + (id === undefined ? "0xFFFF" : id);
    if (i !== db.instAlpha.length - 1)
      code += ",";
    code += "\n";
  }
  code += "};\n\n";

  // Generate NameIndex.
  code += disclaimer;
  code += "enum " + Arch + "InstData_NameIndex {\n";
  for (k in db.instMap) {
    var inst = db.instMap[k];
    code += "  " + inst.id + "_NameIndex = " + inst.nameIndex + ",\n";
  }
  code = code.substr(0, code.length - 2) + "\n};\n";
  code += "#endif // !ASMJIT_DISABLE_NAMES\n"
  code += "\n";

  // Generate ExtendedInfo.
  code += disclaimer;
  code += "const " + Arch + "InstExtendedInfo _" + arch + "InstExtendedInfo[] = {\n";
  for (i = 0; i < db.extendedData.length; i++) {
    code += "  { " + db.extendedData[i] + " }";
    if (i !== db.extendedData.length - 1)
      code += ",";
    code += "\n";
  }
  code += "};\n";
  code += "\n";

  code += disclaimer;
  code += "enum " + Arch + "InstData_ExtendedIndex {\n";
  for (k in db.instMap) {
    var inst = db.instMap[k];
    code += "  " + inst.id + "_ExtendedIndex = " + inst.extendedIndex + ",\n";
  }
  code = code.substr(0, code.length - 2) + "\n};\n";

  // Inject.
  data = inject(data,
    "// ${" + Arch + "InstData:Begin}\n",
    "// ${" + Arch + "InstData:End}\n",
    code);

  // Save only if modified.
  if (data !== oldData)
    fs.writeFileSync(fileName, data, "utf8");
};

// ----------------------------------------------------------------------------
// [Main]
// ----------------------------------------------------------------------------

var main = function(files) {
  files.forEach(function(file) {
    generate(file.name, file.arch);
  });
};

main([
  {
    name: "../src/asmjit/x86/x86inst.cpp",
    arch: "x86"
  }
]);
