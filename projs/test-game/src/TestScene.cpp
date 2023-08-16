#include "../inc/TestScene.h"
#include "entities/Player.h"
#include "entities/NullActor.h"
#include "entities/Position.h"
#include "entities/MeshComponent.h"
#include "entities/Light.h"
#include "vlkx/render/Geometry.h"

SHObject_Base_Impl(TestScene)

void TestScene::Build() {
    using ShadowEngine::Assets::Mesh;
    //Add 2 nodes to the scene that have a position and mesh components on them
    Mesh cubeMesh;
    Geo::Mesh::setCubeData(cubeMesh.vertices, cubeMesh.indices);
    auto meshData = std::make_shared<Mesh>(cubeMesh);

    using namespace ShadowEngine::Entities::Builtin;
    auto cube1 = this->Add<NullActor>({});
    cube1->SetName("Cube 1");
    cube1->Add<Position>({-5, 0, 0});
    cube1->Add<MeshComponent>(MeshComponent { meshData });

    auto cube2 = this->Add<NullActor>({});
    cube2->SetName("Cube 2");
    cube2->Add<Position>({5, 0, 0});
    cube2->Add<MeshComponent>(MeshComponent { meshData });

    //Add a light to the center of the scene
    auto light = this->Add<NullActor>({});
    light->SetName("Light");
    light->Add<Position>({0, 0, 0});
    light->Add<Light>({});

}