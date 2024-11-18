#pragma once
#include <shadow/SHObject.h>
#include <shadow/core/Module.h>


namespace SH::Renderer {

class ImGuiModule : public Module {
    SHObject_Base(ImGuiModule)

public:
    void PreInit() override;
    void Init() override;
};

} // Renderer

