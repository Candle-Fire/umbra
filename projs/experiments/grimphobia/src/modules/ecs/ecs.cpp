#include "ecs.h"

SHObject_Base_Impl(Component)

void Entity::AddComponent(Component* comp)
{
  components.push_back(comp);
}

void Ecs::Init()
{

}

void Ecs::Update()
{
}

void Ecs::Render()
{
}

void Ecs::ProcessEvent(SDL_Event& event)
{
}
