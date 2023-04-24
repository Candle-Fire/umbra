#include "reflection.h"

namespace SH::Reflection {

    template<>
    Type const *GetType<std::uint64_t>() {
        static Type type{"std::uint64_t", sizeof(std::uint64_t)};
        return &type;
    }

    template<>
    Type const *GetType<int>() {
        static Type type{"int", sizeof(int)};
        return &type;
    }

    template<>
    Type const *GetType<float>() {
        static Type type{"float", sizeof(float)};
        return &type;
    }

    template<>
    Type const *GetType<std::string>() {
        static Type type{"std::string", sizeof(std::string)};
        return &type;
    }

    template<class T>
    Type const *GetType() {
        return GetTypeImpl(TypeTag<T>{});
    }

    template<class T>
    Type const *GetTypeImpl(TypeTag<std::vector<T>>) {
        static Type type{typeid(T).name(), sizeof(T)};
        return &type;
    }
}

template<>
SH::Reflection::Class const *SH::Reflection::GetClass(SH::Reflection::ClassTag<User>) {

    static SH::Reflection::detail::ClassStorage<User, 3, 0, 0> reflected([](auto self) {
        self->fields[0].type = SH::Reflection::GetType<std::uint64_t>();
        self->fields[0].name = "id";
        self->fields[0].offset = offsetof(User, id);
        self->fields[1].type = SH::Reflection::GetType<std::string>();
        self->fields[1].name = "name";
        self->fields[1].offset = offsetof(User, name);
        self->fields[2].type = SH::Reflection::GetType<std::vector<std::string>>();
        self->fields[2].name = "pets";
        self->fields[2].offset = offsetof(User, pets);
    });
    static SH::Reflection::Class const clazz("User", sizeof(User), reflected.fields, reflected.numFields);

    return &clazz;
}

#if 0

int main() {
    auto clazz = GetClass(ClassTag<User>{});

    for (auto &field : clazz->fields) {
        printf("Name: %s\n", field.name);
        printf("Type: %s\n", field.type->name);
        printf("\n");
    }
}

#endif