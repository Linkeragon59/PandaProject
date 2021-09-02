#pragma once

struct GLFWwindow;

namespace Render
{
	class Renderer;
}

namespace GameWork
{
	class Module;
	class WindowManager;
	class CameraManager;
	class Prop;
	class PropManager;
	class Editor;

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
#if DEBUG_BUILD
		void OpenEditor();
		void CloseEditor();
#endif

		WindowManager* GetWindowManager() const { return myWindowManager; }
		CameraManager* GetCameraManager() const { return myCameraManager; }
		PropManager* GetPropManager() const { return myPropManager; }

		static void OnWindowResize(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnWindowFramebufferResize(GLFWwindow* aWindow, int aWidth, int aHeight);

	private:
		static GameWork* ourInstance;
		GameWork();
		~GameWork();

		bool Update();

		GLFWwindow* myMainWindow = nullptr;
		float myMainWindowAspectRatio = 1.0f;
#if DEBUG_BUILD
		Editor* myEditor = nullptr;
#endif

		std::vector<Module*> myModules;
		WindowManager* myWindowManager = nullptr;
		CameraManager* myCameraManager = nullptr;
		PropManager* myPropManager = nullptr;
#if DEBUG_BUILD
		PropManager* myDebugPropManager = nullptr;
		Prop* myVectorBase = nullptr;
#endif
	};
}
