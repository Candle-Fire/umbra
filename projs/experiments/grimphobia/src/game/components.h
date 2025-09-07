#pragma once
#include "modules/ecs/ecs.h"

class Sprite
{
  int id = 0;
public:
  Sprite(int id): id(id) {  }
};



class Position: public Component
{
  SHObject_Base(Position)
public:
  float x = 0,y = 0,z = 0;
};

class RenderSprite: public Component
{
  SHObject_Base(RenderSprite)
public:
  explicit RenderSprite(const Sprite& sprite): sprite(sprite) {}

private:
  Sprite sprite;
};

class PlayerMovement : public  Component
{
  SHObject_Base(PlayerMovement)
public:
  float speed = 1;
};