#include "RhythmShooterModule.h"

#include "GameWork.h"
#include "Camera.h"
#include "DynamicProp.h"
#include "glTFProp.h"
#include "RenderRenderer.h"
#include "Input.h"

RhythmShooterModule* RhythmShooterModule::ourInstance = nullptr;

namespace
{
}

void RhythmShooterModule::OnRegister()
{
	myCamera = new GameWork::Camera();
	myCamera->SetPosition(glm::vec3(0.0f, 0.0f, 3.0f));
	myCamera->SetDirection(glm::vec3(0.0f, 0.0f, -1.0f));
	myCamera->SetPerspective(800.0f / 600.0f, 45.0f, 0.1f, 256.0f);

	const std::vector<Render::DynamicModelData::Vertex> vertices =
	{
		{{-5.0f, -5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
		{{-5.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
		{{5.0f, 5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
		{{5.0f, -5.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
	};
	const std::vector<uint> indices =
	{
		0, 2, 1, 2, 0, 3
	};
	myShooterPlane = new GameWork::DynamicProp();
	myShooterPlane->SetGeometry(vertices, indices);
	myShooterPlane->SetTexture("Games/RhythmShooter/textures/space.png");
	myShooterPlane->Spawn();

	myTestModel = new GameWork::glTFProp("Games/RhythmShooter/models/duck/Duck.gltf");
	myTestModel->Spawn();

	myTestAnimatedModel = new GameWork::glTFProp("Frameworks/models/CesiumMan/CesiumMan.gltf");
	myTestAnimatedModel->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
	myTestAnimatedModel->Spawn();

#if DEBUG_BUILD
	std::vector<Render::DynamicModelData::Vertex> vectorVertices;
	std::vector<uint> vectorIndices;
	Render::DynamicModelData::GetVectorBaseWidget(vectorVertices, vectorIndices);
	myVectorBase = new GameWork::DynamicProp();
	myVectorBase->SetGeometry(vectorVertices, vectorIndices);
	myVectorBase->SetTexture("Games/RhythmShooter/textures/white.png");
	myVectorBase->Spawn();
#endif
}

void RhythmShooterModule::OnUnregister()
{
	delete myCamera;
	myCamera = nullptr;

	myShooterPlane->Despawn();
	delete myShooterPlane;
	myShooterPlane = nullptr;

	myTestModel->Despawn();
	delete myTestModel;
	myTestModel = nullptr;

	myTestAnimatedModel->Despawn();
	delete myTestAnimatedModel;
	myTestAnimatedModel = nullptr;

#if DEBUG_BUILD
	myVectorBase->Despawn();
	delete myVectorBase;
	myVectorBase = nullptr;
#endif
}

void RhythmShooterModule::OnUpdate()
{
	Render::Renderer* renderer = GameWork::GameWork::GetInstance()->GetMainRenderer();
	myCamera->Update();
	myCamera->Bind(renderer);

	myShooterPlane->Update();
	myShooterPlane->Draw(renderer);

	Input::InputManager* inputManager = Input::InputManager::GetInstance();
	if (inputManager->PollRawInput(Input::RawInput::KeyT) == Input::RawInputState::Pressed)
	{
		myTestModel->Translate(0.01f * myTestModel->GetDirection());
	}
	if (inputManager->PollRawInput(Input::RawInput::KeyR) == Input::RawInputState::Pressed)
	{
		myTestModel->Rotate(0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (inputManager->PollRawInput(Input::RawInput::KeyE) == Input::RawInputState::Pressed)
	{
		myTestModel->Scale(glm::vec3(1.01f, 1.0f, 0.99f));
	}
	myTestModel->Update();
	myTestModel->Draw(renderer);

	myTestAnimatedModel->Update();
	myTestAnimatedModel->Draw(renderer);

#if DEBUG_BUILD
	myVectorBase->Update();
	myVectorBase->Draw(renderer, Render::Renderer::DrawType::Debug);
#endif
}
