#include "Entity.h"

namespace GameWork
{
	void Entity::SetPosition(const glm::vec3& aPosition)
	{
		myPosition = aPosition;
		OnPositionChanged();
	}

	void Entity::Translate(const glm::vec3& aPositionDelta)
	{
		myPosition += aPositionDelta;
		OnPositionChanged();
	}

	void Entity::SetRotation(const glm::vec3& aRotation)
	{
		myRotation = aRotation;
		OnRotationChanged();
	}

	void Entity::Rotate(const glm::vec3& aRotationDelta)
	{
		myRotation += aRotationDelta;
		OnRotationChanged();
	}
}
