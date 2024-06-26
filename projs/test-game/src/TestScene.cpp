#include "../inc/TestScene.h"
#include "entities/Player.h"
#include "shadow/entitiy/entities/NullActor.h"
#include "shadow/entitiy/entities/Position.h"
#include "shadow/entitiy/entities/MeshComponent.h"
#include "shadow/entitiy/entities/Light.h"

SHObject_Base_Impl(TestScene)

void TestScene::Build() {
    //Add 2 nodes to the scene that have a position and mesh components on them
    auto cube1 = this->Add<SH::Entities::Builtin::NullActor>({});
    cube1->SetName("Cube 1");
    cube1->Add<SH::Entities::Builtin::Position>({-5, 0, 0});
    cube1->Add<SH::Entities::Builtin::MeshComponent>({});

    auto cube2 = this->Add<SH::Entities::Builtin::NullActor>({});
    cube2->SetName("Cube 2");
    cube2->Add<SH::Entities::Builtin::Position>({5, 0, 0});
    cube2->Add<SH::Entities::Builtin::MeshComponent>({});

    //Add a light to the center of the scene
    auto light = this->Add<SH::Entities::Builtin::NullActor>({});
    light->SetName("Light");
    light->Add<SH::Entities::Builtin::Position>({0, 0, 0});
    light->Add<SH::Entities::Builtin::Light>({});

}