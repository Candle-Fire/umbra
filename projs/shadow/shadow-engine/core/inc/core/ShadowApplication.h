#pragma once
#include "ShadowWindow.h"
#include "ModuleManager.h"
#include "exports.h"

#include <string>

namespace ShadowEngine {

	/// <summary>
	/// Represents the application
	/// </summary>
    class SH_EXPORT ShadowApplication
	{
		/// <summary>
		/// This is the singleton instance
		/// </summary>
		//static ShadowApplication* instance;

		/// <summary>
		/// The active window that we are drawing to
		/// </summary>
		ShadowWindow* window_;

		/// <summary>
		/// The module manager instance
		/// </summary>
		ModuleManager moduleManager;

		/// <summary>
		/// Represents the running state.
		/// </summary>
		/// If set to false the main loop will stop and continue with the shutdown.
		bool running = true;

		bool no_gui = false;

        std::string game = "";

        void loadGame();

	public:
		/// <summary>
		/// Default constructor
		/// </summary>
		SH_EXPORT ShadowApplication(int argc, char* argv[]);
        SH_EXPORT virtual ~ShadowApplication();

		/// <summary>
		/// Static getter for the singleton instance 
		/// </summary>
		/// Use this for accessing the Application
		/// <returns>The current application reference</returns>
		//static ShadowApplication& Get();

		/// <summary>
		/// Returns the active window used for rendering
		/// </summary>
		/// <returns> The window instance </returns>
		//ShadowWindow& const GetWindow() const { return window_; };
		//void SetWindow(ShadowWindow w) { window_ = w; }

        ShadowEngine::ModuleManager& GetModuleManager() { return moduleManager; };

        SH_EXPORT void Init();
        SH_EXPORT void Start();


    };
}