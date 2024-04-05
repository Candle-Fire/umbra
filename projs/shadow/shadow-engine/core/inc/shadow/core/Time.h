#pragma once

#include "shadow/exports.h"
#include <chrono>

namespace SH {

  // A RAII-operated timer.
  // Instantiate to start the timer.
  // Call timer.elapsed() to get the number of milliseconds since the timer was created.
  // Call timer.swap() to reset the timer and get the time since instantiation.
  class Timer {

  public:
    std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();

    inline void record() {
        timestamp = std::chrono::high_resolution_clock::now();
    }

    inline double secondsSince(std::chrono::high_resolution_clock::time_point ts2) {
        std::chrono::duration<double> span = std::chrono::duration_cast<std::chrono::duration<double>>(ts2 - timestamp);
        return span.count();
    }

    inline double elapsedSeconds() {
        return secondsSince(std::chrono::high_resolution_clock::now());
    }

    inline double elapsedMillis() {
        return elapsedSeconds() * 1000.0f;
    }

    inline double elapsed() {
        return elapsedMillis();
    }

    inline double swap() {
        auto ts2 = std::chrono::high_resolution_clock::now();
        auto elapsed = secondsSince(ts2);
        timestamp = ts2;
        return elapsed;
    }

    static size_t getTimestamp();

    static API int NOW;
    static API int LAST;

    static API double deltaTime;
    static API double deltaTime_ms;

    static API double timeSinceStart;
    static API double startTime;

    static void UpdateTime();
  };
}