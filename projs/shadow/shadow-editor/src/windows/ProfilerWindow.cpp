#include <map>
#include <windows/ProfilerWindow.h>

#include "imgui.h"
#include "shadow/entitiy/editor/HierarchyWindow.h"
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

    template <typename Func>
    void forEachThread(const Func& func) {
        if (data.empty()) return;

        InputMemoryStream blob(data);
        uint32_t version = blob.read<uint32_t>();
        uint32_t counters = blob.read<uint32_t>();
        blob.skip(counters * sizeof(Profiler::Counter));
        uint32_t count = blob.read<uint32_t>();
        uint8_t* iter = (uint8_t*) blob.skip(0);

        ThreadContextReader thread(iter);
        func(thread);

        iter = thread.next();
        for (uint32_t i = 0; i < count; i++) {
            ThreadContextReader reader(iter);
            func(reader);
            iter = reader.next();
        }
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

    void load() {

    }

    void save() {

    }

    void insertStrings() {

    }

    void prepare() {

    }

    void cacheBlocks() {

    }

    void snapshot() {
        data.clear();
        // SH::Profiler::Serialize(data);
        insertStrings();
        prepare();
        cacheBlocks();
    }

    void drawCounters(float fromX, float toX) {

    }

    void drawThread(ThreadContextReader& ctx, float fromX, float toX) {

    }

    void drawContextSwitches(float fromX, float toX) {

    }

    void drawFrameMarkers(float fromx, float toX, float y, size_t& startTime) {

    }

    void drawGPU(float fromX, float toX) {

    }

    void timeline(float fromX, float toX, float top, float bottom, size_t startTime) {

    }


    void drawFlameGraph() {
        if (ImGui::Button(ICON_FA_DOWNLOAD))
            snapshot();

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_COGS)) ImGui::OpenPopup("Advanced Profiler Functions");
        if (ImGui::BeginPopup("Advanced Profiler Functions")) {
            if (ImGui::MenuItem("Load Saved Profile")) load();
            if (ImGui::MenuItem("Save Current Profile")) save();
            ImGui::Checkbox("Show Frame Markers", &showFrames);
            ImGui::Checkbox("Show Mutex Markers", &showMutexes);
            ImGui::Checkbox("Show Context Switch Markers", &showContextSwitches);
            ImGui::Text("Zoom: %f", range / double(100000));
            if (ImGui::MenuItem("Reset Zoom")) range = 100000;

            if (ImGui::BeginMenu("Threads")) {
                size_t threadCtr = 0;
                forEachThread([&](const ThreadContextReader& ctx) {
                    auto thread = threads.find(ctx.id);
                    if (thread != threads.end()) {
                        ImGui::Checkbox(std::string(ctx.name).append(" (").append(std::to_string(ctx.id)).append(")").c_str(), &thread->second.show);
                        threadCtr++;
                    }
                });
                if (!threadCtr) {
                    ImGui::Text("No profiler information for any threads loaded.");
                    ImGui::Text("Try using the snapshot (download) button to the left of this menu.");
                    ImGui::Text("Alternatively, load a saved profile.");
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        if (data.empty()) return;

        size_t freq = SH::Profiler::GetFrequency();
        ImGui::SameLine();
        ImGui::Text("%.3f ms", 1000 * float(range / double(freq)));
        float timelineY = ImGui::GetCursorScreenPos().y;
        // Vertical spacer between buttons, text, and the actual content.
        ImGui::Dummy(ImVec2(-1, ImGui::GetTextLineHeightWithSpacing()));

        const float fromY = ImGui::GetCursorScreenPos().y;
        const float fromX = ImGui::GetCursorScreenPos().x;
        const float toX = fromX + ImGui::GetContentRegionAvail().x;
        size_t viewStart = end - range;
        size_t timelineStart = viewStart;

        if (ImGui::BeginChild("CPU GPU")) {
            ImDrawList* list = ImGui::GetWindowDrawList();
            list->ChannelsSplit(2);

            drawCounters(fromX, toX);
            forEachThread([&](ThreadContextReader& ctx) { drawThread(ctx, fromX, toX); });
            drawContextSwitches(fromX, toX);
            drawFrameMarkers(fromX, toX, fromY, timelineStart);
            drawGPU(fromX, toX);

            if (ImGui::IsMouseHoveringRect(ImVec2(fromX, fromY), ImVec2(toX, ImGui::GetCursorScreenPos().y))) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    end -= size_t(ImGui::GetIO().MouseDelta.x / (toX - fromX)) * range;
                    cacheBlocks();
                }

                // Zoom
                if (ImGui::GetIO().KeyCtrl) {
                    size_t cursor = (((ImGui::GetMousePos().x - fromX) / (toX - fromX)) * range) + viewStart;
                    if (ImGui::GetIO().MouseWheel > 0 && range > 1) {
                        range >>= 1;
                        end = ((end - cursor) >> 1) + cursor;
                    } else if (ImGui::GetIO().MouseWheel < 0) {
                        range <<= 1;
                        end = ((end - cursor) << 1) + cursor;
                    }
                    cacheBlocks();
                }
            }
            list->ChannelsMerge();
        }
        ImGui::EndChild();

        timeline(fromX, toX, timelineY, ImGui::GetCursorScreenPos().y, timelineStart);
    }

    void drawGPU() {

    }

    void drawResources() {

    }

}
