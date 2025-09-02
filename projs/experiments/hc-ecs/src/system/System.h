
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#include <memory>

#include "engine/EngineImpl.h"
#include "reflect/Type.h"
#include "reflect/plugin/Module.h"


namespace sonic {
    class Entity;
    class SceneUpdateContext;
    class Component;
    class Scene;

    /**
     * Component-managing system. A given System manages (stores, updates) all Components of a given type.
     *
     */
    struct System : public reflect::Reflect {
        SONIC_REFLECTION_CLASS(System);

        System() = default;
        System(System const&) = default;
        virtual ~System() {}

        System& operator=(System const& r) = default;

        virtual char const* GetName() const = 0;
        virtual void PreRegister() {} // First step of setup
        virtual void PostRegister() {} // Last step of setup
        virtual void PreUnregister() {} // First step of shutdown
        virtual void PostUnregister() {} // Last step of shutdown

    protected:

        virtual UpdatePriorityList const& GetUpdatePriorities() = 0;
        virtual void RegisterComponent(Component* c) = 0;
        virtual void UnregisterComponent(Component* c) = 0;
        virtual void Update(SceneUpdateContext const& ctx) = 0;
    };

    /**
     * A global, singleton system that manages all components of given types in the scene.
     * Requires very different initialization at a different time, so does not share hierarchy with the regular System class.
     */
    class SceneSystem : public reflect::Reflect {
        SONIC_REFLECTION_CLASS(SceneSystem);

    public:
        virtual size_t GetSystemID() const = 0;
        bool IsGameScene() const;
        bool IsToolScene() const;

    protected:
        virtual UpdatePriorityList const& GetUpdatePriorities() = 0;
        virtual void InitializeSystem(ModuleRegistry const& registry) {};
        virtual void Shutdown() {};
        virtual void Update(SceneUpdateContext const& ctx) {};
        virtual void RegisterComponent(Entity const* e, Component* c) = 0;
        virtual void UnregisterComponent(Entity const* e, Component* c) = 0;

    private:

        Scene* Scene = nullptr;
    };
}

