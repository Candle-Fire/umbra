#pragma once

#include <string>
#include "FontAtlas.h"
#include "Font.h"

namespace rx {

  // Represents a Font Atlas as a Sprite.
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
    rx::Font::RenderMode params;
    ShadowEngine::Resource fontResource;
    std::string fontName;

    SpriteAtlas() = default;
    SpriteAtlas(const std::string& text, const rx::Font::RenderMode& params = rx::Font::RenderMode(), const std::string& fontName = "")
        : params(params), fontName(fontName) {
        SetText(text);
        if (!fontName.empty())
            fontResource = rx::Resource::Load(fontName);
    }
  }

}