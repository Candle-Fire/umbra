#include "../inc/EntitySystem.h"
#include "core/Time.h"
#include "debug/AllocationDebugger.h"
#include "entities/NullEntity.h"
#include "editor/HierarchyWindow.h"
//#include <ShadowTime.h>

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(EntitySystem)

	EntitySystem::EntitySystem() : entityMgr(new EntityManager()), activeScene()
	{

	}

	void EntitySystem::Init()
	{
		LoadEmptyScene();
        activeScene->AddChildEntity<Builtin::NullEntity>();
	}

	void EntitySystem::Update(int frame)
	{
		auto dt = Time::deltaTime_ms;

		entityMgr->UpdateEntities(dt);
	}

	rtm_ptr<Scene> EntitySystem::GetActiveScene()
	{
		return activeScene;
	}

	void EntitySystem::LoadEmptyScene()
	{
		if (activeScene != NULL)
		{
			//SH_CORE_CRITICAL("Scene wasn't unloaded.");
		}

		activeScene = entityMgr->AddEntity<Scene>();
	}

	void EntitySystem::LoadScene(Scene* scene)
	{
		if (activeScene != NULL)
		{
			//SH_CORE_CRITICAL("Scene wasn't unloaded.");
		}

		//activeScene.reset(scene);

		//scene->Init();

		entityMgr->InitEntities();

		//SH_CORE_TRACE("SceneLoaded");
	}

    EntitySystem::~EntitySystem() {

    }

    void EntitySystem::OverlayRender() {

        ShadowEngine::Entities::Debugger::AllocationDebugger::Draw();
        ShadowEngine::Entities::Editor::HierarchyWindow::Draw();

    }

}