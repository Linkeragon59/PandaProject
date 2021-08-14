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
		};

		enum class DrawType
		{
			Normal,
#if DEBUG_BUILD
			Debug,
#endif
		};

		virtual void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection) = 0;
		virtual void DrawModel(Model* aModel, const BaseModelData& someData, DrawType aDrawType = DrawType::Normal) = 0;
		virtual void AddLight(const PointLight& aPointLight) = 0;
		// TODO
		// DrawUI
	};
}
