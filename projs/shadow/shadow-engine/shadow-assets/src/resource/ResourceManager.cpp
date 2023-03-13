#include "ResourceManager.h"
#include "Resource.h"
#include "spdlog/spdlog.h"

namespace ShadowEngine {

    void ResourceTypeManager::create(struct ResourceType type, struct ResourceManager &manager) {
        manager.add(type, this);
        owner = &manager;
    }

    void ResourceTypeManager::destroy() {
        for (auto iter = resources.begin(), end = resources.end(); iter != end; ++iter) {
            Resource* res = iter->second;
            if (!res->isEmpty())
                spdlog::error("Resource Type Manager destruction leaks ", res->path.get());

            destroyResource(*res);
        }
        resources.clear();
    }

    Resource* ResourceTypeManager::get(const Path& path) {
        auto it = resources.find(path.getHash());
        if (it != resources.end()) return it->second;
        return nullptr;
    }

    Resource* ResourceTypeManager::load(const Path &path) {
        if (path.isEmpty()) return nullptr;
        Resource* res = get(path);
        if (res == nullptr) {
            res = createResource(path);
            resources[path.getHash()] = res;
        }

        if (res->isEmpty() && res->desiredState == Resource::State::EMPTY) {
            if (owner->onLoad(*res) == ResourceManager::LoadHook::Action::DEFERRED) {
                res->hooked = true;
                res->desiredState = Resource::State::READY;
                res->increaseReferences();
                res->increaseReferences();
                return res;
            }

            res->doLoad();
        }

        res->increaseReferences();
        return res;
    }

    void ResourceTypeManager::removeUnreferencedResources() {
        if (!unloadEnabled) return;

        std::vector<Resource*> toRemove;
        for (auto i : resources)
            if (i.second->getReferenceCount() == 0) toRemove.push_back(i.second);

        for (auto i : toRemove) {
            auto iter = resources.find(i->getPath().getHash());
            if (iter->second->isReady()) iter->second->performUnload();
        }
    }

    void ResourceTypeManager::reload(const Path &path) {
        Resource* res = get(path);
        if (res) reload(*res);
    }

    void ResourceTypeManager::reload(Resource& res) {
        if (res.state != Resource::State::EMPTY)
            res.performUnload();
        else if (res.desiredState == Resource::State::READY)
            return;

        if (owner->onLoad(res) == ResourceManager::LoadHook::Action::DEFERRED) {
            res.hooked = true;
            res.desiredState = Resource::State::READY;
            res.increaseReferences();
            res.increaseReferences();
        } else {
            res.performUnload();
        }
    }

    void ResourceTypeManager::setUnloadable(bool status) {
        unloadEnabled = status;
        if (!unloadEnabled) return;

        for (auto res : resources)
            if (res.second->getReferenceCount() == 0)
                res.second->performUnload();
    }

    ResourceTypeManager::ResourceTypeManager() :
        resources(),
        owner(nullptr),
        unloadEnabled(true) {

    }

    ResourceTypeManager::~ResourceTypeManager() {

    }

    ResourceManager::ResourceManager() :
        managers(),
        hook(nullptr),
        filesystem(nullptr) {

    }

    ResourceManager::~ResourceManager() = default;

    void ResourceManager::init(FileSystem &fs) {
        filesystem = &fs;
    }

    Resource* ResourceManager::load(ResourceType type, const Path& path) {
        ResourceTypeManager* manager = get(type);
        if (!manager) return nullptr;
        return load(*manager, path);
    }

    Resource* ResourceManager::load(ResourceTypeManager& manager, const Path& path) {
        return manager.load(path);
    }

    ResourceTypeManager* ResourceManager::get(ResourceType type) {
        auto iter = managers.find(type);
        if (iter == managers.end()) return nullptr;
        return iter->second;
    }

    void ResourceManager::LoadHook::continueLoad(Resource &res) {
        res.decreaseReferences();
        res.hooked = false;
        res.desiredState = Resource::State::EMPTY;
        res.doLoad();
    }

    void ResourceManager::setLoadHook(LoadHook *loadHook) {
        hook = loadHook;

        if (hook)
            for (auto manager : managers)
                for (auto res : manager.second->getResources())
                    if (res.second->isFailure())
                        manager.second->reload(*res.second);
    }

    ResourceManager::LoadHook::Action ResourceManager::onLoad(Resource &res) const {
        return hook ? hook->load(res) : LoadHook::Action::IMMEDIATE;
    }

    void ResourceManager::add(ResourceType type, ResourceTypeManager* manager) {
        managers[type] = manager;
    }

    void ResourceManager::remove(ResourceType type) {
        managers.erase(type);
    }

    void ResourceManager::removeUnreferenced() {
        for (auto manager : managers)
            manager.second->removeUnreferencedResources();
    }

    void ResourceManager::setUnloadable(bool enable) {
        for (auto manager : managers)
            manager.second->setUnloadable(enable);
    }

    void ResourceManager::reloadAll() {
        while (filesystem->hasWork()) filesystem->processCallbacks();

        std::vector<Resource*> toReload;
        for (auto manager : managers) {
            ResourceTypeManager::ResourceTable& resources = manager.second->getResources();
            for (auto res : resources) {
                if (res.second->isReady()) {
                    res.second->performUnload();
                    toReload.push_back(res.second);
                }
            }
        }
    }

    void ResourceManager::reload(const Path& path) {
        for (auto manager : managers)
            manager.second->reload(path);
    }

}