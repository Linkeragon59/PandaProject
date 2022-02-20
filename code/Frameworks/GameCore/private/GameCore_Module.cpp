#include "GameCore_Module.h"

#include "GameCore_ModuleManager.h"

namespace GameCore
{
	bool Module::RegisterToManager(Module* aModule)
	{
		return Facade::GetInstance()->GetModuleManager()->RegisterModule(aModule);
	}

	bool Module::UnregisterFromManager(Module* aModule)
	{
		return Facade::GetInstance()->GetModuleManager()->UnregisterModule(aModule);
	}
}
