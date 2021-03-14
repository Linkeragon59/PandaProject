#pragma once

#include "VulkanImage.h"

struct GLFWwindow;

namespace Render
{
	struct VulkanDevice;
	class VulkanSwapChain;

	class VulkanCamera;
	class VulkanModel;

	namespace glTF
	{
		class Model;
	}

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

		const VkDescriptorImageInfo* GetEmptyTextureDescriptor() const { return &myEmptyTexture.myDescriptor; }

		VulkanCamera* GetCamera() const { return myCamera; }
		VulkanModel* GetPandaModel(uint32_t anIndex) const { return myPandaModels[anIndex]; }
		uint32_t GetPandaModelsCount() const { return (uint32_t)myPandaModels.size(); }
		glTF::Model* GetglTFModel() const { return myglTFModel; }

	private:
		static VulkanRenderer* ourInstance;

		VulkanRenderer();
		~VulkanRenderer();

		void CreateVkInstance();
		void SetupDebugMessenger();
		void CreateDevice();

		void SetupEmptyTexture();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		VulkanDevice* myDevice = nullptr;

		// One SwapChain per window/surface
		std::vector<VulkanSwapChain*> mySwapChains;

		VulkanImage myEmptyTexture;

		VulkanCamera* myCamera = nullptr;
		std::vector<VulkanModel*> myPandaModels;
		glTF::Model* myglTFModel = nullptr;
	};
}
