#include <shadow/core/Engine.h>
#include "shadow/assets/resource/ResourceManager.h"
#include "shadow/assets/resource/Resource.h"

namespace ShadowEngine {

  struct PrefabManager final : ResourceTypeManager {
    explicit PrefabManager() : ResourceTypeManager() { }

    Resource* createResource(const Path& path) override {
        return new PrefabResource(path, *this);
    }

    void destroyResource(Resource& res) override {
        free(dynamic_cast<PrefabResource*>(&res));
    }
  };

  struct EngineImpl : Engine {
    void operator=(const EngineImpl&) = delete;
    EngineImpl(const EngineImpl&) = delete;

    EngineImpl(Initialization&& init) :
        prefabManager(), resourceManager(), gameRunning(false), smoothTimeDelta(1/60.f), timeScale(1.0f), paused(false), nextFrame(false) {



    }

  private:

    std::unique_ptr<FileSystem> fileSystem;
    ResourceManager resourceManager;
    std::unique_ptr<SystemManager> systemManager;
    PrefabManager prefabManager;

    float timeScale;
    float deltas[11] = {};
    float smoothTimeDelta;
    bool gameRunning;
    bool paused;
    bool nextFrame;

    FileOutput logFile;
    bool logFileOpen = false;
  };
}