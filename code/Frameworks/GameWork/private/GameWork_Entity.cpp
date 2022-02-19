#include "GameWork_Entity.h"

namespace GameWork
{
	void Entity::SetPosition(const glm::vec3& aPosition)
	{
		myPosition = aPosition;
	}

	void Entity::Translate(const glm::vec3& aTranslation)
	{
		myPosition += aTranslation;
	}

	void Entity::SetOrientation(const glm::vec3& someEulerAngles)
	{
		myOrientation = glm::quat(glm::radians(someEulerAngles));
	}

	void Entity::SetOrientation(float anAngle, const glm::vec3& anAxis)
	{
		myOrientation = glm::angleAxis(glm::radians(anAngle), anAxis);
	}

	void Entity::Rotate(const glm::vec3& someEulerAngles)
	{
		myOrientation = glm::quat(glm::radians(someEulerAngles)) * myOrientation;
	}

	void Entity::Rotate(float anAngle, const glm::vec3& anAxis)
	{
		myOrientation = glm::angleAxis(glm::radians(anAngle), anAxis) * myOrientation;
	}

	void Entity::SetScale(float aScale)
	{
		myScale = glm::vec3(aScale);
	}

	void Entity::SetScale(glm::vec3 aScale)
	{
		myScale = aScale;
	}

	void Entity::Scale(float aScaleMultiplier)
	{
		myScale *= aScaleMultiplier;
	}

	void Entity::Scale(glm::vec3 aScaleMultiplier)
	{
		myScale *= aScaleMultiplier;
	}

	glm::mat4 Entity::GetMatrix() const
	{
		glm::mat4 scaling = glm::scale(myScale);
		glm::mat4 rotation = glm::toMat4(myOrientation);
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), myPosition);
		glm::mat4 matrix = translation * rotation * scaling;
		return matrix;
	}
}
