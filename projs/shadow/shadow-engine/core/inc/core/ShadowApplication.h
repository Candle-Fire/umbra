#pragma once
#include "ShadowWindow.h"
#include "module-manager-v2.h"
#include "exports.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"
#include "delegates/event_bus.h"

#include <string>

namespace ShadowEngine {

	/// <summary>
	/// Represents the application
	/// </summary>
    class ShadowApplication : ShadowEngine::SHObject
	{
    SHObject_Base(ShadowApplication)
		/// <summary>
		/// This is the singleton instance
		/// </summary>
		static ShadowApplication* instance;

        /// <summary>
        /// The active window that we are drawing to
        /// </summary>
        ShadowWindow *window_;

        /// <summary>
        /// The module manager instance
        /// </summary>
        ModuleManager moduleManager;



		/// <summary>
		/// Represents the running state.
		/// </summary>
		/// If set to false the main loop will stop and continue with the shutdown.
		bool running = true;

        bool debug = false;

        std::string game = "";

        void loadGame();

	public:

        EventBus eventBus;
		/// <summary>
		/// Default constructor
		/// </summary>
		ShadowApplication(int argc, char* argv[]);
        virtual ~ShadowApplication();

        /// <summary>
        /// Static getter for the singleton instance
        /// </summary>
        /// Use this for accessing the Application
        /// <returns>The current application reference</returns>
        static ShadowApplication &Get();

        /// <summary>
        /// Returns the active window used for rendering
        /// </summary>
        /// <returns> The window instance </returns>
        //ShadowWindow& const GetWindow() const { return window_; };
        //void SetWindow(ShadowWindow w) { window_ = w; }

        ModuleManager &GetModuleManager() { return moduleManager; };

        void Init();

        void Start();

        void PollEvents();
    };
}