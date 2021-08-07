#pragma once

#include "RenderModel.h"
#include "RenderRenderer.h"

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

		void StartFrame();
		void EndFrame();

		void RegisterWindow(GLFWwindow* aWindow, RendererType aRendererType);
		void UnregisterWindow(GLFWwindow* aWindow);
		Renderer* GetRenderer(GLFWwindow* aWindow);

		Model* SpawnModel(const glTFModelData& someData);
		void DespawnModel(Model* aModel);

	private:
		static Facade* ourInstance;
	};
}
