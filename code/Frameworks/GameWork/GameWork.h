#pragma once

struct GLFWwindow;

namespace Render
{
	class Renderer;
	class Model;
}

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
		Camera* myCamera = nullptr;
		Render::Renderer* myRenderer = nullptr;
		std::vector<Render::Model*> myModels;
	};
}
