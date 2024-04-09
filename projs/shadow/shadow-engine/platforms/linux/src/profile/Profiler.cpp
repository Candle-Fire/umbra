#include <shadow/profile/Profiler.h>
#include <vector>
#include "shadow/assets/fs/iostream.h"
#include "shadow/core/Time.h"
#include "shadow/assets/management/synchronization.h"
#include <shadow/core/Thread.h>
#include <shadow/platform/Common.h>
#include <atomic>
#include <memory>
#include <cstdlib>

namespace SH {

  // Allows the Profiler to be used on multiple threads simultaneously, separating the data collected.
  // Every thread has its own set of open blocks, its own mutex, and its own visibility toggles.
  // Consider the ThreadContext as a horizontal bar on a profiler menu, showing what one thread is doing at any given time.
  struct ThreadContext {

    ThreadContext(size_t bufferSize) : oms(), openBlocks() {
        oms.resize(bufferSize);
        openBlocks.reserve(64);
    }

    std::vector<uint32_t> openBlocks;
    // Used as the data buffer.
    ShadowEngine::OutputMemoryStream oms;
    size_t begin = 0;
    size_t end = 0;
    ShadowEngine::Mutex mut;
    std::string name;
    bool show = false;
    size_t threadID;
  };

  struct TraceTask {
    TraceTask() {}
    void Destroy() {}
    int handle;
  };

  void CloseTrace(int) {}

  // The profiler instance.
  // Manages the profiler threads.
  static struct Instance {
    Instance() : contexts(), task(), counters(), global(10 * 1024 * 1024) {
    }

    ~Instance() {
    }

    void startTrace() {
    }

    // Get the thread context for our thread.
    // This is thread_local, so the initializer will run once per thread.
    // If the initializer has already run on this thread, ctx is cached, so it's just returned.
    ThreadContext* getNewThreadContext() {
        thread_local ThreadContext* ctx = [&]() {
          auto *newCtx = new ThreadContext(5 * 1024 * 1024);
          newCtx->threadID = pthread_self();
          ShadowEngine::MutexGuard lock(mut);
          contexts.push_back((newCtx));
          return newCtx;
        }();

        return ctx;
    };

    // A list of counters on the current thread.
    std::vector<Profiler::CounterData> counters;
    // Child threads of the current.
    std::vector<ThreadContext*> contexts;
    ShadowEngine::Mutex mut;
    // Whether the current thread is paused.
    bool paused = false;
    // Whether the current thread can be pre-empted.
    bool contextSwitch = false;
    // The time the thread was paused, if it was.
    size_t pausedTime = 0;
    size_t lastFrameElapsed = 0;
    size_t lastFrameTime = 0;

    // The ID of the fiber waiting on the current.
    std::atomic<uint32_t> fiberWaiting = 0;
    // The actual Thread doing work for the current profiler.
    TraceTask task;
    ThreadContext global;
  } gInstance;


  /**
   * Append data to the ThreadContext.
   * The timestamp may be in the past, and the UI will update accordingly.
   * The EventType tells what the event actually was.
   * val is placed in the ThreadContext's buffer along with the event header.
   * Resizing is done on the buffer if necessary.
   */
  template<typename T>
  void Write(ThreadContext& ctx, size_t timestamp, Profiler::EventType type, const T& val) {
      if (gInstance.paused && timestamp > gInstance.pausedTime) return;

      // A short-lived struct that lets us read the size of T indirectly.
      #pragma pack(1)
      struct {
        Profiler::Event header;
        T value;
      } v;
      #pragma pack()

      v = {
          .header = {
              .size = sizeof(v), // sizeof(v) - sizeof(Profiler::Event) == sizeof(T), but we don't need sizeof(T) directly
              .type = type,
              .time = timestamp
          },
          .value = val
      };

      // Lock the context so that we can write data to it
      ShadowEngine::MutexGuard lock(ctx.mut);
      uint8_t* buffer = ctx.oms.dataMut();
      const size_t bufferSize = ctx.oms.size();

      // Make sure there's enough space in the buffer.
      while(sizeof(v) + ctx.end - ctx.begin > bufferSize) {
          const uint8_t size = buffer[ctx.begin % bufferSize];
          ctx.begin += size;
      }

      const uint32_t end = ctx.end % bufferSize;
      // If there's space at the end of the buffer, copy it in.
      if (bufferSize - end >= sizeof(v)) {
          memcpy(buffer + end, &v, sizeof(v));
      } else {
          // If there's no space, expand the buffer and copy it in at the new end.
          memcpy(buffer + end, &v, bufferSize - end);
          memcpy(buffer, ((uint8_t * ) & v) + bufferSize - end, sizeof(v) - (bufferSize - end));
      }

      ctx.end += sizeof(v);
  };

