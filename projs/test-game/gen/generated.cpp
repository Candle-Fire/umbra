#include <reflection.h>
using namespace SH::Reflection;

#include <entities/Player.h>
template<> Class const *SH::Reflection::GetClass(ClassTag<Player>){
    static detail::ClassStorage<User, 1, 0, 0> reflected([](auto self) {
        self->fields[0].type = GetType<int>();
        self->fields[0].name = "hp";
        self->fields[0].offset = offsetof(Player, hp);
    });
    static Class const clazz("Player", sizeof(User),reflected.fields, reflected.numFields);
    return &clazz;
}
const Class *Player::GetClass() const { return SH::Reflection::GetClass(ClassTag<Player>{}); }

#include <entities/TestCamera.h>
template<> Class const *SH::Reflection::GetClass(ClassTag<TestCamera>){
    static detail::ClassStorage<User, 2, 0, 0> reflected([](auto self) {
        self->fields[0].type = GetType<float>();
        self->fields[0].name = "width";
        self->fields[0].offset = offsetof(TestCamera, width);
        self->fields[1].type = GetType<float>();
        self->fields[1].name = "height";
        self->fields[1].offset = offsetof(TestCamera, height);
    });
    static Class const clazz("TestCamera", sizeof(User),reflected.fields, reflected.numFields);
    return &clazz;
}
const Class *TestCamera::GetClass() const { return SH::Reflection::GetClass(ClassTag<TestCamera>{}); }

#include <entities/Health.h>
template<> Class const *SH::Reflection::GetClass(ClassTag<Health>){
    static detail::ClassStorage<User, 1, 0, 0> reflected([](auto self) {
        self->fields[0].type = GetType<int>();
        self->fields[0].name = "health";
        self->fields[0].offset = offsetof(Health, health);
    });
    static Class const clazz("Health", sizeof(User),reflected.fields, reflected.numFields);
    return &clazz;
}
const Class *Health::GetClass() const { return SH::Reflection::GetClass(ClassTag<Health>{}); }

