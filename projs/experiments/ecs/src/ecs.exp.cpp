//
// Created by dpeter99 on 22/11/24.
//
#include "ecs.exp.h"

#include <iostream>

Archetype::Id Archetype::next_id = 0;

Types sortTypes(Types t)
{
  std::ranges::sort(t, [](auto a, auto b) { return a.id < b.id; });
  return t;
}

Archetype::Archetype(const Types& types_list): id(next_id++)
{
  size_t next_c          = 0;
  const auto sortedTypes = sortTypes(types_list);
  std::ranges::for_each(sortedTypes, [&](const TypeId& type)
  {
    if(test(type.flags, TypeFlags::Flag))
    {
      this->types.insert({type, -1});
    }
    else
    {
      this->types.insert({type, next_c++});
      TypeInfo into = GetTypeInfoById(type);
      void* page    = malloc(into.size * 1024);
      this->columns.push_back({page, into.size, 1024});
    }
  });
}

void PrintArchetype(const Archetype& a)
{
  std::cout << "Architype ID: " << a.id << std::endl;
  std::cout << "Types:" << std::endl;
  for (const auto& [type, column] : a.types)
  {
    std::cout << "\t"
      << "Type: " << GetTypeNameByID(type) << " (" << type.id << ")"
      <<" Column: " << column;
    if(column >= 0)
    {
      std::cout << " Page: " << a.columns[column].begin().ptr();
    }



    std::cout << std::endl;
  }

}



Archetype& EntityManager::GetArchetype(const Types& types)
{
  if(archetypes.contains(types))
  {
    return archetypes.at(types);
  }

  Archetype a(types);
  const auto res = archetypes.emplace(types, Archetype{types});
  return res.first->second;
}
