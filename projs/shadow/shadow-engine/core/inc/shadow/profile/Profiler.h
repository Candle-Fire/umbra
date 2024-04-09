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
    // Stop the thread being profiled.
    // Useful for scripting.
    static API void Pause(bool paused);
    // Set a specific name for the current thread in the profiler.
    static API void SetThreadName(const char *name);
    // Set whether the current thread will show in the profiler.
    static API void ShowInProfiler(bool show);

    // Start a new block. Will separate the named results out from all others.
    static API void Begin(const char *name);
    // Set a color for the current block.
    static API void BlockColor(DirectX::XMFLOAT3 c);
    // End the current block.
    static API void End();
    // End the current frame.
    static API void Frame();
    // Tell the profiler that a new job is starting on the current thread. Will allow filtering by job type.
    static API void PushJob(size_t signal);
    // Send arbitrary data to the profiler log.
    static API void PushString(const char *value);
    static API void PushInt(const char *key, int value);

    // Create a counter; increase at whim.
    static API size_t MakeCounter(const char *key, float min);
    // Send the counter data to the profiler log.
    static API void PushCounter(size_t ID, float value);

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

    // Data for a fiber thread (soft-switching, used for the job task system
    struct FiberData {
      uint32_t ID;
      uint32_t blocks[16];
      uint32_t count;
      size_t signal;
    };

    // Preparing for a fiber switch
    static API void PreFiberSwitch();
    // Trigger a signal on a fiber thread
    static API void Trigger(size_t signal);
    // A fiber thread is about to start waiting
    static API FiberData BeginFiberWait(size_t signal, bool isMutex);
    // A fiber thread is about to come out of waiting
    static API void EndFiberWait(const FiberData& data);

    // How long did we spend in the last frame?
    static API float GetLastFrameDuration();

    // Can we context switch while profiling?
    // Requires system agreement (ie. Windows must let us attach a profiler thread monitor.)
    static API bool IsContextSwitchEnabled();

    // Get the effective frequency of the process.
    static API size_t GetFrequency();

    // Used within the profiler to track when we get context switched out of profiling.
    struct ContextSwitchRecord {
      uint32_t oldThreadID;
      uint32_t newThreadID;
      size_t timestamp;
      uint8_t cause;
    };

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

    struct Job {
      size_t signal;
    };

    struct FiberWait {
      uint32_t ID;
      size_t signal;
      bool mutex;
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
      FiberWait,
      FiberWake,
      ContextSwitch,
      Job,
      GPUBegin,
      GPUEnd,
      Link,
      Pause,
      GPUStats,
      Continue,
      Signal,
      Counter
    };

    #pragma pack(1)
    struct Event {
      uint8_t size;
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