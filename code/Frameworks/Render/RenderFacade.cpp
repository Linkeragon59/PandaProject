#include "RenderFacade.h"

#include "VulkanRender.h"

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
		delete ourInstance;
		ourInstance = nullptr;
	}

	void Facade::InitializeRendering()
	{
		Vulkan::RenderCore::Create();
	}

	void Facade::FinalizeRendering()
	{
		Vulkan::RenderCore::Destroy();
	}

	void Facade::StartFrame()
	{
		Vulkan::RenderCore::GetInstance()->StartFrame();
	}

	void Facade::EndFrame()
	{
		Vulkan::RenderCore::GetInstance()->EndFrame();
	}

	void Facade::RegisterWindow(GLFWwindow* aWindow, Renderer::Type aRendererType)
	{
		Vulkan::RenderCore::GetInstance()->RegisterWindow(aWindow, aRendererType);
	}

	void Facade::UnregisterWindow(GLFWwindow* aWindow)
	{
		Vulkan::RenderCore::GetInstance()->UnregisterWindow(aWindow);
	}

	Renderer* Facade::GetRenderer(GLFWwindow* aWindow)
	{
		return Vulkan::RenderCore::GetInstance()->GetRenderer(aWindow);
	}

	Model* Facade::SpawnModel(const BaseModelData& someData)
	{
		return Vulkan::RenderCore::GetInstance()->SpawnModel(someData);
	}

	void Facade::DespawnModel(Model* aModel)
	{
		Vulkan::RenderCore::GetInstance()->DespawnModel(aModel);
	}
}
