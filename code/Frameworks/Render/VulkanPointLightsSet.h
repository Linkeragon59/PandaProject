#pragma once

#include "RenderLight.h"
#include "VulkanBuffer.h"

namespace Render::Vulkan
{
	class PointLightsSet
	{
	public:
		void Setup();
		void Destroy();

		void ClearLightData();
		void AddLight(const PointLight& aPointLight);
		void UpdateUBO();

		void Bind(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

	private:
		static const uint ourMaxNumLights = 64;
		struct LightData
		{
			PointLight myLights[ourMaxNumLights];
		} myLightsData;
		uint myNumLights = 0;

		Buffer myLightsUBO;

		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
