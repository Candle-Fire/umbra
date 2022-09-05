//
// Created by dpete on 2022-07-06.
//

#include "core/ModuleManager.h"

#include <stdexcept>

ShadowEngine::ModuleManager* ShadowEngine::ModuleManager::instance = nullptr;

ShadowEngine::ModuleManager::ModuleManager()
{
    if (instance != nullptr)
    {
        //ERROR
    }
    instance = this;
}

ShadowEngine::ModuleManager::~ModuleManager()
{
}

void ShadowEngine::ModuleManager::PushModule(std::shared_ptr<Module> module, const std::string domain)
{
    ModuleRef r = {module, domain};
    modules.emplace_back(r);
    module->PreInit();
}

ShadowEngine::Module& ShadowEngine::ModuleManager::GetModule(std::string name)
{
    for (auto& module : modules)
    {
        if (module.module->GetName() == name)
            return *module.module;
    }
    //SH_ASSERT(false, "Can't find the module");
    throw std::runtime_error("Can't find the module");
}

void ShadowEngine::ModuleManager::Init()
{
    for (auto& module : modules)
    {
        module.module->Init();
    }
}

void ShadowEngine::ModuleManager::Update()
{
    for (auto& module : modules)
    {
        module.module->Update();
    }
}

void ShadowEngine::ModuleManager::LateRender()
{
    for (auto& module : modules)
    {
        module.module->LateRender();
    }
}

void ShadowEngine::ModuleManager::Render()
{
    for (auto& module : modules)
    {
        module.module->Render();
    }
}

void ShadowEngine::ModuleManager::AfterFrameEnd()
{
    for (auto& module : modules)
    {
        module.module->AfterFrameEnd();
    }
}
