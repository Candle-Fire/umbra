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

#include <fmt/format.h>

//#include "../../shadow/shadow-engine/reflection/inc/shadow/SHObject.h"
//#include "../../shadow/shadow-engine/core/inc/shadow/exports.h"


namespace SH {

  class span_dynamic {
    std::byte *p_start;
    std::byte *p_end;
    size_t item_size;
  public:
    span_dynamic() = default;
    span_dynamic(void *mem, size_t item_size, size_t count) :
        p_start(static_cast<std::byte *>(mem)),
        p_end(p_start + (item_size * count)),
        item_size(item_size) {
        assert((p_end - p_start) / item_size == count);
    }

  public:
    class iterator {
    public:
      using difference_type = std::ptrdiff_t;
      using value_type = std::byte;
    private:
      std::byte *pos;
      size_t size;

    public:
      iterator() : pos(nullptr), size(0) {};
      iterator(std::byte *p, size_t item_size) : pos(p), size(item_size) {};

      void Move(size_t n) {
          pos += (n * size);
      }

      iterator &operator++() {
          Move(1);
          return *this;
      }
      iterator operator++(int) {
          iterator tmp(*this);
          operator++();
          return tmp;
      }

      iterator &operator--() {
          Move(-1);
          return *this;
      }
      iterator operator--(int) {
          iterator tmp(*this);
          operator--();
          return tmp;
      }

      iterator &operator+=(difference_type n) {
          Move(n);
          return *this;
      }
      iterator &operator-=(difference_type n) {
          Move(-n);
          return *this;
      }

      iterator operator+(const difference_type &n) const {
          iterator tmp(*this);
          tmp.Move(n);
          return tmp;
      }
      iterator operator-(const difference_type &n) const {
          iterator tmp(*this);
          tmp.Move(-n);
          return tmp;
      }

      std::byte &operator[](const difference_type &n) const {
          iterator tmp(*this);
          tmp.Move(n);
          return *tmp;
      }

      difference_type operator-(const iterator &rhs) const { return pos - rhs.pos; }

      bool operator==(const iterator &rhs) const { return pos == rhs.pos; }
      bool operator!=(const iterator &rhs) const { return pos != rhs.pos; }

      bool operator<(const iterator &rhs) const { return pos < rhs.pos; }
      bool operator<=(const iterator &rhs) const { return pos <= rhs.pos; }
      bool operator>(const iterator &rhs) const { return pos > rhs.pos; }
      bool operator>=(const iterator &rhs) const { return pos >= rhs.pos; }

      std::byte &operator*() const { return *pos; }

      template<class T>
      T &as() { return *(T *) pos; }
      template<class T>
      T *as_ptr() { return (T *) pos; }

      [[nodiscard]] void *ptr() const { return pos; }
    };

    [[nodiscard]] iterator begin() const {
        return {p_start, item_size};
    }

    iterator end() const {
        return {p_end, item_size};
    }

    iterator last() const {
        return end()--;
    }
  };

  span_dynamic::iterator operator+(span_dynamic::iterator::difference_type n, span_dynamic::iterator i) {
      span_dynamic::iterator tmp(i);
      tmp.Move(n);
      return tmp;
  }
  span_dynamic::iterator operator-(span_dynamic::iterator::difference_type n, span_dynamic::iterator i) {
      span_dynamic::iterator tmp(i);
      tmp.Move(-n);
      return tmp;
  }

  bool operator<(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() < rhs; }
  bool operator<(const void *lhr, const span_dynamic::iterator &rhs) { return lhr < rhs.ptr(); }
  bool operator<=(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() <= rhs; }
  bool operator>(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() > rhs; }
  bool operator>(const void *lhr, const span_dynamic::iterator &rhs) { return lhr > rhs.ptr(); }
  bool operator>=(const span_dynamic::iterator &lhr, const void *rhs) { return lhr.ptr() >= rhs; }
  bool operator>=(const void *lhr, const span_dynamic::iterator &rhs) { return lhr >= rhs.ptr(); }
}

