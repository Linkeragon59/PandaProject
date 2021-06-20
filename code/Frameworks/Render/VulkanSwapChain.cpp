#include "VulkanSwapChain.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanDebugMessenger.h"
#include "VulkanCamera.h"

#include "VulkanglTFModel.h"
#include "DummyModel.h"

#include <GLFW/glfw3.h>

namespace Render
{
namespace Vulkan
{
	SwapChain::SwapChain(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		myDevice = Renderer::GetInstance()->GetDevice();

		VK_CHECK_RESULT(glfwCreateWindowSurface(Renderer::GetInstance()->GetVkInstance(), myWindow, nullptr, &mySurface), "Failed to create the surface!");

		glfwSetWindowUserPointer(myWindow, this);
		glfwSetFramebufferSizeCallback(myWindow, FramebufferResizedCallback);

		myCamera = new Camera;

		Setup();
	}

	SwapChain::~SwapChain()
	{
		Cleanup();

		delete myCamera;

		vkDestroySurfaceKHR(Renderer::GetInstance()->GetVkInstance(), mySurface, nullptr);
	}

	void SwapChain::Setup()
	{
		SetupVkSwapChain();
		SetupDepthStencil();
		CreateSyncObjects();
		SetupCommandBuffers();

		myDeferredRenderContext.Setup(myExtent, myColorFormat, myDepthImage.myFormat);
	}

	void SwapChain::Cleanup()
	{
		vkDeviceWaitIdle(myDevice);

		myDeferredRenderContext.Destroy();

		CleanupCommandBuffers();
		DestroySyncObjects();
		CleanupDepthStencil();
		CleanupVkSwapChain();
	}

	void SwapChain::Recreate()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(myWindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwWaitEvents();
			glfwGetFramebufferSize(myWindow, &width, &height);
		}

		// TODO: Recreate only what's necessary
		Cleanup();
		Setup();
	}

	void SwapChain::StartFrame()
	{
		vkWaitForFences(myDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame], VK_TRUE, UINT64_MAX);

		VkSemaphore imageAvailableSemaphore = myImageAvailableSemaphores[myCurrentInFlightFrame];

