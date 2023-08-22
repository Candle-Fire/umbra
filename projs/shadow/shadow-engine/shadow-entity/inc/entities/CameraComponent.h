#pragma once

namespace ShadowEngine::Entities::Builtin {

    class CameraComponent : public ShadowEngine::Entities::Component {
    SHObject_Base(CameraComponent)
    public:
        CameraComponent() : Component() {}
    };
}