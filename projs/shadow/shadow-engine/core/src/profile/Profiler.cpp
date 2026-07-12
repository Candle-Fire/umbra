
#include <atomic>
#include <map>
#include <shadow/profile/Profiler.h>
#include <vector>
#include "shadow/assets/fs/iostream.h"
#include "shadow/core/Time.h"
#include "shadow/assets/management/synchronization.h"
#include <shadow/platform/Common.h>
#include "shadow/event-bus/events.h"

#ifdef _WIN32
#define INITGUID
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <evntcons.h>
#include <atomic>
#include <cstring>
#include <crtdbg.h>
#endif

namespace SH {
    /**
     * Tracks the Profiler through GPU transferrence events, and provides minimal insight into what the GPU is actually doing.
     * It lets us track things like the time spent on tasks, and how many primitive objects are created in doing that, but not much else.
     */
    struct GPUScope {
        struct Pair {
            size_t begin;
            size_t end;
        };

        GPUScope() : read(0), write(0) {
        }

        void begin(size_t time) {
            if (write - read == lengthOf(pairs)) read++;
            pairs[write & lengthOf(pairs)].begin = time;
        }

        void end(size_t time) {
            if (write - read == lengthOf(pairs)) read++;
            pairs[write & lengthOf(pairs)].end = time;
            write++;
        }

        std::string name;
        Pair pairs[100];
        uint32_t read;
        uint32_t write;
    };

    // Allows the Profiler to be used on multiple threads simultaneously, separating the data collected.
    // Every thread has its own set of open blocks, its own mutex, and its own visibility toggles.
    // Consider the ThreadContext as a horizontal bar on a profiler menu, showing what one thread is doing at any given time.
    struct ThreadContext {
        ThreadContext() : firstPage(nullptr), lastPage(nullptr) {
        }

        ~ThreadContext() {
            Page* page = firstPage;
            while (page) {
                Page* next = page->header.next;
                delete page;
                page = next;
            }
        }

        struct Block {
            uint32_t id;
            const char* name;
        };

        struct Page {
            struct Header {
                Page* next = nullptr;
                uint32_t size = 0;
            };

            Page() {
                memset(buffer, 0, 4096 - sizeof(header));
            }

            Header header;
            uint8_t buffer[4096 - sizeof(header)];
        };

        std::vector<Block> openBlocks;
        // Used as the data buffer.
        uint8_t buffer[sizeof(Page::buffer)];
        uint32_t bufferPtr = 0;

        Mutex mut;
        Page* firstPage = nullptr;
        Page* lastPage = nullptr;
        size_t pages = 0;

        std::string name;
        bool show = false;
        size_t threadID = 0;
    };

    // This ifdef contains a bunch of the infrastructure necessary for performance monitoring under Windows.
#ifdef _WIN32
#define SWITCH_OPCODE 36
#pragma pack(1)
    struct TraceProperties {
        EVENT_TRACE_PROPERTIES base;
        char name[sizeof(KERNEL_LOGGER_NAME) + 1];
    };
#pragma pack()

    struct ContextSwitch {
        uint32_t NewThreadID;
        uint32_t OldThreadID;
        int8_t NewThreadPriority;
        int8_t OldThreadPriority;
        uint8_t PreviousContextSwitchState;
        int8_t Padding;
        int8_t OldThreadWaitReason;
        int8_t OldThreadWaitMode;
        int8_t OldThreadState;
        int8_t OldThreadIdealProcessor;
        uint32_t NewThreadWaitTime;
        uint32_t Reserved;
    };

    struct TraceTask : Thread {
        TraceTask();

        int Run() override;
        static void callback(PEVENT_RECORD event);

        TRACEHANDLE handle;
    };
#else

    struct TraceTask {
        TraceTask() {
        }

        void destroy() {
        }

        int handle;
    };
    void CloseTrace(int) {
    }
#endif

    // The profiler instance.
    // Manages the profiler threads.
    static struct Instance {
        Instance() {
            startTrace();
        }

        ~Instance() {
            CloseTrace(task.handle);
            task.destroy();
            for (ThreadContext* ctx : contexts)
                delete ctx;
        }

