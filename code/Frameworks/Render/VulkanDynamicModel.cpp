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
		myDevice = RenderCore::GetInstance()->GetDevice();

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
				{1.0f, 1.0f, 1.0f, 1.0f},
				{1.0f, 1.0f, 1.0f, 1.0f}
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

		PrepareBuffers();

		SetupDescriptorPool();
		SetupDescriptoSets();
	}

	DynamicModel::~DynamicModel()
	{
		myUBOObject.Destroy();
		mySSBOSkin.Destroy();
		myTexture.Destroy();
		mySSBOMaterial.Destroy();

		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();
		myIndexCount = 0;

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void DynamicModel::Update(const BaseModelData& someData)
	{
		memcpy(myUBOObject.myMappedData, &someData.myMatrix, sizeof(glm::mat4));
		// Using a UBO this way is not the most efficient way to pass frequently changing values to the shader.
		// A more efficient way to pass a small buffer of data to shaders are push constants.
		/*static auto startTime = std::chrono::high_resolution_clock::now();
		static float addTime = 0;

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		Input::RawInputState spaceState = inputManager->PollRawInput(Input::RawInput::KeySpace);
		if (spaceState == Input::RawInputState::Pressed)
		{
			addTime += 0.1f;
		}

		auto currentTime = std::chrono::high_resolution_clock::now();
		float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() + addTime;
		if (GetPosition().x < 0.f)
			elapsedTime *= -1.0f;

		ShaderHelpers::ModelData ubo{};
		ubo.myModel = glm::rotate(glm::translate(glm::mat4(1.0f), GetPosition()), elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		memcpy(myUBOObject.myMappedData, &ubo, sizeof(ubo));*/
	}

	void DynamicModel::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) const
	{
		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT32);
		std::array<VkBuffer, 1> modelVertexBuffers = { myVertexBuffer.myBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, (uint)modelVertexBuffers.size(), modelVertexBuffers.data(), offsets);

		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &myDescriptorSet, 0, NULL);

		vkCmdDrawIndexed(aCommandBuffer, myIndexCount, 1, 0, 0, 0);
	}

	void DynamicModel::SetupDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 2;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void DynamicModel::PrepareBuffers()
	{
		myUBOObject.Create(sizeof(ShaderHelpers::ModelData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBOObject.SetupDescriptor();
		myUBOObject.Map();

		glm::mat4 identity(1.0f);
		mySSBOSkin.Create(sizeof(glm::mat4),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBOSkin.SetupDescriptor();
		mySSBOSkin.Map();
		memcpy(mySSBOSkin.myMappedData, &identity, sizeof(glm::mat4));
		mySSBOSkin.Unmap();
		
		glm::vec4 colorFactor(1.0f);
		mySSBOMaterial.Create(sizeof(glm::vec4),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBOMaterial.SetupDescriptor();
		mySSBOMaterial.Map();
		memcpy(mySSBOMaterial.myMappedData, &colorFactor, sizeof(glm::vec4));
		mySSBOMaterial.Unmap();
	}

	void DynamicModel::SetupDescriptoSets()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { ShaderHelpers::GetObjectDescriptorSetLayout() };
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor set");

		std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = &myUBOObject.myDescriptor;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = myDescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].pBufferInfo = &mySSBOSkin.myDescriptor;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = myDescriptorSet;
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].pImageInfo = &myTexture.myDescriptor;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = myDescriptorSet;
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[3].pBufferInfo = &mySSBOMaterial.myDescriptor;

		vkUpdateDescriptorSets(myDevice, static_cast<uint>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
