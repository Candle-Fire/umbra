#pragma once

#include "core/Module.h"
#include "Scene.h"

//Holds the reference to the active scene

namespace ShadowEngine::EntitySystem {

	/// <summary>
	/// The module that manages all the Entities and Scenes
	/// </summary>
	class EntitySystem : public ShadowEngine::Module
	{
		SHObject_Base(EntitySystem)
	private:
		/// <summary>
		/// The main reference to the EntityManager
		/// </summary>
		EntityManager* entityMgr;

		/// <summary>
		/// Reference to the active scene
		/// </summary>
		std::unique_ptr<Scene> activeScene;


	public:
		EntitySystem();

		std::string GetName() override { return "EntitySystem"; };

		void PreInit() override {};
		void Init() override;
		void Update() override;

		void Render() override {};
		void LateRender() override {};

		void LoadEmptyScene();
		void LoadScene(Scene* scene);

		ShadowEngine::Scope<Scene>& GetActiveScene();
	};

}