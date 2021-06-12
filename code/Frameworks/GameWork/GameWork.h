#pragma once

struct GLFWwindow;

namespace GameWork
{
	class Module;
	class Camera;

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
		void Init();
		void Cleanup();

		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

	private:
		bool Update();

		static GameWork* ourInstance;
		GLFWwindow* myWindow = nullptr;

		std::vector<Module*> myModules;

		Camera* myCamera = nullptr;

		void LoadTestAssets();
		void UnloadTestAssets();
		uint myCastleModel = UINT_MAX;
		uint myCastleWindows = UINT_MAX;
		uint myAvocadoModel = UINT_MAX;
		uint myAnimatedModel = UINT_MAX;
		uint myDummyModel = UINT_MAX;
	};
}
