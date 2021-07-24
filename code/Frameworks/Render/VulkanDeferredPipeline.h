#pragma once

namespace Render::Vulkan
{
	struct DeferredPipeline
	{
		DeferredPipeline();

		void Prepare(VkRenderPass aRenderPass);
		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkPipeline myGBufferPipeline = VK_NULL_HANDLE;
		VkPipeline myLightingPipeline = VK_NULL_HANDLE;
		VkPipeline myTransparentPipeline = VK_NULL_HANDLE;

		VkPipelineLayout myGBufferPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout myLightingPipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout myTransparentPipelineLayout = VK_NULL_HANDLE;

		VkDescriptorSetLayout myLightingDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myTransparentDescriptorSetLayout = VK_NULL_HANDLE;

	private:
		void SetupDescriptorSetLayouts();
		void DestroyDescriptorSetLayouts();

		void SetupGBufferPipeline(VkRenderPass aRenderPass);
		void DestroyGBufferPipeline();

		void SetupLightingPipeline(VkRenderPass aRenderPass);
		void DestroyLightingPipeline();

		void SetupTransparentPipeline(VkRenderPass aRenderPass);
		void DestroyTransparentPipeline();
	};
}
