#pragma once

#include <shadow/exports.h>
#include "DirectXMath.h"

namespace SH {

  /**
   * @brief A profiler object.
   * Contains a lot of static utilities if you want more control, otherwise just create an instance (or use the ProfileFunction() macro), and the profiler will track what happens until the object goes out of scope.
   * Provides utilities for renaming, recoloring, filtering in the UI.
   * Allows marking out and timing GPU operations.
   *
   * Standard usage:
   *
   * int Function() {
   *    SH::Profiler::ProfileFunction()
   *    doEngineThings();
   *    loadData();
   * }
   *
   */
  struct Profiler {
    static API void Pause(bool paused);
    static API void SetThreadName(const char *name);
    static API void ShowInProfiler(bool show);

    static API void Begin(const char *name);
    static API void BlockColor(DirectX::XMFLOAT3 c);
    static API void End();
    static API void Frame();
    static API void PushJob(int32_t signal);
    static API void PushString(const char *value);
    static API void PushInt(const char *key, int value);

    static API uint32_t MakeCounter(const char *key, float min);
    static API void PushCounter(uint32_t ID, float value);

    // Profiler: a handle to link the GPU process to a profiler thread. Obtain by calling CreateLink()
    static API void BeginGPU(const char *name, size_t time, size_t profiler);
    // End the GPU block that started at the given timestamp.
    static API void EndGPU(size_t time);
    // Get GPU stats directly.
    static API void GPUStats(size_t primitive);
    // Link the current profiler thread with the given GPU task.
    static API void Link(size_t profiler);
    // Create a handle that can be used to profile GPU operations.
    static API size_t CreateLink();

    // Start the profiler directly.
    explicit Profiler(const char* name) {
        Begin(name);
    }

    // End the profiler when the object goes out of scope.
    ~Profiler() {
        End();
    }

    struct Block {
      const char* name;
      size_t ID;
    };

    struct CounterData {
      const char* name;
      float min;
    };

    struct Counter {
      size_t ID;
      float val;
    };

    struct Int {
      const char* key;
      int val;
    };

    struct GPU {
      const char* name;
      size_t timestamp;
      size_t profiler;
    };

    enum class EventType {
      Begin,
      Color,
      End,
      Frame,
      String,
      Int,
      GPUBegin,
      GPUEnd,
      Link,
      Pause,
      GPuStats,
      Continue,
      Counter
    };

    #pragma pack(1)
    struct Event {
      char size;
      EventType type;
      size_t time;
    };
    #pragma pack()

    #define concat2(a, b) a ## b
    #define concat(a, b) concat2(a,b)

    #define ProfileFunction() SH::Profiler scope(__FUNCTION__);
    #define ProfileBlock(name) SH::Profiler concat(scope, __LINE__)(name);
  };

}