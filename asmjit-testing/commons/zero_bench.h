// ZeroTest - Lightweight and embeddable benchmarking for C++
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org>

// ZeroBench is a minimalist framework for benchmarking with the following goals:
//
//   - Zero dependencies, easy to embed in most projects.
//   - Minimalist interface and features.

#include "zero_lib.h"

#include <chrono>
#include <type_traits>

#ifndef ZERO_BENCH_H_INCLUDED
#define ZERO_BENCH_H_INCLUDED

namespace ZeroBench {

using namespace ZeroLib;

// Benchmark Constants
// ===================

enum class ValueKind : uint8_t {
  kLabel,
  kFailure,

  kUnknown,
  kDurationNano,  // Duration in nano-seconds (the standard duration measured by the engine).
  kDurationPico,  // Duration in pico-seconds so the output can have a sub-nano second precision.
  kCount,
  kBytes,
  kPixels,

  kDuration = kDurationNano
};

enum class Aggregate : uint8_t {
  kStatic,        // Static means it's expected that each sample is the same (like counters, etc...)
  kMin,           // Minimum of all values.
  kMax,           // Maximum of all values.
  kMean,          // Average of all values.
  kP50,           // Median, uses average of two values if the result-set is even.
  kP90,
  kP95,
  kP99,
  kVariance,      // Population variance, in squared source units.
  kMAD,           // Median absolute deviation from the median, without a scale factor.
  kGeometricMean, // Zero if any sample is zero.
  kCV,            // Population standard deviation / mean, displayed as a percentage (zero if the mean is zero).
  kStdDev         // Population standard deviation, in source units.
};

enum class Scale {
  kDefault,       // Default, based on metric-kind and the value itself.
  kDefaultBytes,  // Default, based on metric-kind and the value itself.

  kNone,          // No scale applied.

  kNano,          // 10^-9
  kMicro,         // 10^-6
  kMilli,         // 10^-3

  kKilo,          // 10^3
  kMega,          // 10^6
  kGiga,          // 10^9

  kKiB,           // 2^10
  kMiB,           // 2^20
  kGiB            // 2^30
};

static constexpr uint32_t kMaxLabelCount = 8;
static constexpr uint32_t kMaxValueCount = 32;
static constexpr uint32_t kMaxMetricCount = 32;

static constexpr Color kDefaultLabelColor = Color::kNone;
static constexpr Color kDefaultMetricColor = Color::kBrightBlue;

static constexpr Color kMinDurationColor = Color::kBrightGreen;
static constexpr Color kMaxDurationColor = Color::kBrightRed;
static constexpr Color kOtherDurationColor = Color::kBrightYellow;
static constexpr Color kOtherStatisticColor = Color::kBrightMagenta;

static constexpr uint32_t kDurationValueIndex = kMaxValueCount - 1u;
static constexpr uint32_t kIterCountValueIndex = kMaxValueCount - 2u;

// Benchmark Structs
// =================

struct MetricOptions {
  Color color;
  ValueKind value_kind;
  uint8_t value_index;
  Aggregate aggregate;
  Scale scale;
};

// Benchmark Timer
// ===============

struct Timer {
  std::chrono::steady_clock::time_point _start {};

  inline void start() noexcept {
    _start = std::chrono::steady_clock::now();
  }

  inline uint64_t elapsed() noexcept {
    auto end = std::chrono::steady_clock::now();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - _start).count());
  }
};

// Benchmark Context
// =================

struct ResultValues {
  uint64_t data[kMaxValueCount];
};

//! Context is passed to benchmarks.
struct Context {
  std::chrono::steady_clock::time_point _start {};
  std::chrono::steady_clock::time_point _end {};

  ResultValues& _values;
  String& _failure_message;
  bool _failed = false;

  inline Context(ResultValues& values, String& failure_message) noexcept
    : _values(values),
      _failure_message(failure_message) {}

  inline void start_now() {
    _start = std::chrono::steady_clock::now();
    _end = _start;
  }

  inline void end_now() {
    _end = std::chrono::steady_clock::now();
  }

  inline uint64_t elapsed_nano() const {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(_end - _start).count());
  }

  inline uint64_t value(uint32_t id) const noexcept {
    return (id < kMaxValueCount) ? _values.data[id] : uint64_t(0);
  }

  inline void set_value(uint32_t id, uint64_t v) noexcept {
    if (id < kMaxValueCount) {
      _values.data[id] = v;
    }
  }

  inline void make_failed() noexcept { _failed = true; }
  inline void make_failed(const char* message) noexcept {
    _failed = true;
    _failure_message.assign(message);
  }
};

// Benchmark Interface
// ===================

