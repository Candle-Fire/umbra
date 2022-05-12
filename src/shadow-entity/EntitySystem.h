#pragma once

//#include "ShadowModules/ShadowModule.h"
#include "Scene.h"

//Holds the reference to the active scene

namespace ShadowEngine::EntitySystem {

	/// <summary>
	/// The module that manages all the Entities and Scenes
	/// </summary>
	class EntitySystem : public SHObject /*: public ShadowModule*/
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
		//TODO: should be Scope from the renaming


	public:
		EntitySystem();

		std::string GetName() /* override */ { return "EntitySystem"; };

		void PreInit() /* override */ {};
		void Init() /* override */;
		void Update() /* override */;

		void Render() /* override */ {};
		void LateRender() /* override */ {};

		void LoadEmptyScene();
		void LoadScene(Scene* scene);

		std::unique_ptr<Scene>& GetActiveScene();
	};

}