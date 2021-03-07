#pragma once

#include "VulkanHelpers.h"
#include "VulkanImage.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace Render
{
namespace VulkanglTF
{
	struct Texture
	{
		~Texture();

		void Load(const tinygltf::Image& anImage, VkQueue aTransferQueue);

		VulkanImage myImage;
	};
}
}
