#include <shadow/system/EngineSystem.h>
#include "shadow/core/Libraries.h"
#include "spdlog/spdlog.h"
#include "dylib.hpp"
#include "shadow/util/Delegate.h"
#include <shadow/assets/fs/path.h>

namespace ShadowEngine {

  EngineSystem::~EngineSystem() = default;

  struct SystemManagerImpl : SystemManager {
    SystemManagerImpl() = default;

    ~SystemManagerImpl() {
        while(!systems.empty()) {
            delete systems.back();
            systems.pop_back();
        }

        for (void* lib : libraries)
            ShadowEngine::Library::unload(lib);

    }

    void initSystems() override {
        for (auto* sys : systems)
            sys->init();
    }

    void update(float dt) override {
        for (auto* sys : systems)
            sys->update(dt);
    }

    void* getLibrary(EngineSystem* sys) const override {
        auto idx = std::find(systems.begin(), systems.end(), sys);
        if (idx == systems.end()) return nullptr;

        return libraries[idx - systems.begin()];
    }

    const std::vector<void *> & getLibraries() override {
        return libraries;
    }

    const std::vector<EngineSystem *> & getSystems() override {
        return systems;
    }

    EngineSystem * getSystem(const char *name) override {
        for (auto* sys : systems) {
            if (strcmp(sys->getName(), name) == 0)
                return sys;
        }

        return nullptr;
    }

    Delegate<void (void *)> & onLibraryLoaded() override {
        return libraryLoaded;
    }

    void unload(struct EngineSystem *system) override {
        auto idx = std::find(systems.begin(), systems.end(), system);
        delete systems[idx - systems.begin()];
        ShadowEngine::Library::unload(libraries[idx - systems.begin()]);
        libraries.erase(libraries.begin() + (idx - systems.begin()));
        systems.erase(idx);
    }

    struct EngineSystem * load(const char *path) override {
        std::string p(path);
        std::string ext =
#ifdef _WIN32
            ".dll";
#elif defined __linux__
            ".so";
#else
#error Undefined platform
#endif

        if (!ShadowEngine::Path::hasExtension(p, ext)) p = p.append(ext);
        ShadowEngine::Path sePath(p);

        spdlog::info("Loading EngineSystem from ", p);

        using PluginConstructor = EngineSystem*(*)();
        auto* lib = ShadowEngine::Library::load(sePath);
        if (lib) {
            auto constructor = (PluginConstructor) ((dylib*)lib)->get_function<PluginConstructor>("createPlugin");
            if (constructor) {
                EngineSystem* system = constructor();
                if (!system) {
                    spdlog::error("Start failed!");
                    delete system;
                } else {
                    addSystem(system, lib);
                    libraryLoaded.invoke(lib);
                    spdlog::info("Loaded successfully.");
                    return system;
                }
            } else {
                spdlog::error("Plugin has no create function.");
            }

            ShadowEngine::Library::unload(lib);
        } else {
            spdlog::warn("Failed to load system from disk.");
        }

        return nullptr;
    }

    void addSystem(ShadowEngine::EngineSystem *system, void *library) override {
        systems.push_back(system);
        libraries.push_back(library);
        for (auto* sys : systems) {
            sys->systemAdded(*system);
            system->systemAdded(*sys);
        }
    }

  private:
    Delegate<void(void*)> libraryLoaded;
    std::vector<void*> libraries;
    std::vector<EngineSystem*> systems;
  };
}

std::unique_ptr<ShadowEngine::SystemManager> ShadowEngine::SystemManager::create() {
    return std::make_unique<SystemManagerImpl>();
}

void ShadowEngine::SystemManager::createAllSystems() {
    // TODO
}