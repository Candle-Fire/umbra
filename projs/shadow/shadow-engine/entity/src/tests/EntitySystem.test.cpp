
#include "shadow/entity/EntitySystem.h"
/*
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include "entities/NullActor.h"
#include "entities/Position.h"

TEST_CASE("EntitySystem", "[EntitySystem]") {

    // Start a new entity system
    auto entitySystem = new ShadowEngine::Entities::EntitySystem();
    // Add test scene
    auto scene = entitySystem->GetWorld().AddScene<ShadowEngine::Entities::Scene>({"Generated Scene"});


    // now let's benchmark:
    BENCHMARK("New Entity 100") {
                                    for (int i = 0; i < 100; i++) {
                                        auto child = scene->Add<ShadowEngine::Entities::Builtin::NullActor>({});
                                        child->SetName("NullActor " + std::to_string(i));
                                        child->Add<ShadowEngine::Entities::Builtin::Position>({10.0f * i, 10, 10});
                                    }
                                };

    BENCHMARK("New Entity 1000") {
                                     for (int i = 0; i < 1000; i++) {
                                         auto child = scene->Add<ShadowEngine::Entities::Builtin::NullActor>({});
                                         child->SetName("NullActor " + std::to_string(i));
                                         child->Add<ShadowEngine::Entities::Builtin::Position>({10.0f * i, 10, 10});
                                     }
                                 };
}

*/