/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/
#pragma once
#include <string>

#include "Identifiers.h"

/**
 * Mark this type as a valid target for a Reference.
 */
#define SONIC_REFLECTION_CLASS(Type)                                                            \
    friend sonic::reflect::Meta;                                                                \
    template<typename T> friend class sonic::reflect::Meta_t;                                   \
    public:                                                                                     \
    inline static sonic::reflect::Meta const* Meta = nullptr;                                   \
    static sonic::reflect::Reference GetStaticReference() { return Type::Meta->Ref; }           \
    virtual sonic::reflect::Meta const* GetMeta() const override { return Type::Meta; }         \
    virtual sonic::reflect::Reference GetReference() const override { return Type::Meta->Ref; }

/**
 * Mark the following field / property to the reflection system
 */
#define SONIC_REFLECTION_FIELD( ... )

namespace sonic {
    /**
     * Utilities relating to identifying uninitialized classes and objects by name.
     * Allows for referencing things like Systems without actually needing to create an object or a reference for one.
     */
    namespace reflect {
        class Meta;
        class Reference;
        class Reflect;
        class Property;

        /**
         * A reference to a C++ class.
         * Note: to a CLASS, not to an INSTANCE.
         */
        class Reference {
        public:
            Reference() {}
            Reference(std::string const& str) : ID(str) {}
            Reference(char const* str) : ID(str) {}
            Reference(StringID ID) : ID(ID) {}
            Reference(size_t ID) : ID(ID) {}

            inline bool IsValid() const { return ID.IsValid(); }

            inline explicit operator size_t() const { return ID.ToUInt(); }
            inline size_t ToUInt() const { return ID.ToUInt(); }
            inline StringID ToStringID() const { return ID; }
            inline char const* c_str() const { return ID.c_str(); }

            inline bool operator==(Reference const& r) const { return ID == r.ID; }
            inline bool operator!=(Reference const& r) const { return ID != r.ID; }

            inline bool operator==(StringID const& r) const { return ID == r; }
            inline bool operator!=(StringID const& r) const { return ID != r; }

        private:
            StringID ID;
        };

        /**
         * Describes one field of a C++ class.
         * Most classes have data that can't easily be serialized using the primitives that C++ provides.
         * An array of Propertys allows a class to be fully serialized.
         */
        class Property {
        public:
            enum Flags : uint32_t {
                Array = 0,
                Vector,
                Enum,
                Bitfield,
                Instance,
                Struct,
                Resource
            };

            Property() = default;

            inline bool IsValid() const { return ID.IsValid(); }

            inline bool IsStruct() const { return Flags & Flags::Instance; }
            inline bool IsArray() const { return Flags & Flags::Array || Flags & Flags::Vector; }
            inline bool IsStaticArray() const { return Flags & Flags::Array && !(Flags & Flags::Vector); }
            inline bool IsDynamicArray() const { return Flags & Flags::Vector; }
            inline bool IsBitfield() const { return Flags & Flags::Bitfield; }
            inline bool IsInstance() const { return Flags & Flags::Instance; }
            inline bool IsResource() const { return Flags & Flags::Resource; }
            inline bool IsEnum() const { return Flags & Flags::Enum; }

            template<typename T>
            inline T* GetAddress(void* addr) const {
                return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(addr) + ParentOffset);
            }

            template<typename T>
            inline T const* GetAddress(void const* addr) const {
                return reinterpret_cast<T const*>(reinterpret_cast<uint8_t const*>(addr) + ParentOffset);
            }

            inline void* GetAddress(void* addr) const {
                return reinterpret_cast<uint8_t*>(addr) + ParentOffset;
            }

            inline void const* GetAddress(void const* addr) const {
                return reinterpret_cast<uint8_t const*>(addr) + ParentOffset;
            }

            template<typename T>
            inline T const& GetDefaultValue() const {
                T const& def = *reinterpret_cast<T const*>(Default);
                return def;
            }

            inline void const* GetArrayItemDefault(int32_t idx) const {
                uint8_t const* arrayPtr = (uint8_t const*) ArrayDefault;
                return arrayPtr + (ArrayItemSize * idx);
            }

