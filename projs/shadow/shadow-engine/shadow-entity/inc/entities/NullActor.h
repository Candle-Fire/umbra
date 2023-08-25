#pragma once

#include "graph/graph.h"

namespace SH::Entities::Builtin {

    //Basic NullActor inherited from Actor
    class API NullActor : public Actor {
      SHObject_Base(NullActor)

      public:
        //Empty Build function
        void Build() override {};
    };

}
