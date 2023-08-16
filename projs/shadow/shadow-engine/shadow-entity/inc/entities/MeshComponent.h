#pragma once

#include "graph/graph.h"

#include "Mesh.h"

namespace ShadowEngine::Entities::Builtin {

    //A component that holds a mesh reference
    // TODO: Replace with vlkx::model
    class API MeshComponent : public ShadowEngine::Entities::Component {
      SHObject_Base(MeshComponent)
      public:
        explicit MeshComponent(std::shared_ptr<ShadowEngine::Assets::Mesh>& mesh) : Component(), mesh(mesh) {}

      private:
        std::shared_ptr<ShadowEngine::Assets::Mesh> mesh;
    };

}