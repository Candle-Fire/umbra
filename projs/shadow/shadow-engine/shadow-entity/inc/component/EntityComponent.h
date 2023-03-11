#pragma once
#include <cstdint>
#include "id/ID.h"

namespace SE {

    /**
     * A Component is a part of an Entity that stores and manages data.
     * This data is held separately from the Entity that requests it.
     *
     * Components can be Spatial (ie. Position, Rotation).
     * Components can also be Visual (ie. Mesh, Collider).
     * Components are part of the larger Entity-Component-System architecture.
     *
     * Systems operate over these Components.
     * Entities may have their own Systems, and each Scene has its own set of global Systems.
     * Components may be registered to only one of these.
     *
     * EntityComponent classes are not singleton; new EntityComponent(..) is valid.
     *
     */
    class EntityComponent {
    public:

        /**
         * The Component's possible statuses.
         */
        enum class Status: uint8_t {
            Unloaded,       // The component is not yet loaded.
            Loading,        // The component is being loaded.
            Loaded,         // The component is finished loading, but is not yet populated with data.
            Failed,         // The component encountered an error while loading.
            Initialized     // The component is finished loading, and is populated with data.
        };

        virtual ~EntityComponent();

        // Get the ComponentID.
        inline ComponentID const& GetID() const { return id; }
        // Get the Name of this Component; guaranteed to be unique in a given entity.
        inline std::string& GetName() { return name; }
        // Get the Entity that owns this Component.
        inline EntityID const& GetParent() const { return owner; }

        inline Status GetStatus() const { return status; }
        // Check Status
        inline bool IsLoaded() const { return status == Status::Loaded; }
        inline bool IsLoading() const { return status == Status::Loading; }
        inline bool IsUnloaded() const { return status == Status::Unloaded; }
        inline bool Failed() const { return status == Status::Failed; }
        inline bool Initialized() const { return status == Status::Initialized; }

        // Whether one instance of this Component is allowed per Entity
        virtual bool IsSingleton() const { return false; }

    protected:

        EntityComponent() = default;
        EntityComponent(std::string& name) : name(name) {}

        // Load Component data. May perform background ops.
        virtual void Load() = 0;
        // Unload Component data. Must be blocking.
        virtual void Unload() = 0;

        // Check that everything went properly.
        virtual void UpdateStatus() = 0;

        // Update the status to Initialized. Must only be called if checks passed.
        // The status must be Loaded.
        virtual void Initialize() { status = Status::Initialized; }

        // Prepare for unloading the Component. Must be called before the process begins.
        // The status must be Initialized.
        virtual void Close() { status == Status::Loaded; }

    private:
        // This Component's unique ID.
        ComponentID id = ComponentID::Generate();
        // The Entity that requested this Component.
        EntityID owner;
        // The name of the Component, for visualization
        std::string name;
        // The status of the Component
        Status status = Status::Unloaded;

        // Whether this Component is registered to the Entity's Local Systems.
        bool localComponent = false;
        // Whether this Component is registered to the Scene's Global Systems.
        bool globalComponent = false;
    };
}