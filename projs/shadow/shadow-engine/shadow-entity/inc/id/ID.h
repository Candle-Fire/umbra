#pragma once

#include <id/UUID.h> // Shadow-Engine/core/inc/id

namespace SE {

    // An ID for a section of the scene (which may be unloaded separately of the level; for ie. open world Cells.)
    using EntitySectionID = UUID;
    // An ID for a scene (which contains all sections and entities).
    using EntitySceneID = UUID;

    /**
     * Contains the common data and functions for the Entity System's identifiers.
     */
    struct IDContainer {
    public:
        IDContainer() = default;
        explicit IDContainer(uint64_t v) : id(v) {}

        // Check if the ID is valid (non-zero)
        __inline virtual bool Valid() const { return id != 0; }
        // Set this ID to be invalid (zero).
        __inline virtual void Invalidate() { id = 0; }
        // Check for in/equality against another ID.
        __inline virtual bool operator==(IDContainer const& other) const { return id == other.id; }
        __inline virtual bool operator!=(IDContainer const& other) const { return id != other.id; }
        __inline virtual bool operator<(IDContainer const& other) const { return id < other.id; }

        uint64_t id;
    };


    /**
     * An ID used for an Entity.
     * Is only guaranteed to be unique in the current scene (ie. a level change may also change the IDs of the entities within).
     */
    struct EntityID : public IDContainer {
        /**
         * @return a new, unused Entity ID.
         */
        static EntityID Generate();

        EntityID() = default;
        explicit EntityID(uint64_t v) : IDContainer(v) {}
    };

    /**
     * An ID used for a Component (a segment of data attached to an Entity).
     * Is only guaranteed to be unique in the current scene (ie. a level change may also change the IDs of the components within).
     */
    struct ComponentID : public IDContainer {
        /**
         * @return a new, unused Component ID.
         */
        static ComponentID Generate();

        ComponentID() = default;
        explicit ComponentID(uint64_t v) : IDContainer(v) {}
    };
}

