#pragma once

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

namespace Render
{
	struct VulkanImage
	{
		void Create(uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		VkImage myImage = VK_NULL_HANDLE;
		VmaAllocation myAllocation = VK_NULL_HANDLE;
		VkFormat myFormat = VK_FORMAT_UNDEFINED;

		void CreateImageView(VkImageAspectFlags someAspects);
		VkImageView myImageView = VK_NULL_HANDLE;

		static bool DepthFormatHasStencilAspect(VkFormat aDepthFormat)
		{
			return aDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT;
		}

		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;
		VmaAllocator myAllocator = VK_NULL_HANDLE;
	};
}
