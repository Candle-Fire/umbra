#include "../inc/EntitySystem.h"
#include "core/Time.h"
#include "debug/AllocationDebugger.h"
#include "editor/HierarchyWindow.h"
#include "entities/NullActor.h"
//#include <ShadowTime.h>

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(EntitySystem)

    void EntitySystem::OverlayRender() {
        //ShadowEngine::Entities::Debugger::AllocationDebugger::Draw();
        ShadowEngine::Entities::Editor::HierarchyWindow::Draw();
    }

    EntitySystem::EntitySystem() {
        //Create the root node
        root = nodeManager.ConstructNode<RootNode>();

        //Add a scene to the root
        auto scene = nodeManager.ConstructNode<Scene>();
        root->AddScene(scene);
        //Add 10 NullActors to the scene
        for (int i = 0; i < 10; i++) {
            scene->AddChild(nodeManager.TakeNode(Builtin::NullActor()));

            //auto actor = nodeManager.ConstructNode<Builtin::NullActor>();
            //scene->AddChild(actor);
        }

    }

    EntitySystem::~EntitySystem() {

    }

    void EntitySystem::Init() {

    }

    void EntitySystem::Update(int frame) {

    }

}