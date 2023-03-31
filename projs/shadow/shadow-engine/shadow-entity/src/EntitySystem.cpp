#include "../inc/EntitySystem.h"
#include "core/Time.h"
#include "debug/AllocationDebugger.h"
#include "editor/HierarchyWindow.h"
#include "entities/NullActor.h"
#include "entities/Position.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(EntitySystem)

    void EntitySystem::OverlayRender() {
        //ShadowEngine::Entities::Debugger::AllocationDebugger::Draw();
        ShadowEngine::Entities::Editor::HierarchyWindow::Draw();
    }

    EntitySystem::EntitySystem() {
        //Create the root node


        //AddChild a new scene to the root
        auto scene = root.GetManager().ConstructNode<Scene>();
        root.AddScene(scene);

        //AddChild 10 NullActors to the scene
        for (int i = 0; i < 100; i++) {
            auto child = scene->Add<Builtin::NullActor>({});
            child->SetName("NullActor " + std::to_string(i));
        }

        //AddChild 10 Entities to the scene with a Position component on each
        for (int i = 0; i < 100; i++) {
            auto child = scene->Add<Builtin::NullActor>({});
            child->SetName("NullActor " + std::to_string(i));
            child->Add<Builtin::Position>({10.0f * i, 10, 10});
        }

    }

    EntitySystem::~EntitySystem() {

    }

    void EntitySystem::Init() {

    }

    void EntitySystem::Update(int frame) {

    }

}