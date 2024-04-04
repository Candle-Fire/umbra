#include <shadow/core/Thread.h>

#include <pthread.h>
#include <cstdint>
#include "shadow/profile/Profiler.h"
#include <sched.h>

namespace SH {

  // Platform-specific implementation of a Thread. Provides most of the interface of a Thread, except Run.
  struct ThreadImpl {
    bool forceExit;
    bool finished;
    bool running;
    pthread_t handle;
    const char* name;
    Thread* owner;
    ShadowEngine::ConditionVariable cv;
  };

  static void* threadFunction(void* ptr) {
      static ThreadImpl* impl = reinterpret_cast<ThreadImpl*>(ptr);
      pthread_setname_np(pthread_self(), impl->name);
      SH::Profiler::SetThreadName(impl->name);
      uint32_t ret = 0xFFFFFFFF;
      if (!impl->forceExit) ret = impl->owner->Task();
      impl->finished = true;
      impl->running = false;

      return nullptr;
  }

  Thread::Thread() {
      auto impl = new ThreadImpl();
      impl->running = false;
      impl->forceExit = false;
      impl->finished = false;
      impl->name = "";
      impl->owner = this;

      implementation = impl;
  }

  Thread::~Thread() {
      delete implementation;
  }

  void Thread::Wait(struct ShadowEngine::Mutex &mut) {
      assert(pthread_self() == implementation->handle);
      implementation->cv.sleep(mut);
  }

  void Thread::Notify() {
      implementation->cv.wakeup();
  }

  bool Thread::Start(const char *name, bool extended) {
      pthread_attr_t attr;
      int res = pthread_attr_init(&attr);
      assert(res == 0);
      if (res != 0) return false;
      res = pthread_create(&implementation->handle, &attr, threadFunction, implementation);
      assert(res == 0);
      if (res != 0) return false;
      return 0;
  }

  bool Thread::Join() {
      return pthread_join(implementation->handle, nullptr) == 0;
  }

  void Thread::SetAffinity(size_t mask) {
      cpu_set_t set;
      CPU_ZERO(&set);
      for (int i = 0; i < 64; i++)
          if (mask & ((size_t)1 << i))
              CPU_SET(i, &set);
      pthread_setaffinity_np(implementation->handle, sizeof(set), &set);
  }

  bool Thread::IsRunning() const {
      return implementation->running;
  }

  bool Thread::IsFinished() const {
      return implementation->finished;
  }
}