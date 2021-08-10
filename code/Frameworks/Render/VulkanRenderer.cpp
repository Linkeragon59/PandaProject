#include "VulkanRenderer.h"

#include "VulkanRender.h"
#include "VulkanHelpers.h"
#include "VulkanCamera.h"
#include "VulkanSwapChain.h"
#include "VulkanModel.h"

namespace Render::Vulkan
{
	Renderer::Renderer()
	{
		myDevice = RenderCore::GetInstance()->GetDevice();
		myCamera = new Camera();
	}

	Renderer::~Renderer()
	{
		delete myCamera;
	}

	void Renderer::Setup(SwapChain* aSwapChain)
	{
		Assert(!mySwapChain && aSwapChain);
		mySwapChain = aSwapChain;
		SetupCommandBuffers();
		SetupSyncObjects();
	}

	void Renderer::Cleanup()
	{
		DestroySyncObjects();
		DestroyCommandBuffers();
		mySwapChain = nullptr;
		myCurrentFrameIndex = 0;
	}

	void Renderer::StartFrame()
	{
		vkWaitForFences(myDevice, 1, &myFrameFences[myCurrentFrameIndex], VK_TRUE, UINT64_MAX);

		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(myCommandBuffers[myCurrentFrameIndex], &cmdBufferBeginInfo), "Failed to begin a command buffer");
	}

	void Renderer::EndFrame()
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(myCommandBuffers[myCurrentFrameIndex]), "Failed to end a command buffer");

		VkSemaphore waitSemaphore = mySwapChain->GetCurrentRenderTargetSemaphore();
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[myCurrentFrameIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &myRenderFinishedSemaphores[myCurrentFrameIndex];

		vkResetFences(myDevice, 1, &myFrameFences[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkQueueSubmit(RenderCore::GetInstance()->GetGraphicsQueue(), 1, &submitInfo, myFrameFences[myCurrentFrameIndex]),
			"Failed to submit a command buffer");

		myCurrentFrameIndex = (myCurrentFrameIndex + 1) % mySwapChain->GetImagesCount();
	}

	void Renderer::SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myCamera->Update(aView, aProjection);
	}

	void Renderer::SetupSyncObjects()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		uint framesCount = mySwapChain->GetImagesCount();
		myRenderFinishedSemaphores.resize(framesCount);
		myFrameFences.resize(framesCount);

		for (uint i = 0; i < framesCount; ++i)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myRenderFinishedSemaphores[i]), "Failed to create a semaphore");
			VK_CHECK_RESULT(vkCreateFence(myDevice, &fenceInfo, nullptr, &myFrameFences[i]), "Failed to create a fence");
		}
	}

	void Renderer::DestroySyncObjects()
	{
		uint framesCount = mySwapChain->GetImagesCount();
		for (uint i = 0; i < framesCount; ++i)
		{
			vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(myDevice, myFrameFences[i], nullptr);
		}

		myRenderFinishedSemaphores.clear();
		myFrameFences.clear();
	}

	void Renderer::SetupCommandBuffers()
	{
		uint framesCount = mySwapChain->GetImagesCount();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();
		allocInfo.commandBufferCount = framesCount;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		myCommandBuffers.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()), "Failed to create command buffers!");
	}

	void Renderer::DestroyCommandBuffers()
	{
		myCommandBuffers.clear();
	}
}
