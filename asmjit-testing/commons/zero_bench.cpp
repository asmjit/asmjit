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

#include "zero_bench.h"
#include <string.h>

#include <algorithm>
#include <bit>
#include <cmath>

namespace ZeroBench {

// ExactArray
// ----------

template<typename T>
struct ExactArray {
  T* _values {};
  size_t _size {};

  inline ExactArray() noexcept {}
  inline ~ExactArray() noexcept { release(); }

  inline const T* data() const noexcept { return _values; }
  inline T* data_mut() noexcept { return _values; }

  bool allocate(size_t size) noexcept {
    _values = static_cast<uint64_t*>(calloc(size, sizeof(T)));
    if (_values == nullptr) {
      return false;
    }
    _size = size;
    return true;
  }

  void release() noexcept {
    if (_values) {
      free(_values);
      _values = nullptr;
      _size = 0u;
    }
  }
};

// Statistics
// ----------

struct AggregatedValue {
  uint64_t min;
  uint64_t max;
  uint64_t avg;
  uint64_t p50;
  uint64_t p90;
  uint64_t p95;
  uint64_t p99;

  double variance;
  double stddev;
  double mad;
  double geometric_mean;
  double cv;

  void add(const AggregatedValue& other) noexcept {
    min += other.min;
    max += other.max;
    avg += other.avg;
    p50 += other.p50;
    p90 += other.p90;
    p95 += other.p95;
    p99 += other.p99;
  }
};

struct Results {
  AggregatedValue values[kMaxValueCount];
  String failure_message;