        void startTrace() {
#ifdef _WIN32
            static TRACEHANDLE handle;
            static TraceProperties properties = {
                .base = {
                    .Wnode = {
                        .BufferSize = sizeof(properties),
                        .Guid = SystemTraceControlGuid,
                        .ClientContext = 1,
                        .Flags = WNODE_FLAG_TRACED_GUID,
                    },
                    .LogFileMode = EVENT_TRACE_REAL_TIME_MODE,
                    .EnableFlags = EVENT_TRACE_FLAG_CSWITCH,
                    .LoggerNameOffset = sizeof(properties.base)
                }
            };
            strcpy_s(properties.name, KERNEL_LOGGER_NAME);

            TraceProperties tmp = properties;
            // Start a trace that never starts, so we can see whether the real one will work.
            ControlTrace(NULL, KERNEL_LOGGER_NAME, &tmp.base, EVENT_TRACE_CONTROL_STOP);
            ULONG res = StartTrace(&handle, KERNEL_LOGGER_NAME, &properties.base);
            switch (res) {
            // If we get these errors, then we can't be pre-empted, so disable context switching on our thread.
            case ERROR_ALREADY_EXISTS:
            case ERROR_ACCESS_DENIED:
            case ERROR_BAD_LENGTH:
            default:
                contextSwitch = false;
                break;
            case ERROR_SUCCESS:
                contextSwitch = true;
                break;
            }

            static EVENT_TRACE_LOGFILE trace = {
                .LoggerName = (decltype(trace.LoggerName))KERNEL_LOGGER_NAME,
                .ProcessTraceMode = PROCESS_TRACE_MODE_RAW_TIMESTAMP | PROCESS_TRACE_MODE_REAL_TIME |
                PROCESS_TRACE_MODE_EVENT_RECORD,
                .EventRecordCallback = TraceTask::callback
            };

            task.handle = OpenTrace(&trace);
            task.Start("Profiler Trace", true);
#endif
        }

        // Get the thread context for our thread.
        // This is thread_local, so the initializer will run once per thread.
        // If the initializer has already run on this thread, ctx is cached, so it's just returned.
        ThreadContext* getThreadContext() {
            thread_local ThreadContext* ctx = [&]() {
                auto* newCtx = new ThreadContext();
                newCtx->threadID = pthread_self();
                MutexGuard lock(mut);
                contexts.push_back((newCtx));
                return newCtx;
            }();

            return ctx;
        };

        std::vector<uint32_t> scopeStack;
        std::vector<GPUScope> scopes;
        // A list of counters on the current thread.
        std::vector<Profiler::CounterData> counters;
        // Child threads of the current.
        std::vector<ThreadContext*> contexts;
        Mutex mut;
        Timer timer;
        // Whether the current thread is paused.
        bool paused = false;
        // Whether the current thread can be pre-empted.
        bool contextSwitch = false;
        // The time the thread was paused, if it was.
        size_t pausedTime = 0;
        size_t lastFrameElapsed = 0;
        size_t lastFrameTime = 0;

        // The ID of the fiber waiting on the current.
        std::atomic<uint32_t> fiberWaiting = 0;
        // The actual Thread doing work for the current profiler.
        TraceTask task;
        ThreadContext global;
    } gInstance;

    template <bool lock>
    static void Flush(ThreadContext& ctx) {
        if constexpr (lock) ctx.mut.enter();
        // Circular buffer of 500 x 4k chunks (2MB)
        if (ctx.pages < 500) {
            auto* newPage = new ThreadContext::Page;
            memcpy(newPage->buffer, ctx.buffer, ctx.bufferPtr);
            newPage->header.size = ctx.bufferPtr;
            if (!ctx.firstPage) ctx.firstPage = newPage;
            if (ctx.lastPage) ctx.lastPage->header.next = newPage;
            ctx.lastPage = newPage;
            ctx.pages++;
        } else {
            auto* newPage = ctx.firstPage;
            ctx.firstPage = newPage->header.next;
            newPage->header.next = nullptr;

            memcpy(newPage->buffer, ctx.buffer, ctx.bufferPtr);
            newPage->header.size = ctx.bufferPtr;

            ctx.lastPage->header.next = newPage;
            ctx.lastPage = newPage;
        }

        ctx.bufferPtr = 0;

        if constexpr (lock) ctx.mut.exit();
    }

