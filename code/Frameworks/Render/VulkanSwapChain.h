#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct GLFWwindow;

namespace Render
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(GLFWwindow* aWindow);
		~VulkanSwapChain();

		// TODO: Call on window resize
		void Setup();
		void AcquireNextImage();
		void Cleanup();

		GLFWwindow* GetWindow() const { return myWindow; }
		VkSurfaceKHR GetSurface() const { return mySurface; }

	private:
		friend class VulkanRenderCore;

		void SetupVkSwapChain();
		void SetupDepthStencil();
		void CreatePipelineCache();
		void SetupRenderPass();
		void SetupCommandBuffers();
		void SetupFramebuffers();
		void PrepareUniformBuffers();
		void SetupDescriptorSetLayout();
		void PreparePipelines();
		void SetupDescriptorPool();
		void SetupDescriptorSet();
		void BuildCommandBuffers();
		void Draw();

		GLFWwindow* myWindow = nullptr;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;
		VkDevice myDevice = VK_NULL_HANDLE;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> myImages;
		std::vector<VkImageView> myImageViews;
		VkFormat myFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D myExtent{ 0, 0 };
		VulkanImage myDepthImage;

		VkPipelineCache myPipelineCache = VK_NULL_HANDLE;

		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> myCommandBuffers;

		std::vector<VkFramebuffer> myFramebuffers;

		// Same uniform buffer layout as shader
		struct UBOVS
		{
			glm::mat4 myProjection;
			glm::mat4 myModelView;
			glm::vec4 myLightPos;
		};
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
