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

#include "span_dynamic.h"


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

//####################################################
//################## ID system #######################
//####################################################
#include <map>

#include "id_system.h"

//####################################################
//################## Entity base classes #############
//####################################################



//####################################################
//################### Archetype ######################
//####################################################

using Types = std::vector<TypeId>;

Types sortTypes(Types t);

class Archetype
{
public:
  using Id = uint32_t;
  static Id next_id;

  using Columns = std::map<TypeId, int>;


  Id id;
  Columns types;

  std::vector<SH::span_dynamic> columns;


  Archetype() = default;

  Archetype(const std::initializer_list<TypeId> types_list) : Archetype(std::vector(types_list)) {}

  explicit Archetype(const Types& types_list);
};

void PrintArchetype(const Archetype& a);



template<>
struct std::hash<Types> {
  std::size_t operator()(const Types &vec) const noexcept {
    std::size_t seed = vec.size();
    for (auto &i : vec) {
      seed ^= i.id + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

class EntityManager
{
public:
  std::unordered_map<Types, Archetype> archetypes;

  EntityManager(): archetypes()
  {
  } ;

  Archetype& GetArchetype(const Types& types);



};