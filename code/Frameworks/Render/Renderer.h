#pragma once

struct GLFWwindow;

namespace Render
{
	enum class RendererType
	{
		Deferred,
	};

	class Model;

	class Renderer
	{
	public:
		Renderer() {}
		virtual ~Renderer() {}

		virtual void StartFrame(GLFWwindow* aWindow) = 0;
		virtual void EndFrame() = 0;
		virtual void UpdateView(const glm::mat4& aView, const glm::mat4& aProjection) = 0;
		virtual void DrawModel(Model* aModel) = 0;
	};
}
