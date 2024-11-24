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