    /**
     * Append data to the ThreadContext.
     * The timestamp may be in the past, and the UI will update accordingly.
     * The EventType tells what the event actually was.
     * val is placed in the ThreadContext's buffer along with the event header.
     * Resizing is done on the buffer if necessary.
     */
    template <bool lock, typename T>
    void Write(ThreadContext& ctx, size_t timestamp, Profiler::EventType type, const T& val) {
        // Verify that we have enough space in the temporary buffer.
        constexpr uint32_t bytesToWrite = sizeof(T) + sizeof(Profiler::Event);
        assert(bytesToWrite <= lengthOf(ctx.buffer));

        if constexpr (lock) ctx.mut.enter();
        if (ctx.bufferPtr + bytesToWrite > lengthOf(ctx.buffer))
            Flush<!lock>(ctx);

        if (gInstance.paused && timestamp > gInstance.pausedTime) return;

        Profiler::Event* header = (Profiler::Event*)(ctx.buffer + ctx.bufferPtr);
        header->type = type;
        header->size = bytesToWrite;
        header->time = timestamp;

        memcpy((uint8_t*) header + sizeof(*header), &val, sizeof(val));
        ctx.bufferPtr += bytesToWrite;

        if constexpr (lock) ctx.mut.exit();
    };

    /**
     * Append arbitrary data to the ThreadContext.
     * The EventType tells what the event actually was.
     * size bytes of data is placed in the ThreadContext's buffer along with the event header.
     * Resizing is done on the buffer if necessary.
     */
    template <bool lock>
    void Write(ThreadContext& ctx, Profiler::EventType type, const uint8_t* data, int size) {
        // Verify that we have enough space in the temporary buffer.
        const uint32_t bytesToWrite = size + sizeof(Profiler::Event);
        assert(bytesToWrite <= lengthOf(ctx.buffer));

        if constexpr (lock) ctx.mut.enter();
        if (ctx.bufferPtr + bytesToWrite > lengthOf(ctx.buffer))
            Flush<!lock>(ctx);

        if (gInstance.paused) return;

        Profiler::Event* header = (Profiler::Event*)(ctx.buffer + ctx.bufferPtr);
        header->type = type;
        header->size = bytesToWrite;
        header->time = SH::Timer::getTimestamp();

        memcpy((uint8_t*) header + sizeof(*header), data, size);
        ctx.bufferPtr += bytesToWrite;

        if constexpr (lock) ctx.mut.exit();
    }

    void ExportThreadStrings(std::vector<const char*>* strings, const ThreadContext& ctx) {
        ThreadContext::Page* page = ctx.firstPage;
        while (page) {
            uint32_t idx = 0;
            while (idx < page->header.size) {
                Profiler::Event header;
                memcpy(&header, &page->buffer[idx], sizeof(header));

                switch (header.type) {
                case Profiler::EventType::Begin: {
                    Profiler::Block b;
                    memcpy(&b, &page->buffer[idx + sizeof(Profiler::Event)], sizeof(b));
                    if (std::find(strings->begin(), strings->end(), b.name) == strings->end())
                        strings->push_back(b.name);
                    break;
                }
                case Profiler::EventType::Int: {
                    Profiler::Int i;
                    memcpy(&i, &page->buffer[idx + sizeof(Profiler::Event)], sizeof(i));
                    if (std::find(strings->begin(), strings->end(), i.key) == strings->end())
                        strings->push_back(i.key);
                    break;
                }
                default: break;
                }

                idx += header.size;
            }
            page = page->header.next;
        }
    }

    void ExportStrings(OutputMemoryStream& blob) {
        std::vector<const char*> strings;

        ExportThreadStrings(&strings, gInstance.global);
        for (const ThreadContext* ctx : gInstance.contexts)
            ExportThreadStrings(&strings, *ctx);

        blob.write(strings.size());
        for (const char* iter : strings) {
            blob.write((size_t) iter);
            blob.write(iter, strlen(iter) + 1);
        }
    }

    void ExportThread(OutputMemoryStream& blob, ThreadContext& thread) {
        MutexGuard lock(thread.mut);

        Flush<false>(thread);
        blob.write(thread.name);
        blob.write((uint32_t) thread.threadID);
        blob.write(thread.show);
        uint32_t size = 0;
        const ThreadContext::Page* page = thread.firstPage;
        // Iterate first to get the number of pages.
        while (page) {
            size += page->header.size;
            page = page->header.next;
        }
        blob.write(size);
        // Iterate again to write each page size out.
        page = thread.firstPage;
        while (page) {
            blob.write(page->buffer, page->header.size);
            page = page->header.next;
        }

        ifsystem(,_CrtDumpMemoryLeaks());
    }

    void Profiler::Export(OutputMemoryStream& blob) {
        MutexGuard lock(gInstance.mut);

        blob.write(1);
        blob.write((uint32_t) gInstance.counters.size());
        blob.write(gInstance.counters.data(), gInstance.counters.size() * sizeof(Profiler::CounterData));
        blob.write((uint32_t) gInstance.contexts.size());
        ExportThread(blob, gInstance.global);
        for (ThreadContext* ctx : gInstance.contexts)
            ExportThread(blob, *ctx);

        ExportStrings(blob);
    }

#ifdef _WIN32

