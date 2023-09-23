#include <shadow/reflection.h>
using namespace SH::Reflection;

#include </data/programming/_Projects/umbra/umbra/projs/shadow/shadow-engine/entity/inc/shadow/entitiy/entities/Position.h>
template<> Class const *SH::Reflection::GetClass(ClassTag<SH::Entities::Builtin::Position>){
    static detail::ClassStorage<SH::Entities::Builtin::Position, 1, 0, 0> reflected([](auto self) {
        self->fields[0].type = GetType<float>();
        self->fields[0].name = "x";
        self->fields[0].offset = offsetof(SH::Entities::Builtin::Position, x);
    });
    static Class const clazz("SH::Entities::Builtin::Position", sizeof(User),reflected.fields, reflected.numFields);
    return &clazz;
}
const Class *SH::Entities::Builtin::Position::GetClass() const { return SH::Reflection::GetClass(ClassTag<SH::Entities::Builtin::Position>{}); }

std::vector<Class const *> get_assembly_classes() {
    return {
        GetClass(ClassTag<SH::Entities::Builtin::Position>()),
    };
}
