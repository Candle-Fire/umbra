//
// Created by dpete on 2022-07-06.
//

#include "core/Module.h"

namespace ShadowEngine {

    SHObject_Base_Impl(Module)

    Module::Module(const std::string &id) : id(id) {}

    RendererModule::RendererModule(const std::string &id) : Module(id) {}
} // ShadowEngine