#include "VulkanModel.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include "DummyVulkanPSO.h"

#include <stb_image.h>

#include "Input.h"
#include <chrono>

namespace Render
{
	namespace
	{
		const std::vector<DummyVulkanPSO::Vertex> locVertices =
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

		const std::vector<uint32_t> locIndices =
		{
			0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1,

			7, 8, 9, 7, 9, 10, 7, 10, 11, 7, 11, 12, 7, 12, 13, 7, 13, 8
		};

		const std::string locTestTexture = "Frameworks/textures/panda.jpg";
	}

	void VulkanModel::Update()
	{
		// Using a UBO this way is not the most efficient way to pass frequently changing values to the shader.
		// A more efficient way to pass a small buffer of data to shaders are push constants.
		static auto startTime = std::chrono::high_resolution_clock::now();
		static float addTime = 0;

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		Input::RawInputState spaceState = inputManager->PollRawInput(Input::RawInput::KeySpace);
		if (spaceState == Input::RawInputState::Pressed)
		{
			addTime += 0.1f;
		}

		auto currentTime = std::chrono::high_resolution_clock::now();
		float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() + addTime;
		if (myPosition.x < 0.f)
			elapsedTime *= -1.0f;

		DummyVulkanPSO::PerObjectUBO ubo{};
		ubo.myMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), myPosition), elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		memcpy(myUBO.myMappedData, &ubo, sizeof(ubo));
	}

	void VulkanModel::Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout)
	{
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, 1, 1, &myDescriptorSet, 0, NULL);

		vkCmdBindIndexBuffer(aCommandBuffer, myIndexBuffer.myBuffer, 0, VK_INDEX_TYPE_UINT32);
		std::array<VkBuffer, 1> modelVertexBuffers = { myVertexBuffer.myBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(aCommandBuffer, 0, (uint32_t)modelVertexBuffers.size(), modelVertexBuffers.data(), offsets);

		vkCmdDrawIndexed(aCommandBuffer, myIndexCount, 1, 0, 0, 0);
	}

	VulkanModel::VulkanModel(const glm::vec3& aPosition)
		: myPosition(aPosition)
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();

		VkDeviceSize vertexBufferSize = sizeof(DummyVulkanPSO::Vertex) * locVertices.size();
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
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		myIndexCount = (uint32_t)locIndices.size();

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

		myTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VulkanRenderer::GetInstance()->GetGraphicsQueue());

		myTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myTexture.CreateImageSampler();
		myTexture.SetupDescriptor();

		SetupDescriptorPool();

		PrepareUniformBuffers();
		SetupDescriptoSet();
	}

	VulkanModel::~VulkanModel()
	{
		myDescriptorSet = VK_NULL_HANDLE;
		myUBO.Destroy();

		myVertexBuffer.Destroy();
		myIndexBuffer.Destroy();
		myIndexCount = 0;

		myTexture.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void VulkanModel::SetupDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void VulkanModel::PrepareUniformBuffers()
	{
		myUBO.Create(sizeof(DummyVulkanPSO::PerObjectUBO),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myUBO.SetupDescriptor();

		// Persistent map
		myUBO.Map();

		DummyVulkanPSO::PerObjectUBO ubo{};
		ubo.myMatrix = glm::translate(glm::mat4(1.0f), myPosition);

		memcpy(myUBO.myMappedData, &ubo, sizeof(ubo));
	}

	void VulkanModel::SetupDescriptoSet()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { DummyVulkanPSO::ourPerObjectDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor set");

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		// Binding 0 : Vertex shader uniform buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = &myUBO.myDescriptor;

		// Binding 1 : Fragment shader sampler
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = myDescriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].pImageInfo = &myTexture.myDescriptor;

		vkUpdateDescriptorSets(myDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
