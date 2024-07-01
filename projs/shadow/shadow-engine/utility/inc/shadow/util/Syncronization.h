#pragma once

#include <array>
#ifdef __linux__
#include <pthread.h>
#endif

namespace ShadowEngine {

  struct alignas(8) Mutex {
    friend struct ConditionVariable;

    Mutex();

    Mutex(const Mutex &) = delete;

    ~Mutex();

    void enter();

    void exit();

  private:
#ifdef _WIN32
    uint8_t data[8];
#else
    pthread_mutex_t mutex;
#endif
  };


  struct Semaphore {
    Semaphore(int init_count, int max_count);

    Semaphore(const Semaphore &) = delete;

    ~Semaphore();

    void raise();

    void wait();

  private:
#if defined _WIN32
    void *m_id;
#elif defined __linux__
    struct {
      pthread_mutex_t mutex;
      pthread_cond_t cond;
      volatile /*i32*/ int count; // TODO: I can't find the type anywhere
    } m_id;
#endif
  };


  class ConditionVariable {
  public:
    ConditionVariable();

    ConditionVariable(const ConditionVariable &) = delete;

    ~ConditionVariable();

    void sleep(Mutex &cs);

    void wakeup();

  private:
#ifdef _WIN32
  std::array<uint8_t, 64> data;
#else
    pthread_cond_t cv;
#endif
  };


  struct MutexGuard {
    explicit MutexGuard(Mutex &cs) : m_mutex(cs) {
        cs.enter();
    }

    ~MutexGuard() { m_mutex.exit(); }

    MutexGuard(const MutexGuard &) = delete;

    void operator=(const MutexGuard &) = delete;

  private:
    Mutex &m_mutex;
  };
}