#include "module_manager.h"

#include "modules.h"

void Init()
{
  sdl3_module->Init();
  renderer->Init();
  imgui_module->Init();
  input_manager->Init();
}

void Update()
{
  sdl3_module->Update();
  renderer->Update();
  imgui_module->Update();
  input_manager->Update();
}

void ProcessEvent(SDL_Event& event)
{
  imgui_module->ProcessEvent(event);
  input_manager->ProcessEvent(event);
}

void StartRender()
{
  renderer->StartRender();
}

void Render()
{
  imgui_module->Render();
}
