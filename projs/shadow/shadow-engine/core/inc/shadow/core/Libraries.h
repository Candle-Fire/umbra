#pragma once
#include "PathID.h"

namespace ShadowEngine {

  namespace Library {
    // Load the library from the given path.
    // Return the address of the data in RAM.
    void* load(SH::Path& path);
    // Unload the library at the given address.
    void unload(void* handle);
  }
}
