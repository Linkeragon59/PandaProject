#include "GameCore.h"

#include "Base_Time.h"
#include "Base_Input.h"
#include "Base_Window.h"

#include "Render_Facade.h"
#include "Render_Renderer.h"

#include "GameCore_ModuleManager.h"
#include "GameCore_CameraManager.h"
#include "GameCore_PropManager.h"
#include "GameCore_Graph.h"

#if LINUX_BUILD
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include "soloud.h"
#include "soloud_wav.h"

#if LINUX_BUILD
#pragma GCC diagnostic pop
#endif

#include <GLFW/glfw3.h>

namespace GameCore
{
	namespace
	{
		const std::string locTestWavFile = "Frameworks/audio/Ensoniq-ZR-76-01-Dope-77.wav";
		SoLoud::Soloud locSoloud; // SoLoud engine
		SoLoud::Wav locWave;      // One wave file
	}

	GameCore* GameCore::ourInstance = nullptr;

	bool GameCore::Create()
	{
		Assert(!ourInstance);
		ourInstance = new GameCore;
		return true;
	}

	void GameCore::Destroy()
	{
		Assert(ourInstance);
		SafeDelete(ourInstance);
	}

	void GameCore::Run()
	{
		while (!glfwWindowShouldClose(myMainWindow))
		{
			glfwPollEvents();

			if (!Update())
				break;
		}
	}

	Render::Renderer* GameCore::GetMainWindowRenderer() const
	{
		return Render::GetRenderer(myMainWindow);
	}

	GameCore::GameCore()
	{
		Time::TimeManager::Create();
		Window::WindowManager::Create();
		Input::InputManager::Create();

		myModuleManager = new ModuleManager();

		Window::WindowManager* windowManager = Window::WindowManager::GetInstance();
		myMainWindow = windowManager->OpenWindow("Panda Engine");
		myWindowResizeCallbackId = windowManager->AddWindowSizeCallback([this](int aWidth, int aHeight) {
			myMainWindowAspectRatio = (aHeight != 0) ? (float)aWidth / (float)aHeight : 1.0f;
		}, myMainWindow);

		int width = 0, height = 0;
		glfwGetWindowSize(myMainWindow, &width, &height);
		myMainWindowAspectRatio = (height != 0) ? (float)width / (float)height : 1.0f;

		Render::InitializeRendering();
		Render::RegisterWindow(myMainWindow, Render::Renderer::Type::Deferred);

		myCameraManager = new CameraManager();
		myPropManager = new PropManager();
#if DEBUG_BUILD
		myDebugPropManager = new PropManager(Render::Renderer::DrawType::Debug);
		Render::SimpleGeometryModelData modelData;
		modelData.FillWithPreset(Render::SimpleGeometryModelData::Preset::VectorBaseWidget);
		modelData.myTextureFilename = "Frameworks/textures/white.png";
		myVectorBase = myDebugPropManager->Spawn(modelData);
#endif

		myNodeRegister = new NodeRegister();

		locSoloud.init(); // Initialize SoLoud
		locWave.load(locTestWavFile.c_str()); // Load a wave
	}

	GameCore::~GameCore()
	{
		locSoloud.deinit(); // Clean up!

		delete myCameraManager;
		delete myPropManager;
#if DEBUG_BUILD
		myDebugPropManager->Despawn(myVectorBase);
		delete myDebugPropManager;
#endif

		delete myNodeRegister;

		Render::UnregisterWindow(myMainWindow);
		Render::FinalizeRendering();

		Window::WindowManager* windowManager = Window::WindowManager::GetInstance();
		windowManager->RemoveWindowSizeCallback(myWindowResizeCallbackId);
		windowManager->CloseWindow(myMainWindow);
		myMainWindow = nullptr;

		delete myModuleManager;

		Input::InputManager::Destroy();
		Window::WindowManager::Destroy();
		Time::TimeManager::Destroy();
	}

	bool GameCore::Update()
	{
		Time::TimeManager::GetInstance()->NextFrame();

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		bool escapePressed = inputManager->PollKeyInput(Input::KeyEscape) == Input::Status::Pressed;

		myModuleManager->Update(Module::UpdateType::EarlyUpdate);
		
		Render::StartFrame();

		myModuleManager->Update(Module::UpdateType::MainUpdate);

		myCameraManager->Update();
		myPropManager->Update();
#if DEBUG_BUILD
		myDebugPropManager->Update();
#endif

		Render::EndFrame();

		myModuleManager->Update(Module::UpdateType::LateUpdate);

		return !escapePressed;
	}
}
