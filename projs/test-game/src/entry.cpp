//
// Created by dpete on 02/09/2022.
//

#include <iostream>

#include "core/ShadowApplication.h"
#include "GameModule.h"
#include "core/ShadowApplication.h"

extern "C" __declspec(dllexport) void shadow_main(ShadowEngine::ShadowApplication* app) {

    std::cout << "HIIII from a loaded dll weeeeeee!!!" << std::endl;

    app->GetModuleManager().PushModule(new GameModule(), "game");

}
