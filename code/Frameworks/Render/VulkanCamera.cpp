#include "VulkanCamera.h"

#include "VulkanRenderer.h"

namespace Render
{

	VulkanCamera::VulkanCamera(VkDescriptorSetLayout aDescriptorSetLayout)
		: myDescriptorSetLayout(aDescriptorSetLayout)
	{
		myDevice = VulkanRenderer::GetInstance()->GetDevice();

		SetupDescriptorPool();
		PrepareUniformBuffers();
		SetupDescriptorSet();
	}

	VulkanCamera::~VulkanCamera()
	{
		myDescriptorSet = VK_NULL_HANDLE;
		myUBO.Destroy();

		vkDestroyDescriptorPool(myDevice, myDescriptorPool, nullptr);
		myDescriptorPool = VK_NULL_HANDLE;
	}

	void VulkanCamera::SetupDescriptorPool()
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
		descriptorPoolInfo.maxSets = 2; // TODO: Understand this

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");
	}

	void VulkanCamera::PrepareUniformBuffers()
	{
		myUBO.Create(sizeof(UniformBufferObject),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Persistent map
		myUBO.Map();

		UniformBufferObject ubo{};
		ubo.myModel = glm::rotate(glm::mat4(1.0f), 0.f, glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.myView = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.myProj = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 10.0f);
		ubo.myProj[1][1] *= -1; // adapt calculation for Vulkan

		memcpy(myUBO.myMappedData, &ubo, sizeof(ubo));

		myUBO.SetupDescriptor();
	}

	void VulkanCamera::SetupDescriptorSet()
	{
		std::array<VkDescriptorSetLayout, 1> layouts = { myDescriptorSetLayout };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor pool");

		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = myDescriptorSet;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.dstBinding = 0;
		writeDescriptorSet.pBufferInfo = &myUBO.myDescriptor;
		writeDescriptorSet.descriptorCount = 1;
		std::vector<VkWriteDescriptorSet> writeDescriptorSets = { writeDescriptorSet };

		vkUpdateDescriptorSets(myDevice, (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}
}
