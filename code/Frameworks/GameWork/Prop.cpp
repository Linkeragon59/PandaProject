#include "Prop.h"

#include "RenderFacade.h"
#include "RenderModel.h"
#include "RenderRenderer.h"

namespace GameWork
{
	Prop::Prop()
		: Entity()
	{
	}

	Prop::~Prop()
	{
		if (myIsSpawned)
			Despawn();

		delete myModelData;
	}

	void Prop::Update()
	{
		if (!myIsSpawned)
			return;

		Assert(myModel);
		myModel->Update(*myModelData);
	}

	void Prop::Draw(Render::Renderer* aRenderer)
	{
		if (!myIsSpawned || !myIsVisible)
			return;

		Assert(myModel);
		aRenderer->DrawModel(myModel, *myModelData);
	}

	void Prop::Spawn()
	{
		if (myIsSpawned)
			return;

		myModel = Render::Facade::GetInstance()->SpawnModel(*myModelData);
		Assert(myModel);

		myIsSpawned = true;
	}

	void Prop::Despawn()
	{
		if (!myIsSpawned)
			return;

		Assert(myModel);
		Render::Facade::GetInstance()->DespawnModel(myModel);
		myModel = nullptr;

		myIsSpawned = false;
	}

	void Prop::SetScale(float aScale)
	{
		myScale = aScale;
		UpdateModelDataMatrix();
	}

	void Prop::Scale(float aScaleMultiplier)
	{
		myScale *= aScaleMultiplier;
		UpdateModelDataMatrix();
	}

	void Prop::OnPositionChanged()
	{
		UpdateModelDataMatrix();
	}

	void Prop::OnRotationChanged()
	{
		UpdateModelDataMatrix();
	}

	void Prop::UpdateModelDataMatrix()
	{
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(-myRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		myModelData->myMatrix = glm::translate(rotationMatrix, myPosition);
		myModelData->myMatrix[3][3] = myScale;
	}
}
