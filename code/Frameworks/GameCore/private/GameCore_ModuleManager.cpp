#include "GameCore_ModuleManager.h"

namespace GameCore
{
	bool ModuleManager::RegisterModule(Module* aModule)
	{
		if (std::find(myModules.begin(), myModules.end(), aModule) != myModules.end())
			return false;

		myModules.push_back(aModule);
		aModule->OnRegister();
		TryInitializeModule(aModule);
		return true;
	}

	bool ModuleManager::UnregisterModule(Module* aModule)
	{
		auto module = std::find(myModules.begin(), myModules.end(), aModule);
		if (module == myModules.end())
			return false;

		FinalizeModule(aModule);
		(*module)->OnUnregister();
		myModules.erase(module);
		return true;
	}

	void ModuleManager::Update(Module::UpdateType aType)
	{
		for (Module* module : myModules)
			UpdateModule(module, aType);
		for (Module* module : myModules)
			module->myWasUpdatedThisFrame = false;
	}

	void ModuleManager::TryInitializeModule(Module* aModule)
	{
		for (const std::string& dependency : aModule->myDependencies)
		{
			Module* module = GetModule(dependency);
			if (module && !module->myIsInitialized)
			{
				// The newly registered module can't be initialized yet
				return;
			}
		}

		aModule->OnInitialize();
		aModule->myIsInitialized = true;
		
		// Try initialize modules that depend on the newly initialized module
		for (Module* module : myModules)
		{
			if (module->myIsInitialized)
				continue;
			if (std::find(module->myDependencies.begin(), module->myDependencies.end(), aModule->GetId()) == module->myDependencies.end())
				continue;
			TryInitializeModule(module);
		}
	}

	void ModuleManager::FinalizeModule(Module* aModule)
	{
		aModule->myIsInitialized = false;
		
		// First finalize modules that depend on the module to finalize
		for (Module* module : myModules)
		{
			if (!module->myIsInitialized)
				continue;
			if (std::find(module->myDependencies.begin(), module->myDependencies.end(), aModule->GetId()) == module->myDependencies.end())
				continue;
			FinalizeModule(module);
		}

		aModule->OnFinalize();
	}

	void ModuleManager::UpdateModule(Module* aModule, Module::UpdateType aType)
	{
		if (!aModule->myIsInitialized || aModule->myWasUpdatedThisFrame)
			return;

		// First ensure the dependencies have been updated
		for (const std::string& dependency : aModule->myDependencies)
		{
			Module* module = GetModule(dependency);
			if (module && !module->myWasUpdatedThisFrame)
			{
				UpdateModule(module, aType);
			}
		}

		aModule->OnUpdate(aType);
		aModule->myWasUpdatedThisFrame = true;
	}

	Module* ModuleManager::GetModule(const std::string& anId)
	{
		for (Module* module : myModules)
			if (module->GetId() == anId)
				return module;
		return nullptr;
	}
}
