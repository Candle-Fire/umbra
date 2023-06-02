#include "../inc/TestScene.h"
#include "entities/Player.h"
#include "entities/NullActor.h"
#include "entities/Position.h"
#include "entities/MeshComponent.h"
#include "entities/Light.h"

SHObject_Base_Impl(TestScene)

void TestScene::Build() {
    //Add 2 nodes to the scene that have a position and mesh components on them
    auto cube1 = this->Add<ShadowEngine::Entities::Builtin::NullActor>({});
    cube1->SetName("Cube 1");
    cube1->Add<ShadowEngine::Entities::Builtin::Position>({-5, 0, 0});
    cube1->Add<ShadowEngine::Entities::Builtin::MeshComponent>({});

    auto cube2 = this->Add<ShadowEngine::Entities::Builtin::NullActor>({});
    cube2->SetName("Cube 2");
    cube2->Add<ShadowEngine::Entities::Builtin::Position>({5, 0, 0});
    cube2->Add<ShadowEngine::Entities::Builtin::MeshComponent>({});

    //Add a light to the center of the scene
    auto light = this->Add<ShadowEngine::Entities::Builtin::NullActor>({});
    light->SetName("Light");
    light->Add<ShadowEngine::Entities::Builtin::Position>({0, 0, 0});
    light->Add<ShadowEngine::Entities::Builtin::Light>({});

    auto player = this->Add<test_game::Player>({"Test player"});

}