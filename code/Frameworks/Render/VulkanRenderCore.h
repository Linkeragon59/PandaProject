#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"

#include "VulkanglTFModel.h"

struct GLFWwindow;

namespace Render
{
	struct VulkanDevice;
	class VulkanSwapChain;

	class VulkanRenderCore
	{
	public:
		static void CreateInstance();
		static void DestroyInstance();

		static GLFWwindow* OpenWindow(int aWidth, int aHeight, const char* aTitle);
		static void CloseWindow(GLFWwindow* aWindow);

		static VulkanRenderCore* GetInstance() { return ourInstance; }

		VkInstance GetVkInstance() const { return myVkInstance; }

		VulkanDevice* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		vkglTF::Model* GetDummyScene() const { return myScene; }

	private:
		static VulkanRenderCore* ourInstance;

		VulkanRenderCore();
		~VulkanRenderCore();

		void CreateVkInstance();
		void SetupDebugMessenger();
		void CreateDevice();

		void LoadDummyScene();
		void UnloadDummyScene();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		VulkanDevice* myDevice = nullptr;

		// One SwapChain per window/surface
		std::vector<VulkanSwapChain*> mySwapChains;

		vkglTF::Model* myScene;
	};
}
