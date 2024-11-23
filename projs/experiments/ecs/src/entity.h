
#pragma once

class Object
{
};

class Component : public Object
{
public:
  static constexpr bool isEntity = false;
  Id UUID;

  Component(Id UUID) : UUID(UUID)
  {
  };
};

template <class T>
concept component = std::is_base_of_v<Component, T>;

template <class T>
concept component_only = component<T> && T::isEntity == false;

class Entity : public Component
{
public:
  static constexpr bool isEntity = true;

  Entity(Id UUID) : Component(UUID)
  {
  }

  template <component T>
  T* AddInternalChild();
};

template <class T>
concept entity = std::is_base_of_v<Entity, T> && T::isEntity == true;