#include "../inc/EntitySystem.h"
#include "core/Time.h"
#include "debug/AllocationDebugger.h"
#include "editor/HierarchyWindow.h"
#include "entities/NullActor.h"
#include "entities/Position.h"

#include "core/module-manager-v2.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(EntitySystem)

    MODULE_ENTRY(ShadowEngine::Entities::EntitySystem, EntitySystem)

    void EntitySystem::OverlayRender(SH::Events::OverlayRender &) {
        ShadowEngine::Entities::Debugger::AllocationDebugger::Draw();
        ShadowEngine::Entities::Editor::HierarchyWindow::Draw();
    }

    EntitySystem::EntitySystem() {
        //AddChild a new scene to the world
        auto scene = world.AddScene<Scene>({"Generated Scene"});

        //Add 100 NullActors to the scene with a Position component on each
        for (int i = 0; i < 100; i++) {
            auto child = scene->Add<Builtin::NullActor>({});
            child->SetName("NullActor " + std::to_string(i));
            child->Add<Builtin::Position>({10.0f * i, 10, 10});
        }

        world.system<Builtin::Position>()->forEach([](auto &pos) {
            pos.y += 1.0f;
        });

    }

    EntitySystem::~EntitySystem() {

    }

    void EntitySystem::Init() {
        ShadowEngine::ShadowApplication::Get().GetEventBus()
            .subscribe(this, &EntitySystem::OverlayRender);
    }

    void EntitySystem::Update(int frame) {
        this->world.Step();
    }

}