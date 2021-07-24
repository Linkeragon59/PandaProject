#pragma once

struct GLFWwindow;

#include "Renderer.h"

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

		void RegisterWindow(GLFWwindow* aWindow);
		void UnregisterWindow(GLFWwindow* aWindow);

		Renderer* CreateRenderer(RendererType aRendererType);
		void DestroyRenderer(Renderer* aRenderer);

		void StartFrame();
		void EndFrame();

		//Model* SpawnModel(const std::string& aFilePath, const Model::RenderData& aRenderData);

	private:
		static Facade* ourInstance;
	};
}
