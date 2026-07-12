
// This doesn't work on Linux. Sucks to be you? dpeter won't let me do system-specific source files.
#include "shadow/assets/management/synchronization.h"

#include <cassert>
#ifdef _WIN32

#include <intrin.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "shadow/assets/management/synchronization.h"

namespace SH { struct NewPlaceholder {}; }
inline void* operator new(size_t, SH::NewPlaceholder, void* where) { return where; }
inline void operator delete(void*, SH::NewPlaceholder,  void*) { }

namespace SH {

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

    void ConditionVariable::sleep(Mutex &mut) {
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

namespace SH {
    ConditionVariable::ConditionVariable() {
        const int res = pthread_cond_init(&cond, nullptr);
        assert(res == 0);
    }

    ConditionVariable::~ConditionVariable() {
        const int res = pthread_cond_destroy(&cond);
        assert(res == 0);
    }

    void ConditionVariable::sleep(Mutex& mut) {
        const int res = pthread_cond_wait(&cond, &mut.mutex);
        assert(res == 0);
    }

    void ConditionVariable::wake() {
        const int res = pthread_cond_signal(&cond);
        assert(res == 0);
    }

    Semaphore::Semaphore(int initcount, int maxcount) {
        id.count = initcount;
        int res = pthread_mutex_init(&id.mutex, nullptr);
        assert(res == 0);
        res = pthread_cond_init(&id.cond, nullptr);
        assert(res == 0);
    }

    Semaphore::~Semaphore() {
        int res = pthread_mutex_destroy(&id.mutex);
        assert(res == 0);
        res = pthread_cond_destroy(&id.cond);
        assert(res == 0);
    }

    void Semaphore::raise() {
        int res = pthread_mutex_lock(&id.mutex);
        assert(res == 0);
        res = pthread_cond_signal(&id.cond);
        assert(res == 0);
        id.count = id.count + 1;
        res = pthread_mutex_unlock(&id.mutex);
        assert(res == 0);
    }

    void Semaphore::wait() {
        int res = pthread_mutex_lock(&id.mutex);
        assert(res == 0);
        while (id.count <= 0) {
            res = pthread_cond_wait(&id.cond, &id.mutex);
            assert(res == 0);
        }
        id.count = id.count - 1;
        res = pthread_mutex_unlock(&id.mutex);
        assert(res == 0);
    }

    Mutex::Mutex() {
        const int res = pthread_mutex_init(&mutex, nullptr);
        assert(res == 0);
    }

    Mutex::~Mutex() {
        const int res = pthread_mutex_destroy(&mutex);
        assert(res == 0);
    }

    void Mutex::enter() {
        const int res = pthread_mutex_lock(&mutex);
        assert(res == 0);
    }

    void Mutex::exit() {
        const int res = pthread_mutex_unlock(&mutex);
        assert(res == 0);
    }
}
