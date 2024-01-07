#pragma once

#include <string>
#include "renderer/interfaces/Font.h"

namespace rx {
    class SpriteAtlas {
    private:
        enum Flags {
            EMPTY = 0,
            HIDDEN = 1<<0,
            STATIC = 1<<1
        };
        uint32_t flags = Flags::EMPTY;

    public:
        std::wstring text;
        rx::Image::RenderMode params;

        SpriteAtlas() = default;
        SpriteAtlas(const std::string& text, const rx::)
}