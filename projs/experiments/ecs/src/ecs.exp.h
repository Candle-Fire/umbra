#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <list>
#include <map>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "span_dynamic.h"

#include "id_system.h"

#define __OUT__

/**
* This file contains the Entity System
* The game world is built up from nodes
* Each Node has type
* There are N types of nodes
* - Entites
* - Components
* - Flags
* - Connections
*/



using EntityId = uint32_t;


/**
* Flag enum used to specify ECS node properties
* it is a 4 bit field the est of the uint8 should not be used as it will be cut of
*/
enum class TypeFlags : std::uint8_t {
  None     = 0,
  Flag     = 1 << 1,
  Relation = 1 << 2,
  Unused   = 1 << 3,
  Unused2  = 1 << 4,

  SimpleRelation = Flag | Relation,
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

template <typename T>
concept HasTypeFlags = requires
{
  T::Flags;
  std::is_same_v<TypeFlags, decltype(T::Flags)>;
};

/**
* Type id used by the Entity system to identify the type of the node
* A node can be either :
* - Simple data
* - A connection type
*/
struct __attribute__((packed)) NodeType
{
  TypeId typeId   : 28;
  TypeFlags flags : 4;
  uint32_t entity;
};
static_assert(sizeof(NodeType) == sizeof(uint64_t));

inline int operator<(const NodeType& lhs, const NodeType& rhs){ return rhs.typeId < lhs.typeId; }
inline int operator<=(const NodeType& lhs, const NodeType& rhs){ return rhs.typeId <= lhs.typeId; }
inline int operator>(const NodeType& lhs, const NodeType& rhs){ return rhs.typeId > lhs.typeId; }
inline int operator>=(const NodeType& lhs, const NodeType& rhs){ return rhs.typeId >= lhs.typeId; }
inline bool operator==(const NodeType& lhs, const NodeType& rhs) { return lhs.typeId == rhs.typeId; }
inline bool operator!=(const NodeType& lhs, const NodeType& rhs) { return lhs.typeId != rhs.typeId; }

template <>
struct std::hash<NodeType>
{
  std::size_t operator()(const NodeType& s) const noexcept;
};

template <HasTypeFlags T>
NodeType GetNodeType()
{
  const auto id = GetTypeId<T>().id;
  const TypeFlags flags = T::Flags;
  const NodeType node = {
    .typeId = id,
    .flags = flags,
    .entity = 0,
  };
  return node;
}

template <HasTypeFlags T>
NodeType GetRelationType(const EntityId& other)
{
  const auto id = GetTypeId<T>().id;
  const TypeFlags flags = T::Flags;
  const NodeType node = {
    .typeId = id,
    .flags = flags,
    .entity = other,
  };
  return node;
}




/*
 * Archetype : T1, (T2, T3, T4)
 * | self (T1)  | comp 1 (T2) | comp 2 (T3) | comp 3 (T4) |
 * | T1: 1      | T2: 1       | T3: 1       | T4: 1       |
 * | T1: 2      | T2: 2       | T3: 2       | T4: 2       |
 * | T1: 3      | T2: 3       | T3: 3       | T4: 3       |
 *
 * Archetype : T1, (T2, T3)
 * | self (T1)  | comp 1 (T2) | comp 2 (T3) |
 * | T1: 4      | T2: 4       | T3: 4       |
 * | T1: 5      | T2: 5       | T3: 5       |
 *
 *
 */

// ####################################################
// ################## Entity base classes #############
// ####################################################

template <TypeFlags F = TypeFlags::None>
class Component
{
public:
  static constexpr TypeFlags Flags = F;

