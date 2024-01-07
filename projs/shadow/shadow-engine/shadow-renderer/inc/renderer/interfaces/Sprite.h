#pragma once

#include <renderer/assets/RenderResource.h>

namespace rx {
    class Sprite {
    public:
        enum RenderMode {
            NONE = 0,
            HIDDEN = 1<<0,
            STATIC = 1<<1,
            BILLBOARD = 1<<2,
            ORTHO = 1<<3
        };

        uint32_t flags = RenderMode::NONE;

        std::string textureName;
        std::string maskName;

        Image::RenderMode params;
        Resource textureResource;
        Resource maskResource;

        Sprite(const std::string& newTexture = "", const std::string& newMask = "");
        virtual ~Sprite() = default;

        virtual void FixedUpdate();
        virtual void Update(float dt);
        virtual void Draw(ThreadCommands cmd) const;

#define GET_SET_RENDER_MODE(x, y)                   \
        constexpr void Set##x(bool val = true) {    \
            if (val)                                \
                flags |= y;                         \
            else                                    \
                flags &= y;                         \
        }                                           \
                                                    \
        constexpr bool Is##x() { return flags & y; }

        GET_SET_RENDER_MODE(Hidden, RenderMode::HIDDEN);
        GET_SET_RENDER_MODE(Static, RenderMode::STATIC);
        GET_SET_RENDER_MODE(Billboard, RenderMode::BILLBOARD);
        GET_SET_RENDER_MODE(Orthographic, RenderMode::ORTHO);

        const Texture* GetTexture() const {
            if (textureResource.IsValid())
                return &textureResource.GetTexture();
            return nullptr;
        }
    };
}