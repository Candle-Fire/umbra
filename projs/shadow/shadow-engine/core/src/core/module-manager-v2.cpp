#include "core/module-manager-v2.h"

namespace ShadowEngine {

    void ModuleManager::LoadModule(ModuleHolder &holder) {
        auto assembly = GetAssembly(holder.descriptor.assembly);
        const std::string &symbolName = holder.descriptor.class_name + "_entry";

        if (!assembly.lib->has_symbol(symbolName)) {
            spdlog::error("❌ Could not find the entry for module \"{0}\"", holder.descriptor.id);
            return;
        }
        try {
            auto moduleInit = assembly.lib->get_function<std::shared_ptr<ShadowEngine::Module>()>(symbolName);

            holder.module = moduleInit();
        }
        catch (std::exception &e) {
            spdlog::error("❌ Error while running the entry for module \"{0}\" Error: {1}", holder.descriptor.id,
                          e.what());
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
            if (std::any_of(ITERATE(this->loadedAssemblies), [i](Assembly a) {
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



        //PreInit
        spdlog::info("Running PreInit");
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->PreInit();
            }
        }

        spdlog::info("Running PreInit");
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->Init();
            }
        }

        //Sort
    }

    void ModuleManager::LoadAssembly(const std::string &id) {
        auto assembly = std::find_if(ITERATE(this->loadedAssemblies), [&](const auto &item) {
            return item.id == id;
        });
        if (assembly == this->loadedAssemblies.end()) {
            spdlog::error("No such assembly: {0}", id);
            return;
        }


        dylib *dllptr;

        try {
            dllptr = new dylib("./", assembly->path);
        }
        catch (std::exception &e) {
            spdlog::error(e.what());
            exit(1);
        }

        assembly->lib = dllptr;
    }

    void ModuleManager::dfs(const ModuleHolder &moduleHolder, std::vector<ModuleHolder> &sorted) {
        for (auto u: moduleHolder.descriptor.dependencies) {
            if (!std::any_of(ITERATE(sorted), ModulePredicate(u)) && u != moduleHolder.descriptor.id) {
                auto it = std::find_if(ITERATE(this->modules), ModulePredicate(u));

                if (it != modules.end())
                    dfs(*it, sorted);
                else
                    spdlog::info("Module {0} is missing, required by {1}", u, moduleHolder.descriptor.id);

            }
        }
        sorted.push_back(moduleHolder);
    }

    void ModuleManager::SortModules() {
        std::vector<ModuleHolder> sorted;
        sorted.clear();

        for (auto i: this->modules) {
            if (!std::any_of(ITERATE(sorted), ModulePredicate(i.descriptor.id)))
                dfs(i, sorted);
        }

        this->modules = sorted;

    }

    void ModuleManager::LoadModulesFromAssembly(const std::string &id) {
        LoadAssembly(id);

        auto assembly = GetAssembly(id);
        const std::string &symbolName = "assembly_entry";

        if (!assembly.lib->has_symbol(symbolName)) {
            spdlog::error("❌ Could not find the entry for assembly \"{0}\"", id);
            return;
        }
        try {
            auto moduleInit = assembly.lib->get_function<void(ModuleManager &)>(symbolName);

            moduleInit(*this);
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

    void ModuleManager::Event(SDL_Event *evt) {
        for (auto &holder: this->modules) {
            if (holder.enabled) {
                holder.module->Event(evt);
            }
        }
    }

}