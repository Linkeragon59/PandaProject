#include "VulkanSwapChain.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"
#include "VulkanDebugMessenger.h"
#include "VulkanCamera.h"

#include "VulkanglTFModel.h"
#include "DummyModel.h"

#include <GLFW/glfw3.h>
#include <random>

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
		myLightsUBO.Create(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myLightsUBO.SetupDescriptor();
		myLightsUBO.Map();
		SetupRandomLights();
		UpdateLightsUBO();
		SetupLightsDescriptorPool();
		SetupLightsDescriptorSets();

		Setup();
	}

	SwapChain::~SwapChain()
	{
		Cleanup();

		delete myCamera;
		myLightsUBO.Destroy();
		vkDestroyDescriptorPool(myDevice, myLightsDescriptorPool, nullptr);

		vkDestroySurfaceKHR(Renderer::GetInstance()->GetVkInstance(), mySurface, nullptr);
	}

	void SwapChain::Setup()
	{
		SetupVkSwapChain();
		SetupDepthStencil();
		CreateSyncObjects();
		SetupCommandBuffers();

		myDeferredRenderContext.Setup(myImages, myDepthImage);
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

	void SwapChain::UpdateView(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myCamera->Update(aView, aProjection);
		UpdateLightsUBO();
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

		VkExtent2D extent = {};
		VkSurfaceCapabilitiesKHR capabilities = {};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mySurface, &capabilities);
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			extent = capabilities.currentExtent;
		}
		else
		{
			int width = 0, height = 0;
			glfwGetWindowSize(myWindow, &width, &height);
			extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint)width));
			extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint)height));
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
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
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
			myImages[i].myFormat = surfaceFormat.format;
			myImages[i].myExtent = { extent.width, extent.height, 1 };

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

		myMaxInFlightFrames = imageCount - 1;
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
		vkWaitForFences(myDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame], VK_TRUE, UINT64_MAX);

		VkSemaphore imageAvailableSemaphore = myImageAvailableSemaphores[myCurrentInFlightFrame];
		VkSemaphore renderCompleteSemaphore = myRenderFinishedSemaphores[myCurrentInFlightFrame];

		uint imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(myDevice, myVkSwapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Recreate();
			return;
		}
		else
		{
			Assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire a swapchain image!");
		}

		myDeferredRenderContext.BuildCommandBuffers(myCommandBuffers[imageIndex], imageIndex, myCamera, myLightsDescriptorSet);

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[imageIndex];
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
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(Renderer::GetInstance()->GetGraphicsQueue(), &presentInfo);
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

	void SwapChain::UpdateLightsUBO()
	{
		glm::vec3 cameraPosition = myCamera->GetView()[3];
		myLightsData.myViewPos = glm::vec4(cameraPosition, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
		memcpy(myLightsUBO.myMappedData, &myLightsData, sizeof(LightData));
	}

	void SwapChain::SetupRandomLights()
	{
		std::vector<glm::vec3> colors =
		{
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 0.0f),
		};

		std::default_random_engine rndGen((uint)time(nullptr));
		std::uniform_real_distribution<float> rndDist(-10.0f, 10.0f);
		std::uniform_int_distribution<uint> rndCol(0, static_cast<uint>(colors.size() - 1));

		for (Light& light : myLightsData.myLights)
		{
			light.myPosition = glm::vec4(rndDist(rndGen) * 6.0f, 0.25f + std::abs(rndDist(rndGen)) * 4.0f, rndDist(rndGen) * 6.0f, 1.0f);
			light.myColor = colors[rndCol(rndGen)];
			light.myRadius = 1.0f + std::abs(rndDist(rndGen));
		}
	}

	void SwapChain::SetupLightsDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myLightsDescriptorPool), "Failed to create the descriptor pool");
	}

	void SwapChain::SetupLightsDescriptorSets()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { ShaderHelpers::GetLightsDescriptorSetLayout() };
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myLightsDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myLightsDescriptorSet), "Failed to create the node descriptor set");

		std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = myLightsDescriptorSet;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].pBufferInfo = &myLightsUBO.myDescriptor;
		writeDescriptorSets[0].descriptorCount = 1;
		vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
	}
}
}
