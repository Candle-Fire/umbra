#pragma once

#include <shadow/exports.h>
#include <string>
#include <memory>
#include <vector>
#include "shadow/assets/fs/file.h"
#include <shadow/system/EngineSystem.h>

namespace ShadowEngine {

  struct API Engine {

    struct Initialization {
      const char* workingDir = nullptr;
      std::vector<std::string> plugins;
      std::unique_ptr<FileSystem> fs;
    };

    virtual ~Engine() {}
    static std::unique_ptr<Engine> create(Initialization&& init);

    virtual void init() = 0;
    virtual struct World& createWorld(bool main) = 0;
    virtual void destroyWorld(World& world) = 0;

    virtual struct FileSystem& getFileSystem() = 0;

    virtual struct SystemManager getSystemManager() = 0;
    virtual struct ResourceManager getResourceManager() = 0;

    virtual void startGame(World& world) = 0;
    virtual void stopGame(World& world) = 0;

    virtual void update(World& world) = 0;

    virtual float getLastTimeDelta() const = 0;
    virtual void setTimeScale(float multiplier) = 0;
    virtual void pause(bool pause) = 0;
    virtual bool isPaused() const = 0;

    virtual void nextFrame() = 0;

  protected:
    Engine() {}
    Engine(const Engine&) = delete;
  };
}