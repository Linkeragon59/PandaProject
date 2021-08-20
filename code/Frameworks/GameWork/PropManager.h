#pragma once

#include "RenderModel.h"
#include "RenderRenderer.h"

namespace GameWork
{
	class Prop;

	class PropManager
	{
	public:
		PropManager(Render::Renderer::DrawType aDrawType = Render::Renderer::DrawType::Default);
		~PropManager();

		void Update();

		Prop* Spawn(const Render::ModelData& someData,
			const glm::vec3& aPosition = glm::vec3(0.0f),
			const glm::vec3& anOrientation = glm::vec3(0.0f),
			const glm::vec3& aScale = glm::vec3(1.0f));
		void Despawn(Prop* aProp);

	private:
		std::vector<Prop*> myProps;
		Render::Renderer::DrawType myDrawType;
	};
}
