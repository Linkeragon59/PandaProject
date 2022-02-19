#pragma once

struct GLFWwindow;

namespace Render
{
	class Renderer;
}

namespace GameCore
{
	class ModuleManager;
	class CameraManager;
	class Prop;
	class PropManager;
	class Editor;
	class NodeRegister;

	class GameCore
	{
	public:
		static bool Create();
		static void Destroy();
		static GameCore* GetInstance() { return ourInstance; }

		void Run();

		Render::Renderer* GetMainWindowRenderer() const;
		float GetMainWindowAspectRatio() const { return myMainWindowAspectRatio; }

		ModuleManager* GetModuleManager() const { return myModuleManager; }
		CameraManager* GetCameraManager() const { return myCameraManager; }
		PropManager* GetPropManager() const { return myPropManager; }

		NodeRegister* GetNodeRegister() const { return myNodeRegister; }

	private:
		static GameCore* ourInstance;
		GameCore();
		~GameCore();

		bool Update();

		GLFWwindow* myMainWindow = nullptr;
		float myMainWindowAspectRatio = 1.0f;
		uint myWindowResizeCallbackId = UINT_MAX;

		ModuleManager* myModuleManager = nullptr;
		CameraManager* myCameraManager = nullptr;
		PropManager* myPropManager = nullptr;
#if DEBUG_BUILD
		PropManager* myDebugPropManager = nullptr;
		Prop* myVectorBase = nullptr;
#endif
		NodeRegister* myNodeRegister = nullptr;
	};
}
