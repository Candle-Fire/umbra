#include "player.h"

void Player::Init()
{
  AddComponent(new Position());
  AddComponent(new RenderSprite(Sprite(1)));
  AddComponent(new PlayerMovement(0.1));

}
