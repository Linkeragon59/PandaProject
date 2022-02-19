#include "GameWork_Prop.h"

#include "Render_Facade.h"

namespace GameWork
{
	Prop::~Prop()
	{
		if (myIsSpawned)
			Despawn();

		delete myModelData;
	}

	void Prop::Init(const Render::ModelData& someData)
	{
		myModelData = CreateModelData(someData);
	}

	void Prop::Update()
	{
		if (!myIsSpawned)
			return;

		myModelData->myMatrix = GetMatrix();
		Render::UpdateModel(myModel, *myModelData);
	}

	void Prop::Draw(Render::Renderer* aRenderer, Render::Renderer::DrawType aDrawType)
	{
		if (!myIsSpawned || !myIsVisible)
			return;

		aRenderer->DrawModel(myModel, *myModelData, aDrawType);
	}

	void Prop::Spawn()
	{
		if (myIsSpawned)
			return;

		myModelData->myMatrix = GetMatrix();
		myModel = Render::AddModel(*myModelData);

		myIsSpawned = true;
	}

	void Prop::Despawn()
	{
		if (!myIsSpawned)
			return;

		Render::RemoveModel(myModel);
		myModel = Render::NullHandle;

		myIsSpawned = false;
	}
}
