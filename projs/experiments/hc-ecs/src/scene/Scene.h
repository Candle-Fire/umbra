
/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

#include "engine/EngineImpl.h"
#include "system/System.h"

namespace sonic {

    class EntitySector;

    /**
     * A set of Entities and Systems.
     * Can be either an interactive game world, or a script-only (non-physical) world.
     */
    class Scene {
    public:

        enum class SceneType {
            Game,
            Script
        };


        Scene(SceneType = SceneType::Game);
        ~Scene();

        inline SceneID const& GetID() const { return ID; }
        inline SceneType GetType() const { return Type; }
        inline bool IsGameScene() const { return Type == SceneType::Game; }

        void Init(ModuleRegistry const& reg, std::vector<reflect::Meta const*> systemMetas);
        void Shutdown();

        inline bool IsPaused() const { return Paused; }
        void Pause() { Paused = true; }
        void Resume() { Paused = false; }

        void Update(SceneUpdateContext const& ctx);
        void UpdateLoading();

        SceneSystem* GetSceneSystem(size_t ID) const;

        template<typename T>
        inline T* GetSceneSystem() const { return reinterpret_cast<T*>(GetSceneSystem(T::SystemID)); }

        inline bool IsStopped() const { return TimeScale <= 0.0f; }
        inline void SetStopped() { TimeScale = 0.0f; }
        inline float GetTimeScale() const { return TimeScale; }
        inline void SetTimeScale(float newTimescale) { TimeScale = newTimescale; }
        inline void RequestTimeStep() { TimeStepNeeded = true; }
        inline bool IsTimeStepNeeded() const { return TimeStepNeeded; }
        inline size_t GetTimeStep() const { return TimeStep; }
        inline void SetTimeStep(size_t stepLength) { TimeStep = stepLength; }

    private:

        SceneID                     ID = SceneID::Generate();
        LoadContext                 LoadCtx;
        InitializationContext       InitCtx;
        std::vector<SceneSystem>    SceneSystems;
        SceneType                   Type;
        bool                        Initialized;
        bool                        Paused;

        std::vector<EntitySector*> Sectors;

        std::vector<Entity*>        Entities;
        std::vector<SceneSystem*>   SceneSystemUpdates;

        float                       TimeScale;
        size_t                      TimeStep;
        bool                        TimeStepNeeded;


    };
}
