#pragma once

#include "VulkanImage.h"
#include "VulkanModel.h"

struct GLFWwindow;

namespace Render
{
namespace glTF
{
	class Model;
}
namespace Vulkan
{
	struct Device;
	class SwapChain;

	class Renderer
	{
	public:
		// Shortcut using the Facade singleton
		static Renderer* GetInstance();

		Renderer();
		~Renderer();

		void Init();
		void Finalize();

		void OnWindowOpened(GLFWwindow* aWindow);
		void OnWindowClosed(GLFWwindow* aWindow);
		void OnSetWindowView(GLFWwindow* aWindow, const glm::mat4& aView, const glm::mat4& aProjection);

		void Update();

		VkInstance GetVkInstance() const { return myVkInstance; }

		Device* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		uint GetModelsCount() const { return (uint)myModels.size(); }
		Model* GetModel(uint anIndex) const { return myModels[anIndex]; }

		uint SpawnModel(const std::string& aFilePath, const RenderData& aRenderData);
		void DespawnModel(uint anIndex);

	private:
		void CreateVkInstance();
		void CreateDevice();

		void SetupEmptyTexture();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		Device* myDevice = nullptr;

		Image myMissingTexture;

		// One SwapChain per window/surface
		std::vector<SwapChain*> mySwapChains;

		// This should be per swapchain
		std::vector<Model*> myModels;
		struct DespawningModel
		{
			Model* myModel = nullptr;
			uint myFramesToKeep = 1;
		};
		std::vector<DespawningModel> myDespawningModels;
	};
}
}
