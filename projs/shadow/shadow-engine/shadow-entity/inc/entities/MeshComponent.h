#pragma once

#include "graph/graph.h"

#include "Mesh.h"
#include "temp/model/Builder.h"

namespace ShadowEngine::Entities::Builtin {
    //A component that holds a mesh reference
    // TODO: Replace with vlkx::model
    class API MeshComponent : public ShadowEngine::Entities::Component {
      SHObject_Base(MeshComponent)

    public:
        explicit MeshComponent(std::shared_ptr<ShadowEngine::Assets::Mesh>& mesh) : Component(), mesh(mesh) {}

        // Temporary!
        explicit MeshComponent(std::shared_ptr<vlkxtemp::Model>& model, std::shared_ptr<vlkx::PushConstant>& push) : Component(), model(model), transform_constant(push) {}
        bool isMesh; // if false, is model
        struct Transformation {
            alignas(sizeof(glm::mat4)) glm::mat4 proj_view_model;
        };
        std::shared_ptr<vlkx::PushConstant> transform_constant;
        std::shared_ptr<vlkxtemp::Model> model;

      private:
        std::shared_ptr<ShadowEngine::Assets::Mesh> mesh;

    };

}