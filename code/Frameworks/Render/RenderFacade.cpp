#include "RenderFacade.h"
#include "BasicRenderer.h"
#include "VulkanRenderer.h"

namespace Render
{
	BasicRenderer* Facade::ourBasicRenderer = nullptr;

	void Facade::InitBasicRenderer(GLFWwindow* aWindow)
	{
		ourBasicRenderer = new BasicRenderer(aWindow);
	}

	void Facade::UpdateBasicRenderer()
	{
		ourBasicRenderer->Update();
	}

	void Facade::FinalizeBasicRenderer()
	{
		delete ourBasicRenderer;
	}

	void Facade::InitVulkanRenderer(GLFWwindow* aWindow)
	{
		VulkanRenderer::CreateInstance();
		VulkanRenderer::GetInstance()->OnWindowOpened(aWindow);
	}

	void Facade::UpdateVulkanRenderer()
	{
		VulkanRenderer::GetInstance()->Update();
	}

	void Facade::FinalizeVulkanRenderer(GLFWwindow* aWindow)
	{
		VulkanRenderer::GetInstance()->OnWindowClosed(aWindow);
		VulkanRenderer::DestroyInstance();
	}

}
