
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#pragma once
#include "plugin/Module.h"
#include <map>

#include "Type.h"

namespace sonic {
    namespace reflect {
        /**
         * Relates every Meta with its' associated Reference.
         * Handled as a Plugin Module, to ensure singleton nature.
         */
        class TypeRegistry : public Module {
        public:

            TypeRegistry() = default;
            ~TypeRegistry();

            void RegisterInternal();
            void UnregistereInternal();

            Meta const* RegisterType(Meta const* type);
            void UnregisterType(Meta const* type);

            Meta const* GetTypeMeta(Reference ID) const;

            template<typename T, typename = std::enable_if_t<std::is_base_of<Reflect, T>::value>>
            Meta const* GetTypeMeta() const {
                return T::Meta;
            }

            Property const* ResolvePropertyAddress(Meta const* meta, PropertyAddress const& ID) const;

            inline bool IsRegistered(Reference type) const { return RegisteredTypes.find(type) != RegisteredTypes.end();}

            bool IsDerivedFrom(Reference type, Reference parent) const;

            std::vector<Meta const*> GetAll(bool abstract = true, bool sort = false) const;

            std::vector<Meta const*> GetAllDerived(Reference parent, bool includeParent = false, bool abstract = true, bool sort = false) const;

            std::vector<Reference> GetCastable(Reflect const* type) const;

            bool HasCommonInheritance(Reference a, Reference b) const;

            bool HasCommonInheritance(Meta const* a, Meta const* b) const;

            EnumProperty const* RegisterEnum(EnumProperty const& type);
            void UnregisterEnum(Reference type);

            EnumProperty const* GetEnumProperty(Reference type) const;

            template<typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
            EnumProperty const* GetEnumProperty() const {
                char const* name = typeid(T).name();
                Reference const ref(name + 5); // Skip scrambling
                return GetEnumProperty(ref);
            }

        private:
            std::map<Reference, Meta const*>        RegisteredTypes;
            std::map<Reference, EnumProperty*>      RegisteredEnums;
        };
    }
}
