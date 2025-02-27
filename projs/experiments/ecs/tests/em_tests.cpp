#include "./ecs.exp.h"
#include "catch2/catch_all.hpp"

struct A : Component<>
{
  int a;
  float b;
  void Print() const override {};
};
struct B : Component<>
{
  void Print() const override {}
};
struct C : Component<>
{
  void Print() const override {}
};
struct Position : Component<>
{
  float x, y, z;
  void Print() const override { printf("Pos: \n"); }
};

TEST_CASE("EM archetype storage") {
  SECTION("Gives the correct Archetype") {
    EntityManager em;
    auto& a = em.GetArchetype({
        GetNodeType<A>(),
        GetNodeType<C>(),
        GetNodeType<Position>(),
    });

    REQUIRE(a.id > 0);
  }

  SECTION("Gives the same Archetype") {
    EntityManager em;
    auto& a = em.GetArchetype({
        GetNodeType<A>(),
        GetNodeType<C>(),
        GetNodeType<Position>(),
    });

    auto& b = em.GetArchetype({
        GetNodeType<A>(),
        GetNodeType<C>(),
        GetNodeType<Position>(),
    });

    REQUIRE(a.id == b.id);
    REQUIRE(&a == &b);
  }

  SECTION("Archetype map matches internal Type list")
  {
    EntityManager em;
    auto& a = em.GetArchetype({
        GetNodeType<A>(),
        GetNodeType<C>(),
        GetNodeType<Position>(),
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




