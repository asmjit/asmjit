// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef PERFORMANCETIMER_H_INCLUDED
#define PERFORMANCETIMER_H_INCLUDED

#include <asmjit/core.h>
#include <chrono>

class PerformanceTimer {
public:
  typedef std::chrono::high_resolution_clock::time_point TimePoint;

  TimePoint _startTime {};
  TimePoint _endTime {};

  inline void start() {
    _startTime = std::chrono::high_resolution_clock::now();
  }

  inline void stop() {
    _endTime = std::chrono::high_resolution_clock::now();
  }

  inline double duration() const {
    std::chrono::duration<double> elapsed = _endTime - _startTime;
    return elapsed.count() * 1000;
  }
};

static inline double mbps(double duration, uint64_t outputSize) noexcept {
  if (duration == 0)
    return 0.0;

  double bytesTotal = double(outputSize);
  return (bytesTotal * 1000) / (duration * 1024 * 1024);
}

#endif // PERFORMANCETIMER_H_INCLUDED
