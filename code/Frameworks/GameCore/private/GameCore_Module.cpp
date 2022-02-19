#include "GameCore_Module.h"

#include "GameCore_ModuleManager.h"

namespace GameCore
{
	bool Module::RegisterToManager(Module* aModule)
	{
		return GameCore::GetInstance()->GetModuleManager()->RegisterModule(aModule);
	}

	bool Module::UnregisterFromManager(Module* aModule)
	{
		return GameCore::GetInstance()->GetModuleManager()->UnregisterModule(aModule);
	}
}
