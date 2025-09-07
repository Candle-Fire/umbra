#include "player.h"

void Player::Init()
{
  AddComponent(new Position());
  AddComponent(new RenderSprite(Sprite(1)));
}