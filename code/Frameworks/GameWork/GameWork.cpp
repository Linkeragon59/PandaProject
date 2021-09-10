#include "GameWork.h"

#include "Base_Time.h"
#include "Base_Input.h"
#include "Base_Window.h"

#include "Render_Facade.h"
#include "Render_Renderer.h"

#include "GameWork_Module.h"
#include "GameWork_CameraManager.h"
#include "GameWork_PropManager.h"

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
#include <chrono>
#include <iostream>
#include <functional>

namespace GameWork
{
	namespace
	{
		/*const std::string locTestWavFile = "Frameworks/audio/Ensoniq-ZR-76-01-Dope-77.wav";
		SoLoud::Soloud locSoloud; // SoLoud engine
		SoLoud::Wav locWave;      // One wave file
		bool locSoundPlaying = false;*/
	}

	GameWork* GameWork::ourInstance = nullptr;

	bool GameWork::Create()
	{
		Assert(!ourInstance);
		ourInstance = new GameWork;
		return true;
	}

	void GameWork::Destroy()
	{
		Assert(ourInstance);
		SafeDelete(ourInstance);
	}

	void GameWork::Run()
	{
		while (!glfwWindowShouldClose(myMainWindow))
		{
			int wantedFPS = 60;
			std::chrono::duration<double, std::milli> oneFrame = std::chrono::milliseconds((long)(1000.0f / wantedFPS));

			auto start = std::chrono::high_resolution_clock::now();
			glfwPollEvents();

			if (!Update())
				break;

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;

			//if (elapsed < oneFrame)
			//	std::this_thread::sleep_for(oneFrame - elapsed);
		}
	}

	bool GameWork::RegisterModule(Module* aModule)
	{
		for (Module* mod : myModules)
		{
			if (strcmp(mod->GetId(), aModule->GetId()) == 0)
				return false;
		}

		myModules.push_back(aModule);
		aModule->OnRegister();
		return true;
	}

	bool GameWork::UnregisterModule(Module* aModule)
	{
		for (auto it = myModules.begin(); it != myModules.end(); ++it)
		{
			if (strcmp((*it)->GetId(), aModule->GetId()) == 0)
			{
				(*it)->OnUnregister();
				myModules.erase(it);
				return true;
			}
		}
		return false;
	}

	Render::Renderer* GameWork::GetMainWindowRenderer() const
	{
		return Render::GetRenderer(myMainWindow);
	}

	GameWork::GameWork()
	{
		Time::TimeManager::Create();
		Window::WindowManager::Create();
		Input::InputManager::Create();

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

		//locSoloud.init(); // Initialize SoLoud
		//locWave.load(locTestWavFile.c_str()); // Load a wave
	}

	GameWork::~GameWork()
	{
		//locSoloud.deinit(); // Clean up!

		delete myCameraManager;
		delete myPropManager;
#if DEBUG_BUILD
		myDebugPropManager->Despawn(myVectorBase);
		delete myDebugPropManager;
#endif

		Render::UnregisterWindow(myMainWindow);
		Render::FinalizeRendering();

		Window::WindowManager* windowManager = Window::WindowManager::GetInstance();
		windowManager->RemoveWindowSizeCallback(myWindowResizeCallbackId);
		windowManager->CloseWindow(myMainWindow);
		myMainWindow = nullptr;

		Input::InputManager::Destroy();
		Window::WindowManager::Destroy();
		Time::TimeManager::Destroy();
	}

	bool GameWork::Update()
	{
		Time::TimeManager::GetInstance()->NextFrame();

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		bool escapePressed = inputManager->PollKeyInput(Input::KeyEscape) == Input::Status::Pressed;
		
		Render::StartFrame();

		// Update Modules
		for (Module* mod : myModules)
		{
			mod->OnUpdate();
		}

		myCameraManager->Update();
		myPropManager->Update();
#if DEBUG_BUILD
		myDebugPropManager->Update();
#endif

		Render::EndFrame();

		return !escapePressed;
	}
}