static_assert(std::input_or_output_iterator<SH::span_dynamic::iterator>);
static_assert(std::random_access_iterator<SH::span_dynamic::iterator>);

static_assert(std::ranges::random_access_range<SH::span_dynamic>);

namespace SH {

  class PoolAllocator {
  public:

    struct Item {
      Item *next;
    };

    explicit PoolAllocator(size_t item_size) : item_size(std::max(item_size, sizeof(Item))) {
        chunks.push_back(new Chunk(item_size));
    }

    class Chunk {
      span_dynamic memory; //= span_dynamic(nullptr, 0, 0);
      Item *next_free = nullptr;
    public:
      explicit Chunk(size_t item_size) {
          void *m = malloc(item_size * 1024);
          memory = span_dynamic(m, item_size, 1024);

          next_free = memory.begin().as_ptr<Item>();
          for (auto it = memory.begin(); it != memory.end(); it++) {
              auto next = it + 1;
              it.as_ptr<Item>()->next = next.as_ptr<Item>();
          }
          memory.last().as_ptr<Item>()->next = nullptr;
      }

      bool HasSpace() { return next_free != nullptr; }

      void *allocate() {
          assert(next_free != nullptr);
          auto ptr = next_free;
          next_free = next_free->next;
          return ptr;
      }

      void deallocate(void *p) {
          assert(contains(p));
          auto item = static_cast<Item *>(p);
          item->next = next_free;
          next_free = item;
      }

      bool contains(void *p) {
          return p >= memory.begin() && p < memory.end();
      }
    };

  public:

    virtual void *allocate() {
        auto chunk_it = std::ranges::find_if(chunks, [](auto chunk) { return chunk->HasSpace(); });

        // If no chunk has space, create a new one
        if (chunk_it == chunks.end()) {
            chunks.push_back(new Chunk(item_size));
            chunk_it = std::prev(chunks.end());
        }

        return (*chunk_it)->allocate();
    };

    virtual void deallocate(void *p) {
        auto chunk_it = std::ranges::find_if(chunks, [p](auto chunk) { return chunk->contains(p); });
        if (chunk_it == chunks.end())
            throw std::invalid_argument("WTF man common use pointer correctly");

        (*chunk_it)->deallocate(p);
    };
  private:
    std::vector<Chunk *> chunks;
    size_t item_size;
  };

}

