#pragma once

#include "shadow/entitiy/graph/graph.h"

#include "shadow/renderer/temp/model/Builder.h"
#include "Mesh.h"

namespace SH::Entities::Builtin {
  //A component that holds a mesh reference
  // TODO: Replace with vlkx::model
  class MeshComponent : public SH::Entities::Component {
  SHObject_Base(MeshComponent)

  public:
    explicit MeshComponent(std::shared_ptr<SH::Assets::Mesh>& mesh) : Component(), mesh(mesh) { isMesh = true; }

    // Temporary!
    explicit MeshComponent(std::shared_ptr<vlkxtemp::Model>& model, std::shared_ptr<vlkx::PushConstant>& push) : Component(), model(model), transform_constant(push) { isMesh = false; }
    bool isMesh; // if false, is model
    struct Transformation {
      alignas(sizeof(glm::mat4)) glm::mat4 proj_view_model;
    };
    std::shared_ptr<vlkx::PushConstant> transform_constant;
    std::shared_ptr<vlkxtemp::Model> model;

  private:
    std::shared_ptr<SH::Assets::Mesh> mesh;

  };

}