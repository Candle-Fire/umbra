#include "ShadowWindow.h"


ShadowWindow::ShadowWindow(int W, int H) : Height(H), Width(W)
{
    // Create our window
    sdlWindowPtr = SDL_CreateWindow( "Candlefire", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Width, Height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN );

    // Make sure creating the window succeeded
    if ( !sdlWindowPtr ) {
        //Raise an error in the log
        //std::cout << "Error creating window: " << SDL_GetError()  << std::endl;
    }

    // Get the surface from the window
    sdlSurface = SDL_GetWindowSurface( sdlWindowPtr );

    // Make sure getting the surface succeeded
    if ( !sdlSurface ) {
        //Raise an error in the log
        //std::cout << "Error getting surface: " << SDL_GetError() << std::endl;
    }

	// Create window
    /*
	this->winPtr = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	SH_CORE_ASSERT(winPtr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());

	context = ShadowEngine::Rendering::GraphicsContext::Create(this);
	context->Init();
     */
}

ShadowWindow::~ShadowWindow()
{
}
