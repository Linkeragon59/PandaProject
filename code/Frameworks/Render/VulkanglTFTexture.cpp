#include "VulkanglTFTexture.h"

#include "VulkanRenderCore.h"
#include "VulkanBuffer.h"

namespace Render
{
namespace VulkanglTF
{

	Texture::~Texture()
	{
		myImage.Destroy();
	}

	void Texture::Load(const tinygltf::Image& anImage, VkQueue aTransferQueue)
	{
		// TODO: check anImage.component, if equal to 3 and the Vulkan device doesn't support RGB only, modify the buffer
		assert(anImage.component == 4);
		const unsigned char* buffer = &anImage.image[0];
		VkDeviceSize bufferSize = anImage.image.size();

		uint32_t width = anImage.width;
		uint32_t height = anImage.height;
		// TODO : generate MipMaps
		//uint32_t mipsLevels = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

		VulkanBuffer stagingBuffer;
		stagingBuffer.Create(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		memcpy(stagingBuffer.myMappedData, buffer, bufferSize);
		stagingBuffer.Unmap();

		myImage.Create(width,
			height,
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
			copyRegion.imageExtent = { width, height, 1 };
			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.myBuffer, myImage.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		}
		EndOneTimeCommand(commandBuffer, aTransferQueue);
		myImage.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aTransferQueue);

		stagingBuffer.Destroy();

		// TODO: Add sampler support to VulkanImage, and VkDescriptorImageInfo descriptor

		myImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
	}
}
}
