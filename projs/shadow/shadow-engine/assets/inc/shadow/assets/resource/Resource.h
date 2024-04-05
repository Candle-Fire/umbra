#pragma once

#include "shadow/assets/fs/hash.h"
#include "shadow/assets/fs/path.h"
#include "shadow/assets/fs/file.h"
#include <shadow/util/DelegateList.h>

namespace ShadowEngine {

  /**
   * A runtime-only struct that determines the type of a resource - whether it be a texture, mesh, animation, or other data.
   * Provides some specializations for living in a map.
   */
  struct ResourceType {
    ResourceType() = default;
    explicit ResourceType(const std::string& name);
    bool operator!=(const ResourceType& o) const { return o.hash != hash; }
    bool operator==(const ResourceType& o) const { return o.hash == hash; }
    bool operator< (const ResourceType& o) const { return o.hash.getHash() < hash.getHash(); }
    bool isValid() const { return hash.getHash() != 0; }

    HeapHash hash;
  };

  // A Resource Type that is guaranteed to be invalid.
  static std::string empty;
  const ResourceType INVALID_RESOURCE(empty);

  // A specialization of HashFunc for ResourceTypes, since they already have a HeapHash within.
  template<> struct HashFunc<ResourceType> {
    static uint32_t get(const ResourceType& key) { return HashFunc<HeapHash>::get(key.hash); }
  };

#pragma pack(1)
  struct ResourceHeader {
    static const uint32_t MAGIC;
    uint32_t magic = MAGIC; // VXI Package header
    uint32_t version = 0;
    uint32_t flags = 0;
    uint32_t padding = 0;
    uint32_t decompressedSize = 0;
  };
#pragma pack()

  /**
   * A basic Resource type.
   * Represents a single file loaded from disk.
   * May have dependencies on other Resources, and other Resources may depend on this.
   * Resources are reference-counted, and are removed when they go out of usage.
   */

  struct Resource {

    friend struct ResourceTypeManager;
    friend struct ResourceManager;

    enum class State : uint32_t {
      EMPTY = 0,
      READY,
      FAILED
    };

    using Observer = DelegateList<void(State, State, Resource&)>;

    virtual ~Resource();
    virtual ResourceType getType() const = 0;
    State getState() const { return state; }

    bool isEmpty() const { return state == State::EMPTY; }
    bool isReady() const { return state == State::READY; }
    bool isFailure() const { return state == State::FAILED; }

    uint32_t getReferenceCount() const { return references; }

    Observer const& getCallback() const { return callback; }
    size_t getSize() const { return size; }

    const Path& getPath() const { return path; }

    struct ResourceTypeManager& getManager() { return manager; }

    uint32_t decreaseReferences();
    uint32_t increaseReferences() { return references++; }

    bool toInitialize() const { return desiredState == State::READY; }
    bool isHooked() const { return hooked; }

    template <auto Function, typename C> void onLoaded(C* instance) {
        callback.bind<Function>(instance);
        if (isReady()) (instance->*Function)(State::READY, State::READY, *this);
    }

  protected:
    Resource(Path  path, ResourceTypeManager& manager);

    virtual void onReadying() {}
    virtual void unload() = 0;
    virtual bool load(size_t size, const uint8_t* mem) = 0;

    void onCreated(State newState);
    void performUnload();
    void addDependency(Resource& dependent);
    void removeDependency(Resource& dependent);
    void checkState();
    void refresh();

    State desiredState;
    uint16_t emptyDependencies;
    ResourceTypeManager& manager;

  private:

    void doLoad();
    void fileLoaded(size_t fileSize, const uint8_t* mem, bool success);
    void stateChanged(State old, State newState, Resource&);

    Resource(const Resource&) = delete;
    void operator=(const Resource&) = delete;

    Observer callback;
    size_t size;
    Path path;
    uint32_t references;
    uint16_t failedDependencies;
    FileSystem::AsyncHandle handle;
    State state;
    bool hooked = false;
  };

  struct PrefabResource : Resource {
    PrefabResource(const Path& path, ResourceTypeManager& resource_manager);
    ResourceType getType() const override;
    void unload() override;
    bool load(size_t size, const uint8_t* data) override;

    OutputMemoryStream data;
    StableHash hash;
    static const ResourceType TYPE;
  };
}