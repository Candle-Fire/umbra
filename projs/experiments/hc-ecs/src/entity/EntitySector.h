
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/
#pragma once
#include <mutex>
#include "Identifiers.h"
#include "engine/EngineImpl.h"
#include "mathematics/Maths.h"

namespace sonic {
    class Entity;
    /**
     * A group of entities, loaded and managed as one.
     */
    class EntitySector {
        enum class Status {
            LoadFailed,
            Unloaded,
            Loading,
            Loaded,
            Unloading
        };

        struct RemovalRequest {
            RemovalRequest(Entity* e, bool shouldDestroy) : Entity(e), ShouldDestroy(shouldDestroy) {}
            Entity* Entity = nullptr;
            bool ShouldDestroy = false;
        };

    public:

        EntitySector();
        EntitySector(EntitySector const& sector);
        EntitySector(EntitySector&& sector);;
        ~EntitySector();

        EntitySector& operator=(EntitySector const& sector);
        EntitySector& operator=(EntitySector&& sector);

        inline EntitySectorID GetID() const { return ID; }
        inline bool IsRuntimeSector() const { return RuntimeSector; }

        void Load(LoadContext const& load, InitializationContext& init);
        void Unload(LoadContext const& load, InitializationContext& init);

        bool UpdateState(LoadContext const& load, InitializationContext& init);
        bool HasPendingRequests() const;

        inline bool IsLoading() const { return Status == Status::Loading; }
        inline bool IsLoaded() const { return Status == Status::Loaded; }
        inline bool IsUnloading() const { return Status == Status::Unloading; }
        inline bool HasLoadingFailed() const { return Status == Status::LoadFailed; }

        inline size_t GetNumEntities() const { return Entities.size(); }
        inline std::vector<Entity*> const& GetEntities() const { return Entities; }
        inline Entity* FindEntity(EntityID id) const {
            auto iter = std::ranges::find_if(EntityLookup, [&id] (std::pair<EntityID, Entity*> const& e) {return e.first == id;});
            return (iter != EntityLookup.end()) ? iter->second : nullptr;
        }

        inline bool ContainsEntity(EntityID id) const {
            return FindEntity(id) != nullptr;
        }

        void AddEntities(std::vector<Entity*> const& entities, maths::Transformation const& offset = maths::Transformation::Identity);
        void AddEntity(Entity* e);

        Entity* RemoveEntity(EntityID e);
        void DestroyEntity(EntityID e);


    private:

        void OnEntityStateUpdated(Entity* e);
        void ProcessSectorLoading(LoadContext const& load);
        void ProcessSectorUnloading(LoadContext const& load, InitializationContext& init);
        void ProcessEntityRegistration(InitializationContext& init);
        void ProcessEntityShutdown(InitializationContext& init);
        void ProcessEntityRemoval(LoadContext const& load);
        void ProcessEntityLoad(LoadContext const& load, InitializationContext& init);

        Entity* RemoveEntityInternal(EntityID id, bool destroy);

        EntitySectorID  ID = EntitySectorID::Generate();

        std::recursive_mutex Mutex;

        std::vector<Entity*> Entities;
        std::map<EntityID, Entity*> EntityLookup;
        std::vector<Entity*> EntitiesLoading;
        std::vector<Entity*> EntitiesToLoad;
        std::vector<RemovalRequest> EntitiesToRemove;

        Status Status = Status::Unloaded;
        bool RuntimeSector = false;
    };
}
