#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#include <vector>
#include <string>

namespace Render
{
	bool CheckInstanceLayersSupport(const std::vector<const char*>& someLayers);
	bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions);

	VkCommandBuffer BeginOneShotCommand();
	void EndOneShotCommand(VkQueue aQueue, VkCommandBuffer aCommandBuffer);

	void TransitionImageLayout(VkQueue aQueue, VkImage anImage, VkFormat aFormat, VkImageLayout anOldLayout, VkImageLayout aNewLayout);

	void CreateImage(uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkImage& anOutImage, VmaAllocation& anOutImageAlloc);
	void CreateBuffer(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkBuffer& anOutBuffer, VmaAllocation& anOutBufferAlloc);

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
