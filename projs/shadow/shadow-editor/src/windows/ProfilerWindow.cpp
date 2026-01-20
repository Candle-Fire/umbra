#include <map>
#include <windows/ProfilerWindow.h>

#include "imgui.h"
#include "shadow/platform/Common.h"

bool isOpen = false;
bool showContextSwitches = false;
bool showMutexes = true;
bool showFrames = true;

uint32_t frameIdx = 0;
size_t end;
size_t range;

std::map<uint32_t, SH::Editor::Profiler::ThreadData> threads;
std::vector<SH::Editor::Profiler::Counter> counters;
std::map<uint32_t, SH::Editor::Profiler::Block> blocks;

SH::OutputMemoryStream data;

struct {
    uint32_t frame = 0;
    size_t link;
} hoveredLink;

struct {
    uint32_t frame = 0;
    uint32_t id = 0;
    ImVec2 pos;
    uint32_t signal = 0;
} hoveredFiber;

struct {
    uint32_t frame = 0;
    uint32_t signal = 0;
    ImVec2 pos;
} hoveredSignal;

struct {
    uint32_t frame = 0;
    size_t id = 0;
} hoveredMutex;

struct {
    uint32_t frame = 0;
    uint32_t signal = 0;
    ImVec2 pos;
} hoveredTask;

struct {
    uint32_t frame = 0;
    uint32_t id = 0;
} hoveredBlock;

namespace SH::Editor::Profiler {

    const char* getContexSwitchReasonString(uint8_t reason) {
        const char* reasons[] = {
                     "Executive"		   ,
                     "FreePage"		   ,
                     "PageIn"		   ,
                     "PoolAllocation"   ,
                     "DelayExecution"   ,
                     "Suspended"		   ,
                     "UserRequest"	   ,
                     "WrExecutive"	   ,
                     "WrFreePage"	   ,
                     "WrPageIn"		   ,
                     "WrPoolAllocation" ,
                     "WrDelayExecution" ,
                     "WrSuspended"	   ,
                     "WrUserRequest"	   ,
                     "WrEventPair"	   ,
                     "WrQueue"		   ,
                     "WrLpcReceive"	   ,
                     "WrLpcReply"	   ,
                     "WrVirtualMemory"  ,
                     "WrPageOut"		   ,
                     "WrRendezvous"	   ,
                     "WrKeyedEvent"	   ,
                     "WrTerminated"	   ,
                     "WrProcessInSwap"  ,
                     "WrCpuRateControl" ,
                     "WrCalloutStack"   ,
                     "WrKernel"		   ,
                     "WrResource"	   ,
                     "WrPushLock"	   ,
                     "WrMutex"		   ,
                     "WrQuantumEnd"	   ,
                     "WrDispatchInt"	   ,
                     "WrPreempted"	   ,
                     "WrYieldExecution" ,
                     "WrFastMutex"	   ,
                     "WrGuardedMutex"   ,
                     "WrRundown"		   ,
                     "MaximumWaitReason",
                 };
        if (reason >= (uint8_t) lengthOf(reasons)) return "Unknown";
        return reasons[reason];
             }

    const char* toString(Asset::Resource::State state) {
        switch (state) {
        case Asset::Resource::State::EMPTY: return "Empty";
        case Asset::Resource::State::FAILED: return "Failed";
        case Asset::Resource::State::READY: return "Ready";
                 }
        return "Unknown";
             }


    template <typename T>
    void read(const ThreadContextReader& ctx, uint32_t p, T& value) {
        const uint8_t* buf = ctx.buffer;
        const uint32_t buf_size = ctx.bufferSize;
        const uint32_t l = p % buf_size;
        if (l + sizeof(value) <= buf_size) {
                     memcpy(&value, buf + l, sizeof(value));
            return;
                 }

                 memcpy(&value, buf + l, buf_size - l);
                 memcpy((uint8_t*)&value + (buf_size - l), buf, sizeof(value) - (buf_size - l));
             }

    void read(const ThreadContextReader& ctx, uint32_t p, uint8_t* ptr, int size) {
        const uint8_t* buf = ctx.buffer;
        const uint32_t buf_size = ctx.bufferSize;
        const uint32_t l = p % buf_size;
        if (l + size <= buf_size) {
                     memcpy(ptr, buf + l, size);
            return;
                 }

                 memcpy(ptr, buf + l, buf_size - l);
                 memcpy(ptr + (buf_size - l), buf, size - (buf_size - l));
             }

    template <typename T>
    void overwrite(ThreadContextReader& ctx, uint32_t p, const T& v) {
                 p = p % ctx.bufferSize;
        if (ctx.bufferSize - p >= sizeof(v)) {
                     memcpy(ctx.buffer + p, &v, sizeof(v));
                 } else {
            const uint32_t prefix_len = ctx.bufferSize - p;
                     memcpy(ctx.buffer + p, &v, prefix_len);
                     memcpy(ctx.buffer, ((uint8_t*) &v) + prefix_len, sizeof(v) - prefix_len);
                 }
             }

    ThreadContextReader getThreadReader() {
        InputMemoryStream blob(data);
        blob.skip(sizeof(uint32_t));
        blob.skip(blob.read<uint32_t>() + sizeof(Profiler::Counter));
        blob.skip(sizeof(uint32_t));
        return ThreadContextReader(data.dataMut() + blob.pos());
    }

    void makeVisible() {
        isOpen = true;
    }


    void draw() {
        ProfileFunction();

        if (!isOpen) return;

        if (ImGui::Begin("Profiler"), &isOpen) {
            static uint32_t tab = 0;

            auto makeTab = [](const char* label, bool selected) {
                if (selected) {
                    ImGui::Dummy(ImVec2(10, 1));
                    ImGui::SameLine();
                }

                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[selected ? ImGuiCol_TabActive : ImGuiCol_Tab]);
                bool res = false;
                if (ImGui::Button(label, ImVec2(-1, 0)))
                    res = true;
                ImGui::PopStyleColor();
                return res;
            };

            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            if (ImGui::BeginChild("Selector", ImVec2(150, 0))) {
                if (makeTab("Flame Graph", tab == 0)) tab = 0;
                if (makeTab("GPU", tab == 1)) tab = 1;
                if (makeTab("Resources", tab == 2)) tab = 2;
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::BeginChild("Content", ImVec2(0,0))) {
                switch (tab) {
                case 0: drawFlameGraph(); break;
                case 1: drawGPU(); break;
                case 2: drawResources(); break;
                }
            }
            ImGui::EndChild();
            ++frameIdx;
        }
        ImGui::End();
    }

    void drawFlameGraph() {

    }

    void drawGPU() {

    }

    void drawResources() {

    }

}
