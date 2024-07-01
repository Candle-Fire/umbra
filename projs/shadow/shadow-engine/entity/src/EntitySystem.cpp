#include "shadow/entity/EntitySystem.h"
#include "shadow/core/Time.h"
#include "shadow/entity/debug/AllocationDebugger.h"

#include "shadow/core/module-manager-v2.h"

namespace SH::Entities {

  SHObject_Base_Impl(EntitySystem)

  MODULE_ENTRY(SH::Entities::EntitySystem, EntitySystem)


  EntitySystem::EntitySystem() {
      //AddChild a new scene to the world
      auto scene = world.AddScene<Scene>({"Generated Scene"});

  }

  EntitySystem::~EntitySystem() {

  }

  void EntitySystem::Init() {
  }

  void EntitySystem::Update(int frame) {
      this->world.Step();
  }

}