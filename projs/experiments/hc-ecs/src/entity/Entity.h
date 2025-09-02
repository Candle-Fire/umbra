/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#pragma once
#include "reflect/Type.h"
#include "system/System.h"
#include "component/SpatialComponent.h"
#include <engine/EngineImpl.h>

namespace sonic {
    /**
     * A container for a set of Systems, and the Components they manage.
     * If the Entity is attached, the parent is responsible for setup/teardown.
     */
    class Entity : public reflect::Reflect {
        SONIC_REFLECTION_CLASS(Entity);

        struct Action {
            enum class Type {
                Unknown,
                CreateSystem,
                DestroySystem,
                AddComponent,
                DestroyComponent,
                Wait
            };

            void const* ptr = nullptr;
            ComponentID id;
            Type type;
        };

        enum class RegistryState {
            Unregistered,
            Registering,
            Registered,
            Unregistering
        };

    public:

        enum class Status {
            Unloaded,
            Loaded,
            Initialized
        };

        enum class SpatialAttachmentRule {
            SceneTransform,
            LocalTransform
        };

        Entity() = default;
        Entity(StringID name) : Name(name) {}
        ~Entity();

        inline EntityID const& GetID() const { return ID; }
        inline StringID GetName() const { return Name; }
        inline EntitySectorID GetSectorID() const { return SectorID; }

        // TODO
        void GetResources() const;

        inline bool IsSpatialEntity() const { return RootSpatialComponent != nullptr; }
        inline SpatialComponent* GetRootSpatialComponent() { return RootSpatialComponent; }
        inline SpatialComponent const* GetRootSpatialComponent() const { return RootSpatialComponent; }
        inline ComponentID const& GetRootSpatialComponentID() const { return RootSpatialComponent->GetID(); }
        inline maths::OBB const& GetRootSpatialComponentBounds() const { return RootSpatialComponent->GetSceneBoundingBox(); }

        maths::AABB GetCompleteBounds() const;

        inline maths::Transformation const& GetLocalTransformation() const { return RootSpatialComponent->GetLocalTransform(); }
        inline maths::Transformation const& GetSceneTransformation() const { return RootSpatialComponent->GetSceneTransform(); }

        inline void SetSceneTransformation(maths::Transformation const& trans) const { RootSpatialComponent->SetSceneTransform(trans); }

        inline bool HasParent() const { return ParentEntity != nullptr; }
        inline Entity* GetParent() const { return ParentEntity; }
        inline EntityID const& GetParentID() const { return ParentEntity->GetID(); }

        bool IsChildOf(Entity const* parent) const;

        void SetParent(Entity* parent, StringID socket = StringID(), SpatialAttachmentRule attachRule = SpatialAttachmentRule::SceneTransform);
        void ClearParent(SpatialAttachmentRule attachRule = SpatialAttachmentRule::SceneTransform);

        inline StringID const& GetAttachmentSocketID() const { return AttachedSocketID; }
        inline maths::Transformation GetAttachmentSocketTransformation(StringID id) const { return RootSpatialComponent->GetAttachmentSocketTransformation(id); }

        inline bool HasChildren() const { return !AttachedEntities.empty(); }
        std::vector<Entity*> GetChildren() const { return AttachedEntities; }

        inline bool IsInSector() const { return SectorID.IsValid(); }
        inline bool IsInitialized() const { return Status == Status::Initialized; }
        inline bool IsRegistered() const { return RegistryState == RegistryState::Registered; }
        inline bool IsLoading() const { return Status != Status::Unloaded; }
        inline bool IsLoaded() const { return Status == Status::Loaded; }
        inline bool IsUnloaded() const { return Status == Status::Unloaded; }
        inline bool HasPendingActions() const { return !DeferredActions.empty(); }

        inline size_t GetNumComponents() const { return Components.size(); }
        inline std::vector<Component*> const& GetComponents() const { return Components; }
        inline Component const* FindComponent(ComponentID const& id) const {
            auto iter = std::ranges::find_if(Components,
                                             [&id] (Component* c) {
                                                 return c->GetID() == id;
                                             }
            );

            return (iter != Components.end()) ? *iter : nullptr;
        }
        inline Component* FindComponent(ComponentID const& id) {
            return const_cast<Component*>(const_cast<Entity const*>(this)->FindComponent(id));
        }
        void CreateComponent(reflect::Meta const* meta, ComponentID const& parent = ComponentID());
        void AddComponent(Component* c, ComponentID const& parent = ComponentID());
        void DestroyComponent(ComponentID const& id);

        inline size_t GetNumSystems() const { return Systems.size(); }
        inline std::vector<System*> const& GetSystems() const { return Systems; }
        void UpdateSystems(SceneUpdateContext const& context);
        template<typename T>
        T* GetSystem() {
            for (auto sys : Systems)
                if (sys->GetMeta()->Ref == T::GetStaticReference())
                    return reinterpret_cast<T*>(sys);
            return nullptr;
        }
        void CreateSystem(reflect::Meta const* meta);
        template<typename T>
        inline void CreateSystem() {
            CreateSystem(T::Meta);
        }
        void DestroySystem(reflect::Reference sysReference);
        void DestroySystem(reflect::Meta const* meta);
        template<typename T>
        inline void DestroySystem() {
            DestroySystem(T::Meta);
        }

    private:

        inline SpatialComponent* FindAttachmentComponent(SpatialComponent* search, StringID socket) const;
        inline SpatialComponent* FindAttachmentComponent(StringID socket) const { return FindAttachmentComponent(RootSpatialComponent, socket); }

        void CreateSpatialAttachment();

        void DestroySpatialAttachment(SpatialAttachmentRule rule);

        void RefreshChildSpatialATtachments();

        void RemoveComponentFromHierarchy(SpatialComponent* component);

        void GenerateSystemUpdateList();
        void RegisterComponentLocally(Component* c);
        void UnregisterComponentLocally(Component* c);

        // TODO: Loading context from engine ???
        bool UpdateState(LoadContext const& load, InitializationContext& init);
        void LoadComponents(LoadContext const& load);
        void UnloadComponents(LoadContext const& load);
        void Initialize(InitializationContext& init);
        void Shutdown(InitializationContext& init);

        void CreateSystemImmediately(reflect::Meta const* meta);
        void DestroySystemImmediately(reflect::Meta const* meta);
        void AddComponentImmediately(Component* c, SpatialComponent* parent);
        void DestroyComponentImmediately(Component* c);

    protected:

        EntityID        ID = EntityID::Generate();
        EntitySectorID  SectorID;
        SONIC_REFLECTION_FIELD()
        StringID        Name;
        Status          Status = Status::Unloaded;
        RegistryState   RegistryState = RegistryState::Unregistered;

        std::vector<System*>    Systems;
        std::vector<Component*> Components;

        SpatialComponent* RootSpatialComponent = nullptr;
        std::vector<Entity*> AttachedEntities;
        Entity* ParentEntity = nullptr;
        SONIC_REFLECTION_FIELD()
        StringID        AttachedSocketID;
        bool SpatialAttachmentReady = false;

        std::vector<Action> DeferredActions;

    };
}
