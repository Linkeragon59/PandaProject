#include "VulkanPointLightsSet.h"

#include "VulkanRender.h"
#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"

namespace Render::Vulkan
{
	void PointLightsSet::Setup()
	{
		myDevice = RenderCore::GetInstance()->GetDevice();

		myLightsUBO.Create(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myLightsUBO.SetupDescriptor();
		myLightsUBO.Map();

		ClearLightData();
		SetupLightsDescriptors();
	}

	void PointLightsSet::Destroy()
	{
		myLightsUBO.Destroy();

		vkDestroyDescriptorPool(myDevice, myLightsDescriptorPool, nullptr);

		myDevice = VK_NULL_HANDLE;
	}

	void PointLightsSet::ClearLightData()
	{
		for (PointLight& light : myLightsData.myLights)
		{
			light.myPosition = glm::vec4(0.0f);
			light.myColor = glm::vec4(0.0f);
		}
		myNumLights = 0;
	}

	void PointLightsSet::AddLight(const PointLight& aPointLight)
	{
		if (myNumLights >= ourMaxNumLights)
			return;

		myLightsData.myLights[myNumLights] = aPointLight;
		myNumLights++;
	}

	void PointLightsSet::UpdateUBO()
	{
		memcpy(myLightsUBO.myMappedData, &myLightsData, sizeof(LightData));
	}

	void PointLightsSet::Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) const
	{
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &myLightsDescriptorSet, 0, NULL);
	}

	void PointLightsSet::SetupLightsDescriptors()
	{
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo{};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = (uint)poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = 1;

		VK_CHECK_RESULT(vkCreateDescriptorPool(myDevice, &descriptorPoolInfo, nullptr, &myLightsDescriptorPool), "Failed to create the descriptor pool");

		std::array<VkDescriptorSetLayout, 1> layouts = { ShaderHelpers::GetLightsDescriptorSetLayout() };
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = myLightsDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint)layouts.size();
		VK_CHECK_RESULT(vkAllocateDescriptorSets(myDevice, &descriptorSetAllocateInfo, &myLightsDescriptorSet), "Failed to create the node descriptor set");

		std::array<VkWriteDescriptorSet, 1> writeDescriptorSets{};
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].dstSet = myLightsDescriptorSet;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].pBufferInfo = &myLightsUBO.myDescriptor;
		writeDescriptorSets[0].descriptorCount = 1;
		vkUpdateDescriptorSets(myDevice, static_cast<uint>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
	}
}
