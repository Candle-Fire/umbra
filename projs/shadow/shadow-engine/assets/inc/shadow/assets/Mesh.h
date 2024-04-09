#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace SH::Assets {
    //Mesh with verts for a cube
    class Mesh {
      public:
        Mesh();

        //The vertices of the mesh
        std::vector<glm::vec3> vertices;
        //The indices of the mesh
        std::vector<uint32_t> indices;
    };
}