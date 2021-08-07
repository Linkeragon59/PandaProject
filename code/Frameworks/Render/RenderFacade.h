#pragma once

#include "RenderModel.h"
#include "RendererType.h"

struct GLFWwindow;

namespace Render
{
	class Facade
	{
	public:
		static void Create();
		static void Destroy();
		static Facade* GetInstance() { return ourInstance; }

		void InitializeRendering();
		void FinalizeRendering();

		void RegisterWindow(GLFWwindow* aWindow, RendererType aRendererType);
		void UnregisterWindow(GLFWwindow* aWindow);

		void SetViewProj(GLFWwindow* aWindow, const glm::mat4& aView, const glm::mat4& aProjection);
		Model* SpawnModel(const glTFModelData& someData);
		void DrawModel(GLFWwindow* aWindow, const Model* aModel, const glTFModelData& someData);

		void StartFrame();
		void EndFrame();

	private:
		static Facade* ourInstance;
	};
}
