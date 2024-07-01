#pragma once

#include "shadow/entity/graph/graph.h"

namespace SH::Entities::Builtin {

  class CameraComponent : public SH::Entities::Component {
    SHObject_Base(CameraComponent)
  public:
    CameraComponent() : Component() {}
  };
}