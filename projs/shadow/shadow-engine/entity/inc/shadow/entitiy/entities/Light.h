#pragma once

#include <glm/glm.hpp>

#include "shadow/entitiy/graph/graph.h"

//A light component in the builtin namespace with light color, type, and intensity
namespace SH::Entities::Builtin {

    //enum of light types
    enum class LightType {
        Directional,
        Point,
        Spot
    };

    class API Light : public SH::Entities::Component {
      SHObject_Base(Light)
      public:
        Light() : Component() {}

      private:
        glm::vec3 color;
        float intensity;
        LightType type;
    };

}