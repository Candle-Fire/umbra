#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <shadow/core/Time.h>
#include <shadow/core/jobs/Job.h>

#include "spdlog/spdlog.h"

namespace SH::Jobs {
    struct State {
        size_t nCores = 0;
        PriorityQueue priorities[Priority::SIZE];
        std::atomic_bool alive { true };

        void Destroy() {
            alive.store(false);
            bool wake = true;
            // Notify all threads to finish their work
            std::thread waker([&] { while (wake) for (auto& x : priorities) x.wake.notify_all(); });
            // Wait for all threads to stop
            for (auto& x : priorities) for (auto& thread : x.threads) thread.join();
            wake = false;
            // Wait for all threads' dependencies to finish
            waker.join();

            // Cleanup
            for (auto& x : priorities) {
                x.queues.reset();
                x.threads.clear();
                x.nThreads = 0;
            }
            nCores = 0;
        }

        ~State() { Destroy(); }
    };

    static State internalState;

    void Init(size_t maxThreads) {
        // Don't reinit if we're already doing something, that's BAD
        if (internalState.nCores > 0) return;
        maxThreads = std::max(1ull, maxThreads);

        SH::Timer timer;

        internalState.nCores = std::thread::hardware_concurrency();

        for (size_t pri = 0; pri < static_cast<size_t>(Priority::SIZE); pri++) {
            const Priority p = (Priority) pri;
            PriorityQueue& queue = internalState.priorities[pri];

            queue.nThreads =
                p == Priority::HIGHEST ? internalState.nCores - 1 :
                p == Priority::LOW ? internalState.nCores - 2 :
                /* p == Priority::STREAM ? */ 1;
            queue.nThreads = std::clamp(queue.nThreads, 1ull, maxThreads);
            queue.queues.reset(new TaskQueue[queue.nThreads]);
            queue.threads.reserve(queue.nThreads);

            for (size_t thread = 0; thread < queue.nThreads; thread++) {
                std::thread& worker = queue.threads.emplace_back([thread, &queue] {
                    while (internalState.alive.load()) {
                        queue.Execute(thread);
                        std::unique_lock lock(queue.locker);
                        queue.wake.wait(lock);
                    }
                });

                auto handle = worker.native_handle();
                int core = p == Priority::STREAM ? internalState.nCores - 1 - thread : thread + 1;

                // TODO: What the fuck?
                // MINGW doesn't support either std::thread, pthread, or WIN32 threads properly.
                // std::thread returns a pthread_t, with no way to get a WIN32 handle from it.
                // To get a WIN32 handle, we'd have to have the thread store its own handle, and wait for it to be available from the parent thread.
                // pthread doesn't support pthread_setaffinity_np, but it does support pthread_setname_np
                // WIN32 doesn't support SetThreadDescription, but it does support SetThreadPriority.
                // That's a very disgusting system for setting thread name & priority in one go.
                // The only real solutions are:
                //  - MINGW does not get any thread management for jobs
                //    As a consequence, debugging expensive / crashing jobs becomes literally impossible on that platform
                //  - MINGW is banned in developing Umbra.
                //    As a consequence, developing the engine is significantly harder for those of us stuck to Windows, thanks to MSVC being extremely broken
                //  - Creating jobs is significantly more expensive due to the 4-way wait, lock and join required to fetch all the context we require to properly set job priorities
                //    As a consequence, the runtime of the engine suffers, as opposed to just the development.
                // I think this one needs to be mulled over.
                // For now, I'll leave mingw without thread management, and implement it for linux when i switch to that platform later on.
            }
        }

        spdlog::debug(std::string("SH::Jobs initialized in ") + std::to_string(timer.elapsedMillis()) + "ms, utilizing " + std::to_string(internalState.nCores) + " cores, with " + std::to_string(GetThreadsOfPriority(Priority::HIGHEST)) + " high-priority threads, " + std::to_string(GetThreadsOfPriority(Priority::LOW)) + " low-priority threads, and " + std::to_string(GetThreadsOfPriority(Priority::STREAM)) + "asset-streaming threads.");
    }

    void Destroy() {
        internalState.Destroy();
    }

    size_t GetThreadsOfPriority(Priority p) {
        return internalState.priorities[static_cast<size_t>(p)].nThreads;
    }

    void Run(ExecutionContext &context, const std::function<void(TaskArguments)> &task) {
        PriorityQueue& queue = internalState.priorities[static_cast<size_t>(context.priority)];
        context.count.fetch_add(1);

        TaskGroup group { task, &context, 0, 0, 1, 0 };

        if (queue.nThreads <= 1) {
            group.Execute();
            return;
        }

        queue.queues[queue.nextQueue.fetch_add(1) & queue.nThreads].Push(group);
        queue.wake.notify_one();
    }

    void Dispatch(ExecutionContext &context, size_t jobs, size_t groups, const std::function<void(TaskArguments)> &task, size_t sharedMem) {
        if (jobs == 0 || groups == 0) return;

        PriorityQueue& queue = internalState.priorities[static_cast<size_t>(context.priority)];
        const uint32_t nGroups = DispatchGroups(jobs, groups);

        context.count.fetch_add(nGroups);

        TaskGroup group;
        group.context = &context;
        group.func = task;
        group.shared = (uint32_t) sharedMem;

        for (uint32_t grp = 0; grp < nGroups; grp++) {
            group.group = grp;
            group.groupIdx = grp * groups;
            group.groupEnd = std::min(group.groupIdx + groups, jobs);

            if (queue.nThreads <= 1)
                group.Execute();
            else
                queue.queues[queue.nextQueue.fetch_add(1) % queue.nThreads].Push(group);
        }

        if (queue.nThreads > 1)
            queue.wake.notify_all();
    }

    size_t DispatchGroups(size_t jobs, size_t groups) {
        return (jobs + groups - 1) / groups;
    }


    bool IsWorking(const ExecutionContext &context) {
        return context.count.load() > 0;
    }

    void WaitFor(const ExecutionContext &context) {
        if (IsWorking(context)) {
            PriorityQueue& queue = internalState.priorities[static_cast<size_t>(context.priority)];

            queue.wake.notify_all();

            queue.Execute(queue.nextQueue.fetch_add(1) % queue.nThreads);

            while (IsWorking(context))
                std::this_thread::yield();
        }
    }








}
