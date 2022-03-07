#include "GameCore_EntityModule.h"

#include "GameCore_EntityCameraComponent.h"

namespace GameCore
{
	DEFINE_GAMECORE_MODULE(EntityModule);

	EntityId EntityModule::Create()
	{
		EntityId newEntity;
		if (myFreeEntityIds.size() == 0)
		{
			newEntity = myNextEntityId++;
		}
		else
		{
			newEntity = *myFreeEntityIds.begin();
			myFreeEntityIds.erase(newEntity);
		}

		for (uint i = 0; i < (uint)myComponentContainers.size(); ++i)
			myComponentContainers[i]->OnEntityCreated(newEntity);

		return newEntity;
	}

	void EntityModule::Destroy(EntityId anId)
	{
		if (anId >= myNextEntityId || myFreeEntityIds.find(anId) != myFreeEntityIds.end())
			return;

		for (uint i = 0; i < (uint)myComponentContainers.size(); ++i)
			myComponentContainers[i]->OnEntityDestroyed(anId);

		myFreeEntityIds.insert(anId);
	}

	void EntityModule::OnRegister()
	{
	}

	void EntityModule::OnUnregister()
	{
		for (ComponentContainerBase* container : myComponentContainers)
			delete container;
		myComponentContainers.clear();
	}

	void EntityModule::OnUpdate(UpdateType aType)
	{
		if (aType == Module::UpdateType::MainUpdate)
		{
			ComponentContainer<EntityCameraComponent>* container = GetComponentContainer<EntityCameraComponent>();
			for (EntityCameraComponent* component : *container)
			{
				component->SetAspectRatio(Facade::GetInstance()->GetMainWindowAspectRatio());
				component->Update();
			}
		}
	}
}
