#include "shadow/assets/resource/Resource.h"
#include "shadow/assets/fs/file.h"
#include "shadow/assets/management/delegate.h"
#include <shadow/assets/resource/ResourceManager.h>
#include <spdlog/spdlog.h>

#include <utility>

namespace ShadowEngine {
  const uint32_t ResourceHeader::MAGIC = 'VXIP';

  ResourceType::ResourceType(const std::string& name) {
      hash = HeapHash(name);
  }

  Resource::Resource(ShadowEngine::Path  path, ShadowEngine::ResourceTypeManager &manager)
      : references(0),
        emptyDependencies(0),
        failedDependencies(0),
        state(State::EMPTY),
        desiredState(State::EMPTY),
        path(std::move(path)),
        size(),
        callback(),
        manager(manager),
        handle(FileSystem::AsyncHandle::invalid()) {
  }

  Resource::~Resource() = default;

  void Resource::refresh() {
      if (state == State::EMPTY) return;

      const State old = state;
      state = State::EMPTY;
      callback.invoke(old, state, *this);
      checkState();
  }

  void Resource::checkState() {
      State old = state;
      if (failedDependencies > 0 && state != State::FAILED) {
          state = State::FAILED;
      } else if (failedDependencies == 0) {
          if (emptyDependencies > 0 && state != State::EMPTY)
              state = State::EMPTY;

          if (emptyDependencies == 0 && state != State::READY && desiredState != State::EMPTY) {
              onReadying();

              if (emptyDependencies != 0 || state == State::READY || desiredState == State::EMPTY)
                  return;

              if (failedDependencies != 0) {
                  checkState();
                  return;
              }

              state = State::READY;
          }
      }
      callback.invoke(old, state, *this);
  }

  void Resource::fileLoaded(size_t fileSize, const uint8_t *mem, bool success) {
      handle = FileSystem::AsyncHandle::invalid();
      if (desiredState != State::READY) return;

      if (!success) {
          ResourceManager& owner = getManager().getOwner();
          if (!hooked && owner.isHooked()) {
              if (owner.onLoad(*this) == ResourceManager::LoadHook::Action::DEFERRED) {
                  hooked = true;
                  desiredState = State::READY;
                  increaseReferences();
                  return;
              }
          }

          --emptyDependencies;
          ++failedDependencies;
          checkState();
          handle = FileSystem::AsyncHandle::invalid();
          return;
      }

      const auto* header = (const ResourceHeader*) mem;

      if (size < sizeof(*header)) {
          spdlog::error("Invalid resource: ", path.get(), ": size mismatch. Expected ", fileSize, ", got " , sizeof(*header));
          failedDependencies++;
      } else if (header->magic != ResourceHeader::MAGIC) {
          spdlog::error("Invalid resource: " , path.get(), ": magic number mismatch. Expected " , ResourceHeader::MAGIC, ", got ", header->magic);
          failedDependencies++;
      } else if (header->version > 0) {
          spdlog::error("Invalid resource: ", path.get(), ": verison mismatch. Expected 0, got ", header->version);
          failedDependencies++;
      } else {
          // TODO: Compression?
          if (!load(size - sizeof(*header), mem + sizeof(*header)))
              failedDependencies++;
          size = header->decompressedSize;
      }

      emptyDependencies--;
      checkState();
      handle = FileSystem::AsyncHandle::invalid();
  }

  void Resource::performUnload() {
      if (handle.valid()) {
          FileSystem& fs = manager.getOwner().getFileSystem();
          fs.cancelAsync(handle);
          handle = FileSystem::AsyncHandle::invalid();
      }

      hooked = false;
      desiredState = State::EMPTY;
      unload();

      size = 0;
      emptyDependencies = 1;
      failedDependencies = 0;
      checkState();
  }

  void Resource::onCreated(ShadowEngine::Resource::State newState) {
      state = newState;
      desiredState = State::READY;
      failedDependencies = state == State::FAILED ? 1 : 0;
      emptyDependencies = 0;
  }

  void Resource::doLoad() {
      if (desiredState == State::READY) return;
      desiredState = State::READY;

      if (handle.valid()) return;

      FileSystem& fs = manager.getOwner().getFileSystem();
      FileSystem::ContentCallback cb = makeDelegate<&Resource::fileLoaded>(this);

      const PathHash hash = path.getHash();
      Path resourcePath("./resources/" + std::to_string(hash.getHash()) + ".res");
      handle = fs.readAsync(resourcePath, cb);
  }

  void Resource::addDependency(ShadowEngine::Resource &dependent) {
      dependent.callback.bind<&Resource::stateChanged>(this);
      if (dependent.isEmpty()) emptyDependencies++;
      if (dependent.isFailure()) failedDependencies++;

      checkState();
  }

  void Resource::removeDependency(ShadowEngine::Resource &dependent) {
      dependent.callback.unbind<&Resource::stateChanged>(this);
      if (dependent.isEmpty()) --emptyDependencies;
      if (dependent.isFailure()) --failedDependencies;

      checkState();
  }

  uint32_t Resource::decreaseReferences() {
      --references;
      if (references == 0 && manager.unloadEnabled)
          performUnload();

      return references;
  }

  void Resource::stateChanged(ShadowEngine::Resource::State old, ShadowEngine::Resource::State newState,
                              ShadowEngine::Resource &) {
      if (old == State::EMPTY) --emptyDependencies;
      if (old == State::FAILED) --failedDependencies;

      if (newState == State::EMPTY) ++emptyDependencies;
      if (newState == State::FAILED) ++failedDependencies;

      checkState();
  }

  const ResourceType PrefabResource::TYPE("prefab");

  PrefabResource::PrefabResource(const ShadowEngine::Path &path,
                                 ShadowEngine::ResourceTypeManager &resource_manager) : Resource(path, resource_manager) {}

  ResourceType PrefabResource::getType() const { return TYPE; }

  void PrefabResource::unload() { data.clear(); }

  bool PrefabResource::load(size_t size, const uint8_t *mem) {
      data.resize(size);
      memcpy(data.dataMut(), mem, size);
      hash = StableHash(mem, size);
      return true;
  }
}