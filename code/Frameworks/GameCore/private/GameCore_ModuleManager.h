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

		void RebuildUpdateQueue();
		void PushModuleToUpdateQueue(Module* aModule);

		Module* GetModule(const std::string& anId);

		std::vector<Module*> myModules;
		std::vector<Module*> myModulesToUpdate; // sorted by update order
	};
}
