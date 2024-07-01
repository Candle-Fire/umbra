
// This doesn't work on Linux. Sucks to be you? dpeter won't let me do system-specific source files.
#ifdef _WIN32

#include <intrin.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "shadow/util/Synchronization.h"

namespace ShadowEngine { struct NewPlaceholder {}; }
inline void* operator new(size_t, ShadowEngine::NewPlaceholder, void* where) { return where; }
inline void operator delete(void*, ShadowEngine::NewPlaceholder,  void*) { }

namespace ShadowEngine {

  Semaphore::Semaphore(int initCount, int maxCount) {
      id = ::CreateSemaphore(nullptr, initCount, maxCount, nullptr);
  }

  Semaphore::~Semaphore() {
      ::CloseHandle(id);
  }

  void Semaphore::raise() {
      ::ReleaseSemaphore(id, 1, nullptr);
  }

  void Semaphore::wait() {
      ::WaitForSingleObject(id, INFINITE);
  }

  ConditionVariable::ConditionVariable() {
      memset(data, 0, sizeof(data));
      auto* var = new (NewPlaceholder(), data) CONDITION_VARIABLE;
      InitializeConditionVariable(var);
  }

  ConditionVariable::~ConditionVariable() {
      ((CONDITION_VARIABLE*)data)->~CONDITION_VARIABLE();
  }

  void ConditionVariable::sleep(ShadowEngine::Mutex &mut) {
      ::SleepConditionVariableSRW((CONDITION_VARIABLE*) data, (SRWLOCK*) mut.data, INFINITE, 0);
  }

  void ConditionVariable::wake() {
      ::WakeConditionVariable((CONDITION_VARIABLE*) data);
  }

  Mutex::Mutex() {
      memset(data, 0, sizeof(data));
      auto* lock = new (NewPlaceholder(), data) SRWLOCK;
      ::InitializeSRWLock(lock);
  }

  Mutex::~Mutex() {
      auto* lock = (SRWLOCK*) data;
      lock->~SRWLOCK();
  }

  void Mutex::enter() {
      auto* lock = (SRWLOCK*) data;
      ::AcquireSRWLockExclusive(lock);
  }

  void Mutex::exit() {
      auto* lock = (SRWLOCK*) data;
      ::ReleaseSRWLockExclusive(lock);
  }
}

#endif 