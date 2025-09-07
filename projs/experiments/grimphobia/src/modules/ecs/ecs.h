#pragma once
#include <memory>
#include <vector>
#include <ranges>

#include "SHObject.h"
#include "../../../../../extern/spdlog/include/spdlog/fmt/bundled/chrono.h"
#include "modules/modules.h"

class Component : public SH::SHObject
{
SHObject_Base(Component)
};

class Entity
{
  std::vector<Component*> components;
public:
  Entity() = default;
  virtual ~Entity() = default;

  virtual void Init() = 0;

  void AddComponent(Component* comp);

  template<class T>
  requires std::is_base_of_v<Component, T>
  bool HasComponent();

  template<class T>
  requires std::is_base_of_v<Component, T>
  T* GetComponent();

};

template <class T>
requires std::is_base_of_v<Component, T>
bool Entity::HasComponent()
{
  for (auto component : this->components)
  {
    if (component->GetTypeId() == T::TypeId())
    {
      return true;
    }
  }
  return false;
}

template <class T>
requires std::is_base_of_v<Component, T>
T* Entity::GetComponent()
{
  for (auto component : this->components)
  {
    if (component->GetTypeId() == T::TypeId())
    {
      return (T*)component;
    }
  }
  return nullptr;
}

class Scene
{
public:
  std::vector<Entity*> entities;


  template<class T>
  T* AddEntity(T* ptr)
  {
    entities.push_back(ptr);
    ptr->Init();
    return ptr;
  }
};

class Ecs : public IModule
{
public:
  Scene scene;
  void Init() override;
  void Update() override;
  void Render() override;
  void ProcessEvent(SDL_Event& event) override;
};
extern std::shared_ptr<Ecs> ecs;

template<class ...T>
auto FilterEntities()
{
  return  std::views::filter(ecs->scene.entities, [](Entity* e){
      return (... && e->HasComponent<T>());
    })
    | std::views::transform([](Entity* e){
      return std::tuple(
        e->GetComponent<T>()...
      );
    });
}