#pragma once

#include "Render_VulkanImage.h"

namespace Render
{
	struct glTFImage
	{
		void Load(const tinygltf::Model& aModel, uint anImageIndex, VkQueue aTransferQueue);
		void LoadEmpty(VkQueue aTransferQueue);

		void Load(uint aWidth, uint aHeight, const unsigned char* aBuffer, VkDeviceSize aBufferSize, VkQueue aTransferQueue);

		VulkanImagePtr myImage;
	};

	struct glTFTexture
	{
		uint myImageIndex;
	};
}
