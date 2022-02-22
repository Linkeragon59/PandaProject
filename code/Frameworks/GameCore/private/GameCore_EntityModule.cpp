#include "GameCore_EntityModule.h"

namespace GameCore
{
	glm::mat4 Entity3DTransformComponent::GetMatrix() const
	{
		glm::mat4 scaling = glm::scale(myScale);
		glm::mat4 rotation = glm::toMat4(myOrientation);
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), myPosition);
		glm::mat4 matrix = translation * rotation * scaling;
		return matrix;
	}

	Entity3DTransformComponent* Entity3DTransformComponent::GetComponent(EntityHandle aHandle)
	{
		return EntityModule::GetInstance()->GetPositionComponent(aHandle);
	}

	Entity3DTransformComponent* Entity3DTransformComponent::AddComponent(EntityHandle aHandle)
	{
		return EntityModule::GetInstance()->AddPositionComponent(aHandle);
	}

	void Entity3DTransformComponent::RemoveComponent(EntityHandle aHandle)
	{
		EntityModule::GetInstance()->RemovePositionComponent(aHandle);
	}

	DEFINE_GAMECORE_MODULE(EntityModule);

	EntityHandle EntityModule::Create()
	{
		EntityHandle newEntity;
		if (myFreeEntityIds.size() == 0)
		{
			newEntity = (uint)myUsedEntityIds.size();
		}
		else
		{
			newEntity = *myFreeEntityIds.begin();
			myFreeEntityIds.erase(newEntity);
		}
		myUsedEntityIds.insert(newEntity);
		return newEntity;
	}

	void EntityModule::Destroy(EntityHandle aHandle)
	{
		if (myUsedEntityIds.erase(aHandle) > 0)
			myFreeEntityIds.insert(aHandle);
	}

	bool EntityModule::Exists(EntityHandle aHandle)
	{
		return myUsedEntityIds.find(aHandle) != myUsedEntityIds.end();
	}

	Entity3DTransformComponent* EntityModule::GetPositionComponent(EntityHandle aHandle)
	{
		auto entityComponent = myPositionComponents.find(aHandle);
		if (entityComponent != myPositionComponents.end())
			return &entityComponent->second;
		return nullptr;
	}

	Entity3DTransformComponent* EntityModule::AddPositionComponent(EntityHandle aHandle)
	{
		if (Entity3DTransformComponent* component = GetPositionComponent(aHandle))
			return component;
		myPositionComponents[aHandle] = Entity3DTransformComponent();
		return &myPositionComponents[aHandle];
	}

	void EntityModule::RemovePositionComponent(EntityHandle aHandle)
	{
		myPositionComponents.erase(aHandle);
	}
}
