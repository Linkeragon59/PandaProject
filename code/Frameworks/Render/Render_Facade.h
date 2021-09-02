#pragma once

#include "Render_Model.h"
#include "Render_Renderer.h"

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

		void RegisterWindow(GLFWwindow* aWindow, Renderer::Type aRendererType);
		void UnregisterWindow(GLFWwindow* aWindow);
		void ResizeWindow(GLFWwindow* aWindow);
		Renderer* GetRenderer(GLFWwindow* aWindow);

		Model* SpawnModel(const ModelData& someData);
		void DespawnModel(Model* aModel);

	private:
		static Facade* ourInstance;
	};
}
