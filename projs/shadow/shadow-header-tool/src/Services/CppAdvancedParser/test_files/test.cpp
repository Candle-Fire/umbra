#pragma once

#include "graph/graph.h"

//Position component in the builtin namespace
namespace ShadowEngine::Entities::Builtin {

    class
    [[using SH: Reflect("asd"), NoFunc()]]
    [[SH::Reflect("asd"), NoFunc()]]
    [[SH::reflect("asd", 1201, "Test_ANEM_FOR"), NoFunc()]]
    [[SH::test]]
    Position : public ShadowEngine::Entities::Component {
            SHReflect(Position);
            SHObject_Base(Position);
            public:
            [[SH::Reflect]]float x = 0;
            [[SH::Reflect]]float y = 0;
            [[SH::Reflect]]float z = 0;

            Position() = default;

            Position(float x_, float y_, float z_)
                : x(x_), y(y_), z(z_) {}

            [[SH::Reflect]] void foo(int a) {
                return;
            }
        };
}
