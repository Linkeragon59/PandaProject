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

		template<typename T>
		T* AddComponent()
		{
			return T::AddComponent(myId);
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