//! A single named benchmark.
class Benchmark {
public:
  String _labels[kMaxLabelCount];

  Benchmark() noexcept;
  virtual ~Benchmark() noexcept;

  virtual void prepare(Context& ctx);
  virtual void run(Context& ctx);
  virtual void teardown();

  inline const String& name() const noexcept { return _labels[0]; }
  inline const String& label(uint32_t label_id) const noexcept { return _labels[label_id]; }

  void set_label(uint32_t id, const char* str);
  void set_label(uint32_t id, const char* str, size_t size);
  inline void set_label(uint32_t id, const String& str) { set_label(id, str.data(), str.size()); }
};

class SimpleBenchmark : public Benchmark {
public:
  //! A benchmarking function - takes no parameters and returns no value.
  using EntryFunc = void (*)(Context& ctx);

  EntryFunc _entry_func {};

  SimpleBenchmark(const char* name, EntryFunc func) noexcept;
  ~SimpleBenchmark() noexcept override;

  void run(Context& ctx) override;
};

class BenchmarkRef {
public:
  Benchmark* _ptr;

  BenchmarkRef(const BenchmarkRef& other) noexcept = delete;
  inline BenchmarkRef& operator=(const BenchmarkRef& other) noexcept = delete;

  explicit inline BenchmarkRef(Benchmark* ptr) noexcept
    : _ptr(ptr) {}

  inline BenchmarkRef(BenchmarkRef&& other) noexcept {
    Benchmark* p = other._ptr;
    other._ptr = nullptr;
    _ptr = p;
  }

  template<typename T, typename... Args>
  static inline BenchmarkRef make(Args&&... args) {
    return BenchmarkRef(new T(forward<Args>(args)...));
  }

  inline ~BenchmarkRef() noexcept {
    if (_ptr) {
      delete _ptr;
    }
  }

  inline BenchmarkRef& operator=(BenchmarkRef&& other) noexcept {
    Benchmark* p = other._ptr;
    other._ptr = nullptr;
    _ptr = p;
    return *this;
  }

  inline Benchmark& operator*() const noexcept { return *_ptr; }
  inline Benchmark* operator->() const noexcept { return _ptr; }

  inline void swap(BenchmarkRef& other) noexcept {
    Benchmark* p = other._ptr;
    other._ptr = _ptr;
    _ptr = p;
  }

  inline Benchmark* ptr() const noexcept { return _ptr; }
};

#define ZERO_BENCH_RECORD(...) ::ZeroBench::BenchmarkRef::make<::ZeroBench::SimpleBenchmark>(#__VA_ARGS__, __VA_ARGS__)

// Benchmark Runner
// ================

class Runner {
public:
  CmdLine _cmd_line;
  uint32_t _iteration_count {};
  uint32_t _max_iteration_count {};
  uint32_t _warm_up_iterations {};
  uint64_t _warm_up_nanoseconds {};

  String _run_filter;
  bool _use_timings {};

  struct Label {
    String name;
  };

  struct Metric {
    String name;
    MetricOptions options;
  };

  Label _labels[kMaxLabelCount] {};
  Metric _metrics[kMaxMetricCount] {};

  uint32_t _labels_mask = 0u;
  uint32_t _metrics_mask = 0u;

  Runner(int argc, const char* const* argv) noexcept;

  const CmdLine& cmd_line() const noexcept { return _cmd_line; }

  void set_label(uint32_t label_id, const char* name) noexcept;
  void set_label(uint32_t label_id, const String& name) noexcept;
  void set_metric(uint32_t metric_id, const char* name, const MetricOptions& options) noexcept;
  void set_metric(uint32_t metric_id, const String& name, const MetricOptions& options) noexcept;

  //! Adds a metric. Distribution statistics (P90 through StdDev) have no combined value in the Total row.
  uint32_t add_metric(const char* name, const MetricOptions& options) noexcept;
  uint32_t add_metric(const String& name, const MetricOptions& options) noexcept;
  void add_time_metrics() noexcept;

  inline bool is_label_enabled(uint32_t label_id) const noexcept { return (_labels_mask & (1u << label_id)) != 0u; }
  inline bool is_metric_enabled(uint32_t metric_id) const noexcept { return (_metrics_mask & (1u << metric_id)) != 0u; }

  //! Benchmarks multiple functions and prints their time.
  int run(const BenchmarkRef* benchmarks, size_t benchmark_count);

  //! Benchmarks multiple functions and prints their time (array version).
  template<size_t N>
  inline int run(const BenchmarkRef (&benchmarks)[N]) { return run(benchmarks, N); }
};

} // {ZeroBench}

#endif // ZERO_BENCH_H_INCLUDED
