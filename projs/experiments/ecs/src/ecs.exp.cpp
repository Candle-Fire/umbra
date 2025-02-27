#include "ecs.exp.h"

#include <iostream>
#include "lib/rang.hpp"


std::size_t std::hash<NodeType>::operator()(const NodeType& s) const noexcept
{
  return std::hash<uint64_t>{}(s.typeId);
}
Types sortTypes(Types t)
{
  std::ranges::sort(t, [](auto a, auto b) { return a < b; });
  return t;
}

Archetype::Id Archetype::next_id = 0;

Archetype::Id GetNextId()
{
  return Archetype::next_id++;
}

Archetype::Archetype(const Types& types_list): id(GetNextId())
{
  size_t next_c = 0;
  types = sortTypes(types_list);
  std::ranges::for_each(types, [&](const NodeType& type)
  {
    if(test(type.flags, TypeFlags::Flag))
    {
      this->column_map.insert({type, -1});
    }
    else
    {
      this->column_map.insert({type, next_c++});
      const TypeInfo& info = GetTypeInfoById(type.typeId);
      void* page    = malloc(info.size * PAGE_SIZE);
      this->columns.push_back({page, info.size, PAGE_SIZE});
    }
  });

  next_free = 0;
  size_t next = 1;
  for (int i = 0; i < PAGE_SIZE-1; ++i)
  {
    rows.push_back({.next = next++});
  }
  rows.push_back({.next = (size_t)-1, .in_use = false});
}




Archetype &EntityManager::GetArchetype(const Types &types)
{
  if (archetypes.contains(types))
  {
    return archetypes.at(types);
  }

  Archetype a(types);
  const auto res = archetypes.try_emplace(types, types);
  return res.first->second;
}

EntityManager::EntityCreationResult EntityManager::CreateEntity(const Types &type_id)
{
  auto &a        = GetArchetype(type_id);
  const auto row = a.Allocate();

  EntityId id = GetEntityId();

  auto [fst, snd] = entities.emplace(id, EntityRecord{&a, row});
  return {
    .id        = id,
    .archetype = &a,
    .row       = row,
  };
}

void EntityManager::MoveEntity(const EntityId &id, const Types &types, EntityCreationResult &result)
{
  auto &record = entities[id];

  auto &archetype       = GetArchetype(types);
  const auto target_row = archetype.Allocate();

  archetype.CopyFrom(*record.archetype, record.row, target_row);

  record.archetype->Deallocate(record.row);

  record.row       = target_row;
  record.archetype = &archetype;

  result.id        = id;
  result.archetype = &archetype;
  result.row       = record.row;
}

EntityRef EntityManager::AddEntity()
{
  auto [id, a, row] = CreateEntity({});
  return {this, id};
}


// ###################################
// Print Helpers
//###################################

void PrintArchetype(const Archetype& a)
{
  std::cout << "# Archetype ID: " << a.id << std::endl;
  std::cout << "  Types:" << std::endl;
  for (const auto& [type, column] : a.column_map)
  {
    std::cout << "  \t"
      << "Type: " << GetTypeNameByID(type.typeId) << " (" << type.typeId << ")"
      <<" Column: " << column;
    if(column >= 0)
    {
      std::cout << " Page: " << a.columns[column].begin().ptr();
    }

    std::cout << std::endl;
  }

  for (int i = 0; i < a.rows.size(); ++i)
  {
    if(a.rows[i].in_use)
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


    for (auto map : record.archetype->column_map)
    {
      if(map.second >= 0)
      {
        std::cout << "\t";
        const auto& comp = record.archetype->columns[map.second][record.row].as<Component<>>();
        comp.Print();
      }
      else
      {
        std::cout << "\t";
        std::cout << GetTypeNameByID(map.first.typeId);
        if (test(map.first.flags, TypeFlags::Relation))
        {
          std::cout << " -> " << map.first.entity;
        }
      }
    }

    std::cout << std::endl;
  }
}
