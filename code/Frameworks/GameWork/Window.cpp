#include "Window.h"

#include "RenderFacade.h"
#include "Input.h"
#include "Camera.h"
#include "Gui.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace GameWork
{
	Window::Window(uint aWidth, uint aHeight, const char* aTitle)
	{
		myWindow = glfwCreateWindow(aWidth, aHeight, aTitle, nullptr, nullptr);

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->AddWindow(myWindow);
		inputManager->SetupCallbacks(inputManager->GetWindowId(myWindow));

		myCamera = new Camera();
		myCamera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
		//myCamera->SetTarget(glm::vec3(0.0f, 0.0f, 0.0f));
		myCamera->SetPerspective(800.0f / 600.0f, 60.0f, 0.1f, 256.0f);
	}

	Window::~Window()
	{
		delete myCamera;

		Input::InputManager::GetInstance()->RemoveWindow(myWindow);

		glfwDestroyWindow(myWindow);
	}

	void Window::Init()
	{
		//Render::Facade::GetInstance()->OpenWindow(myWindow);

		// Temp
		LoadTestAssets();
	}

	bool Window::Update()
	{
		myCamera->Update();
		//Render::Facade::GetInstance()->SetWindowView(myWindow, myCamera->GetViewMatrix(), myCamera->GetPerspectiveMatrix());

		return !ShouldClose();
	}

	void Window::Finalize()
	{
		// Temp
		UnloadTestAssets();

		//Render::Facade::GetInstance()->CloseWindow(myWindow);
	}

	bool Window::ShouldClose() const
	{
		return glfwWindowShouldClose(myWindow);
	}

	void Window::GetSize(int& anOutWidth, int& anOutHeight) const
	{
		glfwGetWindowSize(myWindow, &anOutWidth, &anOutHeight);
	}

	void Window::LoadTestAssets()
	{
		if (myTempAssetsLoaded)
			return;

		/*{
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
		}*/
	}

	void Window::UnloadTestAssets()
	{
		if (!myTempAssetsLoaded)
			return;

		/*Render::Facade::GetInstance()->DespawnModel(myCastleModel);
		myCastleModel = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myCastleWindows);
		myCastleWindows = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myAvocadoModel);
		myAvocadoModel = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myAnimatedModel);
		myAnimatedModel = UINT_MAX;
		Render::Facade::GetInstance()->DespawnModel(myDummyModel);
		myDummyModel = UINT_MAX;*/
	}
}
