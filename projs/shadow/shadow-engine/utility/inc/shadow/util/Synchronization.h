#pragma once

#include <cstdint>
#include <atomic>
#include <thread>

#ifdef __linux__
#include <pthread.h>
#endif

namespace ShadowEngine {
  // A simple synchronization system that allows one "accessing thread" at a time.
  struct alignas(8) Mutex {
    friend struct ConditionVariable;
    Mutex();
    Mutex(const Mutex&) = delete;
    ~Mutex();

    void enter();
    void exit();
  private:
#ifdef _WIN32
    uint8_t data[8] {};
#else
    pthread_mutex_t mutex;
#endif
  };

  // A simple synchronization system that allows many threads to wait for one thread to complete an operation.
  struct Semaphore {
    Semaphore(int initcount, int maxcount);
    Semaphore(const Semaphore&) = delete;
    ~Semaphore();

    void raise();
    void wait();
  private:
#ifdef _WIN32
    void* id;
#else
    struct {
            pthread_mutex_t mutex;
            pthread_mutex_cond cond;
            volatile int32_t count;
        } id;
#endif
  };

  struct ConditionVariable {
    ConditionVariable();
    ConditionVariable(const ConditionVariable&) = delete;
    ~ConditionVariable();

    void sleep(Mutex& mut);
    void wake();

  private:
#ifdef _WIN32
    uint8_t data[64];
#else
    pthread_cond_t cond;
#endif
  };

  // A simple RAII wrapper for Mutexes, that locks and unlocks as the Guard goes in and out of scope.
  struct MutexGuard {
    explicit MutexGuard(Mutex& mut) : mut(mut) {
        mut.enter();
    }

    ~MutexGuard() { mut.exit(); }

    MutexGuard(const MutexGuard&) = delete;
    void operator=(const MutexGuard&) = delete;

  private:
    Mutex& mut;
  };


  class SpinLock {
  private:
    std::atomic_flag lck = ATOMIC_FLAG_INIT;
  public:
    inline void Lock() {
        int spin = 0;
        while (!TryLock()) {
            if (spin < 10)
                _mm_pause(); // SMT thread swap can occur here
            else
                std::this_thread::yield(); // OS thread swap can occur here. It is important to keep it as fallback, to avoid any chance of lockup by busy wait

            spin++;
        }
    }

    inline bool TryLock() {
        return !lck.test_and_set(std::memory_order_acquire);
    }

    inline void Unlock() {
        lck.clear(std::memory_order_release);
    }
  };
}