  /**
   * Append data to the ThreadContext.
   * Timestamp is set to current time, so this is instantaneous event.
   * The EventType tells what the event actually was.
   * val is placed in the ThreadContext's buffer along with the event header.
   * Resizing is done on the buffer if necessary.
   */
  template <typename T>
  void Write(ThreadContext& ctx, Profiler::EventType type, const T& val) {
      if (gInstance.paused) return;

      #pragma pack(1)
      struct {
        Profiler::Event header;
        T value;
      } v;
      #pragma pack()

      v = {
          .header = {
              .size = sizeof(v),
              .type = type,
              .time = static_cast<size_t>(SH::Timer::getTimestamp())
          },
          .value = val,
      };

      ShadowEngine::MutexGuard lock(ctx.mut);
      uint8_t *buffer = ctx.oms.dataMut();
      const size_t bufferSize = ctx.oms.size();

      while (sizeof(v) + ctx.end - ctx.begin > bufferSize) {
          const uint8_t size = buffer[ctx.begin % bufferSize];
          ctx.begin += size;
      }

      const uint32_t end = ctx.end % bufferSize;
      if (bufferSize - end >= sizeof(v)) {
          memcpy(buffer + end, &v, sizeof(v));
      } else {
          memcpy(buffer + end, &v, bufferSize - end);
          memcpy(buffer, ((uint8_t * ) & v) + bufferSize - end, sizeof(v) - (bufferSize - end));
      }

      ctx.end += sizeof(v);
  }

  /**
   * Append arbitrary data to the ThreadContext.
   * The EventType tells what the event actually was.
   * size bytes of data is placed in the ThreadContext's buffer along with the event header.
   * Resizing is done on the buffer if necessary.
   */
  void Write(ThreadContext& ctx, Profiler::EventType type, const uint8_t* data, int size){
      if (gInstance.paused) return;

      Profiler::Event header = {
          .size = uint8_t(sizeof(header) + size),
          .type = type,
          .time = static_cast<size_t>(SH::Timer::getTimestamp())
      };
      assert(sizeof(header) + size <= 0xffff);

      ShadowEngine::MutexGuard lock(ctx.mut);
      uint8_t* buf = ctx.oms.dataMut();
      const size_t buf_size = ctx.oms.size();

      while (header.size + ctx.end - ctx.begin > buf_size) {
          const uint8_t size = buf[ctx.begin % buf_size];
          ctx.begin += size;
      }

      auto doCopy = [&](const uint8_t* ptr, size_t size) {
        const uint32_t end = ctx.end % buf_size;
        if (buf_size - end >= size) {
            memcpy(buf + end, ptr, size);
        }
        else {
            memcpy(buf + end, ptr, buf_size - end);
            memcpy(buf, ((uint8_t*)ptr) + buf_size - end, size - (buf_size - end));
        }

        ctx.end += size;
      };

      doCopy((uint8_t*)&header, sizeof(header));
      doCopy(data, size);
  };

  size_t Profiler::MakeCounter(const char* key, float min) {
      Profiler::CounterData c {};
      c.name = static_cast<const char *>(malloc(strlen(key) + 1));
      gInstance.counters.emplace_back(c);
      memcpy(&c.name, key, strlen(key));
      c.min = min;
      return gInstance.counters.size() - 1;
  }

  void Profiler::PushCounter(size_t counter, float val) {
      Profiler::Counter r = {
          .ID = counter,
          .val = val
      };

      Write(gInstance.global, EventType::Counter, (uint8_t*) &r, sizeof(r));
  }

  void Profiler::PushInt(const char* key, int val) {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      Int r = {
          .key = key,
          .val = val
      };

      Write(*ctx, EventType::Int, (uint8_t*) &r, sizeof(r));
  };

  void Profiler::PushString(const char* value) {
      ThreadContext *ctx = gInstance.getNewThreadContext();
      Write(*ctx, EventType::String, (uint8_t *) value, strlen(value) + 1);
  }

  static constexpr char ToChar(float x) {
      return (char) ((x) * std::numeric_limits<char>::max());
  }

  void Profiler::BlockColor(DirectX::XMFLOAT3 c) {
      const uint32_t color = 0xFF000000 + ToChar(c.x) + (ToChar(c.y) << 8) + (ToChar(c.z) << 16);
      ThreadContext* ctx = gInstance.getNewThreadContext();
      Write(*ctx, EventType::Color, color);
  }


