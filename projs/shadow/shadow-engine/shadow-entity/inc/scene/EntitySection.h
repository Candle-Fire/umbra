#pragma once
#include <vector>
#include <map>

namespace SE {
    // Forward declaration, see inc/entity/Entity.h
    class Entity;

    /**
     * A section is a collection of Entities with linked runtime.
     * ie. a game level could have one giant linked Section.
     *     a cell of an open-world game could represent one Region,
     *     and rooms of a BSP level could represent one Region.
     */
    class EntitySection {

        /**
         * The possible statuses for this Section.
         * Entities are not valid until the Loaded status is reached.
         */
        enum class Status {
            Failed = -1,        // An error occurred during loading.
            Unloaded = 0,       // The Section is present, but none of the data is populated.
            Loading,            // The Section is being loaded with data.
            Loaded,             // The Section is present and contains valid data.
            Unloading           // The Section is present but preparing to unload.
        };

        /**
         * Request the deletion of an entity, after ie. destruction or death.
         */
         struct PendingRemoval {
             PendingRemoval(Entity* entity, bool destroy)
                : entity(entity), destroy(destroy) {}

             Entity* entity = nullptr;
             bool destroy = false;
         };

    public:

        EntitySection();
        EntitySection(EntitySection const& section);
        EntitySection(EntitySection&& section);
        ~EntitySection();

        EntitySection& operator=(EntitySection const& section);
        EntitySection& operator=(EntitySection&& section);

        // Get the ID of this Section
        inline EntitySectionID GetID() const { return id; }
        // Whether or not this Section was generated, without being loaded from disk.
        inline bool IsTransient() const { return isTransient; }

        // TODO: Contexts?
        void Load();
        void Unload();

        // Check that everything went properly, and update state accordingly.
        bool UpdateState();

        // Whether there are any pending entity removals or additions.
        inline bool HasPendingEntityChanges() const { return (entitiesPendingLoading.size() + entitiesPendingRemoval.size()) > 0; }

        bool IsLoading() const { return status == Status::Loading; }
        inline bool IsLoaded() const { return status == Status::Loaded; }
        inline bool IsUnloaded() const { return status == Status::Unloaded; }
        inline bool LoadingFailed() const { return status == Status::Failed; }

        // Get the total number of entities in the section
        inline size_t GetEntityCount() const { return entities.size(); }
        // Get all entities in the region.
        inline std::vector<Entity*> const& GetEntities() const { return entities; }
        // Get a specific entity by its' ID.
        inline Entity* GetEntity(const EntityID& eID) const {
            auto i = entityLookup.find(eID);
            return (i != entityLookup.end()) ? i->second : nullptr;
        }
        // Check whether this region contains the specified entity.
        inline bool ContainsEntity(const EntityID& eID) const { return GetEntity(eID) != nullptr; }

        // Add all of the specified entities to the region.
        // Transfers ownership if already present.
        // This operation takes time, but is guaranteed to be finished by the start of the next frame.
        void AddEntities(const std::vector<Entity*>& entities);

        // Add an entity to the region.
        // Transfers ownership if already present.
        void AddEntity(Entity* entity);

        // Queue an entity for removal from this region.
        // Transfers ownership to the caller.
        // This operation takes time, and no guarantee is given for completion.
        Entity* RemoveEntity(const EntityID& eID);

        // Queue an entity for unloading from this region.
        // This operation takes time, and no guarantee is given for completion.
        void DestroyEntity(const EntityID& eID);

    private:

        // A callback for when an entity's state changes (ie. when loading)
        void EntityStateUpdated(Entity* entity);

        // TODO: Process Loading, Unloading, Entity Shutdown, Entity Removal, Entity Loading.

    private:

        EntitySectionID id = UUID::Generate();
        Status status = Status::Unloaded;

        // Transient = created on-the-fly, rather than existing on disk.
        bool const isTransient = false;

        // The list of every entity in the Section, including those that are not yet loaded, or those that are due to be unloaded.
        std::vector<Entity*> entities;

        // A fast map for looking up entities by ID.
        std::map<EntityID, Entity*> entityLookup;

        // The list of entities that are currently being initialized to be put into the Section.
        std::vector<Entity*> entitiesLoading;

        // The list of entities that are waiting to be loaded.
        std::vector<Entity*> entitiesPendingLoading;

        // The list of entities that are waiting to be removed.
        std::vector<Entity*> entitiesPendingRemoval;

    };

}