#include "../inc/TransformEntity.h"
#include "Scene.h"

namespace ShadowEngine::Entities {

    SHObject_Base_Impl(TransformEntity)

	void TransformEntity::SetParent(rtm_ptr<Entity> e)
	{
        Entity::SetParent(e);
	}

	ShadowEntity::Transform* TransformEntity::GetTransform()
	{
		return &this->transform;
	}


	 void TransformEntity::UpdateWorldTransform()
     {
		if (this->parent)
		{
			this->w_transform = (this->transform * *parent->GetTransform());
		}
		else
		{
			if (scene != nullptr)
                this->w_transform = this->transform * *scene->GetTransform();

            this->w_transform = this->transform;
		}
	}

	void TransformEntity::SetPosition(glm::vec3 location)
	{
		if (transform.GetPosition() != location) {
			this->transform.SetPosition(location);
			UpdateWorldTransform();
			TransformUpdated();
		}
	}

	void TransformEntity::SetRotation(glm::vec3 rotation)
	{
		if (transform.GetEulerRotation() != rotation) {
			this->transform.SetEulerRotation(rotation);
			UpdateWorldTransform();
			TransformUpdated();
		}
	}
	/*
	void TransformEntity::RotateBy(glm::vec3 rotation)
	{
		this->transform.RotateByEulerRotation(rotation);
		this->w_transform = UpdateWorldTransform(this->transform);
		TransformUpdated();

	}
	*/
	void TransformEntity::SetScale(glm::vec3 scale)
	{
		if (transform.GetScale() != scale) {
			this->transform.SetScale(scale);
			UpdateWorldTransform();
			TransformUpdated();
		}
	}

	void TransformEntity::SetRelativeTransform(const ShadowEntity::Transform& NewTransform)
	{
		this->transform = NewTransform;
		UpdateWorldTransform();
		TransformUpdated();
	}

	glm::vec3 TransformEntity::GetPosition()
	{
		return this->transform.GetPosition();
	}

	glm::vec3 TransformEntity::GetRotation()
	{
		return this->transform.GetEulerRotation();
	}

	glm::vec3 TransformEntity::GetScale()
	{
		return this->transform.GetScale();
	}

	glm::mat4 TransformEntity::GetWorldTransformMatrix()
	{
		return w_transform.GetTransformMatrix();
	}

	void TransformEntity::TransformUpdated()
	{
		this->TransformChanged();

		//This transform has changed, so we need to update the children about it
		for (auto & child : hierarchy)
		{
			child->ParentTransformUpdated();
		}

		for (auto & child : internalHierarchy)
		{
			child->ParentTransformUpdated();
		}
	}

	void TransformEntity::ParentTransformUpdated()
	{
		//Re-calculate the transform, this always uses the parent or world.
		UpdateWorldTransform();

		TransformUpdated();
	}
}
