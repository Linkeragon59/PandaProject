#pragma once

#include "Render_Light.h"
#include "Render_VulkanBuffer.h"

namespace Render
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

		VulkanBufferPtr myLightsUBO;
	};
}
