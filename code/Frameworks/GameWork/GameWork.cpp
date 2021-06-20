#include "GameWork.h"

#include "Input.h"
#include "RenderFacade.h"

#include "Module.h"
#include "Window.h"

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
		delete ourInstance;
		ourInstance = nullptr;
	}

	void GameWork::Run()
	{
		while (!myWindows[0]->ShouldClose())
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

	uint GameWork::OpenWindow(uint aWidth, uint aHeight, const char* aTitle, bool aShouldInit /*= true*/)
	{
		// TODO: use free slots
		myWindows.push_back(new Window(aWidth, aHeight, aTitle));

		if (aShouldInit)
			myWindows.back()->Init();

		return (uint)myWindows.size() - 1;
	}

	void GameWork::CloseWindow(uint aWindowIndex, bool aShouldFinalize /*= true*/)
	{
		if (aShouldFinalize)
			myWindows[aWindowIndex]->Finalize();

		delete myWindows[aWindowIndex];
		myWindows[aWindowIndex] = nullptr;
	}

	GameWork::GameWork()
	{
		Input::InputManager::Create();

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		OpenWindow(locWindowWidth, locWindowHeight, "Panda Project v0.1", false);

		Render::Facade::Create();
		Render::Facade::GetInstance()->InitRenderer();

		myWindows[0]->Init();
	}

	GameWork::~GameWork()
	{
		myWindows[0]->Finalize();

		Render::Facade::GetInstance()->FinalizeRenderer();
		Render::Facade::Destroy();

		CloseWindow(0, false);
		glfwTerminate();

		Input::InputManager::Destroy();
	}

	bool GameWork::Update()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		bool escapePressed = inputManager->PollRawInput(Input::RawInput::KeyEscape) == Input::RawInputState::Pressed;

		Input::RawInputState bState = inputManager->PollRawInput(Input::RawInput::KeyB);
		if (bState == Input::RawInputState::Pressed)
		{
			std::cout << "B PRESSED" << std::endl;
			if (myWindows.size() == 1)
				OpenWindow(locWindowWidth, locWindowHeight, "Test");
		}

		Input::RawInputState cState = inputManager->PollRawInput(Input::RawInput::KeyC);
		if (cState == Input::RawInputState::Pressed)
		{
			std::cout << "C PRESSED" << std::endl;
			if (myWindows.size() > 1 && myWindows[1])
				CloseWindow(1);
		}

		// Update Modules
		for (Module* mod : myModules)
		{
			mod->OnUpdate();
		}

		// Update Windows
		for (uint i = 0; i < myWindows.size(); ++i)
		{
			if (myWindows[i] && !myWindows[i]->Update() && i > 0)
				CloseWindow(i);
		}

		Render::Facade::GetInstance()->UpdateRenderer();

		return !escapePressed;
	}
}
