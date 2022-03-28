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
		VkPipeline myGBufferPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myGBufferPipelineLayout = VK_NULL_HANDLE;

		VkPipeline myLightingPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myLightingPipelineLayout = VK_NULL_HANDLE;
		VkDescriptorSetLayout myLightingDescriptorSetLayout = VK_NULL_HANDLE;

#if DEBUG_BUILD
		VkPipeline myDebug3DPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myDebug3DPipelineLayout = VK_NULL_HANDLE;
#endif

		VkPipeline myGuiPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myGuiPipelineLayout = VK_NULL_HANDLE;

	private:
		void SetupDescriptorSetLayouts();
		void DestroyDescriptorSetLayouts();

		void SetupGBufferPipeline(VkRenderPass aRenderPass, uint aSubpass);
		void DestroyGBufferPipeline();

		void SetupLightingPipeline(VkRenderPass aRenderPass, uint aSubpass);
		void DestroyLightingPipeline();

#if DEBUG_BUILD
		void SetupDebugForwardPipeline(VkRenderPass aRenderPass, uint aSubpass);
		void DestroyDebugForwardPipeline();
#endif

		void SetupGuiPipeline(VkRenderPass aRenderPass, uint aSubpass);
		void DestroyGuiPipeline();
	};
}
