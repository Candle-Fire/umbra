
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/
#pragma once

#include <stddef.h>
#include <vector>


#define SONIC_MODULE(name)                                                      \
constexpr static size_t const ModuleID = std::hash( #name );                    \
virtual size_t GetModuleID() const override final { return name::ModuleID; }

namespace sonic {
    /**
     * A way to reference singleton plugin modules using the Reflection system.
     */
    class Module {
    public:

        Module() = default;
        Module(Module const&) = default;
        virtual ~Module() = default;

        Module& operator=(Module const&) = default;

        virtual size_t GetModuleID() const = 0;
    };

    /**
     * Keeps track of all active Modules in use.
     ! The SONIC_MODULE macro must be used for the GetModule generic function to work.
     */
    class ModuleRegistry {
    public:

        ModuleRegistry() {}
        ~ModuleRegistry();

        void RegisterModule(Module* mod);
        void UnregisterModule(Module* mod);

        template<typename T>
        inline T* GetModule() const {
            for (auto mod : RegisteredModules)
                if (mod->GetModuleID() == T::ModuleID)
                    return reinterpret_cast<T*>(mod);

            return nullptr;
        }

        std::vector<Module*>   RegisteredModules;
    };
}
