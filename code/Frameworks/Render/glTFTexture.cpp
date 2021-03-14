#include "glTFTexture.h"

#include "glTFVulkanPSO.h"

#include "VulkanRenderer.h"
#include "VulkanHelpers.h"
#include "VulkanBuffer.h"

namespace Render
{
namespace glTF
{
	Image::~Image()
	{
		myImage.Destroy();
		myDescriptorSet = VK_NULL_HANDLE;
	}

	void Image::Load(const tinygltf::Model& aModel, uint32_t anImageIndex, VkQueue aTransferQueue)
	{
		const tinygltf::Image& gltfImage = aModel.images[anImageIndex];

		// TODO: check anImage.component, if equal to 3 and the Vulkan device doesn't support RGB only, modify the buffer
		assert(gltfImage.component == 4);
		const unsigned char* buffer = &gltfImage.image[0];
		VkDeviceSize bufferSize = gltfImage.image.size();

		uint32_t width = gltfImage.width;
		uint32_t height = gltfImage.height;
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

		myImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myImage.CreateImageSampler();
		myImage.SetupDescriptor();
	}

	void Image::SetupDescriptorSet(VkDescriptorPool aDescriptorPool)
	{
		VkDevice device = VulkanRenderer::GetInstance()->GetDevice();

		std::array<VkDescriptorSetLayout, 1> layouts = { glTF::VulkanPSO::ourPerImageDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = aDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the skin descriptor set");

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		// Binding 0 : Fragment shader sampler
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].pImageInfo = &myImage.myDescriptor;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
}
