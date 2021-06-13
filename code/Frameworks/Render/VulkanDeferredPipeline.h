#pragma once

#include "VulkanBuffer.h"
#include "VulkanDeferredRenderPass.h"

namespace Render
{
namespace glTF
{
	class Model;
}
namespace Vulkan
{
	struct DeferredPipeline
	{
		DeferredPipeline();

		void Prepare(const DeferredRenderPass& aRenderPass);
		void Update();
		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;

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
		};

		LightData myLightsData;

		// Per Scene UBOs
		Buffer myLightsUBO;

		VkPipeline myGBufferPipeline = VK_NULL_HANDLE;
		VkPipeline myLightingPipeline = VK_NULL_HANDLE;
		VkPipeline myTransparentPipeline = VK_NULL_HANDLE;

		VkPipelineLayout myGBufferPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout myLightingPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout myTransparentPipelineLayout = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout myLightingDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myTransparentDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet myLightingDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSet myTransparentDescriptorSet = VK_NULL_HANDLE;

	private:
		void PrepareUBOs();

		void UpdateLightsUBO();
		void SetupRandomLights();

		void SetupDescriptorPool();
		void SetupDescriptorSetLayouts();
		void SetupDescriptorSets(const DeferredRenderPass& aRenderPass);

		void SetupGBufferPipeline(const DeferredRenderPass& aRenderPass);
		void DestroyGBufferPipeline();

		void SetupLightingPipeline(const DeferredRenderPass& aRenderPass);
		void DestroyLightingPipeline();

		void SetupTransparentPipeline(const DeferredRenderPass& aRenderPass);
		void DestroyTransparentPipeline();
	};
}
}
