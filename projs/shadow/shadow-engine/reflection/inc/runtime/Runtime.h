#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <set>

#include "shadow/core/PathID.h"

#include "Assembly.h"

namespace Runtime {

  #define RUNTIME_BOOT() Runtime::Runtime::Boot(argv[0])

  /**
    * @brief Comparator for sorting and searching assemblies.
    *
    * @details This comparator is used for storing assemblies in a sorted
    * container like std::set. It uses the logical ID for comparisons.
    */
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

  /**
   * @brief Manages the application's runtime environment.
   *
   * @details This class acts as a central repository for assemblies. It
   * handles loading, unloading, and accessing assemblies during the
   * application's lifetime.
   */
  class Runtime {

    static Runtime *instance; ///< Singleton instance of Runtime.

    /// Set of known assemblies, sorted by their logical IDs.
    std::set<Assembly, CompareAssembly> known_assemblies;

    /// Private constructor to enforce the singleton pattern.
    Runtime();

  public:
    /**
     * @brief Initializes the runtime environment.
     *
     * @param path System path to the main executable.
     */
    void static Boot(std::string path);

    /**
     * @brief Retrieves the singleton instance of Runtime.
     *
     * @return The singleton instance.
     */
    static Runtime &Get();

    /**
     * @brief Adds an assembly to the known list.
     *
     * @param assembly The assembly to add.
     * @throws std::runtime_error If the assembly is already known.
     */
    void AddAssembly(const Assembly &assembly);

    /**
     * @brief Finds an assembly by its logical ID.
     *
     * @param id Logical ID of the assembly to find (e.g., "assembly:/core").
     * @return An optional containing a pointer to the assembly if it exists,
     * `std::nullopt` otherwise.
     */
    std::optional<const Assembly *> GetAssembly(const SH::Path &id);

    /**
     * @brief Loads an assembly by its logical ID.
     *
     * @param id Logical ID of the assembly to load.
     */
    void LoadAssembly(const SH::Path &id);

    /**
     * @brief Retrieves the assembly that is running the current application.
     *
     * @return The main executable assembly.
     *
     * @note This should be the only assembly of type EXE.
     */
    const Assembly &GetExecutingAssembly();

  private:

    /**
     * @brief Internal method to load a library.
     *
     * @param path System path to the library.
     * @param type Type of the assembly.
     * @return Native handle to the loaded library.
     */
    Native::native_handle_type LoadLibrary(const char *path, AssemblyType type);

  };

}
