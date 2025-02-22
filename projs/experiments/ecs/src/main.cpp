//
// Created by dpeter99 on 2023.09.10..
//
#include "ecs.exp.h"

struct A : Entity
{
  char sentinel[9] = "Entity A";
};

struct Position final : Component<>
{
  int x, y, z = 0;
  Position(const int& x, const int& y, const int& z) : x(x), y(y), z(z) {}

  void Print() const override
  {
    printf("Position (%d, %d, %d)\n", x, y, z);
  }
};
struct Name final : Component<>
{
  char sentinel[5] = "Name";

  void Print() const override
  {
    printf("Name: %s\n", sentinel);
  }
};

int main() {
  EntityManager em;
  SystemManager sm(em);

  const auto a = em.AddEntity();

  for (int i = 0; i < 10; ++i)
  {
    auto b = em.AddEntity();
    b.AddComponent<Position>({ i, 22, 32})
      .AddComponent<Name>({})
      .AddRelation<ChildOf>(a);
  }

  auto s = System<Position>([](Position& position)
  {
    position.z += 10;
  });

  sm.addSystem(&s);

  sm.runAllSystems();

  PrintEM(em);

  return 0;
}
