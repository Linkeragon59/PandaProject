#include "GameWork.h"

#include "Input.h"
#include "RenderFacade.h"

#include "Module.h"
#include "Camera.h"
#include "Prop.h"
#include "glTFProp.h"
#include "DynamicProp.h"
#include "PointLight.h"

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

		Render::Renderer* locRenderer;
		Render::Renderer* locRenderer2;
		
		std::vector<Prop*> locProps;
		std::vector<PointLight> locLights;
		Camera* locCamera = nullptr;

		const std::vector<Render::DynamicModelData::Vertex> locVertices =
		{
			{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},
			{{0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{0.87f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.25f}, {0.0f, 1.0f, 0.0f, 1.0f}},
			{{0.87f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.75f}, {0.0f, 1.0f, 1.0f, 1.0f}},
			{{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
			{{-0.87f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.75f}, {1.0f, 0.0f, 1.0f, 1.0f}},
			{{-0.87f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.25f}, {1.0f, 0.0f, 0.0f, 1.0f}},

			{{0.0f, 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}},
			{{0.0f, -1.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{0.87f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.25f}, {0.0f, 1.0f, 0.0f, 1.0f}},
			{{0.87f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.065f, 0.75f}, {0.0f, 1.0f, 1.0f, 1.0f}},
			{{0.0f, 1.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
			{{-0.87f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.75f}, {1.0f, 0.0f, 1.0f, 1.0f}},
			{{-0.87f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.935f, 0.25f}, {1.0f, 0.0f, 0.0f, 1.0f}}
		};

		const std::vector<uint> locIndices =
		{
			0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1,

			7, 8, 9, 7, 9, 10, 7, 10, 11, 7, 11, 12, 7, 12, 13, 7, 13, 8
		};

		const std::string locTestTexture = "Frameworks/textures/panda.jpg";
		const std::string locTestglTFModel = "Frameworks/models/CesiumMan/CesiumMan.gltf";

		void locSpawnModels()
		{
			if (locProps.size() == 0)
			{
				glTFProp* prop1 = new glTFProp(locTestglTFModel);
				prop1->Spawn();
				locProps.push_back(prop1);

				DynamicProp* prop2 = new DynamicProp();
				prop2->SetGeometry(locVertices, locIndices);
				prop2->SetTexture(locTestTexture);
				prop2->Spawn();
				locProps.push_back(prop2);
			}
		}

		void locDespawnModels()
		{
			for (Prop* prop : locProps)
			{
				prop->Despawn();
			}
			locProps.clear();
		}

		void locSetupLights()
		{
			locLights.resize(2);
			locLights[0].SetPosition(glm::vec3(-47.0f, 26.0f, 46.0f));
			locLights[0].SetColor(glm::vec3(0.0f, 0.0f, 1.0f));
			locLights[0].SetIntensity(1.0f);
			locLights[1].SetPosition(glm::vec3(47.0f, -26.0f, -46.0f));
			locLights[1].SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
			locLights[1].SetIntensity(1.0f);
		}

		void locSetupCamera()
		{
			locCamera = new Camera();
			locCamera->SetPosition(glm::vec3(0.0f, -0.75f, -2.0f));
			locCamera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
			locCamera->SetPerspective(800.0f / 600.0f, 60.0f, 0.1f, 256.0f);
		}
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
		//myWindow2 = glfwCreateWindow(locWindowWidth, locWindowHeight, "Panda Project v0.12", nullptr, nullptr);

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->AddWindow(myWindow);
		inputManager->SetupCallbacks(inputManager->GetWindowId(myWindow));

		locSetupCamera();

		Render::Facade::Create();
		Render::Facade::GetInstance()->InitializeRendering();
		Render::Facade::GetInstance()->RegisterWindow(myWindow, Render::RendererType::Deferred);
		//Render::Facade::GetInstance()->RegisterWindow(myWindow2, Render::RendererType::Deferred);

		locSpawnModels();
		locSetupLights();
	}

	GameWork::~GameWork()
	{
		locDespawnModels();

		//Render::Facade::GetInstance()->UnregisterWindow(myWindow2);
		Render::Facade::GetInstance()->UnregisterWindow(myWindow);
		Render::Facade::GetInstance()->FinalizeRendering();
		Render::Facade::Destroy();

		delete locCamera;

		Input::InputManager::GetInstance()->RemoveWindow(myWindow);

		//glfwDestroyWindow(myWindow2);
		glfwDestroyWindow(myWindow);
		
		glfwTerminate();

		Input::InputManager::Destroy();
	}

	bool GameWork::Update()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		bool escapePressed = inputManager->PollRawInput(Input::RawInput::KeyEscape) == Input::RawInputState::Pressed;

		if (inputManager->PollRawInput(Input::RawInput::KeyI) == Input::RawInputState::Pressed)
		{
			locDespawnModels();
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyO) == Input::RawInputState::Pressed)
		{
			locSpawnModels();
		}
		
		// Update Modules
		for (Module* mod : myModules)
		{
			mod->OnUpdate();
		}
		
		Render::Facade::GetInstance()->StartFrame();

		locRenderer = Render::Facade::GetInstance()->GetRenderer(myWindow);
		//locRenderer2 = Render::Facade::GetInstance()->GetRenderer(myWindow2);

		locCamera->Update();
		locCamera->Bind(locRenderer);
		//locCamera->Rotate(glm::vec3(180.0f, 0.0f, 0.0f));
		//locCamera->Bind(locRenderer2);
		//locCamera->Rotate(glm::vec3(-180.0f, 0.0f, 0.0f));

		for (Prop* prop : locProps)
		{
			//prop->Translate(glm::vec3(0.001f));
			//prop->Rotate(glm::vec3(0.1f));
			prop->Update();
			prop->Draw(locRenderer);
		}

		for (PointLight& light : locLights)
		{
			light.Bind(locRenderer);
		}

		Render::Facade::GetInstance()->EndFrame();

		return !escapePressed;
	}
}
