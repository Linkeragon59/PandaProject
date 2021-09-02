#pragma once

#include "VulkanImage.h"

namespace Render::Vulkan
{
	struct GuiPipeline
	{
		GuiPipeline();

		void Prepare(VkRenderPass aRenderPass);
		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;

		void SetupPipeline(VkRenderPass aRenderPass);
		void DestroyPipeline();
		VkPipeline myPipeline = VK_NULL_HANDLE;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
	};
}
