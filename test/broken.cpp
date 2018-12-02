// [Broken]
// Lightweight Unit Testing for C++.
//
// [License]
// Public Domain (Unlicense)

#include "./broken.h"
#include <stdarg.h>

// ============================================================================
// [Broken - Global]
// ============================================================================

// Zero initialized globals.
struct BrokenGlobal {
  // Command line arguments array.
  const char** _argv;
  // Command line argument count.
  size_t _argc;

  // Output file.
  FILE* _file;

  // Unit tests.
  BrokenAPI::Unit* _unitList;
  BrokenAPI::Unit* _unitRunning;

  bool hasArg(const char* name) const noexcept {
    for (size_t i = 1; i < _argc; i++)
      if (strcmp(_argv[i], name) == 0)
        return true;
    return false;
  }

  inline FILE* file() const noexcept { return _file ? _file : stdout; }
};
static BrokenGlobal _brokenGlobal;

// ============================================================================
// [Broken - API]
// ============================================================================

// Get whether the string `a` starts with string `b`.
static bool BrokenAPI_startsWith(const char* a, const char* b) noexcept {
  for (size_t i = 0; ; i++) {
    if (b[i] == '\0') return true;
    if (a[i] != b[i]) return false;
  }
}

// Get whether the strings `a` and `b` are equal, ignoring case and treating
// `-` as `_`.
static bool BrokenAPI_matchesFilter(const char* a, const char* b) noexcept {
  for (size_t i = 0; ; i++) {
    unsigned char ca = static_cast<unsigned char>(a[i]);
    unsigned char cb = static_cast<unsigned char>(b[i]);

    // If filter is defined as wildcard the rest automatically matches.
    if (cb == '*')
      return true;

    if (ca == '-') ca = '_';
    if (cb == '-') cb = '_';

    if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca + (unsigned char)('a' - 'A'));
    if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb + (unsigned char)('a' - 'A'));

    if (ca != cb)
      return false;

    if (ca == '\0')
      return true;
  }
}

static bool BrokenAPI_canRun(BrokenAPI::Unit* unit) noexcept {
  BrokenGlobal& global = _brokenGlobal;

  size_t i, argc = global._argc;
  const char** argv = global._argv;

  const char* unitName = unit->name;
  bool hasFilter = false;

  for (i = 1; i < argc; i++) {
    const char* arg = argv[i];

    if (BrokenAPI_startsWith(arg, "--run-") && strcmp(arg, "--run-all") != 0) {
      hasFilter = true;

      if (BrokenAPI_matchesFilter(unitName, arg + 6))
        return true;
    }
  }

  // If no filter has been specified the default is to run.
  return !hasFilter;
}

static void BrokenAPI_runUnit(BrokenAPI::Unit* unit) noexcept {
  BrokenAPI::info("Running %s", unit->name);

  _brokenGlobal._unitRunning = unit;
  unit->entry();
  _brokenGlobal._unitRunning = NULL;
}

static void BrokenAPI_runAll() noexcept {
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

static void BrokenAPI_listAll() noexcept {
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

bool BrokenAPI::hasArg(const char* name) noexcept {
  return _brokenGlobal.hasArg(name);
}

void BrokenAPI::add(Unit* unit) noexcept {
  Unit** pPrev = &_brokenGlobal._unitList;
  Unit* current = *pPrev;

  // C++ static initialization doesn't guarantee anything. We sort all units by
  // name so the execution will always happen in deterministic order.
  while (current != NULL) {
    if (strcmp(current->name, unit->name) >= 0)
      break;

    pPrev = &current->next;
    current = *pPrev;
  }

  *pPrev = unit;
  unit->next = current;
}

void BrokenAPI::setOutputFile(FILE* file) noexcept {
  BrokenGlobal& global = _brokenGlobal;

  global._file = file;
}

int BrokenAPI::run(int argc, const char* argv[], Entry onBeforeRun, Entry onAfterRun) noexcept {
  BrokenGlobal& global = _brokenGlobal;

  global._argc = unsigned(argc);
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

void BrokenAPI::info(const char* fmt, ...) noexcept {
  BrokenGlobal& global = _brokenGlobal;
  FILE* dst = global.file();

  if (!fmt) fmt = "";
  size_t size = strlen(fmt);

  const char* prefix = global._unitRunning ? "  " : "";
  if (size != 0) {
    va_list ap;
    va_start(ap, fmt);
    fputs(prefix, dst);
    vfprintf(dst, fmt, ap);
    va_end(ap);
  }

  if (size == 0 || fmt[size - 1] != '\n')
    fputs("\n", dst);

  fflush(dst);
}

void BrokenAPI::fail(const char* file, int line, const char* fmt, ...) noexcept {
  BrokenGlobal& global = _brokenGlobal;
  FILE* dst = global.file();

  if (!fmt) fmt = "";
  size_t size = strlen(fmt);

  fputs("  Failed!", dst);
  if (size != 0) {
    va_list ap;
    va_start(ap, fmt);
    fputs(" ", dst);
    vfprintf(dst, fmt, ap);
    va_end(ap);
  }

  if (size > 0 && fmt[size - 1] != '\n')
    fputs("\n", dst);

  fprintf(dst, "  File: %s (Line: %d)\n", file, line);
  fflush(dst);

  exit(1);
}