/*
 * Archetype : (T1, T2, T3, T4)
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

//####################################################
//################## ID system #######################
//####################################################
#pragma region ID-System

union Id {
  std::byte bytes[sizeof(uint64_t)];
  uint32_t half[2];
  uint64_t id;

  Id(uint64_t id) : id(id) {};
  Id(uint32_t high, uint32_t low) : half{high, low} {};

  Id Next() {
      return Id(this->id++);
  }
};

using TypeId = Id;

template<>
struct std::hash<TypeId> {
  std::size_t operator()(const TypeId &s) const noexcept {
      return std::hash<uint64_t>{}(s.id);
  };
};

bool operator==(const Id &lhs, const Id &rhs) { return lhs.id == rhs.id; }

std::unordered_map<TypeId, std::string> typeMap;

TypeId next_id(0);

template<class T>
TypeId GetTypeId() {
    static TypeId id = next_id.id++;
    static auto a = typeMap.insert({id, typeid(T).name()});
    return id;
}

#pragma endregion ID-System

//####################################################
//################## Entity base classes #############
//####################################################

class Object {

};

class Component : public Object {
public:
  static constexpr bool isEntity = false;
  Id UUID;

  Component(Id UUID) : UUID(UUID) {};
};
template<class T>
concept component = std::is_base_of_v<Component, T>;

template<class T>
concept component_only = component<T> && T::isEntity == false;

class Entity : public Component {
public:
  static constexpr bool isEntity = true;
  Entity(Id UUID) : Component(UUID) {

  }

  template<component T>
  T *AddInternalChild();

};
template<class T>
concept entity = std::is_base_of_v<Entity, T> && T::isEntity == true;

//####################################################
//############### Prefab stuff #######################
//####################################################

class Asset {

};

class Prefab : public Asset {

};

class PrefabEntity : public Entity {
  Prefab p;
public:
  PrefabEntity(Id UUID, Prefab asset) : Entity(UUID), p(asset) {

  }

};

//####################################################
//################Built in entities ##################
//####################################################

class Scene : public Entity {
  std::string name = "Test";
public:
  Scene(Id UUID) : Entity(UUID) {};
};

enum Relation {
  PARENT = 1,
};

class Archetype {
public:
  using Id = uint32_t;
  static Id next_id;

  using Types = std::vector<TypeId>;

  Types types;
  Id id;

  using Column = std::vector<Object *>;

  std::vector<Column> data = std::vector<Column>(0);

  static Types sortTypes(Types t) {
      std::ranges::sort(t, [](auto a, auto b) { return a.id < b.id; });
      return t;
  }
public:
  Archetype() = default;
  Archetype(std::initializer_list<TypeId> types) : types(sortTypes(types)), id(next_id++) {}
  Archetype(Types types) : types(sortTypes(types)), id(next_id++) {}

  std::pair<size_t, Column &> AddRow() {
      auto &row = data.emplace_back(Column(types.size()));
      return {data.size() - 1, row};
  }

  size_t getColumn(TypeId column_type) {
      auto a = std::ranges::find(types, column_type);
      if(a == types.end())
          return -1;

      size_t const pos = std::distance(types.begin(), a);
      return pos;
  }

  void RemoveRow(size_t i) {
      data.erase(data.begin() + i);
  };
};

template<>
struct std::hash<Archetype::Types> {
  std::size_t operator()(const Archetype::Types &vec) const noexcept {
      std::size_t seed = vec.size();
      for (auto &i : vec) {
          seed ^= i.id + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
  }
};

class EntityManager {

  std::unordered_map<TypeId, SH::PoolAllocator> pools;

  template<component Ent>
  SH::PoolAllocator &GetPool(TypeId type) {
      if (!pools.contains(type)) {
          pools.emplace(type, SH::PoolAllocator(sizeof(Ent)));
      }
      return pools.at(type);
  }

  std::unordered_map<Archetype::Types, Archetype> archetypes;

  struct ArchetypeRecord {
    Archetype *archetype;
    size_t row;
  };

  std::unordered_map<Id, ArchetypeRecord> entity_archetype;

  Archetype &GetArchetype(Archetype arc) {
      if (archetypes.contains(arc.types)) {
          return archetypes.at(arc.types);
      } else {
          auto res = archetypes.emplace(arc.types, arc).first;
          return res->second;
      }
  }

  ArchetypeRecord &GetEntityArchetype(Entity &ent) {
      return entity_archetype.at(ent.UUID);
  }

  uint32_t nextUUID = 0;
  Id GetNewUUID() {
      return Id(nextUUID++, 0);
  }

  template<component Ent>
  void *AllocateForNew() {
      SH::PoolAllocator &pool = GetPool<Ent>(GetTypeId<Ent>());
      void *pos = pool.allocate();
      return pos;
  }

  void MoveToArchetype(ArchetypeRecord &src, Archetype &dst){
      auto dst_row = dst.AddRow();

      for (size_t i = 0; i < src.archetype->types.size(); i++) {
          auto column_type = src.archetype->types[i];
          size_t dest_column = dst.getColumn(column_type);
          if(dest_column == -1)
              continue;
          dst_row.second[dest_column] = src.archetype->data[src.row][i];
      }

      src.archetype->RemoveRow(src.row);

      src.row = dst_row.first;
      src.archetype = &dst;
  }

public:
  static EntityManager *entity_manager;

  EntityManager() {
      entity_manager = this;
  }

  template<component_only Comp>
  Comp *AddChild(Entity &parent) {
      // Allocate the space for the entity
      const TypeId &comp_id = GetTypeId<Comp>();

      void *memory = AllocateForNew<Comp>();

      //Find old archetype
      auto &record = GetEntityArchetype(parent);
      auto &old_arch = *record.archetype;

      //Construct new type list and find the archetype
      Archetype::Types types(old_arch.types);
      types.emplace_back(comp_id);

      Archetype &new_arch = GetArchetype(Archetype(types));

      MoveToArchetype(record, new_arch);

      record.archetype->data[record.row][new_arch.getColumn(comp_id)] = (Comp *) memory;
      // Create it
      Comp *component = new(memory)Comp(GetNewUUID());

      // Add it to the parent
      return component;
  }

  template<entity Ent>
  Ent *AddChild(Entity &parent) {
      const TypeId &type_id = GetTypeId<Ent>();
      
      // Allocate the space for the entity
      void *pos = AllocateForNew<Ent>();

      //Find or make archetype
      Archetype &a = GetArchetype(
          Archetype({
                        type_id,
                        Id(parent.UUID.half[0], Relation::PARENT)
                    }));

      auto &row = a.AddRow().second;
      row[a.getColumn(type_id)] = (Ent *) pos;

      Id Uuid = GetNewUUID();

      entity_archetype.insert({Uuid, ArchetypeRecord{&a, a.getColumn(type_id)}});

      // Create it
      Ent *entity = new(pos)Ent(Uuid);

      return entity;
  }

  /*
   * Add a fresh new entity
   */
  template<entity Ent>
  Ent *Add() {
      // Allocate the space for the entity
      void *pos = AllocateForNew<Ent>();

      //Find or make archetype
      Archetype &a = GetArchetype(Archetype({GetTypeId<Ent>()}));

      auto &row = a.AddRow().second;
      row[0] = (Ent *) pos;

      Id Uuid = GetNewUUID();

      entity_archetype.insert({Uuid, ArchetypeRecord{&a, a.getColumn(GetTypeId<Ent>())}});

      // Create it
      Ent *entity = new(pos)Ent(Uuid);

      return entity;
  }

  #pragma region DumpData
  std::string GetTypeOrRelationName(Id type_id) {
      std::string res = "";
      if (type_id.half[1] > 0) {
          switch (type_id.half[1]) {
              case Relation::PARENT:res += "Parent: ";
                  break;
              default:break;
          }
      }
      res += typeMap.at(type_id.half[0]);
      return res;
  }

  void DumpData() {

      {
          std::printf("Pools: \n");
          for (auto &pool : pools) {
              fmt::print("{0:20} count:{1}\n", typeMap.at(pool.first), "??");
          }
          std::printf("\n");
      }

      {
          std::printf("Archetypes: \n");
          size_t max_len = 0;
          for (auto &arch : archetypes) {
              std::vector<std::string> target(arch.second.types.size());
              for (int i = 0; i < arch.second.types.size(); ++i) {
                  target[i] = GetTypeOrRelationName(arch.second.types[i]);
              }
              max_len = std::max(max_len, fmt::format("{}", fmt::join(target, " | ")).size());
          }

          for (auto &arch : archetypes) {
              std::printf("%i :( ", arch.second.id);

              std::vector<std::string> target(arch.second.types.size());
              for (int i = 0; i < arch.second.types.size(); ++i) {
                  target[i] = GetTypeOrRelationName(arch.second.types[i]);
              }

              fmt::print("{0:<{1}} )\t\t", fmt::format("{}", fmt::join(target, " | ")), max_len);
              fmt::print("count: {}", arch.second.data.size());

              fmt::print("\n");
          }
          fmt::print("\n");
      }

      std::printf("Entity map: \n");
      for (auto &data : entity_archetype) {

          fmt::print("ID: {0}, (row: {1}, arch: {2})", data.first.id, data.second.row, data.second.archetype->id);

          fmt::println("");
      }
  }
  #pragma endregion DumpData

};

template<component T>
T *Entity::AddInternalChild() {
    return EntityManager::entity_manager->AddChild<T>(*this);
}

EntityManager *EntityManager::entity_manager = nullptr;

Archetype::Id Archetype::next_id = 0;


