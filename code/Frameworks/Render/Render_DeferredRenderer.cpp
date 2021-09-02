#include "Render_DeferredRenderer.h"

#include "Render_VulkanDevice.h"
#include "Render_Camera.h"
#include "Render_SwapChain.h"
#include "Render_ShaderHelpers.h"
#include "Render_Debug.h"
#include "Render_ModelImpl.h"

namespace Render
{
	void DeferredRenderer::Setup(SwapChain* aSwapChain)
	{
		RendererImpl::Setup(aSwapChain);

		myExtent = mySwapChain->GetExtent();
		myColorFormat = mySwapChain->GetColorFormat();
		myDepthFormat = RenderCore::GetInstance()->GetVulkanDevice()->FindBestDepthFormat();

		SetupAttachments();
		SetupRenderPass();
		SetupPipeline();
		SetupDescriptorSets();
		SetupFrameBuffers();

		myPointLightsSet.Setup();
	}

	void DeferredRenderer::Cleanup()
	{
		myPointLightsSet.Destroy();

		DestroyFrameBuffers();
		DestroyDescriptorSets();
		DestroyPipeline();
		DestroyRenderPass();
		DestroyAttachments();

		myExtent = {};
		myColorFormat = VK_FORMAT_UNDEFINED;
		myDepthFormat = VK_FORMAT_UNDEFINED;

		RendererImpl::Cleanup();
	}

	void DeferredRenderer::StartFrame()
	{
		RendererImpl::StartFrame();

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
		Debug::BeginRegion(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], "Subpass 0: Deferred G-Buffer", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		cmdBufferInheritanceInfo.subpass = 1;
		VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
		Debug::BeginRegion(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], "Subpass 1: Deferred Lighting", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

#if DEBUG_BUILD
		cmdBufferInheritanceInfo.subpass = 2;
		VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
		Debug::BeginRegion(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], "Subpass 2: Debug Forward", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
#endif

		vkCmdBindPipeline(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipeline);
		myCamera->BindViewProj(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], myDeferredPipeline.myGBufferPipelineLayout, 0);

		vkCmdBindPipeline(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipeline);
		vkCmdBindDescriptorSets(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 0, 1, &myLightingDescriptorSet, 0, NULL);
		myPointLightsSet.Bind(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], myDeferredPipeline.myLightingPipelineLayout, 1);

#if DEBUG_BUILD
		vkCmdBindPipeline(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myDebug3DPipeline);
		myCamera->BindViewProj(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], myDeferredPipeline.myDebug3DPipelineLayout, 0);
#endif

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

		myPointLightsSet.ClearLightData();
	}

	void DeferredRenderer::EndFrame()
	{
		myPointLightsSet.UpdateUBO();

		Debug::EndRegion(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex]), "Failed to end a command buffer");

		vkCmdDraw(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], 4, 1, 0, 0);
		Debug::EndRegion(mySecondaryCommandBuffersCombine[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersCombine[myCurrentFrameIndex]), "Failed to end a command buffer");

#if DEBUG_BUILD
		Debug::EndRegion(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex]);
		VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex]), "Failed to end a command buffer");
#endif

		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex]);
		vkCmdNextSubpass(myCommandBuffers[myCurrentFrameIndex], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersCombine[myCurrentFrameIndex]);
#if DEBUG_BUILD
		vkCmdNextSubpass(myCommandBuffers[myCurrentFrameIndex], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		vkCmdExecuteCommands(myCommandBuffers[myCurrentFrameIndex], 1, &mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex]);
#endif

		vkCmdEndRenderPass(myCommandBuffers[myCurrentFrameIndex]);

		RendererImpl::EndFrame();
	}

	void DeferredRenderer::SetViewport(const VkViewport& aViewport)
	{
		vkCmdSetViewport(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], 0, 1, &aViewport);
		vkCmdSetViewport(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], 0, 1, &aViewport);
#if DEBUG_BUILD
		vkCmdSetViewport(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], 0, 1, &aViewport);
#endif
	}

	void DeferredRenderer::SetScissor(const VkRect2D& aScissor)
	{
		vkCmdSetScissor(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], 0, 1, &aScissor);
		vkCmdSetScissor(mySecondaryCommandBuffersCombine[myCurrentFrameIndex], 0, 1, &aScissor);
#if DEBUG_BUILD
		vkCmdSetScissor(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], 0, 1, &aScissor);
