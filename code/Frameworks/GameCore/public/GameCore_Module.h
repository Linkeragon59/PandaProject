#pragma once

#include "GameCore_Facade.h"

namespace GameCore
{
	class ModuleManager;

	class Module
	{
	public:
		virtual ~Module() {};
		virtual const char* GetId() const = 0;
		bool IsInitialized() const { return myIsInitialized; }

		enum class UpdateType
		{
			EarlyUpdate,	// Very beginning of the frame
			MainUpdate,
			LateUpdate,		// Very end of the frame
		};

	protected:
		friend class ModuleManager;

		const bool operator==(const Module* anOther) { return strcmp(GetId(), anOther->GetId()) == 0; }
		
		virtual void OnRegister() {}
		virtual void OnUnregister() {}

		// Called when all dependencies are Initialized
		virtual void OnInitialize() {}
		// Called when any dependency was Finalized or before Unregistering
		virtual void OnFinalize() {}

		// Called each frame after the dependencies have been Updated
		virtual void OnUpdate(UpdateType /*aType*/) {}

		template<typename ModuleType>
		static bool RegisterModule(ModuleType*& anInstance, const std::vector<std::string>& someDependencies)
		{
			if (!Facade::GetInstance())
				return false;

			anInstance = new ModuleType();
			anInstance->myDependencies = someDependencies;
			if (RegisterToManager(anInstance))
			{
				return true;
			}
			delete anInstance;
			anInstance = nullptr;
			return false;
		}

		template<typename ModuleType>
		static bool UnregisterModule(ModuleType*& anInstance)
		{
			if (!Facade::GetInstance())
				return false;

			bool res = UnregisterFromManager(anInstance);
			SafeDelete(anInstance);
			return res;
		}

	private:
		static bool RegisterToManager(Module* aModule);
		static bool UnregisterFromManager(Module* aModule);
		
		std::vector<std::string> myDependencies;
		bool myIsInitialized = false;
	};
}

#define DECLARE_GAMECORE_MODULE(Module, Id) \
public: \
	static bool		Register(const std::vector<std::string>& someDependencies = {})	{ return RegisterModule<Module>(ourInstance, someDependencies); } \
	static bool		Unregister()													{ return UnregisterModule<Module>(ourInstance); } \
	static Module*	GetInstance()													{ return ourInstance; } \
	const char*		GetId() const override											{ return Id; } \
private: \
	static Module* ourInstance;

#define DEFINE_GAMECORE_MODULE(Module)	Module* Module::ourInstance = nullptr
