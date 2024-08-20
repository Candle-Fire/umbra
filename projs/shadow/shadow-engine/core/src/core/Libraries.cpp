#include <shadow/core/Libraries.h>
#include "dylib.hpp"

namespace ShadowEngine {
  void* Library::load(SH::Path& p) {
      return new dylib("./", p.c_str());
  }

  void Library::unload(void* handle) {
      delete ((dylib*)handle);
  }
}