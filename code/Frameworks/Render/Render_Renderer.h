#pragma once

#include "Render_Light.h"
#include "Render_Model.h"

struct ImGuiContext;

namespace Render
{
	class Renderer
	{
	public:
		enum class Type
		{
			Invalid,
			Deferred,
#if DEBUG_BUILD
			Editor,
#endif
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
		virtual void DrawGui(ImGuiContext* aGuiContext) = 0; // TODO : Add a Gui class
	};
}
