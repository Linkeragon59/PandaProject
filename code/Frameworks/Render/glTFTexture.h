#pragma once

#include "VulkanImage.h"

namespace Render
{
	struct glTFTexture
	{
		~glTFTexture();

		void Load(const tinygltf::Image& anImage, VkQueue aTransferQueue);

		VulkanImage myImage;
	};
}
