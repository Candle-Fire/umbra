#include <core/Libraries.h>
#include <fs/path.h>
#include "dylib.hpp"

namespace ShadowEngine {
    void* Library::load(ShadowEngine::Path& p) {
        return new dylib("./", p.c_str());
    }

    void Library::unload(void* handle) {
        delete ((dylib*)handle);
    }
}