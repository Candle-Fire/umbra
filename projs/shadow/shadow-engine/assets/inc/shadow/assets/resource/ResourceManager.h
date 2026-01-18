#pragma once
#include <map>
#include <shadow/assets/resource/Resource.h>

namespace SH::Asset {

    /**
     * Handles all of the Resources of a single Type.
     * Handles reference counting, hot reloading, and etc.
     */

    struct ResourceTypeManager {
        friend struct Resource;
        friend struct ResourceManager;

        using ResourceTable = std::map<PathHash, Resource*>;

        // Create a Manager for a given Type, parented to the given Manager.
        void create(struct ResourceType type, struct ResourceManager& manager);
        // Destroy this manager, remove all resources of this Manager's Type from the engine.
        void destroy();

        // Toggle whether individual Resources of this Type can be unloaded from the engine
        void setUnloadable(bool status);

        // Delete all Resources of this Type that no longer have anything referencing them.
        void removeUnreferencedResources();

        // Reload the resource at the given path.
        void reload(const Path& path);
        // Reload the resource.
        void reload(Resource& resource);

        // Get all resources managed by this Manager.
        ResourceTable& getResources() { return resources; }

        ResourceTypeManager();
        virtual ~ResourceTypeManager();
        ResourceManager& getOwner() const { return *owner; }

    protected:
        // Load a resource from the given Path. file > engine
        Resource* load(const Path& path);
        // Create a resource, saved to the given path. engine > file
        virtual Resource* createResource(const Path& path) = 0;
        // Destroy the resource from the manager. Remains on disk, if it exists there.
        virtual void destroyResource(Resource& res) = 0;
        // Get the resource from the given path, loading it if it is not already in memory.
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

        /**
         * A callback hook, to allow for custom behaviours when a new Resource is loaded from disk.
         * Note: loaded includes reloading from filesystem.
         */
        struct LoadHook {
            enum class Action { IMMEDIATE, DEFERRED };
            virtual ~LoadHook();
            virtual void loadRaw(const Path& requester, const Path& path) = 0;
            virtual Action load(Resource& res) = 0;
            void continueLoad(Resource& res, bool success);
        };

        ResourceManager();
        ~ResourceManager();
        ResourceManager(const ResourceManager& o) = delete;

        // Prepare this Resource Manager to operate on the given DFS structure. This will be the root of all engine files.
        void init(struct FileSystem& fs);

        // Get the Manager of Resources of a given Type.
        ResourceTypeManager* get(ResourceType);
        // Get the Manager of all Resource Types.
        const ResourceTypeManagers& getAll() const { return managers; }

        // Load a template-typed ResourceType from the given path.
        template <typename R>
        R* load(const Path& path) {
            return static_cast<R*>(load(R::TYPE, path));
        }

        // Load a Resource from the given Path.
        Resource* load(ResourceTypeManager& manager, const Path& path);
        // Load a Resource from the given Path.
        Resource* load(ResourceType type, const Path& path);

        // Load a Resource's Dependency.
        bool loadRaw(const Path& requester, const Path& path, OutputMemoryStream& data);

        // Set the Load Hook's implementation.
        void setLoadHook(LoadHook* hook);
        // Check whether something is implementing the Load Hook.
        bool isHooked() const { return hook; }
        // Call the Load Hook.
        LoadHook::Action onLoad(Resource& res) const;

        // Add a new Manager for a given ResourceType.
        void add(ResourceType, ResourceTypeManager* manager);
        // Remove the manager for the given ResourceType.
        void remove(ResourceType type);
        // Reload the resource at the given path.
        void reload(const Path& path);
        // Reload all resources, except those that were created virtually and do not exist on disk.
        void reloadAll();
        // Remove all resources that do not have any other resources referencing them.
        void removeUnreferenced();

        // Set whether individual resources can be unloaded.
        void setUnloadable(bool enable);

        FileSystem& getFileSystem() { return *filesystem; }

    private:
        ResourceTypeManagers managers;
        FileSystem* filesystem;
        LoadHook* hook;

    };
}