#include "entities/Player.h"


SHObject_Base_Impl(Player)

void Player::Build() {
    this->AddInternalChildEntity<TestCamera>();
}
