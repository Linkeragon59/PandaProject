#pragma once

namespace GameWork
{
	class Module;
	class Window;

	class GameWork
	{
	public:
		static bool Create();
		static void Destroy();
		static GameWork* GetInstance() { return ourInstance; }

		void Run();

		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

		uint OpenWindow(uint aWidth, uint aHeight, const char* aTitle, bool aShouldInit = true);
		void CloseWindow(uint aWindowIndex, bool aShouldFinalize = true);

	private:
		GameWork();
		~GameWork();
		bool Update();

		static GameWork* ourInstance;

		std::vector<Module*> myModules;
		std::vector<Window*> myWindows;
	};
}
