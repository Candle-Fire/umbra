#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <cassert>
#include <cstdint>
#include <ranges>
#include <algorithm>
#include <functional>
#include <numeric>

enum class TypeFlags : std::uint8_t {
  None     = 0,
  Flag     = 1 << 1,
  Relation = 1 << 2,
  Unused   = 1 << 3,
  Unused2  = 1 << 4,
};

inline TypeFlags operator|(TypeFlags lhs, TypeFlags rhs) {
  return static_cast<TypeFlags>(
      static_cast<std::underlying_type_t<TypeFlags>>(lhs) |
      static_cast<std::underlying_type_t<TypeFlags>>(rhs)
  );
}
inline TypeFlags operator&(const TypeFlags& lhs, const TypeFlags& rhs)
{
  return static_cast<TypeFlags>(
      static_cast<std::underlying_type_t<TypeFlags>>(lhs) &
      static_cast<std::underlying_type_t<TypeFlags>>(rhs)
  );
}
inline bool test(const TypeFlags& lhs, const TypeFlags& rhs)
{
  return static_cast<std::underlying_type_t<TypeFlags>>(lhs & rhs);
}




struct __attribute__((packed)) TypeId
{
  uint32_t id     : 28;
  TypeFlags flags : 4;
  uint32_t entity;
};
static_assert(sizeof(TypeId) == sizeof(uint64_t));

inline int operator<(const TypeId& lhs, const TypeId& rhs){ return rhs.id > lhs.id; }
inline bool operator==(const TypeId& lhs, const TypeId& rhs) { return lhs.id == rhs.id; }

template <>
struct std::hash<TypeId>
{
  std::size_t operator()(const TypeId& s) const noexcept;;
};



struct TypeInfo
{
  TypeId id;
  std::string name;
  size_t size;
};



extern std::unordered_map<std::string, TypeInfo> typeMap;

extern TypeId next_id;

template <class T>
TypeInfo GetTypeId(const TypeFlags& flags = TypeFlags::None)
{
  const char* name = typeid(T).name();
  if(!typeMap.contains(name))
  {
    TypeInfo newId = {
      .id = {.id = next_id.id++, .flags = flags},
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

void __jumpTypeID(uint32_t n);
