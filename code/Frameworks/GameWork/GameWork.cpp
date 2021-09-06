#include "GameWork.h"

#include "Base_Time.h"
#include "Base_Input.h"

#include "Render_Facade.h"
#include "Render_Renderer.h"

#include "GameWork_Module.h"
#include "GameWork_WindowManager.h"
#include "GameWork_CameraManager.h"
#include "GameWork_PropManager.h"
#include "GameWork_Editor.h"

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

namespace GameWork
{
	GameWork* GameWork::ourInstance = nullptr;

	namespace
	{
		const uint locWindowWidth = 1280;
		const uint locWindowHeight = 720;
		
		/*const std::string locTestWavFile = "Frameworks/audio/Ensoniq-ZR-76-01-Dope-77.wav";
		SoLoud::Soloud locSoloud; // SoLoud engine
		SoLoud::Wav locWave;      // One wave file
		bool locSoundPlaying = false;*/
	}

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

#if DEBUG_BUILD
	void GameWork::OpenEditor()
	{
		if (myEditor)
			return;

		myEditor = new Editor();
	}

	void GameWork::CloseEditor()
	{
		if (!myEditor)
			return;

		SafeDelete(myEditor);
	}
#endif

	GameWork::GameWork()
	{
		Time::TimeManager::Create();
		Input::InputManager::Create();

		myWindowManager = new WindowManager();
		myMainWindow = myWindowManager->OpenWindow(locWindowWidth, locWindowHeight, "Panda Project v0.1");
		Input::InputManager::GetInstance()->SetMainWindow(myMainWindow);
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

		myWindowManager->CloseWindow(myMainWindow);
		myMainWindow = nullptr;

		delete myWindowManager;

		Input::InputManager::Destroy();
		Time::TimeManager::Destroy();
	}

	bool GameWork::Update()
	{
		Time::TimeManager::GetInstance()->NextFrame();

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		bool escapePressed = inputManager->PollRawInput(Input::RawInput::KeyEscape) == Input::RawInputState::Pressed;

		/*if (inputManager->PollRawInput(Input::RawInput::KeyU) == Input::RawInputState::Pressed)
		{
			if (!locSoundPlaying)
			{
				locSoundPlaying = true;
				locSoloud.play(locWave); // Play the wave
			}
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyY) == Input::RawInputState::Pressed)
		{
			if (locSoundPlaying)
			{
				locSoundPlaying = false;
				locSoloud.stopAll(); // Stop the wave
			}
		}*/
#if DEBUG_BUILD
		if (inputManager->PollRawInput(Input::RawInput::KeyK) == Input::RawInputState::Pressed)
		{
			OpenEditor();
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyL) == Input::RawInputState::Pressed)
		{
			CloseEditor();
		}
#endif
		
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

#if DEBUG_BUILD
		if (myEditor)
			myEditor->Update();
#endif

		Render::EndFrame();

		return !escapePressed;
	}

	void GameWork::OnWindowResize(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		Assert(ourInstance);
		if (aWindow == ourInstance->myMainWindow)
		{
			ourInstance->myMainWindowAspectRatio = (aHeight != 0) ? (float)aWidth / (float)aHeight : 1.0f;
		}
	}

	void GameWork::OnWindowFramebufferResize(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		(void)aWidth;
		(void)aHeight;
		Render::ResizeWindow(aWindow);
	}
}
