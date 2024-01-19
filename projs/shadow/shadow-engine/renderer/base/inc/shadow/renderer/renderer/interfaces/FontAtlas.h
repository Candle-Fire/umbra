#pragma once

#include <string>
#include "shadow/renderer/renderer/interfaces/FontAtlas.h"

namespace rx {
  class SpriteAtlas {
  private:
    enum Flags {
      EMPTY = 0,
      HIDDEN = 1 << 0,
      STATIC = 1 << 1
    };
    uint32_t flags = Flags::EMPTY;

  public:
    std::wstring text;
    rx::Image::RenderMode params;

    SpriteAtlas() = default;
    SpriteAtlas(const std::string &text /*, const rx::*/){};
    //TODO: this file was never finished
    // https://github.com/Candle-Fire/umbra/blob/f49e67bdb6a3347e4a26395e374ed9a495c35d1e/projs/shadow/shadow-engine/shadow-renderer/inc/renderer/interfaces/FontAtlas.h
  };
}