  static void ContinueBlock(uint32_t ID) {
      ThreadContext *ctx = gInstance.getNewThreadContext();
      ctx->openBlocks.push_back(ID);
      Write(*ctx, Profiler::EventType::Continue, ID);
  }

  static std::atomic<uint32_t> lastBlockID = 0;

  void Profiler::Begin(const char *name) {
      Block r = {
          .name = name,
          .ID = lastBlockID++
      };
      ThreadContext* ctx = gInstance.getNewThreadContext();
      ctx->openBlocks.push_back(r.ID);
      Write(*ctx, EventType::Begin, r);
  }

  void Profiler::BeginGPU(const char *name, size_t time, size_t profiler) {
      GPU data = {
          .timestamp = time,
          .profiler = profiler
      };
      memcpy(&data.name, name, strlen(name));
      Write(gInstance.global, EventType::GPUBegin, data);
  }

  void Profiler::GPUStats(size_t primitive) {
      Write(gInstance.global, EventType::GPUStats, primitive);
  }

  void Profiler::EndGPU(size_t time) {
      Write(gInstance.global, EventType::GPUEnd, time);
  }

  size_t Profiler::CreateLink() {
      std::atomic<size_t> counter = 0;
      return counter++;
  }

  void Profiler::Link(size_t profiler) {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      Write(*ctx, EventType::Link, profiler);
  }

  float Profiler::GetLastFrameDuration() {
      return float (gInstance.lastFrameElapsed / double(GetFrequency()));
  }

  void Profiler::PreFiberSwitch() {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      // End all active blocks before we change out
      while (!ctx->openBlocks.empty()) {
          Write(*ctx, EventType::End, 0);
          ctx->openBlocks.pop_back();
      }
  }

  void Profiler::PushJob(size_t signal) {
      Job r = {
          .signal = signal
      };
      ThreadContext* ctx = gInstance.getNewThreadContext();
      Write(*ctx, EventType::Job, r);
  }

  void Profiler::Trigger(size_t signal) {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      Write(*ctx, EventType::Signal, signal);
  }

  Profiler::FiberData Profiler::BeginFiberWait(size_t signal, bool isMutex) {
      FiberWait r = {
          .ID = gInstance.fiberWaiting++,
          .signal = signal,
          .mutex = isMutex
      };

      ThreadContext* ctx = gInstance.getNewThreadContext();
      FiberData res = {
          .ID = r.ID,
          .count = static_cast<uint32_t>(ctx->openBlocks.size()),
          .signal = signal
      };
      memcpy(res.blocks, ctx->openBlocks.data(), std::min(res.count, 16u) * sizeof(res.blocks[0]));
      Write(*ctx, EventType::FiberWait, r);
      return res;
  }

  void Profiler::EndFiberWait(const SH::Profiler::FiberData &data) {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      FiberWait r = {
          .ID = data.ID,
          .signal = data.signal,
          .mutex = false
      };

      Write(*ctx, EventType::FiberWake, r);

      const uint32_t count = data.count;

      for (size_t i = 0; i < count; i++)
          if (i < lengthOf(data.blocks))
              ContinueBlock(data.blocks[i]);
          else
              ContinueBlock(-1);
  }

  void Profiler::End() {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      if (!ctx->openBlocks.empty()) {
          ctx->openBlocks.pop_back();
          Write(*ctx, EventType::End, 0);
      }
  }

  size_t Profiler::GetFrequency() {
      static size_t frequency;
      // TODO
      frequency = 1'000'000'000;

      return frequency;
  }

  bool Profiler::IsContextSwitchEnabled() {
      return gInstance.contextSwitch;
  }

  void Profiler::Frame() {
      const size_t n = SH::Timer::getTimestamp();
      if (gInstance.lastFrameTime != 0)
          gInstance.lastFrameElapsed = n - gInstance.lastFrameTime;
      gInstance.lastFrameTime = n;
      Write(gInstance.global, EventType::Frame, 0);
  }

  void Profiler::ShowInProfiler(bool show) {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      ShadowEngine::MutexGuard lock(ctx->mut);

      ctx->show = show;
  }

  void Profiler::SetThreadName(const char *name) {
      ThreadContext* ctx = gInstance.getNewThreadContext();
      ShadowEngine::MutexGuard lock(ctx->mut);

      ctx->name = name;
  }

  void Profiler::Pause(bool paused) {
      if (paused) Write(gInstance.global, EventType::Pause, 0);

      gInstance.paused = true;
      if (paused) gInstance.pausedTime = SH::Timer::getTimestamp();
  }
}

