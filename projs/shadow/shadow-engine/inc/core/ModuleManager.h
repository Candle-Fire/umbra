#ifndef UMBRA_MODULEMANAGER_H
#define UMBRA_MODULEMANAGER_H

#include <memory>
#include <list>
#include "Module.h"

namespace ShadowEngine {

    struct ModuleRef{
    public:
        Module* module;
        std::string domain;
    };

    class ModuleManager {
    public:
        static ModuleManager *instance;

        std::list<ModuleRef> modules;

        ModuleManager();

        ~ModuleManager();

        void PushModule(Module *module, std::string domain);

        Module &GetModule(std::string name);

        template<typename T>
        T *GetModuleByType() {
            for (auto &module: modules) {
                if (module.module->GetTypeId() == T::TypeId())
                    return dynamic_cast<T *>(module.module);
            }
            //SH_CORE_ERROR("Can't find the module {0}", T::Type());
            return nullptr;
        }

        void Init();

        void Update();

        void LateRender();

        void Render();

        void AfterFrameEnd();
    };
}

#endif //UMBRA_MODULEMANAGER_H
