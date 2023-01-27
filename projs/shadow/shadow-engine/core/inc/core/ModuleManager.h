#ifndef UMBRA_MODULEMANAGER_H
#define UMBRA_MODULEMANAGER_H

#include <memory>
#include <list>
#include "Module.h"

namespace ShadowEngine {

    struct ModuleHolder{
    public:
        std::shared_ptr<Module> module;
        std::string domain;

        bool disabled = false;

        // Reinterpret this module as if it were a Renderer Module.
        // A shortcut for `std::static_pointer_cast<std::shared_ptr<RendererModule>>(ShadowEngine::ModuleManager::instance->GetModule("renderer"))
        std::shared_ptr<RendererModule> operator->() const { return std::static_pointer_cast<RendererModule>(module); }
/*
        ModuleHolder() {}
        ModuleHolder(std::shared_ptr<Module> module, std::string domain) : module(module), domain(domain) {}
        ModuleHolder(ModuleHolder& o) {
            this->module = o.module;
            this->domain = o.domain;
            this->disabled = o.disabled;
        }
*/
    };

    class ModuleManager {
    public:
        //Singleton stuff
        static API ModuleManager *instance;
        static ModuleManager* getInstance() { return instance; }


        std::vector<ModuleHolder> modules;
        ModuleHolder renderer;

        bool finalized = false;


        ModuleManager();
        ~ModuleManager();

        void PushModule(const std::shared_ptr<Module>& module, const std::string& domain);

        Module &GetModule(const std::string& name);

        template<typename T> requires std::is_base_of_v<Module, T>
        std::weak_ptr<T> GetModule() {
            for (auto &moduleHolder: modules) {
                if (moduleHolder.module->GetTypeId() == T::TypeId())
                    return std::weak_ptr<T>(std::dynamic_pointer_cast<T>(moduleHolder.module));
            }
            //SH_CORE_ERROR("Can't find the module {0}", T::Type()); TODO: add back when logger macros are added
            return std::weak_ptr<T>();
        }

        void RemoveModule(std::weak_ptr<Module> ptr);

        void SortDeps();

        void Init();

        void Finalise();

        void Update(int frame);

        void LateRender(VkCommandBuffer& commands, int frame);

        void OverlayRender();

        void Recreate();

        void Render(VkCommandBuffer& commands, int frame);

        void PreRender();

        void AfterFrameEnd();

        void Destroy();

        void Event(SDL_Event* evt);

        void dfs(ModuleHolder module, std::vector<ModuleHolder>& sorted);
    };
}

#endif //UMBRA_MODULEMANAGER_H
