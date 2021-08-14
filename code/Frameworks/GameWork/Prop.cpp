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
		myModelData->myMatrix = GetMatrix();
		myModel->Update(*myModelData);
	}

	void Prop::Draw(Render::Renderer* aRenderer, Render::Renderer::DrawType aDrawType)
	{
		if (!myIsSpawned || !myIsVisible)
			return;

		Assert(myModel);
		aRenderer->DrawModel(myModel, *myModelData, aDrawType);
	}

	void Prop::Spawn()
	{
		if (myIsSpawned)
			return;

		myModelData->myMatrix = GetMatrix();
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
}
