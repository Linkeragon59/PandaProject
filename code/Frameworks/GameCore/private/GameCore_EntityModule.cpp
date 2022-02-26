#include "GameCore_EntityModule.h"

namespace GameCore
{
	DEFINE_GAMECORE_MODULE(EntityModule);

	void EntityModule::OnRegister()
	{
		myEntityManager = new ECS::EntityManager;
		myComponentManager = new ECS::ComponentManager;
	}

	void EntityModule::OnUnregister()
	{
		SafeDelete(myEntityManager);
		SafeDelete(myComponentManager);
	}

	glm::mat4 Entity3DTransformComponent::GetMatrix() const
	{
		glm::mat4 scaling = glm::scale(myScale);
		glm::mat4 rotation = glm::toMat4(myOrientation);
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), myPosition);
		glm::mat4 matrix = translation * rotation * scaling;
		return matrix;
	}
}
