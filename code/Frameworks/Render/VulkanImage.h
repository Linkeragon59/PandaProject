#pragma once

#include "VulkanHelpers.h"

namespace Render
{
	struct VulkanImage
	{
		static bool DepthFormatHasStencilAspect(VkFormat aDepthFormat)
		{
			return aDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT;
		}

		void Create(uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		VkImage myImage = VK_NULL_HANDLE;
		VmaAllocation myAllocation = VK_NULL_HANDLE;
		VkFormat myFormat = VK_FORMAT_UNDEFINED;

		void CreateImageView(VkImageAspectFlags someAspects);
		VkImageView myImageView = VK_NULL_HANDLE;

		void CreateImageSampler();
		VkSampler myImageSampler = VK_NULL_HANDLE;

		void TransitionLayout(VkImageLayout anOldLayout, VkImageLayout aNewLayout, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);

		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;
		VmaAllocator myAllocator = VK_NULL_HANDLE;
	};
}
