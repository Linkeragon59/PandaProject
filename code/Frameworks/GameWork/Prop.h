#pragma once

#include "Entity.h"
#include "RenderRenderer.h"

namespace Render
{
	struct BaseModelData;
	class Model;
}

namespace GameWork
{
	class Prop : public Entity
	{
	public:
		Prop();
		virtual ~Prop();

		void Update() override;
		void Draw(Render::Renderer* aRenderer, Render::Renderer::DrawType aDrawType = Render::Renderer::DrawType::Normal);

		void Spawn();
		void Despawn();

		void Hide() { myIsVisible = false; }
		void Show() { myIsVisible = true; }
		bool IsVisible() const { return myIsVisible; }

	protected:
		Render::BaseModelData* myModelData = nullptr;
		Render::Model* myModel = nullptr;

		bool myIsSpawned = false;

		float myScale = 1.0f;
		bool myIsVisible = true;
	};
}
