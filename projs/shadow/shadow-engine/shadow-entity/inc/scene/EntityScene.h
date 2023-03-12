#pragma once

#include <scene/EntitySection.h>

namespace SE {

    /**
     * A scene is a collection of Sections that contain Entities.
     * Scenes have support for asynchronous task management.
     * Scenes have their own input state, local to the world.
     * Scenes have Systems that operate over all Scene Components of all entities within.
     * Scenes can be either for Tools or for Gameplay.
     *
     * Scenes each have a primary viewport and any number of secondary viewports.
     *
     * Scenes contain Sections, which contain Entities.
     * One of the Sections in the Scene is always loaded; similar to Spawn Chunks.
     *
     * Scenes have a local timescale, which can be increased to make physics and logic run faster.
     */
    class EntityScene {

        /**
         * The type of the scene.
         * More-or-less exists to allow differentiating between an Editor and a Runtime.
         */
        enum Type {
            TOOLS,  // This scene exists for tooling support, ie. the builtin editor.
            GAME    // This scene exists for gameplay, ie. the runtime.
        };

    public:

        EntityScene(Type wType = Type::GAME);
        ~EntityScene();

        // Fetch the ID of this scene.
        inline EntitySceneID const& GetID() const { return id; }
        // Whether this is a Gameplay scene.
        inline bool IsGameScene() const { return type == Type::GAME; }

        // Prepare the Scene for processing entity updates. TODO: Systems.
        void Initialize();
        // Prepare all Regions and Entities for the closing of the Scene.
        void Shutdown();


        // Whether or not updates are suspended. ( Game is Paused )
        inline bool IsSuspended() const { return suspended; }

        // Stop all Regions, Entities and Systems from updating.
        void Suspend() { suspended = true; }

        // Resume updates to the Entities and Systems within.
        void Resume() { suspended = false; }

        // Run a single tick of Entity and System updates. TODO: context
        void Update();

        // Handle Entities, Regions, Resources that want to load or unload.
        void UpdateLoads();

        // TODO: Systems

        // TODO: Input


        // Check whether time is paused; the Scene itself can be unpaused during this, to allow movement through a paused world.
        inline bool IsTimePaused() const { return timeScale <= 0.0f; }

        // Pause time in the Scene.
        inline void PauseTime() { timeScale = 0.0f; }

        // The Time Scale for the Scene
        inline float GetTimeScale() const { return timeScale; }

        // Set the Time Scale for the Scene.
        inline void SetTimeScale(float newScale) { timeScale = newScale; }

        // Request a Time Step through a paused world.
        inline void TimeStep() { timeStepNeeded = true; }

        // Whether a Scene with Paused Time needs a time step
        inline bool TimeStepRequested() const { return timeStepNeeded; }

        // How long a single Time Step last, at most.
        inline float TimeStepLength() const { return timeStepLength; }

        // Set the maximum length of a Time Step.
        inline void SetTimeStepLength(float newStep) { timeStepLength = newStep; }

        // TODO: Viewports

        // Fetch the Section that is always present and loaded; be careful adding things to this.
        EntitySection* GetPersistentSection() { return sections[0]; }
        EntitySection const* GetPersistentSection() const { return sections[0]; }

        // Get the first Section that is not persistent, if present.
        EntitySection* GetFirstNonPersistentSection() { return sections.size() > 1 ? sections[1] : nullptr; }
        EntitySection const* GetFirstNonPersistentSection() const { return sections.size() > 1 ? sections[1] : nullptr; }

        // Create a transient (memory-resident) Section.
        EntitySection* CreateTransientSection();

        // Get a Section from memory.
        EntitySection const* GetSection(EntitySectionID const& sID) const;
        EntitySection* GetSection(EntitySectionID const& sID);

        // Whether anything is loading into a Section
        bool IsLoading() const;

        // Whether a Section exists in this scene
        bool HasSection(const EntitySectionID& eID) const;
        // Whether the Section in this scene is loaded.
        bool isSectionLoaded(const EntitySectionID& sID) const;

        // Load a Section from the Scene file into memory.
        EntitySectionID LoadSection(EntitySectionID& section);
        // Unload a section from memory; delete every entity within.
        void UnloadSection(EntitySectionID& section);

        inline Entity* FindEntity(EntityID& entity) const {
            Entity* e = nullptr;
            for (auto const& section : sections) {
                e = section->GetEntity(entity);
                if (e != nullptr) break;
            }

            return e;
        }

    private:
        // The ID of this Scene.
        EntitySceneID id = UUID::Generate();

        // The system that manages asynchronous background tasks for this Scene, such as Systems and Entity management.
        // Tasks* tasks;

        // The local Input State, which exists relative to the viewport, allowing for tri picking.
        // Input input;

        // TODO: Loading/Initialization

        // TODO: Scene Systems

        // The type of this Scene; default to game.
        Type type = Type::GAME;

        // Whether or not everything in the map is ready to start.
        bool initialized = false;
        // Whether or not everything in the map is paused; no logic runs, nothing updates.
        bool suspended = false;

        // TODO: Viewport

        // The list of Sections in this Scene.
        std::vector<EntitySection*> sections;

        // Every entity that is eligible for updates.
        // Ie. Every Initialized Entity in every Loaded Section.
        std::vector<Entity*> entityUpdateList;

        //TODO: Scene Systems 2

        // The multiplier for the number of steps per second (1/timescale * stepInterval)
        float timeScale = 1.0f;
        // The maximum length of each time step; if a step goes over this, it is aborted and the next begins processing.
        float timeStepLength = 1.0f/30;
        // Whether or not a new Time Step should begin immediately. Only applicable to Scenes with Paused Time.
        bool timeStepNeeded = false;
    };
}