#include "core/module-manager-v2.h"

void ModuleManager::LoadModule(ModuleHolder &holder) {
    auto assembly = GetAssembly(holder.descriptor.assembly);
    const std::string &symbolName = holder.descriptor.class_name + "_entry";

    if(!assembly.lib->has_symbol(symbolName))
    {
        spdlog::error("❌ Could not find the entry for module \"{0}\"", holder.descriptor.id);
        return;
    }

    auto moduleInit = assembly.lib->get_function<std::shared_ptr<Module>()>(symbolName);

    holder.module = moduleInit();
}

void ModuleManager::Init() {
    this->loadedAssemblies.push_back({
                                             .id = "assembly:/core"
                                     });

    for (const auto& i : this->modules) {
        spdlog::debug("\"{0}\" is registered", i.descriptor.id);
    }

    //Sort
    this->SortModules();

    spdlog::debug("Sorted order:");
    for (const auto& i : this->modules) {
        spdlog::debug("\"{0}\" is registered", i.descriptor.id);
    }

    //Load
    for (auto& i : this->modules) {
        spdlog::debug("Loading {0}", i.descriptor.id);
        if(std::any_of(ITERATE(this->loadedAssemblies),[i](Assembly a){ return a.id == i.descriptor.assembly; })){
            spdlog::debug("✅ Assembly \"{0}\" is already loaded", i.descriptor.assembly);
        }
        else{
            spdlog::debug("🔃 Loading assembly \"{0}\"", i.descriptor.assembly);
            this->LoadAssembly(i.descriptor.assembly);
        }

        this->LoadModule(i);
    }

    for(auto& holder : this->modules){
        if(holder.module)
            spdlog::info("Module {0}({1}) loaded",holder.module holder.descriptor.id);
    }

    //Init
    //Sort
}
