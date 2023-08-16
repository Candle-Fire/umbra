#include <core/Libraries.h>
#include <fs/path.h>
#include "dylib.hpp"

namespace ShadowEngine::Library {
    void* load(ShadowEngine::Path& p) {
        return new dylib("./", p.c_str());
    }

    void unload(void* handle) {
        delete ((dylib*)handle);
    }
}