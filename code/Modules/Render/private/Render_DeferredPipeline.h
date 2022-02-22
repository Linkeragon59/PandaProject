#pragma once

namespace Render
{
	struct DeferredPipeline
	{
		DeferredPipeline();

		void Prepare(VkRenderPass aRenderPass);
		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;

		// TODO:
		// - Add transparent sub pass
		// - Add UI sub pass
		VkPipeline myGBufferPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myGBufferPipelineLayout = VK_NULL_HANDLE;

		VkPipeline myLightingPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myLightingPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myLightingDescriptorSetLayout = VK_NULL_HANDLE;

#if DEBUG_BUILD
		VkPipeline myDebug3DPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myDebug3DPipelineLayout = VK_NULL_HANDLE;
#endif

	private:
		void SetupDescriptorSetLayouts();
		void DestroyDescriptorSetLayouts();

		void SetupGBufferPipeline(VkRenderPass aRenderPass);
		void DestroyGBufferPipeline();

		void SetupLightingPipeline(VkRenderPass aRenderPass);
		void DestroyLightingPipeline();

#if DEBUG_BUILD
		void SetupDebugForwardPipeline(VkRenderPass aRenderPass);
		void DestroyDebugForwardPipeline();
#endif
	};
}