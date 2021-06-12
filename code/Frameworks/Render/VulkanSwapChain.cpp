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

		Setup();
	}

	SwapChain::~SwapChain()
	{
		Cleanup();

		vkDestroySurfaceKHR(Renderer::GetInstance()->GetVkInstance(), mySurface, nullptr);
	}

	void SwapChain::Setup()
	{
		SetupVkSwapChain();
		SetupDepthStencil();

		myDeferredPipeline.Prepare(myExtent, myColorFormat, myDepthImage.myFormat);
		myUIOverlay.Prepare(myDeferredPipeline.myRenderPass);

		SetupCommandBuffers();
		SetupFramebuffers();

		for (uint i = 0; i < (uint)myCommandBuffers.size(); ++i)
			BuildCommandBuffer(i);

		CreateSyncObjects();
	}

	void SwapChain::Cleanup()
	{
		vkDeviceWaitIdle(myDevice);

		for (uint i = 0; i < myMaxInFlightFrames; ++i)
		{
			vkDestroyFence(myDevice, myInFlightFrameFences[i], nullptr);
			vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(myDevice, myImageAvailableSemaphores[i], nullptr);
		}
		myInFlightFrameFences.clear();
		myRenderFinishedSemaphores.clear();
		myImageAvailableSemaphores.clear();

		for (auto framebuffer : myFramebuffers)
			vkDestroyFramebuffer(myDevice, framebuffer, nullptr);
		myFramebuffers.clear();

		myCommandBuffers.clear();
		myCommandBuffersDirty.clear();

		myUIOverlay.Destroy();
		myDeferredPipeline.Destroy();

		myDepthImage.Destroy();
		for (auto imageView : myImageViews)
			vkDestroyImageView(myDevice, imageView, nullptr);
		myImageViews.clear();
		myImages.clear();

		vkDestroySwapchainKHR(myDevice, myVkSwapChain, nullptr);
		myVkSwapChain = VK_NULL_HANDLE;
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

		// TODO: Maybe we don't have to recreate everything?
		Cleanup();
		Setup();
	}

	void SwapChain::Update()
	{
		myDeferredPipeline.Update();
		myUIOverlay.Update(myExtent.width, myExtent.height);
		DrawFrame();
	}

	void SwapChain::SetDirty()
	{
		for (uint i = 0; i < (uint)myCommandBuffers.size(); ++i)
			myCommandBuffersDirty[i] = true;
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

		VK_CHECK_RESULT(vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &myVkSwapChain), "Failed to create a swap chain!");

		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, nullptr);
		myImages.resize(imageCount);
		vkGetSwapchainImagesKHR(myDevice, myVkSwapChain, &imageCount, myImages.data());

		myImageViews.resize(myImages.size());
		for (size_t i = 0, e = myImages.size(); i < e; ++i)
		{
			VkImageViewCreateInfo viewCreateInfo{};
			viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCreateInfo.image = myImages[i];
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCreateInfo.format = myColorFormat;
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

			VK_CHECK_RESULT(vkCreateImageView(myDevice, &viewCreateInfo, nullptr, &myImageViews[i]), "Failed to create an image view for the swap chain!");
		}

		myMaxInFlightFrames = imageCount - 1;
	}

	void SwapChain::SetupDepthStencil()
	{
		VkFormat depthFormat = Renderer::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();

		myDepthImage.Create(myExtent.width, myExtent.height,
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

	void SwapChain::SetupCommandBuffers()
	{
		myCommandBuffers.resize(myImages.size());
		myCommandBuffersDirty.resize(myImages.size(), true);
		
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = Renderer::GetInstance()->GetGraphicsCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint)myCommandBuffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()), "Failed to create command buffers!");
	}

	void SwapChain::SetupFramebuffers()
	{
		myFramebuffers.resize(myImages.size());

		std::array<VkImageView, 5> attachments{};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = myDeferredPipeline.myRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = myExtent.width;
		framebufferInfo.height = myExtent.height;
		framebufferInfo.layers = 1;

		for (size_t i = 0, e = myFramebuffers.size(); i < e; ++i)
		{
			attachments[0] = myImageViews[i];
			attachments[1] = myDeferredPipeline.myPositionAttachement.myImageView;
			attachments[2] = myDeferredPipeline.myNormalAttachement.myImageView;
			attachments[3] = myDeferredPipeline.myAlbedoAttachement.myImageView;
			attachments[4] = myDepthImage.myImageView;
			VK_CHECK_RESULT(vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &myFramebuffers[i]), "Failed to create a framebuffer!");
		}
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

	void SwapChain::BuildCommandBuffer(uint anImageIndex)
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		std::array<VkClearValue, 5> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[3].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[4].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = myDeferredPipeline.myRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = myExtent.width;
		renderPassBeginInfo.renderArea.extent.height = myExtent.height;
		renderPassBeginInfo.clearValueCount = (uint)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = myFramebuffers[anImageIndex];

		VK_CHECK_RESULT(vkBeginCommandBuffer(myCommandBuffers[anImageIndex], &cmdBufferBeginInfo), "Failed to begin a command buffer");

		vkCmdBeginRenderPass(myCommandBuffers[anImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myExtent.width;
		viewport.height = (float)myExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(myCommandBuffers[anImageIndex], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = myExtent;
		scissor.offset = { 0, 0 };
		vkCmdSetScissor(myCommandBuffers[anImageIndex], 0, 1, &scissor);

		Camera* camera = Renderer::GetInstance()->GetCamera();

		// First sub pass
		// Renders the components of the scene to the G-Buffer attachments
		Debug::BeginRegion(myCommandBuffers[anImageIndex], "Subpass 0: Deferred G-Buffer creation", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		{
			vkCmdBindPipeline(myCommandBuffers[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipeline);

			camera->BindViewProj(myCommandBuffers[anImageIndex], myDeferredPipeline.myGBufferPipelineLayout, 0);
			for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
			{
				if (Model* model = Renderer::GetInstance()->GetModel(i))
				{
					if (!model->IsTransparent())
						model->Draw(myCommandBuffers[anImageIndex], myDeferredPipeline.myGBufferPipelineLayout, 1);
				}
			}
		}
		Debug::EndRegion(myCommandBuffers[anImageIndex]);

		// Second sub pass
		// This subpass will use the G-Buffer components as input attachment for the lighting
		Debug::BeginRegion(myCommandBuffers[anImageIndex], "Subpass 1: Deferred composition", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		{
			vkCmdNextSubpass(myCommandBuffers[anImageIndex], VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(myCommandBuffers[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipeline);

			vkCmdBindDescriptorSets(myCommandBuffers[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 0, 1, &myDeferredPipeline.myLightingDescriptorSet, 0, NULL);
			vkCmdDraw(myCommandBuffers[anImageIndex], 4, 1, 0, 0);
		}
		Debug::EndRegion(myCommandBuffers[anImageIndex]);

		// Third subpass
		// Render transparent geometry using a forward pass that compares against depth generated during G-Buffer fill
		Debug::BeginRegion(myCommandBuffers[anImageIndex], "Subpass 2: Forward transparency", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		{
			vkCmdNextSubpass(myCommandBuffers[anImageIndex], VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(myCommandBuffers[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipeline);

			vkCmdBindDescriptorSets(myCommandBuffers[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipelineLayout, 0, 1, &myDeferredPipeline.myTransparentDescriptorSet, 0, NULL);
			camera->BindViewProj(myCommandBuffers[anImageIndex], myDeferredPipeline.myTransparentPipelineLayout, 1);
			for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
			{
				if (Model* model = Renderer::GetInstance()->GetModel(i))
				{
					if (model->IsTransparent())
						model->Draw(myCommandBuffers[anImageIndex], myDeferredPipeline.myTransparentPipelineLayout, 2);
				}
			}
		}
		Debug::EndRegion(myCommandBuffers[anImageIndex]);
		Debug::BeginRegion(myCommandBuffers[anImageIndex], "Subpass 2b: UI Overlay", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		{
			myUIOverlay.Draw(myCommandBuffers[anImageIndex]);
		}
		Debug::EndRegion(myCommandBuffers[anImageIndex]);

		vkCmdEndRenderPass(myCommandBuffers[anImageIndex]);

		VK_CHECK_RESULT(vkEndCommandBuffer(myCommandBuffers[anImageIndex]), "Failed to end a command buffer");

		myCommandBuffersDirty[anImageIndex] = false;
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

		if (myCommandBuffersDirty[imageIndex])
			BuildCommandBuffer(imageIndex);

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
}
}
