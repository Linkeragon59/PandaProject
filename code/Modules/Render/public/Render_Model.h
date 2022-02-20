#pragma once

#include "Render_ModelData.h"
#include "Render_ShaderHelpers.h"

namespace Render
{
	class Model
	{
	public:
		virtual ~Model() {};
		virtual void Update(const ModelData& someData) = 0;
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) = 0;
	};
}