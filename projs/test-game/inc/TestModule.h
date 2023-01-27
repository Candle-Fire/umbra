#pragma once

#include "core/Module.h"
#include "core/ModuleManager.h"
#include "spdlog/spdlog.h"

class TestModule : public ShadowEngine::Module {
SHObject_Base(TestModule)

public:
    ~TestModule() override = default;

    explicit TestModule() : Module("module:/test-game/TestModule") {}

    std::vector<std::string> GetDependencies() override {
        return { "module:/test-game/GameModule" };
    }

    void PreInit() override {
        spdlog::info("{0} PreInit", this->GetName());
    }

    void Init() override {
        auto self = ShadowEngine::ModuleManager::instance->GetModule<TestModule>();
        ShadowEngine::ModuleManager::instance->RemoveModule(self);

        spdlog::info("{0} Init", this->GetName());
    }

    void Update(int frame) override {
        spdlog::info("{0} Update", this->GetName());
    }

    void Recreate() override {

    }

    void PreRender() override {

    }

    void Render(VkCommandBuffer &commands, int frame) override {

    }

    void LateRender(VkCommandBuffer &commands, int frame) override {

    }

    void OverlayRender() override {

    }

    void AfterFrameEnd() override {

    }

    void Destroy() override {

    }

    void Event(SDL_Event *e) override {

    }

    std::string GetName() override {
        return "Test Module";
    }

};
