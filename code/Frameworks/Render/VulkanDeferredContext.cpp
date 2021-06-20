#include "VulkanDeferredContext.h"

#include "VulkanRenderer.h"
#include "VulkanHelpers.h"
#include "VulkanDebugMessenger.h"
#include "VulkanCamera.h"

namespace Render
{
namespace Vulkan
{
	bool locUseSecondaryCommandBuffers = false;

	void RenderContextDeferred::Setup(const std::vector<Image>& someColorImages, Image& aDepthStencilImage)
	{
		myDevice = Renderer::GetInstance()->GetDevice();

		VkExtent2D extent = { someColorImages[0].myExtent.width, someColorImages[0].myExtent.height };
		SetupAttachments(extent);
		SetupRenderPass(someColorImages[0].myFormat, aDepthStencilImage.myFormat);

		SetupPipeline();
		SetupDescriptorPool();
		SetupDescriptorSets();

		SetupCommandBuffers((uint)someColorImages.size());

		SetupFramebuffers(someColorImages, aDepthStencilImage);
	}

	void RenderContextDeferred::Destroy()
	{
		for (auto framebuffer : myFramebuffers)
			vkDestroyFramebuffer(myDevice, framebuffer, nullptr);
		myFramebuffers.clear();

		mySecondaryCommandBuffersGBuffer.clear();
		mySecondaryCommandBuffersCombine.clear();
		mySecondaryCommandBuffersTransparent.clear();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;

		myDeferredPipeline.Destroy();

		vkDestroyRenderPass(Renderer::GetInstance()->GetDevice(), myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;

		myPositionAttachment.Destroy();
		myNormalAttachment.Destroy();
		myAlbedoAttachment.Destroy();
	}

	void RenderContextDeferred::BuildCommandBuffers(VkCommandBuffer aCommandBuffer, uint anImageIndex, Camera* aCamera, VkDescriptorSet aLightsSet)
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		std::array<VkClearValue, 5> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[3].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		clearValues[4].depthStencil = { 1.0f, 0 };

		VkExtent2D extent = { myPositionAttachment.myExtent.width, myPositionAttachment.myExtent.height }; // TODO

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = myRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = extent;
		renderPassBeginInfo.clearValueCount = (uint)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = myFramebuffers[anImageIndex];

		VK_CHECK_RESULT(vkBeginCommandBuffer(aCommandBuffer, &cmdBufferBeginInfo), "Failed to begin a command buffer");

		if (!locUseSecondaryCommandBuffers)
			vkCmdBeginRenderPass(aCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		else
			vkCmdBeginRenderPass(aCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.extent = extent;
		scissor.offset = { 0, 0 };

		if (!locUseSecondaryCommandBuffers)
		{
			vkCmdSetViewport(aCommandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(aCommandBuffer, 0, 1, &scissor);
			{
				// First sub pass
				// Renders the components of the scene to the G-Buffer attachments
				Debug::BeginRegion(aCommandBuffer, "Subpass 0: Deferred G-Buffer creation", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				{
					vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipeline);

					aCamera->BindViewProj(aCommandBuffer, myDeferredPipeline.myGBufferPipelineLayout, 0);
					for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
					{
						if (Model* model = Renderer::GetInstance()->GetModel(i))
						{
							if (!model->IsTransparent())
								model->Draw(aCommandBuffer, myDeferredPipeline.myGBufferPipelineLayout, 1);
						}
					}
				}
				Debug::EndRegion(aCommandBuffer);
			}

			vkCmdNextSubpass(aCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			{
				// Second sub pass
				// This subpass will use the G-Buffer components as input attachment for the lighting
				Debug::BeginRegion(aCommandBuffer, "Subpass 1: Deferred composition", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				{
					vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipeline);

					vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 0, 1, &myLightingDescriptorSet, 0, NULL);
					vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 1, 1, &aLightsSet, 0, NULL);
					vkCmdDraw(aCommandBuffer, 4, 1, 0, 0);
				}
				Debug::EndRegion(aCommandBuffer);
			}

			vkCmdNextSubpass(aCommandBuffer, VK_SUBPASS_CONTENTS_INLINE);

			{
				// Third subpass
				// Render transparent geometry using a forward pass that compares against depth generated during G-Buffer fill
				Debug::BeginRegion(aCommandBuffer, "Subpass 2: Forward transparency", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				{
					vkCmdBindPipeline(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipeline);

					vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipelineLayout, 0, 1, &myTransparentDescriptorSet, 0, NULL);
					aCamera->BindViewProj(aCommandBuffer, myDeferredPipeline.myTransparentPipelineLayout, 1);
					for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
					{
						if (Model* model = Renderer::GetInstance()->GetModel(i))
						{
							if (model->IsTransparent())
								model->Draw(aCommandBuffer, myDeferredPipeline.myTransparentPipelineLayout, 2);
						}
					}
				}
				Debug::EndRegion(aCommandBuffer);
				Debug::BeginRegion(aCommandBuffer, "Subpass 2b: UI Overlay", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				{
					//myUIOverlay.Draw(aCommandBuffer);
				}
				Debug::EndRegion(aCommandBuffer);
			}
		}
		else
		{
			VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo{};
			cmdBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			cmdBufferInheritanceInfo.renderPass = myRenderPass;
			cmdBufferInheritanceInfo.framebuffer = myFramebuffers[anImageIndex];
			cmdBufferInheritanceInfo.subpass = 0;
			{
				VkCommandBufferBeginInfo secondaryCmdBufferBeginInfo{};
				secondaryCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				secondaryCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				secondaryCmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;
				VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersGBuffer[anImageIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
				vkCmdSetViewport(mySecondaryCommandBuffersGBuffer[anImageIndex], 0, 1, &viewport);
				vkCmdSetScissor(mySecondaryCommandBuffersGBuffer[anImageIndex], 0, 1, &scissor);
				vkCmdBindPipeline(mySecondaryCommandBuffersGBuffer[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myGBufferPipeline);
			}
			cmdBufferInheritanceInfo.subpass = 1;
			{
				VkCommandBufferBeginInfo secondaryCmdBufferBeginInfo{};
				secondaryCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				secondaryCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				secondaryCmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;
				VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersCombine[anImageIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
				vkCmdSetViewport(mySecondaryCommandBuffersCombine[anImageIndex], 0, 1, &viewport);
				vkCmdSetScissor(mySecondaryCommandBuffersCombine[anImageIndex], 0, 1, &scissor);
				vkCmdBindPipeline(mySecondaryCommandBuffersCombine[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipeline);
			}
			cmdBufferInheritanceInfo.subpass = 2;
			{
				VkCommandBufferBeginInfo secondaryCmdBufferBeginInfo{};
				secondaryCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				secondaryCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				secondaryCmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;
				VK_CHECK_RESULT(vkBeginCommandBuffer(mySecondaryCommandBuffersTransparent[anImageIndex], &secondaryCmdBufferBeginInfo), "Failed to begin a command buffer");
				vkCmdSetViewport(mySecondaryCommandBuffersTransparent[anImageIndex], 0, 1, &viewport);
				vkCmdSetScissor(mySecondaryCommandBuffersTransparent[anImageIndex], 0, 1, &scissor);
				vkCmdBindPipeline(mySecondaryCommandBuffersTransparent[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipeline);
			}

			aCamera->BindViewProj(mySecondaryCommandBuffersGBuffer[anImageIndex], myDeferredPipeline.myGBufferPipelineLayout, 0);
			for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
			{
				if (Model* model = Renderer::GetInstance()->GetModel(i))
				{
					if (!model->IsTransparent())
						model->Draw(mySecondaryCommandBuffersGBuffer[anImageIndex], myDeferredPipeline.myGBufferPipelineLayout, 1);
				}
			}

			vkCmdBindDescriptorSets(mySecondaryCommandBuffersCombine[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 0, 1, &myLightingDescriptorSet, 0, NULL);
			vkCmdBindDescriptorSets(mySecondaryCommandBuffersCombine[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myLightingPipelineLayout, 1, 1, &aLightsSet, 0, NULL);
			vkCmdDraw(mySecondaryCommandBuffersCombine[anImageIndex], 4, 1, 0, 0);

			vkCmdBindDescriptorSets(mySecondaryCommandBuffersTransparent[anImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, myDeferredPipeline.myTransparentPipelineLayout, 0, 1, &myTransparentDescriptorSet, 0, NULL);
			aCamera->BindViewProj(mySecondaryCommandBuffersTransparent[anImageIndex], myDeferredPipeline.myTransparentPipelineLayout, 1);
			for (uint i = 0, e = Renderer::GetInstance()->GetModelsCount(); i < e; ++i)
			{
				if (Model* model = Renderer::GetInstance()->GetModel(i))
				{
					if (model->IsTransparent())
						model->Draw(mySecondaryCommandBuffersTransparent[anImageIndex], myDeferredPipeline.myTransparentPipelineLayout, 2);
				}
			}
			//myUIOverlay.Draw(mySecondaryCommandBuffersTransparent[anImageIndex]);

			VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersGBuffer[anImageIndex]), "Failed to end a command buffer");
			VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersCombine[anImageIndex]), "Failed to end a command buffer");
			VK_CHECK_RESULT(vkEndCommandBuffer(mySecondaryCommandBuffersTransparent[anImageIndex]), "Failed to end a command buffer");

			vkCmdExecuteCommands(aCommandBuffer, 1, &mySecondaryCommandBuffersGBuffer[anImageIndex]);
			vkCmdNextSubpass(aCommandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			vkCmdExecuteCommands(aCommandBuffer, 1, &mySecondaryCommandBuffersCombine[anImageIndex]);
			vkCmdNextSubpass(aCommandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			vkCmdExecuteCommands(aCommandBuffer, 1, &mySecondaryCommandBuffersTransparent[anImageIndex]);
		}

		vkCmdEndRenderPass(aCommandBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(aCommandBuffer), "Failed to end a command buffer");
	}

	void RenderContextDeferred::SetupAttachments(VkExtent2D anExtent)
	{
		myPositionAttachment.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myPositionAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myPositionAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myNormalAttachment.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myNormalAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myNormalAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myAlbedoAttachment.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myAlbedoAttachment.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myAlbedoAttachment.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void RenderContextDeferred::SetupRenderPass(VkFormat aColorFormat, VkFormat aDepthFormat)
	{
		std::array<VkAttachmentDescription, 5> attachments{};
		{
			// Color attachment
			attachments[0].format = aColorFormat;
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
			attachments[4].format = aDepthFormat;
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

	void RenderContextDeferred::SetupPipeline()
	{
		myDeferredPipeline.Prepare(myRenderPass);
	}

	void RenderContextDeferred::SetupDescriptorPool()
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
	}

	void RenderContextDeferred::SetupDescriptorSets()
	{
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

	void RenderContextDeferred::SetupCommandBuffers(uint anImageCount)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = Renderer::GetInstance()->GetGraphicsCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = anImageCount;

		mySecondaryCommandBuffersGBuffer.resize(anImageCount);
		mySecondaryCommandBuffersCombine.resize(anImageCount);
		mySecondaryCommandBuffersTransparent.resize(anImageCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersGBuffer.data()), "Failed to create command buffers!");
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersCombine.data()), "Failed to create command buffers!");
		VK_CHECK_RESULT(vkAllocateCommandBuffers(myDevice, &allocInfo, mySecondaryCommandBuffersTransparent.data()), "Failed to create command buffers!");
	}

	void RenderContextDeferred::SetupFramebuffers(const std::vector<Image>& someColorImages, Image& aDepthStencilImage)
	{
		myFramebuffers.resize(someColorImages.size());

		VkExtent2D extent = { someColorImages[0].myExtent.width, someColorImages[0].myExtent.height };

		std::array<VkImageView, 5> attachments{};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = myRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		for (size_t i = 0, e = myFramebuffers.size(); i < e; ++i)
		{
			attachments[0] = someColorImages[i].myImageView;
			attachments[1] = myPositionAttachment.myImageView;
			attachments[2] = myNormalAttachment.myImageView;
			attachments[3] = myAlbedoAttachment.myImageView;
			attachments[4] = aDepthStencilImage.myImageView;
			VK_CHECK_RESULT(vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &myFramebuffers[i]), "Failed to create a framebuffer!");
		}
	}
}
}
