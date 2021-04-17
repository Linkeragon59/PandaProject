#pragma once

#include "VulkanImage.h"
#include "VulkanDeferredPipeline.h"

struct GLFWwindow;

namespace Render
{
namespace Vulkan
{
	class SwapChain
	{
	public:
		SwapChain(GLFWwindow* aWindow);
		~SwapChain();

		void Setup();
		void Cleanup();
		void Recreate();

		void Update();

		GLFWwindow* GetWindow() const { return myWindow; }
		VkSurfaceKHR GetSurface() const { return mySurface; }

	private:
		static void FramebufferResizedCallback(GLFWwindow* aWindow, int aWidth, int aHeight);

		void SetupVkSwapChain();
		void SetupDepthStencil();

		void SetupCommandBuffers();
		void SetupFramebuffers();
		void CreateSyncObjects();

		void BuildCommandBuffers();
		void DrawFrame();

		GLFWwindow* myWindow = nullptr;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;
		VkDevice myDevice = VK_NULL_HANDLE;

		bool myFramebufferResized = false;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> myImages;
		std::vector<VkImageView> myImageViews;
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D myExtent{ 0, 0 };
		Image myDepthImage;

		DeferredPipeline myDeferredPipeline;

		// One per swapchain image
		std::vector<VkCommandBuffer> myCommandBuffers;
		std::vector<VkFramebuffer> myFramebuffers;
		std::vector<VkFence> myImageFences;

		// One per in flight image
		std::vector<VkSemaphore> myImageAvailableSemaphores;
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
		std::vector<VkFence> myInFlightFrameFences;
		uint32_t myCurrentInFlightFrame = 0;
	};
}
}
