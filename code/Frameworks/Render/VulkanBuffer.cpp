#include "VulkanBuffer.h"
#include "VulkanRenderCore.h"

namespace Render
{

	void VulkanBuffer::Create(VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = aSize;
		bufferInfo.usage = aUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = nullptr;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.flags = 0;
		if (someProperties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (someProperties == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			if (aUsage == VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			{
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			}
			else
			{
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			}
		}
		else
		{
			throw std::runtime_error("Buffer allocation usage not supported yet.");
		}
		allocInfo.requiredFlags = someProperties;

		if (vmaCreateBuffer(VulkanRenderCore::GetInstance()->GetAllocator(), &bufferInfo, &allocInfo, &myBuffer, &myAllocation, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a buffer!");

		SetupDescriptor();
	}

	void VulkanBuffer::Map()
	{
		if (vmaMapMemory(VulkanRenderCore::GetInstance()->GetAllocator(), myAllocation, &myMappedData) != VK_SUCCESS)
			throw std::runtime_error("Failed to map a buffer allocation!");
	}

	void VulkanBuffer::Unmap()
	{
		vmaUnmapMemory(VulkanRenderCore::GetInstance()->GetAllocator(), myAllocation);
	}

	void VulkanBuffer::Flush()
	{
		if (vmaFlushAllocation(VulkanRenderCore::GetInstance()->GetAllocator(), myAllocation, 0, VK_WHOLE_SIZE) != VK_SUCCESS)
			throw std::runtime_error("Failed to flush a buffer allocation!");
	}

	void VulkanBuffer::SetupDescriptor(VkDeviceSize aSize, VkDeviceSize anOffset)
	{
		myDescriptor.buffer = myBuffer;
		myDescriptor.offset = anOffset;
		myDescriptor.range = aSize;
	}

	void VulkanBuffer::Destroy()
	{
		if (myBuffer)
			vmaDestroyBuffer(VulkanRenderCore::GetInstance()->GetAllocator(), myBuffer, myAllocation);
	}

}
