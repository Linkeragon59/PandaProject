#include "VulkanSwapChain.h"

#include "VulkanRenderCore.h"
#include "VulkanHelpers.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <assert.h>

#include <array>

namespace Render
{
	VulkanSwapChain::VulkanSwapChain(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		if (glfwCreateWindowSurface(VulkanRenderCore::GetInstance()->GetVkInstance(), myWindow, nullptr, &mySurface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create the surface!");

		Setup();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		Cleanup();

		vkDestroySurfaceKHR(VulkanRenderCore::GetInstance()->GetVkInstance(), mySurface, nullptr);
	}

	void VulkanSwapChain::Setup()
	{
		SetupVkSwapChain();
		SetupDepthStencil();
		SetupCommandBuffers();
		SetupFramebuffers();
	}

	void VulkanSwapChain::Cleanup()
	{
		vkDeviceWaitIdle(VulkanRenderCore::GetInstance()->GetDevice());

		for (auto framebuffer : myFramebuffers)
			vkDestroyFramebuffer(VulkanRenderCore::GetInstance()->GetDevice(), framebuffer, nullptr);
		myFramebuffers.clear();

		vkDestroyImageView(VulkanRenderCore::GetInstance()->GetDevice(), myDepthImageView, nullptr);
		vmaDestroyImage(VulkanRenderCore::GetInstance()->GetAllocator(), myDepthImage, myDepthImageAlloc);
		myDepthImage = VK_NULL_HANDLE;
		myDepthImageAlloc = VK_NULL_HANDLE;
		myDepthImageView = VK_NULL_HANDLE;

		for (auto imageView : myImageViews)
			vkDestroyImageView(VulkanRenderCore::GetInstance()->GetDevice(), imageView, nullptr);
		myImageViews.clear();

		vkDestroySwapchainKHR(VulkanRenderCore::GetInstance()->GetDevice(), myVkSwapChain, nullptr);
		myVkSwapChain = VK_NULL_HANDLE;
	}

	void VulkanSwapChain::SetupVkSwapChain()
	{
		// TODO: Support separate queues for graphics and present.
		assert(VulkanRenderCore::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.has_value());
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			VulkanRenderCore::GetInstance()->GetPhysicalDevice(),
			VulkanRenderCore::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.value(),
			mySurface,
			&presentSupport);
		if (!presentSupport)
			throw std::runtime_error("The Graphics queue that was chosen when selecting the device doesn't have present support!");

		VkSurfaceCapabilitiesKHR capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanRenderCore::GetInstance()->GetPhysicalDevice(), mySurface, &capabilities);
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			myExtent = capabilities.currentExtent;
		}
		else
		{
			int width = 0, height = 0;
			glfwGetWindowSize(myWindow, &width, &height);
			myExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)width));
			myExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height));
		}

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanRenderCore::GetInstance()->GetPhysicalDevice(), mySurface, &formatCount, nullptr);
		assert(formatCount > 0);
		std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanRenderCore::GetInstance()->GetPhysicalDevice(), mySurface, &formatCount, availableFormats.data());
		VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				surfaceFormat = availableFormat;
		}
		myFormat = surfaceFormat.format;

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanRenderCore::GetInstance()->GetPhysicalDevice(), mySurface, &presentModeCount, nullptr);
		assert(presentModeCount > 0);
		std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanRenderCore::GetInstance()->GetPhysicalDevice(), mySurface, &presentModeCount, availablePresentModes.data());
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = availablePresentMode;
		}

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0)
			imageCount = std::min(imageCount, capabilities.maxImageCount);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mySurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
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

		if (vkCreateSwapchainKHR(VulkanRenderCore::GetInstance()->GetDevice(), &createInfo, nullptr, &myVkSwapChain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a swap chain!");

		vkGetSwapchainImagesKHR(VulkanRenderCore::GetInstance()->GetDevice(), myVkSwapChain, &imageCount, nullptr);
		myImages.resize(imageCount);
		vkGetSwapchainImagesKHR(VulkanRenderCore::GetInstance()->GetDevice(), myVkSwapChain, &imageCount, myImages.data());

		myImageViews.resize(myImages.size());
		for (size_t i = 0, e = myImages.size(); i < e; ++i)
		{
			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = myImages[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = myFormat;
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

			if (vkCreateImageView(VulkanRenderCore::GetInstance()->GetDevice(), &viewCreateInfo, nullptr, &myImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create an image view for the swap chain!");
		}
	}

	void VulkanSwapChain::SetupDepthStencil()
	{
		VkFormat depthFormat = VulkanRenderCore::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();

		CreateImage(
			myExtent.width,
			myExtent.height,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			myDepthImage,
			myDepthImageAlloc);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = myDepthImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
		};
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (VulkanRenderCore::GetInstance()->GetVulkanDevice()->HasStencilAspect(depthFormat))
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(VulkanRenderCore::GetInstance()->GetDevice(), &viewInfo, nullptr, &myDepthImageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a view for the depth image!");

		// optional
		/*TransitionImageLayout(
			VulkanRenderCore::GetInstance()->GetGraphicsQueue(),
			myDepthImage,
			bestDepthFormat,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);*/
	}

	void VulkanSwapChain::SetupCommandBuffers()
	{
		myCommandBuffers.resize(myImages.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = VulkanRenderCore::GetInstance()->GetGraphicsCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(myCommandBuffers.size());

		if (vkAllocateCommandBuffers(VulkanRenderCore::GetInstance()->GetDevice(), &allocInfo, myCommandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to create command buffers!");
	}

	void VulkanSwapChain::SetupFramebuffers()
	{
		myFramebuffers.resize(myImageViews.size());

		for (size_t i = 0, e = myFramebuffers.size(); i < e; ++i)
		{
			std::array<VkImageView, 2> attachments = { myImageViews[i], myDepthImageView };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = VulkanRenderCore::GetInstance()->GetRenderPass();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = myExtent.width;
			framebufferInfo.height = myExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanRenderCore::GetInstance()->GetDevice(), &framebufferInfo, nullptr, &myFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create a framebuffer!");
		}
	}

}
