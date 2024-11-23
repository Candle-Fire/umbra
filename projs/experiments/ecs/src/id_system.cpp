#include "id_system.h"

std::unordered_map<std::string, TypeInfo> typeMap;

TypeId next_id({.id = 1});

std::size_t std::hash<TypeId>::operator()(const TypeId& s) const noexcept
{
  return std::hash<uint64_t>{}(s.id);
}

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

void __jumpTypeID(uint32_t n)
{
  next_id.id += n;
}
