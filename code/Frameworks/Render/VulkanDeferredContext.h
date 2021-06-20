#pragma once

#include "VulkanImage.h"
#include "VulkanDeferredPipeline.h"

namespace Render
{
namespace Vulkan
{
	class Camera;

	struct RenderContextDeferred
	{
		void Setup(const std::vector<Image>& someColorImages, Image& aDepthStencilImage);
		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;

		// GBuffer attachments
		Image myPositionAttachment;
		Image myNormalAttachment;
		Image myAlbedoAttachment;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		DeferredPipeline myDeferredPipeline;
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myLightingDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSet myTransparentDescriptorSet = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> mySecondaryCommandBuffersGBuffer;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersCombine;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersTransparent;

		std::vector<VkFramebuffer> myFramebuffers;

		void BuildCommandBuffers(VkCommandBuffer aCommandBuffer, uint anImageIndex, Camera* aCamera, VkDescriptorSet aLightsSet);

	private:
		void SetupAttachments(VkExtent2D anExtent);
		void SetupRenderPass(VkFormat aColorFormat, VkFormat aDepthFormat);

		void SetupPipeline();
		void SetupDescriptorPool();
		void SetupDescriptorSets();

		void SetupCommandBuffers(uint anImageCount);

		void SetupFramebuffers(const std::vector<Image>& someColorImages, Image& aDepthStencilImage);
	};
}
}
