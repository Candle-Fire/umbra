#include "id_system.h"

std::unordered_map<std::string, TypeInfo> typeMap;

TypeId next_id = 1;

TypeInfo GetTypeInfoById(const TypeId& id)
{
  return std::ranges::find_if(typeMap, [id](const auto& i)
  {
    return i.second.id == id;
  })->second;
}

std::string GetTypeNameByID(TypeId id)
{
  return std::ranges::find_if(typeMap, [id](const auto& i)
  {
    return i.second.id == id;
  })->first;
}
