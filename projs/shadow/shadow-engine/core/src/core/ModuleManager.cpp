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
= default;

void ShadowEngine::ModuleManager::PushModule(const std::shared_ptr<Module>& module, const std::string& domain)
{
    ModuleHolder r = {module, domain};
    modules.emplace_back(r);
    if (domain == "renderer")
        renderer = r;
    module->PreInit();
}

ShadowEngine::Module& ShadowEngine::ModuleManager::GetModule(const std::string& name)
{
    for (auto& module : modules)
    {
        if (module.module->GetName() == name)
            return *module.module;
    }
    //SH_ASSERT(false, "Can't find the module");
    throw std::runtime_error("Can't find the module");
}

void ShadowEngine::ModuleManager::RemoveModule(std::weak_ptr<Module> ptr) {
    if(finalized) return;

    for (auto& moduleHolder: this->modules) {
        if(moduleHolder.module == ptr.lock()){
            moduleHolder.disabled = true;
        }
    }
}

void ShadowEngine::ModuleManager::Init()
{
    for (auto& module : modules)
    {
        module.module->Init();
    }

    this->Finalise();
}

void ShadowEngine::ModuleManager::Finalise() {

    modules.erase(
            std::remove_if(modules.begin(), modules.end(),
                           [](ModuleHolder& a){return a.disabled;})
    );

    this->finalized = true;
}

void ShadowEngine::ModuleManager::Destroy()
{
    for (auto& module : modules)
    {
        module.module->Destroy();
    }
}


void ShadowEngine::ModuleManager::PreRender()
{
    for (auto& module : modules)
    {
        module.module->PreRender();
    }
}

void ShadowEngine::ModuleManager::Event(SDL_Event* evt)
{
    for (auto& module : modules)
    {
        module.module->Event(evt);
    }
}

void ShadowEngine::ModuleManager::Update(int frame)
{
    for (auto& module : modules)
    {
        module.module->Update(frame);
    }
}

void ShadowEngine::ModuleManager::LateRender(VkCommandBuffer& commands, int frame)
{
    for (auto& module : modules)
    {
        module.module->LateRender(commands, frame);
    }
}

void ShadowEngine::ModuleManager::Render(VkCommandBuffer& commands, int frame)
{
    for (auto& module : modules)
    {
        module.module->Render(commands, frame);
    }
}

void ShadowEngine::ModuleManager::OverlayRender()
{
    for (auto& module : modules)
    {
        module.module->OverlayRender();
    }
}

void ShadowEngine::ModuleManager::Recreate()
{
    for (auto& module : modules)
    {
        module.module->Recreate();
    }
}


void ShadowEngine::ModuleManager::AfterFrameEnd()
{
    for (auto& module : modules)
    {
        module.module->AfterFrameEnd();
    }
}

