//
// Created by dpete on 02/09/2022.
//

#include <iostream>

#include "core/ShadowApplication.h"
#include "GameModule.h"
#include "core/ShadowApplication.h"
#include "vlkx/vulkan/VulkanManager.h"

extern "C" __declspec(dllexport) void shadow_main(ShadowEngine::ShadowApplication* app) {

    std::cout << "HIIII from a loaded dll weeeeeee!!!" << std::endl;

    app->GetModuleManager().PushModule(std::make_shared<GameModule>(), "game");

    if(app == &ShadowEngine::ShadowApplication::Get()){
        std::cout << "They are the same!!!" << std::endl;
    }

    printf("dll side: %p \n", VulkanManager::getInstance());
    printf("dll next ID: %llu \n", ShadowEngine::SHObject::GenerateId());

}
