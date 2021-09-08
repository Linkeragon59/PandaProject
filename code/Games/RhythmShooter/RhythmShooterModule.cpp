#include "RhythmShooterModule.h"

#include "GameWork.h"
#include "GameWork_CameraManager.h"
#include "GameWork_Camera.h"
#include "GameWork_PropManager.h"
#include "GameWork_Prop.h"

#include "Render_Renderer.h"
#include "Base_Input.h"

RhythmShooterModule* RhythmShooterModule::ourInstance = nullptr;

void RhythmShooterModule::OnRegister()
{
	GameWork::CameraManager* cameraManager = GameWork::GameWork::GetInstance()->GetCameraManager();

	myCamera = cameraManager->AddCamera();
	myCamera->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
	myCamera->SetDirection(glm::vec3(0.0f, 0.0f, -1.0f));

	GameWork::PropManager* propManager = GameWork::GameWork::GetInstance()->GetPropManager();

	{
		Render::SimpleGeometryModelData modelData;
		modelData.FillWithPreset(Render::SimpleGeometryModelData::Preset::Sphere);
		modelData.myTextureFilename = "Games/RhythmShooter/textures/earth.png";
		mySimpleGeometryTest = propManager->Spawn(modelData, glm::vec3(-1.5f, 0.0f, 0.0f));
	}

	{
		Render::glTFModelData modelData;
		modelData.myFilename = "Games/RhythmShooter/models/cube/Cube.gltf";
		myTestModel = propManager->Spawn(modelData, glm::vec3(0.0f, 2.5f, 0.0f));
	}

	{
		Render::glTFModelData modelData;
		modelData.myFilename = "Frameworks/models/CesiumMan/CesiumMan.gltf";
		//modelData.myFilename = "Games/RhythmShooter/models/Asteroid01/Asteroid01.gltf";
		myTestAnimatedModel = propManager->Spawn(modelData, glm::vec3(0.0f, 0.0f, 1.0f));
	}
}

void RhythmShooterModule::OnUnregister()
{
	GameWork::CameraManager* cameraManager = GameWork::GameWork::GetInstance()->GetCameraManager();

	cameraManager->RemoveCamera(myCamera);
	myCamera = nullptr;

	GameWork::PropManager* propManager = GameWork::GameWork::GetInstance()->GetPropManager();

	propManager->Despawn(mySimpleGeometryTest);
	mySimpleGeometryTest = nullptr;

	propManager->Despawn(myTestModel);
	myTestModel = nullptr;

	propManager->Despawn(myTestAnimatedModel);
	myTestAnimatedModel = nullptr;
}

void RhythmShooterModule::OnUpdate()
{
	Input::InputManager* inputManager = Input::InputManager::GetInstance();
	if (inputManager->PollKeyInput(Input::KeyR) == Input::Status::Pressed)
	{
		myTestModel->Rotate(0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (inputManager->PollKeyInput(Input::KeyE) == Input::Status::Pressed)
	{
		myTestModel->Scale(glm::vec3(1.01f, 1.0f, 0.99f));
	}
}
