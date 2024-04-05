#pragma once

#include <string>
#include <stdexcept>

#if (defined(_WIN32) || defined(_WIN64))
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#if (defined(_WIN32) || defined(_WIN64))
#define DYLIB_WIN_MAC_OTHER(win_def, mac_def, other_def) win_def
#define DYLIB_WIN_OTHER(win_def, other_def) win_def
#elif defined(__APPLE__)
#define DYLIB_WIN_MAC_OTHER(win_def, mac_def, other_def) mac_def
#define DYLIB_WIN_OTHER(win_def, other_def) other_def
#else
#define DYLIB_WIN_MAC_OTHER(win_def, mac_def, other_def) other_def
#define DYLIB_WIN_OTHER(win_def, other_def) other_def
#endif

namespace Native {

  struct filename_components {
    static constexpr const char *prefix = DYLIB_WIN_OTHER("", "lib");
    static constexpr const char *suffix = DYLIB_WIN_MAC_OTHER(".dll", ".dylib", ".so");
  };

  /**
  *  This exception is raised when the library failed to load a dynamic library or a symbol
  *
  *  @param message the error message
  */
  class Exception : public std::runtime_error {
  public:
    explicit Exception(const std::string &message) : std::runtime_error(message) {}
  };

  /**
   *  This exception is raised when the library failed to load or encountered symbol resolution issues
   *
   *  @param message the error message
   */
  class load_error : public Exception {
  public:
    explicit load_error(const std::string &message) : Exception(message) {}
  };

  /**
   *  This exception is raised when the library failed to load a symbol
   *
   *  @param message the error message
   */
  class symbol_error : public Exception {
  public:
    explicit symbol_error(const std::string &message) : Exception(message) {}
  };

  using native_handle_type = DYLIB_WIN_OTHER(HINSTANCE, void *);
  using native_symbol_type = DYLIB_WIN_OTHER(FARPROC, void *);

  native_handle_type open(const char *path) noexcept;

  native_symbol_type locate_symbol(native_handle_type lib, const char *name) noexcept;

  void close(native_handle_type lib) noexcept;

  std::string get_error_description() noexcept;

  /**
    *  Get a symbol from the dynamic library currently loaded in the object
    *
    *  @throws dylib::symbol_error if the symbol could not be found
    *
    *  @param symbol_name the symbol name to get from the dynamic library
    *
    *  @return a pointer to the requested symbol
    */
  native_symbol_type get_symbol(native_handle_type handle, const char *symbol_name);

  native_symbol_type get_symbol(native_handle_type handle, const std::string &symbol_name);

}