  void add(const Results& other) noexcept {
    for (size_t i = 0; i < kMaxValueCount; i++) {
      values[i].add(other.values[i]);
    }
  }
};

static size_t calculate_repeat_count(uint64_t t_nano) noexcept {
  uint64_t ps = uint64_t(150000000) / std::max<uint64_t>(t_nano, 10u);
  return std::max<size_t>(size_t(ps), 3u);
}

static uint64_t calculate_mean(const uint64_t* values, size_t count) noexcept {
  uint64_t n = static_cast<uint64_t>(count);

  uint64_t avg = 0u;
  uint64_t rem = 0u;
  uint64_t sum = 0u;

  auto accumulate = [&]() noexcept {
    avg += sum / n;
    rem += sum % n;

    if (rem >= n) {
      avg++;
      rem -= n;
    }
  };

  for (size_t i = 0; i < count; i++) {
    uint64_t v = values[i];
    uint64_t x = sum + v;

    if (x < sum) {
      // Overflow.
      accumulate();
      sum = v;
    }
    else {
      sum = x;
    }
  }

  accumulate();
  return avg;
}

static uint64_t avg_u64(uint64_t a, uint64_t b) noexcept { return (a & b) + ((a ^ b) >> 1); }

static uint64_t calculate_percentile(const uint64_t* values, size_t count, uint32_t percent) noexcept {
  // Split both products into quotient and remainder to avoid overflow and floating-point rounding.
  size_t position = count - 1u;
  size_t remainder = (position % 100u) * percent;
  size_t index = (position / 100u) * percent + remainder / 100u;
  size_t next = std::min(index + 1u, count - 1u);

  uint64_t fraction = uint64_t(remainder % 100u);
  uint64_t delta = values[next] - values[index];

  return values[index] + (delta / 100u) * fraction + ((delta % 100u) * fraction) / 100u;
}

static void calculate_statistics_from_values(uint64_t* values, size_t count, AggregatedValue& out) noexcept {
  out = {};
  if (!count) {
    return;
  }

  std::sort(values, values + count, [](const uint64_t& a, const uint64_t& b) -> bool { return a < b; });

  out.min = values[0];
  out.max = values[count - 1u];

  if (out.min == out.max) {
    out.avg = out.min;
    out.p50 = out.min;
    out.p90 = out.min;
    out.p95 = out.min;
    out.p99 = out.min;
    out.geometric_mean = double(out.min);
    return;
  }

  out.avg = calculate_mean(values, count);
  out.p50 = values[count / 2u];

  bool half_median = false;
  if (count % 2u == 0) {
    uint64_t lower = values[count / 2u - 1u];
    half_median = ((lower ^ out.p50) & 1u) != 0;
    out.p50 = avg_u64(lower, out.p50);
  }

  out.p90 = calculate_percentile(values, count, 90u);
  out.p95 = calculate_percentile(values, count, 95u);
  out.p99 = calculate_percentile(values, count, 99u);

  // Subtract an integer origin before conversion to preserve small differences between large samples.
  double sum = 0.0;
  double log_sum = 0.0;
  for (size_t i = 0; i < count; i++) {
    sum += double(values[i] - out.min);
    if (out.min != 0u) {
      log_sum += std::log(double(values[i]));
    }
  }

  double mean_offset = sum / double(count);
  double squared_deviations = 0.0;
  for (size_t i = 0; i < count; i++) {
    double delta = double(values[i] - out.min) - mean_offset;
    squared_deviations += delta * delta;
  }

  out.variance = squared_deviations / double(count);
  out.stddev = std::sqrt(out.variance);
  out.geometric_mean = out.min != 0u ? std::exp(log_sum / double(count)) : 0.0;
  out.cv = out.stddev / (double(out.min) + mean_offset);

  // Reuse the samples for absolute deviations. If the true median has a half unit, all
  // deviations have the same half unit; keep their integer parts exact until after sorting.
  for (size_t i = 0; i < count; i++) {
    uint64_t v = values[i];
    values[i] = v <= out.p50 ? out.p50 - v : v - out.p50 - uint64_t(half_median);
  }
  std::sort(values, values + count);
  out.mad = double(values[count / 2u]);
  if (count % 2u == 0) {
    uint64_t lower = values[count / 2u - 1u];
    out.mad = double(lower) + double(values[count / 2u] - lower) * 0.5;
  }

  out.mad += half_median ? 0.5 : 0.0;
}

// Benchmark Single
// ----------------

static bool bench_single(Benchmark* benchmark, size_t iteration_count, size_t max_iteration_count, size_t warm_up_count, uint64_t warm_up_nanos, Results& results) {
  constexpr size_t kBatchCapacity = 8;
  ResultValues batched_results[kBatchCapacity] {};

  memset(results.values, 0, sizeof(results.values));

  {
    Context ctx(batched_results[0], results.failure_message);
    benchmark->prepare(ctx);
    if (ctx._failed) {
      return false;
    }
  }

  if (warm_up_count || warm_up_nanos) {
    size_t warm_up_iter = 0;
    for (;;) {
      Context ctx(batched_results[0], results.failure_message);
      ctx.start_now();
      benchmark->run(ctx);
      ctx.end_now();

      if (ctx._failed) {
        benchmark->teardown();
        return false;
      }

      warm_up_iter++;
      if (warm_up_count) {
        if (warm_up_iter >= warm_up_count) {
          break;
        }
      }

      if (warm_up_nanos) {
        uint64_t t = ctx.elapsed_nano();
        if (t < 200u) t = 200u; // If the precision is low, we don't want to get 0.
        if (t >= warm_up_nanos) {
          break;
        }
        warm_up_nanos -= t;
      }

      if (warm_up_iter > 100000) {
        // Stop if the number of iterations is too high already.
        break;
      }
    }
  }

  memset(batched_results, 0, sizeof(batched_results));

  bool success = true;
  size_t iteration = 0u;

  size_t batch_index = 0u;
  size_t result_position = 0u;
  ExactArray<uint64_t> result_array;

  do {
    // The benchmark can call it again in case that it needs to do some work before it can start.
    Context ctx(batched_results[batch_index], results.failure_message);

    ctx.start_now();
    benchmark->run(ctx);

    // We don't want to increase time in case the benchmark called end_now()
    // already, for example in case that there is an expensive tear-down.
    if (ctx._start == ctx._end) {
      ctx.end_now();
    }

    if (ctx._failed) {
      success = false;
      break;
    }

    uint64_t t_nano = ctx.elapsed_nano();
    batched_results[batch_index].data[kDurationValueIndex] = t_nano;

    // Repeat based on `t_nano`.
    if (iteration_count == 0u) {
      iteration_count = calculate_repeat_count(t_nano);
      if (max_iteration_count != 0 && iteration_count > max_iteration_count) {
        iteration_count = max_iteration_count;
      }
    }

    // Allocate results array after the first iteration ended. This would allocate enough to
    // store results from ALL benchmarks, so we can calculate exact statistics later.
    if (++iteration == 1u) {
      if (!result_array.allocate(iteration_count * kMaxValueCount)) {
        results.failure_message.append_format("Failed to allocate result-array");
        success = false;
        break;
      }
    }

    // Transpose results to the allocated `result_array`.
    if (++batch_index >= kBatchCapacity || iteration >= iteration_count) {
      for (size_t value_index = 0; value_index < kMaxValueCount; value_index++) {
        size_t result_array_index = value_index * iteration_count + result_position;
        for (size_t batch_row = 0; batch_row < batch_index; batch_row++) {
          result_array.data_mut()[result_array_index + batch_row] = batched_results[batch_row].data[value_index];
        }
      }
      result_position += batch_index;
      batch_index = 0u;
    }
  } while (iteration < iteration_count);

  benchmark->teardown();

  if (success) {
    for (size_t value_id = 0; value_id < kMaxValueCount; value_id++) {
      calculate_statistics_from_values(result_array.data_mut() + iteration_count * value_id, iteration_count, results.values[value_id]);
    }

    AggregatedValue& iter_count = results.values[kIterCountValueIndex];
    iter_count = {};
    iter_count.min = iteration_count;
    iter_count.max = iteration_count;
    iter_count.avg = iteration_count;
    iter_count.p50 = iteration_count;
    iter_count.p90 = iteration_count;
    iter_count.p95 = iteration_count;
    iter_count.p99 = iteration_count;
    iter_count.geometric_mean = double(iteration_count);
    return true;
  }
  else {
    return false;
  }
}

// Benchmark Interface
// -------------------

Benchmark::Benchmark() noexcept {}
Benchmark::~Benchmark() noexcept {}

void Benchmark::prepare([[maybe_unused]] Context& ctx) {}
void Benchmark::run(Context& ctx) { ctx.make_failed("Benchmark not initialized"); }
void Benchmark::teardown() {}

void Benchmark::set_label(uint32_t id, const char* str) {
  set_label(id, str, strlen(str));
}

void Benchmark::set_label(uint32_t id, const char* str, size_t size) {
  if (id < kMaxLabelCount) {
    _labels[id].assign(str, size);
  }
}

SimpleBenchmark::SimpleBenchmark(const char* name, EntryFunc func) noexcept : Benchmark() {
  set_label(0u, name);
  _entry_func = func;
}
SimpleBenchmark::~SimpleBenchmark() noexcept {}

void SimpleBenchmark::run(Context& ctx) { _entry_func(ctx); }

// Benchmark Runner
// ----------------

Runner::Runner(int argc, const char* const* argv) noexcept : _cmd_line(argc, argv) {
  _iteration_count = _cmd_line.value_as_uint("--iter-count", 0u);
  _max_iteration_count = _cmd_line.value_as_uint("--max-iter-count", 0u);

  _warm_up_iterations = _cmd_line.value_as_uint("--warm-up-iters", 0u);
  _warm_up_nanoseconds = _cmd_line.value_as_uint("--warm-up-nanos", 1000000u);

  const char* run_filter = _cmd_line.get_arg("--run", nullptr);
  if (run_filter && run_filter[0]) {
    _run_filter.assign(run_filter);
  }

  if (ZeroLib::is_color_output_supported() && !_cmd_line.has_arg("--no-color")) {
    ZeroLib::set_color_output(true);
  }

  set_label(0, "Benchmark");
}

static void Runner_set_label(Runner& self, uint32_t label_id, const char* name, size_t size) noexcept {
  if (label_id >= kMaxLabelCount) {
    return;
  }

  self._labels[label_id].name.assign(name, size);
  self._labels_mask |= 1u << label_id;
}

static void Runner_set_metric(Runner& self, uint32_t metric_id, const char* name, size_t size, const MetricOptions& options) noexcept {
  if (metric_id >= kMaxMetricCount) {
    return;
  }

  self._metrics[metric_id].name.assign(name, size);
  self._metrics[metric_id].options = options;
  self._metrics_mask |= 1u << metric_id;
}

void Runner::set_label(uint32_t label_id, const char* name) noexcept {
  Runner_set_label(*this, label_id, name, strlen(name));
}

void Runner::set_label(uint32_t label_id, const String& name) noexcept {
  Runner_set_label(*this, label_id, name.data(), name.size());
}

void Runner::set_metric(uint32_t metric_id, const char* name, const MetricOptions& options) noexcept {
  Runner_set_metric(*this, metric_id, name, strlen(name), options);
}

void Runner::set_metric(uint32_t metric_id, const String& name, const MetricOptions& options) noexcept {
  Runner_set_metric(*this, metric_id, name.data(), name.size(), options);
}

uint32_t Runner::add_metric(const char* name, const MetricOptions& options) noexcept {
  uint32_t metric_id = static_cast<uint32_t>(std::countr_one(_metrics_mask));
  Runner_set_metric(*this, metric_id, name, strlen(name), options);
  return metric_id;
}

uint32_t Runner::add_metric(const String& name, const MetricOptions& options) noexcept {
  uint32_t metric_id = static_cast<uint32_t>(std::countr_one(_metrics_mask));
  Runner_set_metric(*this, metric_id, name.data(), name.size(), options);
  return metric_id;
}

void Runner::add_time_metrics() noexcept {
  add_metric("T. Min", {
    .color = ZeroBench::kMinDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = ZeroBench::kDurationValueIndex,
    .aggregate = ZeroBench::Aggregate::kMin,
    .scale = ZeroBench::Scale::kDefault
  });

  add_metric("T. Med", {
    .color = ZeroBench::kOtherDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = ZeroBench::kDurationValueIndex,
    .aggregate = ZeroBench::Aggregate::kP50,
    .scale = ZeroBench::Scale::kDefault
  });

  add_metric("T. Max", {
    .color = ZeroBench::kMaxDurationColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = ZeroBench::kDurationValueIndex,
    .aggregate = ZeroBench::Aggregate::kMax,
    .scale = ZeroBench::Scale::kDefault
  });

  add_metric("Std Dev", {
    .color = ZeroBench::kOtherStatisticColor,
    .value_kind = ZeroBench::ValueKind::kDuration,
    .value_index = ZeroBench::kDurationValueIndex,
    .aggregate = ZeroBench::Aggregate::kStdDev,
    .scale = ZeroBench::Scale::kDefault
  });
}

struct ColumnProperties {
  size_t width;
  Color color;

