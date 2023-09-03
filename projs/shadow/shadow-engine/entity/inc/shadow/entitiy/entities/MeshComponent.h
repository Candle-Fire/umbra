#pragma once

#include "shadow/entitiy/graph/nodes.h"

#include "shadow/assets/Mesh.h"

namespace SH::Entities::Builtin {

  //A component that holds a mesh reference
  class API MeshComponent : public SH::Entities::Component {
  SHObject_Base(MeshComponent)
  public:
    MeshComponent() : Component() {}

  private:
    std::shared_ptr<SH::Assets::Mesh> mesh;
  };

}