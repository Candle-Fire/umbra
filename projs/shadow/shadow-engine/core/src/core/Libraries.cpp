#include <shadow/core/Libraries.h>
#include <shadow/assets/fs/path.h>
#include "dylib.hpp"

namespace ShadowEngine {
  void* Library::load(ShadowEngine::Path& p) {
      return new dylib("./", p.c_str());
  }

  void Library::unload(void* handle) {
      delete ((dylib*)handle);
  }
}