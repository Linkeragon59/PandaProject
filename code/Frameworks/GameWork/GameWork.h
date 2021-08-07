#pragma once

struct GLFWwindow;

namespace GameWork
{
	class Module;
	class Camera;

	class GameWork
	{
	public:
		static bool Create();
		static void Destroy();
		static GameWork* GetInstance() { return ourInstance; }

		void Run();

		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

	private:
		static GameWork* ourInstance;
		GameWork();
		~GameWork();

		bool Update();

		std::vector<Module*> myModules;

		GLFWwindow* myWindow = nullptr;
		GLFWwindow* myWindow2 = nullptr;
		Camera* myCamera = nullptr;
	};
}
