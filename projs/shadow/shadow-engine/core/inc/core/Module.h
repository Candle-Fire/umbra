#ifndef UMBRA_MODULE_H
#define UMBRA_MODULE_H

#include "SHObject.h"
#include "SDL_events.h"
#include <memory>
#include "vlkx/vulkan/abstraction/Commands.h"

namespace ShadowEngine {

    /// <summary>
    /// ShadowModules are the base of the engine. They add core abilities.
    /// </summary>
    class Module : public SHObject
    {
        SHObject_Base(Module)

    public:

        /// <summary>
        /// Pre Init is called when the module is added to the engine
        /// </summary>
        virtual void PreInit() = 0;


        /// <summary>
        /// Init is called after all the modules are added
        /// </summary>
        virtual void Init() = 0;

        /// <summary>
        /// update is called each frame
        /// </summary>
        virtual void Update(int frame) = 0;

        virtual void Recreate() = 0;

        virtual void PreRender() = 0;

        virtual void Render(VkCommandBuffer& commands, int frame) = 0;

        virtual void LateRender(VkCommandBuffer& commands, int frame) = 0;

        virtual void OverlayRender() = 0;

        virtual void AfterFrameEnd() = 0;

        virtual void Destroy() = 0;

        virtual void Event(SDL_Event* e) = 0;

        /// <summary>
        /// Returns the name of the module
        /// </summary>
        /// <returns></returns>
        virtual std::string GetName() {
            return this->GetType();
        };
    };

    /**
     * A class especially for modules that are renderers.
     * Allows the engine to access state from the renderer independent of implementation.
     */
    class RendererModule : public Module {
    public:
        // Begin the render pass using the given commands.
        // Will call out through the regular modules to gather geometry to render.
        virtual void BeginRenderPass(const std::unique_ptr<vlkx::RenderCommand>& commands) = 0;

        virtual void EnableEditor() = 0;

        virtual VkExtent2D GetRenderExtent() = 0;
    };

} // ShadowEngine

#endif //UMBRA_MODULE_H
