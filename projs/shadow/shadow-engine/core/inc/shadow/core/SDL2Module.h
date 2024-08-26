#pragma once

#include <SDL.h>

#include "Module.h"
#include "ShadowWindow.h"
#include "shadow/SHObject.h"

namespace SH {

  class SDL2Module final : public Module {
  SHObject_Base(SDL2Module)

  public:
    ShadowWindow *window;

    SDL2Module() : Module() {}

  private:
    void Init() override;

    void PreInit() override;

    void Destroy() override;

  public:
    void Update(int frame) override;

  };

}