#pragma once

#include <set>
#include <map>

namespace GameCore::ECS
{
	typedef uint EntityId;

	class EntityManager
	{
	public:
		EntityId Create()
		{
			EntityId newEntity;
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

		void Destroy(EntityId anId)
		{
			if (myUsedEntityIds.erase(anId) > 0)
				myFreeEntityIds.insert(anId);
		}

		bool Exists(EntityId anId)
		{
			return myUsedEntityIds.find(anId) != myUsedEntityIds.end();
		}

	private:
		std::set<EntityId> myUsedEntityIds;
		std::set<EntityId> myFreeEntityIds;
	};

	class ComponentBaseContainer
	{
	public:
		// anElementSize is the size of one element in bytes
		// aChunkSize is the number of elements in one contiguous chunk of data
		ComponentBaseContainer(uint anElementSize, uint aChunkSize)
			: myElementSize(anElementSize)
			, myChunkSize(aChunkSize)
		{}

		virtual ~ComponentBaseContainer()
		{
			for (char* chunk : myChunks)
			{
				delete[] chunk;
			}
		}

		inline uint GetCapacity() const { return myChunkSize * (uint)myChunks.size(); }
		inline uint GetSize() const { return mySize; }

		void Reserve(uint anElementCount)
		{
			while (GetCapacity() < anElementCount)
			{
				myChunks.push_back(new char[myElementSize * myChunkSize]);
			}
		}

		void Resize(uint anElementCount)
		{
			if (mySize >= anElementCount)
				return;
			Reserve(anElementCount);
			mySize = anElementCount;
		}

		inline void* Get(uint anElementIndex)
		{
			Assert(anElementIndex < mySize);
			return myChunks[anElementIndex / myChunkSize] + (anElementIndex % myChunkSize) * myElementSize;
		}

		inline const void* Get(uint anElementIndex) const
		{
			Assert(anElementIndex < mySize);
			return myChunks[anElementIndex / myChunkSize] + (anElementIndex % myChunkSize) * myElementSize;
		}

	private:
		uint myElementSize = 0;
		uint myChunkSize = 0;
		uint mySize = 0;
		std::vector<char*> myChunks;
	};

	template<typename Type, uint ChunkSize = 128>
	class ComponentContainer : public ComponentBaseContainer
	{
	public:
		ComponentContainer() : ComponentBaseContainer(sizeof(Type), ChunkSize) {}

		inline bool HasComponent(EntityId anId)
		{
			return myEntityToIndexMap.find(anId) != myEntityToIndexMap.end();
		}

		inline Type* GetComponent(EntityId anId)
		{
			Assert(HasComponent(anId));
			return reinterpret_cast<Type*>(Get(myEntityToIndexMap[anId]));
		}

		inline const Type* GetComponent(EntityId anId) const
		{
			Assert(HasComponent(anId));
			return reinterpret_cast<const Type*>(Get(myEntityToIndexMap[anId]));
		}

		template<typename ... Args>
		Type* AddComponent(EntityId anId, Args&&... SomeArgs)
		{
			uint index = UINT_MAX;
			if (myFreeIndices.size() == 0)
			{
				index = (uint)myEntityToIndexMap.size();
			}
			else
			{
				index = *myFreeIndices.begin();
				myFreeIndices.erase(index);
			}

			myEntityToIndexMap[anId] = index;
			Resize(index + 1);

			void* ptr = Get(index);
			new(ptr) Type(std::forward<Args>(SomeArgs)...);
			return reinterpret_cast<Type*>(ptr);
		}

		void RemoveComponent(EntityId anId)
		{
			auto it = myEntityToIndexMap.find(anId);
			if (it != myEntityToIndexMap.end())
			{
				Type* ptr = static_cast<Type*>(Get(it->second));
				ptr->~Type();

				myFreeIndices.insert(it->second);
				myEntityToIndexMap.erase(it);
			}
		}

	private:
		std::map<uint, uint> myEntityToIndexMap;
		std::set<uint> myFreeIndices;
	};

	class ComponentManager
	{
	public:
		~ComponentManager()
		{
			for (ComponentBaseContainer* container : myComponentContainers)
				delete container;
		}

		template<typename Type>
		inline bool HasComponent(EntityId anId)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->HasComponent(anId);
		}

		template<typename Type>
		inline Type* GetComponent(EntityId anId)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->GetComponent(anId);
		}

		template<typename Type>
		inline const Type* GetComponent(EntityId anId) const
		{
			return static_cast<const ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->GetComponent(anId);
		}

		template<typename Type, typename ... Args>
		inline Type* AddComponent(EntityId anId, Args&&... SomeArgs)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->AddComponent(anId, std::forward<Args>(SomeArgs)...);
		}

		template<typename Type>
		inline void RemoveComponent(EntityId anId)
		{
			return static_cast<ComponentContainer<Type>*>(myComponentContainers[GetComponentId<Type>()])->RemoveComponent(anId);
		}

	private:
		template<typename Type>
		inline uint GetComponentId()
		{
			static uint id = myComponentIdCounter++;
			if ((uint)myComponentContainers.size() == id)
				myComponentContainers.push_back(new ComponentContainer<Type>());
			return id;
		}

		uint myComponentIdCounter = 0;
		std::vector<ComponentBaseContainer*> myComponentContainers;
	};
}
