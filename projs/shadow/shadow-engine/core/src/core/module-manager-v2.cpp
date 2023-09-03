#pragma execution_character_set("utf-8")

#include <ranges>
#include <algorithm>

#include "shadow/core/module-manager-v2.h"
#include "runtime/Runtime.h"

namespace SH {

  void ModuleManager::LoadModule(ModuleHolder &holder) {
      auto assembly = Runtime::Runtime::Get().GetAssembly(holder.descriptor.assembly);
      if (!assembly.has_value() || !assembly.value()->IsLoaded()) {
          spdlog::error("❌ Assembly \"{0}\" for module:\"{1}\" is unknown or not loaded",
                        holder.descriptor.assembly.get(),
                        holder.descriptor.id);
          holder.enabled = false;
          return;
      }
      const Runtime::Assembly &assembly_value = *assembly.value();

      const std::string &symbolName = holder.descriptor.class_name + "_entry";

      if (!assembly_value.HasSymbol(symbolName)) {
          spdlog::error("❌ Could not find the entry for module \"{0}\"", holder.descriptor.id);
          holder.enabled = false;
          return;
      }
      try {
          auto module_init = assembly_value.GetFunction<void(std::shared_ptr<SH::Module> *)>(symbolName);

          module_init(&holder.module);
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

      for (const auto &i : this->modules) {
          spdlog::debug("\"{0}\" is registered", i.descriptor.id);
      }

      //Sort
      this->SortModules();
      this->PrintModuleInfo();

      //Load
      for (auto &i : this->modules) {
          spdlog::trace("Loading {0}", i.descriptor.id);
          auto a = Runtime::Runtime::Get().GetAssembly(i.descriptor.assembly);
          if (!a.has_value()) {
              spdlog::error("❌ Assembly \"{0}\" for module:\"{1}\" is not known",
                            i.descriptor.assembly.get(),
                            i.descriptor.id);
              continue;
          }
          Runtime::Runtime::Get().LoadAssembly(i.descriptor.assembly);

          this->LoadModule(i);
          if (i.enabled)
              spdlog::trace("Module {0}({1}) created", i.module->GetName(), i.descriptor.id);
      }

      this->SortModules();
      this->PrintModuleInfo();

      //PreInit
      spdlog::info("Running PreInit");
      for (auto &holder : this->modules) {
          if (holder.enabled) {
              try {
                  holder.module->PreInit();
              }
              catch (const std::exception &e) {
                  spdlog::error("❌ Error while running PreInit for module \"{0}\" Error: {1}", holder.descriptor.id,
                                e.what());
              }
          }
      }

      this->SortModules();
      this->PrintModuleInfo();

      spdlog::info("Running Init");
      for (auto &holder : this->modules) {
          if (holder.enabled) {
              try {
                  holder.module->Init();
              }
              catch (const std::exception &e) {
                  spdlog::error("❌ Error while running Init for module \"{0}\" Error: {1}", holder.descriptor.id,
                                e.what());
              }
          }
      }

      //Sort
      this->SortModules();
      this->PrintModuleInfo();

      this->finalized = true;
  }

  void ModuleManager::Dfs(ModuleHolder &module_holder, std::vector<ModuleHolder> &sorted) {
      for (auto u : module_holder.descriptor.dependencies) {
          if (!std::ranges::any_of(sorted, ModulePredicate(u)) && u != module_holder.descriptor.id) {
              auto it = std::ranges::find_if(this->modules, ModulePredicate(u));

              if (it != modules.end()) {
                  if (!it->enabled) {
                      spdlog::error("Module {0} is disabled, required by {1}", u, module_holder.descriptor.id);
                      module_holder.enabled = false;
                  } else
                      Dfs(*it, sorted);
              } else
                  spdlog::info("Module {0} is missing, required by {1}", u, module_holder.descriptor.id);

          } else {
              auto it = std::ranges::find_if(sorted, ModulePredicate(u));
              if (it != sorted.end()) {
                  if (!it->enabled) {
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

      for (auto i : this->modules) {
          if (!std::ranges::any_of(sorted, ModulePredicate(i.descriptor.id)))
              Dfs(i, sorted);
      }

      this->modules = sorted;

  }

  void ModuleManager::LoadModulesFromAssembly(const SH::Path &id) {
      Runtime::Runtime::Get().LoadAssembly(id);

      auto assembly_optional = Runtime::Runtime::Get().GetAssembly(id);
      if (!assembly_optional.has_value() || !assembly_optional.value()->IsLoaded()) {
          spdlog::error("❌ Assembly \"{0}\" is unknown or not loaded", id.get());
          return;
      }
      const Runtime::Assembly &assembly = *assembly_optional.value();

      const std::string &symbolName = "assembly_entry";

      if (!assembly.HasSymbol(symbolName)) {
          spdlog::error("❌ Could not find the entry for assembly \"{0}\"({1})", id.get(), assembly.GetID().get());
          return;
      }
      try {
          auto module_init = assembly.GetFunction<void(ModuleManager &)>(symbolName);

          module_init(*this);
      }
      catch (std::exception &e) {
          spdlog::error("❌ Error while running the entry for assembly \"{0}\" Error: {1}", id.get(), e.what());
          return;
      }
  }

  void ModuleManager::Update(int frame) {
      for (auto &holder : this->modules) {
          if (holder.enabled) {
              holder.module->Update(frame);
          }
      }
  }

  void ModuleManager::DeactivateModule(Module *module_ptr, bool force) {
      spdlog::info("Deactivating module {0}", module_ptr->GetType());
      if (!this->finalized || force) {
          auto m = std::find_if(ITERATE(this->modules), [&](const ModuleHolder &item) {
            return item.module.get() == module_ptr;
          });
          if (m != this->modules.end()) {
              m->enabled = false;
          }
      }
  }

  bool ModuleManager::IsModuleActive(const ID &id) {
      auto m = std::find_if(ITERATE(this->modules), ModulePredicate(id));
      return m != this->modules.end() && m->enabled;
  }

  void ModuleManager::IfModuleActive(const ID &id, const std::function<void()> &callback) {
      if (IsModuleActive(id))
          callback();
  }

  void ModuleManager::PrintModuleInfo() {
      spdlog::info("Module info:");
      for (const auto &i : this->modules) {
          spdlog::info("Module {0}({1}) is {2}",
                       i.descriptor.name,
                       i.descriptor.id,
                       i.enabled ? "enabled" : "disabled");
      }
  }

}