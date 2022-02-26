#include "GameCore_Facade.h"

#include "GameCore_ModuleManager.h"
#include "GameCore_TimeModule.h"
#include "GameCore_WindowModule.h"
#include "GameCore_InputModule.h"

#include "GameCore_Entity.h"
#include "GameCore_EntityModule.h"
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

	Facade* Facade::ourInstance = nullptr;

	bool Facade::Create()
	{
		Assert(!ourInstance);
		ourInstance = new Facade;
		return ourInstance->Initialize();
	}

	void Facade::Destroy()
	{
		Assert(ourInstance);
		ourInstance->Finalize();
		SafeDelete(ourInstance);
	}

	void Facade::Run()
	{
		while (!glfwWindowShouldClose(myMainWindow))
		{
			glfwPollEvents();

			if (!Update())
				break;
		}
	}

	Facade::Facade()
	{
		myModuleManager = new ModuleManager();
	}

	Facade::~Facade()
	{
		delete myModuleManager;
	}

	bool Facade::Initialize()
	{
		TimeModule::Register();
		WindowModule::Register();
		InputModule::Register();

		myMainWindow = WindowModule::GetInstance()->OpenWindow("Panda Engine");
		myWindowResizeCallbackId = WindowModule::GetInstance()->AddWindowSizeCallback([this](int aWidth, int aHeight) {
			myMainWindowAspectRatio = (aHeight != 0) ? (float)aWidth / (float)aHeight : 1.0f;
			}, myMainWindow);

		int width = 0, height = 0;
		glfwGetWindowSize(myMainWindow, &width, &height);
		myMainWindowAspectRatio = (height != 0) ? (float)width / (float)height : 1.0f;

		//		myCameraManager = new CameraManager();
		//		myPropManager = new PropManager();
		//#if DEBUG_BUILD
		//		myDebugPropManager = new PropManager(Render::Renderer::DrawType::Debug);
		//		Render::SimpleGeometryModelData modelData;
		//		modelData.FillWithPreset(Render::SimpleGeometryModelData::Preset::VectorBaseWidget);
		//		modelData.myTextureFilename = "Frameworks/textures/white.png";
		//		myVectorBase = myDebugPropManager->Spawn(modelData);
		//#endif
		//
		myNodeRegister = new NodeRegister();
		EntityModule::Register();

		Entity entity = Entity::Create();
		if (entity.HasComponent<Entity3DTransformComponent>())
		{
			{
				Entity3DTransformComponent* component = entity.AddComponent<Entity3DTransformComponent>(glm::vec3(1.0f));
				component->SetPosition(glm::vec3(1.f));
			}
			{
				Entity3DTransformComponent* component = entity.GetComponent<Entity3DTransformComponent>();
				component->SetPosition(glm::vec3(1.f));
			}
			entity.RemoveComponent<Entity3DTransformComponent>();
		}
		entity.Destroy();

		locSoloud.init(); // Initialize SoLoud
		locWave.load(locTestWavFile.c_str()); // Load a wave

		return true;
	}

	void Facade::Finalize()
	{
		locSoloud.deinit(); // Clean up!

//		delete myCameraManager;
//		delete myPropManager;
//#if DEBUG_BUILD
//		myDebugPropManager->Despawn(myVectorBase);
//		delete myDebugPropManager;
//#endif
//
		SafeDelete(myNodeRegister);
		EntityModule::Unregister();

		WindowModule::GetInstance()->RemoveWindowSizeCallback(myWindowResizeCallbackId);
		WindowModule::GetInstance()->CloseWindow(myMainWindow);
		myMainWindow = nullptr;

		InputModule::Unregister();
		WindowModule::Unregister();
		TimeModule::Unregister();
	}

	bool Facade::Update()
	{
		myModuleManager->Update(Module::UpdateType::EarlyUpdate);

		bool escapePressed = InputModule::GetInstance()->PollKeyInput(Input::KeyEscape) == Input::Status::Pressed;

		myModuleManager->Update(Module::UpdateType::MainUpdate);

//		myCameraManager->Update();
//		myPropManager->Update();
//#if DEBUG_BUILD
//		myDebugPropManager->Update();
//#endif

		myModuleManager->Update(Module::UpdateType::LateUpdate);

		return !escapePressed;
	}
}
