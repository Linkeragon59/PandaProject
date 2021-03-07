#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>
#include <string>
#include <stdexcept>

#define VK_CHECK_RESULT(X, Msg) if (X != VK_SUCCESS) { throw std::runtime_error(Msg); }

namespace Render
{
	bool CheckInstanceLayersSupport(const std::vector<const char*>& someLayers);
	bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions);

	VkShaderModule CreateShaderModule(const std::string& aFilename);

	// Put an image memory barrier for setting an image layout on the sub resource into the given command buffer
	void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
}
