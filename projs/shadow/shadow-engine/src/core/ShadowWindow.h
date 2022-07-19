#pragma once

#include <SDL.h>

class ShadowWindow
{
public:

	int Height;
	int Width;

	SDL_Window* sdlWindowPtr;

    SDL_Surface* sdlSurface = NULL;


	//ShadowEngine::Ref<ShadowEngine::Rendering::GraphicsContext> context;

	ShadowWindow(int W, int H);
	
	~ShadowWindow();
};
