#pragma once

#include <string>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"
#include "dylib.hpp"

using ID = std::string;


class Module{

};

struct Assembly{
    ID id;
    std::string path;

    dylib* lib;
};

struct ModuleDescriptor{
     ID id;
     std::string name;
     std::string class_name;
     ID assembly;

     std::vector<ID> dependencies;
};

struct ModuleHolder{
    ModuleDescriptor descriptor;
    std::shared_ptr<Module> module;
};

#define ITERATE(It) It.begin(), (It).end()

class ModuleManager{

    std::vector<Assembly> loadedAssemblies;

    std::vector<ModuleHolder> modules;

    void LoadAssembly(const std::string& path){
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

    Assembly& GetAssembly(const ID& id){
        return *std::find_if(ITERATE(this->loadedAssemblies),[id](const Assembly& a){ return a.id == id; });
    }

    void LoadModule(ModuleHolder& holder);



    static auto ModulePredicate(const std::string& target){
        return [target](const auto &item) { return target == item.descriptor.id;};
    }

    void dfs(const ModuleHolder& moduleHolder, std::vector<ModuleHolder>& sorted) {
        //visited[v] = true;
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

    void SortModules() {
        int module_count = this->modules.size();

        std::vector<ModuleHolder> sorted;
        sorted.clear();

        for (auto i : this->modules) {
            spdlog::debug("Processing {0}", i.descriptor.id);
            if (!std::any_of(ITERATE(sorted),ModulePredicate(i.descriptor.id)))
                dfs(i, sorted);
        }

        //reverse(sorted.begin(), sorted.end());

        this->modules = sorted;

    }

public:

    void AddDescriptors(ModuleDescriptor descriptor){ modules.push_back( {descriptor=descriptor} ); }

    void Init();

};

