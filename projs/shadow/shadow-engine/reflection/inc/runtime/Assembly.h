#pragma once

#include <string>
#include <vector>
#include <memory>

#include "dylib.hpp"

#include "shadow/core/PathID.h"
#include "runtime/native_loading.h"

namespace Runtime {

  /**
    * @brief Enumeration for defining the type of an assembly.
    *
    * @details This enum distinguishes between libraries that can be dynamically
    * loaded and the main executable.
    */
  enum AssemblyType {
    /**
     * @brief Indicates that this assembly is a dynamically loadable library.
     *
     * @details Use this for assemblies that are meant to be loaded and unloaded
     * during runtime.
     */
    LIB,

    /**
     * @brief Indicates that this assembly is the main executable of the
     * application.
     *
     * @details There should typically be only one assembly of this type per
     * application.
     */
    EXE,
  };

  /**
   * @brief Represents a dynamically loaded assembly.
   *
   * @details This class serves as a wrapper around native library loading. It
   * provides functionality to load a library, query its symbols, and retrieve
   * function pointers. It can represent either a dynamically loadable library or
   * the main application executable.
   *
   * @note The destructor takes care of unloading the library.
   */
  class Assembly {
  public:
    /**
     * @brief Default constructor.
     */
    Assembly() = default;

    /**
     * @brief Parameterized constructor.
     *
     * @param id Logical ID of the DLL (e.g., "assembly:/core").
     * @param path System path to the library file.
     * @param type Type of the assembly (default is LIB).
     */
    Assembly(const SH::Path &id, const std::string &path, AssemblyType type = AssemblyType::LIB)
        : id(id), path(path), type(type) {}

    /**
     * @brief Destructor.
     *
     * @details Unloads the assembly if it is loaded.
     */
    ~Assembly();

    /**
     * @brief Checks if the assembly is loaded.
     *
     * @return `true` if the assembly is loaded, `false` otherwise.
     */
    bool IsLoaded() const;

    /**
     * @brief Checks if the assembly has a symbol with the given name.
     *
     * @param name Name of the symbol to look for.
     * @return `true` if the symbol exists, `false` otherwise.
     */
    [[nodiscard]] bool HasSymbol(const std::string &name) const;

    /**
     * @brief Retrieves a function from the assembly.
     *
     * @tparam T The function's type.
     * @param symbol_name The name of the symbol to retrieve.
     * @return A pointer to the function if it exists, `nullptr` otherwise.
     *
     * Example usage:
     * @code
     * typedef void (*FuncType)();
     * FuncType func = assembly.GetFunction<FuncType>("function_name");
     * if (func != nullptr) {
     *     func();
     * }
     * @endcode
     */
    template<typename T>
    T *GetFunction(const char *symbol_name) const {
        return reinterpret_cast<T *>(Native::get_symbol(this->lib, symbol_name));
    }

    /**
     * @brief Overloaded version of GetFunction that takes a std::string.
     *
     * @tparam T The function's type.
     * @param symbol_name The name of the symbol to retrieve.
     * @return A pointer to the function if it exists, `nullptr` otherwise.
     */
    template<typename T>
    T *GetFunction(const std::string &symbol_name) const {
        return GetFunction<T>(symbol_name.c_str());
    }

    /**
     *  Get a variable from the dynamic library currently loaded in the object
     *
     *  @throws dylib::symbol_error if the symbol could not be found
     *
     *  @param T the template argument must be the type of the variable to get
     *  @param symbol_name the symbol name of a variable to get from the dynamic library
     *
     *  @return a reference to the requested variable
     */
    template<typename T>
    T &get_variable(const char *symbol_name) const {
        return *reinterpret_cast<T *>(Native::get_symbol(this->lib, symbol_name));
    }

    template<typename T>
    T &get_variable(const std::string &symbol_name) const {
        return get_variable<T>(symbol_name.c_str());
    }

    /**
     * @brief Gets the logical ID of the assembly.
     *
     * @return The logical ID.
     */
    const SH::Path &GetID() const;

  protected:
    /**
     * @brief Loads the assembly into memory.
     *
     * @details This function is responsible for actually loading the assembly.
     */
    void Load();

    void RunEntry();

    static constexpr const char *entryNameSymbol = "assembly_entry_name";
    static constexpr const char *entryDefaultSymbol = "assembly_entry";
  private:
    /// @brief The logical ID of the DLL file (eg: "assembly:/core")
    SH::Path id;
    /// @brief The system path to the DLL file being referenced
    std::string path;

    AssemblyType type = AssemblyType::LIB;

    /// @brief The dylib pointer to the loaded
    Native::native_handle_type lib = nullptr;

    friend class Runtime;
  };

} // namespace Runtime