#pragma once

#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDeferredPipeline.h"
#include "VulkanPointLightsSet.h"

namespace Render::Vulkan
{
	class Camera;

	class DeferredRenderer : public Renderer
	{
	public:
		void Setup(SwapChain* aSwapChain) override;
		void Cleanup() override;

		void StartFrame() override;
		void EndFrame() override;

		void SetViewport(const VkViewport& aViewport);
		void SetScissor(const VkRect2D& aScissor);

		void DrawModel(Render::Model* aModel, const BaseModelData& someData, DrawType aDrawType = DrawType::Normal) override;
		void AddLight(const PointLight& aPointLight) override;

	private:
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat myDepthFormat = VK_FORMAT_UNDEFINED;

		// Attachments - TODO : one per frame?
		void SetupAttachments();
		void DestroyAttachments();
		// GBuffer attachments
		Image myPositionAttachment;
		Image myNormalAttachment;
		Image myAlbedoAttachment;
		// Depth attachment
		Image myDepthAttachment;

		// Point Lights
		PointLightsSet myPointLightsSet;

		// Render Pass
		void SetupRenderPass();
		void DestroyRenderPass();
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		// Pipelines
		void SetupPipeline();
		void DestroyPipeline();
		DeferredPipeline myDeferredPipeline;

		// Descriptors
		void SetupDescriptorSets();
		void DestroyDescriptorSets();
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myLightingDescriptorSet = VK_NULL_HANDLE;

		// Command Buffers - one per frame
		void SetupCommandBuffers() override;
		void DestroyCommandBuffers() override;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersGBuffer;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersCombine;
#if DEBUG_BUILD
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersDebugForward;
#endif

		// Frame Buffers - one per frame
		void SetupFrameBuffers();
		void DestroyFrameBuffers();
		std::vector<VkFramebuffer> myFrameBuffers;
	};
}
