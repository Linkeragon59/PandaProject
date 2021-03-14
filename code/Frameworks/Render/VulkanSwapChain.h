#pragma once

#include "VulkanImage.h"

struct GLFWwindow;

namespace Render
{
	class DummyVulkanPSO;
	namespace glTF
	{
		class VulkanPSO;
	}

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(GLFWwindow* aWindow);
		~VulkanSwapChain();

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

		void SetupRenderPass();

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
		VulkanImage myDepthImage;

		VkRenderPass myRenderPass = VK_NULL_HANDLE;
		DummyVulkanPSO* myDummyPSO = nullptr;
		glTF::VulkanPSO* myglTFPSO = nullptr;

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
