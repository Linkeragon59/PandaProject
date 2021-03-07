#include "VulkanImage.h"

#include "VulkanRenderCore.h"

namespace Render
{
	void VulkanImage::Create(uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties)
	{
		myDevice = VulkanRenderCore::GetInstance()->GetDevice();
		myAllocator = VulkanRenderCore::GetInstance()->GetAllocator();

		myFormat = aFormat;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = myFormat;
		imageInfo.extent = { aWidth, aHeight, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = aTiling;
		imageInfo.usage = aUsage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0; // Ignored when using VK_SHARING_MODE_EXCLUSIVE
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocInfo{};
		if (someProperties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else
		{
			throw std::runtime_error("Image allocation usage not supported yet.");
		}
		allocInfo.requiredFlags = someProperties;

		VK_CHECK_RESULT(vmaCreateImage(myAllocator, &imageInfo, &allocInfo, &myImage, &myAllocation, nullptr), "Failed to create an image!");
	}

	void VulkanImage::CreateImageView(VkImageAspectFlags someAspects)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = myImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = myFormat;
		viewInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
		};
		viewInfo.subresourceRange.aspectMask = someAspects;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VK_CHECK_RESULT(vkCreateImageView(myDevice, &viewInfo, nullptr, &myImageView), "Failed to create an image view!");
	}

	void VulkanImage::Destroy()
	{
		if (myImageView)
		{
			vkDestroyImageView(myDevice, myImageView, nullptr);
			myImageView = VK_NULL_HANDLE;
		}
		
		if (myImage)
		{
			vmaDestroyImage(myAllocator, myImage, myAllocation);
			myImage = VK_NULL_HANDLE;
			myAllocation = VK_NULL_HANDLE;
			myFormat = VK_FORMAT_UNDEFINED;
		}
	}
}
