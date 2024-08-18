#pragma once

#include "../ITexture2D.h"
#include "rendering/IRenderer.h"

namespace SH::Renderer::R2D {

    /***
     * Interface for drawing 2D games.
     * Allows setting camera and drawing sprites.
     * @brief Interface for 2D renderer
     */
    class IRenderer2D : public IRenderer {

    public:
        virtual ~IRenderer2D() = default;

        /***
         * Sets the camera position.
         * @param x X position
         * @param y Y position
         */
        virtual void setCameraPosition(float x, float y) = 0;

        /***
         * Sets the camera zoom.
         * @param zoom Zoom
         */
        virtual void setCameraZoom(float zoom) = 0;

        /***
         * Draws a sprite.
         * @param x X position
         * @param y Y position
         * @param width Width
         * @param height Height
         * @param texture Texture
         * @param color Color
         */
        virtual void drawSprite(float x,
                                float y,
                                float width,
                                float height) = 0;
    };

}