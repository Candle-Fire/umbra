#pragma once

#include <cstddef>
#include "Canvas.h"
#include "Interface.h"

namespace rx {

    /**
     * The root of the renderer tree.
     * Defines the functions that need to be implemented for the render to happen.
     * Essentially the renderer "interface".
     */
    class Renderer : public Canvas {
    private:
        uint32_t layerMask = 0xFFFFFFFF;
    public:
        virtual ~Renderer() = default;

        // Load resources that will be needed.
        virtual void Load() {}
        // First time setup things.
        virtual void Start() {}
        // Called when switching to another Renderer
        virtual void Stop() {}
        // Called before the per-frame update
        virtual void PreUpdate() {}
        // Called with fixed frequency - for timer tasks. Ask the engine for the timestamp.
        virtual void FixedUpdate() {}
        // Called when the world is updating after the frame render.
        virtual void Update(float dt) {}
        // Called after update. Bake models or whatever in response to game activities.
        virtual void PostUpdate() {}
        // Render to images
        virtual void Render() const {}
        // Render to canvas
        virtual void Compose(rx::ThreadCommands cmd) const {}

        inline uint32_t GetLayerMask() const { return layerMask; }
        inline void SetLayerMask(uint32_t value) { layerMask = value; }

        rx::ColorSpace colorSpace = ColorSpace::SRGB;

        // Render job methods
    };
}