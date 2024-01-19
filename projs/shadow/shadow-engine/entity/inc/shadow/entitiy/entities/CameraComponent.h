#pragma once

namespace ShadowEngine::Entities::Builtin {

  class CameraComponent : public SH::Entities::Component {
    SHObject_Base(CameraComponent)
  public:
    CameraComponent() : Component() {}
  };
}