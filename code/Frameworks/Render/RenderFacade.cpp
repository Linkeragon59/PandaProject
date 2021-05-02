#include "RenderFacade.h"

#include "RenderCamera.h"
#include "VulkanRenderer.h"

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

	void Facade::InitVulkanRenderer(GLFWwindow* aWindow)
	{
		myVulkanRenderer = new Vulkan::Renderer;
		myVulkanRenderer->Init();
		myVulkanRenderer->OnWindowOpened(aWindow);
	}

	void Facade::UpdateVulkanRenderer()
	{
		myVulkanRenderer->Update();
	}

	void Facade::FinalizeVulkanRenderer(GLFWwindow* aWindow)
	{
		myVulkanRenderer->OnWindowClosed(aWindow);
		myVulkanRenderer->Finalize();
		delete myVulkanRenderer;
		myVulkanRenderer = nullptr;
	}

	Facade::Facade()
	{
		myCamera = new Camera;
	}

	Facade::~Facade()
	{
		delete myCamera;
		Assert(!myVulkanRenderer);
	}
}
