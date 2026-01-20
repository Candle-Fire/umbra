#include <shadow/jobs/Job.h>
#include <algorithm>
#include <shadow/core/Time.h>

#include "spdlog/spdlog.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace SH::Jobs {


    struct State {
        size_t nCores = 0;
        PriorityQueue priorities[static_cast<uint32_t>(Priority::SIZE)];
        std::atomic_bool alive { true };

        void Destroy() {
            if (isShuttingDown()) return;

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

    void init(uint32_t maxThreads) {
        // Don't reinit if we're already doing something, that's BAD
        if (internalState.nCores > 0) return;
        maxThreads = std::max(std::thread::hardware_concurrency(), maxThreads);

        spdlog::flush_on(spdlog::level::info);
        spdlog::info("Preparing SH::Jobs, with {} max threads.", maxThreads);


        SH::Timer timer;

        internalState.nCores = std::thread::hardware_concurrency();

        for (size_t pri = 0; pri < static_cast<size_t>(Priority::SIZE); pri++) {
            const Priority p = (Priority) pri;
            PriorityQueue& queue = internalState.priorities[pri];

            queue.nThreads =
                p == Priority::HIGHEST ? internalState.nCores - 1 :
                p == Priority::LOW ? internalState.nCores - 2 :
                /* p == Priority::STREAM ? */ 1;
            queue.nThreads = std::clamp((uint32_t) queue.nThreads, 1u, maxThreads);
            queue.queues.reset(new TaskQueue[queue.nThreads]);
            queue.threads.reserve(queue.nThreads);

            for (size_t i = 0; i < 256; i++)
                queue.threadLookup[i] = i % queue.nThreads;

            for (size_t thread = 0; thread < queue.nThreads; thread++) { // thread < 1; thread++) {
                //spdlog::debug("Setting up thread {} for priority {}.", thread, p == Priority::HIGHEST ? "highest" : p == Priority::LOW ? "lowest" : "stream");
                std::thread& worker = queue.threads.emplace_back([thread, &queue, p] {
#ifdef WIN32
                    HANDLE wHandle = GetCurrentThread();
                    // Set Thread Affinity within the thread, because pthread_gethandle isn't happy

                    int core = p == Priority::STREAM ? internalState.nCores - 1 - thread : thread + 1;
                    DWORD_PTR affinity = 1 << core;
                    DWORD_PTR res = SetThreadAffinityMask(wHandle, affinity);
                    if (!res) {
                        LPVOID lpMsgBuf;
                        DWORD dw = GetLastError();

                        if (FormatMessage(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL,
                            dw,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            (LPTSTR) &lpMsgBuf,
                            0, NULL) == 0) {
                                ExitProcess(dw);
                            }
                        spdlog::error("SetThreadAffinity returned {}", (const char*)lpMsgBuf);
                    }
                    assert(res);
                    BOOL priorityRes = SetThreadPriority(wHandle,
                        p == Priority::HIGHEST ? THREAD_PRIORITY_NORMAL :
                        p == Priority::LOW ? THREAD_PRIORITY_LOWEST :
                        /* p = Priority::STREAMING ? */ THREAD_PRIORITY_BELOW_NORMAL);

                    assert(priorityRes);
                    std::string threadName =
                        std::string("SH::Job::") +
                            (p == Priority::HIGHEST ? "" :
                            p == Priority::LOW ? "Low::" :
                            /*p == Priority::STREAMING*/ "Stream::") +
                            std::to_string(thread);
                    pthread_setname_np(pthread_self(), threadName.c_str());
#endif

                    //spdlog::debug("Worker thread {} starting loop", thread);
                    while (internalState.alive.load()) {
                        queue.execute(thread);
                        std::unique_lock lock(queue.wakeLocker);
                        queue.wake.wait(lock);
                    }
                });

#ifdef WIN32
#elif defined(PLATFORM_LINUX)

                // TODO: cpuset for pthread_setaffinity, pthread_setname.
                // TODO: cannot write it under windows because symbols are missing
#endif

            }
        }

        spdlog::debug("SH::Jobs initialized in {}ms, utilizing {} cores, with {} high-priority threads, {} low-priority threads, and {} asset-streaming threads.", std::to_string(timer.elapsedMillis()), std::to_string(internalState.nCores), std::to_string(getThreadsOfPriority(Priority::HIGHEST)), std::to_string(getThreadsOfPriority(Priority::LOW)), std::to_string(getThreadsOfPriority(Priority::STREAM)));
    }

    void destroy() {
        internalState.Destroy();
    }

    bool isShuttingDown() {
        return internalState.alive.load(std::memory_order_relaxed) == false;
    }

    size_t getThreadsOfPriority(Priority p) {
        return internalState.priorities[static_cast<size_t>(p)].nThreads;
    }

    void run(ExecutionContext &context, const std::function<void(TaskArguments)> &task) {
        PriorityQueue& queue = internalState.priorities[static_cast<size_t>(context.priority)];
        context.count.fetch_add(1);

        TaskGroup group { task, &context, 0, 0, 1, 0 };

        if (queue.nThreads <= 1) {
            group.execute();
            return;
        }

        queue.nextTaskQueue().push(group);
        queue.wake.notify_one();
    }

    void run(const std::function<void()>& task) {
        ExecutionContext context;
        PriorityQueue& queue = internalState.priorities[static_cast<size_t>(Priority::HIGHEST)];
        std::function const fixTask = [task](TaskArguments) { task(); };

        TaskGroup group { fixTask, &context, 0, 0, 1, 0 };

        if (queue.nThreads <= 1) {
            group.execute();
            return;
        }

        queue.nextTaskQueue().push(group);
        queue.wake.notify_one();
    }


    void dispatch(ExecutionContext &context, size_t jobs, size_t groups, const std::function<void(TaskArguments)> &task, size_t sharedMem) {
        if (jobs == 0 || groups == 0) return;

        PriorityQueue& queue = internalState.priorities[static_cast<size_t>(context.priority)];
        const uint32_t nGroups = dispatchGroup(jobs, groups);

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
                group.execute();
            else
                queue.nextTaskQueue().push(group);
        }

        if (queue.nThreads > 1)
            queue.wake.notify_all();
    }

    size_t dispatchGroup(size_t jobs, size_t groups) {
        return (jobs + groups - 1) / groups;
    }


    bool isWorking(const ExecutionContext &context) {
        return context.count.load() > 0;
    }

    void waitFor(const ExecutionContext &context) {
        if (isWorking(context)) {
            PriorityQueue& queue = internalState.priorities[static_cast<size_t>(context.priority)];

            queue.wake.notify_all();

            queue.execute(queue.nextIndex());

            while (isWorking(context)) {
                std::unique_lock<std::mutex> lock(queue.waitLocker);
                if (isWorking(context))
                    queue.wait.wait(lock, [&context] { return !isWorking(context); });
            }
        }
    }
}
