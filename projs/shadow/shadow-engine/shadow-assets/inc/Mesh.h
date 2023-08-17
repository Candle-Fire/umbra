#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "vlkx/render/Geometry.h"

namespace ShadowEngine::Assets {
    //Mesh with verts for a cube
    class Mesh {
      public:
        Mesh() {};

        //The vertices of the mesh
        std::vector<Geo::VertexAll> vertices;
        //The indices of the mesh
        std::vector<uint32_t> indices;
    };
}