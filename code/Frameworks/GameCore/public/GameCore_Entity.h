#pragma once

namespace GameCore
{
	class EntityHandle
	{
	public:
		EntityHandle() : myId(UINT_MAX) {}
		EntityHandle(uint anId) : myId(anId) {}
		operator uint() const { return myId; }

		static EntityHandle Create();
		static void Destroy(EntityHandle aHandle);

		template<typename T>
		T* GetComponent()
		{
			return T::GetComponent(myId);
		}

		template<typename T, typename... Args>
		T* AddComponent(Args&&... someArgs)
		{
			return T::AddComponent(myId, std::forward<Args>(someArgs)...);
		}

		template<typename T>
		void RemoveComponent()
		{
			return T::RemoveComponent(myId);
		}

	private:
		uint myId;
	};
}
