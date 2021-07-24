#pragma once

#include "RenderModel.h"

namespace Render::Vulkan
{
	class VulkanModel : public Model
	{
	public:
		VulkanModel(const RenderData& someRenderData)
			: Model(someRenderData)
		{}

		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) = 0;
	};
}
