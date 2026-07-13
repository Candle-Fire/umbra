#pragma once
#include "module-manager-v2.h"
#include "shadow/exports.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "shadow/event-bus/event_bus.h"
#include "shadow/assets/fs/file.h"

#include <string>

#include "shadow/assets/resource/ResourceManager.h"

namespace SH {

    /// <summary>
    /// The globally-available engine core struct.
    /// Provides an access point to core useful objects, such as the DFS corresponding to the application's physical file location, the event bus, and the module manager.
    /// </summary>
    class API ShadowApplication : SHObject {
      SHObject_Base(ShadowApplication)
    public:
      static std::unique_ptr<FileSystem> diskFS;
      /// <summary>
        /// This is the singleton instance
        /// </summary>
        static ShadowApplication *instance;
    private:

      /// <summary>
        /// The module manager instance
        /// </summary>
        ModuleManager moduleManager;

        SH::Events::EventBus<0> eventBus;

        SH::Asset::ResourceManager resourceManager;
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

        SH::Asset::ResourceManager& GetResourceManager() { return resourceManager; }

        void Init();

        void Start();
    };
}