            StringID        ID;                         // The name of this property
            Reference       Ref;                        // The type of the property
            Reference       ParentRef;                  // The type of the parent class that this is a property of.
            Reference       TemplateRef;                // The type of the template argument that the property holds
            int32_t         Size;                       // The size in bytes of the full property / field / array in memory.
            int32_t         ParentOffset;               // The offset from the start of the parent type to the start of this property's data
            int32_t         ArraySize;                  // The number of elements in the array, if present. If dynamic, the default size.
            int32_t         ArrayItemSize;              // The size in bytes of a single element in the array.
            void const*     Default = nullptr;          // A pointer to the default value of the property (return from an empty initializer list)
            void const*     ArrayDefault = nullptr;     // A pointer to the contained data within the default array
            size_t          Flags;                      // Information about the property's actual type
        };

        /**
         * Ancillary information about Properties that are Enum typed.
         */
        class EnumProperty {
        public:
            struct Position {
                StringID ID;
                int32_t Idx = 0;
                size_t Value = 0;
            };

            inline size_t GetNumPositions() const { return Positions.size(); }

            inline bool IsValidValue(StringID label) const {
                for (auto const& pos : Positions)
                    if (pos.ID == label)
                        return true;
                return false;
            }

            inline size_t GetValue(StringID label) const {
                for (auto const& pos : Positions)
                    if (pos.ID == label)
                        return pos.Value;
                return Positions.begin()->Value;
            }

            inline bool TryGetValue(StringID label, size_t& out) const {
                for (auto const& pos : Positions) {
                    if (pos.ID == label) {
                        out = pos.Value;
                        return true;
                    }
                }
                return false;
            }

            inline StringID GetFirstLabelForValue(size_t val) const {
                for (auto const& pos : Positions)
                    if (pos.Value == val)
                        return pos.ID;

                return StringID();
            }

            inline bool TryGetFirstLabelForValue(size_t val, StringID& out) const {
                for (auto const& pos : Positions) {
                    if (pos.Value == val) {
                        out = pos.ID;
                        return true;
                    }
                }
                return false;
            }

            Reference ID;
            std::vector<Position> Positions;
        };

        /**
         * Addresses the location and index of a Property from absolute location.
         * Properties are elements of classes, and naming them can be annoying.
         * The Address contains the class that the property is a member of, its' name, and the index if there are multiple of the same name.
         */
        class PropertyAddress {
        public:

            struct AddressElement {
                AddressElement() : Idx(-1) {}
                AddressElement(StringID id) : ID(id), Idx(-1) {}
                AddressElement(StringID id, int32_t idx) : ID(id), Idx(idx) {}

                inline bool IsArrayElement() const { return Idx != -1; }

                inline bool operator==(AddressElement const& rhs) const { return  ID == rhs.ID && Idx == rhs.Idx; }

                inline bool operator!=(AddressElement const& rhs) const { return !(*this == rhs); }

                StringID  ID;
                int32_t   Idx;
            };

            PropertyAddress() {}
            PropertyAddress(std::string const& Str);

            inline bool IsValid() const { return !AddressElements.empty(); }

            inline size_t GetHash() const { return Hash; }

            void Clear() { AddressElements.clear(); Hash = 0; }

            inline size_t GetNumElements() const { return AddressElements.size(); }

            inline AddressElement const& First() const { return AddressElements.front(); }

            inline AddressElement const& Last() const { return AddressElements.back(); }

            inline bool IsPathToArray() const { return Last().IsArrayElement(); }

            inline void Append(StringID newElement, int32_t idx = -1) {
                AddressElements.emplace_back(AddressElement(newElement, idx));
                GenerateHash();
            }

            inline void RemoveLast() { AddressElements.pop_back(); GenerateHash(); }

            inline void ReplaceLast(StringID newElement, int32_t idx = -1) {
                AddressElements.back() = AddressElement(newElement, idx);
                GenerateHash();
            }

            inline PropertyAddress GetWithoutClass() const {
                PropertyAddress sub;
                sub.AddressElements = std::vector(AddressElements.begin() + 1, AddressElements.end());
                sub.GenerateHash();
                return sub;
            }

