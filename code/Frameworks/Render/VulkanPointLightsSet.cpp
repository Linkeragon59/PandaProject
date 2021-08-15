#include "VulkanPointLightsSet.h"

#include "VulkanRender.h"
#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"

namespace Render::Vulkan
{
	void PointLightsSet::Setup()
	{
		myLightsUBO.Create(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myLightsUBO.SetupDescriptor();
		myLightsUBO.Map();

		ClearLightData();
	}

	void PointLightsSet::Destroy()
	{
		myLightsUBO.Destroy();
	}

	void PointLightsSet::ClearLightData()
	{
		for (PointLight& light : myLightsData.myLights)
		{
			light.myPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
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

	void PointLightsSet::Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex)
	{
		ShaderHelpers::LightsSetDescriptorInfo info;
		info.myLightsInfo = &myLightsUBO.myDescriptor;
		VkDescriptorSet descriptorSet = RenderCore::GetInstance()->GetDescriptorSet(ShaderHelpers::BindType::LightsSet, info);
		vkCmdBindDescriptorSets(aCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, aPipelineLayout, aDescriptorSetIndex, 1, &descriptorSet, 0, NULL);
	}
}
