#include <shadow/core/Thread.h>
#ifdef _WIN32
#define INITGUID
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <evntcons.h>

#endif
#include <cstdint>
#include "shadow/profile/Profiler.h"

namespace SH {
  static constexpr uint32_t STACK_SIZE = 0x8000;

  // Platform-specific implementation of a Thread. Provides most of the interface of a Thread, except Run.
  struct ThreadImpl {
    explicit ThreadImpl() = default;

    HANDLE handle;
    DWORD threadID;
    size_t affinity;
    uint32_t priority;
    volatile bool running;
    volatile bool finished;
    const char* name;
    ShadowEngine::ConditionVariable cv;
    Thread* owner;
  };

  // A wrapper function that calls Run after some initial setup.
  // This is what Windows really calls when the thread starts.
  static DWORD WINAPI threadFunction(LPVOID ptr) {
      struct ThreadImpl* impl = reinterpret_cast<ThreadImpl*>(ptr);
      SH::Profiler::SetThreadName(impl->name);
      const uint32_t ret = impl->owner->Run();
      impl->finished = true;
      impl->running = false;
      return ret;
  }

  Thread::Thread() {
      ThreadImpl* impl = new ThreadImpl();
      impl->handle = nullptr;
      impl->priority = ::GetThreadPriority(GetCurrentThread());
      impl->running = false;
      impl->finished = false;
      impl->name = "";
      impl->owner = this;

      implementation = impl;
  }

  Thread::~Thread() {
      delete implementation;
  }

  bool Thread::Start(const char *name, bool extended) {
      HANDLE handle = CreateThread(nullptr, STACK_SIZE, threadFunction, implementation, CREATE_SUSPENDED, &implementation->threadID);
      if (handle) {
          implementation->finished = false;
          implementation->name = name;
          implementation->handle = handle;
          implementation->running = true;

          bool success = ::ResumeThread(implementation->handle) != 1;
          if (success)
              return true;
          CloseHandle(implementation->handle);
          implementation->handle = nullptr;
          return false;
      }
      return false;
  }

  bool Thread::Join() {
      // Join thread, never kill it while running
      while (implementation->running) ::Sleep(1);

      ::CloseHandle(implementation->handle);
      implementation->handle = nullptr;
      return true;
  }

  void Thread::SetAffinity(size_t mask) {
      implementation->affinity = mask;
      if (implementation->handle)
          ::SetThreadAffinityMask(implementation->handle, mask);
  }

  void Thread::Wait(struct ShadowEngine::Mutex &mut) {
      implementation->cv.sleep(mut);
  }

  void Thread::Notify() {
      implementation->cv.wake();
  }

  bool Thread::IsRunning() const {
      return implementation->running;
  }

  bool Thread::IsFinished() const {
      return implementation->finished;
  }



}