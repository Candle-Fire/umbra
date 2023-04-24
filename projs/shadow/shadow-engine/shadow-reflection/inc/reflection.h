//############################
//####  Reflection system ####
//############################

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <typeinfo>
#include <span>

struct Type {
    char const *name;
    std::size_t size;
};

struct Field {
    Type const *type;
    char const *name;
    std::size_t offset;
};

struct Function {
    Field *returnValue;
    //Field *parameters[N];
    char const *name;
};

struct Class : Type {

    Class(
        Field *fields,
        size_t fields_count
    ) :
        fields(fields, fields_count) {}

    std::span<Field> fields;

    //Function functions[N];
};

template<class T>
struct TypeTag {};
template<class T>
struct ClassTag {};

namespace detail {

    template<class Type, size_t NFields, size_t NFunctions, size_t NTemplateArgs>
    struct ClassStorage {
        template<class Lambda>
        ClassStorage(Lambda &&ctor) noexcept {
            ctor(this);
        }

        size_t const numFields = NFields;
        size_t const numFunctions = NFunctions;
        size_t const numTemplateArgs = NTemplateArgs;
        /* Arrays of size 0 are UB. */
        Field fields[NFields + 1];
        Function functions[NFunctions + 1];
        //TemplateArgument templateArgs[NTemplateArgs + 1];
    };

} /* namespace detail */











template<class T>
Class const *GetClass(ClassTag<T>);

template<class T>
Type const *GetType();

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

class User {
  public:
    std::uint64_t id;
    std::string name;
    std::vector<std::string> pets;
};

template<>
Class const *GetClass(ClassTag<User>) {

    static detail::ClassStorage<User, 3, 0, 0> reflected([](auto self) {
        self->fields[0].type = GetType<std::uint64_t>();
        self->fields[0].name = "id";
        self->fields[0].offset = offsetof(User, id);
        self->fields[1].type = GetType<std::string>();
        self->fields[1].name = "name";
        self->fields[1].offset = offsetof(User, name);
        self->fields[2].type = GetType<std::vector<std::string>>();
        self->fields[2].name = "pets";
        self->fields[2].offset = offsetof(User, pets);
    });
    static Class const clazz(reflected.fields, reflected.numFields);

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