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

var upFirst = function(s) {
  if (!s)
    return s;
  return s[0].toUpperCase() + s.substr(1);
};

var trimLeft = function(s) {
  return s.replace(/^\s+/, "");
}

var inject = function(s, start, end, code) {
  var iStart = s.indexOf(start);
  var iEnd   = s.indexOf(end);
  
  if (iStart === -1)
    throw new Error("Couldn't locate start mark.");

  if (iEnd === -1)
    throw new Error("Couldn't locate end mark.");

  return s.substr(0, iStart + start.length) + code + s.substr(iEnd);
};

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
  
  // Create database.
  var db = new Database();
  var re = new RegExp(
    "INST\\(([A-Za-z0-9_]+)\\s*," +       // [01] Inst-Code.
    "\\s*\\\"([A-Za-z0-9_ ]*)\\\"\\s*," + // [02] Inst-Name.
    "([^,]+)," +                          // [03] Inst-Group.
    "([^,]+)," +                          // [04] Inst-Flags.
    "([^,]+)," +                          // [05] Move-Size.
    "([^,]+)," +                          // [06] Operand-Flags[0].
    "([^,]+)," +                          // [07] Operand-Flags[1].
    "([^,]+)," +                          // [08] Operand-Flags[2].
    "([^,]+)," +                          // [09] Operand-Flags[3].
    "\\s*E\\(([A-Z_]+)\\)\\s*," +         // [10] EFLAGS.
    "(.{17}[^,]*)," +                     // [11] OpCode[0].
    "(.{17}[^\\)]*)\\)",                  // [12] OpCode[1].
    "g");

  while (m = re.exec(data)) {
    // Extract instruction ID and Name.
    var id = m[1];
    var name = m[2];

    // Extract data that goes to the secondary table (ExtendedInfo).
    var instGroup = trimLeft(m[3]);
    var instFlags = trimLeft(m[4]);
    var moveSize = trimLeft(m[5]);
    
    var opFlags0 = trimLeft(m[6]);
    var opFlags1 = trimLeft(m[7]);
    var opFlags2 = trimLeft(m[8]);
    var opFlags3 = trimLeft(m[9]);
    var eflags = m[10];
    var opCode1 = trimLeft(m[12]);

    // Generate EFlags-In and EFlags-Out.
    var eflagsIn = decToHex(getEFlagsMask(eflags, "RX"), 2);
    var eflagsOut = decToHex(getEFlagsMask(eflags, "WXU"), 2);

    var extData = "" +
      instGroup + ", " +
      moveSize  + ", " +
      eflagsIn  + ", " +
      eflagsOut + ", " +
      instFlags + ", " + 
      "{ " + opFlags0 + ", " + opFlags1 + ", " + opFlags2 + ", " + opFlags3 + ", U }, " +
      opCode1;

    db.add(name, id, extData);
  }
  db.index();

  console.log("Number of instructions: " + db.instNames.array.length);
  console.log("Instruction names size: " + db.instNames.getSize());
  console.log("Extended-info length  : " + db.extendedData.length);

  // Generate InstName[] string.
  code += disclaimer;
  code += "#if !defined(ASMJIT_DISABLE_INST_NAMES)\n";
  code += "const char _" + arch + "InstName[] =\n";
  for (var k in db.instMap) {
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
  for (var i = 0; i < db.instAlpha.length; i++) {
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
  for (var k in db.instMap) {
    var inst = db.instMap[k];
    code += "  " + inst.id + "_NameIndex = " + inst.nameIndex + ",\n";
  }
  code = code.substr(0, code.length - 2) + "\n};\n";
  code += "#endif // !ASMJIT_DISABLE_INST_NAMES\n"
  code += "\n";

  // Generate ExtendedInfo.
  code += disclaimer;
  code += "const " + Arch + "InstExtendedInfo _" + arch + "InstExtendedInfo[] = {\n";
  for (var i = 0; i < db.extendedData.length; i++) {
    code += "  { " + db.extendedData[i] + " }";
    if (i !== db.extendedData.length - 1)
      code += ",";
    code += "\n";
  }
  code += "};\n";
  code += "\n";

  code += disclaimer;
  code += "enum k" + Arch + "InstData_ExtendedIndex {\n";
  for (var k in db.instMap) {
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
