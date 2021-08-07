#include "VulkanDeferredRenderer.h"

#include "VulkanRender.h"
#include "VulkanDevice.h"
#include "VulkanCamera.h"
#include "VulkanSwapChain.h"
#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"
#include "VulkanDebug.h"

#include <random>

namespace Render::Vulkan
{
	void DeferredRenderer::Setup(SwapChain* aSwapChain)
	{
		Renderer::Setup(aSwapChain);

		myExtent = mySwapChain->GetExtent();
		myColorFormat = mySwapChain->GetColorFormat();
		myDepthFormat = RenderCore::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();

		SetupAttachments();
		SetupRenderPass();
		SetupPipeline();
		SetupDescriptorSets();
		SetupFrameBuffers();

		SetupLights();
	}

	void DeferredRenderer::Cleanup()
	{
		DestroyLights();

		DestroyFrameBuffers();
		DestroyDescriptorSets();
		DestroyPipeline();
		DestroyRenderPass();
		DestroyAttachments();

		myExtent = {};
		myColorFormat = VK_FORMAT_UNDEFINED;
		myDepthFormat = VK_FORMAT_UNDEFINED;

		Renderer::Cleanup();
	}

	void DeferredRenderer::StartFrame()
	{
		Renderer::StartFrame();

		std::array<VkImageView, 5> attachments{};
		attachments[0] = mySwapChain->GetCurrentRenderTarget();
		attachments[1] = myPositionAttachment.myImageView;
		attachments[2] = myNormalAttachment.myImageView;
		attachments[3] = myAlbedoAttachment.myImageView;
		attachments[4] = myDepthAttachment.myImageView;

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

		std::array<VkClearValue, 5> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[3].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[4].depthStencil = { 1.0f, 0 };

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

		VkCommandBufferBeginInfo secondaryCmdBufferBeginInfo{};
		secondaryCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		secondaryCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		secondaryCmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;

		cmdBufferInheritanceInfo.subpass = 0;
		VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
		Debug::BeginRegion(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], "Subpass 0: Deferred G-Buffer creation", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		cmdBufferInheritanceInfo.subpass = 1;
		VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
		Debug::BeginRegion(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], "Subpass 1: Deferred composition", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		cmdBufferInheritanceInfo.subpass = 2;
		VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
		Debug::BeginRegion(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], "Subpass 2: Forward transparency", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		vkCmdBindPipeline(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipeline);
		myCamera->BindViewProj(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], myDeferredPipeline.myGBufferPipelineLayout, 0);

		vkCmdBindPipeline(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipeline);
		vkCmdBindDescriptorSets(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 0, 1, &myLightingDescriptorSet, 0, NULL);
		// Temporary
		vkCmdBindDescriptorSets(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 1, 1, &myLightsDescriptorSet, 0, NULL);

