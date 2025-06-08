#include <UUID.h>
#include <catch2/catch_all.hpp>

/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/


/**
 * A unique 64-bit identifier, for things where UUIDs are too large.
 * This is a macro to ensure that Entity IDs cannot be used as Scene IDs and vice versa.
 *  The type system is used to ensure that IDs point to the correct objects.
 */
#define UNIQUE_IDENTIFIER(obj)                                          \
    struct obj##ID {                                                        \
        static obj##ID Generate();                                          \
    public:                                                             \
        obj##ID() = default;                                                \
        explicit obj##ID(size_t v) : data(v) {}                             \
        bool IsValid() const { return data != 0; }                      \
        void Clear() { data = 0; }                                      \
        bool operator==(obj##ID const& r) const { return data == r.data; }  \
        bool operator!=(obj##ID const& r) const { return data != r.data; }  \
                                                                        \
        size_t data = 0;                                                \
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
}


template<>
struct std::hash<sonic::SceneID> {
    std::size_t operator()(const sonic::SceneID &s) const noexcept;
};

template<>
struct std::hash<sonic::EntityID> {
    std::size_t operator()(const sonic::EntityID &s) const noexcept;
};

template<>
struct std::hash<sonic::ComponentID> {
    std::size_t operator()(const sonic::ComponentID &s) const noexcept;
};

namespace Catch {
    template < >
    struct StringMaker<sonic::SceneID> {
        static std::string convert(sonic::SceneID const &p) {
            return std::to_string(p.data);
        }
    };

    template < >
    struct StringMaker<sonic::EntityID> {
        static std::string convert(sonic::EntityID const &p) {
            return std::to_string(p.data);
        }
    };

    template < >
    struct StringMaker<sonic::ComponentID> {
        static std::string convert(sonic::ComponentID const &p) {
            return std::to_string(p.data);
        }
    };
}