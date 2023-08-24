#pragma once

#include "graph/graph.h"

#include "Mesh.h"

namespace ShadowEngine::Entities::Builtin {

    //A component that holds a mesh reference
    class API MeshComponent : public ShadowEngine::Entities::Component {
      SHObject_Base(MeshComponent)
      public:
        MeshComponent() : Component() {}

      private:
        std::shared_ptr<ShadowEngine::Assets::Mesh> mesh;
    };

}