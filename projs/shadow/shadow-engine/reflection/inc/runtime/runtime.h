#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <set>

#include "shadow/core/PathID.h"

#include "assembly.h"

namespace Runtime {

  #define RUNTIME_BOOT() Runtime::Runtime::Boot(argv[0])

  struct CompareAssembly {
    using is_transparent = void;

    bool operator()(const Assembly &a, const Assembly &b) const {
        return a.GetID() < b.GetID();
    }
    bool operator()(const SH::Path &a, const Assembly &b) const {
        return a < b.GetID();
    }
    bool operator()(const Assembly &a, const SH::Path &b) const {
        return a.GetID() < b;
    }
  };

  class Runtime {

    static Runtime *instance;

    std::set<Assembly, CompareAssembly> known_assemblies;

    Runtime();

  public:
    /**
     * @brief Boots the runtime based on the given path for the executing assembly
     * @param path
     */
    void static Boot(std::string path);

    /**
     * @brief Gets the singleton instance of the runtime
     * @return The singleton instance of the runtime
     */
    static Runtime &Get();

    /// @brief Adds an assembly to the runtime
    /// @param assembly The assembly to add
    /// @details It errors out if the assembly is already known
    void AddAssembly(const Assembly &assembly);

    /// @brief Finds an assembly based on the Id
    /// @param id The ID of the assembly to find ("assembly:/core")
    /// @return Reference to the assembly
    /// @details It errors out if the id is not known
    std::optional<const Assembly *> GetAssembly(const SH::Path &id);

    /**
     * @brief Loads an assembly with the given ID
     * @param path The path to the assembly
     * @return The loaded assembly
     */
    void LoadAssembly(const SH::Path &id);

    /**
     * @brief Get the assembly that is running the current application
     * @return The assembly that is running the current application
     *
     * @note This is the only assembly that is a EXE type
     */
    const Assembly &GetExecutingAssembly();

  private:

    Native::native_handle_type LoadLibrary(const char *path, AssemblyType type);

  };

}
