#pragma once
#include "shadow/SHObject.h"
#include "shadow/core/Module.h"

namespace SH::Asset {
    class API AssetManagerModule : public SH::Module {
        SHObject_Base(AssetManagerModule)
    public:
        AssetManagerModule() : Module() {}
    };
}
