#pragma once

#include "Renderer.h"
#include "RenderModel.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanDeferredPipeline.h"

namespace Render::Vulkan
{
	class Camera;

	class DeferrerRenderer : public Renderer
	{
	public:
		DeferrerRenderer();
		void Setup(uint aFramesCount, VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat);
		void Destroy();

		void StartFrame(GLFWwindow* aWindow) override;
		void StartFrame(VkImageView aColorAttachment);
		void EndFrame() override;
		void EndFrame(VkSemaphore aColorAttachmentAvailableSemaphore, VkSemaphore aRenderCompleteSemaphore);

		void UpdateView(const glm::mat4& aView, const glm::mat4& aProjection) override;

		void SetViewport(VkViewport aViewport);
		void SetScissor(VkRect2D aScissor);
		void DrawModel(Model* aModel) override;

	private:
		VkDevice myDevice = VK_NULL_HANDLE;

		uint myFramesCount = 0; // Keep resources alive for that number of frames
		uint myCurrentFrame = 0;
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat myDepthFormat = VK_FORMAT_UNDEFINED;

		Camera* myCamera = nullptr;

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
		void SetupCommandBuffers();
		void DestroyCommandBuffers();
		std::vector<VkCommandBuffer> myCommandBuffers;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersGBuffer;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersCombine;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersTransparent;

		// Frame Buffers - one per frame
		void SetupFrameBuffers();
		void DestroyFrameBuffers();
		std::vector<VkFramebuffer> myFrameBuffers;

		// In-flight resources fences - one per frame
		void SetupFrameFences();
		void DestroyFrameFences();
		std::vector<VkFence> myFrameFences;

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
