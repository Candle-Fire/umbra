#pragma once

#include <cassert>
#include <cstdint>
#include <ranges>
#include <unordered_map>


using TypeId = uint32_t;

//inline int operator<(const TypeId& lhs, const TypeId& rhs){ return rhs > lhs; }
//inline bool operator==(const TypeId& lhs, const TypeId& rhs) { return lhs == rhs; }


struct TypeInfo
{
  TypeId id;
  std::string name;
  size_t size;
};
extern std::unordered_map<std::string, TypeInfo> typeMap;

extern uint32_t next_id;



template <class T>
TypeInfo GetTypeId()
{
  const char* name = typeid(T).name();
  if(!typeMap.contains(name))
  {
    TypeInfo newId = {
      .id = next_id++,
      .name = name,
      .size = sizeof(T),
    };
    typeMap.emplace(name, newId);
    return newId;
  }
  return typeMap.at(name);
}

TypeInfo GetTypeInfoById(const TypeId& id);

std::string GetTypeNameByID(TypeId id);



