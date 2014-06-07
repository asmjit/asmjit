// [GenDefs]
//
// The purpose of this script is to fetch all instructions' names into a single
// string. It prevents relocation that has to be done by linked to make all
// pointers the binary application/library uses valid. This approach decreases
// the final size of AsmJit binary.

var fs = require("fs");

// ----------------------------------------------------------------------------
// [Configuration]
// ----------------------------------------------------------------------------

var injectStartMarker = "// ${kInstData:Begin}\n";
var injectEndMarker = "// ${kInstData:End}\n";

// ----------------------------------------------------------------------------
// [Utilities]
// ----------------------------------------------------------------------------

var uppercaseFirst = function(s) {
  if (!s)
    return s;
  return s[0].toUpperCase() + s.substr(1);
};

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

// FullIndex - Index of the name of the instruction in one big string.

var Database = (function() {
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

  var Database = function() {
    this.map = {};
    this.alphabetical = new Array(26);
    this.fullString = new IndexedString();
  };

  Database.prototype.add = function(name, id) {
    this.map[name] = {
      id: id,
      fullIndex: 0,
      hasV: 0
    };
  };

  Database.prototype.index = function() {
    var map = this.map;
    var alphabetical = this.alphabetical;

    for (var name in map) {
      var inst = map[name];
      inst.fullIndex = this.fullString.add(name);

      var aIndex = name.charCodeAt(0) - 'a'.charCodeAt(0);
      if (aIndex < 0 || aIndex >= 26)
        throw new Error("Alphabetical index error");

      if (alphabetical[aIndex] === undefined)
        alphabetical[aIndex] = inst.id;

      if (name.indexOf("v") === 0) {
        inst.hasV = 1;
        name = name.substr(1);
      }
    }
  };

  return Database;
})();

// ----------------------------------------------------------------------------
// [Generate]
// ----------------------------------------------------------------------------

var generate = function(fileName, arch) {
  var oldData = fs.readFileSync(fileName, "utf8").replace(/\r\n/g, "\n");

  var data = oldData;
  var code = "";

  var Arch = uppercaseFirst(arch);

  // Create database.
  var db = new Database();
  var re = new RegExp("INST\\(([A-Za-z0-9_]+)\\s*,\\s*\\\"([A-Za-z0-9_ ]*)\\\"", "g");

  while (m = re.exec(data)) {
    var id = m[1];
    var name = m[2];

    db.add(name, id);
  }
  db.index();

  console.log("Full size: " + db.fullString.getSize());

  // Generate InstName[] string.
  code += "const char _instName[] =\n";
  for (var k in db.map) {
    var inst = db.map[k];
    code += "  \"" + k + "\\0\"\n";
  }
  code = code.substr(code, code.length - 1) + ";\n\n";

  // Generate AlphaIndex.
  code += "enum kInstAlphaIndex {\n";
  code += "  kInstAlphaIndexFirst = 'a',\n";
  code += "  kInstAlphaIndexLast = 'z',\n";
  code += "  kInstAlphaIndexInvalid = 0xFFFF\n";
  code += "};\n";
  code += "\n";

  // Generate NameIndex.
  code += "static const uint16_t _instAlphaIndex[26] = {\n";
  for (var i = 0; i < db.alphabetical.length; i++) {
    var id = db.alphabetical[i];
    code += "  " + (id === undefined ? "0xFFFF" : id);
    if (i !== db.alphabetical.length - 1)
      code += ",";
    code += "\n";
  }
  code += "};\n\n";

  code += "enum kInstData_NameIndex {\n";
  for (var k in db.map) {
    var inst = db.map[k];
    code += "  " + inst.id + "_NameIndex = " + inst.fullIndex + ",\n";
  }
  code = code.substr(code, code.length - 2) + "\n};\n";

  // Inject.
  data = inject(data, injectStartMarker, injectEndMarker, code);

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
