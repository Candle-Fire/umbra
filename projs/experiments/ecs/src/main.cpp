//
// Created by dpeter99 on 2023.09.10..
//
#include "ecs.exp.h"

class Position : public Component {
public:
  float x;
  float y;
  float z;
};

class Mesh : public Component {

};

class HP : public Component {
public:
  int Hp;
};

class Player : public Entity {
public:
  Player(Id UUID) : Entity(UUID) {
      auto *p = this->AddInternalChild<Position>();
      p->x = 10;
      this->AddInternalChild<Mesh>();
      this->AddInternalChild<HP>();
  }

};

static_assert(Scene::isEntity == true);

int main() {
    EntityManager em;

    Scene *scene = em.Add<Scene>();
    Player *player = em.AddChild<Player>(*scene);

    em.DumpData();

    return 0;
}