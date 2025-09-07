//
// Created by dpeter99 on 06/09/2025.
//

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game/player.h"
#include "modules/modules.h"
#include "modules/ecs/ecs.h"


std::shared_ptr<SDL3Module> sdl3_module;
std::shared_ptr<ImGUIModule> imgui_module;
std::shared_ptr<Renderer> renderer;
std::shared_ptr<Ecs> ecs;



int main()
{
  sdl3_module = std::make_shared<SDL3Module>();
  imgui_module = std::make_shared<ImGUIModule>();
  renderer = std::make_shared<Renderer>();
  ecs = std::make_shared<Ecs>();

  Init();

  Player* player = ecs->scene.AddEntity(new Player());

  bool done = false;
  while (!done)
  {
    Update();

    for (auto comps : FilterEntities<Position, PlayerMovement>())
    {

    }

    StartRender();
  }

  // Close and destroy the window
  SDL_DestroyWindow(sdl3_module->window);

  // Clean up
  SDL_Quit();
  return 0;
}
