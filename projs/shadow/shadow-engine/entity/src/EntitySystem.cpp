#include "shadow/entitiy/EntitySystem.h"
#include "shadow/core/Time.h"
#include "shadow/entitiy/debug/AllocationDebugger.h"
#include "shadow/entitiy/editor/HierarchyWindow.h"
#include "shadow/entitiy/entities/NullActor.h"
#include "shadow/entitiy/entities/Position.h"

#include "shadow/core/module-manager-v2.h"
#include "shadow/event-bus/render_events.h"
#include "shadow/entitiy/entities/MeshComponent.h"

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
      SH::ShadowApplication::Get().GetEventBus()
          .subscribe(this, &EntitySystem::Render);
  }

  void EntitySystem::Render(SH::Events::Render &render) {
      spdlog::info("ES render");
      // Collect renderables
      auto container = world.GetManager().GetContainerByType<SH::Entities::Builtin::MeshComponent>();
      if (container == nullptr) return;
      rtm_ptr<NodeBase> positionNode;
      Builtin::MeshComponent* mesh;

      for (auto& it : *container) {
          spdlog::info("found mesh");
          // Try find the sister position first
          auto parent = *it.GetParent().Get();
          for (auto& comp : parent.GetHierarchy()) {
              if (comp.Get()->GetTypeId() == SH::Entities::Builtin::Position::TypeId()) {
                  spdlog::info("found position");
                  positionNode = comp;
                  mesh = &it;

                  if (mesh->isMesh) return;
                  spdlog::info("is model");

                  // Translate by position, submit mesh for render
                  auto pos = dynamic_cast<SH::Entities::Builtin::Position*>(positionNode.Get());
                  *mesh->transform_constant->getData<Builtin::MeshComponent::Transformation>(render.frame) = { glm::translate(glm::identity<glm::mat4>(), { pos->x, pos->y, pos->z } ) };
                  spdlog::info("rendering {}", mesh->GetType());
                  mesh->model->draw(render.buffer, render.frame, 1);
              }
          }
      }

  }

  void EntitySystem::Update(int frame) {
      this->world.Step();
  }

}