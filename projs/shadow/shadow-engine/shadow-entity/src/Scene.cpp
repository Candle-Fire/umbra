#include "../inc/Scene.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(Scene)

	void Scene::Start()
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