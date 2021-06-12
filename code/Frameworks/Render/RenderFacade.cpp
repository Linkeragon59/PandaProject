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

	void Facade::InitRenderer(GLFWwindow* aWindow)
	{
		myVulkanRenderer = new Vulkan::Renderer;
		myVulkanRenderer->Init();
		myVulkanRenderer->OnWindowOpened(aWindow);
	}

	void Facade::UpdateRenderer(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myVulkanRenderer->Update(aView, aProjection);
	}

	void Facade::FinalizeRenderer(GLFWwindow* aWindow)
	{
		myVulkanRenderer->OnWindowClosed(aWindow);
		myVulkanRenderer->Finalize();
		delete myVulkanRenderer;
		myVulkanRenderer = nullptr;
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
