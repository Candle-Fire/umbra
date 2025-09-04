/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/
#pragma once
#include "Scene.h"
#include "reflect/Type.h"
#include "reflect/plugin/Module.h"

namespace sonic {
    class Scene;
}

namespace sonic {

    class SceneManager : public Module {
    public:
        SONIC_MODULE(SceneManager);

        ~SceneManager();

        void Initialize(ModuleRegistry const& registry);
        void Shutdown();

        void StartFrame();
        void EndFrame();

        bool IsLoading() const;
        void UpdateLoading();

        Scene* GetGameScene();

        Scene const* GetGameScene() const { return const_cast<SceneManager*>(this)->GetGameScene(); }

        Scene* CreateScene(Scene::SceneType type);

        void DestroyScene(Scene* scene);

        std::vector<Scene*> const& GetScenes() const { return Scenes; }

        void UpdateScenes(SceneUpdateContext const& ctx);

        ModuleRegistry const* Registry;
        std::vector<Scene*> Scenes;
        std::vector<reflect::Meta const*> SystemMetas;
    };
}
