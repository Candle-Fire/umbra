#pragma once

#include "SHObject.h"
#include "SDL_events.h"
#include <memory>
#include "vlkx/vulkan/abstraction/Commands.h"

namespace ShadowEngine {

/// @brief ShadowModules are the base of the engine. They add core abilities.
    class API Module : public SHObject {
      SHObject_Base(Module)

      protected:
        std::string id;

      public:

        /// @brief Gives back the ID under witch the module was registered in
        /// @return The full ID for the module including the type eg.: "module:/renderer/vulkan"
        [[nodiscard]] std::string GetId() const {
            return this->id;
        }

        /// @brief Pre Init is called when the module is added to the engine
        virtual void PreInit() {};

        /// @brief Init is called after all the modules are added
        virtual void Init() {};

        /// @brief update is called each frame
        virtual void Update(int frame) {};

        virtual void Destroy() {};

        /// <summary>
        /// Returns the name of the module
        /// </summary>
        /// <returns></returns>
        std::string GetName() {
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
        virtual void BeginRenderPass(const std::unique_ptr<vlkx::RenderCommand> &commands) = 0;

        virtual void EnableEditor() = 0;

        virtual VkExtent2D GetRenderExtent() = 0;
    };

}

