#include "VulkanDeferredRenderPass.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

namespace Render
{
namespace Vulkan
{
	void DeferredRenderPass::Setup(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat)
	{
		myPositionAttachement.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myPositionAttachement.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myPositionAttachement.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myNormalAttachement.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myNormalAttachement.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myNormalAttachement.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		myAlbedoAttachement.Create(anExtent.width, anExtent.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myAlbedoAttachement.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myAlbedoAttachement.SetupDescriptor(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

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
			attachments[1].format = myPositionAttachement.myFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Normals
			attachments[2].format = myNormalAttachement.myFormat;
			attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			// Albedo
			attachments[3].format = myAlbedoAttachement.myFormat;
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

		VK_CHECK_RESULT(vkCreateRenderPass(Renderer::GetInstance()->GetDevice(), &renderPassInfo, nullptr, &myRenderPass), "Failed to create the render pass!");
	}

	void DeferredRenderPass::Destroy()
	{
		vkDestroyRenderPass(Renderer::GetInstance()->GetDevice(), myRenderPass, nullptr);
		myRenderPass = VK_NULL_HANDLE;

		myPositionAttachement.Destroy();
		myNormalAttachement.Destroy();
		myAlbedoAttachement.Destroy();
	}
}
}