#pragma once

#include "imstb_rectpack.h"
#include "shadow/assets/resource/Resource.h"
#include <renderer/ImageRenderer.h>

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

        rx::Image::RenderMode params;
        rx::Texture& textureResource;
        rx::Texture&  maskResource;

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
            if (textureResource.isValid())
                return &textureResource;
            return nullptr;
        }

        struct Packer {
          using Rect = stbrp_rect;
          stbrp_context context = {};
          std::vector<stbrp_node> nodes;
          std::vector<Rect> rects;
          int width = 0;
          int height = 0;

          void Clear() { rects.clear(); }

          void AddRect(const Rect &r) {
              rects.push_back(r);
              width = std::max(width, r.w);
              height = std::max(height, r.h);
          }

          bool Pack(int max) {
              while (width <= max || height <= max) {
                  if (nodes.size() < width)
                      nodes.resize(width);

                  stbrp_init_target(&context, width, height, nodes.data(), int(nodes.size()));
                  if (stbrp_pack_rects(&context, rects.data(), int(rects.size())))
                      return true;

                  if (height < width)
                      height *= 2;
                  else
                      width *= 2;

              }
              width = 0;
              height = 0;
              return false;
          }
        };
    };

}