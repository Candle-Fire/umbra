#include <cstdio>
#include <cstdlib>

#include "modules.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"

void SDL3Module::Init()
{
  SDL_SetAppMetadata("Example Renderer Clear", "1.0", "com.example.renderer-clear");

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
  {
    std::printf("Error: SDL_Init(): %s\n", SDL_GetError());
    abort();
  }

  // Create SDL window graphics context
  window_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  window = SDL_CreateWindow("Dear ImGui SDL3+SDL_GPU example", (int)(1280 * window_scale), (int)(720 * window_scale), window_flags);
  if (window == nullptr)
  {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    abort();
  }
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);
}

void SDL3Module::Update()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ::ProcessEvent(event);
  }
}
