#pragma once
#include <memory>
#include <list>

#include "SHObject.h"
#include "Entity.h"
#include "EntityManager.h"
#include "TransformEntity.h"
//#include "ShadowMath/Transform.h"

class Camera;

namespace ShadowEngine::Entities {

	/// <summary>
	/// Represents a scene, that is composed of entities.
	/// 
	/// </summary>
	class Scene : public TransformEntity
	{
		SHObject_Base(Scene)

        Entity_Base_NoCtor(TransformEntity, Entity);

        Entity::HierarchyList staticHierarchy;

	public:
		Scene() {
            this->name = "Scene";
		}
		virtual ~Scene() = default;



/*
		template<class T>
		void DestroyEntity(rtm_ptr<T>& entity) {
			EntityManager::Instance->RemoveEntity<T>(entity);
			m_entities.remove(entity);
		}

		void DestroyScene() {
			for (auto var : m_entities)
			{
				EntityManager::Instance->RemoveEntity(var);
			}
		}
*/

	};

}
