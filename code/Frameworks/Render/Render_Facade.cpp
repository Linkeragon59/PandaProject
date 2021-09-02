#include "Render_Facade.h"

namespace Render
{
	Facade* Facade::ourInstance = nullptr;

	void Facade::Create()
	{
		Assert(!ourInstance);
		ourInstance = new Facade;
	}

	void Facade::Destroy()
	{
		Assert(ourInstance);
		SafeDelete(ourInstance);
	}

	void Facade::InitializeRendering()
	{
		RenderCore::Create();
	}

	void Facade::FinalizeRendering()
	{
		RenderCore::Destroy();
	}

	void Facade::StartFrame()
	{
		RenderCore::GetInstance()->StartFrame();
	}

	void Facade::EndFrame()
	{
		RenderCore::GetInstance()->EndFrame();
	}

	void Facade::RegisterWindow(GLFWwindow* aWindow, Renderer::Type aRendererType)
	{
		RenderCore::GetInstance()->RegisterWindow(aWindow, aRendererType);
	}

	void Facade::UnregisterWindow(GLFWwindow* aWindow)
	{
		RenderCore::GetInstance()->UnregisterWindow(aWindow);
	}

	void Facade::ResizeWindow(GLFWwindow* aWindow)
	{
		RenderCore::GetInstance()->ResizeWindow(aWindow);
	}

	Renderer* Facade::GetRenderer(GLFWwindow* aWindow)
	{
		return RenderCore::GetInstance()->GetRenderer(aWindow);
	}

	Model* Facade::SpawnModel(const ModelData& someData)
	{
		return RenderCore::GetInstance()->SpawnModel(someData);
	}

	void Facade::DespawnModel(Model* aModel)
	{
		RenderCore::GetInstance()->DespawnModel(aModel);
	}
}
