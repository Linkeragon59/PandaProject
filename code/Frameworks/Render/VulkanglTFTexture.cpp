#include "VulkanglTFTexture.h"

#include "VulkanRenderer.h"
#include "VulkanHelpers.h"
#include "VulkanBuffer.h"
#include "VulkanDeferredPipeline.h"

namespace Render
{
namespace Vulkan
{
namespace glTF
{
	Image::~Image()
	{
		myImage.Destroy();
	}

	void Image::Load(const tinygltf::Model& aModel, uint anImageIndex, VkQueue aTransferQueue)
	{
		const tinygltf::Image& gltfImage = aModel.images[anImageIndex];

		// TODO: check anImage.component, if equal to 3 and the Vulkan device doesn't support RGB only, modify the buffer
		Assert(gltfImage.component == 4);
		const unsigned char* buffer = &gltfImage.image[0];
		VkDeviceSize bufferSize = gltfImage.image.size();

		uint width = gltfImage.width;
		uint height = gltfImage.height;
		
		Load(width, height, buffer, bufferSize, aTransferQueue);
	}

	void Image::LoadEmpty(VkQueue aTransferQueue)
	{
		uint width = 1;
		uint height = 1;
		VkDeviceSize bufferSize = width * height * 4;
		unsigned char* buffer = new unsigned char[bufferSize];
		memset(buffer, 0xff, bufferSize);

		Load(width, height, buffer, bufferSize, aTransferQueue);

		delete[] buffer;
	}

	void Image::Load(uint aWidth, uint aHeight, const unsigned char* aBuffer, VkDeviceSize aBufferSize, VkQueue aTransferQueue)
	{
		// TODO : generate MipMaps
		//uint mipsLevels = static_cast<uint>(floor(log2(std::max(width, height))) + 1.0);

		Buffer stagingBuffer;
		stagingBuffer.Create(aBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		memcpy(stagingBuffer.myMappedData, aBuffer, aBufferSize);
		stagingBuffer.Unmap();

		myImage.Create(aWidth, aHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// TODO: Could merge all the commands together instead of doing several commands
		myImage.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, aTransferQueue);
		VkCommandBuffer commandBuffer = BeginOneTimeCommand();
		{
			VkBufferImageCopy copyRegion{};
			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = { aWidth, aHeight, 1 };
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.myBuffer, myImage.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		}
		EndOneTimeCommand(commandBuffer, aTransferQueue);
		myImage.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aTransferQueue);

		stagingBuffer.Destroy();

		myImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myImage.CreateImageSampler();
		myImage.SetupDescriptor();
	}
}
}
}
