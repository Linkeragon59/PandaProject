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

		void BuildCommandBuffer(uint32_t anImageIndex);
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
		std::vector<bool> myCommandBuffersDirty;
		std::vector<VkFramebuffer> myFramebuffers;

		// One per in flight frame
		uint32_t myMaxInFlightFrames = 0;
		uint32_t myCurrentInFlightFrame = 0;
		std::vector<VkSemaphore> myImageAvailableSemaphores;
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
		std::vector<VkFence> myInFlightFrameFences;
	};
}
}
