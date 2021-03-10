#pragma once

#include "VulkanHelpers.h"

namespace Render
{
	struct VulkanBuffer
	{
		~VulkanBuffer();

		void Create(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		VkBuffer myBuffer = VK_NULL_HANDLE;
		VmaAllocation myAllocation = VK_NULL_HANDLE;

		void Map();
		void Unmap();
		void Flush();
		void* myMappedData = nullptr;

		void SetupDescriptor(VkDeviceSize aSize = VK_WHOLE_SIZE, VkDeviceSize anOffset = 0);
		VkDescriptorBufferInfo myDescriptor{};

		void Destroy();

		VmaAllocator myAllocator = VK_NULL_HANDLE;
	};
}
