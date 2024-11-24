#include "ecs.exp.h"

#include <iostream>
#include "lib/rang.hpp"

Archetype::Id Archetype::next_id = 0;

Types sortTypes(Types t)
{
  std::ranges::sort(t, [](auto a, auto b) { return a < b; });
  return t;
}

Archetype::Archetype(const Types& types_list): id(next_id++)
{
  size_t next_c          = 0;
  types = sortTypes(types_list);
  std::ranges::for_each(types, [&](const TypeId& type)
  {
    if(test(type.flags, TypeFlags::Flag))
    {
      this->column_map.insert({type, -1});
    }
    else
    {
      this->column_map.insert({type, next_c++});
      const TypeInfo& info = GetTypeInfoById(type);
      void* page    = malloc(info.size * PAGE_SIZE);
      this->columns.push_back({page, info.size, PAGE_SIZE});
    }
  });

  empty = 0;
  size_t next = 1;
  for (int i = 0; i < PAGE_SIZE-1; ++i)
  {
    rows.push_back({.next = next++});
  }
  rows.push_back({.next = (size_t)-1});
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




void PrintArchetype(const Archetype& a)
{
  std::cout << "# Archetype ID: " << a.id << std::endl;
  std::cout << "  Types:" << std::endl;
  for (const auto& [type, column] : a.column_map)
  {
    std::cout << "  \t"
      << "Type: " << GetTypeNameByID(type) << " (" << type.id << ")"
      <<" Column: " << column;
    if(column >= 0)
    {
      std::cout << " Page: " << a.columns[column].begin().ptr();
    }

    std::cout << std::endl;
  }

  for (int i = 0; i < a.rows.size(); ++i)
  {
    if(a.rows[i].next == -1)
    {
      std::cout << rang::fg::green << "█" << rang::style::reset;
    }
    else
    {
      std::cout << rang::fg::gray << "█" << rang::style::reset;
    }
    if((i+1) % (PAGE_SIZE/4) == 0)
    {
      std::cout << std::endl;
    }
  }
  std::cout << std::endl;

}

void PrintEM(EntityManager& em)
{
  std::cout << "Entity Manager" << std::endl;

  for (auto archetype : em.archetypes)
  {
    PrintArchetype(archetype.second);
  }

  std::cout << "----------------" << std::endl;
  std::cout << "\tEntities: " << std::endl;

  for (const auto& [id, record] : em.entities)
  {
    std::cout << "Entity: " << id
              << " Arch: " << record.archetype->id
              << " Row: " << record.row << std::endl;
  }
}
