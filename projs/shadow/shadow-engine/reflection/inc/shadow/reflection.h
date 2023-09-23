#pragma once
//############################
//####  Reflection system ####
//############################


#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <typeinfo>
#include <span>

#include "shadow/util/hash.h"

#pragma GCC diagnostic ignored_attributes "SH::Reflect"

class SHObject;

namespace SH::Reflection {

  struct Type {
    char const *name;
    std::size_t size;
    SH::StableHash hash;

    //Type() noexcept
    //    : size(0), name() {}

    Type(char const *name, int size
    ) noexcept
        : size(size), name(name), hash(name) {}

    /* --------------------------------------------------------------------- */
    /* Operator                                                              */
    /* --------------------------------------------------------------------- */
    bool
    operator==(Type const &other) const noexcept {
        return hash == other.hash;
    }

    bool
    operator!=(Type const &other) const noexcept {
        return !(*this == other);
    }

  };

  struct Field {
    Type const *type;
    char const *name;
    std::ptrdiff_t offset;

    template<class T>
    T const *
    GetPointer(void const *instance) const noexcept {
        /* assert(*TypeResolver<T>::Get() == *m_type && "type mismatch"); */
        return static_cast<T const *>((char *) instance + offset);
    }

    template<class T>
    T *
    GetPointer(void *instance) const noexcept {
        /* assert(*TypeResolver<T>::Get() == *m_type && "type mismatch"); */
        return (T *) ((char *) instance + offset);
    }
  };

  struct Function {
    Field *returnValue;
    //Field *parameters[N];
    char const *name;
  };

  struct Class : Type {

    Class(
        const char *name,
        int size,
        Field *fields,
        size_t fields_count)
        : Type(name, size), fields(fields, fields_count) {}

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

  //template<class T>
  //Class const *GetClass(ClassTag<T>) {
  //    return nullptr;
  //};

  template<class T>
  Type const *GetType();
}

#define SHReflect(type)    \
public: \
    const SH::Reflection::Class *GetClass() const override; \
private:

class User {
public:
  std::uint64_t id;
  std::string name;
  std::vector<std::string> pets;
};