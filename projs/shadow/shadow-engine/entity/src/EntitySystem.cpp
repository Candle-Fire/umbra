#include "../inc/shadow/entitiy/EntitySystem.h"
#include "shadow/core/Time.h"
#include "shadow/entitiy/debug/AllocationDebugger.h"
#include "shadow/entitiy/editor/HierarchyWindow.h"
#include "shadow/entitiy/entities/NullActor.h"
#include "shadow/entitiy/entities/Position.h"

#include "shadow/core/module-manager-v2.h"

namespace SH::Entities {

    SHObject_Base_Impl(EntitySystem)

    MODULE_ENTRY(SH::Entities::EntitySystem, EntitySystem)

    void EntitySystem::OverlayRender(SH::Events::OverlayRender &) {
        SH::Entities::Debugger::AllocationDebugger::Draw();
        SH::Entities::Editor::HierarchyWindow::Draw();
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
        SH::ShadowApplication::Get().GetEventBus()
            .subscribe(this, &EntitySystem::OverlayRender);
    }

    void EntitySystem::Update(int frame) {
        this->world.Step();
    }

}