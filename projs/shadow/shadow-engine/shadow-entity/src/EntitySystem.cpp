#include "../inc/EntitySystem.h"
#include "core/Time.h"
#include "debug/AllocationDebugger.h"
#include "editor/HierarchyWindow.h"
#include "entities/NullActor.h"
#include "entities/Position.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(EntitySystem)

    void EntitySystem::OverlayRender() {
        ShadowEngine::Entities::Debugger::AllocationDebugger::Draw();
        ShadowEngine::Entities::Editor::HierarchyWindow::Draw();
    }

    EntitySystem::EntitySystem() {
        //AddChild a new scene to the world
        auto scene = world.AddScene<Scene>({"Generated Scene"});

        //Add 100 NullActors to the scene with a Position component on each
        for (int i = 0; i < 1000000; i++) {
            auto child = scene->Add<Builtin::NullActor>({});
            child->SetName("NullActor " + std::to_string(i));
            child->Add<Builtin::Position>({10.0f * i, 10, 10});
        }

        world.system<Builtin::Position>()->forEach([](auto &pos) {
            pos.y += 1.0f;
        });

        world.system<Builtin::Position>()->forEach([](auto &pos) {
            if (pos.y > 100)
                pos.GetParent()->Destroy();
        });

    }

    EntitySystem::~EntitySystem() {

    }

    void EntitySystem::Init() {

    }

    void EntitySystem::Update(int frame) {
        this->world.Step();
    }

}