  ValueKind value_kind;
  uint8_t value_index;
  Aggregate aggregate;  // Metric options.
  Scale scale;          // Metric options.

  // Column options.
  bool right_aligned;
};

struct Padding {
  size_t left;
  size_t right;
};

class Table {
public:
  static constexpr size_t kMaxColumnCount = 8u + kMaxLabelCount + kMaxMetricCount;

  size_t column_count {};
  size_t failure_column_count {};
  String headings[kMaxColumnCount];
  ColumnProperties columns[kMaxColumnCount] {};
  ColumnProperties failure_columns[kMaxColumnCount] {};

  void add_column(const char* title, size_t title_size, const ColumnProperties& props) noexcept {
    headings[column_count].assign(title, title_size);
    columns[column_count] = props;
    column_count++;
  }

  void add_column(const char* title, const ColumnProperties& props) noexcept { add_column(title, strlen(title), props); }
  void add_column(const String& title, const ColumnProperties& props) noexcept { add_column(title.data(), title.size(), props); }

  void synthesize_failure_columns(size_t end) noexcept {
    failure_column_count = end + 1u;

    for (size_t i = 0; i < failure_column_count; i++) {
      failure_columns[i] = columns[i];
      failure_columns[i].color = Color::kBrightRed;
    }

    failure_columns[end].value_kind = ValueKind::kFailure;
    failure_columns[end].right_aligned = false;

    for (size_t i = failure_column_count; i < column_count; i++) {
      failure_columns[end].width += columns[i].width + 1u;
    }
  }
};

static Padding calculate_padding(size_t text_size, size_t cell_width, bool right_aligned) noexcept {
  size_t left = 0;
  size_t right = text_size > cell_width ? 0 : cell_width - text_size;

  if (right_aligned) {
    std::swap(left, right);
  }

  if (left > 0u && right == 0u) {
    left--;
    right++;
  }

  if (right > 0u && left == 0u) {
    left++;
    right--;
  }

  return Padding{left, right};
};

static void format_time(String& sb, double t, Scale scale, bool squared = false) noexcept {
  if (scale == Scale::kDefault) {
    double magnitude = squared ? std::sqrt(t) : t;
    if (magnitude < 0.00001) {
      scale = Scale::kMicro;
    }
    else if (magnitude < 1.0) {
      scale = Scale::kMilli;
    }
    else {
      scale = Scale::kNone;
    }
  }

  switch (scale) {
    case Scale::kNano:
      sb.append_format("%.3f%s", t * (squared ? 1e18 : 1e9), squared ? "ns^2" : "ns");
      break;
    case Scale::kMicro:
      sb.append_format("%.3f%s", t * (squared ? 1e12 : 1e6), squared ? "us^2" : "us");
      break;
    case Scale::kMilli:
      sb.append_format("%.3f%s", t * (squared ? 1e6 : 1e3), squared ? "ms^2" : "ms");
      break;
    default:
      sb.append_format("%.3f%s", t, squared ? "s^2 " : "s ");
      break;
  }
}

static void format_metric(String& sb, double val, ValueKind kind, Scale scale, bool squared = false) noexcept {
  struct UnitString {
    char unit[8];
  };

  static constexpr UnitString byte_units_1[4] = {{"B"}, {"kB"}, {"MB"}, {"GB"}};
  static constexpr UnitString byte_units_2[4] = {{"B"}, {"KiB"}, {"MiB"}, {"GiB"}};
  static constexpr UnitString count_units[4] = {{""}, {"k"}, {"M"}, {"G"}};

  const UnitString* unit_text = nullptr;

  switch (kind) {
    case ValueKind::kDuration:
      format_time(sb, double(val) / (squared ? 1e18 : 1e9), scale, squared);
      return;

    case ValueKind::kBytes:
      unit_text = byte_units_1;

      if (scale == Scale::kDefaultBytes) {
        if (val < double(1024 * 1024)) {
          scale = Scale::kKiB;
        }
        else if (val < double(1024 * 1024 * 1024)) {
          scale = Scale::kMiB;
        }
        else {
          scale = Scale::kGiB;
        }
      }

      if (scale == Scale::kKiB || scale == Scale::kMiB || scale == Scale::kGiB) {
        unit_text = byte_units_2;
        if (scale == Scale::kKiB) {
          sb.append_format("%.3f%s", val / double(1024), unit_text[1].unit);
        }
        else if (scale == Scale::kMiB) {
          sb.append_format("%.3f%s", val / double(1024 * 1024), unit_text[2].unit);
        }
        else {
          sb.append_format("%.3f%s", val / double(1024 * 1024 * 1024), unit_text[3].unit);
        }
        return;
      }
      break;

    case ValueKind::kCount:
    case ValueKind::kPixels:
      unit_text = count_units;
      if (scale != Scale::kNone) {
        break;
      }
      [[fallthrough]];

    default:
      sb.append_value(val);
      return;
  }

  double magnitude = squared ? std::sqrt(val) : val;
  const char* exponent = squared ? "^2" : "";

  if (magnitude >= 1000000000.0) {
    sb.append_format("%.3f%s%s", val / (squared ? 1e18 : 1e9), unit_text[3].unit, exponent);
  }
  else if (magnitude >= 1000000.0) {
    sb.append_format("%.3f%s%s", val / (squared ? 1e12 : 1e6), unit_text[2].unit, exponent);
  }
  else {
    sb.append_format("%.3f%s%s", val / (squared ? 1e6 : 1e3), unit_text[1].unit, exponent);
  }
}

static void format_aggregated_value(String& sb, const AggregatedValue& av, const ColumnProperties& props, bool is_total) noexcept {
  uint64_t value = 0u;
  double value_fp = 0.0;
  bool can_aggregate = true;

  switch (props.aggregate) {
    case Aggregate::kStatic       : value = av.min; break;
    case Aggregate::kMin          : value = av.min; break;
    case Aggregate::kMax          : value = av.max; break;
    case Aggregate::kMean         : value = av.avg; break;
    case Aggregate::kP50          : value = av.p50; break;
    case Aggregate::kP90          : value = av.p90; break;
    case Aggregate::kP95          : value = av.p95; break;
    case Aggregate::kP99          : value = av.p99; break;
    case Aggregate::kVariance     : can_aggregate = false; value_fp = av.variance; break;
    case Aggregate::kStdDev       : can_aggregate = false; value_fp = av.stddev; break;
    case Aggregate::kMAD          : can_aggregate = false; value_fp = av.mad; break;
    case Aggregate::kGeometricMean: can_aggregate = false; value_fp = av.geometric_mean; break;
    case Aggregate::kCV           : can_aggregate = false; value_fp = av.cv; break;
  }

  if (is_total && (!can_aggregate || props.aggregate == Aggregate::kP90 ||
                                     props.aggregate == Aggregate::kP95 ||
                                     props.aggregate == Aggregate::kP99)) {
    // Separate benchmark distributions cannot be combined by adding their statistics.
    sb.append("N/A");
  }
  else if (!can_aggregate) {
    if (props.aggregate == Aggregate::kCV) {
      sb.append_format("%.3f%%", value_fp * 100.0);
    }
    else {
      format_metric(sb, value_fp, props.value_kind, props.scale, props.aggregate == Aggregate::kVariance);
    }
  }
  else {
    format_metric(sb, double(value), props.value_kind, props.scale);
  }
}

int Runner::run(const BenchmarkRef* benchmarks, size_t benchmark_count) {
  Table table;
  size_t max_label_len[kMaxLabelCount] {};

  for (uint32_t label_id = 0; label_id < kMaxLabelCount; label_id++) {
    max_label_len[label_id] = _labels[label_id].name.size();
  }

  for (size_t i = 0; i < benchmark_count; i++) {
    if (!_run_filter.is_empty() && !StringUtils::match_case(benchmarks[i]->name(), _run_filter)) {
      continue;
    }

    for (uint32_t label_id = 0; label_id < kMaxLabelCount; label_id++) {
      max_label_len[label_id] = std::max(max_label_len[label_id], benchmarks[i]->_labels[label_id].size());
    }
  }

  for (uint32_t label_id = 0; label_id < kMaxLabelCount; label_id++) {
    if (is_label_enabled(label_id)) {
      table.add_column(_labels[label_id].name, {
        .width = max_label_len[label_id] + 2u,
        .color = kDefaultLabelColor,
        .value_kind = ValueKind::kLabel,
        .value_index = static_cast<uint8_t>(label_id),
        .aggregate = Aggregate::kStatic,
        .scale = Scale::kNone,
        .right_aligned = false
      });
    }
  }

  size_t failure_column_count = table.column_count;
  if (_metrics_mask == 0) {
    printf("Metrics is empty, use add_time_metrics() to add basic metrics");
    return 1;
  }

  for (uint32_t metric_id = 0; metric_id < kMaxMetricCount; metric_id++) {
    if (is_metric_enabled(metric_id)) {
      const Metric& metric = _metrics[metric_id];
      table.add_column(metric.name, {
        .width = std::max<size_t>(metric.name.size() + 2, 11),
        .color = metric.options.color,
        .value_kind = metric.options.value_kind,
        .value_index = metric.options.value_index,
        .aggregate = metric.options.aggregate,
        .scale = metric.options.scale,
        .right_aligned = true
      });
    }
  }

  table.synthesize_failure_columns(failure_column_count);

  StringTmp<256> frame;
  StringTmp<256> sb;

  frame.append_start_escape(Color::kGray);
  for (size_t column_index = 0; column_index < table.column_count; column_index++) {
    const ColumnProperties& props = table.columns[column_index];

    frame.append('+');
    frame.append_chars('-', props.width);

    Padding padding = calculate_padding(table.headings[column_index].size(), props.width, props.right_aligned);
    sb.c_append(Color::kGray, "|");
    sb.c_append_padded(Color::kBrightWhite, table.headings[column_index], padding.left, padding.right);
  }
  frame.append("+");
  frame.append_end_escape(Color::kGray);
  frame.append("\n");

  sb.c_append(Color::kGray, "|");
  sb.append("\n");

  print(frame);
  print(sb);
  print(frame);

  Results results_total {};

  auto format_table_row = [&](String& sb, ColumnProperties* column_props, size_t column_count, const String* labels, const Results& results, bool is_total) {
    StringTmp<256> col;
    sb.c_append(Color::kGray, "|");

    for (size_t column_index = 0; column_index < column_count; column_index++) {
      const ColumnProperties& props = column_props[column_index];
      col.clear();
      switch (props.value_kind) {
        case ValueKind::kLabel: {
          col.append(labels[props.value_index]);
          break;
        }

        case ValueKind::kFailure: {
          col.append(results.failure_message);
          break;
        }

        default: {
          const AggregatedValue& av = results.values[props.value_index];
          format_aggregated_value(col, av, props, is_total);
          break;
        }
      }

      Padding padding = calculate_padding(col.size(), props.width, props.right_aligned);
      sb.c_append_padded(props.color, col, padding.left, padding.right);
      sb.c_append(Color::kGray, "|");
    }
    sb.append("\n");
  };

  for (size_t i = 0; i < benchmark_count; i++) {
    if (!_run_filter.is_empty() && !StringUtils::match_case(benchmarks[i]->name(), _run_filter)) {
      continue;
    }

    Benchmark* benchmark = benchmarks[i].ptr();
    Results results;

    bool ok = bench_single(benchmark, _iteration_count, _max_iteration_count, _warm_up_iterations, _warm_up_nanoseconds, results);

    sb.clear();
    if (ok) {
      format_table_row(sb, table.columns, table.column_count, benchmark->_labels, results, false);
      results_total.add(results);
    }
    else {
      format_table_row(sb, table.failure_columns, table.failure_column_count, benchmark->_labels, results, false);
    }
    print(sb);
  }

  if (benchmark_count > 1) {
    print(frame);

    String total_labels[kMaxLabelCount];
    total_labels[0].assign("Total");

    sb.clear();
    format_table_row(sb, table.columns, table.column_count, total_labels, results_total, true);
    print(sb);
  }

  print(frame);
  return 0;
}

}
