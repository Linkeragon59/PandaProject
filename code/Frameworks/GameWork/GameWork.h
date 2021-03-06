#pragma once

#include <vector>

struct GLFWwindow;

namespace GameWork
{
	class Module;

	class GameWork
	{
	public:
		GameWork();
		~GameWork();

		static bool Create();
		static void Destroy();
		static GameWork* GetInstance() { return ourInstance; }

		void Run();
		void InitWindow();
		void Cleanup();

		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

	private:
		bool Update();

		static GameWork* ourInstance;
		GLFWwindow* myWindow = nullptr;

		std::vector<Module*> myModules;
	};
}
