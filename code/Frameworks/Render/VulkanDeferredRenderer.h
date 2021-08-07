#pragma once

#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDeferredPipeline.h"

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
		void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection) override;

		void DrawModel(const Render::Model* aModel, const glTFModelData& someData) override;

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
		VkDescriptorSet myTransparentDescriptorSet = VK_NULL_HANDLE;

		// Command Buffers - one per frame
		void SetupCommandBuffers() override;
		void DestroyCommandBuffers() override;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersGBuffer;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersCombine;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersTransparent;

		// Frame Buffers - one per frame
		void SetupFrameBuffers();
		void DestroyFrameBuffers();
		std::vector<VkFramebuffer> myFrameBuffers;

		// Temporary
		// Lights will be moved to a separate class later
		static const uint ourNumLights = 64;
		struct Light
		{
			glm::vec4 myPosition;
			glm::vec3 myColor;
			float myRadius;
		};
		struct LightData
		{
			glm::vec4 myViewPos;
			Light myLights[ourNumLights];
		} myLightsData;
		Buffer myLightsUBO;
		VkDescriptorPool myLightsDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myLightsDescriptorSet = VK_NULL_HANDLE;
		void SetupLights();
		void DestroyLights();
		void UpdateLightsUBO(const glm::vec3& aCameraPos);
		void SetupRandomLights();
		void SetupLightsDescriptorPool();
		void SetupLightsDescriptorSets();
	};
}
