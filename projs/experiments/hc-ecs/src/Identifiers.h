#include <UUID.h>
#include <catch2/catch_all.hpp>
#include <cstddef>

/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/


/**
 * A unique 64-bit identifier, for things where UUIDs are too large.
 * This is a macro to ensure that Entity IDs cannot be used as Scene IDs and vice versa.
 *  The type system is used to ensure that IDs point to the correct objects.
 */
#define UNIQUE_IDENTIFIER(obj)                                              \
    struct obj##ID {                                                        \
        static obj##ID Generate();                                          \
    public:                                                                 \
        obj##ID() = default;                                                \
        explicit obj##ID(size_t v) : Data(v) {}                             \
        bool IsValid() const { return Data != 0; }                          \
        void Clear() { Data = 0; }                                          \
        bool operator==(obj##ID const& r) const { return Data == r.Data; }  \
        bool operator!=(obj##ID const& r) const { return Data != r.Data; }  \
                                                                            \
        size_t Data = 0;                                                    \
    }


namespace sonic {

    /**
     *  Entity Sectors are batches of entities that are loaded and managed as a group.
     *  They allow hot-reloading of entity positions & component data by the editor, as the Sector can control when the entity data is exposed to the rest of the ECS.
     */
    using EntitySectorID = UUID;

    /**
     *  Entity Scenes are specifically collections of all the entities in a single scene, for every scene currently loaded.
     *  They can be manipulated, loaded, and used in parallel.
     *  See EntityScene for implementation.
     */
    UNIQUE_IDENTIFIER(Scene);

    /**
     *  Entities are objects that can be interacted with.
     */
    UNIQUE_IDENTIFIER(Entity);

    /**
     *  Components are data storage for an Entity.
     */
    UNIQUE_IDENTIFIER(Component);

    /**
     * Strings can be interacted with by a System, so for those controlled by the ECS we have a similar unique ID.
     */
    struct StringID {

        static void Initialize();                           // Ensure the internal String ID state-tracking system is ready for use.
        static void Shutdown();                             // Return all the memory that the String ID state-tracking system is currently using

        StringID() = default;
        explicit StringID(std::nullptr_t) : Data(0) {}
        explicit StringID(size_t v) : Data(v) {}
        explicit StringID(char const* str);
        explicit StringID(std::string const& str);
        explicit StringID(std::stringstream const& str);

        char const* c_str() const;                          // Fetch the String that this ID tracks.

        inline void Clear() { Data = 0; }                   // Clear the String ID - does NOT clear the String data. This is an orphaning operation.
        inline bool IsValid() const { return Data != 0; }
        inline size_t ToUInt() const { return Data; }       // Fetch the size_t representation of the String ID.
        inline operator size_t() const { return Data; }     // Fetch the size_t representation of the String ID.

        inline bool operator==(StringID const& r) const { return Data == r.Data; }
        inline bool operator!=(StringID const& r) const { return Data != r.Data; }

        size_t Data = 0;
    };

    /**
     * Strings that are hard-coded into the Game or Engine files that are not meant to be manipulated by Systems can still be stored linearly, but need to be treated differently.
     * A static String is one that is global to a translation unit - source-code level Static.
     */
    struct StaticStringID {
        StaticStringID(char const* str);

        inline StringID const& GetID() const {
            if (!Ready) {
                ID = StringID(Buffer);
                Ready = true;
            }
            return ID;
        }

        inline operator StringID() const { return GetID(); }
        inline bool operator==(StringID const& r) const { return GetID() == r; }
        inline char const* c_str() const { return GetID().c_str(); }

    private:
        StaticStringID() = delete;

        char Buffer[64] = {0};
        mutable StringID ID;
        mutable bool Ready = false;

    };

}


template<>
struct std::hash<sonic::SceneID> {
    std::size_t operator()(const sonic::SceneID &s) const noexcept { return s.Data; }
};

template<>
struct std::hash<sonic::EntityID> {
    std::size_t operator()(const sonic::EntityID &s) const noexcept { return s.Data; }
};

template<>
struct std::hash<sonic::ComponentID> {
    std::size_t operator()(const sonic::ComponentID &s) const noexcept { return s.Data; }
};

template<>
struct std::hash<sonic::StringID> {
    std::size_t operator()(const sonic::StringID &s) const noexcept { return s.ToUInt(); }
};

namespace Catch {
    template < >
    struct StringMaker<sonic::SceneID> {
        static std::string convert(sonic::SceneID const &p) {
            return std::to_string(p.Data);
        }
    };

    template < >
    struct StringMaker<sonic::EntityID> {
        static std::string convert(sonic::EntityID const &p) {
            return std::to_string(p.Data);
        }
    };

    template < >
    struct StringMaker<sonic::ComponentID> {
        static std::string convert(sonic::ComponentID const &p) {
            return std::to_string(p.Data);
        }
    };

    template < >
    struct StringMaker<sonic::StringID> {
        static std::string convert(sonic::StringID const &p) {
            return p.c_str();
        }
    };
}