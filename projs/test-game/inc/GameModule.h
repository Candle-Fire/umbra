#pragma once

#include "core/Module.h"

class GameModule : public ShadowEngine::Module {

    SHObject_Base(GameModule)

    std::string tets = "asdasd";
public:
    void PreInit() override;

    void Init() override;

    void Update() override;

    void Render() override;

    void LateRender() override;

    void AfterFrameEnd() override;

};