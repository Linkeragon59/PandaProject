#include "GameWork.h"

#include "Module.h"
#include "Camera.h"

#include "Thread.h"
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

void JumpCallback() 
{
	std::cout << "JUMP" << std::endl;
}

void ShootCallback()
{
	std::cout << "SHOOT" << std::endl;
}

namespace GameWork
{
	GameWork* GameWork::ourInstance = nullptr;

	namespace
	{
		const uint locWindowWidth = 1280;
		const uint locWindowHeight = 720;
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

		ourInstance->Init();

		Render::Facade::Create();
		Render::Facade::GetInstance()->InitRenderer(ourInstance->myWindow);

		ourInstance->LoadTestAssets();

		return true;
	}

	void GameWork::Destroy()
	{
		ourInstance->UnloadTestAssets();

		Render::Facade::GetInstance()->FinalizeRenderer(ourInstance->myWindow);
		Render::Facade::Destroy();

		ourInstance->Cleanup();

		InputManager::GetInstance()->RemoveWindow(ourInstance->myWindow);
		InputManager::Destroy();

		delete ourInstance;
		ourInstance = nullptr;
	}

	void GameWork::Run()
	{
		/*auto test = [](const char* aString, uint aMult)
		{
			std::cout << aString << " started" << std::endl;
			uint counter = 0;
			while (counter < (UINT_MAX - 1) / aMult)
				counter++;
			std::cout << aString << " done" << std::endl;
		};

		ThreadHelpers::WorkerPool* workerPool = new ThreadHelpers::WorkerPool();
		ThreadHelpers::WorkerPool* workerPoolLow = new ThreadHelpers::WorkerPool(ThreadHelpers::WorkerPriority::Low);
		{
#if DEBUG_BUILD
			workerPool->SetWorkersName("Short Task");
			workerPoolLow->SetWorkersName("Long Task");
#endif
			workerPool->SetWorkersCount();
			workerPoolLow->SetWorkersCount();

			for (uint i = 0; i < 2; i++)
			{
				workerPool->RequestJob([test]() { test("Job 1", 1); });
				workerPool->RequestJob([test]() { test("Job 2", 2); });
				workerPool->RequestJob([test]() { test("Job 3", 3); });
				workerPool->RequestJob([test]() { test("Job 4", 1); });
				workerPool->RequestJob([test]() { test("Job 5", 2); });
				workerPool->RequestJob([test]() { test("Job 6", 3); });
				workerPool->RequestJob([test]() { test("Job 7", 1); });
				workerPool->RequestJob([test]() { test("Job 8", 2); });
				workerPool->RequestJob([test]() { test("Job 9", 3); });
				workerPool->RequestJob([test]() { test("Job 10", 1); });
				workerPool->RequestJob([test]() { test("Job 11", 2); });
				workerPool->RequestJob([test]() { test("Job 12", 3); });

				workerPoolLow->RequestJob([test]() { test("Low Job 1", 1); });
				workerPoolLow->RequestJob([test]() { test("Low Job 2", 2); });
				workerPoolLow->RequestJob([test]() { test("Low Job 3", 3); });
				workerPoolLow->RequestJob([test]() { test("Low Job 4", 1); });
				workerPoolLow->RequestJob([test]() { test("Low Job 5", 2); });
				workerPoolLow->RequestJob([test]() { test("Low Job 6", 3); });
				workerPoolLow->RequestJob([test]() { test("Low Job 7", 1); });
				workerPoolLow->RequestJob([test]() { test("Low Job 8", 2); });
				workerPoolLow->RequestJob([test]() { test("Low Job 9", 3); });
				workerPoolLow->RequestJob([test]() { test("Low Job 10", 1); });
				workerPoolLow->RequestJob([test]() { test("Low Job 11", 2); });
				workerPoolLow->RequestJob([test]() { test("Low Job 12", 3); });

				workerPool->Wait();
				workerPoolLow->Wait();
			}
		}
		delete workerPool;
		delete workerPoolLow;*/

		InputManager* inputManager = InputManager::GetInstance();
		inputManager->SetupCallbacks();

		uint keySpaceCallback = inputManager->AddCallback(RawInput::KeySpace, JumpCallback);
		uint mouseLeftCallback = inputManager->AddCallback(RawInput::MouseLeft, ShootCallback);

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

		inputManager->RemoveCallback(keySpaceCallback);
		inputManager->RemoveCallback(mouseLeftCallback);
	}

	void GameWork::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		myWindow = glfwCreateWindow(locWindowWidth, locWindowHeight, "Panda Project v0.1", nullptr, nullptr);
	}

	void GameWork::Init()
	{
		myCamera = new Camera();
		myCamera->SetPosition(glm::vec3(0.0f, -0.75f, -2.0f));
		myCamera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		myCamera->SetPerspective(800.0f / 600.0f, 60.0f, 0.1f, 256.0f);
	}

	void GameWork::Cleanup()
	{
		if (myCamera)
		{
			delete myCamera;
			myCamera = nullptr;
		}

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

		RawInputState bState = inputManager->PollRawInput(RawInput::KeyB);
		if (bState == RawInputState::Pressed)
		{
			std::cout << "B PRESSED" << std::endl;
			LoadTestAssets();
		}
		
		RawInputState cState = inputManager->PollRawInput(RawInput::KeyC);
		if (cState == RawInputState::Pressed)
		{
			std::cout << "C PRESSED" << std::endl;
			UnloadTestAssets();
		}

		bool escapePressed = inputManager->PollRawInput(RawInput::KeyEscape) == RawInputState::Pressed;

		// Update Modules
		for (Module* mod : myModules)
		{
			mod->OnUpdate();
		}

		myCamera->Update();

		// Update Render
		Render::Facade::GetInstance()->UpdateRenderer(myCamera->GetViewMatrix(), myCamera->GetPerspectiveMatrix());

		return !escapePressed;
	}

	void GameWork::LoadTestAssets()
	{
		if (myCastleModel != UINT_MAX)
			return;

		{
			Render::RenderData data;
			myCastleModel = Render::Facade::GetInstance()->SpawnModel("Frameworks/models/samplebuilding.gltf", data);
		}

		{
			Render::RenderData data;
			data.myIsTransparent = true;
			myCastleWindows = Render::Facade::GetInstance()->SpawnModel("Frameworks/models/samplebuilding_glass.gltf", data);
		}

		{
			Render::RenderData data;
			data.myMatrix[3][3] = 50.0f;
			myAvocadoModel = Render::Facade::GetInstance()->SpawnModel("Frameworks/models/Avocado/Avocado.gltf", data);
		}

		{
			Render::RenderData data;
			data.myIsAnimated = true;
			myAnimatedModel = Render::Facade::GetInstance()->SpawnModel("Frameworks/models/CesiumMan/CesiumMan.gltf", data);
		}

		{
			Render::RenderData data;
			data.myMatrix[3][0] = 1.0f;
			data.myMatrix[3][1] = 1.0f;
			data.myMatrix[3][2] = -1.0f;
			myDummyModel = Render::Facade::GetInstance()->SpawnModel("", data);
		}
	}

	void GameWork::UnloadTestAssets()
	{
		if (myCastleModel == UINT_MAX)
			return;

		Render::Facade::GetInstance()->DespawnModel(myCastleModel);
		myCastleModel = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myCastleWindows);
		myCastleWindows = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myAvocadoModel);
		myAvocadoModel = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myAnimatedModel);
		myAnimatedModel = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myDummyModel);
		myDummyModel = UINT_MAX;
	}
}
