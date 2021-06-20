#include "RenderFacade.h"

#include "VulkanCamera.h"
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

	void Facade::InitRenderer()
	{
		myVulkanRenderer = new Vulkan::Renderer;
		myVulkanRenderer->Init();
	}

	void Facade::UpdateRenderer()
	{
		myVulkanRenderer->Update();
	}

	void Facade::FinalizeRenderer()
	{
		myVulkanRenderer->Finalize();
		delete myVulkanRenderer;
		myVulkanRenderer = nullptr;
	}

	void Facade::OpenWindow(GLFWwindow* aWindow)
	{
		myVulkanRenderer->OnWindowOpened(aWindow);
	}

	void Facade::CloseWindow(GLFWwindow* aWindow)
	{
		myVulkanRenderer->OnWindowClosed(aWindow);
	}

	void Facade::SetWindowView(GLFWwindow* aWindow, const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myVulkanRenderer->OnSetWindowView(aWindow, aView, aProjection);
	}

	uint Facade::SpawnModel(const std::string& aFilePath, const RenderData& aRenderData)
	{
		return myVulkanRenderer->SpawnModel(aFilePath, aRenderData);
	}

	void Facade::DespawnModel(uint anIndex)
	{
		myVulkanRenderer->DespawnModel(anIndex);
	}

	Facade::Facade()
	{
	}

	Facade::~Facade()
	{
		Assert(!myVulkanRenderer);
	}
}
