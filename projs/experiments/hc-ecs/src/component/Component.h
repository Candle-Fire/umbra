
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#include "reflect/Type.h"

namespace sonic {
    /**
     * A component is a chunk of data assigned to an object.
     * A given component is stored contiguously with other components of the same type.
     * All Components must be registered with a System that will operate over its' data.
     * A Component cannot belong to multiple Systems.
     * Components are uniquely identified by either:
     *  - the Component's unique ID, OR
     *      - the Entity's unique ID, AND
     *      - the name of the Component slot on the Entity that it is attributed to.
     * So, A Component's data can be found either by the Component ID, or the Entity and its' named characteristic.
     */
    class Component : public reflect::Reflect {
    public:

        SONIC_REFLECTION_CLASS(Component);

        enum class Status {
            Uninitialized = 0,      // The Component has been unloaded, or the entity it belongs to is not yet added to the world.
            Loading,                // The Component or the Entity that it belongs to is in the process of being added to the world.
            Loaded,                 // The Component or the Entity that it belongs to is added to the world, but the data is not yet ready for usage.
            Errored,                // Something went wrong when adding the Component or the Entity that it belongs to, to the world. Likely, another Component of the same Entity has thrown an exception.
            Initialized,            // The Component or the Entity that it belongs to is fully added to the world and ready to use, and data has been allocated and prepared for this Component.
        };

        /**
         * A given Component must be registered with a System, but said System may belong either to the Entity World, or to a given Entity.
         */
        enum class Registry {
            None = 0,
            Entity,
            World
        };

        virtual ~Component();

        inline ComponentID const& GetID() { return ID; }
        inline StringID GetName() { return Name; }
        inline EntityID GetEntity() { return Entity; }

        inline bool IsUninitialized() const { return Status == Status::Uninitialized; }
        inline bool IsLoading() const { return Status == Status::Loading; }
        inline bool IsLoaded() const { return Status == Status::Loaded; }
        inline bool IsErrored() const { return Status == Status::Errored; }
        inline bool IsInitialized() const { return Status == Status::Initialized; }
        inline Status GetStatus() const { return Status; }

        virtual bool IsSingleton() const { return false; }

    protected:

        Component() = default;
        Component(Component const& c) = default;
        Component(StringID name) : Name(name) {}

        Component& operator=(Component const& c) = default;

        // TODO: Loading requires context from the engine: task system, resource access, etc. How do we keep that detached from the engine?
        virtual void Load(void* data) = 0;      // Begin loading Component data, connected resources, related hierarchies, etc.
        // TODO: See above.
        virtual void Unload(void* data) = 0;    // Unload and free all Component data.
        virtual void UpdateLoadStage() = 0;     // Update loading state, applies recursively.
        virtual void Initialize() {}            // Data has finished loading - initial content may need to be applied.
        virtual void Shutdown() {}              // Prepare for transition to unloaded state.

        ComponentID     ID = ComponentID::Generate();       // The unique Component ID.
        EntityID        Entity;                             // The unique Entity that this Component is attached to.
        SONIC_REFLECTION_FIELD()
        StringID        Name;                               // The name of the Component slot on the parent Entity that this Component is assigned to.
        Status          Status = Status::Uninitialized;     // The operational readiness status of the Component.
        Registry        Registry = Registry::None;          // Where this Component is registered, in the System hierarchy
    };
} // namespace sonic
