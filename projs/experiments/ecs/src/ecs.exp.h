#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <list>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>

#include "span_dynamic.h"

#include "id_system.h"

#include <cstring>

/*
 * Archetype : T1 (T2, T3, T4)
 * | self (T1)  | comp 1 (T2) | comp 2 (T3) | comp 3 (T4) |
 * | T1: 1      | T2: 1       | T3: 1       | T4: 1       |
 * | T1: 2      | T2: 2       | T3: 2       | T4: 2       |
 * | T1: 3      | T2: 3       | T3: 3       | T4: 3       |
 *
 * Archetype : (T1, T2, T3)
 * | self (T1)  | comp 1 (T2) | comp 2 (T3) |
 * | T1: 4      | T2: 4       | T3: 4       |
 * | T1: 5      | T2: 5       | T3: 5       |
 *
 *
 */

// ####################################################
// ################## Entity base classes #############
// ####################################################

using EntityId = uint32_t;

class Entity
{

};

class Component
{
public:
  virtual ~Component() = default;
  virtual void Print() const = 0;
};

// ####################################################
// ################### Archetype ######################
// ####################################################

using Types = std::vector<TypeId>;

Types sortTypes(Types t);

template <>
struct std::hash<Types>
{
  std::size_t operator()(const Types &vec) const noexcept
  {
    std::size_t seed = vec.size();
    for (auto &i : vec)
    {
      seed ^= i.id + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};


struct RowMeta
{
  // EntityId id;
  size_t next;
  bool in_use;
};

constexpr size_t PAGE_SIZE = 2*2*2*2*2*2;

class Archetype
{
public:
  using Id = uint32_t;
  static Id next_id;

  using ColumnMap = std::map<TypeId, int>;


  Id id;
  Types types;
  ColumnMap column_map;


  int next_free;
  std::vector<RowMeta> rows;
  std::vector<SH::span_dynamic> columns;


  Archetype() = default;

  Archetype(const std::initializer_list<TypeId> types_list) : Archetype(std::vector(types_list)) {}

  explicit Archetype(const Types &types_list);

  SH::span_dynamic& GetColumn(TypeId id)
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
  EntityRef &RemoveComponent();
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

  EntityCreationResult CreateEntity(const Types &type_id)
  {
    auto &a        = GetArchetype(type_id);
    const auto row = a.Allocate();

    EntityId id = GetEntityId();

    auto [fst, snd] = entities.emplace(id, EntityRecord{&a, row});
    return {
      .id = id,
      .archetype = &a,
      .row = row,
    };
  }

  EntityRef AddEntity()
  {
    auto [id, a, row] = CreateEntity({});
    return {this, id};
  }

  template <class T>
  EntityRefTyped<T> AddEntity()
  {
    TypeId type_id = GetTypeId<T>().id;
    auto [id, a, row] = CreateEntity({type_id});

    T* ptr = a->GetColumn(type_id)[row].as_ptr<T>();
    new(ptr) T();

    return EntityRefTyped<T>(this,id);
  }

  template <class T>
  void AddComponent(EntityId entity_id, T&& value)
  {
    const TypeId type_id = GetTypeId<T>().id;

    auto& record = entities[entity_id];
    Types new_types = record.archetype->types;
    new_types.push_back(type_id);
    new_types = sortTypes(new_types);

    auto &archetype       = GetArchetype(new_types);
    const auto target_row = archetype.Allocate();

    archetype.CopyFrom(*record.archetype, record.row, target_row);
    record.archetype->Deallocate(record.row);

    T* ptr = archetype.GetColumn(type_id)[target_row].as_ptr<T>();
    new(ptr)T(value);

    record.row = target_row;
    record.archetype = &archetype;
  }

  template <class T>
  void RemoveComponent(EntityId entity_id)
  {
    const TypeId type_id = GetTypeId<T>().id;
    auto& record = entities[entity_id];

    Types new_types = record.archetype->types;
    std::erase(new_types, type_id);
    new_types = sortTypes(new_types);

    auto &archetype       = GetArchetype(new_types);
    const auto target_row = archetype.Allocate();

    archetype.CopyFrom(*record.archetype, record.row, target_row);

    record.archetype->Deallocate(record.row);

    record.row = target_row;
    record.archetype = &archetype;
  }
};



template <typename T>
EntityRef &EntityRef::AddComponent(T &&val)
{
  em->AddComponent(id, std::forward<T>(val));
  return *this;
}
template <typename T>
EntityRef &EntityRef::RemoveComponent()
{
  em->RemoveComponent<T>(id);
  return *this;
}



void PrintArchetype(const Archetype &a);

void PrintEM(EntityManager &em);