    TraceTask::TraceTask() : Thread() {
    }

    // The profiler's Thread code.
    int TraceTask::Run() {
        ProcessTrace(&handle, 1, nullptr, nullptr);
        return 0;
    }

    // Called by Windows when the trace is pre-empted by a context switch.
    void TraceTask::callback(PEVENT_RECORD event) {
        if (event->EventHeader.EventDescriptor.Opcode != SWITCH_OPCODE) return;
        if (sizeof(ContextSwitch) != event->UserDataLength) return;

        const ContextSwitch* cs = reinterpret_cast<ContextSwitch*>(event->UserData);
        Profiler::ContextSwitchRecord rec = {
            .oldThreadID = cs->OldThreadID,
            .newThreadID = cs->NewThreadID,
            .timestamp = static_cast<size_t>(event->EventHeader.TimeStamp.QuadPart),
            .cause = static_cast<uint8_t>(cs->OldThreadWaitReason)
        };

        Write<true>(gInstance.global, rec.timestamp, Profiler::EventType::ContextSwitch, rec);
    };
#endif


    size_t Profiler::MakeCounter(const char* key, float min) {
        MutexGuard lock(gInstance.mut);
        CounterData& c = gInstance.counters.emplace_back();
        memcpy(&c.name, key, strlen(key));
        c.min = min;
        return gInstance.counters.size() - 1;
    }

    void Profiler::PushCounter(size_t counter, float val) {
        Profiler::Counter r = {
            .ID = counter,
            .val = val
        };

        Write<true>(gInstance.global, SH::Timer::getTimestamp(), EventType::Counter, r);
    }

