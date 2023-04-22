#pragma once

#include "core/Module.h"
#include "core/module-manager-v2.h"
#include "core/ShadowApplication.h"
#include "spdlog/spdlog.h"

class TestModule : public ShadowEngine::Module {
  SHObject_Base(TestModule)

  public:
    ~TestModule() override = default;

    explicit TestModule() : Module() {}

    void PreInit() override {
        spdlog::info("{0} PreInit", this->GetName());
    }

    void Init() override {
        //auto self = ShadowEngine::ModuleManager::instance->GetModule<TestModule>();
        ShadowEngine::ShadowApplication::Get().GetModuleManager().DeactivateModule(this);

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
};
