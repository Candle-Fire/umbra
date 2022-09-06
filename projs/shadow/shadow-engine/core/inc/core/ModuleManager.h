#ifndef UMBRA_MODULEMANAGER_H
#define UMBRA_MODULEMANAGER_H

#include <memory>
#include <list>
#include "Module.h"

#ifdef EXPORTING_SH_ENGINE
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

namespace ShadowEngine {

    struct ModuleRef{
    public:
        std::shared_ptr<Module> module;
        std::string domain;

        std::shared_ptr<RendererModule> operator->() const { return std::static_pointer_cast<RendererModule>(module); }
    };

    class ModuleManager {
    public:
        static API ModuleManager *instance;
        static ModuleManager* getInstance() { return instance; }

        std::list<ModuleRef> modules;
        ModuleRef renderer;

        ModuleManager();

        ~ModuleManager();

        void PushModule(const std::shared_ptr<Module>& module, const std::string& domain);

        Module &GetModule(const std::string& name);

        template<typename T>
        T *GetModuleByType() {
            for (auto &module: modules) {
                if (module.module->GetTypeId() == T::TypeId())
                    return dynamic_cast<T *>(module.module.get());
            }
            //SH_CORE_ERROR("Can't find the module {0}", T::Type());
            return nullptr;
        }

        void Init();

        void Update(int frame);

        void LateRender(VkCommandBuffer& commands, int frame);

        void OverlayRender();

        void Recreate();

        void Render(VkCommandBuffer& commands, int frame);

        void PreRender();

        void AfterFrameEnd();

        void Destroy();

        void Event(SDL_Event* evt);
    };
}

#endif //UMBRA_MODULEMANAGER_H
