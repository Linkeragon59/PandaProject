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

	void Facade::RegisterWindow(GLFWwindow* aWindow)
	{
		Vulkan::RenderCore::GetInstance()->RegisterWindow(aWindow);
	}

	void Facade::UnregisterWindow(GLFWwindow* aWindow)
	{
		Vulkan::RenderCore::GetInstance()->UnregisterWindow(aWindow);
	}

	Render::Renderer* Facade::CreateRenderer(RendererType aRendererType)
	{
		return Vulkan::RenderCore::GetInstance()->CreateRenderer(aRendererType);
	}

	void Facade::DestroyRenderer(Renderer* aRenderer)
	{
		Vulkan::RenderCore::GetInstance()->DestroyRenderer(aRenderer);
	}

	void Facade::StartFrame()
	{
		Vulkan::RenderCore::GetInstance()->StartFrame();
	}

	void Facade::EndFrame()
	{
		Vulkan::RenderCore::GetInstance()->EndFrame();
	}

}
