#include "RhythmShooterModule.h"

#include "GameCore_Facade.h"
//#include "GameCore_CameraManager.h"
//#include "GameCore_Camera.h"
//#include "GameCore_PropManager.h"
//#include "GameCore_Prop.h"

#include "Render_EntityRenderComponent.h"
#include "GameCore_EntityModule.h"
#include "GameCore_InputModule.h"

DEFINE_GAMECORE_MODULE(RhythmShooterModule);

void RhythmShooterModule::OnRegister()
{
	//GameCore::CameraManager* cameraManager = GameCore::Facade::GetInstance()->GetCameraManager();
	//
	//myCamera = cameraManager->AddCamera();
	//myCamera->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
	//myCamera->SetDirection(glm::vec3(0.0f, 0.0f, -1.0f));

	{
		mySimpleGeometryTest = GameCore::Entity::Create();
		GameCore::Entity3DTransformComponent* transformComponent = mySimpleGeometryTest.AddComponent<GameCore::Entity3DTransformComponent>(glm::vec3(0.f));
		transformComponent->SetPosition(glm::vec3(-1.5f, 0.0f, 0.0f));
		Render::EntitySimpleGeometryModelComponent* modelComponent = mySimpleGeometryTest.AddComponent<Render::EntitySimpleGeometryModelComponent>();
		modelComponent->FillWithPreset(Render::EntitySimpleGeometryModelComponent::Preset::Sphere);
		modelComponent->myTextureFilename = "Games/RhythmShooter/textures/earth.png";
		modelComponent->Load();
	}

	/*{
		Render::glTFModelData modelData;
		modelData.myFilename = "Games/RhythmShooter/models/cube/Cube.gltf";
		myTestModel = propManager->Spawn(modelData, glm::vec3(0.0f, 2.5f, 0.0f));
	}

	{
		Render::glTFModelData modelData;
		modelData.myFilename = "Frameworks/models/CesiumMan/CesiumMan.gltf";
		//modelData.myFilename = "Games/RhythmShooter/models/Asteroid01/Asteroid01.gltf";
		myTestAnimatedModel = propManager->Spawn(modelData, glm::vec3(0.0f, 0.0f, 1.0f));
	}*/
}

void RhythmShooterModule::OnUnregister()
{
	//GameCore::CameraManager* cameraManager = GameCore::Facade::GetInstance()->GetCameraManager();
	//
	//cameraManager->RemoveCamera(myCamera);
	//myCamera = nullptr;

	Render::EntitySimpleGeometryModelComponent* modelComponent = mySimpleGeometryTest.GetComponent<Render::EntitySimpleGeometryModelComponent>();
	modelComponent->Unload();
	mySimpleGeometryTest.RemoveComponent<Render::EntitySimpleGeometryModelComponent>();
	mySimpleGeometryTest.RemoveComponent<GameCore::Entity3DTransformComponent>();
	mySimpleGeometryTest.Destroy();

	/*propManager->Despawn(myTestModel);
	myTestModel = nullptr;

	propManager->Despawn(myTestAnimatedModel);
	myTestAnimatedModel = nullptr;*/
}

void RhythmShooterModule::OnUpdate(GameCore::Module::UpdateType aType)
{
	if (aType == GameCore::Module::UpdateType::MainUpdate)
	{
		/*Input::InputManager* inputManager = Input::InputManager::GetInstance();
		if (inputManager->PollKeyInput(Input::KeyR) == Input::Status::Pressed)
		{
			myTestModel->Rotate(0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (inputManager->PollKeyInput(Input::KeyE) == Input::Status::Pressed)
		{
			myTestModel->Scale(glm::vec3(1.01f, 1.0f, 0.99f));
		}*/
	}
}
