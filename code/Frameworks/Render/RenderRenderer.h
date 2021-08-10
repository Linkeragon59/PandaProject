#pragma once

#include "RenderLight.h"
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
		virtual void DrawModel(const Model* aModel, const BaseModelData& someData) = 0;
		virtual void AddLight(const PointLight& aPointLight) = 0;
		// TODO
		// DrawUI
	};
}
