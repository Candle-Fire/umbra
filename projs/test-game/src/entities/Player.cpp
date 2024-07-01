#include "entities/Player.h"
#include "entities/TestCamera.h"
#include "entities/Health.h"

SHObject_Base_Impl(Player)

void Player::Build() {
    //Add position, health, and camera components
    Add<TestCamera>({});
    Add<Health>({});

}
