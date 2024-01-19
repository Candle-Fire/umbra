#pragma once

#include "shadow/renderer/renderer/Renderer.h"
#include "shadow/renderer/renderer/Interface.h"

namespace rx {
    class Sprite;
    class SpriteAtlas;

    /**
     * A basic 2D renderer.
     * Can draw sprites, textures and fonts to the screen.
     * That's all you really need for a 2D game, right?
     */
    class Renderer2D : public Renderer {
    private:
        rx::Texture targetStenciled;
        rx::Texture targetStencilResolved;
        rx::Texture target;

        glm::vec2 currentBufferSize {};
        float currentLayoutScale = 0;
        float HDRScaling = 9;
    public:
        // Resolution dependant - framebuffers
        virtual void ResizeBuffers();
        // DPI dependant - UI
        virtual void ResizeLayout();

        void Update(float dt) override;
        void FixedUpdate() override;
        void Render() const override;
        void Compose(rx::ThreadCommands cmd) const override;

        const rx::Texture& GetRenderResult() const { return target; }
        virtual const rx::Texture* GetDepthStencil() const { return nullptr; }
        // When focusing a UI element like a menu, the background usually blurs, right?
        virtual const rx::Texture* GetBlurredBackground() const { return nullptr; }

        void AddSprite(rx::Sprite* s, const std::string& layer = "");
        void RemoveSprite(rx::Sprite* s);
        void ClearSprites();
        int GetSpriteOrder(Sprite* s);

        void AddAtlas(rx::SpriteAtlas* atlas, const std::string& layer = "");
        void RemoveAtlas(rx::SpriteAtlas* atlas);
        void ClearAtlases();
        void GetAtlasOrder(rx::SpriteAtlas* atlas);

        struct Renderable2D {
            enum class Type {
                SPRITE,
                ATLAS,
            } type = Type::SPRITE;

            union {
                Sprite* sprite = nullptr;
                SpriteAtlas* atlas;
            };

            int order = 0;
        };

        struct RenderLayer2D {
            std::vector<Renderable2D> renderables;
            std::string layerName;
            int order = 0;
        };

        std::vector<RenderLayer2D> layers { 1 };
        void AddLayer(const std::string& name);
        void SetLayerOrder(const std::string& name, int order);
        void SetSpriteOrder(Sprite* s, int order);
        void SetAtlasOrder(SpriteAtlas* s, int order);
        void SortLayers();
        void ClearLayers();

        float resScale = 1.0f;
        glm::vec2 GetInternalResolution() const {
            return { uint32_t((float) GetPhysicalWidth() * resScale), uint32_t((float) GetPhysicalHeight() * resScale) };
        }

        float GetHDRScaling() const { return HDRScaling; }
        void SetHDRScaling(float value) { HDRScaling = value; }
    };
}