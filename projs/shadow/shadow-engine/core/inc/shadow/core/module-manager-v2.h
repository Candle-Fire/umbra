#pragma once

#include <string>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"
#include "dylib.hpp"

#include "Module.h"
#include "shadow/exports.h"
#include "shadow/core/PathID.h"

using ID = std::string;

/*
 * ########################
 * ######## DANGER ########
 * ########################
*/
/// @brief This is a helper for creating module specific entry points. This is dodgy.
/// Only change if you know what you are doing.
/// This creates a C linkage exported function. The function gets a pointer to a shared_ptr and puts a new pointer to the module into it.
/// It is used by the module manager to create modules.
#define MODULE_ENTRY(name, shortname) extern "C" { void EXPORT shortname ## _entry(std::shared_ptr<name>* ptr){*ptr = std::make_shared<name>();} }

/// @brief Helper macro for standard funcs iterate
// TODO: Move this to a global helper file
#define ITERATE(It) (It).begin(), (It).end()

namespace SH {

  /// @brief Contains metadata about modules
  struct ModuleDescriptor {
    /// @brief The logical ID of the module eg.: "module:/renderer/vulkan"
    ID id;
    /// @brief The human readable name for the module
    std::string name;
    /// @brief The class that should be created for this module
    std::string class_name;
    /// @brief The assembly this module is from
    SH::Path assembly;

    /// @brief The modules that need to ble loaded for this module to run
    std::vector<ID> dependencies;
  };

  /// @brief Structure to hold modules and the corresponding descriptions for them
  struct ModuleHolder {
    /// @brief Desricptor for the module
    ModuleDescriptor descriptor;
    /// @brief Reference to the module
    std::shared_ptr<SH::Module> module;
    /// @brief whether this module is enabled
    /// This could mean that it was not enabled for running
    /// or it was deactivated while setup (eg.: not able to run on the platform)
    bool enabled = true;
  };

  /// @brief Central manager for runtime loaded engine modules
  class API ModuleManager {

    /// @brief List of all of the known modules
    /// These modules can be active, inactive or not even loaded
    std::vector<ModuleHolder> modules;

    /// @brief Indicates when modules can't be removed anymore
    /// This limitation doesn't mean the module's pointers won't change
    /// but the dependency resolving isn't run again.
    /// (This will potentially change)
    bool finalized = false;

    /// @brief Tries to construct a module
    /// @param holder The module holder that needs to be load
    void LoadModule(ModuleHolder &holder);

    /// @brief Helper predicate function for finding modules based on id
    /// @param target The module ID that is being searched
    /// @return Gives back a function lambda that is usable as a predicate in std functions.
    static std::function<bool(const ModuleHolder &)> ModulePredicate(const std::string &target) {
        return [target](const auto &item) { return target == item.descriptor.id; };
    }

    /// @brief Depth first sort helper
    /// @param module_holder The module that is currently being processed
    /// @param sorted The sorted modules
    void Dfs(ModuleHolder &module_holder, std::vector<ModuleHolder> &sorted);

    /// @brief Sorts the modules based on their dependencies
    void SortModules();

    void PrintModuleInfo();

  public:

    /// @brief Registers a new module descriptor
    /// @param descriptor The module descriptor to register
    void AddDescriptors(ModuleDescriptor descriptor) { modules.push_back({descriptor = descriptor}); }

    /// @brief [BEWARE DRAGONS] Instructs the module manager to load modules form an assembly's default entry point
    /// @param id The id of the assembly to load from
    ///
    /// The function loads in the given Assembly and calls the "assembly_entry" function
    /// This function can register the modules that are in the Assembly for later creation
    ///
    void LoadModulesFromAssembly(const SH::Path &id);

    /// @brief Runs the module startup
    ///
    /// This runs the module sorting and loads assemblies for them. PreInit and Init is run by it
    void Init();

    /// @brief Deactivaes the module
    ///
    /// @param force [BEWARE DRAGONS] This forces the deactivation even if the module stack has been finalized
    void DeactivateModule(Module *module_ptr, bool force = false);

    /// @brief Returns a module by it's logical ID and casts it to T
    ///
    /// @param id The module id to find
    template<class T>
    std::weak_ptr<T> GetById(const std::string &id) {
        spdlog::trace("Searching for module {0} of type {1}", id, T::Type());
        for (const auto &i : this->modules) {
            spdlog::trace("T:{0} id:{1}, test: {2}, {3}", T::Type(), id, i.module->GetType(), i.descriptor.id);
            if (i.enabled && i.descriptor.id == id && dynamic_cast<T *>(i.module.get()) != nullptr) {
                return std::dynamic_pointer_cast<T>(i.module);
            }
        }
        throw std::logic_error("Module " + id + " could not be found");
    }

    /// @brief Reruns the full list of known modules
    const std::vector<ModuleHolder> &GetModules() { return this->modules; }

    /// @brief Returns if the module is active
    /// @param id The id of the module to check
    bool IsModuleActive(const ID &id);

    /// @brief Runs the callback function if the given module is active
    void IfModuleActive(const ID &id, const std::function<void()> &callback);

    /// @brief Runs the callback function if the given module is active
    template<class T>
    void IfModuleActive(const ID &id, const std::function<void(T &)> &callback) {
        if (IsModuleActive(id)) {
            auto m = GetById<T>(id);
            callback(m);
        }
    }

    // Event functions

    void Update(int frame);
  };

}