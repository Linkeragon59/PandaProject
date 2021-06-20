#pragma once

namespace Render
{
namespace Vulkan
{
	struct Image
	{
		static bool DepthFormatHasStencilAspect(VkFormat aDepthFormat)
		{
			return aDepthFormat >= VK_FORMAT_D16_UNORM_S8_UINT;
		}

		~Image();

		void Create(uint aWidth, uint aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties);
		VkImage myImage = VK_NULL_HANDLE;
		VmaAllocation myAllocation = VK_NULL_HANDLE;
		VkFormat myFormat = VK_FORMAT_UNDEFINED;
		VkExtent3D myExtent = { 0, 0, 0 };

		void CreateImageView(VkImageAspectFlags someAspects);
		VkImageView myImageView = VK_NULL_HANDLE;

		void CreateImageSampler();
		VkSampler myImageSampler = VK_NULL_HANDLE;

		void SetupDescriptor(VkImageLayout aLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkDescriptorImageInfo myDescriptor;

		void TransitionLayout(VkImageLayout anOldLayout, VkImageLayout aNewLayout, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);

		void Destroy();

		VkDevice myDevice = VK_NULL_HANDLE;
		VmaAllocator myAllocator = VK_NULL_HANDLE;
	};
}
}
