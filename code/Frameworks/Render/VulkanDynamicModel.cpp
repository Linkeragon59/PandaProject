#include "VulkanDynamicModel.h"

#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"
#include "VulkanRender.h"

#include <stb_image.h>

#include "Input.h"
#include <chrono>

namespace Render::Vulkan
{
	DynamicModel::DynamicModel(const BaseModelData& someData)
	{
		Assert(someData.GetType() == BaseModelData::Type::Dynamic);
		const DynamicModelData& modelData = static_cast<const DynamicModelData&>(someData);

		std::vector< ShaderHelpers::Vertex> fullVertices;
		fullVertices.reserve(modelData.myVertices.size());
		for (const DynamicModelData::Vertex& vertex : modelData.myVertices)
		{
			ShaderHelpers::Vertex fullVertex =
			{
				vertex.myPosition,
				vertex.myNormal,
				vertex.myUV,
				vertex.myColor,
				{0.0f, 0.0f, 0.0f, 0.0f},
				{0.25f, 0.25f, 0.25f, 0.25f},
				{0.0f, 0.0f, 0.0f, 0.0f}
			};
			fullVertices.push_back(fullVertex);
		}

		VkDeviceSize vertexBufferSize = sizeof(ShaderHelpers::Vertex) * modelData.myVertices.size();
		VkDeviceSize indexBufferSize = sizeof(uint) * modelData.myIndices.size();
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(modelData.myTextureFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		Assert(pixels, "Failed to load an image!");

		VkDeviceSize textureSize = static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4;
		Buffer vertexStaging, indexStaging, textureStaging;

		vertexStaging.Create(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vertexStaging.Map();
		memcpy(vertexStaging.myMappedData, fullVertices.data(), (size_t)vertexBufferSize);
		vertexStaging.Unmap();

		indexStaging.Create(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		indexStaging.Map();
		memcpy(indexStaging.myMappedData, modelData.myIndices.data(), (size_t)indexBufferSize);
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
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myIndexCount = (uint)modelData.myIndices.size();

		myTexture.Create(texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myTexture.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			RenderCore::GetInstance()->GetGraphicsQueue());

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
			imageCopyRegion.imageExtent = { static_cast<uint>(texWidth), static_cast<uint>(texHeight), 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myTexture.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		EndOneTimeCommand(commandBuffer, RenderCore::GetInstance()->GetGraphicsQueue());

		vertexStaging.Destroy();
		indexStaging.Destroy();
		textureStaging.Destroy();

		myTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			RenderCore::GetInstance()->GetGraphicsQueue());

		myTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myTexture.CreateImageSampler();
		myTexture.SetupDescriptor();

		myUBOObject.Create(sizeof(ShaderHelpers::ModelMatrixData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBOObject.SetupDescriptor();
		myUBOObject.Map();
	}

	DynamicModel::~DynamicModel()
	{
		myUBOObject.Destroy();
		myTexture.Destroy();

		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();
		myIndexCount = 0;
	}

	void DynamicModel::Update(const BaseModelData& someData)
	{
		memcpy(myUBOObject.myMappedData, &someData.myMatrix, sizeof(glm::mat4));
	}

	void DynamicModel::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType)
	{
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT32);
		std::array<VkBuffer, 1> modelVertexBuffers = { myVertexBuffer.myBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, (uint)modelVertexBuffers.size(), modelVertexBuffers.data(), offsets);

		ShaderHelpers::ObjectDescriptorInfo info;
		info.myModelMatrixInfo = &myUBOObject.myDescriptor;
		info.myImageSamplerInfo = &myTexture.myDescriptor;
		VkDescriptorSet descriptorSet = RenderCore::GetInstance()->GetDescriptorSet(aType, info);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, NULL);

		vkCmdDrawIndexed(aCommandBuffer, myIndexCount, 1, 0, 0, 0);
	}
}