  virtual ~Component() = default;
  virtual void Print() const = 0;
};

class Entity : Component<>
{

};


struct ChildOf final : Component<TypeFlags::SimpleRelation>
{
  void Print() const override
  {
    printf("ChildOf");
  }
};

// ####################################################
// ################### Archetype ######################
// ####################################################

// TODO: Find a better sorted storage for types
using Types = std::vector<NodeType>;

Types sortTypes(Types t);

inline Types addType(const Types &t, const NodeType id)
{
  Types new_types = t;
  new_types.push_back(id);
  new_types = sortTypes(new_types);
  return new_types;
}

inline Types removeType(const Types &t, const NodeType id)
{
  Types new_types = t;
  std::erase(new_types, id);
  new_types = sortTypes(new_types);
  return new_types;
}

template <>
struct std::hash<Types>
{
  std::size_t operator()(const Types &vec) const noexcept
  {
    std::size_t seed = vec.size();
    for (auto &i : vec)
    {
      seed ^= i.typeId + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};



struct RowMeta
{
  size_t next;
  bool in_use;
};

constexpr size_t PAGE_SIZE = 2*2*2*2*2*2;

class Archetype
{
public:
  using Id = uint32_t;
  static Id next_id;

  using ColumnMap = std::map<NodeType, int>;


  Id id;
  Types types;
  ColumnMap column_map;


  int next_free;
  std::vector<RowMeta> rows;
  std::vector<SH::span_dynamic> columns;


  Archetype() = default;

  Archetype(const std::initializer_list<NodeType> types_list) : Archetype(std::vector(types_list)) {}

  explicit Archetype(const Types &types_list);

  SH::span_dynamic& GetColumn(NodeType id)
  {
    return  columns[column_map.at(id)];
  }

  /**
  * Returns the index of a new row or -1 if it is full
  */
  size_t Allocate()
  {
    if(next_free == -1)
    {
      return -1;
    }
    auto row = next_free;
    next_free = rows[row].next;
    rows[row].next = -1;
    rows[row].in_use = true;
    return row;
  }


  void Deallocate(size_t row)
  {
    assert(row < rows.size() && "Invalid row index");
    rows[row].next = next_free;
    rows[row].in_use = false;
    next_free = row;
  }

  void CopyFrom(const Archetype &source, const size_t src_row, const size_t target_row) const
  {
    assert(src_row < source.rows.size() && target_row < this->rows.size() && "Invalid row indices");

    size_t source_index = 0;
    size_t dest_index = 0;

    while (source_index < source.types.size() && dest_index < this->types.size())
    {
      if (source.types[source_index] == this->types[dest_index])
      {
        const int source_column_index = source.column_map.at(source.types[source_index]);
        const int dest_column_index = this->column_map.at(this->types[dest_index]);

        auto& dest_column = this->columns[dest_column_index];
        auto& source_column = source.columns[source_column_index];

        void *dest       = dest_column[target_row].ptr();
        const void * src = source_column[src_row].ptr();

        assert(dest_column.element_size() == source_column.element_size() && "Element sizes don't match");

        std::memcpy(dest, src, source_column.element_size());

        ++source_index;
        ++dest_index;
      }
      else if (source.types[source_index] < this->types[dest_index])
      {
        ++source_index;
      }
      else
      {
        ++dest_index;
      }
    }
  }
};


class EntityManager;

struct EntityRef
{
  EntityManager *em;
  EntityId id;

  EntityRef(EntityManager *em, const EntityId &id) : em(em), id(id) {}

  template <typename T>
  EntityRef &AddComponent(T &&val);

  template <typename T>
  EntityRef &AddRelation(const EntityRef& other);

  template <typename T>
  EntityRef &RemoveComponent();

  template <typename T>
  T& GetComponent();
};


template <class T>
struct EntityRefTyped : EntityRef
{
  EntityRefTyped(EntityManager *em, const EntityId& id) : EntityRef(em, id) {}
};


class EntityManager
{
public:
  std::unordered_map<Types, Archetype> archetypes;


  struct EntityRecord
  {
    Archetype* archetype;
    size_t row;
  };

  std::unordered_map<EntityId, EntityRecord> entities;

  EntityId next_id;

  EntityManager() : next_id(1){};

  Archetype &GetArchetype(const Types &types);

  EntityId GetEntityId()
  {
    return next_id++;
  }

  struct EntityCreationResult
  {
    EntityId id;
    Archetype* archetype;
    size_t row;
  };

  EntityCreationResult CreateEntity(const Types &type_id);

  void MoveEntity(const EntityId &id, const Types &types, __OUT__ EntityCreationResult &result);

  EntityRef AddEntity();

  template <class T>
  EntityRefTyped<T> AddEntity()
  {
    NodeType type_id = GetTypeId<T>().id;
    auto [id, a, row] = CreateEntity({type_id});

    T* ptr = a->GetColumn(type_id)[row].as_ptr<T>();
    new(ptr) T();

    return EntityRefTyped<T>(this,id);
  }

  template <class T>
  void AddComponent(EntityId entity_id, T&& value)
  {
    const NodeType type_id = GetNodeType<T>();
    auto& record = entities[entity_id];

    Types new_types = addType(record.archetype->types, type_id);

    EntityCreationResult res;
    MoveEntity(entity_id, new_types, res);

    T* ptr = res.archetype->GetColumn(type_id)[res.row].as_ptr<T>();
    new(ptr)T(value);
  }

  template <class T>
  void RemoveComponent(EntityId entity_id)
  {
    const NodeType type_id = GetTypeId<T>().id;
    auto& record = entities[entity_id];

    Types new_types = removeType(record.archetype->types, type_id);

    EntityCreationResult res;
    MoveEntity(entity_id, new_types, res);
  }

  template <class T>
  void AddRelation(EntityId entity_id, EntityId other)
  {
    const NodeType type_id = GetRelationType<T>(other);
    auto& record = entities[entity_id];
    Types new_types = addType(record.archetype->types, type_id);

    EntityCreationResult res;
    MoveEntity(entity_id, new_types, res);
  }

};

template <typename T>
EntityRef &EntityRef::AddComponent(T &&val)
{
  em->AddComponent(id, std::forward<T>(val));
  return *this;
}
template <typename T>
EntityRef &EntityRef::AddRelation(const EntityRef& other)
{
  em->AddRelation<T>(id, other.id);
  return *this;
}
template <typename T>
EntityRef &EntityRef::RemoveComponent()
{
  em->RemoveComponent<T>(id);
  return *this;
}
template <typename T>
T& EntityRef::GetComponent()
{
  const auto& record = em->entities.at(id);
  const auto& column = record.archetype->GetColumn(GetNodeType<T>());
  return column[record.row].template as<T>();
}


class ISystem
{
public:
  virtual ~ISystem() = default;
  virtual  void Run(EntityManager& em) = 0;
};

template<class... T>
class System : public ISystem{
public:
  Types query;
  std::function<void(T&...)> action;

  explicit System(const std::function<void(T &...)> &action) : action(action) { query = {GetNodeType<T>()...}; }

  ~System() override;


  void Run(EntityManager &em) override
  {
    for (auto archetype : em.archetypes)
    {
      if (std::ranges::includes(archetype.second.types, query))
      {
        std::vector<>
      }
    }
  }
};

class SystemManager
{
  std::vector<ISystem> systems;

  EntityManager& em;

public:
  explicit SystemManager(EntityManager &em) : em(em) {}

  void addSystem(ISystem& system)
  {
    systems.push_back(system);
  }

  void runAllSystems()
  {
    for (ISystem &system : systems)
    {
      system.Run(em);
    }
  }
};


void PrintArchetype(const Archetype &a);

void PrintEM(EntityManager &em);
