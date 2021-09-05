#include "Render_EditorRenderer.h"

#if DEBUG_BUILD

#include "Render_SwapChain.h"
#include "Render_ShaderHelpers.h"
#include "Render_Debug.h"
#include "Render_GuiImpl.h"

namespace Render
{
	void EditorRenderer::Setup(SwapChain* aSwapChain)
	{
		RendererImpl::Setup(aSwapChain);
		myExtent = mySwapChain->GetExtent();
		myColorFormat = mySwapChain->GetColorFormat();

		SetupRenderPass();
		SetupPipeline();
		SetupFrameBuffers();
	}

	void EditorRenderer::Cleanup()
	{
		DestroyFrameBuffers();
		DestroyPipeline();
		DestroyRenderPass();

		myExtent = {};
		myColorFormat = VK_FORMAT_UNDEFINED;

		RendererImpl::Cleanup();
	}

	void EditorRenderer::StartFrame()
	{
		RendererImpl::StartFrame();

		std::array<VkImageView, 1> attachments{};
		attachments[0] = mySwapChain->GetCurrentRenderTarget();

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = myRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = myExtent.width;
		framebufferInfo.height = myExtent.height;
		framebufferInfo.layers = 1;

		if (myFrameBuffers[myCurrentFrameIndex] != VK_NULL_HANDLE)
		{
			// Recycle old frame buffers
			vkDestroyFramebuffer(myDevice, myFrameBuffers[myCurrentFrameIndex], nullptr);
		}
		VK_CHECK_RESULT(vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &myFrameBuffers[myCurrentFrameIndex]), "Failed to create a framebuffer!");

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = myRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = myExtent;
		renderPassBeginInfo.clearValueCount = (uint)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = myFrameBuffers[myCurrentFrameIndex];

		vkCmdBeginRenderPass(myCommandBuffers[myCurrentFrameIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo{};
		cmdBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		cmdBufferInheritanceInfo.renderPass = myRenderPass;
		cmdBufferInheritanceInfo.framebuffer = myFrameBuffers[myCurrentFrameIndex];
		cmdBufferInheritanceInfo.subpass = 0;

		VkCommandBufferBeginInfo secondaryCmdBufferBeginInfo{};
		secondaryCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		secondaryCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		secondaryCmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;
		
		VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersGui[myCurrentFrameIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
		Debug::BeginRegion(mySecondaryCommandBuffersGui[myCurrentFrameIndex], "Subpass 0: Gui", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		vkCmdBindPipeline(mySecondaryCommandBuffersGui[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myGuiPipeline.myPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)myExtent.width;
		viewport.height = (float)myExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		SetViewport(viewport);

		VkRect2D scissor{};
		scissor.extent = myExtent;
		scissor.offset = { 0, 0 };
		SetScissor(scissor);
	}

	void EditorRenderer::EndFrame()
	{
		Debug::EndRegion(mySecondaryCommandBuffersGui[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersGui[myCurrentFrameIndex]), "Failed to end a command buffer");

		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersGui[myCurrentFrameIndex]);

		vkCmdEndRenderPass(myCommandBuffers[myCurrentFrameIndex]);

		RendererImpl::EndFrame();
	}

	void EditorRenderer::SetViewport(const VkViewport& aViewport)
	{
		vkCmdSetViewport(mySecondaryCommandBuffersGui[myCurrentFrameIndex], 0, 1, &aViewport);
	}

	void EditorRenderer::SetScissor(const VkRect2D& aScissor)
	{
		vkCmdSetScissor(mySecondaryCommandBuffersGui[myCurrentFrameIndex], 0, 1, &aScissor);
	}

	void EditorRenderer::DrawGui(Gui* aGui)
	{
		GuiImpl* guiImpl = static_cast<GuiImpl*>(aGui);
		guiImpl->Draw(mySecondaryCommandBuffersGui[myCurrentFrameIndex], myGuiPipeline.myPipelineLayout, 0);
	}

	void EditorRenderer::SetupRenderPass()
	{
		std::array<VkAttachmentDescription, 1> attachments{};
		{
			// Color attachment
			attachments[0].format = myColorFormat;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		// Subpasses
		VkAttachmentReference colorReferences[1];
		colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		std::vector<VkSubpassDescription> subpassDescriptions;
		{
			// First subpass: Draw Gui
			VkSubpassDescription description{};
			description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			description.colorAttachmentCount = 1;
			description.pColorAttachments = colorReferences;
			subpassDescriptions.push_back(description);
		}

		// TODO: Understand subpass dependencies better!
		// Subpass dependencies for layout transitions
		std::vector<VkSubpassDependency> dependencies;
		{
			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(dependency);
		}

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<uint>(subpassDescriptions.size());
		renderPassInfo.pSubpasses = subpassDescriptions.data();
		renderPassInfo.dependencyCount = static_cast<uint>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VK_CHECK_RESULT(vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass), "Failed to create the render pass!");
	}

	void EditorRenderer::DestroyRenderPass()
	{
		vkDestroyRenderPass(RenderCore::GetInstance()->GetDevice(), myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;
	}

	void EditorRenderer::SetupPipeline()
	{
		myGuiPipeline.Prepare(myRenderPass);
	}

	void EditorRenderer::DestroyPipeline()
	{
		myGuiPipeline.Destroy();
	}

	void EditorRenderer::SetupCommandBuffers()
	{
		RendererImpl::SetupCommandBuffers();

		uint framesCount = mySwapChain->GetImagesCount();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();
		allocInfo.commandBufferCount = framesCount;

		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		mySecondaryCommandBuffersGui.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersGui.data()), "Failed to create command buffers!");
	}

	void EditorRenderer::DestroyCommandBuffers()
	{
		RendererImpl::DestroyCommandBuffers();

		mySecondaryCommandBuffersGui.clear();
	}

	void EditorRenderer::SetupFrameBuffers()
	{
		myFrameBuffers.resize(mySwapChain->GetImagesCount(), VK_NULL_HANDLE);
	}

	void EditorRenderer::DestroyFrameBuffers()
	{
		for (uint i = 0; i < mySwapChain->GetImagesCount(); ++i)
		{
			if (myFrameBuffers[i] != VK_NULL_HANDLE)
				vkDestroyFramebuffer(myDevice, myFrameBuffers[i], nullptr);
		}
		myFrameBuffers.clear();
	}
}

#endif
