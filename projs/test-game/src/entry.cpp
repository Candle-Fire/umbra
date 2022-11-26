//
// Created by dpete on 02/09/2022.
//

#include <iostream>

#include "core/ShadowApplication.h"
#include "GameModule.h"
#include "core/ShadowApplication.h"
#include "vlkx/vulkan/VulkanModule.h"

extern "C" __declspec(dllexport) void shadow_main(ShadowEngine::ShadowApplication* app) {
    app->GetModuleManager().PushModule(std::make_shared<GameModule>(), "game");
}
