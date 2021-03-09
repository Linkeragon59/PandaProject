#include "VulkanModel.h"

#include "VulkanRenderer.h"

#include <stb_image.h>

namespace Render
{
	namespace
	{
		const std::vector<VulkanModel::Vertex> locVertices =
		{
			{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}},
			{{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}},
			{{0.87f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.065f, 0.25f}},
			{{0.87f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.065f, 0.75f}},
			{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.5f, 1.0f}},
			{{-0.87f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.935f, 0.75f}},
			{{-0.87f, -0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.935f, 0.25f}},

			{{0.0f, 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}},
			{{0.0f, -1.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}},
			{{0.87f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.065f, 0.25f}},
			{{0.87f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.065f, 0.75f}},
			{{0.0f, 1.0f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.5f, 1.0f}},
			{{-0.87f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.935f, 0.75f}},
			{{-0.87f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.935f, 0.25f}}
		};

		const std::vector<uint16_t> locIndices =
		{
			0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1,

			7, 8, 9, 7, 9, 10, 7, 10, 11, 7, 11, 12, 7, 12, 13, 7, 13, 8
		};

		const std::string locTestTexture = "Frameworks/textures/panda.jpg";
	}

	VulkanModel::VulkanModel()
	{
		VkDeviceSize vertexBufferSize = sizeof(Vertex) * locVertices.size();
		VkDeviceSize indexBufferSize = sizeof(uint32_t) * locIndices.size();
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(locTestTexture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("Failed to load an image!");
		}
		VkDeviceSize textureSize = static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4;

		VulkanBuffer vertexStaging, indexStaging, textureStaging;

		vertexStaging.Create(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vertexStaging.Map();
		memcpy(vertexStaging.myMappedData, locVertices.data(), (size_t)vertexBufferSize);
		vertexStaging.Unmap();

		indexStaging.Create(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexStaging.Map();
		memcpy(indexStaging.myMappedData, locIndices.data(), (size_t)indexBufferSize);
		indexStaging.Unmap();

		textureStaging.Create(textureSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		memcpy(textureStaging.myMappedData, pixels, static_cast<size_t>(textureSize));
		textureStaging.Unmap();

		stbi_image_free(pixels);

		myVertexBuffer.Create(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myIndexBuffer.Create(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myTexture.Create(texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myTexture.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VulkanRenderer::GetInstance()->GetGraphicsQueue());

		VkCommandBuffer commandBuffer = BeginOneTimeCommand();
		{
			VkBufferCopy bufferCopyRegion{};
			bufferCopyRegion.size = vertexBufferSize;
			vkCmdCopyBuffer(commandBuffer, vertexStaging.myBuffer, myVertexBuffer.myBuffer, 1, &bufferCopyRegion);

			bufferCopyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(commandBuffer, indexStaging.myBuffer, myIndexBuffer.myBuffer, 1, &bufferCopyRegion);

			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myTexture.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		EndOneTimeCommand(commandBuffer, VulkanRenderer::GetInstance()->GetGraphicsQueue());

		vertexStaging.Destroy();
		indexStaging.Destroy();
		textureStaging.Destroy();

		myVertexBuffer.SetupDescriptor();
		myIndexBuffer.SetupDescriptor();

		myTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VulkanRenderer::GetInstance()->GetGraphicsQueue());

		myTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myTexture.CreateImageSampler();
	}

	VulkanModel::~VulkanModel()
	{
		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();

		myTexture.Destroy();
	}
}
