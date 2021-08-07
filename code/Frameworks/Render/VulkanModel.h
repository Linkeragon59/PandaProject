#pragma once

#include "RenderModel.h"

namespace Render::Vulkan
{
	class Model : public Render::Model
	{
	public:
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) const = 0;
	};
}
