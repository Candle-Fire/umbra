#pragma once
#include <map>
#include <shadow/assets/fs/hash.h>
#include <shadow/assets/fs/path.h>

namespace ShadowEngine {

  /**
   * Handles all of the Resources of a single Type.
   * Handles reference counting, hot reloading, and etc.
   */

  struct ResourceTypeManager {
    friend struct Resource;
    friend struct ResourceManager;

    using ResourceTable = std::map<PathHash, struct Resource*>;

    void create(struct ResourceType type, struct ResourceManager& manager);
    void destroy();

    void setUnloadable(bool status);

    void removeUnreferencedResources();

    void reload(const Path& path);
    void reload(Resource& resource);

    ResourceTable& getResources() { return resources; }

    ResourceTypeManager();
    virtual ~ResourceTypeManager();
    ResourceManager& getOwner() const { return *owner; }

  protected:
    Resource* load(const Path& path);
    virtual Resource* createResource(const Path& path) = 0;
    virtual void destroyResource(Resource& res) = 0;
    Resource* get(const Path& path);

    ResourceTable resources;
    ResourceManager* owner;
    bool unloadEnabled;
  };

  /**
   * Handles all of the ResourceTypeManagers, for every ResourceType with at least one applicable Resource
   */
  struct ResourceManager {
    using ResourceTypeManagers = std::map<ResourceType, ResourceTypeManager*>;

    struct LoadHook {
      enum class Action { IMMEDIATE, DEFERRED };
      virtual ~LoadHook();
      virtual Action load(Resource& res) = 0;
      void continueLoad(Resource& res);
    };

    static ResourceManager& Get();
    ResourceManager();
    ~ResourceManager();
    ResourceManager(const ResourceManager& o) = delete;

    void init(struct FileSystem& fs);

    ResourceTypeManager* get(ResourceType);
    const ResourceTypeManagers& getAll() const { return managers; }

    template <typename R>
    R* load(const Path& path) {
        return static_cast<R*>(load(R::TYPE, path));
    }

    Resource* load(ResourceTypeManager& manager, const Path& path);
    Resource* load(ResourceType type, const Path& path);

    void setLoadHook(LoadHook* hook);
    bool isHooked() const { return hook; }
    LoadHook::Action onLoad(Resource& res) const;
    void add(ResourceType, ResourceTypeManager* manager);
    void remove(ResourceType type);
    void reload(const Path& path);
    void reloadAll();
    void removeUnreferenced();
    void setUnloadable(bool enable);

    FileSystem& getFileSystem() { return *filesystem; }

  private:
    ResourceTypeManagers managers;
    FileSystem* filesystem;
    LoadHook* hook;

  };
}