#pragma once

#include <string>
#include <vector>
#include <memory>

#include "dylib.hpp"

#include "shadow/core/PathID.h"
#include "runtime/native_loading.h"

namespace Runtime {

  enum AssemblyType {
    LIB,
    EXE,
  };

  class Assembly {
  public:
    Assembly() = default;

    Assembly(const SH::Path &id, const std::string &path, AssemblyType type = AssemblyType::LIB)
        : id(id), path(path), type(type) {}

    ~Assembly();

    bool IsLoaded() const;

    [[nodiscard]] bool HasSymbol(const std::string &name) const;

    template<typename T>
    T *GetFunction(const char *symbol_name) const {
        return reinterpret_cast<T *>(Native::get_symbol(this->lib, symbol_name));
    }

    template<typename T>
    T *GetFunction(const std::string &symbol_name) const {
        return GetFunction<T>(symbol_name.c_str());
    }

    const SH::Path &GetID() const;

  protected:
    void Load();
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