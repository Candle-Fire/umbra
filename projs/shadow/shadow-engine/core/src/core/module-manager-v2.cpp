#pragma execution_character_set("utf-8")

#include "core/module-manager-v2.h"
#include <ranges>

namespace ShadowEngine {

    void ModuleManager::LoadModule(ModuleHolder &holder) {
        auto assembly = GetAssembly(holder.descriptor.assembly);
        const std::string &symbolName = holder.descriptor.class_name + "_entry";

        if (!assembly.lib->has_symbol(symbolName)) {
            spdlog::error("❌ Could not find the entry for module \"{0}\"", holder.descriptor.id);
            holder.enabled = false;
            return;
        }
        try {
            auto module_init = assembly.lib->get_function<std::shared_ptr<ShadowEngine::Module>()>(symbolName);

            holder.module = module_init();
        }
        catch (std::exception &e) {
            spdlog::error("❌ Error while running the entry for module \"{0}\" Error: {1}", holder.descriptor.id,
                          e.what());
            holder.enabled = false;
            return;
        }

        holder.enabled = true;
    }

    void ModuleManager::Init() {

        for (const auto &i: this->modules) {
            spdlog::debug("\"{0}\" is registered", i.descriptor.id);
        }

        //Sort
        this->SortModules();

        spdlog::debug("Sorted order:");
        for (const auto &i: this->modules) {
            spdlog::debug("\"{0}\" is registered", i.descriptor.id);
        }

        //Load
        for (auto &i: this->modules) {
            spdlog::info("Loading {0}", i.descriptor.id);
            if (std::any_of(ITERATE(this->assemblies), [i](Assembly a) {
                return a.id == i.descriptor.assembly && a.lib != nullptr;
            })) {
                spdlog::debug("✅ Assembly \"{0}\" is already loaded", i.descriptor.assembly);
            } else {
                spdlog::info("➕ Loading assembly \"{0}\"", i.descriptor.assembly);
                this->LoadAssembly(i.descriptor.assembly);
            }

            this->LoadModule(i);
            if (i.enabled)
                spdlog::info("Module {0}({1}) created", i.module->GetName(), i.descriptor.id);
        }

        this->SortModules();

        //PreInit
        spdlog::info("Running PreInit");
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                try{
                    holder.module->PreInit();
                }
                catch (const std::exception& e) {
                    spdlog::error("❌ Error while running PreInit for module \"{0}\" Error: {1}", holder.descriptor.id,
                                  e.what());
                }
            }
        }

        this->SortModules();

        spdlog::info("Running Init");
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                try {
                    holder.module->Init();
                }
                catch (const std::exception& e) {
                    spdlog::error("❌ Error while running Init for module \"{0}\" Error: {1}", holder.descriptor.id,
                                  e.what());
                }
            }
        }

        //Sort

        this->finalized = true;
    }

    void ModuleManager::LoadAssembly(const std::string &id) {
        auto assembly = std::find_if(ITERATE(this->assemblies), [&](const auto &item) {
            return item.id == id;
        });
        if (assembly == this->assemblies.end()) {
            spdlog::error("No such assembly: {0}", id);
            return;
        }


        dylib *dllptr;

        try {
            dllptr = new dylib(
                "./",
                assembly->path,
                assembly->type == AssemblyType::LIB ? dylib::add_filename_decorations : dylib::no_filename_decorations);
        }
        catch (std::exception &e) {
            spdlog::error(e.what());
            exit(1);
        }

        assembly->lib = dllptr;
    }

    void ModuleManager::Dfs(ModuleHolder &module_holder, std::vector<ModuleHolder> &sorted) {
        for (auto u: module_holder.descriptor.dependencies) {
            if (!std::ranges::any_of(sorted, ModulePredicate(u)) && u != module_holder.descriptor.id) {
                auto it = std::ranges::find_if(this->modules, ModulePredicate(u));

                if (it != modules.end()) {
                    if(!it->enabled){
                        spdlog::error("Module {0} is disabled, required by {1}", u, module_holder.descriptor.id);
                        module_holder.enabled = false;
                    }
                    else
                        Dfs(*it, sorted);
                }
                else
                    spdlog::info("Module {0} is missing, required by {1}", u, module_holder.descriptor.id);

            }
            else{
                auto it = std::ranges::find_if(sorted, ModulePredicate(u));
                if(it != sorted.end()){
                    if(!it->enabled){
                        spdlog::error("Module {0} is disabled, required by {1}", u, module_holder.descriptor.id);
                        module_holder.enabled = false;
                    }
                }
            }
        }
        sorted.push_back(module_holder);
    }

    void ModuleManager::SortModules() {
        std::vector<ModuleHolder> sorted;
        sorted.clear();

        for (auto i: this->modules) {
            if (!std::ranges::any_of(sorted, ModulePredicate(i.descriptor.id)))
                Dfs(i, sorted);
        }

        this->modules = sorted;

    }

    void ModuleManager::LoadModulesFromAssembly(const std::string &id) {
        LoadAssembly(id);

        auto assembly = GetAssembly(id);
        const std::string &symbolName = "assembly_entry";

        if (!assembly.lib->has_symbol(symbolName)) {
            spdlog::error("❌ Could not find the entry for assembly \"{0}\"({1})", id, assembly.path.c_str());
            return;
        }
        try {
            auto module_init = assembly.lib->get_function<void(ModuleManager &)>(symbolName);

            module_init(*this);
        }
        catch (std::exception &e) {
            spdlog::error("❌ Error while running the entry for assembly \"{0}\" Error: {1}", id, e.what());
            return;
        }
    }

    void ModuleManager::PreRender() {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->PreRender();
            }
        }
    }

    void ModuleManager::AfterFrameEnd() {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->AfterFrameEnd();
            }
        }
    }

    void ModuleManager::Update(int frame) {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->Update(frame);
            }
        }
    }

    void ModuleManager::LateRender(VkCommandBuffer &commands, int frame) {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->LateRender(commands, frame);
            }
        }
    }

    void ModuleManager::OverlayRender() {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->OverlayRender();
            }
        }
    }

    void ModuleManager::Recreate() {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->Recreate();
            }
        }
    }

    void ModuleManager::Render(VkCommandBuffer &commands, int frame) {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->Render(commands, frame);
            }
        }
    }

}