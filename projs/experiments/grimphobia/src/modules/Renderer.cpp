#include <cstdio>
#include <ranges>

#include "modules.h"
#include "ecs/ecs.h"
#include "game/components.h"

#include "SDL3_image/SDL_image.h"

SDL_Texture* texture;



void Renderer::Init()
{
  renderer2d = SDL_CreateGPURenderer(sdl3_module->window, SDL_GPU_SHADERFORMAT_SPIRV, &device);

  game_target = SDL_CreateTexture(renderer2d, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 800,700);

  texture = IMG_LoadTexture(renderer2d, "/home/dpeter99/Documents/Projects/Umbra/umbra/projs/experiments/grimphobia/assets/textures/TileSet_001.png");
}

void Renderer::StartRender()
{
  // Start the render

  // Cal other module's render



  // Draw the world
  // SDL_SetRenderTarget(renderer2d, game_target);

  for (auto entity :
    std::views::filter(ecs->scene.entities, [](Entity* e){
      return e->HasComponent<Position>() && e->HasComponent<RenderSprite>();
    })
    | std::views::transform([](Entity* e){
      return std::tuple(
        e->GetComponent<Position>(),
        e->GetComponent<RenderSprite>()
      );
    })
    )
  {
    const auto [pos, render] = entity;

    SDL_FRect dest = {pos->x, pos->y, 32,32};

    SDL_RenderTexture(renderer2d, texture, nullptr, &dest);
  }


  //SDL_FlushRenderer(renderer2d);

  // GPU_RenderData *data = (GPU_RenderData *)renderer2d->internal;

  ::Render();

  // Finish render
  SDL_RenderPresent(renderer2d);
}

void Renderer::Render()
{
}


void Renderer::Update()
{
}

void Renderer::ProcessEvent(SDL_Event& event)
{
}
