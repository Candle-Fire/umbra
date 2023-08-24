#pragma once

#include "Module.h"

class CoreModule : public ShadowEngine::Module {
  SHObject_Base(CoreModule)
  public:
    CoreModule() : Module() {}

    void PreInit() override {

    }

    void Init() override {

    }

    void Update(int frame) override {

    }

    void Destroy() override {

    }
};
