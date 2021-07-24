#include "GameWork.h"

#include "Input.h"
#include "RenderFacade.h"
#include "Renderer.h"
#include "RenderModel.h"

#include "Module.h"
#include "Camera.h"

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

	GameWork::GameWork()
	{
		Input::InputManager::Create();

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		myWindow = glfwCreateWindow(locWindowWidth, locWindowHeight, "Panda Project v0.1", nullptr, nullptr);

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->AddWindow(myWindow);
		inputManager->SetupCallbacks(inputManager->GetWindowId(myWindow));

		myCamera = new Camera();
		myCamera->SetPosition(glm::vec3(0.0f, -0.75f, -2.0f));
		myCamera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		myCamera->SetPerspective(800.0f / 600.0f, 60.0f, 0.1f, 256.0f);

		Render::Facade::Create();
		Render::Facade::GetInstance()->InitRendering(Render::Facade::RenderAPI::Vulkan);
		Render::Facade::GetInstance()->RegisterWindow(myWindow);
		myRenderer = Render::Facade::GetInstance()->CreateRenderer(Render::RendererType::Deferred);
	}

	GameWork::~GameWork()
	{
		Render::Facade::GetInstance()->DestroyRenderer(myRenderer);
		Render::Facade::GetInstance()->UnregisterWindow(myWindow);
		Render::Facade::GetInstance()->FinalizeRendering();
		Render::Facade::Destroy();

		delete myCamera;

		Input::InputManager::GetInstance()->RemoveWindow(myWindow);

		glfwDestroyWindow(myWindow);
		
		glfwTerminate();

		Input::InputManager::Destroy();
	}

	bool GameWork::Update()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		bool escapePressed = inputManager->PollRawInput(Input::RawInput::KeyEscape) == Input::RawInputState::Pressed;
		
		// Update Modules
		for (Module* mod : myModules)
		{
			mod->OnUpdate();
		}
		
		Render::Facade::GetInstance()->StartFrame();

		myRenderer->BindWindow(myWindow);

		myCamera->Update();
		myRenderer->UpdateView(myCamera->GetViewMatrix(), myCamera->GetPerspectiveMatrix());

		for (Render::Model* model : myModels)
		{
			model->Update();
			myRenderer->DrawModel(model);
		}

		Render::Facade::GetInstance()->EndFrame();

		return !escapePressed;
	}
}
