#pragma once

#include "VulkanImage.h"

namespace Render
{
namespace Vulkan
{
namespace glTF
{
	struct Image
	{
		~Image();

		void Load(const tinygltf::Model& aModel, uint anImageIndex, VkQueue aTransferQueue);
		void LoadEmpty(VkQueue aTransferQueue);

		void Load(uint aWidth, uint aHeight, const unsigned char* aBuffer, VkDeviceSize aBufferSize, VkQueue aTransferQueue);

		Render::Vulkan::Image myImage;
	};

	struct Texture
	{
		uint myImageIndex;
	};
}
}
}
