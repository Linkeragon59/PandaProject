#pragma once

#include "Render_Light.h"
#include "Render_Handle.h"

namespace Render
{
	class Renderer;

	class Renderer
	{
	public:
		enum class RendererType
		{
			Invalid,
			Deferred,
			Editor,
		};

		enum class DrawType
		{
			Default,
#if DEBUG_BUILD
			Debug,
#endif
		};

		Renderer(Type aType);
		~Renderer();

		void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection);
		void AddLight(const PointLight& aPointLight);
		void DrawModel(Handle aModelHandle, const ModelData& someData, DrawType aDrawType = DrawType::Default);
		void DrawGui(Handle aGuiHandle);

		Renderer* GetImpl() const { return myImpl; }

	private:
		Type myType = Type::Invalid;
		Renderer* myImpl;
	};
}
