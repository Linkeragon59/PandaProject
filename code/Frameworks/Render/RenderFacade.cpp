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

	void Facade::RegisterWindow(GLFWwindow* aWindow, RendererType aRendererType)
	{
		Vulkan::RenderCore::GetInstance()->RegisterWindow(aWindow, aRendererType);
	}

	void Facade::UnregisterWindow(GLFWwindow* aWindow)
	{
		Vulkan::RenderCore::GetInstance()->UnregisterWindow(aWindow);
	}

	void Facade::SetViewProj(GLFWwindow* aWindow, const glm::mat4& aView, const glm::mat4& aProjection)
	{
		Vulkan::RenderCore::GetInstance()->SetViewProj(aWindow, aView, aProjection);
	}

	Model* Facade::SpawnModel(const glTFModelData& someData)
	{
		return Vulkan::RenderCore::GetInstance()->SpawnModel(someData);
	}

	void Facade::DrawModel(GLFWwindow* aWindow, const Model* aModel, const glTFModelData& someData)
	{
		Vulkan::RenderCore::GetInstance()->DrawModel(aWindow, aModel, someData);
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
