#include "GameCore_Module.h"

namespace GameCore
{
	class ModuleManager
	{
	public:
		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

		void Update(Module::UpdateType aType);

	private:
		void TryInitializeModule(Module* aModule);
		void FinalizeModule(Module* aModule);

		void UpdateModule(Module* aModule, Module::UpdateType aType);

		Module* GetModule(const std::string& anId);

		std::vector<Module*> myModules;
	};
}
