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

		VulkanImage myImage;

		void SetupDescriptorSet(VkDescriptorPool aDescriptorPool);
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};

	struct Texture
	{
		uint32_t myImageIndex;
	};
}
}
