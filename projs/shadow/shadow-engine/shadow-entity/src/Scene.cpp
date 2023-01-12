#include "../inc/Scene.h"

namespace ShadowEngine::EntitySystem {

    SHObject_Base_Impl(Scene)

	void EntitySystem::Scene::Start()
	{
		for (auto& entity : m_entities)
		{
			//entity->Start();
		}
	}

	void Scene::Update()
	{
		for (auto& entity : m_entities)
		{
			//entity->Update(dt);
		}
	}

	void Scene::Init() {

		for (auto& entity : m_entities)
		{
			//entity->Init();
		}
	}
}