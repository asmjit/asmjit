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
      if (stat.isSymbolicLink()) {
        continue;
      }

      if (stat.isDirectory()) {
        subarray = listPrivate(subarray,
          path.join(dir, baseName), displayDir ? displayDir + "/" + baseName : baseName, accept);
        continue;
      }

      if (stat.isFile()) {
        if (accept(baseName))
          array.push({ name: fullName, display: displayDir ? displayDir + "/" + baseName : baseName });
        continue;
      }
    }

    return array.concat(subarray);
  };

  return function(dir, accept) {
    return listPrivate([], dir, "", accept);
  };
})();

/**
 * Inject data into string.
 */
var inject = function(s, start, end, what) {
  assert(start <= s.length);
  assert(end <= s.length);

  return s.substr(0, start) + what + s.substr(end);
};

/**
 * Is the extension c++ header file?
 */
var isCppHeaderExt = function(ext) {
  return ext === ".h"     ||
         ext === ".hh"    ||
         ext === ".hpp"   ||
         ext === ".hxx"   ;
};

/**
 * Is the extension c++ source file?
 */
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

/**
 * Sanity spaces.
 */
var sanitySpaces = function(data, name) {
  // Remove carriage return.
  data = data.replace(/\r\n/g, "\n");
  // Remove spaces before the end of the line.
  data = data.replace(/[ \t]+\n/g, "\n");
  // Convert tabs to spaces.
  data = data.replace(/\t/g, "  ");

  return data;
};

/**
 * Sanity header guards.
 */
var sanityHeaderGuards = (function() {
  var parseGuardName = function(data, i) {
    var m = data.substr(i).match(/[\w][\d\w]*/);
    return m ? m[0] : null;
  };

  var makeGuardName = function(name) {
    // Remove leading '/' or '\'.
    if (/^[\\\/]/.test(name))
      name = name.substr(1);
    return "_" + name.toUpperCase().replace(/[\/\\\.-]/g, "_");
  };

  var directiveMarks = [
    "#ifndef ",
    "#endif // ",
    "#define "
  ];

  var directiveNames = [
    "#ifndef ",
    "#endif  ",
    "#define "
  ];

  return function(data, name) {
    var i = 0;
    var nl = true;

    var guard = "// " + "[Guard]" + "\n";
    var nFound = 0;

    while (i < data.length) {
      if (nl && data.substr(i, guard.length) === guard) {
        i += guard.length;
        nFound++;

        if (i >= data.length)
          break;

        for (var j = 0; j < directiveMarks.length; ) {
          var m = directiveMarks[j];
          if (data.substr(i, m.length) === m && data.charAt(i + m.length) === '_') {
            i += directiveMarks[j].length;

            var oldGuardName = parseGuardName(data, i);
            var newGuardName;

            if (oldGuardName) {
              var startPosition = i;
              var endPosition = i + oldGuardName.length;

              newGuardName = makeGuardName(name);
              if (oldGuardName !== newGuardName) {
                console.log(name + ": " + directiveNames[j] + newGuardName);
                data = inject(data, startPosition, endPosition, newGuardName);

                i += newGuardName.length;
                i = data.indexOf('\n', i);

                if (i === -1) {
                  // Terminates the loop.
                  i = data.length;
                  j = 9999;
                  nl = false;
                  break;
                }
                else {
                  i++;
                }
              }
            }
            j += 2;
          }
          // Don't process '#define' directive if previous '#ifndef' wasn't matched.
          else {
            if (++j == 2)
              break;
          }
        }
      }
      else {
        nl = data.charAt(i) === '\n';
        i++;
      }
    }

    if (nFound & 1) {
      console.log(name + ": Odd number of guards found: " + nFound);
    }

    return data;
  };
})();

/**
 * Sanity #include order.
 */
var sanityIncludeOrder = function(data, name, directive) {
  var i = 0;
  var nl = true;

  var startPosition = -1;
  var endPosition = -1;
  var list = null;
  var replacement;
  
  while (i < data.length) {
    if (nl && data.substr(i, directive.length) === directive) {
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
        if (data.charAt(i) === '\n') {
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
        replacement = list.join("\n") + "\n";

        assert(replacement.length == endPosition - startPosition);
        data = inject(data, startPosition, endPosition, replacement);
      }

      startPosition = -1;
      endPosition = -1;
      list = null;

      nl = false;
      i++;
    }
    else {
      nl = data.charAt(i) === '\n';
      i++;
    }
  }

  return data;
};

/**
 * Sanity the given data of file.
 */
var sanity = function(data, name) {
  var ext = path.extname(name).toLowerCase();

  // Sanity spaces.
  data = sanitySpaces(data, name);
  
  // Fix C/C++ header guards and sort '#include' files.
  if (isCppHeaderExt(ext)) {
    data = sanityHeaderGuards(data, name);
    data = sanityIncludeOrder(data, name, "#include");
  }

  return data;
};

/**
 * Entry.
 */
var main = function(dir) {
  filesToSanitize(dir, filesToAccept).forEach(function(file) {
    var oldData = fs.readFileSync(file.name, "utf8");
    var newData = sanity(oldData, file.display);

    if (oldData !== newData) {
      console.log(file.display + ": Writing...");
      fs.writeFileSync(file.name, newData, "utf8");
    }
  });
};

main(path.join(__dirname, "../src"));
