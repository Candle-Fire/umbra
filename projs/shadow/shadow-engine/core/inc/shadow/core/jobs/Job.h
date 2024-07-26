#pragma once
#include <atomic>
#include <cstdint>
#include <functional>


/**
 * A threaded task/job system.
 * Can run multiple things in parallel, with automatic dependency ordering.
 * Create a TaskContext and call the static execute or dispatch methods to run stuff off-thread.
 */
namespace SH::Jobs {
    /**
     * The data that a task needs in order to handle itself and its relation with other tasks that may be running simultaneously.
     * Most data relates to task groups, but the idx means this is easier set per task.
     */
    struct TaskArguments {
        uint32_t idx;           // The index of the current task. When using dispatch, this is the iterator index, denoting the nth task that was started.
        uint32_t group;         // The index of the current task group. When using dispatch, this is the group index, denoting the nth dispatch that was started.
        uint32_t groupIdx;      // The index of the current task in the group.
        bool first;             // Whether this is the first task in its' group.
        bool last;              // Whether this is the last task in its' group.
        void* data;             // Data shared between a whole group.
    };

    /**
     * A task priority.
     * Higher priority tasks will pause lower priority tasks when all threads are being utilized.
     */
    enum class Priority {
        HIGHEST,        // Default priority.
        LOW,            // Low priority tasks. Do not pause regular tasks.
        STREAM,         // Resource streaming, background tasks that never take priority over others
        SIZE
    };

    /**
     * The context of a thread. Stores priority and how many tasks are waiting for this to finish.
     */
    struct ExecutionContext {
        std::atomic<uint32_t> count { 0 };
        Priority priority;
    };

    /**
     * A collection of tasks, a member of a task queue, running on any thread, with ordering data.
     * All tasks in the group run the same code, but with different indexes passed via the Arguments.
     */
    struct TaskGroup {
        std::function<void(TaskArguments)> func;                                // The actual code the task executes
        ExecutionContext* context;                                              // A context, with priority
        uint32_t group;                                                         // The ID of the group that the thread is a part of
        uint32_t groupIdx;                                                      // The index of the current task into the group it is a part of
        uint32_t groupEnd;                                                      // The index of the last task in the current group - may be the same as the group index for groups of 1, or the last task in a group.
        uint32_t shared;                                                        // The size of the shared memory buffer for the current task group.

        /**
         * Execute all tasks in the group between the current idx and the last task in the group.
         * func will be passed the indexes via the TaskArguments.
         * Shared memory buffers will be allocated and passed if requested.
         */
        inline void Execute() {
            TaskArguments args;
            args.group = group;
            if (shared > 0) {
                thread_local static std::vector<uint8_t> sharedData(shared);
                args.data = sharedData.data();
            } else
                args.data = nullptr;
            for (size_t i = groupIdx; i < groupEnd; i++) {
                args.idx = i;
                args.groupIdx = i - groupIdx;
                args.first = i == groupIdx;
                args.last = i == groupEnd - 1;

                func(args);
            }

            context->count.fetch_sub(1);
        }
    };

    /**
     * A list of Task Groups, representing the groups assigned to a single thread.
     */
    struct TaskQueue {
        std::deque<TaskGroup> groups;
        std::mutex lock;

        inline void Push(const TaskGroup& tsk) {
            std::scoped_lock locker(lock);
            groups.push_back(tsk);
        }

        inline bool Pop(TaskGroup& tsk) {
            std::scoped_lock locker(lock);
            if (groups.empty()) return false;
            tsk = std::move(groups.front());
            groups.pop_front();
            return true;
        }
    };

    /**
     * A list of Task Queues, representing all the tasks set for a given priority.
     */
    struct PriorityQueue {
        size_t nThreads;                            // The number of threads allocated to this priority.
        std::vector<std::thread> threads;           // The actual threads executing code for this priority.
        std::unique_ptr<TaskQueue[]> queues;        // A list of TaskQueues, one for every thread.
        std::atomic<uint32_t> nextQueue { 0 };      // The next TaskQueue to process.
        std::condition_variable wake;               // The condition that, when passed, will wake any queues depending on the current.
        std::mutex locker;

        /**
         * Process all waiting threads.
         * @param firstQueue the first queue to perform work on, enabling skipping queues already processed.
         */
        inline void Execute(size_t firstQueue) {
            TaskGroup task;
            for (size_t i = 0; i < nThreads; i++) {
                TaskQueue& tasks = queues[firstQueue & nThreads];
                while (tasks.Pop(task))
                    task.Execute();
                firstQueue++;
            }
        }
    };

    /**
     * Initialize the Task management system.
     * @param maxThreads the maximum number of threads that can be dedicated entirely to running Tasks.
     */
    void Init(uint32_t maxThreads = 0xFFFFFFFF);

    /**
     * Prepare the Task management system for engine shutdown.
     */
    void Destroy();

    /**
     * Fetch the number of Threads currently running, for the given priority.
     * @param p the priority of threads to check
     * @return The number of threads currently running a task with the given priority.
     */
    size_t GetThreadsOfPriority(Priority p = Priority::HIGHEST);

    /**
     * Run the given task off-thread, with the given priority.
     * @param context a task context, with priority
     * @param task the function to run off-thread
     */
    void Run(ExecutionContext& context, const std::function<void(TaskArguments)>& task);

    /**
     * Dispatch a set of jobs, running the same code with different arguments.
     * The arguments that change are the task index and group index.
     * Usually use this where iterating over a static list and performing an expensive task on every object within.
     * @param context a task context, with priority
     * @param jobs the number of jobs to create, total. Usually, max index of the list being processed
     * @param groups the number of jobs to run per thread. Usually, jobs / max_threads if jobs > max_threads, or 5 otherwise.
     * @param task the function to run per index
     * @param sharedMem the size of a shared memory buffer to allocate for every group.
     */
    void Dispatch(ExecutionContext& context, size_t jobs, size_t groups, const std::function<void(TaskArguments)>& task, size_t sharedMem = 0);

    /**
     * Fetch the number of groups to be created for a given number of jobs, for groups of a given size.
     * @param jobs the total number of jobs to create
     * @param groups the number of jobs to put in each group
     * @return The number of groups to create to fit all jobs
     */
    size_t DispatchGroups(size_t jobs, size_t groups);

    /**
     * Check whether any threads are working for the given context.
     * @param context a task context, with priority
     * @return Whether any threads are working for this context
     */
    bool IsWorking(const ExecutionContext& context);

    /**
     * Wait for the given context to stop running.
     * The current thread will be added to the thread pool for jobs.
     * @param context a task context, with priority
     */
    void WaitFor(const ExecutionContext& context);
}
