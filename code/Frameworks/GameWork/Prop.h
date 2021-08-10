#pragma once

#include "Entity.h"

namespace Render
{
	struct BaseModelData;
	class Model;
	class Renderer;
}

namespace GameWork
{
	class Prop : public Entity
	{
	public:
		Prop();
		virtual ~Prop();

		void Update() override;
		void Draw(Render::Renderer* aRenderer);

		void Spawn();
		void Despawn();

		void SetScale(float aScale);
		void Scale(float aScaleMultiplier);
		float GetScale() const { return myScale; }

		void Hide() { myIsVisible = false; }
		void Show() { myIsVisible = true; }
		bool IsVisible() const { return myIsVisible; }

	protected:
		void OnPositionChanged() override;
		void OnRotationChanged() override;

		void UpdateModelDataMatrix();

		Render::BaseModelData* myModelData = nullptr;
		Render::Model* myModel = nullptr;

		bool myIsSpawned = false;

		float myScale = 1.0f;
		bool myIsVisible = true;
	};
}
