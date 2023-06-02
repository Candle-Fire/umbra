#include "entities/Player.h"
#include "entities/TestCamera.h"

#include "entities/Position.h"
#include "entities/Health.h"

SHObject_Base_Impl(test_game::Player)

void test_game::Player::Build() {
    //Add position, health, and camera components
    Add<TestCamera>({});
    Add<Health>({});
    Add<ShadowEngine::Entities::Builtin::Position>({});

}
