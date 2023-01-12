#include "../inc/EntitySystem.h"
#include "core/Time.h"
//#include <ShadowTime.h>

namespace ShadowEngine::EntitySystem {

    SHObject_Base_Impl(EntitySystem)

	EntitySystem::EntitySystem() : activeScene(std::make_unique<Scene>())
	{
		entityMgr = new EntityManager();
	}

	void EntitySystem::Init()
	{
		
	}

	void EntitySystem::Update(int frame)
	{
		auto dt = Time::deltaTime_ms;

		entityMgr->UpdateEntities(dt);
	}

	std::unique_ptr<Scene>& EntitySystem::GetActiveScene()
	{
		return activeScene;
	}

	void EntitySystem::LoadEmptyScene()
	{
		if (activeScene != NULL)
		{
			//SH_CORE_CRITICAL("Scene wasn't unloaded.");
		}

		//activeScene = std::make_unique<ShadowScene>();
	}

	void EntitySystem::LoadScene(Scene* scene)
	{
		if (activeScene != NULL)
		{
			//SH_CORE_CRITICAL("Scene wasn't unloaded.");
		}

		activeScene.reset(scene);

		scene->Init();

		entityMgr->InitEntities();

		//SH_CORE_TRACE("SceneLoaded");
	}

}