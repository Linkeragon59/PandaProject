#include "GameCore_Entity.h"
#include "GameCore_EntityModule.h"

namespace GameCore
{
	EntityHandle EntityHandle::Create()
	{
		return EntityModule::GetInstance()->Create();
	}

	void EntityHandle::Destroy(EntityHandle aHandle)
	{
		EntityModule::GetInstance()->Destroy(aHandle);
	}
}
