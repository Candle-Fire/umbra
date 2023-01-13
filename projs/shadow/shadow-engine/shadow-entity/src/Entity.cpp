#include <iostream>
#include "../inc/Entity.h"
#include "../inc/EntityRegistry.h"
#include "../inc/Scene.h"


namespace ShadowEngine::Entities
{
    SHObject_Base_Impl(Entity)


	EntityFlags Entity::GetFlags()
	{
		return EntityFlags::NONE;
	}

	ShadowEntity::Transform* Entity::GetTransform()
	{
		if (parent) {
			return parent->GetTransform();
		}
		else {
			return this->scene->GetCenter();
		}
		
	}

	void Entity::SetScene(Scene* se)
	{
		this->scene = se;
	}

	void Entity::SetParent(rtm_ptr<Entity> e)
	{
		this->parent = e;
	}

	Entity::Entity()
	{

	}

	Entity::Entity(Scene* scene)
	{
		this->scene = scene;
	}

	Entity* Entity::Create(Scene* scene)
	{
		throw "The base entity cannot be instantiated";
	}

	std::ostream& operator<<(std::ostream& os, const Entity& dt)
	{
		os << dt.GetType();
		return os;
	}
}
