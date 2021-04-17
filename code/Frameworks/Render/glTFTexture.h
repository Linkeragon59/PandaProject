#pragma once

#include "VulkanImage.h"

namespace Render
{
namespace glTF
{
	struct Image
	{
		~Image();

		void Load(const tinygltf::Model& aModel, uint32_t anImageIndex, VkQueue aTransferQueue);
		void LoadEmpty(VkQueue aTransferQueue);

		void Load(uint32_t aWidth, uint32_t aHeight, const unsigned char* aBuffer, VkDeviceSize aBufferSize, VkQueue aTransferQueue);

		Render::Vulkan::Image myImage;

		void SetupDescriptorSet(VkDescriptorPool aDescriptorPool);
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};

	struct Texture
	{
		uint32_t myImageIndex;
	};
}
}
