var assert = require("assert");
var fs = require("fs");
var path = require("path");

/**
 * List all files that can be processed by sanitizer in the given directory.
 */
var filesToSanitize = (function() {
  var listPrivate = function(array, dir, displayDir, accept) {
    var files = fs.readdirSync(dir);
    var subarray = [];

    for (var i = 0; i < files.length; i++) {
      var baseName = files[i];
      var fullName = path.normalize(path.join(dir, baseName));

      var stat = fs.lstatSync(fullName);
      if (stat.isSymbolicLink())
        continue;

      if (stat.isDirectory()) {
        listPrivate(subarray, path.join(dir, baseName), displayDir + baseName, accept);
        continue;
      }

      if (stat.isFile()) {
        if (accept(baseName))
          array.push({ name: fullName, display: displayDir + baseName });
        continue;
      }
    }

    return array.concat(subarray);
  };

  return function(dir, accept) {
    return listPrivate([], dir, "", accept);
  };
})();

var isCppHeaderExt = function(ext) {
  return ext === ".h"     ||
         ext === ".hh"    ||
         ext === ".hpp"   ||
         ext === ".hxx"   ;
};

var isCppSourceExt = function(ext) {
  return ext === ".c"     ||
         ext === ".cc"    ||
         ext === ".cpp"   ||
         ext === ".cxx"   ;
};

/**
 * Filter that returns true if the given file name should be processed.
 */
var filesToAccept = function(name) {
  var ext = path.extname(name).toLowerCase();

  return isCppHeaderExt(ext) ||
         isCppSourceExt(ext) ||
         ext === ".cmake"    ||
         ext === ".m"        ||
         ext === ".md"       ||
         ext === ".mm"       ;
};

var sanitySpaces = function(data) {
  // Remove carriage return.
  data = data.replace(/\r\n/g, "\n");
  // Remove spaces before the end of the line.
  data = data.replace(/[ \t]+\n/g, "\n");
  // Convert tabs to spaces.
  data = data.replace(/\t/g, "  ");

  return data;
};

var sanityHeaderGuards = function(data) {

  return data;
};

var sanityIncludeOrder = function(data, directive) {
  var i = 0;
  var nl = true;

  var startPosition = -1;
  var endPosition = -1;
  var list = null;
  var replacement;
  
  while (i < data.length) {
    if (nl && data.indexOf(directive, i) === i) {
      var iLocal = i

      if (startPosition === -1) {
        startPosition = i;
        list = [];
      }

      for (;;) {
        if (++i >= data.length) {
          list.push(data.substring(iLocal, i));
          break;
        }
        if (data[i] === '\n') {
          list.push(data.substring(iLocal, i));
          i++;
          break;
        }
      }
    }
    else if (startPosition !== -1) {
      assert(nl === true);
      endPosition = i;

      if (list.length > 1) {
        list.sort();
        replacement = list.join("\n");
        assert(replacement.length == endPosition - startPosition - 1);

        data = data.substring(0, startPosition) + 
               replacement +
               "\n" +
               data.substring(endPosition);
      }

      startPosition = -1;
      endPosition = -1;
      list = null;

      nl = false;
      i++;
    }
    else {
      nl = data[i] === '\n';
      i++;
    }
  }

  return data;
};

var sanity = function(data, name) {
  var ext = path.extname(name).toLowerCase();

  // Sanity spaces.
  data = sanitySpaces(data);
  
  // Fix C/C++ header guards.
  if (isCppHeaderExt(ext)) {
    data = sanityHeaderGuards(data);
  }

  // Sort #include files.
  if (isCppHeaderExt(ext) || isCppSourceExt(ext)) {
    data = sanityIncludeOrder(data, "#include");
  }

  return data;
};

var main = function(dir) {
  filesToSanitize(dir, filesToAccept).forEach(function(file) {
    var oldData = fs.readFileSync(file.name, "utf8");
    var newData = sanity(oldData, file.display);

    if (oldData !== newData) {
      console.log("Sanitizing: " + file.display);
      fs.writeFileSync(file.name, newData, "utf8");
    }
  });
};

main(path.join(__dirname, "../src"));
