// [GenDefs]
//
// The purpose of this script is to fetch all instructions' names into a single
// string and to optimize common patterns that appear in instruction data. It
// prevents relocation of small strings (instruction names) that has to be done
// by a linker to make all pointers the binary application/library uses valid.
// This approach decreases the final size of AsmJit binary and relocation data.

var fs = require("fs");

// ----------------------------------------------------------------------------
// [Utilities]
// ----------------------------------------------------------------------------

function upFirst(s) {
  if (!s)
    return s;
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
// [Database]
// ----------------------------------------------------------------------------

var Database = (function() {
  // `IndexedString` class.
  var IndexedString = function() {
    this.array = [];
    this.index = 0;
    this.map = {};
  };

  IndexedString.prototype.add = function(s) {
    var index = this.map[s];

    if (typeof index === "number")
      return index;

    index = this.index;
    this.array.push(s);
    this.index += s.length + 1;
    this.map[s] = index;
    return index;
  };

  IndexedString.prototype.get = function(s) {
    return this.map[s];
  };

  IndexedString.prototype.format = function(indent) {
    var s = "";
    var array = this.array;

    for (var i = 0; i < array.length; i++) {
      s += indent + "\"" + array[i] + "\\0\"";
      if (i === array.length - 1)
        s += ";";
      s += "\n";
    }

    return s;
  };

  IndexedString.prototype.getSize = function() {
    return this.index;
  };

  // `Database` class.
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
      nameIndex     : 0,  // Instruction name-index, used directly by AsmJit.
      vPrefix       : 0,  // Instruction starts with 'v', not used at this point.
      extendedData  : extendedData,
      extendedIndex : ""
    };
  };

  Database.prototype.index = function() {
    var instMap = this.instMap;
    var instNames = this.instNames;
    var instAlpha = this.instAlpha;

    var extendedData = this.extendedData;
    var extendedMap = this.extendedMap;

    for (var name in instMap) {
      var inst = instMap[name];

      var nameIndex = instNames.add(name);
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

      if (name.indexOf("v") === 0) {
        inst.vPrefix = 1;
      }
    }
  };

  return Database;
})();

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
  for (k in db.instMap) {
    var inst = db.instMap[k];
    code += "  \"" + k + "\\0\"\n";
  }
  code = code.substr(code, code.length - 1) + ";\n\n";

  // Generate AlphaIndex.
  code += disclaimer;
  code += "enum k" + Arch + "InstAlphaIndex {\n";
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
  code += "enum k" + Arch + "InstData_NameIndex {\n";
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
  code += "enum k" + Arch + "InstData_ExtendedIndex {\n";
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
