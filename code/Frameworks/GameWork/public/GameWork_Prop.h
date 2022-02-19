#pragma once

#include "GameWork_Entity.h"
#include "Render_Renderer.h"
#include "Render_Handle.h"
#include "Render_ModelData.h"

namespace GameWork
{
	class Prop : public Entity
	{
	public:
		virtual ~Prop();
		void Init(const Render::ModelData& someData);

		void Update() override;
		void Draw(Render::Renderer* aRenderer, Render::Renderer::DrawType aDrawType = Render::Renderer::DrawType::Default);

		void Spawn();
		void Despawn();

		void Hide() { myIsVisible = false; }
		void Show() { myIsVisible = true; }
		bool IsVisible() const { return myIsVisible; }

	protected:
		virtual Render::ModelData* CreateModelData(const Render::ModelData& someData) = 0;

		Render::Handle myModel = Render::NullHandle;
		Render::ModelData* myModelData = nullptr;

		bool myIsSpawned = false;
		bool myIsVisible = true;
	};
}
