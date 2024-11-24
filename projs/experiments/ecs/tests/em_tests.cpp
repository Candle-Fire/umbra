#include "./ecs.exp.h"
#include "catch2/catch.hpp"

struct A {};
struct Position {};
struct C {};

TEST_CASE("EM archetype storage") {
  SECTION("Gives the correct Archetype") {
    EntityManager em;
    auto& a = em.GetArchetype({
        GetTypeId<A>().id,
        GetTypeId<C>(TypeFlags::Flag).id,
        GetTypeId<Position>().id,
    });

    REQUIRE(a.id > 0);
  }

  SECTION("Gives the same Archetype") {
    EntityManager em;
    auto& a = em.GetArchetype({
        GetTypeId<A>().id,
        GetTypeId<C>(TypeFlags::Flag).id,
        GetTypeId<Position>().id,
    });

    auto& b = em.GetArchetype({
        GetTypeId<A>().id,
        GetTypeId<C>(TypeFlags::Flag).id,
        GetTypeId<Position>().id,
    });

    REQUIRE(a.id == b.id);
    REQUIRE(&a == &b);
  }

  SECTION("Archetype map matches internal Type list")
  {
    EntityManager em;
    auto& a = em.GetArchetype({
        GetTypeId<A>().id,
        GetTypeId<C>(TypeFlags::Flag).id,
        GetTypeId<Position>().id,
    });

    for (auto [types, arch] : em.archetypes)
    {
      Types arch_types;
      std::ranges::transform(arch.column_map, std::back_inserter(arch_types), [](auto i)
      {
        return i.first;
      });

      REQUIRE(std::ranges::equal(types, arch_types));
    }

  }
}

TEST_CASE("Component Data")
{
  EntityManager em;
  auto entity = em.AddEntity();

  REQUIRE(entity.id != 0);

}


TEST_CASE("Archetype Allocation and Deallocation") {
  // Create an archetype with a specific type
  Types types_list = { GetTypeId<A>().id };
  Archetype archetype(types_list);

  SECTION("Allocation") {
    size_t allocated_row = archetype.Allocate();
    REQUIRE(allocated_row != -1);
  }

  SECTION("De-allocation") {
    size_t allocated_row = archetype.Allocate();
    archetype.Deallocate(allocated_row);
    REQUIRE(archetype.rows[allocated_row].next != -1);
  }

  SECTION("Free Linked List Correctness After Multiple Allocations and Deallocations") {
    std::vector<size_t> allocated_rows;

    // Allocate multiple rows
    for (int i = 0; i < 5; ++i) {
      size_t row = archetype.Allocate();
      REQUIRE(row != -1);
      allocated_rows.push_back(row);
    }

    // Deallocate in reverse order
    for (int i = 4; i >= 0; --i) {
      archetype.Deallocate(allocated_rows[i]);
    }

    // Allocate again and ensure the free list is correct
    for (int i = 0; i < 5; ++i) {
      size_t row = archetype.Allocate();
      REQUIRE(row == allocated_rows[i]);
    }
  }
}

