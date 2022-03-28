#include "RhythmShooterModule.h"

#include "GameCore_EntityCameraComponent.h"
#include "GameCore_EntityTransformComponent.h"
#include "Render_EntityRenderComponent.h"
#include "GameCore_EntityModule.h"
#include "GameCore_InputModule.h"

DEFINE_GAMECORE_MODULE(RhythmShooterModule);

void RhythmShooterModule::OnRegister()
{
	myCamera = GameCore::Entity::Create();
	GameCore::EntityCameraComponent* cameraComponent = myCamera.AddComponent<GameCore::EntityCameraComponent>();
	cameraComponent->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
	cameraComponent->SetDirection(glm::vec3(0.0f, 0.0f, -1.0f));

	{
		mySimpleGeometryTest = GameCore::Entity::Create();
		mySimpleGeometryTest.AddComponent<GameCore::Entity3DTransformComponent>(glm::vec3(-1.5f, 0.0f, 0.0f));
		Render::EntitySimpleGeometryModelComponent* modelComponent = mySimpleGeometryTest.AddComponent<Render::EntitySimpleGeometryModelComponent>();
		modelComponent->FillWithPreset(Render::EntitySimpleGeometryModelComponent::Preset::Sphere);
		modelComponent->myTextureFilename = "Games/RhythmShooter/textures/earth.png";
		modelComponent->Load();

		Render::EntityGuiComponent* guiComponent = mySimpleGeometryTest.AddComponent<Render::EntityGuiComponent>();
		guiComponent->Load();
	}

	{
		myTestModel = GameCore::Entity::Create();
		myTestModel.AddComponent<GameCore::Entity3DTransformComponent>(glm::vec3(0.0f, 2.5f, 0.0f));
		Render::EntityglTFModelComponent* modelComponent = myTestModel.AddComponent<Render::EntityglTFModelComponent>();
		modelComponent->myFilename = "Games/RhythmShooter/models/cube/Cube.gltf";
		modelComponent->Load();
	}

	{
		myTestAnimatedModel = GameCore::Entity::Create();
		myTestAnimatedModel.AddComponent<GameCore::Entity3DTransformComponent>(glm::vec3(0.0f, 0.0f, 1.0f));
		Render::EntityglTFModelComponent* modelComponent = myTestAnimatedModel.AddComponent<Render::EntityglTFModelComponent>();
		modelComponent->myFilename = "Frameworks/models/CesiumMan/CesiumMan.gltf";
		//modelComponent->myFilename = "Games/RhythmShooter/models/Asteroid01/Asteroid01.gltf";
		modelComponent->Load();
	}
}

void RhythmShooterModule::OnUnregister()
{
	myCamera.Destroy();

	mySimpleGeometryTest.Destroy();
	myTestModel.Destroy();
	myTestAnimatedModel.Destroy();
}

void RhythmShooterModule::OnUpdate(GameCore::Module::UpdateType aType)
{
	if (aType == GameCore::Module::UpdateType::MainUpdate)
	{
		if (GameCore::Entity3DTransformComponent* component = myTestModel.GetComponent<GameCore::Entity3DTransformComponent>())
		{
			GameCore::InputModule* inputModule = GameCore::InputModule::GetInstance();
			if (inputModule->PollKeyInput(Input::KeyR) == Input::Status::Pressed)
			{
				component->Rotate(0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
			}
			if (inputModule->PollKeyInput(Input::KeyE) == Input::Status::Pressed)
			{
				component->Scale(glm::vec3(1.01f, 1.0f, 0.99f));
			}
		}
	}
}
