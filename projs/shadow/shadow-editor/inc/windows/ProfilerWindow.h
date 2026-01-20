#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "shadow/assets/fs/iostream.h"
#include "shadow/assets/resource/Resource.h"
#include "shadow/profile/Profiler.h"

namespace SH::Editor::Profiler {

    struct ThreadData {
        struct Rect {
            uint32_t id;
            size_t startTime;
            size_t endTime;
            uint32_t line;
            uint32_t firstProp;
            uint32_t properties;
        };

        struct Signal {
            size_t time;
            uint32_t signal;
            uint32_t line;
        };

        struct FiberWait : SH::Profiler::FiberWait {
            size_t time;
            uint32_t line;
            bool isStarter;
        };

        struct GPU {
            std::string name;
            size_t profileLink;
            size_t startTime;
            size_t endTime;
            size_t prims;
            uint32_t line;
        };

        struct Mutex : SH::Profiler::Block {
            uint32_t line;
        };

        ThreadData(size_t threadID, const char* name, bool show) : id(threadID), name(name), show(show), y(0) {}

        uint32_t id;
        const char* name;
        bool show;
        bool open = false;
        uint32_t lines = 0;
        size_t lastSwitchTime = 0;
        size_t lastSwitchOffset = 0;
        float y;

        std::vector<uint32_t> props;
        std::vector<FiberWait> waits;
        std::vector<Rect> rects;
        std::vector<Signal> signals;
        std::vector<size_t> frames;
        std::vector<uint32_t> switches;
        std::vector<Mutex> muts;
        std::vector<GPU> gpuBlocks;
    };

    struct ThreadContextReader {

        ThreadContextReader(const uint8_t* ptr) {
            InputMemoryStream reader(ptr, 1024 * 9);

            // Can't directly return the ptr, so..
            std::string n = reader.readString();
            char* str = new char[n.size() + 1];
            strcpy(str, n.c_str());
            name = str;
            reader.read(id);
            show = reader.read<uint8_t>();
            reader.read(bufferSize);
            buffer = (uint8_t*) reader.getData() + reader.pos();
        }

        uint8_t* next() { return buffer + bufferSize; }

        const char* name;
        uint32_t id;
        uint32_t bufferSize;
        bool show;
        uint8_t* buffer;
    };

    struct Counter {
        struct Record {
            size_t time;
            float value;
        };

        std::vector<Record> records;
        std::string name;
        float min = FLT_MAX;
        float max = -FLT_MAX;
    };

    struct Block {
        Block() {
            job.signal = 0;
        }

        const char* name;
        size_t link = 0;
        uint32_t color = 0xFFDDDDDD;
        SH::Profiler::Job job;
    };

    ThreadContextReader getThreadReader();

    void prepare();

    void timeline(float fromX, float toX, float top, float bottom, size_t startTime);
    void drawCounters(float fromX, float toX);
    void drawResources();
    void drawThreads(ThreadContextReader& ctx, float fromX, float toX);
    void drawContextSwitches(float fromX, float toX);
    void drawGPU();
    void drawGPUGraph(size_t time, float fromX, float toX);
    void drawFrameMarkers(float fromx, float toX, float y, size_t& startTime);
    void drawFlameGraph();

    void draw();
    void makeVisible();
    void toggleUI();

    void insertStrings();

    void load();
    void save();

    template <typename T>
    void forEachThread(const T& t);

    void cacheBlocks();

    float getXFor(size_t time, float fromX, float toX);

}
