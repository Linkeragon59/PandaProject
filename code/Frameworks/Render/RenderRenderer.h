#pragma once

#include "RenderModel.h"

namespace Render
{
	enum class RendererType
	{
		Invalid,
		Deferred,
	};

	class Renderer
	{
	public:
		virtual void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection) = 0;
		virtual void DrawModel(const Model* aModel, const glTFModelData& someData) = 0;
	};
}
