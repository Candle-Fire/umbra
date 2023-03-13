#pragma once

#include <string>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"
#include "dylib.hpp"

#include "Module.h"

using ID = std::string;

/*
 * ########################
 * ######## DANGER ########
 * ########################
*/
/// @brief This is a helper for creating module specific entry points. This is doggy.
/// Only change if you know what you are doing.
#define MODULE_ENTRY(name, shortname) extern "C" { std::shared_ptr<name> shortname ## _entry(){return std::make_shared<name>();} }

/// @brief Helper macro for standard funcs iterate
// TODO: Move this to a global helper file
#define ITERATE(It) (It).begin(), (It).end()

namespace ShadowEngine {

    /// @brief Represents known DLL files that could be loaded during runtime
    struct Assembly {
        /// @brief The logical ID of the DLL file (eg: "assembly:/core")
        ID id;
        /// @brief The system path to the DLL file being referenced
        std::string path;
        /// @brief The dylib pointer to the loaded
        dylib *lib = nullptr;
    };

    /// @brief Contains metadata about modules
    struct ModuleDescriptor {
        /// @brief The logical ID of the module eg.: "module:/renderer/vulkan"
        ID id;
        /// @brief The human readable name for the module
        std::string name;
        /// @brief The class that should be created for this module
        std::string class_name;
        /// @brief The assembly this module is from
        ID assembly;

        /// @brief The modules that need to ble loaded for this module to run
        std::vector<ID> dependencies;
    };

    /// @brief Structure to hold modules and the corresponding descriptions for them
    struct ModuleHolder {
        /// @brief Desricptor for the module
        ModuleDescriptor descriptor;
        /// @brief Reference to the module
        std::shared_ptr<ShadowEngine::Module> module;
        /// @brief weather this module is enabled
        /// This could mean that it was not enabled for running
        /// or it was deactivated while setup (eg.: not able to run on the platform)
        bool enabled;
    };


    /// @brief Central manager for runtime loaded engine modules
    class ModuleManager {

        /// @brief List of all of the known assemblies
        /// These can be loaded in to get modules
        std::vector<Assembly> assemblies;

        /// @brief List of all of the known modules
        /// These modules can be active, inactive or not even loaded
        std::vector<ModuleHolder> modules;

        /// @brief Indicates when modules can't be removed anymore
        /// This limitation doesn't mean the module's pointers won't change
        /// but the dependency resolving isn't run again.
        /// (This will potentially change)
        bool finalized = false;

        /// @brief Loads an assembly into the running application
        /// @param id The logical ID of the assembly to laod ("assembly:/core")
        void LoadAssembly(const std::string &id);

        /// @brief Finds an assembly based on the Id
        /// @param id The ID of the assembly to find ("assembly:/core")
        /// @return Reference to the assembly
        /// @details It errors out if the id is not known
        Assembly &GetAssembly(const ID &id) {
            return *std::find_if(ITERATE(this->assemblies), [id](const Assembly &a) { return a.id == id; });
        }

        /// @brief Tries to construct a module
        /// @param holder The module holder that needs to be load
        void LoadModule(ModuleHolder &holder);

        /// @brief Helper predicate function for finding modules based on id
        /// @param target The module ID that is being searched
        /// @return Gives back a function lambda that is usable as a predicate in std functions.
        static std::function<bool(const ModuleHolder&)> ModulePredicate(const std::string &target) {
            return [target](const auto &item) { return target == item.descriptor.id; };
        }

        /// @brief Depth first sort helper
        /// @param module_holder The module that is currently being processed
        /// @param sorted The sorted modules
        void Dfs(const ModuleHolder &module_holder, std::vector<ModuleHolder> &sorted);

        /// @brief Sorts the modules based on their dependencies
        void SortModules();

    public:

        /// @brief Registers a new module descriptor
        /// @param descriptor The module descriptor to register
        void AddDescriptors(ModuleDescriptor descriptor) { modules.push_back({descriptor = descriptor}); }

        /// @brief Adds a new assembly to the known Assembly
        /// This doesn't load the assembly only makes it known
        /// @param assembly The Assembly to add
        void AddAssembly(const Assembly& assembly) { this->assemblies.push_back(assembly); }

        /// @brief [BEWARE DRAGONS] Instructs the module manager to load modules form an assembly's default entry point
        /// @param id The id of the assembly to load from
        ///
        /// The function loads in the given Assembly and calls the "assembly_entry" function
        /// This function can register the modules that are in the Assembly for later creation
        ///
        void LoadModulesFromAssembly(const std::string &id);

        /// @brief Runs the module startup
        /// 
        /// This runs the module sorting and loads assemblies for them. PreInit and Init is run by it
        void Init();

        /// @brief Deactivaes the module
        ///
        /// @param force [BEWARE DRAGONS] This forces the deactivation even if the module stack has been finalized
        void DeactivateModule(Module* module_ptr, bool force = false){
            if(!this->finalized || force){
                auto m = std::find_if(ITERATE(this->modules), [&](const ModuleHolder& item){
                    return item.module.get() == module_ptr;
                });
                if(m != this->modules.end()){
                    m->enabled = false;
                }
            }
        }

        /// @brief Returns a module by it's logical ID and casts it to T
        /// 
        /// @param id The module id to find
        template<class T>
        std::weak_ptr<T> GetById(const std::string &id) {
            for (const auto &i: this->modules) {
                if (i.enabled && i.descriptor.id == id) {
                    return std::dynamic_pointer_cast<T>(i.module);
                }
            }
            throw std::exception(); //TODO
        }

        /// @brief Retruns the full list of known modules
        const std::vector<ModuleHolder>& GetModules(){ return this->modules; }

        /// @brief Returns if the module is active
        /// @param id The id of the module to check
        bool IsModuleActive(const ID& id){
            auto m = std::find_if(ITERATE(this->modules), ModulePredicate(id));
            return m != this->modules.end() && m->enabled;
        }

        /// @brief Runs the calback function if the given module is active
        void IfModuleActive(const ID& id, const std::function<void()>& callback){
            if(IsModuleActive(id))
                callback();
        }

        /// @brief Runs the calback function if the given module is active
        template<class T>
        void IfModuleActive(const ID& id, const std::function<void(T&)>& callback){
            if(IsModuleActive(id)) {
                auto m = GetById<T>(id);
                callback(m);
            }
        }

        // Event functions

        void Update(int frame);

        void LateRender(VkCommandBuffer &commands, int frame);

        void OverlayRender();

        void Recreate();

        void Render(VkCommandBuffer &commands, int frame);

        void PreRender();

        void AfterFrameEnd();

        void Event(SDL_Event *evt);
    };

}