#include "core/ModuleManager.h"
#include "spdlog/spdlog.h"

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
    ModuleHolder r {module, domain};
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
    SortDeps();

    for (auto& module : modules)
    {
        if(!module.disabled)
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

#define Iterate(It) It.begin(), It.end()

auto ModulePredicate(std::string target){
    return [target](const auto &item) { return target == item->GetId();};
}

void ShadowEngine::ModuleManager::dfs(ModuleHolder module, std::vector<ModuleHolder>& sorted) {
    //visited[v] = true;
    for (auto u : module->GetDependencies()) {
        if (!std::any_of( Iterate(sorted),ModulePredicate(u)) && u != module->GetId())
        {
            auto it = std::find_if(Iterate(this->modules), ModulePredicate(u));

            if(it != modules.end())
                dfs(*it, sorted);
            else
                spdlog::info("Module {0} is missing, required by {1}", u ,module->GetId());

        }
    }
    sorted.push_back(module);
}

void ShadowEngine::ModuleManager::SortDeps() {
    int module_count = this->modules.size();

    for (auto i : this->modules) {
        spdlog::info("Module {0} is present", i->GetId());
    }


    std::vector<ModuleHolder> sorted;
    sorted.clear();

    for (auto i : this->modules) {
        if (!std::any_of(Iterate(sorted),ModulePredicate(i->GetId())))
            dfs(i, sorted);
    }

    //reverse(sorted.begin(), sorted.end());

    this->modules = sorted;

}

