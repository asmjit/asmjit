// [Broken]
// Lightweight Unit Testing for C++.
//
// [License]
// Public Domain (Unlicense)

// [Dependencies - Broken]
#include "./broken.h"

// ============================================================================
// [Broken - Global]
// ============================================================================

// Zero initialized globals.
struct BrokenGlobal {
  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  bool hasArg(const char* a) const {
    int argc = _argc;
    const char** argv = _argv;

    for (int i = 1; i < argc; i++) {
      if (::strcmp(argv[i], a) == 0)
        return true;
    }

    return false;
  }

  FILE* getFile() const {
    return _file ? _file : stdout;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  // Application arguments.
  int _argc;
  const char** _argv;

  // Output file.
  FILE* _file;

  // Current context.
  const char* _currentFile;
  int _currentLine;

  // Unit tests.
  BrokenAPI::Unit* _unitList;
  BrokenAPI::Unit* _unitRunning;
};
static BrokenGlobal _brokenGlobal;

// ============================================================================
// [Broken - API]
// ============================================================================

// Get whether the string `a` starts with string `b`.
static bool BrokenAPI_startsWith(const char* a, const char* b) {
  for (size_t i = 0; ; i++) {
    if (b[i] == '\0') return true;
    if (a[i] != b[i]) return false;
  }
}

// Get whether the strings `a` and `b` are equal, ignoring case and treating
// `-` as `_`.
static bool BrokenAPI_matchesFilter(const char* a, const char* b) {
  for (size_t i = 0; ; i++) {
    unsigned char ca = static_cast<unsigned char>(a[i]);
    unsigned char cb = static_cast<unsigned char>(b[i]);

    // If filter is defined as wildcard the rest automatically matches.
    if (cb == '*')
      return true;

    if (ca == '-') ca = '_';
    if (cb == '-') cb = '_';

    if (ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
    if (cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';

    if (ca != cb)
      return false;

    if (ca == '\0')
      return true;
  }
}

static bool BrokenAPI_canRun(BrokenAPI::Unit* unit) {
  BrokenGlobal& global = _brokenGlobal;

  int i, argc = global._argc;
  const char** argv = global._argv;

  const char* unitName = unit->name;
  bool hasFilter = false;

  for (i = 1; i < argc; i++) {
    const char* arg = argv[i];

    if (BrokenAPI_startsWith(arg, "--run-") && ::strcmp(arg, "--run-all") != 0) {
      hasFilter = true;

      if (BrokenAPI_matchesFilter(unitName, arg + 6))
        return true;
    }
  }

  // If no filter has been specified the default is to run.
  return !hasFilter;
}

static void BrokenAPI_runUnit(BrokenAPI::Unit* unit) {
  BrokenAPI::info("Running %s", unit->name);

  _brokenGlobal._unitRunning = unit;
  unit->entry();
  _brokenGlobal._unitRunning = NULL;
}

static void BrokenAPI_runAll() {
  BrokenAPI::Unit* unit = _brokenGlobal._unitList;

  bool hasUnits = unit != NULL;
  size_t count = 0;

  while (unit != NULL) {
    if (BrokenAPI_canRun(unit)) {
      BrokenAPI_runUnit(unit);
      count++;
    }
    unit = unit->next;
  }

  if (count) {
    INFO("\nSuccess:");
    INFO("  All tests passed!");
  }
  else {
    INFO("\nWarning:");
    INFO("  No units %s!", hasUnits ? "matched the filter" : "defined");
  }
}

static void BrokenAPI_listAll() {
  BrokenAPI::Unit* unit = _brokenGlobal._unitList;

  if (unit != NULL) {
    INFO("Units:");
    do {
      INFO("  %s", unit->name);
      unit = unit->next;
    } while (unit != NULL);
  }
  else {
    INFO("Warning:");
    INFO("  No units defined!");
  }
}

void BrokenAPI::add(Unit* unit) {
  Unit** pPrev = &_brokenGlobal._unitList;
  Unit* current = *pPrev;

  // C++ static initialization doesn't guarantee anything. We sort all units by
  // name so the execution will always happen in deterministic order.
  while (current != NULL) {
    if (::strcmp(current->name, unit->name) >= 0)
      break;

    pPrev = &current->next;
    current = *pPrev;
  }

  *pPrev = unit;
  unit->next = current;
}

void BrokenAPI::setOutputFile(FILE* file) {
  BrokenGlobal& global = _brokenGlobal;

  global._file = file;
}

int BrokenAPI::setContext(const char* file, int line) {
  BrokenGlobal& global = _brokenGlobal;

  global._currentFile = file;
  global._currentLine = line;

  return 1;
}

int BrokenAPI::run(int argc, const char* argv[],
  Entry onBeforeRun,
  Entry onAfterRun) {

  BrokenGlobal& global = _brokenGlobal;

  global._argc = argc;
  global._argv = argv;

  if (global.hasArg("--help")) {
    INFO("Options:");
    INFO("  --help    - print this usage");
    INFO("  --list    - list all tests");
    INFO("  --run-... - run a test(s), trailing wildcards supported");
    INFO("  --run-all - run all tests");
    return 0;
  }

  if (global.hasArg("--list")) {
    BrokenAPI_listAll();
    return 0;
  }

  if (onBeforeRun)
    onBeforeRun();

  // We don't care about filters here, it's implemented by `runAll`.
  BrokenAPI_runAll();

  if (onAfterRun)
    onAfterRun();

  return 0;
}

int BrokenAPI::info(const char* fmt, ...) {
  BrokenGlobal& global = _brokenGlobal;
  FILE* dst = global.getFile();

  const char* prefix = global._unitRunning ? "  " : "";
  size_t len = ::strlen(fmt);

  if (len != 0) {
    va_list ap;
    va_start(ap, fmt);
    ::fputs(prefix, dst);
    ::vfprintf(dst, fmt, ap);
    va_end(ap);
  }

  if (len == 0 || fmt[len - 1] != '\n')
    ::fputs("\n", dst);

  ::fflush(dst);
  return 1;
}

int BrokenAPI::fail(const char* fmt, va_list ap) {
  BrokenGlobal& global = _brokenGlobal;
  FILE* dst = global.getFile();

  ::fputs("  Failed!", dst);
  if (fmt == NULL)
    fmt = "";

  size_t len = ::strlen(fmt);
  if (len != 0) {
    ::fputs(" ", dst);
    ::vfprintf(dst, fmt, ap);
  }

  if (len > 0 && fmt[len - 1] != '\n')
    ::fputs("\n", dst);

  ::fprintf(dst, "  File: %s (Line: %d)\n", global._currentFile, global._currentLine);
  ::fflush(dst);

  ::exit(1);
  return 1;
}
