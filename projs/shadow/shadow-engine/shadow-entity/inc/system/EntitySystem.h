#pragma once
#include <string>

namespace SE {

    // Forward declaration.
    class EntityComponent;

    /**
     * A System is a routine that operates over the Components in an Entity.
     * A System may be local to a given Entity, or it may be glboal to the Scene.
     *
     * Components must be registered to the System that works over them.
     *
     * TODO: Think of better names for these functions
     */
    class EntitySystem {
    public:

        virtual ~EntitySystem();
        virtual std::string& GetName() const = 0;

        // Pre-Component Register
        virtual void Initialize() {}

        // Post-Component Register
        virtual void Initialized() {}

        // Pre-Component Unregister
        virtual void Uninitialize() {}

        // Post-Component Unregister
        virtual void Uninitialized() {}

    protected:

        // Register a Component with this System
        virtual void RegisterComponent(EntityComponent* component) = 0;
        // Unregister a Component from this system
        virtual void UnregisterComponent(EntityComponent* component) = 0;

        // Update over all Components. TODO: Context?
        virtual void Update() = 0;
    };
}