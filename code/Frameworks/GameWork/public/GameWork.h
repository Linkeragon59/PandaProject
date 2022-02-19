#pragma once

struct GLFWwindow;

namespace Render
{
	class Renderer;
}

namespace GameWork
{
	class Module;
	class CameraManager;
	class Prop;
	class PropManager;
	class Editor;
	class NodeRegister;

	class GameWork
	{
	public:
		static bool Create();
		static void Destroy();
		static GameWork* GetInstance() { return ourInstance; }

		void Run();

		bool RegisterModule(Module* aModule);
		bool UnregisterModule(Module* aModule);

		Render::Renderer* GetMainWindowRenderer() const;
		float GetMainWindowAspectRatio() const { return myMainWindowAspectRatio; }

		CameraManager* GetCameraManager() const { return myCameraManager; }
		PropManager* GetPropManager() const { return myPropManager; }

		NodeRegister* GetNodeRegister() const { return myNodeRegister; }

	private:
		static GameWork* ourInstance;
		GameWork();
		~GameWork();

		bool Update();

		GLFWwindow* myMainWindow = nullptr;
		float myMainWindowAspectRatio = 1.0f;
		uint myWindowResizeCallbackId = UINT_MAX;

		std::vector<Module*> myModules;
		CameraManager* myCameraManager = nullptr;
		PropManager* myPropManager = nullptr;
#if DEBUG_BUILD
		PropManager* myDebugPropManager = nullptr;
		Prop* myVectorBase = nullptr;
#endif
		NodeRegister* myNodeRegister = nullptr;
	};
}
