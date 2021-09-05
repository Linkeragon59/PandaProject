#pragma once

#include "Render_RendererImpl.h"
#include "Render_VulkanBuffer.h"
#include "Render_VulkanImage.h"
#include "Render_DeferredPipeline.h"
#include "Render_PointLightsSet.h"

namespace Render
{
	class Camera;

	class DeferredRenderer : public RendererImpl
	{
	public:
		void Setup(SwapChain* aSwapChain) override;
		void Cleanup() override;

		void StartFrame() override;
		void EndFrame() override;

		void SetViewport(const VkViewport& aViewport);
		void SetScissor(const VkRect2D& aScissor);

		void DrawModel(Model* aModel, const ModelData& someData, DrawType aDrawType = DrawType::Default) override;
		void AddLight(const PointLight& aPointLight) override;
		void DrawGui(Gui* /*aGui*/) override {};

	private:
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat myDepthFormat = VK_FORMAT_UNDEFINED;

		// Attachments
		void SetupAttachments();
		void DestroyAttachments();
		// GBuffer attachments
		VulkanImage myPositionAttachment;
		VulkanImage myNormalAttachment;
		VulkanImage myAlbedoAttachment;
		// Depth attachment
		VulkanImage myDepthAttachment;

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
