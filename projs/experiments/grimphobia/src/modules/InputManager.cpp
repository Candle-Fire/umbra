#include "modules.h"
#include "SDL3/SDL_oldnames.h"


void InputManager::Init()
{
}

void InputManager::Update()
{
}

void InputManager::Render()
{
}

void InputManager::ProcessEvent(SDL_Event& event)
{
  switch (event.key.key)
  {
  case SDLK_LEFT:
      key_state.left = event.type == SDL_EVENT_KEY_DOWN;
      break;
    case SDLK_RIGHT:
      key_state.right = event.type == SDL_EVENT_KEY_DOWN;
      break;
    case SDLK_UP:
      key_state.forward = event.type == SDL_EVENT_KEY_DOWN;
      break;
    case SDLK_DOWN:
      key_state.back = event.type == SDL_EVENT_KEY_DOWN;
      break;
  }

  movement.x = key_state.left ? 1 : key_state.right ? -1 : 0;
  movement.y = key_state.forward ? 1 : key_state.back ? -1 : 0;
}
