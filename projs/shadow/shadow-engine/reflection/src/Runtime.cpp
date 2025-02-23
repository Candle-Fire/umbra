#include "runtime/Runtime.h"
#include "spdlog/spdlog.h"
#include "runtime/native_loading.h"

namespace Runtime {

  Runtime *Runtime::instance = nullptr;

  void Runtime::Boot(std::string path) {
      //Start singleton instance
      new Runtime();

      //Start an assembly from the given path
      SH::Path const
          id = SH::Path("assembly:/" + std::filesystem::path(path).filename().replace_extension("").string());

      Assembly assembly(id, path, AssemblyType::EXE);

      assembly.lib = Native::open(nullptr);

      instance->known_assemblies.insert(assembly);
  }

  Runtime &Runtime::Get() {
      return *instance;
  }

  const Assembly &Runtime::GetExecutingAssembly() {
      return *std::ranges::find_if(known_assemblies, [](const auto &a) { return a.type == AssemblyType::EXE; });
  }

  void Runtime::LoadAssembly(const SH::Path &id) {
      auto assembly = this->known_assemblies.find(id);
      if (assembly == this->known_assemblies.end()) {
          spdlog::error("❌ Assembly \"{0}\" is not known", id.get());
          throw std::runtime_error("Assembly not known");
      }
      if (assembly->IsLoaded()) {
          spdlog::trace("🧩 Assembly \"{0}\" is already loaded", id.get());
      } else {
          spdlog::debug("🧩 Loading assembly \"{0}\"", id.get());
          auto a = const_cast<Assembly *>(assembly.operator->());
          a->Load();
      }

  }

  Runtime::Runtime() {
      //Check if the singleton instance has already been created
      if (instance != nullptr) {
          throw std::runtime_error("Runtime instance already exists");
      }

      instance = this;
  }

  std::optional<const Assembly *> Runtime::GetAssembly(const SH::Path &id) {
      auto it = known_assemblies.find(id);
      return it == this->known_assemblies.end() ? std::nullopt : std::optional<const Assembly *>(it.operator->());
  }

  void Runtime::AddAssembly(const Assembly &assembly) {
      if (this->known_assemblies.contains(assembly.GetID())) {
          spdlog::error("❌ Assembly \"{0}\" is already known", assembly.GetID().get());
          throw std::runtime_error("Assembly already known");
      }
      this->known_assemblies.insert(assembly);

  }

} // namespace Runtime