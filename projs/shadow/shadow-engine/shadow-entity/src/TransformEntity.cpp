#include "../inc/TransformEntity.h"

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


	ShadowEntity::Transform TransformEntity::CalcNewComponentToWorld(
		const ShadowEntity::Transform& NewRelativeTransform) const
	{
		if (this->parent)
		{
			return NewRelativeTransform * *parent->GetTransform();
		}
		else
		{
			if (scene != nullptr)
				return NewRelativeTransform * *scene->GetCenter();

			return NewRelativeTransform;
		}
	}

	void TransformEntity::SetPosition(glm::vec3 location)
	{
		if (transform.GetPosition() != location) {
			this->transform.SetPosition(location);
			this->w_transform = CalcNewComponentToWorld(this->transform);
			TransformUpdated();
		}
	}

	void TransformEntity::SetRotation(glm::vec3 rotation)
	{
		if (transform.GetEulerRotation() != rotation) {
			this->transform.SetEulerRotation(rotation);
			this->w_transform = CalcNewComponentToWorld(this->transform);
			TransformUpdated();
		}
	}
	/*
	void TransformEntity::RotateBy(glm::vec3 rotation)
	{
		this->transform.RotateByEulerRotation(rotation);
		this->w_transform = CalcNewComponentToWorld(this->transform);
		TransformUpdated();

	}
	*/
	void TransformEntity::SetScale(glm::vec3 scale)
	{
		if (transform.GetScale() != scale) {
			this->transform.SetScale(scale);
			this->w_transform = CalcNewComponentToWorld(this->transform);
			TransformUpdated();
		}
	}

	void TransformEntity::SetRelativeTransform(const ShadowEntity::Transform& NewTransform)
	{
		this->transform = NewTransform;
		this->w_transform = CalcNewComponentToWorld(NewTransform);
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

		//This transform has changed so we need to update the children about it
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
		//Recalculat the transform, this always uses the parent or world.
		this->w_transform = CalcNewComponentToWorld(this->transform);

		TransformUpdated();
	}
}
