#pragma once

#include "graph/graph.h"

namespace ShadowEngine::Entities::Builtin {

    //Basic NullActor inherited from Actor
    class NullActor : public Actor {
      SHObject_Base(NullActor);

        //Empty Build function
        void Build() override {};
    };

} // Builtin
