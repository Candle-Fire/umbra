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

  auto a = em.AddEntity();

  auto b = em.AddEntity();
  b.AddComponent<Position>({ 10, 22, 32})
    .AddComponent<Name>({});

  b.GetComponent<Position>().x = 100;

  em.AddRelation<ChildOf>(b.id,a.id);

  PrintEM(em);

  return 0;
}
