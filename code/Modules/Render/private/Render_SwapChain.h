#pragma once

#include "Render_Renderer.h"
#include "Render_VulkanImage.h"

struct GLFWwindow;

namespace Render
{
	class Renderer;

	class SwapChain
	{
	public:
		SwapChain(GLFWwindow* aWindow, RendererType aRendererType);
		~SwapChain();
		
		void Setup();
		void Cleanup();
		void Recreate();

		void AcquireNext();
		void Present();

		GLFWwindow* GetWindowHandle() const { return myWindow; }

		uint GetImagesCount() const { return (uint)myImages.size(); };
		VkExtent2D GetExtent() const { return myExtent; }
		VkFormat GetColorFormat() const { return myColorFormat; }
		VkImageView GetCurrentRenderTarget() const { return myImages[myCurrentImageIndex].myImageView; }
		VkSemaphore GetCurrentRenderTargetSemaphore() const { return myCurrentImageAvailableSemaphore; }

		Renderer* GetRenderer() const;

	private:
		void SetupVkSwapChain();
		void CleanupVkSwapChain();

		void CreateSyncObjects();
		void DestroySyncObjects();

		void CreateRenderer();
		void DestroyRenderer();

		VkDevice myDevice = VK_NULL_HANDLE;

		GLFWwindow* myWindow = nullptr;
		uint myFramebufferResizedCallbackId = UINT_MAX;
		bool myFramebufferResized = false;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
		std::vector<VulkanImage> myImages;
		uint myCurrentImageIndex = 0;
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		std::vector<VkSemaphore> myImageAvailableSemaphores;
		VkSemaphore myCurrentImageAvailableSemaphore = VK_NULL_HANDLE;

		// For now, one renderer per swapchain
		RendererType myRendererType = RendererType::Count;
		Renderer* myRenderer = nullptr;
	};
}
