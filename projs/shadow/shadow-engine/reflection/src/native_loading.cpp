#include "runtime/native_loading.h"

#if (defined(_WIN32) || defined(_WIN64))
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#else
#include <dlfcn.h>
#include <stdexcept>

#endif

namespace Native {

  native_handle_type open(const char *path) noexcept {
#if (defined(_WIN32) || defined(_WIN64))
      return LoadLibraryA(path);
#else
      return dlopen(path, RTLD_NOW | RTLD_LOCAL);
#endif
  }

  native_symbol_type locate_symbol(native_handle_type lib, const char *name) noexcept {
      return DYLIB_WIN_OTHER(GetProcAddress, dlsym)(lib, name);
  }

  void close(native_handle_type lib) noexcept {
      DYLIB_WIN_OTHER(FreeLibrary, dlclose)(lib);
  }

  std::string get_error_description() noexcept {
#if (defined(_WIN32) || defined(_WIN64))
      constexpr const size_t buf_size = 512;
          auto error_code = GetLastError();
          if (!error_code)
              return "Unknown error (GetLastError failed)";
          char description[512];
          auto lang = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
          const DWORD length =
              FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code, lang, description, buf_size, nullptr);
          return (length == 0) ? "Unknown error (FormatMessage failed)" : description;
#else
      auto description = dlerror();
      return (description == nullptr) ? "Unknown error (dlerror failed)" : description;
#endif
  }

  native_symbol_type get_symbol(native_handle_type handle, const char *symbol_name) {
      if (!symbol_name)
          throw std::invalid_argument("Null parameter");
      if (!handle)
          throw std::logic_error("The dynamic library handle is null");

      auto symbol = locate_symbol(handle, symbol_name);

      if (symbol == nullptr)
          throw symbol_error("Could not get symbol \"" + std::string(symbol_name) + "\"\n" + get_error_description());
      return symbol;
  }

  native_symbol_type get_symbol(native_handle_type handle, const std::string &symbol_name) {
      return get_symbol(handle, symbol_name.c_str());
  }

}