#pragma once

#include "RenderModel.h"
#include "VulkanShaderHelpers.h"

namespace Render::Vulkan
{
	class Model : public Render::Model
	{
	public:
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::DescriptorLayout aLayout) = 0;
	};
}
