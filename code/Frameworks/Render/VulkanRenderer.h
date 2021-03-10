#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"

struct GLFWwindow;

namespace Render
{
	struct VulkanDevice;
	class VulkanSwapChain;

	class VulkanCamera;
	class VulkanModel;

	class VulkanRenderer
	{
	public:
		static void CreateInstance();
		static void DestroyInstance();
		static VulkanRenderer* GetInstance() { return ourInstance; }

		void OnWindowOpened(GLFWwindow* aWindow);
		void OnWindowClosed(GLFWwindow* aWindow);

		void Update();

		VkInstance GetVkInstance() const { return myVkInstance; }

		VulkanDevice* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		VulkanCamera* GetCamera() const { return myCamera; }
		VulkanModel* GetPandaModel(uint32_t anIndex) const { return myPandaModels[anIndex]; }
		uint32_t GetPandaModelsCount() const { return (uint32_t)myPandaModels.size(); }

	private:
		static VulkanRenderer* ourInstance;

		VulkanRenderer();
		~VulkanRenderer();

		void CreateVkInstance();
		void SetupDebugMessenger();
		void CreateDevice();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		VulkanDevice* myDevice = nullptr;

		// One SwapChain per window/surface
		std::vector<VulkanSwapChain*> mySwapChains;

		VulkanCamera* myCamera = nullptr;
		std::vector<VulkanModel*> myPandaModels;
	};
}
