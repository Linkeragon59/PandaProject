#pragma once

#include "RenderLight.h"
#include "RenderModel.h"

namespace Render
{
	class Renderer
	{
	public:
		enum class Type
		{
			Invalid,
			Deferred,
			Gui,
		};

		enum class DrawType
		{
			Default,
#if DEBUG_BUILD
			Debug,
#endif
		};

		virtual void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection) = 0;
		virtual void DrawModel(Model* aModel, const ModelData& someData, DrawType aDrawType = DrawType::Default) = 0;
		virtual void AddLight(const PointLight& aPointLight) = 0;
		virtual void DrawGui() = 0; // TODO : Add a Gui class
	};
}
