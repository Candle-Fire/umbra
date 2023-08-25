#pragma once
#include "ShadowWindow.h"
#include "module-manager-v2.h"
#include "exports.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"
#include "event-bus/event_bus.h"

#include <string>

namespace SH {

    /// <summary>
    /// Represents the application
    /// </summary>
    class API ShadowApplication : SH::SHObject {
      SHObject_Base(ShadowApplication)
        /// <summary>
        /// This is the singleton instance
        /// </summary>
        static ShadowApplication *instance;

        /// <summary>
        /// The module manager instance
        /// </summary>
        ModuleManager moduleManager;

        SH::Events::EventBus<0> eventBus;

        /// <summary>
        /// Represents the running state.
        /// </summary>
        /// If set to false the main loop will stop and continue with the shutdown.
        bool running = true;

        bool debug = false;

        std::string game = "";

      public:

        //EventBus::EventBus eventBus;

        /// <summary>
        /// Default constructor
        /// </summary>
        ShadowApplication(int argc, char *argv[]);

        virtual ~ShadowApplication();

        /// <summary>
        /// Static getter for the singleton instance
        /// </summary>
        /// Use this for accessing the Application
        /// <returns>The current application reference</returns>
        static ShadowApplication &Get();

        ModuleManager &GetModuleManager() { return moduleManager; };

        SH::Events::EventBus<0> GetEventBus() { return eventBus; };

        void Init();

        void Start();
    };
}