#include "VulkanSwapChain.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"
#include "VulkanDevice.h"

#include "VulkanCamera.h"
#include "glTFModel.h"
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

		//myDummyPSO = new DummyPSO(myRenderPass);
		//myglTFPSO = new glTF::VulkanPSO(myRenderPass);

		SetupCommandBuffers();
		SetupFramebuffers();

		CreateSyncObjects();

		BuildCommandBuffers();
	}

	void SwapChain::Cleanup()
	{
		vkDeviceWaitIdle(myDevice);

		for (uint32_t i = 0, e = (uint32_t)myImages.size() - 1; i < e; ++i)
		{
			vkDestroyFence(myDevice, myInFlightFrameFences[i], nullptr);
			vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(myDevice, myImageAvailableSemaphores[i], nullptr);
		}
		myInFlightFrameFences.clear();
		myRenderFinishedSemaphores.clear();
		myImageAvailableSemaphores.clear();
		myImageFences.clear();

		for (auto framebuffer : myFramebuffers)
			vkDestroyFramebuffer(myDevice, framebuffer, nullptr);
		myFramebuffers.clear();

		myCommandBuffers.clear();

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
		assert(Renderer::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.has_value());
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice,
			Renderer::GetInstance()->GetVulkanDevice()->myQueueFamilyIndices.myGraphicsFamily.value(),
			mySurface,
			&presentSupport);
		if (!presentSupport)
			throw std::runtime_error("The device doesn't support presenting on the graphics queue!");

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
			myExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)width));
			myExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height));
		}

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, nullptr);
		assert(formatCount > 0);
		std::vector<VkSurfaceFormatKHR> availableFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mySurface, &formatCount, availableFormats.data());
		VkSurfaceFormatKHR surfaceFormat = availableFormats[0];
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				surfaceFormat = availableFormat;
		}
		myColorFormat = surfaceFormat.format;

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, nullptr);
		assert(presentModeCount > 0);
		std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mySurface, &presentModeCount, availablePresentModes.data());
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
		myDepthImage.TransitionLayout(
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			Renderer::GetInstance()->GetGraphicsQueue());
	}

	void SwapChain::SetupCommandBuffers()
	{
		myCommandBuffers.resize(myImages.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = Renderer::GetInstance()->GetGraphicsCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(myCommandBuffers.size());

		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()), "Failed to create command buffers!");
	}

	void SwapChain::SetupFramebuffers()
	{
		myFramebuffers.resize(myImages.size());

		std::array<VkImageView, 5> attachments{};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = myDeferredPipeline.myRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
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
		myImageFences.resize(myImages.size(), VK_NULL_HANDLE);

		const size_t maxInFlightImagesCount = myImages.size() - 1;

		myImageAvailableSemaphores.resize(maxInFlightImagesCount);
		myRenderFinishedSemaphores.resize(maxInFlightImagesCount);
		myInFlightFrameFences.resize(maxInFlightImagesCount);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < maxInFlightImagesCount; ++i)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myImageAvailableSemaphores[i]), "Failed to create a semaphore");
			VK_CHECK_RESULT(vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myRenderFinishedSemaphores[i]), "Failed to create a semaphore");
			VK_CHECK_RESULT(vkCreateFence(myDevice, &fenceInfo, nullptr, &myInFlightFrameFences[i]), "Failed to create a fence");
		}
	}

	void SwapChain::BuildCommandBuffers()
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
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		for (uint32_t i = 0; i < (uint32_t)myCommandBuffers.size(); ++i)
		{
			// Set target frame buffer
			renderPassBeginInfo.framebuffer = myFramebuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(myCommandBuffers[i], &cmdBufferBeginInfo), "Failed to begin a command buffer");

			vkCmdBeginRenderPass(myCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)myExtent.width;
			viewport.height = (float)myExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent = myExtent;
			scissor.offset = { 0, 0 };
			vkCmdSetScissor(myCommandBuffers[i], 0, 1, &scissor);

			vkCmdSetViewport(myCommandBuffers[i], 0, 1, &viewport);

			// Draw objects with the Dummy Pipeline
			/*vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDummyPSO->GetPipeline());

			std::array<VkDescriptorSet, 1> dummyPerFrameDescriptorSets = { camera->GetDummyDescriptorSet() };
			vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDummyPSO->GetPipelineLayout(),
				0, (uint32_t)dummyPerFrameDescriptorSets.size(), dummyPerFrameDescriptorSets.data(), 0, NULL);

			for (uint32_t j = 0; j < Renderer::GetInstance()->GetPandaModelsCount(); ++j)
			{
				VulkanModel* pandaModel = Renderer::GetInstance()->GetPandaModel(j);
				pandaModel->Draw(myCommandBuffers[i], myDummyPSO->GetPipelineLayout());
			}*/

			// Draw objects with the glTF Pipeline
			/*vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myglTFPSO->GetPipeline());

			std::array<VkDescriptorSet, 1> glTFPerFrameDescriptorSets = { camera->GetglTFDescriptorSet() };
			vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myglTFPSO->GetPipelineLayout(),
				0, (uint32_t)glTFPerFrameDescriptorSets.size(), glTFPerFrameDescriptorSets.data(), 0, NULL);

			if (glTF::Model* model = Renderer::GetInstance()->GetglTFModel())
				model->Draw(myCommandBuffers[i], myglTFPSO->GetPipelineLayout());*/

			// First sub pass
			// Renders the components of the scene to the G-Buffer attachments
			{
				vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipeline);
				vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipelineLayout, 0, 1, &myDeferredPipeline.myGBufferDescriptorSet, 0, NULL);
				myDeferredPipeline.myCastleModel->Draw(myCommandBuffers[i], myDeferredPipeline.myGBufferPipelineLayout);
				myDeferredPipeline.myAvocadoModel->Draw(myCommandBuffers[i], myDeferredPipeline.myGBufferPipelineLayout);
				myDeferredPipeline.myAnimatedModel->Draw(myCommandBuffers[i], myDeferredPipeline.myGBufferPipelineLayout);
				myDeferredPipeline.myDummyModel->Draw(myCommandBuffers[i], myDeferredPipeline.myGBufferPipelineLayout);
			}

			// Second sub pass
			// This subpass will use the G-Buffer components as input attachment for the lighting
			{
				vkCmdNextSubpass(myCommandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipeline);
				vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 0, 1, &myDeferredPipeline.myLightingDescriptorSet, 0, NULL);
				vkCmdDraw(myCommandBuffers[i], 4, 1, 0, 0);
			}

			// Third subpass
			// Render transparent geometry using a forward pass that compares against depth generated during G-Buffer fill
			{
				vkCmdNextSubpass(myCommandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipeline);
				vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipelineLayout, 0, 1, &myDeferredPipeline.myTransparentDescriptorSet, 0, NULL);
				myDeferredPipeline.myCastleWindows->Draw(myCommandBuffers[i], myDeferredPipeline.myTransparentPipelineLayout);
			}

			vkCmdEndRenderPass(myCommandBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(myCommandBuffers[i]), "Failed to end a command buffer");
		}
	}

	void SwapChain::DrawFrame()
	{
		vkWaitForFences(myDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(myDevice, myVkSwapChain, UINT64_MAX, myImageAvailableSemaphores[myCurrentInFlightFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Recreate();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire a swapchain image!");
		}

		if (myImageFences[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(myDevice, 1, &myImageFences[imageIndex], VK_TRUE, UINT64_MAX);

		myImageFences[imageIndex] = myInFlightFrameFences[myCurrentInFlightFrame];

		VkSemaphore waitSemaphores[] = { myImageAvailableSemaphores[myCurrentInFlightFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { myRenderFinishedSemaphores[myCurrentInFlightFrame] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(myDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame]);

		VK_CHECK_RESULT(vkQueueSubmit(Renderer::GetInstance()->GetGraphicsQueue(), 1, &submitInfo, myInFlightFrameFences[myCurrentInFlightFrame]),
			"Failed to submit a command buffer");

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &myVkSwapChain;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(Renderer::GetInstance()->GetGraphicsQueue(), &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myFramebufferResized)
		{
			myFramebufferResized = false;
			Recreate();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present a swapchain image!");
		}

		myCurrentInFlightFrame = (myCurrentInFlightFrame + 1) % ((uint32_t)myImages.size() - 1);
	}
}
}