		VkResult result = vkAcquireNextImageKHR(myDevice, myVkSwapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &myCurrentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Recreate();
			return;
		}
		else
		{
			Assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire a swapchain image!");
		}
	}

	void SwapChain::EndFrame()
	{
		VkSemaphore imageAvailableSemaphore = myImageAvailableSemaphores[myCurrentInFlightFrame];
		VkSemaphore renderCompleteSemaphore = myRenderFinishedSemaphores[myCurrentInFlightFrame];

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[myCurrentImageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore;

		vkResetFences(myDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame]);
		VK_CHECK_RESULT(vkQueueSubmit(Renderer::GetInstance()->GetGraphicsQueue(), 1, &submitInfo, myInFlightFrameFences[myCurrentInFlightFrame]),
			"Failed to submit a command buffer");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &myVkSwapChain;
		presentInfo.pImageIndices = &myCurrentImageIndex;

		VkResult result = vkQueuePresentKHR(Renderer::GetInstance()->GetGraphicsQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myFramebufferResized)
		{
			myFramebufferResized = false;
			Recreate();
		}
		else
		{
			Assert(result == VK_SUCCESS, "Failed to present a swapchain image!");
		}

		myCurrentInFlightFrame = (myCurrentInFlightFrame + 1) % myMaxInFlightFrames;
	}

	void SwapChain::UpdateView(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myCamera->Update(aView, aProjection);
		myDeferredRenderContext.UpdateLightsUBO(myCamera->GetView()[3]);
	}

	void SwapChain::Update()
	{
		DrawFrame();
	}

	void SwapChain::FramebufferResizedCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		(void)aWidth;
		(void)aHeight;
		auto app = reinterpret_cast<SwapChain*>(glfwGetWindowUserPointer(aWindow));
		app->myFramebufferResized = true;
	}

	void SwapChain::SetupVkSwapChain()
	{
		VkPhysicalDevice physicalDevice = Renderer::GetInstance()->GetPhysicalDevice();

		// TODO: Support separate queues for graphics and present.
		Assert(Renderer::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.has_value());
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			Renderer::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.value(),
			mySurface,
			&presentSupport);
		Assert(presentSupport, "The device doesn't support presenting on the graphics queue!");

		VkSurfaceCapabilitiesKHR capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mySurface, &capabilities);
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			myExtent = capabilities.currentExtent;
		}
		else
		{
			int width = 0, height = 0;
			glfwGetWindowSize(myWindow, &width, &height);
			myExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint)width));
			myExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint)height));
		}

		uint formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, nullptr);
		Assert(formatCount > 0);
		std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, availableFormats.data());
		VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				surfaceFormat = availableFormat;
		}
		myColorFormat = surfaceFormat.format;

		uint presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, nullptr);
		Assert(presentModeCount > 0);
		std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, availablePresentModes.data());
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = availablePresentMode;
		}

		uint imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0)
			imageCount = std::min(imageCount, capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mySurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = myColorFormat;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = myExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &myVkSwapChain), "Failed to create a swap chain!");

		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, nullptr);
		std::vector<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, images.data());

		myImages.resize(imageCount);
		for (uint i = 0; i < imageCount; ++i)
		{
			myImages[i].myImage = images[i];
			myImages[i].myFormat = myColorFormat;
			myImages[i].myExtent = { myExtent.width, myExtent.height, 1 };

			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = myImages[i].myImage;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = myImages[i].myFormat;
			viewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCreateInfo.subresourceRange.baseMipLevel = 0;
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseArrayLayer = 0;
			viewCreateInfo.subresourceRange.layerCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(myDevice, &viewCreateInfo, nullptr, &myImages[i].myImageView), "Failed to create an image view for the swap chain!");
		}
		myCurrentImageIndex = 0;

		myMaxInFlightFrames = imageCount - 1;
		myCurrentInFlightFrame = 0;
	}

	void SwapChain::CleanupVkSwapChain()
	{
		for (uint i = 0, e = (uint)myImages.size(); i < e; ++i)
		{
			vkDestroyImageView(myDevice, myImages[i].myImageView, nullptr);
			myImages[i].myImage = VK_NULL_HANDLE;
		}
		myImages.clear();

		vkDestroySwapchainKHR(myDevice, myVkSwapChain, nullptr);
		myVkSwapChain = VK_NULL_HANDLE;
	}

	void SwapChain::SetupDepthStencil()
	{
		VkFormat depthFormat = Renderer::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();

		myDepthImage.Create(myImages[0].myExtent.width, myImages[0].myExtent.height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkImageAspectFlags aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Image::DepthFormatHasStencilAspect(depthFormat))
			aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
		myDepthImage.CreateImageView(aspects);

		// optional
		//myDepthImage.TransitionLayout(
		//	VK_IMAGE_LAYOUT_UNDEFINED,
		//	VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		//	Renderer::GetInstance()->GetGraphicsQueue());
	}

	void SwapChain::CleanupDepthStencil()
	{
		myDepthImage.Destroy();
	}

	void SwapChain::CreateSyncObjects()
	{
		myImageAvailableSemaphores.resize(myMaxInFlightFrames);
		myRenderFinishedSemaphores.resize(myMaxInFlightFrames);
		myInFlightFrameFences.resize(myMaxInFlightFrames);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < myMaxInFlightFrames; ++i)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myImageAvailableSemaphores[i]), "Failed to create a semaphore");
			VK_CHECK_RESULT(vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myRenderFinishedSemaphores[i]), "Failed to create a semaphore");
			VK_CHECK_RESULT(vkCreateFence(myDevice, &fenceInfo, nullptr, &myInFlightFrameFences[i]), "Failed to create a fence");
		}
	}

	void SwapChain::DestroySyncObjects()
	{
		for (uint i = 0; i < myMaxInFlightFrames; ++i)
		{
			vkDestroyFence(myDevice, myInFlightFrameFences[i], nullptr);
			vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(myDevice, myImageAvailableSemaphores[i], nullptr);
		}
		myInFlightFrameFences.clear();
		myRenderFinishedSemaphores.clear();
		myImageAvailableSemaphores.clear();
	}

	void SwapChain::SetupCommandBuffers()
	{
		myCommandBuffers.resize(myImages.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = Renderer::GetInstance()->GetGraphicsCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint)myCommandBuffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()), "Failed to create command buffers!");
	}

	void SwapChain::CleanupCommandBuffers()
	{
		myCommandBuffers.clear();
	}

	void SwapChain::DrawFrame()
	{
		StartFrame();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myExtent.width;
		viewport.height = (float)myExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.extent = myExtent;
		scissor.offset = { 0, 0 };

		myDeferredRenderContext.Begin(myCommandBuffers[myCurrentImageIndex], myImages[myCurrentImageIndex].myImageView, myDepthImage.myImageView);
		myDeferredRenderContext.SetViewport(viewport);
		myDeferredRenderContext.SetScissor(scissor);
		myDeferredRenderContext.BindCamera(myCamera);
		for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
		{
			if (Model* model = Renderer::GetInstance()->GetModel(i))
			{
				myDeferredRenderContext.DrawModel(model);
			}
		}
		myDeferredRenderContext.End();
		
		EndFrame();
	}
}
}
