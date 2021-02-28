#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>

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
		void SetupCommandBuffers();
		void SetupFramebuffers();

		GLFWwindow* myWindow = nullptr;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> myImages;
		std::vector<VkImageView> myImageViews;
		VkFormat myFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D myExtent{ 0, 0 };

		VkImage myDepthImage = VK_NULL_HANDLE;
		VmaAllocation myDepthImageAlloc = VK_NULL_HANDLE;
		VkImageView myDepthImageView = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> myCommandBuffers;

		std::vector<VkFramebuffer> myFramebuffers;
	};
}