            inline AddressElement const& operator[](size_t idx) const {
                return AddressElements[idx];
            }

            inline bool operator==(PropertyAddress const& rhs) const {
                return Hash == rhs.Hash;
            }

            inline bool operator!=(PropertyAddress const& rhs) const { return Hash != rhs.Hash; }

            inline PropertyAddress& operator+=(StringID newElement) {
                AddressElements.emplace_back(newElement);
                GenerateHash();
                return *this;
            }

            std::string ToString() const;

        private:

            void GenerateHash();

            std::vector<AddressElement>     AddressElements;
            size_t                          Hash = 0;

        };

        /**
         * Contains information about a given Reference's type data - the name of the class, its position in the inheritance tree, etc.
         * Meta must be filed for every class that is Reflected.
         */
        class Meta {

        public:

            Meta() = default;
            Meta(Meta const&) = default;
            virtual ~Meta() = default;

            Meta& operator=(Meta const&) = default;

            inline Reflect const* GetDefaultInstance() const { return Default; }

            inline char const* GetTypeName() const { return Ref.ToStringID().c_str(); }
            bool IsAbstract() const { return Abstract; }
            bool IsDerivedFrom(Reference parent) const;

            template<typename T>
            inline bool IsDerivedFrom() const { return IsDerivedFrom(T::GetStaticReference()); }

            Property* GetProperty(StringID prop);
            Property const* GetProperty(StringID prop) const { return const_cast<Meta*>(this)->GetProperty(prop); }

            template<typename T>
            void RegisterProperty(Reflect const* defaultInstance) { abort(); }

            virtual void CopyProperties(Reflect* from, Reflect const* to) const = 0;

            virtual Reflect* CreateInstance() const = 0;
            virtual void CreateInstanceInPlace(Reflect* dest) = 0;
            virtual void RecreateInstance(Reflect* dest) = 0;

            Reference                       Ref;                        // The reference to the class type being described
            Reflect const*                  Default;                    // The default instance of the given type: what is returned by the empty initializer list
            Meta const*                     ParentMeta = nullptr;       // The Meta of the direct parent of this class, if applicable.
            std::vector<Property>           Properties;                 // All properties registered for this type.
            std::map<StringID, int32_t>     PropertyMap;                // The indices of the properties with the given string-referenced names in the above list.
            int32_t                         Size = -1;                  // The size in bytes of a fully initialized class of this type.
            int32_t                         Alignment = -1;             // The minimum alignment (low-bit-mask) of a class of this type.
            bool                            Abstract = false;           // Whether the class being described has any pure virtual functions.
        };

        /**
         * A simple typed holder for the metadata of a given class.
         */
        template<typename T>
        class Meta_t : public Meta {};

        /**
         * Base type for all classes onto which we need to reflect.
         */
        class Reflect {

        public:

            Reflect() = default;
            Reflect(Reflect const&) = default;
            virtual ~Reflect() = default;

            Reflect& operator=(Reflect const&) = default;

            virtual Reference GetReference() const = 0;
            virtual Meta const* GetMeta() const = 0;

            virtual void Deserialized() {}
        };

        template<typename T>
        bool IsDerivedFrom(Reflect const* type) {
            if (type == nullptr) return false;
            return type->GetMeta()->IsDerivedFrom(T::GetStaticReference());
        }

        template<typename T>
        T* Cast(Reflect* type) {
            return reinterpret_cast<T*>(type);
        }

        template<typename T>
        T const* Cast(Reflect const* type) {
            return reinterpret_cast<T const*>(type);
        }

        template<typename T>
        T* TryCast(Reflect* type) {
            if (type != nullptr && type->GetMeta()->IsDerivedFrom(T::GetStaticReference()))
                return reinterpret_cast<T*>(type);
            return nullptr;
        }

        template<typename T>
        T const* TryCast(Reflect const* type) {
            if (type != nullptr && type->GetMeta()->IsDerivedFrom(T::GetStaticReference()))
                return reinterpret_cast<T const*>(type);
            return nullptr;
        }
    }
}
