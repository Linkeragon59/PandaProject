#pragma once

#include "RenderSwapChain.h"
#include "VulkanImage.h"

struct GLFWwindow;

namespace Render::Vulkan
{
	class SwapChain : public Render::SwapChain
	{
	public:
		SwapChain(GLFWwindow* aWindow);
		~SwapChain();

		void Setup() override;
		void Cleanup() override;
		void Recreate() override;

		void AcquireNext() override;
		void Present() override;

	private:
		void SetupVkSwapChain();
		void CleanupVkSwapChain();

		void CreateSyncObjects();
		void DestroySyncObjects();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
		uint myCurrentImageIndex = 0;
		std::vector<Image> myImages;
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;

		// One per in flight frame
		uint myMaxInFlightFrames = 0;
		uint myCurrentInFlightFrame = 0;
		std::vector<VkSemaphore> myImageAvailableSemaphores;
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
	};
}
