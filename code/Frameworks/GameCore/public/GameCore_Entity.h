#pragma once

#include "GameCore_EntityModule.h"

namespace GameCore
{
	class Entity
	{
	public:
		Entity() : myId(UINT_MAX) {}
		Entity(ECS::EntityId anId) : myId(anId) {}
		operator ECS::EntityId() const { return myId; }

		static Entity Create()
		{
			return EntityModule::GetInstance()->GetEntityManager()->Create();
		}

		inline void Destroy()
		{
			EntityModule::GetInstance()->GetEntityManager()->Destroy(myId);
		}

		template<typename T>
		inline bool HasComponent()
		{
			return EntityModule::GetInstance()->GetComponentManager()->HasComponent<T>(myId);
		}

		template<typename T>
		inline T* GetComponent()
		{
			return EntityModule::GetInstance()->GetComponentManager()->GetComponent<T>(myId);
		}

		template<typename T>
		inline const T* GetComponent() const
		{
			return EntityModule::GetInstance()->GetComponentManager()->GetComponent<T>(myId);
		}

		template<typename T, typename... Args>
		inline T* AddComponent(Args&&... someArgs)
		{
			return EntityModule::GetInstance()->GetComponentManager()->AddComponent<T>(myId, std::forward<Args>(someArgs)...);
		}

		template<typename T>
		inline void RemoveComponent()
		{
			EntityModule::GetInstance()->GetComponentManager()->RemoveComponent<T>(myId);
		}

	private:
		ECS::EntityId myId;
	};
}
