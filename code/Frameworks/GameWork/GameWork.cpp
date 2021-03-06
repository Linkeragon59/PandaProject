#include "GameWork.h"

#include "Module.h"
#include "Input.h"

#include <GLFW/glfw3.h>
#include "RenderFacade.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>

//using namespace Input;
using Input::InputManager;
using Input::RawInput;
using Input::RawInputState;

namespace GameWork
{
	GameWork* GameWork::ourInstance = nullptr;

	namespace
	{
		const uint32_t locWindowWidth = 800;
		const uint32_t locWindowHeight = 600;
	}

	GameWork::GameWork()
	{
	}

	GameWork::~GameWork()
	{
	}

	bool GameWork::Create()
	{
		ourInstance = new GameWork;
		ourInstance->InitWindow();

		InputManager::Create();
		InputManager::GetInstance()->AddWindow(ourInstance->myWindow);

		try
		{
			Render::Facade::InitBasicRenderer(ourInstance->myWindow);
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return false;
		}

		return true;
	}

	void GameWork::Destroy()
	{
		Render::Facade::FinalizeBasicRenderer();

		InputManager::GetInstance()->RemoveWindow(ourInstance->myWindow);
		InputManager::Destroy();

		ourInstance->Cleanup();
		delete ourInstance;
		ourInstance = nullptr;
	}

	void GameWork::Run()
	{
		while (!glfwWindowShouldClose(myWindow))
		{
			int wantedFPS = 60;
			std::chrono::duration<double, std::milli> oneFrame = std::chrono::milliseconds((long)(1000.0f / wantedFPS));

			auto start = std::chrono::high_resolution_clock::now();
			glfwPollEvents();

			if (!Update())
				break;

			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> elapsed = end - start;

			if (elapsed < oneFrame)
				std::this_thread::sleep_for(oneFrame - elapsed);
		}
	}

	void GameWork::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		myWindow = glfwCreateWindow(locWindowWidth, locWindowHeight, "Panda Project v0.1", nullptr, nullptr);
	}

	void GameWork::Cleanup()
	{
		glfwDestroyWindow(myWindow);
		myWindow = nullptr;
		glfwTerminate();
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

	bool GameWork::Update()
	{
		// Check inputs
		InputManager* inputManager = InputManager::GetInstance();

		Input::RawInputState mouseRightState = inputManager->PollRawInput(Input::RawInput::MouseRight);
		if (mouseRightState == Input::RawInputState::Pressed)
		{
			RawInputState wState = inputManager->PollRawInput(RawInput::KeyW);
			if (wState == RawInputState::Pressed)
				std::cout << " FORWARD";

			RawInputState aState = inputManager->PollRawInput(RawInput::KeyA);
			if (aState == RawInputState::Pressed)
				std::cout << " LEFT";

			RawInputState dState = inputManager->PollRawInput(RawInput::KeyD);
			if (dState == RawInputState::Pressed)
				std::cout << " RIGHT";

			RawInputState sState = inputManager->PollRawInput(RawInput::KeyS);
			if (sState == RawInputState::Pressed)
				std::cout << " BACKWARD";

			double mouseX, mouseY;
			inputManager->PollMousePosition(mouseX, mouseY);
			std::cout << " | Mouse position: " << mouseX << " : " << mouseY << std::endl;
		}
		
		bool escapePressed = inputManager->PollRawInput(RawInput::KeyEscape) == RawInputState::Pressed;

		// Update Render
		try
		{
			Render::Facade::UpdateBasicRenderer();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}

		// Update Modules
		for (Module* mod : myModules)
		{
			mod->OnUpdate();
		}

		return !escapePressed;
	}
}