		vkCmdBindPipeline(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipeline);
		vkCmdBindDescriptorSets(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipelineLayout, 0, 1, &myTransparentDescriptorSet, 0, NULL);
		myCamera->BindViewProj(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], myDeferredPipeline.myTransparentPipelineLayout, 1);

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

	void DeferredRenderer::EndFrame()
	{
		Debug::EndRegion(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex]), "Failed to end a command buffer");

		vkCmdDraw(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], 4, 1, 0, 0);
		Debug::EndRegion(mySecondaryCommandBuffersCombine[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersCombine[myCurrentFrameIndex]), "Failed to end a command buffer");

		Debug::EndRegion(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex]), "Failed to end a command buffer");

		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex]);
		vkCmdNextSubpass(myCommandBuffers[myCurrentFrameIndex], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersCombine[myCurrentFrameIndex]);
		vkCmdNextSubpass(myCommandBuffers[myCurrentFrameIndex], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersTransparent[myCurrentFrameIndex]);

		vkCmdEndRenderPass(myCommandBuffers[myCurrentFrameIndex]);

		Renderer::EndFrame();
	}

	void DeferredRenderer::SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		Renderer::SetViewProj(aView, aProjection);
		UpdateLightsUBO(myCamera->GetView()[3]);
	}

	void DeferredRenderer::SetViewport(const VkViewport& aViewport)
	{
		vkCmdSetViewport(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], 0, 1, &aViewport);
		vkCmdSetViewport(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], 0, 1, &aViewport);
		vkCmdSetViewport(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], 0, 1, &aViewport);
	}

	void DeferredRenderer::SetScissor(const VkRect2D& aScissor)
	{
		vkCmdSetScissor(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], 0, 1, &aScissor);
		vkCmdSetScissor(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], 0, 1, &aScissor);
		vkCmdSetScissor(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], 0, 1, &aScissor);
	}

	void DeferredRenderer::DrawModel(const Model* aModel, const glTFModelData& someData)
	{
		if (!someData.myIsTransparent)
			aModel->Draw(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], myDeferredPipeline.myGBufferPipelineLayout, 1);
		else
			aModel->Draw(mySecondaryCommandBuffersTransparent[myCurrentFrameIndex], myDeferredPipeline.myTransparentPipelineLayout, 2);
	}

	void DeferredRenderer::SetupAttachments()
	{
		myPositionAttachment.Create(myExtent.width, myExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myPositionAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myPositionAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myNormalAttachment.Create(myExtent.width, myExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myNormalAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myNormalAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myAlbedoAttachment.Create(myExtent.width, myExtent.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myAlbedoAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myAlbedoAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myDepthAttachment.Create(myExtent.width, myExtent.height,
			myDepthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkImageAspectFlags aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (Image::DepthFormatHasStencilAspect(myDepthFormat))
			aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;
		myDepthAttachment.CreateImageView(aspects);
	}

	void DeferredRenderer::DestroyAttachments()
	{
		myPositionAttachment.Destroy();
		myNormalAttachment.Destroy();
		myAlbedoAttachment.Destroy();
		myDepthAttachment.Destroy();
	}

	void DeferredRenderer::SetupRenderPass()
	{
		std::array<VkAttachmentDescription, 5> attachments{};
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

			// Deferred attachments
			// Position
			attachments[1].format = myPositionAttachment.myFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Normals
			attachments[2].format = myNormalAttachment.myFormat;
			attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Albedo
			attachments[3].format = myAlbedoAttachment.myFormat;
			attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// Depth attachment
			attachments[4].format = myDepthFormat;
			attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		// Three subpasses
		VkAttachmentReference colorReferences[4];
		colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[2] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		colorReferences[3] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentReference depthReference = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkAttachmentReference inputReferences[3];
		inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		std::array<VkSubpassDescription, 3> subpassDescriptions{};
		{
			// First subpass: Fill G-Buffer components
			subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[0].colorAttachmentCount = 4;
			subpassDescriptions[0].pColorAttachments = colorReferences;
			subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

			// Second subpass: Final Lighting (using G-Buffer components)
			subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[1].colorAttachmentCount = 1;
			subpassDescriptions[1].pColorAttachments = colorReferences;
			subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
			subpassDescriptions[1].inputAttachmentCount = 3;
			subpassDescriptions[1].pInputAttachments = inputReferences;

			// Third subpass: Forward transparency
			subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescriptions[2].colorAttachmentCount = 1;
			subpassDescriptions[2].pColorAttachments = colorReferences;
			subpassDescriptions[2].pDepthStencilAttachment = &depthReference;
			subpassDescriptions[2].inputAttachmentCount = 1;
			subpassDescriptions[2].pInputAttachments = inputReferences;
		}

		// TODO: Understand subpass dependencies better!
		// Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 3> dependencies{};
		{
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = 0;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// This dependency transitions the input attachment from color attachment to shader read
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = 1;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[2].srcSubpass = 1;
			dependencies[2].dstSubpass = 2;
			dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
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

	void DeferredRenderer::DestroyRenderPass()
	{
		vkDestroyRenderPass(RenderCore::GetInstance()->GetDevice(), myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;
	}

	void DeferredRenderer::SetupPipeline()
	{
		myDeferredPipeline.Prepare(myRenderPass);
	}

	void DeferredRenderer::DestroyPipeline()
	{
		myDeferredPipeline.Destroy();
	}

	void DeferredRenderer::SetupDescriptorSets()
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSizes[0].descriptorCount = 4; //  3 - Lighting, 1 - Transparent

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 2; // Lighting, Transparent

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");

		// Lighting
		{
			std::array<VkDescriptorSetLayout, 1> layouts = { myDeferredPipeline.myLightingDescriptorSetLayout };
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = layouts.data();
			descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
			VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myLightingDescriptorSet), "Failed to create the node descriptor set");

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].pImageInfo = &myPositionAttachment.myDescriptor;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].pImageInfo = &myNormalAttachment.myDescriptor;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[2].dstSet = myLightingDescriptorSet;
			writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[2].dstBinding = 2;
			writeDescriptorSets[2].pImageInfo = &myAlbedoAttachment.myDescriptor;
			writeDescriptorSets[2].descriptorCount = 1;
			vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}

		// Transparent
		{
			std::array<VkDescriptorSetLayout, 1> layouts = { myDeferredPipeline.myTransparentDescriptorSetLayout };
			VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
			descriptorSetAllocateInfo.pSetLayouts = layouts.data();
			descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
			VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myTransparentDescriptorSet), "Failed to create the node descriptor set");

			std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].dstSet = myTransparentDescriptorSet;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].pImageInfo = &myPositionAttachment.myDescriptor;
			writeDescriptorSets[0].descriptorCount = 1;
			vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}
	}

	void DeferredRenderer::DestroyDescriptorSets()
	{
		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void DeferredRenderer::SetupCommandBuffers()
	{
		uint framesCount = mySwapChain->GetImagesCount();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();
		allocInfo.commandBufferCount = framesCount;

		// Primary
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		myCommandBuffers.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()), "Failed to create command buffers!");

		// Secondary
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		mySecondaryCommandBuffersGBuffer.resize(framesCount);
		mySecondaryCommandBuffersCombine.resize(framesCount);
		mySecondaryCommandBuffersTransparent.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersGBuffer.data()), "Failed to create command buffers!");
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersCombine.data()), "Failed to create command buffers!");
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersTransparent.data()), "Failed to create command buffers!");
	}

	void DeferredRenderer::DestroyCommandBuffers()
	{
		myCommandBuffers.clear();

		mySecondaryCommandBuffersGBuffer.clear();
		mySecondaryCommandBuffersCombine.clear();
		mySecondaryCommandBuffersTransparent.clear();
	}

	void DeferredRenderer::SetupFrameBuffers()
	{
		myFrameBuffers.resize(mySwapChain->GetImagesCount(), VK_NULL_HANDLE);
	}

	void DeferredRenderer::DestroyFrameBuffers()
	{
		for (uint i = 0; i < mySwapChain->GetImagesCount(); ++i)
		{
			if (myFrameBuffers[i] != VK_NULL_HANDLE)
				vkDestroyFramebuffer(myDevice, myFrameBuffers[i], nullptr);
		}
		myFrameBuffers.clear();
	}

	// Temporary
	void DeferredRenderer::SetupLights()
	{
		myLightsUBO.Create(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myLightsUBO.SetupDescriptor();
		myLightsUBO.Map();
		SetupRandomLights();
		SetupLightsDescriptorPool();
		SetupLightsDescriptorSets();
	}

	void DeferredRenderer::DestroyLights()
	{
		myLightsUBO.Destroy();
		vkDestroyDescriptorPool(myDevice, myLightsDescriptorPool, nullptr);
	}

	void DeferredRenderer::UpdateLightsUBO(const glm::vec3& aCameraPos)
	{
		myLightsData.myViewPos = glm::vec4(aCameraPos, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
		memcpy(myLightsUBO.myMappedData, &myLightsData, sizeof(LightData));
	}

	void DeferredRenderer::SetupRandomLights()
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

	void DeferredRenderer::SetupLightsDescriptorPool()
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

	void DeferredRenderer::SetupLightsDescriptorSets()
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
