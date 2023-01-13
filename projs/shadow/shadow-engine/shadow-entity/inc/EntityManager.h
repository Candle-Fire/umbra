#pragma once
#include <unordered_map>
#include <cassert>
#include <vector>
#include "EntityContainer.h"
//#include "Entity.h"

namespace ShadowEngine::Entities {

    template<class T>
    struct rtm_ptr;

    class Entity;
	class EntityManager;
	
	struct SystemCallbacks {
		typedef void (*Callback)(EntityManager* mgr);
		typedef void (*UpdateCallback)(EntityManager* mgr, float dt);

		UpdateCallback update;
		Callback init;
	};


	/**
	 * \brief Manages the memory and IDs of Entities
	 */
	class EntityManager {

	public:
		
		/**
		 * \brief Singleton instance
		 */
		static EntityManager* Instance;

	private:
		//Map the runtime index of the entity to the container
		using EntityContainerRegistry = std::unordered_map<int, IEntityContainer*>;

		/**
		 * \brief Map of the Entity Containers mapped to the entity type ID
		 */
		EntityContainerRegistry m_EntityContainerRegistry;


		using EntityLookupTable = std::vector<Entity*>;

		/**
		 * This table is used to get a Entity by it's runtime Index.
		 * It is a fast lookup, but it is not unique.
		 * If a Entity is freed up it's index will be given out to another Entity of the same type
		 *
		 * To use the table simply access the element at the index of the Entity.
		 *
		 * \brief Quick access Look Up Table of active Entities
		 *
		 */
		EntityLookupTable	m_EntityLUT;
		
		//Extra number of spaces to allocate in the LUT
		const int ENTITY_LUT_GROW = 2048;
		int LUTNextFree = 0;
		bool LUTFragm = false;
		std::vector<int> LUTFragmFree;

		//using SystemUpdate = std::function<void(EntityManager*)>;
		std::vector<SystemCallbacks> systems;

		/**
		 * \brief The next assignable Unique ID
		 */
		int nextUID = 0;

		/**
		 * \brief Returns the correct container for the entity type
		 * \tparam T The type of the entity
		 * \return The entity container accosted with this type
		 */
		template<class T> requires std::is_base_of<Entity,T>::value
		inline EntityContainer<T>* GetComponentContainer()
		{
			int CID = T::TypeId();

			auto it = this->m_EntityContainerRegistry.find(CID);
			EntityContainer<T>* cc = nullptr;

			if (it == this->m_EntityContainerRegistry.end())
			{
				cc = new EntityContainer<T>();
				this->m_EntityContainerRegistry[CID] = cc;

                //TODO: Why do we need the template word there now?
				T::template RegisterDefaultUpdate<T>(*this);
			}
			else
				cc = static_cast<EntityContainer<T>*>(it->second);

			assert(cc != nullptr && "Failed to create ComponentContainer<Type>!");
			return cc;
		}

		inline IEntityContainer* GetComponentContainer(int typeID);


		/**
		 * \brief Assigns the next free LUT index to this entity
		 * \param component
		 * \return
		 */
		int AssignIndexToEntity(Entity* component);

		/**
		 * \brief Frees up the given index
		 * \param id
		 */
		void ReleaseIndex(int id);

	public:
		EntityManager();


		/**
		 * \brief Instantiates a new entity
		 * \tparam T Type of the Entity
		 * \tparam ARGS Constructor parameters of the Entity
		 * \param args Constructor parameters of the Entity
		 * \return
		 */
		template<typename T, class ...ARGS>
		rtm_ptr<T> AddEntity(ARGS&&... args)
		{
			//The type ID of the Entity we are trying to add
			const int CTID = T::TypeId();

			// acquire memory for new entity object of type Type
			void* pObjectMemory = GetComponentContainer<T>()->CreateObject();

			//Assign the index and the UID to the object
			int runtimeIndex = this->AssignIndexToEntity((T*)pObjectMemory);
			((T*)pObjectMemory)->m_runtimeIndex = runtimeIndex;
			((T*)pObjectMemory)->m_runtimeUID = nextUID;
			nextUID++;

			// create Entity in place
			Entity* component = new (pObjectMemory)T(std::forward<ARGS>(args)...);

			return rtm_ptr((T*)component);
		}

		void RemoveEntity(int entityIndex, int typeID);

		/**
		 * \brief Removes an entity
		 * \tparam T The type of the entity to remove
		 * \param entityIndex The entity index to be removed
		 */
		template<class T>
		void RemoveEntity(const int entityIndex)
		{
			this->RemoveEntity(entityIndex, T::TypeID());
		}

		template<class T> requires std::is_base_of<Entity,T>::value
		void RemoveEntity(T* entity)
		{
			RemoveEntity<T>(entity->m_runtimeIndex);
		}

		template<class T>
		void RemoveEntity(rtm_ptr<T> entity)
		{
			this->RemoveEntity(entity->m_runtimeIndex, entity->GetTypeId());
		}

		template<class T>
		inline T* GetEntityByIndex(int index)
		{
			return this->m_EntityLUT[index];
		}

		template<class T>
		inline EntityContainer<T>* GetContainerByType() {
			int CID = T::TypeId();

			auto it = this->m_EntityContainerRegistry.find(CID);

			return static_cast<EntityContainer<T>*>(it->second);
		}

		void AddSystem(SystemCallbacks fn);

		void UpdateEntities(float dt);

		void InitEntities();
	};

}