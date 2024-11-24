//
// Created by dpeter99 on 2023.09.10..
//
#include "ecs.exp.h"

struct A : Entity
{
  char sentinel[9] = "Entity A";
};

struct Position : Component
{
  int x, y, z = 0;
};
struct C : Component {};


int main() {

  EntityManager em;

  auto a = em.AddEntity();

  auto b = em.AddEntity();
  b.AddComponent<Position>({.x = 10, .y = 20, .z = 30});




  PrintEM(em);

  return 0;
}