    void Profiler::PushInt(const char* key, int val) {
        ThreadContext* ctx = gInstance.getThreadContext();
        Int r = {
            .key = key,
            .val = val
        };

        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Int, r);
    };

    void Profiler::PushString(const char* value) {
        ThreadContext* ctx = gInstance.getThreadContext();
        Write<false>(*ctx, EventType::String, (uint8_t*)value, strlen(value) + 1);
    }

    static constexpr char ToChar(float x) {
        return (char)((x) * std::numeric_limits<char>::max());
    }

    void Profiler::BlockColor(DirectX::XMFLOAT3 c) {
        const uint32_t color = 0xFF000000 + ToChar(c.x) + (ToChar(c.y) << 8) + (ToChar(c.z) << 16);
        ThreadContext* ctx = gInstance.getThreadContext();
        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Color, color);
    }

    static std::atomic<uint32_t> lastBlockID = 0;

    void Profiler::Begin(const char* name) {
        ThreadContext::Block r = {
            .id = lastBlockID++,
            .name = name
        };
        ThreadContext* ctx = gInstance.getThreadContext();
        ctx->openBlocks.emplace_back(r);
        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Begin, r);
    }

    static GPUScope& GetGPUScope(const char* name, uint32_t& scopeID) {
        for (GPUScope& s : gInstance.scopes) {
            if (s.name == name) {
                // Workaround for not having the index during ranged-for.
                scopeID = &s - &gInstance.scopes[0];
                return s;
            }
        }

        GPUScope& s = gInstance.scopes.emplace_back();
        s.name = name;
        scopeID = gInstance.scopes.size() - 1;
        return s;
    }

    void Profiler::BeginGPU(const char* name, size_t time, size_t profiler) {
        GPU data = {
            .timestamp = time,
            .profiler = profiler
        };
        memcpy(&data.name, name, strlen(name));

        Write<true>(gInstance.global, SH::Timer::getTimestamp(), EventType::GPUBegin, data);

        MutexGuard lock(gInstance.global.mut);
        uint32_t scope;
        GetGPUScope(name, scope).begin(time);
        gInstance.scopeStack.push_back(scope);
    }

    void Profiler::GPUStats(size_t primitive) {
        Write<true>(gInstance.global, SH::Timer::getTimestamp(), EventType::GPUStats, primitive);
    }

    void Profiler::EndGPU(size_t time) {
        Write<true>(gInstance.global, SH::Timer::getTimestamp(), EventType::GPUEnd, time);

        MutexGuard lock(gInstance.global.mut);
        if (gInstance.scopeStack.empty()) return;
        const uint32_t scope = gInstance.scopeStack.back();
        gInstance.scopeStack.pop_back();
        gInstance.scopes[scope].end(time);
    }

    size_t Profiler::CreateLink() {
        std::atomic<size_t> counter = 0;
        return counter++;
    }

    void Profiler::Link(size_t profiler) {
        ThreadContext* ctx = gInstance.getThreadContext();
        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Link, profiler);
    }

    float Profiler::GetLastFrameDuration() {
        return float(gInstance.lastFrameElapsed / double(GetFrequency()));
    }

    void Profiler::PreFiberSwitch() {
        ThreadContext* ctx = gInstance.getThreadContext();
        // End all active blocks before we change out
        while (!ctx->openBlocks.empty()) {
            Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::End, 0);
            ctx->openBlocks.pop_back();
        }
    }

    void Profiler::Task(size_t signal) {
        Job r = {
            .id = lastBlockID.fetch_add(1),
            .signal = signal
        };
        ThreadContext* ctx = gInstance.getThreadContext();

        ctx->openBlocks.emplace_back();
        ctx->openBlocks.back().id = r.id;
        ctx->openBlocks.back().name = "Task";
        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Job, r);
    }

    void Profiler::Trigger(size_t signal) {
        ThreadContext* ctx = gInstance.getThreadContext();
        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Signal, signal);
    }

    Profiler::FiberData Profiler::BeginFiberWait(size_t signal, bool isMutex) {
        FiberWait r = {
            .ID = gInstance.fiberWaiting++,
            .signal = signal,
        };

        ThreadContext* ctx = gInstance.getThreadContext();
        FiberData res = {
            .ID = r.ID,
            .count = static_cast<uint32_t>(ctx->openBlocks.size()),
            .signal = signal
        };
        for (uint32_t i = 0; i < std::min({res.count, lengthOf(res.blocks), static_cast<uint32_t>(ctx->openBlocks.size()) }); i++)
            res.blocks[i] = ctx->openBlocks[i].id;

        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::FiberWait, r);
        return res;
    }

    void Profiler::EndFiberWait(const SH::Profiler::FiberData& data) {
        ThreadContext* ctx = gInstance.getThreadContext();
        FiberWait r = {
            .ID = data.ID,
            .signal = data.signal,
        };

        const size_t time = SH::Timer::getTimestamp();
        Write<false>(*ctx, time, EventType::FiberWake, r);

        const uint32_t count = data.count;
        for (size_t i = 0; i < count; i++) {
            const uint32_t id = i < lengthOf(data.blocks) ? data.blocks[i] : -1;
            ctx->openBlocks.push_back( { id, "N/A" });
            Write<false>(*ctx, time, EventType::Continue, id);
        }
    }

    void Profiler::End() {
        ThreadContext* ctx = gInstance.getThreadContext();
        if (!ctx->openBlocks.empty()) {
            ctx->openBlocks.pop_back();
            Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::End, 0);
        }
    }

    size_t Profiler::GetFrequency() {
        static size_t frequency;
#ifdef _WIN32
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        frequency = f.QuadPart;
#else
        frequency = 1'000'000'000;
#endif

        return frequency;
    }

    bool Profiler::IsContextSwitchEnabled() {
        return gInstance.contextSwitch;
    }

    void Profiler::Frame() {
        const size_t n = SH::Timer::getTimestamp();
        if (gInstance.lastFrameTime != 0)
            gInstance.lastFrameElapsed = n - gInstance.lastFrameTime;
        gInstance.lastFrameTime = n;
        Write<true>(gInstance.global, SH::Timer::getTimestamp(), EventType::Frame, 0);
    }

    void Profiler::PushMutex(size_t id, size_t beginStart, size_t beginEnd, size_t exitStart, size_t exitEnd) {
        ThreadContext* ctx = gInstance.getThreadContext();

        Mutex mut = {
            .mutex = id,
            .enterStart = beginStart,
            .enterEnd = beginEnd,
            .exitStart = exitStart,
            .exitEnd = exitEnd
        };

        Write<false>(*ctx, SH::Timer::getTimestamp(), EventType::Mutex, mut);
    }


    void Profiler::ShowInProfiler(bool show) {
        ThreadContext* ctx = gInstance.getThreadContext();
        MutexGuard lock(ctx->mut);

        ctx->show = show;
    }

    void Profiler::SetThreadName(const char* name) {
        ThreadContext* ctx = gInstance.getThreadContext();
        MutexGuard lock(ctx->mut);

        ctx->name = name;
    }

}
