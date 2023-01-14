#pragma once

#include "core/Module.h"
#include "Scene.h"

//Holds the reference to the active scene

namespace ShadowEngine::Entities {

	/**
	 * The module that manages all the Entities and Scenes
	 */
	class EntitySystem : public ShadowEngine::Module
	{
		SHObject_Base(EntitySystem)
	private:

		/**
		 * The main reference to the EntityManager
		 */
		EntityManager* entityMgr;

		/**
		 * Reference to the active scene
		 */
		std::unique_ptr<Scene> activeScene;

	public:
		EntitySystem();
        ~EntitySystem() override;

        std::string GetName() override { return "EntitySystem"; };

		void PreInit() override {};
		void Init() override;
		void Update(int frame) override;

		void Render(VkCommandBuffer& commands, int frame) override {};
		void LateRender(VkCommandBuffer& commands, int frame) override {};

        void Recreate() override {}
        void PreRender() override {}
        void OverlayRender() override {}
        void AfterFrameEnd() override {}
        void Destroy() override {}
        void Event(SDL_Event *e) override {}


        void LoadEmptyScene();
		void LoadScene(Scene* scene);

		std::unique_ptr<Scene>& GetActiveScene();
	};

}