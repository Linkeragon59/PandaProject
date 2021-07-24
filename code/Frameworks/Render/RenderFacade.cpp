#include "RenderFacade.h"

#include "VulkanCamera.h"
#include "VulkanRenderer.h"

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

#ifdef __linux__
#pragma GCC diagnostic pop
#endif

#if defined(_WINDOWS)
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4324)
#elif defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

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
