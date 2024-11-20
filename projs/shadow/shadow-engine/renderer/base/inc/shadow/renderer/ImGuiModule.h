#pragma once
#include <shadow/SHObject.h>
#include <shadow/core/Module.h>

#include "RendererEvents.h"
#include "RenderManager.h"


namespace SH::Renderer {

class ImGuiModule : public Module {
    SHObject_Base(ImGuiModule)

    std::weak_ptr<RenderManager> render_mgr;

public:
    void PreInit() override;
    void Init() override;
    void SDLEvent(SH::Events::SDLEvent& e);
    void LateRender(LateRender &e);
private:
    void InternalInit() const;
};

} // Renderer