#endif
	}

	void DeferredRenderer::DrawModel(Model* aModel, const ModelData& someData, DrawType aDrawType /*= DrawType::Normal*/)
	{
		(void)someData;
		ModelImpl* modelImpl = static_cast<ModelImpl*>(aModel);

		switch (aDrawType)
		{
		case Renderer::DrawType::Default:
			modelImpl->Draw(mySecondaryCommandBuffersGBuffer[myCurrentFrameIndex], myDeferredPipeline.myGBufferPipelineLayout, 1, ShaderHelpers::BindType::Object);
			break;
#if DEBUG_BUILD
		case Renderer::DrawType::Debug:
			modelImpl->Draw(mySecondaryCommandBuffersDebugForward[myCurrentFrameIndex], myDeferredPipeline.myDebug3DPipelineLayout, 1, ShaderHelpers::BindType::SimpleObject);
			break;
#endif
		default:
			Assert(false, "Unsupported draw type");
			break;
		}		
	}

	void DeferredRenderer::AddLight(const PointLight& aPointLight)
	{
		myPointLightsSet.AddLight(aPointLight);
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
		if (VulkanImage::DepthFormatHasStencilAspect(myDepthFormat))
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

		// Subpasses
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

		std::vector<VkSubpassDescription> subpassDescriptions;
		{
			// First subpass: Fill G-Buffer components
			VkSubpassDescription gbufferDescription{};
			gbufferDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			gbufferDescription.colorAttachmentCount = 4;
			gbufferDescription.pColorAttachments = colorReferences;
			gbufferDescription.pDepthStencilAttachment = &depthReference;
			subpassDescriptions.push_back(gbufferDescription);

			// Second subpass: Final Lighting (using G-Buffer components)
			VkSubpassDescription lightingDescription{};
			lightingDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			lightingDescription.colorAttachmentCount = 1;
			lightingDescription.pColorAttachments = colorReferences;
			lightingDescription.pDepthStencilAttachment = &depthReference;
			lightingDescription.inputAttachmentCount = 3;
			lightingDescription.pInputAttachments = inputReferences;
			subpassDescriptions.push_back(lightingDescription);

#if DEBUG_BUILD
			// Third subpass: Debug Forward
			VkSubpassDescription debugForwardDescription{};
			debugForwardDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			debugForwardDescription.colorAttachmentCount = 1;
			debugForwardDescription.pColorAttachments = colorReferences;
			debugForwardDescription.pDepthStencilAttachment = &depthReference;
			debugForwardDescription.inputAttachmentCount = 1;
			debugForwardDescription.pInputAttachments = inputReferences;
			subpassDescriptions.push_back(debugForwardDescription);
#endif
		}

		// TODO: Understand subpass dependencies better!
		// Subpass dependencies for layout transitions
		std::vector<VkSubpassDependency> dependencies;
		{
			VkSubpassDependency gbufferDependency{};
			gbufferDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			gbufferDependency.dstSubpass = 0;
			gbufferDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			gbufferDependency.srcAccessMask = 0;
			gbufferDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			gbufferDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			gbufferDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(gbufferDependency);

			VkSubpassDependency lightingDependency{};
			lightingDependency.srcSubpass = 0;
			lightingDependency.dstSubpass = 1;
			lightingDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			lightingDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			lightingDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			lightingDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			lightingDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(lightingDependency);

#if DEBUG_BUILD
			VkSubpassDependency debugForwardDependency{};
			debugForwardDependency.srcSubpass = 1;
			debugForwardDependency.dstSubpass = 2;
			debugForwardDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			debugForwardDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			debugForwardDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			debugForwardDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			debugForwardDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies.push_back(debugForwardDependency);
#endif
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
		poolSizes[0].descriptorCount = 3; //  3 - Lighting

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1; // Lighting

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
	}

	void DeferredRenderer::DestroyDescriptorSets()
	{
		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void DeferredRenderer::SetupCommandBuffers()
	{
		RendererImpl::SetupCommandBuffers();

		uint framesCount = mySwapChain->GetImagesCount();

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = RenderCore::GetInstance()->GetGraphicsCommandPool();
		allocInfo.commandBufferCount = framesCount;

		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		mySecondaryCommandBuffersGBuffer.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersGBuffer.data()), "Failed to create command buffers!");
		mySecondaryCommandBuffersCombine.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersCombine.data()), "Failed to create command buffers!");
#if DEBUG_BUILD
		mySecondaryCommandBuffersDebugForward.resize(framesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersDebugForward.data()), "Failed to create command buffers!");
#endif
	}

	void DeferredRenderer::DestroyCommandBuffers()
	{
		RendererImpl::DestroyCommandBuffers();

		mySecondaryCommandBuffersGBuffer.clear();
		mySecondaryCommandBuffersCombine.clear();
#if DEBUG_BUILD
		mySecondaryCommandBuffersDebugForward.clear();
#endif
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
}
