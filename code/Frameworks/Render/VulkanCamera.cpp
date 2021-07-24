#include "VulkanCamera.h"
#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"
#include "VulkanRender.h"

namespace Render::Vulkan
{
	Camera::Camera()
	{
		myViewProjUBO.Create(sizeof(ShaderHelpers::ViewProjData),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myViewProjUBO.SetupDescriptor();
		myViewProjUBO.Map();

		Update(glm::mat4(1.0f), glm::mat4(1.0f));

		VkDevice device = RenderCore::GetInstance()->GetDevice();

		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &myDescriptorPool), "Failed to create the descriptor pool");

		std::array<VkDescriptorSetLayout, 1> layouts = { ShaderHelpers::GetCameraDescriptorSetLayout() };
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the descriptor set");

		std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = myDescriptorSet;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].pBufferInfo = &myViewProjUBO.myDescriptor;
		writeDescriptorSets[0].descriptorCount = 1;
		vkUpdateDescriptorSets(device, (uint)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
	}

	Camera::~Camera()
	{
		vkDestroyDescriptorPool(RenderCore::GetInstance()->GetDevice(), myDescriptorPool, nullptr);

		myViewProjUBO.Destroy();
	}

	void Camera::Update(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myView = aView;
		myProjection = aProjection;

		ShaderHelpers::ViewProjData data;
		data.myView = myView;
		data.myProjection = myProjection;
		memcpy(myViewProjUBO.myMappedData, &data, sizeof(ShaderHelpers::ViewProjData));
	}

	void Camera::BindViewProj(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aSetIndex)
	{
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aSetIndex, 1, &myDescriptorSet, 0, NULL);
	}
}
