#pragma once

#include "Render_Model.h"

namespace Render
{
	class ModelImpl : public Model
	{
	public:
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) = 0;
	};
}
