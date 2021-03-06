#pragma once

#include "GameWork.h"

namespace GameWork
{
	class Module
	{
	public:
		virtual const char* GetId() = 0;

	protected:
		friend class GameWork;
		virtual void OnRegister() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnUnregister() = 0;

		template<typename ModuleType>
		static bool RegisterModule(ModuleType*& anInstance)
		{
			if (!GameWork::GetInstance())
				return false;

			ModuleType* newModule = new ModuleType();

			if (GameWork::GetInstance()->RegisterModule(newModule))
			{
				anInstance = newModule;
				return true;
			}

			delete newModule;
			return false;
		}

		template<typename ModuleType>
		static bool UnregisterModule(ModuleType*& anInstance)
		{
			if (!GameWork::GetInstance())
				return false;

			bool res = GameWork::GetInstance()->UnregisterModule(anInstance);

			delete anInstance;
			anInstance = nullptr;

			return res;
		}
	};
}

#define DECLARE_GAMEWORK_MODULE(Module, Id) \
public: \
	static bool		Register()			{ return RegisterModule<Module>(ourInstance); } \
	static bool		Unregister()		{ return UnregisterModule<Module>(ourInstance); } \
	static Module*	GetInstance()		{ return ourInstance; } \
	const char*		GetId() override	{ return Id; } \
private: \
	static Module* ourInstance; // Needs to be defined and initialized by each module
