#pragma once

#include "VulkanImage.h"
#include "VulkanDeferredPipeline.h"

namespace Render
{
namespace Vulkan
{
	class Camera;
	class Model;

	struct RenderContextDeferred
	{
		void Setup(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat);
		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;

		// GBuffer attachments
		Image myPositionAttachment;
		Image myNormalAttachment;
		Image myAlbedoAttachment;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat myDepthFormat = VK_FORMAT_UNDEFINED;

		DeferredPipeline myDeferredPipeline;
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myLightingDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSet myTransparentDescriptorSet = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> mySecondaryCommandBuffersGBuffer;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersCombine;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersTransparent;
		std::vector<VkFramebuffer> myFramebuffers;
		uint myCurrentFrame = 0;

		VkCommandBuffer myCurrentCommandBuffer = VK_NULL_HANDLE;

		void Begin(VkCommandBuffer aCommandBuffer, VkImageView aColorView, VkImageView aDepthView);
		void SetViewport(VkViewport aViewport);
		void SetScissor(VkRect2D aScissor);
		void BindCamera(Camera* aCamera);
		void DrawModel(Model*aModel);
		void End();

	private:
		void SetupAttachments();
		void SetupRenderPass();

		void SetupPipeline();
		void SetupDescriptorPool();
		void SetupDescriptorSets();

		void SetupSecondaryCommandBuffers();

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
	public:
		void UpdateLightsUBO(const glm::vec3& aCameraPos);
	private:
		void SetupRandomLights();
		void SetupLightsDescriptorPool();
		void SetupLightsDescriptorSets();
	};
}
}
