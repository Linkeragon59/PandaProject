#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>
#include <optional>
#include <memory>

#include "VulkanglTFModel.h"
#include "VulkanBuffer.h"

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

		VkRenderPass GetRenderPass() const { return myRenderPass; }

	private:
		static VulkanRenderCore* ourInstance;

		VulkanRenderCore();
		~VulkanRenderCore();

		void CreateVkInstance();
		void SetupDebugMessenger();
		void CreateDevice();
		void SetupRenderPass();
		void CreatePipelineCache();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		VulkanDevice* myDevice = nullptr;

		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		VkPipelineCache myPipelineCache = VK_NULL_HANDLE;

		// One SwapChain per window/surface
		std::vector<VulkanSwapChain*> mySwapChains;

		// Below stuff is per SwapChain

		void PrepareDummyScene();
		void LoadDummyScene();
		void PrepareUniformBuffers();
		void SetupDescriptorSetLayout();
		void PreparePipelines();
		void SetupDescriptorPool();
		void SetupDescriptorSet();
		void BuildCommandBuffers();
		void Draw();

		// Same uniform buffer layout as shader
		struct UBOVS
		{
			glm::mat4 myProjection;
			glm::mat4 myModelView;
			glm::vec4 myLightPos;
		};
		vkglTF::Model myScene;
		VulkanBuffer myUniform;

		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;

		VkPipeline myPhongPipeline = VK_NULL_HANDLE;
		VkPipeline myToonPipeline = VK_NULL_HANDLE;
		VkPipeline myWireFramePipeline = VK_NULL_HANDLE;
	};
}
