#pragma once

#include "core/Module.h"
#include "EditorWindow.h"

namespace ShadowEngine::Editor {

    class EditorModule : public ShadowEngine::Module {
    SHObject_Base(EditorModule)

        std::vector<std::shared_ptr<EditorWindow>> windows;

    public:
        EditorModule() {}

        void PreInit() override {

        }

        void Init() override;

        void Update(int frame) override {

        }

        void Recreate() override {

        }

        void PreRender() override {

        }

        void Render(VkCommandBuffer &commands, int frame) override {

        }

        void LateRender(VkCommandBuffer &commands, int frame) override {

        }

        void OverlayRender() override;

        void AfterFrameEnd() override {

        }

        void Destroy() override {

        }

        void Event(SDL_Event *e) override {

        }
    };
}