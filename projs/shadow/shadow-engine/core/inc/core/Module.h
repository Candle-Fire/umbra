#pragma once

#include "SHObject.h"
#include "SDL_events.h"
#include <memory>
#include <glm/vec2.hpp>
#include "vlkx/vulkan/abstraction/Commands.h"
#include "event-bus/events.h"

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
     * A class especially for modules that submit data to the screen.
     * There can be only one of these at a time, usually either the editor or the game.
     */
    class SubmitterModule : public Module {
      public:
        virtual void Draw() = 0;

        virtual glm::vec2 GetRenderExtent() = 0;
    };

}

