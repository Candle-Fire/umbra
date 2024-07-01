#pragma once
#include <shadow/exports.h>
#include <cstddef>
#include "shadow/util/Synchronization.h"

namespace SH {

  // A thread of execution. Start & manage threads on the fly.
  struct API Thread {

    explicit Thread();
    virtual ~Thread();

    // The actual work to do.
    virtual int Run() = 0;

    // Start the thread and start it running.
    bool Start(const char* name, bool extended);
    // Wait for the thread to finish, blocking main thread until it's done
    bool Join();

    // Update the core affinity (makes the process favor a certain core that may be available more often)
    void SetAffinity(size_t mask);

    // Wait. The given mutex will lock on the current thread, and will not be avaiable until this thread restarts.
    void Wait(struct ShadowEngine::Mutex& mut);
    // Unlock current thread and resumes. Partner to wait.
    void Notify();

    // Whether we're currently taking up CPU time.
    bool IsRunning() const;
    // Whether our task is finished.
    bool IsFinished() const;

  private:
    struct ThreadImpl* implementation;
  };
}