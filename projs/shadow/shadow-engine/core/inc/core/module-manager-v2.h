#pragma once

#include <string>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"
#include "dylib.hpp"

#include "Module.h"

using ID = std::string;

#define ModuleEntry(name, shortname) extern "C" { std::shared_ptr<name> shortname ## _entry(){return std::make_shared<name>();} }

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
    std::shared_ptr<ShadowEngine::Module> module;
    bool enabled;
};

#define ITERATE(It) It.begin(), (It).end()

class ModuleManager{

    std::vector<Assembly> loadedAssemblies;

    std::vector<ModuleHolder> modules;

    void LoadAssembly(const std::string& path);

    Assembly& GetAssembly(const ID& id){
        return *std::find_if(ITERATE(this->loadedAssemblies),[id](const Assembly& a){ return a.id == id; });
    }

    void LoadModule(ModuleHolder& holder);



    static auto ModulePredicate(const std::string& target){
        return [target](const auto &item) { return target == item.descriptor.id;};
    }

    void dfs(const ModuleHolder& moduleHolder, std::vector<ModuleHolder>& sorted);

    void SortModules();

public:

    void AddDescriptors(ModuleDescriptor descriptor){ modules.push_back( {descriptor=descriptor} ); }

    void Init();

};

