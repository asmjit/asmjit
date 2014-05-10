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

var injectStartMarker = "// ${kInstData:Begin}\n"
var injectEndMarker = "// ${kInstData:End}\n"

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

// FullIndex   - Index of the name of the instruction in one big string (no
//               prefix/suffix concept).
// PrefixIndex - Index to a prefix string.
// SuffixIndex - Index to a suffix string.

var Database = (function() {
  function bestSuffix(s, suffixes) {
    var best = -1;
 
    for (var i = 0; i < suffixes.length; i++) {
      var suffix = suffixes[i];
      var si = s.lastIndexOf(suffix);

      if (si === -1 || si + suffix.length != s.length)
        continue;

      if (best !== -1 && suffix.length < suffixes[best].length)
        continue;

      best = i;
    }
    
    return best;
  }

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

  var Database = function(suffixes) {
    this.map = {};
    this.suffixes = suffixes;

    this.fullString = new IndexedString();
    this.prefixString = new IndexedString();
    this.suffixString = new IndexedString();
  };

  Database.prototype.add = function(name, id) {
    this.map[name] = {
      id: id,
      fullIndex: 0,
      prefixIndex: 0,
      suffixIndex: 0,
      hasV: 0
    };
  };

  Database.prototype.index = function() {
    var map = this.map;
    var suffixes = this.suffixes;

    for (var i = 0; i < suffixes.length; i++) {
      this.suffixString.add(suffixes[i]);
    }

    for (var name in map) {
      var inst = map[name];
      var si = bestSuffix(name, suffixes);

      inst.fullIndex = this.fullString.add(name);

      if (name.indexOf("v") === 0) {
        inst.hasV = 1;
        name = name.substr(1);
      }

      if (si !== -1) {
        var suffix = suffixes[si];
        var prefix = name.substr(0, name.length - suffix.length);

        inst.prefixIndex = this.prefixString.add(prefix);
        inst.suffixIndex = this.suffixString.add(suffix);
      }
      else {
        inst.prefixIndex = this.prefixString.add(name);
        inst.suffixIndex = this.suffixString.add("");
      }
    }
  };

  return Database;
})();

// ----------------------------------------------------------------------------
// [Generate]
// ----------------------------------------------------------------------------

var generate = function(fileName, arch, suffixes) {
  var oldData = fs.readFileSync(fileName, "utf8").replace(/\r\n/g, "\n");

  var data = oldData;
  var code = "";

  var Arch = uppercaseFirst(arch);

  // Create database.
  var db = new Database(suffixes);
  var re = new RegExp("INST\\(([A-Za-z0-9_]+)\\s*,\\s*\\\"([A-Za-z0-9_ ]*)\\\"", "g");

  while (m = re.exec(data)) {
    var id = m[1];
    var name = m[2];
    
    db.add(name, id);
  }
  db.index();

  console.log("Full size: " + db.fullString.getSize());
  console.log("Prefix size: " + db.prefixString.getSize());
  console.log("Suffix size: " + db.suffixString.getSize());

  // Generate InstName[] string.
  code += "const char _instName[] =\n";
  for (var k in db.map) {
    var inst = db.map[k];
    code += "  \"" + k + "\\0\"\n";
  }
  code = code.substr(code, code.length - 1) + ";\n\n";

  // Generate NameIndex.
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
    generate(file.name, file.arch, file.suffixes);
  });
};

main([
  {
    name: "../src/asmjit/x86/x86inst.cpp",
    arch: "x86",
    suffixes: [
      "a", "ae",
      "b", "bd", "be", "bq", "bw",
      "c",
      "d", "dq", "dqa", "dqu", "dw",
      "e",
      "f128",
      "g", "ge",
      "hpd", "hps",
      "i", "i128", "ip",
      "l", "last", "ld", "le", "lpd", "lps", "lw",
      "na", "nae", "nb", "nbe", "nc", "ne", "ng", "nge", "nl", "nle", "no", "np", "ns", "nz",
      "o",
      "p", "pd", "pe", "ph", "pi", "po", "pp", "ps",
      "q",
      "r",
      "s", "sb", "sd", "si", "sq", "ss", "sw",
      "usb", "usw",
      "vpd", "vps",
      "w", "wb", "wd", "wq",
      "z"
    ]
  }
]);
