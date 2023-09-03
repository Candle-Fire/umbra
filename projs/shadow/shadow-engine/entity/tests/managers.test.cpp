#include "catch2/catch.hpp"

#include "shadow/entitiy/graph/managers.h"
#include "shadow/entitiy/entities/NullActor.h"

TEST_CASE("NodeManager - get_component") {
// Set up the test environment

    SECTION("Getting a non-existent component should return nullptr") {
        auto manager = SH::Entities::NodeManager();
        manager.AddNode(SH::Entities::Builtin::NullActor{});

    }
}