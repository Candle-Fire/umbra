#include <shadow/reflection.h>
using namespace SH::Reflection;

#include </data/programming/_Projects/umbra/umbra/projs/test-game/inc/entities/Player.h>
template<> Class const *SH::Reflection::GetClass(ClassTag<Player>){
    static detail::ClassStorage<Player, 0, 0, 0> reflected([](auto self) {
    });
    static Class const clazz("Player", sizeof(User),reflected.fields, reflected.numFields);
    return &clazz;
}
const Class *Player::GetClass() const { return SH::Reflection::GetClass(ClassTag<Player>{}); }

std::vector<Class const *> get_assembly_classes() {
    return {
        GetClass(ClassTag<Player>()),
    };
}
