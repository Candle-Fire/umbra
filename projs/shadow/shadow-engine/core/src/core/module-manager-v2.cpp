#include "core/module-manager-v2.h"

void ModuleManager::LoadModule(ModuleHolder &holder) {
    auto assembly = GetAssembly(holder.descriptor.assembly);
    const std::string &symbolName = holder.descriptor.class_name + "_entry";

    if(!assembly.lib->has_symbol(symbolName))
    {
        spdlog::error("❌ Could not find the entry for module \"{0}\"", holder.descriptor.id);
        return;
    }
    try{
        auto moduleInit = assembly.lib->get_function<std::shared_ptr<ShadowEngine::Module>()>(symbolName);

        holder.module = moduleInit();    
    }
    catch (std::exception& e) {
        spdlog::error("❌ Error while running the entry for module \"{0}\" Error: {1}", holder.descriptor.id, e.what());
        return;
    }

    holder.enabled = true;
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
            spdlog::debug("➕ Loading assembly \"{0}\"", i.descriptor.assembly);
            this->LoadAssembly(i.descriptor.assembly);
        }

        this->LoadModule(i);
    }



    //PreInit
    for(auto& holder : this->modules){
        if(holder.enabled){
            spdlog::info("Module {0}({1}) loaded", holder.module->GetName(), holder.descriptor.id);
            holder.module->PreInit();
        }
    }


    //Sort
}

void ModuleManager::LoadAssembly(const std::string &path) {
    spdlog::info("Assembly {0} is loaded", path);

    dylib* dllptr;

    try {
        dllptr = new dylib("./", path);
    }
    catch (std::exception& e) {
        spdlog::error(e.what());
        exit(1);
    }

    this->loadedAssemblies.push_back({
                                             .id = path,
                                             .lib = dllptr,
                                     });
}

void ModuleManager::dfs(const ModuleHolder &moduleHolder, std::vector<ModuleHolder> &sorted) {
    for (auto u : moduleHolder.descriptor.dependencies) {
        if (!std::any_of( ITERATE(sorted),ModulePredicate(u)) && u != moduleHolder.descriptor.id)
        {
            auto it = std::find_if(ITERATE(this->modules), ModulePredicate(u));

            if(it != modules.end())
                dfs(*it, sorted);
            else
                spdlog::info("Module {0} is missing, required by {1}", u ,moduleHolder.descriptor.id);

        }
    }
    sorted.push_back(moduleHolder);
}

void ModuleManager::SortModules() {
    std::vector<ModuleHolder> sorted;
    sorted.clear();

    for (auto i : this->modules) {
        if (!std::any_of(ITERATE(sorted),ModulePredicate(i.descriptor.id)))
            dfs(i, sorted);
    }

    this->modules = sorted;

}
