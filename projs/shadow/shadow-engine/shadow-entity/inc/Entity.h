#pragma once

#include <string>
#include <iostream>
#include <vector>

#include "EntityBase.h"
#include "SHObject.h"
#include "EntityManager.h"
#include "Transform.h"


namespace ShadowEngine::Entities
{
    struct SystemCallbacks;
    class Scene;

    using Rtm_UUID = int;


    /**
     * Runtime pointer to an Entity
     * It tracks the UUID of the linked entity
     * @tparam Type
     */
	template<class Type>
	class rtm_ptr
	{
	private:
		Type* m_ptr;

        Rtm_UUID m_uid;

	public:
		rtm_ptr(Type* ptr) : m_ptr(ptr), m_uid(ptr->m_runtimeUID) {}

		rtm_ptr() : m_ptr(nullptr) {}
		
		template<class T>
		explicit rtm_ptr(const rtm_ptr<T>& o) {
			m_ptr = (Type*)o.getInternalPointer();
			m_uid = o.getInternalUID();
		}
		
		Type* operator->() const
		{
			if (m_ptr->m_runtimeUID != m_uid) {
				assert(m_ptr->m_runtimeUID == m_uid);
				return nullptr;
			}
			return ((Type*)m_ptr);
		}

        bool isValid() const { return m_ptr != nullptr && m_ptr->m_runtimeUID == m_uid; }

		inline operator bool() const { return this->isValid(); }

		template<class T>
		inline bool operator ==(rtm_ptr<T> o) const {
			return m_ptr == o.m_ptr &&
					m_uid == o.m_uid; 
		}

		template<class T>
		inline operator rtm_ptr<T>() const {
			return rtm_ptr<T>(m_ptr);
		}

		void setNull() {
			m_ptr = nullptr;
			m_uid = -1;
		}

		void* getInternalPointer() const{
			return (void*)m_ptr; 
		}

		int getInternalUID() const { return m_uid; }

	};

    /**
     *  This is the base entity used in the game world. This should never be instantiated only the derived classes
     *  A entity has a parent and internal, external hierarchy. The parent is the Entity that this is under. This effects the position of this Entity.
     *  The internal hierarchy is used for entities that are considered as part of this Entity these would be like components in Unity or UE4
     *  This base Entity does not have a position so it acts like a Component in other engines. If you want to have a position you have to use <see cref="TransformEntity"/>
     *  The base Entity does however does have a <see cref="Entity::GetTransform"/> that returns the position of the first parent's position that is <see cref="TransformEntity"/>
     *  <summary>
     *  Base Entity in the scene
     *  </summary>
     */
	class Entity : public SHObject
	{
		SHObject_Base(Entity)

	public:

        /**
         * <summary>
         * This is the Globally unique ID of this Entity
         * </summary>
         * This ID will be only assigned to this Entity instance
         * It can be used to look up entities, but it is not recommended as it is a slow process
         * For Entity Lookup use the m_runtimeIndex
         */
        Rtm_UUID m_runtimeUID;

		/**
		 * <summary>
		 * This is the UID that represents an invalid ID
		 * </summary>
		 */
		static const Rtm_UUID INVALID_UID = -1;

        /**
         * <summary>
         * This is the ID of the entity usable for lookup
         * </summary>
         * This ID will be reused when the Entity is removed
         * To use this Entity for lookup use the LUT in the EntityManager
         */
		int m_runtimeIndex;

		/**
		 * The name of this entity
		 */
		std::string name;

        /**
         * The scene this Entity is assigned to
         */
		Scene* scene;

        using HierarchyList = std::vector<rtm_ptr<Entity>>;

		/**
		 * The internal hierarchy of this Entity
		 */
        HierarchyList internalHierarchy;

		/**
		 * The external hierarchy of this Entity
		 */
        HierarchyList hierarchy;

		/**
		 * The parent entity
		 *
		 * This can be null when the Entity is the top one in the hierarchy
		 */
		rtm_ptr<Entity> parent;


        /**
         * Empty default constructor
         */
		Entity();

        /**
         * Constructor defining the scene it is located in
         * @param scene The scene that this entity is in
         */
		Entity(Scene* scene);

		virtual ~Entity() {}

        /**
         * Creates a new Entity
         * @param scene The scene that this entity is in
         * @return Pointer to the Entity created
         */
		virtual Entity* Create(Scene* scene);

		virtual void Build() {};

		virtual ShadowEntity::Transform* GetTransform();

		//Internal call
		virtual void ParentTransformUpdated()
		{
			TransformChanged();
		};

		//Override for custom behaviour on transform change
		virtual void TransformChanged() {};

        /**
         * Sets the scene this Entity is in
         * @param se The scene this entity belongs to
         */
		void SetScene(Scene* se);

		virtual void SetParent(rtm_ptr<Entity> e);


		template<class T, class ...ARGS>
		rtm_ptr<T> AddChildEntity(ARGS&&... args) {
			rtm_ptr<T> ptr = EntityManager::Instance->AddEntity<T>(std::forward<ARGS>(args)...);
			ptr->scene = scene;
			hierarchy.push_back(ptr);
			ptr->SetParent(this);

			return ptr;
		}

		template<class T, class ...ARGS>
		rtm_ptr<T> AddInternalChildEntity(ARGS&&... args) {
			rtm_ptr<T> ptr = EntityManager::Instance->AddEntity<T>(std::forward<ARGS>(args)...);
			ptr->scene = scene;
			internalHierarchy.push_back(ptr);
			ptr->SetParent(this);

			return ptr;
		}

	public:
		friend std::ostream& operator<<(std::ostream& os, const Entity& dt);

	public:
		template<class T>
		static void UpdateEntities(EntityManager* mgr, float dt)
		{
			EntityContainer<T>& container = *mgr->GetContainerByType<T>();

			int c = 0;

			for (auto & current : container)
			{
				current.Update(dt);
			}
		}

		template<class T>
		static void InitEntities(EntityManager* mgr)
		{
			auto& container = *mgr->GetContainerByType<T>();

			int c = 0;

			for (auto & current : container)
			{
				current.Init();
			}
		}

		template<class T>
		static void RegisterDefaultUpdate(EntityManager& mgr) {
			SystemCallbacks s;
			s.update = &UpdateEntities<T>;
			s.init = &InitEntities<T>;

			mgr.AddSystem(s);
		}
	};

    template<typename T>
    concept IsEntity = std::is_base_of<Entity,T>::value;
}
