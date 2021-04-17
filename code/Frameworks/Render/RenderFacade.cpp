#include "RenderFacade.h"
#include "VulkanRenderer.h"

namespace Render
{
	void Facade::InitVulkanRenderer(GLFWwindow* aWindow)
	{
		Vulkan::Renderer::CreateInstance();
		Vulkan::Renderer::GetInstance()->OnWindowOpened(aWindow);
	}

	void Facade::UpdateVulkanRenderer()
	{
		Vulkan::Renderer::GetInstance()->Update();
	}

	void Facade::FinalizeVulkanRenderer(GLFWwindow* aWindow)
	{
		Vulkan::Renderer::GetInstance()->OnWindowClosed(aWindow);
		Vulkan::Renderer::DestroyInstance();
	}
}
